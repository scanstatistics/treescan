//***************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//***************************************************************************

#include "HypergeometricProbabilityLookup.h"
#include <fstream>
#include <numeric>

// Macro for accessing natural log calculations from precomputed vector or standard log function
#define STD_LOG(c) caseLog[(unsigned int)c] // STD_LOG(c) log(static_cast<double>(c))

namespace {
    // Returns log(choose(n, k)). We use log-combinations so large factorials are
    // never formed directly; lgamma(n + 1) is log(n!).
    double logChoose(count_t n, count_t k) {
        if (k < 0 || k > n) return -std::numeric_limits<double>::infinity();
        return std::lgamma(static_cast<double>(n) + 1.0) -
               std::lgamma(static_cast<double>(k) + 1.0) -
               std::lgamma(static_cast<double>(n - k) + 1.0);
    }
    // Hypergeometric PMF (probability mass function):
    //   P(X=x) = choose(S, x) * choose(C-S, T-x) / choose(C, T)
    // where C is all cases, S is spatial cases, T is window cases, and x is
    // cases that are both spatial and in the time window.
    double logHypergeometricPmf(count_t C, count_t S, count_t T, count_t x) {
        return logChoose(S, x) + logChoose(C - S, T - x) - logChoose(C, T);
    }
    // Sums probabilities from X=from through X=to without converting each PMF
    // to ordinary probability first. This avoids underflow for very strong clusters.
    double logSumExpHypergeometricTail(count_t C, count_t S, count_t T, count_t from, count_t to) {
        double maxLogP = -std::numeric_limits<double>::infinity();
        for (count_t k = from; k <= to; ++k)
            maxLogP = std::max(maxLogP, logHypergeometricPmf(C, S, T, k));
        if (!std::isfinite(maxLogP)) return maxLogP;

        double scaledSum = 0.0;
        for (count_t k = from; k <= to; ++k)
            scaledSum += std::exp(logHypergeometricPmf(C, S, T, k) - maxLogP);
        return maxLogP + std::log(scaledSum);
    }
    // Adds two log-probabilities and returns log(exp(left) + exp(right)). Used
    // by the dense table so the cumulative tail is accurate before clamping.
    double logAddExp(double left, double right) {
        if (!std::isfinite(left)) return right;
        if (!std::isfinite(right)) return left;
        double maxLogP = std::max(left, right);
        return maxLogP + std::log(std::exp(left - maxLogP) + std::exp(right - maxLogP));
    }
    // Converts a finite log-probability to probability. If the true probability
    // is smaller than double can represent, clamp to denorm_min() so callers see
    // a tiny non-zero probability instead of losing the cluster as zero/unset.
    double probabilityFromLog(double logProbability) {
        if (!std::isfinite(logProbability)) return 0.0;
        double probability = std::exp(logProbability);
        return probability == 0.0 ? std::numeric_limits<double>::denorm_min() : probability;
    }
    // TreeScan stores hypergeometric tail probabilities as negative values.
    // The clamp belongs here, after the complete tail probability is known.
    double negativeProbabilityFromLogTail(double logTail) {
        if (!std::isfinite(logTail)) return HypergeometricProbabilityLookup::PROBABILITY_UNSET;
        return -probabilityFromLog(logTail);
    }
    // Builds the ordinary PMF array used by the day-of-week convolution. The
    // scalar non-DOW tails use log-space directly and do not need this array.
    HypergeometricProbabilityLookup::OffsetPmf calculateHypergeometricPmf(count_t C, count_t S, count_t T) {
        if (C == 0) {
            return (S == 0 && T == 0) ?
                HypergeometricProbabilityLookup::OffsetPmf{ 0, std::vector<double>(1, 1.0) } :
                HypergeometricProbabilityLookup::OffsetPmf();
        }
        if (S > C || T > C) return HypergeometricProbabilityLookup::OffsetPmf();

        count_t min_x = std::max(static_cast<count_t>(0), S + T - C);
        count_t max_x = std::min(S, T);
        HypergeometricProbabilityLookup::OffsetPmf pmf;
        pmf.offset = min_x;
        pmf.probabilities.resize(max_x - min_x + 1, 0.0);
        for (count_t x = min_x; x <= max_x; ++x) {
            pmf.probabilities[x - min_x] = probabilityFromLog(logHypergeometricPmf(C, S, T, x));
        }
        return pmf;
    }
    // Convolution combines independent daily PMFs. Offsets let us store only
    // possible x values, so impossible leading zeroes do not inflate each multiply.
    HypergeometricProbabilityLookup::OffsetPmf convolve(
        const HypergeometricProbabilityLookup::OffsetPmf& left,
        const HypergeometricProbabilityLookup::OffsetPmf& right
    ) {
        if (left.empty() || right.empty()) return HypergeometricProbabilityLookup::OffsetPmf();

        HypergeometricProbabilityLookup::OffsetPmf result;
        result.offset = left.offset + right.offset;
        result.probabilities.assign(left.size() + right.size() - 1, 0.0);
        for (size_t i = 0; i < left.size(); ++i) {
            if (left.probabilities[i] == 0.0) continue;
            for (size_t j = 0; j < right.size(); ++j) {
                if (right.probabilities[j] != 0.0)
                    result.probabilities[i + j] += left.probabilities[i] * right.probabilities[j];
            }
        }
        return result;
    }

    // Stores both cumulative directions for a combined DOW distribution. That
    // lets later requests with the same margins but a different x answer the
    // requested tail by one array lookup instead of reconvolving seven PMFs.
    HypergeometricProbabilityLookup::OffsetTailProbabilities buildTailProbabilities(
        const HypergeometricProbabilityLookup::OffsetPmf& pmf, double expected
    ) {
        if (pmf.empty()) return HypergeometricProbabilityLookup::OffsetTailProbabilities();

        HypergeometricProbabilityLookup::OffsetTailProbabilities tails;
        tails.offset = pmf.offset;
        tails.expected = expected;
        tails.lowerTailProbabilities.resize(pmf.size(), 0.0);
        tails.upperTailProbabilities.resize(pmf.size(), 0.0);

        double lowerTail = 0.0;
        for (size_t i = 0; i < pmf.size(); ++i) {
            lowerTail += pmf.probabilities[i];
            tails.lowerTailProbabilities[i] = lowerTail;
        }

        double upperTail = 0.0;
        for (size_t i = pmf.size(); i > 0; --i) {
            upperTail += pmf.probabilities[i - 1];
            tails.upperTailProbabilities[i - 1] = upperTail;
        }
        return tails;
    }

    double getProbabilityFromTailProbabilities(
        Parameters::ScanRateType scanrate,
        const HypergeometricProbabilityLookup::OffsetTailProbabilities& tails,
        count_t x
    ) {
        if (tails.empty()) return HypergeometricProbabilityLookup::PROBABILITY_UNSET;

        bool useHighTail = false;
        bool useLowTail = false;
        switch (scanrate) {
            case Parameters::LOWRATE:
                useLowTail = true;
                break;
            case Parameters::HIGHORLOWRATE:
                useHighTail = static_cast<double>(x) > tails.expected;
                useLowTail = static_cast<double>(x) < tails.expected;
                break;
            case Parameters::HIGHRATE:
            default:
                useHighTail = true;
                break;
        }

        double tail = 0.0;
        if (useHighTail && x <= tails.maxX()) {
            size_t start = x <= tails.offset ? 0 : static_cast<size_t>(x - tails.offset);
            tail = tails.upperTailProbabilities[start];
        } else if (useLowTail && x >= tails.offset) {
            size_t end = std::min(static_cast<size_t>(x - tails.offset), tails.size() - 1);
            tail = tails.lowerTailProbabilities[end];
        }
        return tail > 0.0 ? -tail : HypergeometricProbabilityLookup::PROBABILITY_UNSET;
    }
}

/** Hashes the four values that uniquely identify an on-demand probability result.
    The unordered_map still uses ProbabilityKey::operator== to confirm exact matches;
    this function only needs to spread keys well enough that repeated requests for
    common (scanrate, T, S, x) combinations do not pile into the same buckets. The
    combine expression is the usual boost-style hash_combine step. */
size_t HypergeometricProbabilityLookup::ProbabilityKeyHash::operator()(const ProbabilityKey& key) const {
    size_t seed = 0;
    auto combine = [&seed](size_t value) {
        seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    };
    combine(std::hash<int>()(static_cast<int>(key.scanrate)));
    combine(std::hash<count_t>()(key.T));
    combine(std::hash<count_t>()(key.S));
    combine(std::hash<count_t>()(key.x));
    return seed;
}

/** Hashes the two margins that determine the possible x values for cache keys.
    Tracking these pairs lets diagnostics estimate the maximum number of scalar
    probability keys that could be requested for the margins touched by a scan. */
size_t HypergeometricProbabilityLookup::ProbabilityMarginKeyHash::operator()(const ProbabilityMarginKey& key) const {
    size_t seed = 0;
    auto combine = [&seed](size_t value) {
        seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    };
    combine(std::hash<count_t>()(key.T));
    combine(std::hash<count_t>()(key.S));
    return seed;
}

/** Hashes the final day-of-week adjusted probability key. The key is deliberately
    wide because the stratified tail depends on all seven C_d, S_d, and T_d margins,
    not just scalar C, S, and T values. */
size_t HypergeometricProbabilityLookup::StratifiedProbabilityKeyHash::operator()(const StratifiedProbabilityKey& key) const {
    size_t seed = 0;
    auto combine = [&seed](size_t value) {
        seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    };
    auto combineCounts = [&combine](const CountByDay_t& counts) {
        for (count_t count : counts) combine(std::hash<count_t>()(count));
    };
    combine(std::hash<int>()(static_cast<int>(key.scanrate)));
    combineCounts(key.totalCasesByDay);
    combineCounts(key.spatialCasesByDay);
    combineCounts(key.windowCasesByDay);
    combine(std::hash<count_t>()(key.x));
    return seed;
}

/** Hashes a day-of-week adjusted margin set without x. If this key repeats while
    the full tail key misses, then the same convolved distribution is being reused
    with different observed cluster counts. */
size_t HypergeometricProbabilityLookup::StratifiedMarginSetKeyHash::operator()(const StratifiedMarginSetKey& key) const {
    size_t seed = 0;
    auto combine = [&seed](size_t value) {
        seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    };
    auto combineCounts = [&combine](const CountByDay_t& counts) {
        for (count_t count : counts) combine(std::hash<count_t>()(count));
    };
    combine(std::hash<int>()(static_cast<int>(key.scanrate)));
    combineCounts(key.totalCasesByDay);
    combineCounts(key.spatialCasesByDay);
    combineCounts(key.windowCasesByDay);
    return seed;
}

/** Hashes one weekday hypergeometric margin triple. These keys are diagnostics for
    reusing the same per-day PMF across final-tail cache misses. */
size_t HypergeometricProbabilityLookup::DayPmfMarginKeyHash::operator()(const DayPmfMarginKey& key) const {
    size_t seed = 0;
    auto combine = [&seed](size_t value) {
        seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    };
    combine(std::hash<count_t>()(key.C));
    combine(std::hash<count_t>()(key.S));
    combine(std::hash<count_t>()(key.T));
    return seed;
}

/** Estimates how many distinct x values could be cached for one (T,S) margin pair.
    This is not the number already observed; it is the scan-rate-specific hypergeometric
    support that could be requested for this pair if the scan encounters every possible x. */
size_t HypergeometricProbabilityLookup::estimateCacheEntriesForMargin(Parameters::ScanRateType scanrate, count_t T, count_t S) const {
    if (!_total_cases || S > _total_cases || T > _total_cases) return 0;

    count_t min = std::max(static_cast<count_t>(0), S + T - _total_cases);
    count_t max = std::min(S, T);
    if (min > max) return 0;

    count_t mean = static_cast<count_t>(ceil(static_cast<double>(S) * static_cast<double>(T) / static_cast<double>(_total_cases)));
    switch (scanrate) {
        case Parameters::LOWRATE:
            if (mean <= min) return 0;
            return static_cast<size_t>(std::min(max, static_cast<count_t>(mean - 1)) - min + 1);
        case Parameters::HIGHORLOWRATE:
            return static_cast<size_t>(max - min + 1);
        case Parameters::HIGHRATE:
        default: {
            count_t highStart = std::max(min, mean);
            if (highStart > max) return 0;
            return static_cast<size_t>(max - highStart + 1);
        }
    }
}

/** Reports on-demand probability cache usage for diagnostics.
    The entry count is exact. The byte count is an estimate because std::unordered_map
    node and allocator overhead are implementation-dependent; it includes the map
    object, bucket array, stored key/value payload, and a small per-node pointer estimate. */
HypergeometricProbabilityLookup::ProbabilityCacheStatistics HypergeometricProbabilityLookup::getProbabilityCacheStatistics() const {
    boost::mutex::scoped_lock lock(_probability_cache_mutex);

    ProbabilityCacheStatistics statistics;
    statistics.C = _total_cases;
    statistics.T = _case_window_count;

    const bool hasStratifiedActivity = _stratified_probability_cache.size() ||
        _stratified_probability_cache_hits || _stratified_probability_cache_misses ||
        _stratified_probability_cache_invalid_requests || _stratified_probability_cache_tail_evaluations;
    const bool hasScalarActivity = _probability_cache.size() ||
        _probability_cache_hits || _probability_cache_misses ||
        _probability_cache_invalid_requests || _probability_cache_tail_evaluations;

    if (hasStratifiedActivity && !hasScalarActivity) {
        statistics.cacheType = ProbabilityCacheStatistics::STRATIFIED_DAY_OF_WEEK;
        statistics.entries = _stratified_probability_cache.size();
        statistics.estimatedMemoryBytes = sizeof(_stratified_probability_cache) +
            _stratified_probability_cache.bucket_count() * sizeof(void*) +
            _stratified_probability_cache.size() * (sizeof(StratifiedProbabilityCache_t::value_type) + 2U * sizeof(void*)) +
            sizeof(_stratified_margin_set_cache) +
            _stratified_margin_set_cache.bucket_count() * sizeof(void*) +
            _stratified_margin_set_cache.size() * (sizeof(StratifiedMarginSetCache_t::value_type) + 2U * sizeof(void*)) +
            sizeof(_stratified_day_pmf_cache) +
            _stratified_day_pmf_cache.bucket_count() * sizeof(void*);
        for (const DayPmfCache_t::value_type& cachedPmf : _stratified_day_pmf_cache) {
            statistics.estimatedMemoryBytes += sizeof(DayPmfCache_t::value_type) + 2U * sizeof(void*) +
                cachedPmf.second.probabilities.capacity() * sizeof(double);
        }
        statistics.cacheHits = _stratified_probability_cache_hits;
        statistics.cacheMisses = _stratified_probability_cache_misses;
        statistics.invalidRequests = _stratified_probability_cache_invalid_requests;
        statistics.requests = statistics.cacheHits + statistics.cacheMisses;
        statistics.tailEvaluations = _stratified_probability_cache_tail_evaluations;
        statistics.dayPmfRequests = _stratified_day_pmf_requests;
        statistics.dayPmfCacheEntries = _stratified_day_pmf_cache.size();
        statistics.dayPmfCacheHits = _stratified_day_pmf_cache_hits;
        statistics.dayPmfCacheMisses = _stratified_day_pmf_cache_misses;
        statistics.uniqueDayPmfMargins = _stratified_day_pmf_cache.size();
        statistics.dayPmfReuseOpportunities = _stratified_day_pmf_reuse_opportunities;
        statistics.uniqueStratifiedMarginSets = _stratified_margin_set_cache.size();
        statistics.stratifiedMarginSetReuseOpportunities = _stratified_margin_set_reuse_opportunities;
        statistics.totalDayPmfTerms = _stratified_day_pmf_total_terms;
        statistics.maxDayPmfTerms = _stratified_day_pmf_max_terms;
        statistics.convolutionCount = _stratified_convolution_count;
        statistics.totalConvolutionInputTerms = _stratified_convolution_input_terms;
        statistics.totalCombinedPmfSize = _stratified_combined_pmf_total_size;
        statistics.maxCombinedPmfSize = _stratified_combined_pmf_max_size;
        return statistics;
    }

    if (hasScalarActivity) statistics.cacheType = ProbabilityCacheStatistics::SCALAR_ON_DEMAND;
    statistics.entries = _probability_cache.size();
    statistics.estimatedMemoryBytes = sizeof(_probability_cache) +
        _probability_cache.bucket_count() * sizeof(void*) +
        _probability_cache.size() * (sizeof(ProbabilityCache_t::value_type) + 2U * sizeof(void*));
    //statistics.evaluatedMarginPairs = _probability_margin_pairs.size();
    //statistics.estimatedMaxCacheEntries = _estimated_max_cache_entries;
    statistics.cacheHits = _probability_cache_hits;
    statistics.cacheMisses = _probability_cache_misses;
    statistics.invalidRequests = _probability_cache_invalid_requests;
    statistics.requests = statistics.cacheHits + statistics.cacheMisses + statistics.invalidRequests;
    statistics.tailEvaluations = _probability_cache_tail_evaluations;
    statistics.totalTailTerms = _probability_cache_total_tail_terms;
    statistics.maxTailTerms = _probability_cache_max_tail_terms;
    return statistics;
}

/** Calculates the negative tail probability for one observed scalar cluster.
    This path is used when C is too large for the dense lookup table. It keeps the
    calculation in log-space until the final return value, then caches the result.

    High-rate tail: P(X >= x)
    Low-rate tail:  P(X <= x)
    High-or-low chooses the side of the expected value S*T/C. */
double HypergeometricProbabilityLookup::getOnDemandProbabilityFor(Parameters::ScanRateType scanrate, count_t T, count_t S, count_t x) const {
    if (!_total_cases || S > _total_cases || T > _total_cases) {
        boost::mutex::scoped_lock lock(_probability_cache_mutex);
        ++_probability_cache_invalid_requests;
        return PROBABILITY_UNSET;
    }

    count_t min = std::max(static_cast<count_t>(0), S + T - _total_cases);
    count_t max = std::min(S, T);
    if (x < min || x > max) {
        boost::mutex::scoped_lock lock(_probability_cache_mutex);
        ++_probability_cache_invalid_requests;
        return PROBABILITY_UNSET;
    }

    ProbabilityKey key = { scanrate, T, S, x };
    //size_t estimatedEntriesForMargin = estimateCacheEntriesForMargin(scanrate, T, S);
    {   // Check existence in a critical section.
        boost::mutex::scoped_lock lock(_probability_cache_mutex);
        //if (_probability_margin_pairs.emplace(ProbabilityMarginKey{ T, S }).second)
        //    _estimated_max_cache_entries += estimatedEntriesForMargin;
        auto found = _probability_cache.find(key);
        if (found != _probability_cache.end()) {
            ++_probability_cache_hits;
            return found->second;
        }
        ++_probability_cache_misses;
    }

    count_t mean = static_cast<count_t>(ceil(static_cast<double>(S) * static_cast<double>(T) / static_cast<double>(_total_cases)));
    bool useHighTail = false;
    bool useLowTail = false;
    switch (scanrate) {
        case Parameters::LOWRATE:
            useLowTail = true;
            break;
        case Parameters::HIGHORLOWRATE:
            useHighTail = x >= mean;
            useLowTail = x < mean;
            break;
        case Parameters::HIGHRATE:
        default:
            useHighTail = true;
            break;
    }

    double logTail = -std::numeric_limits<double>::infinity();
    size_t tailTerms = 0;
    if (useHighTail) {
        tailTerms = static_cast<size_t>(max - x + 1);
        logTail = logSumExpHypergeometricTail(_total_cases, S, T, x, max);
    } else if (useLowTail) {
        tailTerms = static_cast<size_t>(x - min + 1);
        logTail = logSumExpHypergeometricTail(_total_cases, S, T, min, x);
    } else {
        boost::mutex::scoped_lock lock(_probability_cache_mutex);
        ++_probability_cache_invalid_requests;
        return PROBABILITY_UNSET;
    }

    double probability = negativeProbabilityFromLogTail(logTail);
    {   // Add to cache in a critical section and record the miss-side work.
        boost::mutex::scoped_lock lock(_probability_cache_mutex);
        _probability_cache.emplace(key, probability);
        ++_probability_cache_tail_evaluations;
        _probability_cache_total_tail_terms += tailTerms;
        _probability_cache_max_tail_terms = std::max(_probability_cache_max_tail_terms, tailTerms);
    }
    return probability;
}

/** Initializes the hypergeometric lookup machinery for a scan.
    Small C: precompute dense negative tails for every evaluated S and T.
    Large C: skip the dense table and memoize individual tail probabilities.

    The dense table still computes cumulative tails in log-space. We only convert
    to negative probability when storing the finished tail for a particular x. */
void HypergeometricProbabilityLookup::calculateHG(Parameters::ScanRateType scanrate, const std::set<count_t>& caseWindows, count_t C, int threshold) {
    clear();
    _total_cases = C;
    _case_window_count = caseWindows.size();
    _scanrate = scanrate;
    _use_lookup_table = C <= threshold;
    if (!_use_lookup_table) {
        std::cout << "Using memoized on-demand Hypergeometric probabilities for C=" << C << ", T=" << caseWindows.size() << std::endl;
        return;
    }
    std::cout << "Calculating Hypergeometric probabilities for C=" << C << ", T=" << caseWindows.size() << std::endl;
	// Precompute natural logarithm values for 0..C to avoid repeated calls to log() function.
    std::vector<double> caseLog(C + 1, 0.0);
    for (size_t t = 1; t < caseLog.size(); ++t) caseLog[t] = log(t);

    auto calc_hg_0 = [&caseLog](count_t C, count_t T, count_t S) { // probability of 0 cases in cylinder, based on hypergeometric distribution
        double hg = STD_LOG(C - S) - STD_LOG(C);
        for (count_t i = 1; i <= T - 1; ++i)
            hg += STD_LOG(C - S - i) - STD_LOG(C - i);
        return hg;
    };
    auto calc_hg_t = [&caseLog](count_t C, count_t T, count_t S) { // probability of x cases in cylinder, based on hypergeometric distribution
        double hg = STD_LOG(T) - STD_LOG(C);
        for (count_t i = 1; i <= C - S - 1; ++i) {
            hg += STD_LOG(T - i) - STD_LOG(C - i);
            if (hg == 0) break;
        }
        return hg;
    };
    if (caseWindows.empty()) return; // What do we do here, which might happen with multiple data sets?
    // Find the largest case window value - so we know how large to allocate translation array.
    _T_index.resize(*caseWindows.rbegin() + 1);
    _spatial_cases.resize(C + 1); // create SpatialCases vector for all C
    std::vector<double> negativeProbabilities(C + 1, PROBABILITY_UNSET); // temporary storage for negative probabilities
    size_t i = 0;
    for (auto T : caseWindows) {
        if (!T) continue; // skip T=0, which shouldn't be here anyway
        //std::cout << "Calculating HG for T=" << T << " (" << i + 1 << " of " << vT.size() << ")" << std::endl;
        for (count_t S = 2; S <= C - 2; ++S) {
            std::fill(negativeProbabilities.begin(), negativeProbabilities.end(), PROBABILITY_UNSET);
            count_t min = std::max((count_t)0, S + T - C);
            count_t max = std::min(S, T);
            count_t mean = static_cast<count_t>(ceil((double)S * (double)T / (double)C));
            // hgp[x] is log(P(X=x)); later loops accumulate log tail probabilities.
            std::vector<double> hgp(max + 1, 0);
            if (S + T - C <= 0)
                hgp.front() = calc_hg_0(C, T, S);
            else
                hgp[S + T - C] = calc_hg_t(C, T, S);
            for (count_t x = min + 1; x <= max; ++x)
                hgp[x] = hgp[x - 1] + (STD_LOG(S - x + 1) - STD_LOG(x)) + (STD_LOG(T - x + 1) - STD_LOG(C - S - T + x));
            // hgp[x] stores log(P(X=x)). logAddExp is the log-space version of
            // adding one PMF value to the running tail probability; this keeps
            // extremely small tail probabilities from underflowing to zero.
            if (scanrate == Parameters::HIGHRATE || scanrate == Parameters::HIGHORLOWRATE) {
                // For high rates nHG(x|S,T) = negative of the cumulative probability of x or more cases, x>=2.
                double logTail = hgp[max];
                negativeProbabilities[max] = negativeProbabilityFromLogTail(logTail);
                for (count_t x = max - 1; x >= mean; --x) {
                    logTail = logAddExp(logTail, hgp[x]);
                    negativeProbabilities[x] = negativeProbabilityFromLogTail(logTail);
                }
            }
            if (scanrate == Parameters::LOWRATE || scanrate == Parameters::HIGHORLOWRATE) {
                // For low rates nHG(x|S,T) = negative of the cumulative probability of x or less cases, x >= 0.
                double logTail = hgp[min];
                negativeProbabilities[min] = negativeProbabilityFromLogTail(logTail);
                for (count_t x = min + 1; x <= mean - 1; ++x) {
                    logTail = logAddExp(logTail, hgp[x]);
                    negativeProbabilities[x] = negativeProbabilityFromLogTail(logTail);
                }
            }
            _spatial_cases[S].addNegativeProbabilitiesFor(i, negativeProbabilities);
        }
        _T_index[T] = static_cast<unsigned int>(i); // map T to index in HG table
        ++i;
    }
}

/** Returns negative tail probability for the day-of-week stratified hypergeometric distribution.
    Each weekday contributes X_d ~ Hypergeometric(C_d, S_d, T_d), and this calculates
    the tail probability for X = sum_d X_d. The final tail is memoized by x, while
    the combined DOW tail arrays are memoized by margins without x. */
double HypergeometricProbabilityLookup::getStratifiedProbabilityFor(
    Parameters::ScanRateType scanrate, const CountByDay_t& totalCasesByDay, const CountByDay_t& spatialCasesByDay, const CountByDay_t& windowCasesByDay, count_t x
) const {
    StratifiedProbabilityKey key = { scanrate, totalCasesByDay, spatialCasesByDay, windowCasesByDay, x };
    //std::cout << "key=" << key.toString() << std::endl;
    StratifiedMarginSetKey marginSetKey = { scanrate, totalCasesByDay, spatialCasesByDay, windowCasesByDay };
    //std::cout << "marginSetKey=" << marginSetKey.toString() << std::endl;
    {
        boost::mutex::scoped_lock lock(_probability_cache_mutex);
        auto found = _stratified_probability_cache.find(key);
        if (found != _stratified_probability_cache.end()) {
            ++_stratified_probability_cache_hits;
            return found->second;
        }
        ++_stratified_probability_cache_misses;

        auto foundTails = _stratified_margin_set_cache.find(marginSetKey);
        if (foundTails != _stratified_margin_set_cache.end()) {
            ++_stratified_margin_set_reuse_opportunities;
            double probability = getProbabilityFromTailProbabilities(scanrate, foundTails->second, x);
            if (probability == PROBABILITY_UNSET) {
                ++_stratified_probability_cache_invalid_requests;
            } else {
                _stratified_probability_cache.emplace(key, probability);
                ++_stratified_probability_cache_tail_evaluations;
            }
            return probability;
        }
    }

    std::vector<OffsetPmf> dayPmfs;
    dayPmfs.reserve(totalCasesByDay.size());
    size_t totalDayPmfTerms = 0;
    size_t maxDayPmfTerms = 0;
    size_t convolutionCount = 0;
    size_t totalConvolutionInputTerms = 0;
    size_t totalCombinedPmfSize = 0;
    size_t maxCombinedPmfSize = 0;
    double expected = 0.0;
    double probability = PROBABILITY_UNSET;
    OffsetTailProbabilities tails;

    for (size_t d = 0; d < totalCasesByDay.size(); ++d) {
        count_t C = totalCasesByDay[d], S = spatialCasesByDay[d], T = windowCasesByDay[d];
        if (C) expected += static_cast<double>(S) * static_cast<double>(T) / static_cast<double>(C);

        DayPmfMarginKey dayPmfKey = { C, S, T };
        //std::cout << "dayPmfKey=" << dayPmfKey.toString() << std::endl;
        OffsetPmf dayPmf;
        bool foundDayPmf = false;
        {
            boost::mutex::scoped_lock lock(_probability_cache_mutex);
            ++_stratified_day_pmf_requests;
            auto found = _stratified_day_pmf_cache.find(dayPmfKey);
            if (found != _stratified_day_pmf_cache.end()) {
                ++_stratified_day_pmf_cache_hits;
                ++_stratified_day_pmf_reuse_opportunities;
                dayPmf = found->second;
                foundDayPmf = true;
            } else {
                ++_stratified_day_pmf_cache_misses;
            }
        }
        if (!foundDayPmf) {
            dayPmf = calculateHypergeometricPmf(C, S, T);
            boost::mutex::scoped_lock lock(_probability_cache_mutex);
            _stratified_day_pmf_cache.emplace(dayPmfKey, dayPmf);
        }

        totalDayPmfTerms += dayPmf.size();
        maxDayPmfTerms = std::max(maxDayPmfTerms, dayPmf.size());
        if (dayPmf.empty()) {
            dayPmfs.clear();
            break;
        }
        dayPmfs.push_back(dayPmf);
    }

    if (!dayPmfs.empty()) {
        std::sort(dayPmfs.begin(), dayPmfs.end(), [](const OffsetPmf& left, const OffsetPmf& right) {
            return left.size() < right.size();
        });

        OffsetPmf pmf = dayPmfs.front();
        for (size_t i = 1; i < dayPmfs.size(); ++i) {
            ++convolutionCount;
            totalConvolutionInputTerms += pmf.size() * dayPmfs[i].size();
            pmf = convolve(pmf, dayPmfs[i]);
            totalCombinedPmfSize += pmf.size();
            maxCombinedPmfSize = std::max(maxCombinedPmfSize, pmf.size());
            if (pmf.empty()) break;
        }

        tails = buildTailProbabilities(pmf, expected);
        probability = getProbabilityFromTailProbabilities(scanrate, tails, x);
    }

    {
        boost::mutex::scoped_lock lock(_probability_cache_mutex);
        _stratified_day_pmf_total_terms += totalDayPmfTerms;
        _stratified_day_pmf_max_terms = std::max(_stratified_day_pmf_max_terms, maxDayPmfTerms);
        _stratified_convolution_count += convolutionCount;
        _stratified_convolution_input_terms += totalConvolutionInputTerms;
        _stratified_combined_pmf_total_size += totalCombinedPmfSize;
        _stratified_combined_pmf_max_size = std::max(_stratified_combined_pmf_max_size, maxCombinedPmfSize);
        if (!tails.empty())
            _stratified_margin_set_cache.emplace(marginSetKey, tails);
        if (probability == PROBABILITY_UNSET) {
            ++_stratified_probability_cache_invalid_requests;
        } else {
            _stratified_probability_cache.emplace(key, probability);
            ++_stratified_probability_cache_tail_evaluations;
        }
    }
    return probability;
}

/** Adds negative probabilities for a given index T, reducing the storage by removing PROBABILITY_UNSET values. */
void HypergeometricProbabilityLookup::SpatialCases::addNegativeProbabilitiesFor(size_t idx_T, const std::vector<double>& np) {
    // We're expecting idx_T to be added in order, so we can just check the size.
    if (_probabilities_of_x_at_T.size() != idx_T)
        throw prg_error("SpatialCases::addFor: idx_T out of order.", "HypergeometricProbabilityLookup::SpatialCases");
    // Figure out what will be the zero-based index for x.
    size_t zero_base = std::distance(np.begin(), std::find_if_not(np.begin(), np.end(), [](double d) {return d == PROBABILITY_UNSET; }));
    // Figure out how big the array will be after removing PROBABILITY_UNSET values.
    auto setnp_front = np.begin();
    for (; setnp_front != np.end(); ) {
        if (*setnp_front != PROBABILITY_UNSET) break;
        ++setnp_front;
    }
    auto in_front = std::distance(np.begin(), setnp_front);
    auto setnp_back = np.rbegin();
    for (; setnp_back != np.rend(); ) {
        if (*setnp_back != PROBABILITY_UNSET) break;
        ++setnp_back;
    }
    auto in_back = std::distance(setnp_back.base(), np.rbegin().base());
    // Emplace the data to the back of vector - which is at index idx_T.
    _probabilities_of_x_at_T.emplace_back(zero_base, MinimalGrowthArray<double>(np.size() - in_front - in_back, PROBABILITY_UNSET));
    auto& data = _probabilities_of_x_at_T.back().second;
    int i = 0;
    for (std::vector<double>::const_iterator it = setnp_front; it != (setnp_front + data.size()); ++it, ++i)
        data[i] = *it;
}

/** Prints HG lookup table to file. */
void HypergeometricProbabilityLookup::printHG(const std::set<count_t>& caseWindows) const {
    std::ofstream outfile("HG_lookup.txt");
    for (auto T : caseWindows) {
        if (!T) continue;
        outfile << std::endl;
        for (unsigned int S = 2; S < _spatial_cases.size(); ++S) {
            for (unsigned int x = 0; x < _spatial_cases.size(); ++x) {
                double probability = getProbabilityFor_Checked(T, S, x);
                outfile << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10);
                if (probability == PROBABILITY_UNSET) {
                    outfile << "T = " << T << ", S = " << S << ", x = " << x << ", HG = " << PROBABILITY_UNSET << " (unset)" << std::endl;
                } else {
                    outfile << "T = " << T << ", S = " << S << ", x = " << x << ", HG = " << probability << " : ";
                    outfile << std::scientific << probability << std::endl;
                }
            }
            outfile << std::endl;
        }
        outfile << std::endl << std::endl;
    }
}
