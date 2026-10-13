//***************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//***************************************************************************

#include "HypergeometricProbabilityLookup.h"
#include <fstream>
#include <numeric>
#include <utility>

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

    // Does this scan rate ever read the lower / upper cumulative direction? These must stay
    // in step with the tail selection in getProbabilityFromTailProbabilities below, including
    // its default case, or a scan will look for a direction that was never stored.
    bool scanRateReadsLowerTail(Parameters::ScanRateType scanrate) {
        return scanrate == Parameters::LOWRATE || scanrate == Parameters::HIGHORLOWRATE;
    }
    bool scanRateReadsUpperTail(Parameters::ScanRateType scanrate) {
        return scanrate != Parameters::LOWRATE; // HIGHRATE, HIGHORLOWRATE, and the default
    }

    // Stores the cumulative direction(s) this scan rate reads for a combined DOW
    // distribution. That lets later requests with the same margins but a different x answer
    // the requested tail by one array lookup instead of reconvolving seven PMFs. Only one
    // direction is built for a single-sided scan rate, which halves what is by a wide margin
    // the largest cache here.
    HypergeometricProbabilityLookup::OffsetTailProbabilities buildTailProbabilities(
        Parameters::ScanRateType scanrate, const HypergeometricProbabilityLookup::OffsetPmf& pmf, double expected
    ) {
        if (pmf.empty()) return HypergeometricProbabilityLookup::OffsetTailProbabilities();

        HypergeometricProbabilityLookup::OffsetTailProbabilities tails;
        tails.offset = pmf.offset;
        tails.expected = expected;

        if (scanRateReadsLowerTail(scanrate)) {
            tails.lowerTailProbabilities.resize(pmf.size(), 0.0);
            double lowerTail = 0.0;
            for (size_t i = 0; i < pmf.size(); ++i) {
                lowerTail += pmf.probabilities[i];
                tails.lowerTailProbabilities[i] = lowerTail;
            }
        }
        if (scanRateReadsUpperTail(scanrate)) {
            tails.upperTailProbabilities.resize(pmf.size(), 0.0);
            double upperTail = 0.0;
            for (size_t i = pmf.size(); i > 0; --i) {
                upperTail += pmf.probabilities[i - 1];
                tails.upperTailProbabilities[i - 1] = upperTail;
            }
        }
        return tails;
    }

    // Total probability mass of the convolved distribution as the stored tails see it. Each
    // direction accumulates the same total from its own extreme, so either end serves.
    // Returns a negative value when there is nothing to measure.
    double observedCombinedPmfMass(const HypergeometricProbabilityLookup::OffsetTailProbabilities& tails) {
        if (!tails.lowerTailProbabilities.empty()) return tails.lowerTailProbabilities.back();
        if (!tails.upperTailProbabilities.empty()) return tails.upperTailProbabilities.front();
        return -1.0;
    }

    // The convolved day-of-week PMF is a probability distribution, so it must sum to one.
    // calculateHypergeometricPmf leaves log space before the convolution runs, so any
    // combined bin below ~1e-308 collapses to exactly zero and that mass disappears from the
    // total. Measuring the shortfall says whether the loss is theoretical or active at real
    // case counts. When the scan rate stores both directions this also cross-checks them
    // against each other, since they sum the same distribution in opposite orders. Returns a
    // negative value when there is nothing to measure.
    //
    // Note this is a weak test for the far-tail underflow that discards extreme clusters:
    // the bins at risk hold a negligible share of the mass by definition, so a clean result
    // here does not by itself mean no cluster was lost. The underflow request counter is
    // what answers that.
    double massConservationDeviation(const HypergeometricProbabilityLookup::OffsetTailProbabilities& tails) {
        double deviation = -1.0;
        if (!tails.lowerTailProbabilities.empty())
            deviation = std::max(deviation, std::fabs(1.0 - tails.lowerTailProbabilities.back()));
        if (!tails.upperTailProbabilities.empty())
            deviation = std::max(deviation, std::fabs(1.0 - tails.upperTailProbabilities.front()));
        return deviation;
    }

    /** How a tail request resolved. UNDERFLOWED still returns a usable probability - clamped
        to the smallest representable value - so it is not an invalid request; it is recorded
        because it marks a cluster too extreme to rank against its peers. OUT_OF_SUPPORT and
        NO_DISTRIBUTION return PROBABILITY_UNSET: the first is correct and expected, the
        second means the margins yielded no distribution to evaluate. */
    // UNDERFLOWED, not UNDERFLOW: <math.h> defines UNDERFLOW as a matherr constant.
    enum class TailOutcome { VALID, NO_DISTRIBUTION, OUT_OF_SUPPORT, UNDERFLOWED };

    double getProbabilityFromTailProbabilities(
        Parameters::ScanRateType scanrate,
        const HypergeometricProbabilityLookup::OffsetTailProbabilities& tails,
        count_t x,
        TailOutcome& outcome
    ) {
        if (tails.empty()) {
            outcome = TailOutcome::NO_DISTRIBUTION;
            return HypergeometricProbabilityLookup::PROBABILITY_UNSET;
        }

        // One flag, not a pair: the two tails are complementary, so a pair invites a state
        // where neither is set and x selects no tail at all. That is what happened for
        // HIGHORLOWRATE when x landed exactly on the expectation, which is reachable whenever
        // the expectation is integral - the cluster then fell through and was discarded.
        //
        // Ties go to the high tail. For integer x and real e, x >= ceil(e) and x >= e are the
        // same predicate, so comparing against the raw expectation reproduces the
        // x >= ceil(S*T/C) convention that the scalar and dense-table paths already use,
        // without needing the ceil. Note the expectation itself legitimately differs between
        // paths - pooled S*T/C is not sum_d S_d*T_d/C_d - only the convention is shared.
        bool useHighTail = true;
        switch (scanrate) {
            case Parameters::LOWRATE:
                useHighTail = false;
                break;
            case Parameters::HIGHORLOWRATE:
                useHighTail = static_cast<double>(x) >= tails.expected;
                break;
            case Parameters::HIGHRATE:
            default:
                break;
        }

        // A cached entry is keyed on scanrate, so the direction this scan rate reads is
        // always the direction that was built. Check rather than trust it: indexing a
        // never-populated tail would silently return zero, which reads as a lost cluster.
        if (useHighTail ? tails.upperTailProbabilities.empty() : tails.lowerTailProbabilities.empty())
            throw prg_error("Tail probabilities missing for scan rate %d.", "getProbabilityFromTailProbabilities", static_cast<int>(scanrate));

        double tail = 0.0;
        bool inSupport = false;
        if (useHighTail) {
            if (x <= tails.maxX()) {
                size_t start = x <= tails.offset ? 0 : static_cast<size_t>(x - tails.offset);
                tail = tails.upperTailProbabilities[start];
                inSupport = true;
            }
        } else if (x >= tails.offset) {
            size_t end = std::min(static_cast<size_t>(x - tails.offset), tails.size() - 1);
            tail = tails.lowerTailProbabilities[end];
            inSupport = true;
        }
        if (tail > 0.0) {
            outcome = TailOutcome::VALID;
            return -tail;
        }
        // Reaching a tail array entry and finding zero is underflow, not an impossible x:
        // the entry exists because x is inside the convolved support. Not reaching one at
        // all means x lies outside the tail being evaluated.
        if (inSupport) {
            // The true tail is smaller than a double can hold, which makes this the most
            // extreme cluster the scan has found - exactly the one that must not be thrown
            // away. Clamp to the smallest representable probability so it survives and ranks
            // at the ceiling, matching what the scalar path already does through
            // negativeProbabilityFromLogTail. Every cluster clamped this way ties with the
            // others, so they cannot be ordered among themselves; carrying an explicit log
            // scale through the convolution is what would separate them.
            outcome = TailOutcome::UNDERFLOWED;
            return -std::numeric_limits<double>::denorm_min();
        }
        outcome = TailOutcome::OUT_OF_SUPPORT;
        return HypergeometricProbabilityLookup::PROBABILITY_UNSET;
    }
}

/** Acquires the cache mutex, recording whether the thread had to wait for it.
    The uncontended path is a single try_lock, the same atomic a plain lock would do, and
    reads no clock at all. A thread that loses the race pays one extra failed compare-and-swap
    plus two clock reads, which slightly inflates the measurement in exactly the case being
    measured; that is acceptable for telling a contended mutex from an idle one.

    Two things to keep in mind when reading the numbers. try_lock is permitted to fail
    spuriously, so the contended count is an upper bound. And because a thread can be
    descheduled while holding the lock, the hold times include any time it spent off a
    processor - on a fully subscribed machine that, not the work inside, is what produces the
    occasional hold of a hundred microseconds or more. */
HypergeometricProbabilityLookup::CacheLock::CacheLock(const CacheShard& shard) : _shard(shard) {
    StratifiedCounters& counters = _shard.counters;
    if (!_shard.mutex.try_lock()) {
        const std::chrono::steady_clock::time_point waitFrom = std::chrono::steady_clock::now();
        _shard.mutex.lock();
        const unsigned long long waited = static_cast<unsigned long long>(
            std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - waitFrom).count());
        ++counters.lockContentions;
        counters.lockWaitNanoseconds += waited;
        counters.maxLockWaitNanoseconds = std::max(counters.maxLockWaitNanoseconds, waited);
    }
    // Safe to touch the counters unsynchronized only because this shard's mutex is now held.
    ++counters.lockAcquisitions;
    _sample_hold_time = (counters.lockAcquisitions % CACHE_LOCK_HOLD_SAMPLE_INTERVAL) == 0;
    if (_sample_hold_time) _held_from = std::chrono::steady_clock::now();
}

/** Releases the shard mutex, closing out a hold-time sample when this acquisition was one. */
HypergeometricProbabilityLookup::CacheLock::~CacheLock() {
    if (_sample_hold_time) {
        StratifiedCounters& counters = _shard.counters;
        const unsigned long long held = static_cast<unsigned long long>(
            std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - _held_from).count());
        ++counters.lockHoldSamples;
        counters.lockHoldNanoseconds += held;
        counters.maxLockHoldNanoseconds = std::max(counters.maxLockHoldNanoseconds, held);
    }
    _shard.mutex.unlock();
}

/** Sizes the shard array from the thread count the user asked for.

    Collision probability on any one shard falls roughly as utilization divided by the shard
    count, so eight shards per thread keeps it well under one percent at any thread count
    while the array stays small - each shard is a mutex, three empty maps and a block of
    counters, and the maps allocate their buckets lazily. A power of two so the index is a
    shift of a mixed hash rather than a division. The floor keeps single-threaded runs from
    degenerating to one lock; the ceiling bounds the fixed cost on very large machines. */
void HypergeometricProbabilityLookup::allocateShards(unsigned int threads) {
    size_t requested = static_cast<size_t>(std::max(1u, threads)) * 8U;
    size_t count = 64;
    while (count < requested && count < 1024) count <<= 1;

    _shard_count = count;
    unsigned int log2Count = 0;
    while ((static_cast<size_t>(1) << log2Count) < count) ++log2Count;
    // 64, not sizeof(size_t)*8: shardIndexFor mixes in a fixed 64-bit type so that the index
    // is correct whether size_t is 32 or 64 bits wide.
    _shard_shift = 64U - log2Count;
    _shards = std::unique_ptr<CacheShard[]>(new CacheShard[count]);
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

/** Reports on-demand probability cache usage for diagnostics.
    The entry count is exact. The byte count is an estimate because std::unordered_map
    node and allocator overhead are implementation-dependent; it includes the map
    object, bucket array, stored key/value payload, and a small per-node pointer estimate. */
HypergeometricProbabilityLookup::ProbabilityCacheStatistics HypergeometricProbabilityLookup::getProbabilityCacheStatistics() const {
    ProbabilityCacheStatistics statistics;
    statistics.C = _total_cases;
    statistics.T = _case_window_count;
    statistics.shards = _shard_count;

    // Each shard is locked only long enough to read its own state. The totals are therefore
    // a composite of moments rather than one instant, which is fine for diagnostics and is
    // the price of not stopping every thread to take a consistent snapshot.
    double minCombinedPmfMass = std::numeric_limits<double>::infinity();
    for (size_t index = 0; index < _shard_count; ++index) {
        const CacheShard& shard = _shards[index];
        // Deliberately not a CacheLock: reading the statistics would otherwise add one
        // acquisition per shard to the very counters being reported.
        boost::mutex::scoped_lock lock(shard.mutex);
        const StratifiedCounters& counters = shard.counters;

        statistics.entries += shard.tailCache.size();
        statistics.uniqueStratifiedMarginSets += shard.marginSetCache.size();
        statistics.dayPmfCacheEntries += shard.dayPmfCache.size();

        // Tracked per tier because they scale differently and the totals alone hide which
        // one is growing. Tail arrays and day PMFs are heap allocated, so sizeof(value_type)
        // accounts for the vector objects but none of the doubles they own; that payload
        // dominates the margin set cache and has to be walked to be counted.
        statistics.tailCacheBytes += sizeof(shard.tailCache) +
            shard.tailCache.bucket_count() * sizeof(void*) +
            shard.tailCache.size() * (sizeof(StratifiedProbabilityCache_t::value_type) + 2U * sizeof(void*));
        statistics.marginSetCacheBytes += sizeof(shard.marginSetCache) +
            shard.marginSetCache.bucket_count() * sizeof(void*);
        for (const StratifiedMarginSetCache_t::value_type& cachedTails : shard.marginSetCache) {
            const size_t tailTerms = cachedTails.second.lowerTailProbabilities.capacity() +
                                     cachedTails.second.upperTailProbabilities.capacity();
            statistics.cachedMarginSetTailTerms += tailTerms;
            statistics.marginSetCacheBytes += sizeof(StratifiedMarginSetCache_t::value_type) +
                2U * sizeof(void*) + tailTerms * sizeof(double);
        }
        statistics.dayPmfCacheBytes += sizeof(shard.dayPmfCache) +
            shard.dayPmfCache.bucket_count() * sizeof(void*);
        for (const DayPmfCache_t::value_type& cachedPmf : shard.dayPmfCache) {
            statistics.cachedDayPmfTerms += cachedPmf.second.probabilities.capacity();
            statistics.dayPmfCacheBytes += sizeof(DayPmfCache_t::value_type) + 2U * sizeof(void*) +
                cachedPmf.second.probabilities.capacity() * sizeof(double);
        }

        statistics.cacheHits += counters.probabilityCacheHits;
        statistics.cacheMisses += counters.probabilityCacheMisses;
        statistics.invalidRequests += counters.invalidRequests;
        statistics.outOfSupportRequests += counters.outOfSupportRequests;
        statistics.underflowRequests += counters.underflowRequests;
        statistics.noDistributionRequests += counters.noDistributionRequests;
        statistics.tailEvaluations += counters.tailEvaluations;
        statistics.stratifiedMarginSetReuseOpportunities += counters.marginSetReuseOpportunities;
        statistics.dayPmfRequests += counters.dayPmfRequests;
        statistics.dayPmfCacheHits += counters.dayPmfCacheHits;
        statistics.dayPmfCacheMisses += counters.dayPmfCacheMisses;
        statistics.dayPmfReuseOpportunities += counters.dayPmfReuseOpportunities;
        statistics.totalDayPmfTerms += counters.dayPmfTotalTerms;
        statistics.maxDayPmfTerms = std::max(statistics.maxDayPmfTerms, counters.dayPmfMaxTerms);
        statistics.convolutionCount += counters.convolutionCount;
        statistics.totalConvolutionInputTerms += counters.convolutionInputTerms;
        statistics.totalCombinedPmfSize += counters.combinedPmfTotalSize;
        statistics.maxCombinedPmfSize = std::max(statistics.maxCombinedPmfSize, counters.combinedPmfMaxSize);
        statistics.massConservationChecks += counters.massConservationChecks;
        if (counters.massConservationChecks)
            minCombinedPmfMass = std::min(minCombinedPmfMass, counters.minCombinedPmfMass);
        // The worst deviation and the margins that produced it have to travel together.
        if (counters.maxMassDeviation > statistics.maxMassDeviation) {
            statistics.maxMassDeviation = counters.maxMassDeviation;
            statistics.worstMassDeviationMargins = counters.worstMassDeviationMargins;
        }
        statistics.lockAcquisitions += counters.lockAcquisitions;
        statistics.lockContentions += counters.lockContentions;
        statistics.lockWaitNanoseconds += counters.lockWaitNanoseconds;
        statistics.maxLockWaitNanoseconds = std::max(statistics.maxLockWaitNanoseconds, counters.maxLockWaitNanoseconds);
        statistics.lockHoldSamples += counters.lockHoldSamples;
        statistics.lockHoldNanoseconds += counters.lockHoldNanoseconds;
        statistics.maxLockHoldNanoseconds = std::max(statistics.maxLockHoldNanoseconds, counters.maxLockHoldNanoseconds);
        statistics.maxShardAcquisitions = std::max(statistics.maxShardAcquisitions, counters.lockAcquisitions);
    }
    statistics.uniqueDayPmfMargins = statistics.dayPmfCacheEntries;
    statistics.estimatedMemoryBytes = statistics.tailCacheBytes + statistics.marginSetCacheBytes + statistics.dayPmfCacheBytes;
    // Leave the minimum mass at its default rather than reporting infinity when nothing
    // has been convolved yet.
    if (statistics.massConservationChecks) statistics.minCombinedPmfMass = minCombinedPmfMass;

    const bool hasStratifiedActivity = statistics.entries || statistics.cacheHits ||
        statistics.cacheMisses || statistics.invalidRequests || statistics.tailEvaluations;

    boost::mutex::scoped_lock scalarLock(_probability_cache_mutex);
    const bool hasScalarActivity = _probability_cache.size() ||
        _probability_cache_hits || _probability_cache_misses ||
        _probability_cache_invalid_requests || _probability_cache_tail_evaluations;

    if (hasStratifiedActivity && !hasScalarActivity) {
        statistics.cacheType = ProbabilityCacheStatistics::STRATIFIED_DAY_OF_WEEK;
        statistics.requests = statistics.cacheHits + statistics.cacheMisses;
        return statistics;
    }

    // The scalar path has its own unsharded mutex and its own counters; reset whatever the
    // shard walk accumulated so the scalar report is not a mixture of the two.
    statistics = ProbabilityCacheStatistics();
    statistics.C = _total_cases;
    statistics.T = _case_window_count;
    if (hasScalarActivity) statistics.cacheType = ProbabilityCacheStatistics::SCALAR_ON_DEMAND;
    statistics.entries = _probability_cache.size();
    statistics.estimatedMemoryBytes = sizeof(_probability_cache) +
        _probability_cache.bucket_count() * sizeof(void*) +
        _probability_cache.size() * (sizeof(ProbabilityCache_t::value_type) + 2U * sizeof(void*));
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
    {   // Check existence in a critical section.
        boost::mutex::scoped_lock lock(_probability_cache_mutex);
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
    Option 1: precompute dense negative tails for every evaluated S and T.
    :option 2 skip the dense table and memoize individual tail probabilities.

    The dense table still computes cumulative tails in log-space. We only convert
    to negative probability when storing the finished tail for a particular x. */
void HypergeometricProbabilityLookup::calculateHG(Parameters::ScanRateType scanrate, const std::set<count_t>& caseWindows, count_t C, bool use_lookup_table, unsigned int threads) {
    // Before clear(), which walks the shards that this allocates.++
    allocateShards(threads);
    clear();
    _total_cases = C;
    _case_window_count = caseWindows.size();
    _scanrate = scanrate;
    _use_lookup_table = use_lookup_table;
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
    // Tier 1 and tier 2 share a shard because both are keyed on these margins, so the whole
    // lookup below stays a single lock acquisition. The per-day PMF lookups further down take
    // a different shard's lock, which is safe only because that lock is never held at the same
    // time as this one. Nesting two shard locks would make deadlock possible; do not.
    CacheShard& marginShard = shardForMargins(marginSetKey);
    // Both paths that evaluate a tail classify the result the same way. Only the two
    // conditions that yield no probability count as invalid; a clamped underflow is a usable
    // answer and is tracked separately. Callers must already hold the shard's mutex.
    auto recordTailOutcome = [&marginShard](TailOutcome outcome) {
        StratifiedCounters& counters = marginShard.counters;
        switch (outcome) {
            case TailOutcome::VALID:
                break;
            case TailOutcome::UNDERFLOWED:
                ++counters.underflowRequests;
                break;
            case TailOutcome::OUT_OF_SUPPORT:
                ++counters.outOfSupportRequests;
                ++counters.invalidRequests;
                break;
            default:
                ++counters.noDistributionRequests;
                ++counters.invalidRequests;
                break;
        }
    };
    {
        CacheLock lock(marginShard);
        auto found = marginShard.tailCache.find(key);
        if (found != marginShard.tailCache.end()) {
            ++marginShard.counters.probabilityCacheHits;
            return found->second;
        }
        ++marginShard.counters.probabilityCacheMisses;

        auto foundTails = marginShard.marginSetCache.find(marginSetKey);
        if (foundTails != marginShard.marginSetCache.end()) {
            ++marginShard.counters.marginSetReuseOpportunities;
            TailOutcome outcome = TailOutcome::NO_DISTRIBUTION;
            double probability = getProbabilityFromTailProbabilities(scanrate, foundTails->second, x, outcome);
            recordTailOutcome(outcome);
            if (probability != PROBABILITY_UNSET) {
                marginShard.tailCache.emplace(key, probability);
                ++marginShard.counters.tailEvaluations;
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
    double massDeviation = -1.0; // negative until a convolved distribution exists to measure
    TailOutcome outcome = TailOutcome::NO_DISTRIBUTION; // stays this way if no day PMF survives
    OffsetTailProbabilities tails;

    for (size_t d = 0; d < totalCasesByDay.size(); ++d) {
        count_t C = totalCasesByDay[d], S = spatialCasesByDay[d], T = windowCasesByDay[d];
        if (C) expected += static_cast<double>(S) * static_cast<double>(T) / static_cast<double>(C);

        DayPmfMarginKey dayPmfKey = { C, S, T };
        //std::cout << "dayPmfKey=" << dayPmfKey.toString() << std::endl;
        // Hold a pointer to the cached PMF rather than copying it out. Inserting into an
        // unordered_map never invalidates references to existing elements, and nothing here
        // erases - clear() only runs between scans - so the pointer stays good after the
        // lock is released. Copying instead would put a multi-kilobyte memcpy plus an
        // allocation inside the critical section on every cache hit, and this cache is hit
        // over a million times per scan.
        CacheShard& dayShard = shardForDayMargins(dayPmfKey);
        const OffsetPmf* cachedDayPmf = nullptr;
        {
            CacheLock lock(dayShard);
            ++dayShard.counters.dayPmfRequests;
            auto found = dayShard.dayPmfCache.find(dayPmfKey);
            if (found != dayShard.dayPmfCache.end()) {
                ++dayShard.counters.dayPmfCacheHits;
                ++dayShard.counters.dayPmfReuseOpportunities;
                cachedDayPmf = &found->second;
            } else {
                ++dayShard.counters.dayPmfCacheMisses;
            }
        }
        OffsetPmf computedDayPmf;
        if (!cachedDayPmf) {
            computedDayPmf = calculateHypergeometricPmf(C, S, T);
            // Build the cache's own copy before taking the lock, then move it in, so neither
            // the allocation nor the copy lands in the critical section. Two threads that
            // miss on the same margins both compute; the second emplace is discarded and
            // each thread goes on using its own copy, which is equivalent.
            OffsetPmf pmfToCache = computedDayPmf;
            CacheLock lock(dayShard);
            dayShard.dayPmfCache.emplace(dayPmfKey, std::move(pmfToCache));
        }
        const OffsetPmf& dayPmf = cachedDayPmf ? *cachedDayPmf : computedDayPmf;

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

        tails = buildTailProbabilities(scanrate, pmf, expected);
        massDeviation = massConservationDeviation(tails);
        probability = getProbabilityFromTailProbabilities(scanrate, tails, x, outcome);
    }

    {
        CacheLock lock(marginShard);
        StratifiedCounters& counters = marginShard.counters;
        if (massDeviation >= 0.0) {
            // Checked once per convolution rather than once per request, since this branch
            // only runs when the margin set cache missed. That is one check per distinct
            // margin set, except where two threads miss on the same margins and duplicate
            // the convolution.
            ++counters.massConservationChecks;
            counters.minCombinedPmfMass = std::min(counters.minCombinedPmfMass, observedCombinedPmfMass(tails));
            if (massDeviation > counters.maxMassDeviation) {
                counters.maxMassDeviation = massDeviation;
                counters.worstMassDeviationMargins = marginSetKey.toString();
            }
        }
        counters.dayPmfTotalTerms += totalDayPmfTerms;
        counters.dayPmfMaxTerms = std::max(counters.dayPmfMaxTerms, maxDayPmfTerms);
        counters.convolutionCount += convolutionCount;
        counters.convolutionInputTerms += totalConvolutionInputTerms;
        counters.combinedPmfTotalSize += totalCombinedPmfSize;
        counters.combinedPmfMaxSize = std::max(counters.combinedPmfMaxSize, maxCombinedPmfSize);
        // Moved, not copied: tails is a local with no use after this point, and its arrays
        // run to tens of thousands of doubles on the largest margin sets. Copying one in
        // here is a several-hundred-kilobyte memcpy plus an allocation held against every
        // other thread, which is what produces the longest holds on this mutex.
        if (!tails.empty())
            marginShard.marginSetCache.emplace(marginSetKey, std::move(tails));
        recordTailOutcome(outcome);
        if (probability != PROBABILITY_UNSET) {
            marginShard.tailCache.emplace(key, probability);
            ++counters.tailEvaluations;
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
