#ifndef __HYPERGEOMETRICPROBABILITYLOOKUP_H
#define __HYPERGEOMETRICPROBABILITYLOOKUP_H

#include "MinimalGrowthArray.h"
#include "PrjException.h"
#include "Parameters.h"
#include <array>
#include <vector>
#include <set>
#include <algorithm>
#include <chrono>
#include <limits>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <boost/thread/mutex.hpp>

/** Provides negative hypergeometric tail probabilities for tree-time scans.
    The hypergeometric parameters used here are:
      C = total cases in the study population,
      S = cases in the spatial branch across all time,
      T = cases in the time window across all branches,
      x = observed cases in the branch-window cluster.

    TreeScan's hypergeometric scan stores probabilities as negative values so the
    existing likelihood code can maximize the strongest cluster by transforming
    -P(tail) into a positive test statistic later. */
class HypergeometricProbabilityLookup {
public:
    typedef std::array<count_t, 7> CountByDay_t;
    static constexpr double PROBABILITY_UNSET = -99.0;
    /** Above this case count, calculating every possible S in a dense table can
        consume too much memory. Larger populations use the memoized on-demand path. */
    static constexpr int DENSE_LOOKUP_CASE_THRESHOLD = 5000;
    /** A convolved day-of-week PMF must sum to one. What counts as an acceptable deviation
        is a judgment, not a trigger: the noise floor is set by lgamma precision in the
        per-day PMF entries and rises with C - about 2e-9 at C=1e6 and 8e-9 at C=3e6, since
        the absolute error of lgamma(C_d) scales with its magnitude - so any fixed threshold
        needs revisiting as C grows. This value sits several decades above the floor measured
        at C=3e6, and genuine mass loss would be orders of magnitude larger again. Used by
        tests and as the yardstick for reading maxMassDeviation in a scan report. */
    static constexpr double MASS_CONSERVATION_TOLERANCE = 1e-6;
    /** Sample one acquisition in this many when timing how long the cache mutex is held.
        A power of two so the check is a mask. Hold time varies little between acquisitions
        of the same kind, so a sparse sample estimates the mean well at negligible cost. */
    static constexpr size_t CACHE_LOCK_HOLD_SAMPLE_INTERVAL = 1024;

    struct ProbabilityCacheStatistics {
        enum CacheType { NONE, SCALAR_ON_DEMAND, STRATIFIED_DAY_OF_WEEK };

        CacheType cacheType = NONE;
        size_t C = 0;
        size_t T = 0;
        size_t entries = 0;
        size_t estimatedMemoryBytes = 0;
        /** Per-tier split of estimatedMemoryBytes for the stratified path, which they sum
            to. Kept separate because the tiers scale differently: the final tail cache grows
            with the number of distinct (margins, x) pairs, while the margin set cache grows
            with both the number of margin sets and the size of each convolved distribution. */
        size_t tailCacheBytes = 0;
        size_t marginSetCacheBytes = 0;
        size_t dayPmfCacheBytes = 0;
        size_t requests = 0;
        size_t cacheHits = 0;
        size_t cacheMisses = 0;
        size_t invalidRequests = 0;
        /** Tail outcomes for the stratified path. outOfSupportRequests and
            noDistributionRequests are the two conditions that yield PROBABILITY_UNSET and
            together they sum to invalidRequests. underflowRequests is NOT invalid: those
            requests return a probability clamped to the smallest representable value because
            the true tail was too small to hold. It is tracked because every cluster clamped
            that way ties with the others and cannot be ranked against them, so a non-zero
            value is what would justify carrying an explicit log scale through the
            convolution. */
        size_t outOfSupportRequests = 0;
        size_t underflowRequests = 0;
        size_t noDistributionRequests = 0;
        size_t tailEvaluations = 0;
        size_t totalTailTerms = 0;
        size_t maxTailTerms = 0;
        size_t dayPmfRequests = 0;
        size_t dayPmfCacheEntries = 0;
        size_t dayPmfCacheHits = 0;
        size_t dayPmfCacheMisses = 0;
        size_t uniqueDayPmfMargins = 0;
        size_t dayPmfReuseOpportunities = 0;
        size_t uniqueStratifiedMarginSets = 0;
        size_t stratifiedMarginSetReuseOpportunities = 0;
        size_t totalDayPmfTerms = 0;
        size_t maxDayPmfTerms = 0;
        size_t convolutionCount = 0;
        size_t totalConvolutionInputTerms = 0;
        size_t totalCombinedPmfSize = 0;
        size_t maxCombinedPmfSize = 0;
        /** Total doubles held by each cache. The totalDayPmfTerms and totalCombinedPmfSize
            figures above are weighted by request and by convolution stage, so they describe
            the distributions being evaluated rather than the ones being stored - cached day
            PMFs run several times larger than their request-weighted average, because common
            margins are small and rare margins are large. These are entry-weighted and are
            the ones to reason about memory with. */
        size_t cachedDayPmfTerms = 0;
        size_t cachedMarginSetTailTerms = 0;
        /** Mass conservation of the convolved day-of-week PMFs. One check is recorded per
            convolution, which is the miss path, so this counts distinct margin sets except
            where concurrent threads duplicate one. maxMassDeviation is the worst observed
            |1 - total mass| and minCombinedPmfMass the smallest observed total mass; read
            those directly rather than against a fixed threshold, since the floating-point
            noise floor rises with C. */
        size_t massConservationChecks = 0;
        double maxMassDeviation = 0.0;
        double minCombinedPmfMass = 0.0;
        std::string worstMassDeviationMargins;
        /** Contention on the single mutex guarding every cache tier. lockWaitNanoseconds is
            summed across threads, so it is thread-time lost to waiting and can exceed the
            wall clock. Hold time is sampled one acquisition in
            CACHE_LOCK_HOLD_SAMPLE_INTERVAL; its mean sets the ceiling on lookups per second
            that no amount of additional threads can pass, which is what says whether
            sharding this mutex would be worth the work. */
        size_t lockAcquisitions = 0;
        size_t lockContentions = 0;
        unsigned long long lockWaitNanoseconds = 0;
        unsigned long long maxLockWaitNanoseconds = 0;
        size_t lockHoldSamples = 0;
        unsigned long long lockHoldNanoseconds = 0;
        unsigned long long maxLockHoldNanoseconds = 0;
    };

    struct OffsetPmf {
        count_t offset = 0;
        std::vector<double> probabilities;

        bool empty() const { return probabilities.empty(); }
        size_t size() const { return probabilities.size(); }
        count_t maxX() const { return probabilities.empty() ? offset : offset + static_cast<count_t>(probabilities.size() - 1); }
    };

    /** Cumulative tail probabilities for a combined day-of-week distribution. Only the
        direction(s) the scan rate actually reads are populated: a high-rate scan never
        consults the lower tail, and storing it doubles the largest cache in the stratified
        path for nothing. Both vectors have the same length whenever they are populated. */
    struct OffsetTailProbabilities {
        count_t offset = 0;
        double expected = 0.0;
        std::vector<double> lowerTailProbabilities;
        std::vector<double> upperTailProbabilities;

        bool empty() const { return lowerTailProbabilities.empty() && upperTailProbabilities.empty(); }
        size_t size() const { return std::max(lowerTailProbabilities.size(), upperTailProbabilities.size()); }
        count_t maxX() const { return empty() ? offset : offset + static_cast<count_t>(size() - 1); }
    };
    class SpatialCases {
    public:
        typedef std::vector<std::pair<size_t, MinimalGrowthArray<double>>> SpatialCasesData_t;

    private:
        SpatialCasesData_t _probabilities_of_x_at_T;

    public:
        const SpatialCasesData_t& getProbabilitiesOfXAtT() const { return _probabilities_of_x_at_T; }
        void addNegativeProbabilitiesFor(size_t idx_T, const std::vector<double>& np);
        double getProbabilityAt(size_t idx_T, size_t x) const {
            const auto& pair = _probabilities_of_x_at_T[idx_T]; // get probabilities for T
            return pair.second.getArray()[x - pair.first]; // get probability at x, with consideration of zero base

        }
        double getProbabilityAtChecked(size_t idx_T, size_t x) const {
            if (!_probabilities_of_x_at_T.size()) {
                //std::cout << "_probabilities_of_x_at_T.size() == 0" << std::endl;
                return PROBABILITY_UNSET;
            }
            const auto& pair = _probabilities_of_x_at_T.at(idx_T);
            if (x < pair.first || x >= pair.first + pair.second.size()) {
                //std::cout << "out of range: x=" << x << ", pair.first=" << pair.first << ", pair.second.size()==" << pair.second.size() << std::endl;
                return PROBABILITY_UNSET;
            }
            return pair.second.at(x - pair.first);
        }
    };

protected:
    /** Cache key for the on-demand scalar path. The same margins can be queried
        repeatedly while scanning real and simulated clusters. */
    struct ProbabilityKey {
        Parameters::ScanRateType scanrate;
        count_t T;
        count_t S;
        count_t x;

        bool operator==(const ProbabilityKey& other) const {
            return scanrate == other.scanrate && T == other.T && S == other.S && x == other.x;
        }
    };
    struct ProbabilityKeyHash {
        size_t operator()(const ProbabilityKey& key) const;
    };
    typedef std::unordered_map<ProbabilityKey, double, ProbabilityKeyHash> ProbabilityCache_t;
    struct StratifiedProbabilityKey {
        Parameters::ScanRateType scanrate;
        CountByDay_t totalCasesByDay;
        CountByDay_t spatialCasesByDay;
        CountByDay_t windowCasesByDay;
        count_t x;

        bool operator==(const StratifiedProbabilityKey& other) const {
            return scanrate == other.scanrate &&
                   totalCasesByDay == other.totalCasesByDay &&
                   spatialCasesByDay == other.spatialCasesByDay &&
                   windowCasesByDay == other.windowCasesByDay &&
                   x == other.x;
        }
        std::string toString() const {
            std::stringstream buffer;
            buffer << "scanrate=" << scanrate;
            buffer << ";totalCasesByDay=["; for (int x : totalCasesByDay) buffer << x << ','; buffer << "]";
            buffer << ";spatialCasesByDay=["; for (int x : spatialCasesByDay) buffer << x << ','; buffer << "]";
            buffer << ";windowCasesByDay=["; for (int x : windowCasesByDay) buffer << x << ','; buffer << "]";
            buffer << ";x=" << x;
            return buffer.str();
        }
    };
    struct StratifiedProbabilityKeyHash {
        size_t operator()(const StratifiedProbabilityKey& key) const;
    };
    typedef std::unordered_map<StratifiedProbabilityKey, double, StratifiedProbabilityKeyHash> StratifiedProbabilityCache_t;
    struct StratifiedMarginSetKey {
        Parameters::ScanRateType scanrate;
        CountByDay_t totalCasesByDay;
        CountByDay_t spatialCasesByDay;
        CountByDay_t windowCasesByDay;

        bool operator==(const StratifiedMarginSetKey& other) const {
            return scanrate == other.scanrate &&
                   totalCasesByDay == other.totalCasesByDay &&
                   spatialCasesByDay == other.spatialCasesByDay &&
                   windowCasesByDay == other.windowCasesByDay;
        }
        std::string toString() const {
            std::stringstream buffer;
            buffer << "scanrate=" << scanrate;
            buffer << ";totalCasesByDay=["; for (int x : totalCasesByDay) buffer << x << ','; buffer << "]";
            buffer << ";spatialCasesByDay=["; for (int x : spatialCasesByDay) buffer << x << ','; buffer << "]";
            buffer << ";windowCasesByDay=["; for (int x : windowCasesByDay) buffer << x << ','; buffer << "]";
            return buffer.str();
        }
    };
    struct StratifiedMarginSetKeyHash {
        size_t operator()(const StratifiedMarginSetKey& key) const;
    };
    typedef std::unordered_map<StratifiedMarginSetKey, OffsetTailProbabilities, StratifiedMarginSetKeyHash> StratifiedMarginSetCache_t;
    struct DayPmfMarginKey {
        count_t C;
        count_t S;
        count_t T;

        bool operator==(const DayPmfMarginKey& other) const {
            return C == other.C && S == other.S && T == other.T;
        }
        std::string toString() const {
            std::stringstream buffer;
            buffer << "C=" << C << ", S=" << S << ", T=" << T;
            return buffer.str();
        }
    };
    struct DayPmfMarginKeyHash {
        size_t operator()(const DayPmfMarginKey& key) const;
    };
    typedef std::unordered_map<DayPmfMarginKey, OffsetPmf, DayPmfMarginKeyHash> DayPmfCache_t;

    /** Scoped lock over _probability_cache_mutex that measures its own contention. One
        mutex guards every cache tier here, so it serializes all probability lookups and is
        the first suspect for limiting throughput as threads are added - but that was never
        measured, only assumed.

        try_lock is attempted first so an uncontended acquire costs the same single atomic
        as a plain lock, and the clock reads are paid only by a thread that actually has to
        wait. Hold time is sampled rather than always measured, because timing every
        acquisition would cost two clock reads on a path taken hundreds of millions of times
        per scan. Mean hold time is the number that matters most: it sets the ceiling on
        lookups per second no matter how many threads are running. */
    class CacheLock {
        const HypergeometricProbabilityLookup& _owner;
        std::chrono::steady_clock::time_point _held_from;
        bool _sample_hold_time = false;

    public:
        explicit CacheLock(const HypergeometricProbabilityLookup& owner);
        ~CacheLock();
        CacheLock(const CacheLock&) = delete;
        CacheLock& operator=(const CacheLock&) = delete;
    };

    std::vector<SpatialCases> _spatial_cases; // spatial cases dimension
    mutable std::vector<unsigned int> _T_index; // index mapping for T values
    count_t _total_cases = 0;
    size_t _case_window_count = 0;
    Parameters::ScanRateType _scanrate = Parameters::HIGHRATE;
    bool _use_lookup_table = true;
    mutable boost::mutex _probability_cache_mutex;
    mutable ProbabilityCache_t _probability_cache;
    mutable size_t _probability_cache_hits = 0;
    mutable size_t _probability_cache_misses = 0;
    mutable size_t _probability_cache_invalid_requests = 0;
    mutable size_t _probability_cache_tail_evaluations = 0;
    mutable size_t _probability_cache_total_tail_terms = 0;
    mutable size_t _probability_cache_max_tail_terms = 0;
    mutable StratifiedProbabilityCache_t _stratified_probability_cache;
    mutable StratifiedMarginSetCache_t _stratified_margin_set_cache;
    mutable size_t _stratified_probability_cache_hits = 0;
    mutable size_t _stratified_probability_cache_misses = 0;
    mutable size_t _stratified_probability_cache_invalid_requests = 0;
    mutable size_t _stratified_probability_cache_out_of_support_requests = 0;
    mutable size_t _stratified_probability_cache_underflow_requests = 0;
    mutable size_t _stratified_probability_cache_no_distribution_requests = 0;
    mutable size_t _stratified_probability_cache_tail_evaluations = 0;
    mutable size_t _stratified_margin_set_reuse_opportunities = 0;
    mutable DayPmfCache_t _stratified_day_pmf_cache;
    mutable size_t _stratified_day_pmf_requests = 0;
    mutable size_t _stratified_day_pmf_cache_hits = 0;
    mutable size_t _stratified_day_pmf_cache_misses = 0;
    mutable size_t _stratified_day_pmf_reuse_opportunities = 0;
    mutable size_t _stratified_day_pmf_total_terms = 0;
    mutable size_t _stratified_day_pmf_max_terms = 0;
    mutable size_t _stratified_convolution_count = 0;
    mutable size_t _stratified_convolution_input_terms = 0;
    mutable size_t _stratified_combined_pmf_total_size = 0;
    mutable size_t _stratified_combined_pmf_max_size = 0;
    mutable size_t _stratified_mass_conservation_checks = 0;
    mutable double _stratified_max_mass_deviation = 0.0;
    mutable double _stratified_min_combined_pmf_mass = std::numeric_limits<double>::infinity();
    mutable std::string _stratified_worst_mass_deviation_margins;
    mutable size_t _cache_lock_acquisitions = 0;
    mutable size_t _cache_lock_contentions = 0;
    mutable unsigned long long _cache_lock_wait_nanoseconds = 0;
    mutable unsigned long long _cache_lock_max_wait_nanoseconds = 0;
    mutable size_t _cache_lock_hold_samples = 0;
    mutable unsigned long long _cache_lock_hold_nanoseconds = 0;
    mutable unsigned long long _cache_lock_max_hold_nanoseconds = 0;

    /** Calculates a scalar negative tail probability without using the dense
        lookup table. This is used for large C values and memoizes each result. */
    double getOnDemandProbabilityFor(Parameters::ScanRateType scanrate, count_t T, count_t S, count_t x) const;

public:
    HypergeometricProbabilityLookup() = default;
    HypergeometricProbabilityLookup(const HypergeometricProbabilityLookup& other) {
        throw prg_error("copy constructor not implemented.", "HypergeometricProbabilityLookup");
    }

    /** Initializes hypergeometric evaluation for a scan.
        For small C, this precomputes the dense table of negative tail probabilities.
        For large C, this records C and scanrate, then evaluates probabilities on demand. */
    void calculateHG(Parameters::ScanRateType scanrate, const std::set<count_t>& caseWindows, count_t C, int threshold=DENSE_LOOKUP_CASE_THRESHOLD);
    /** Returns a negative tail probability for the day-of-week stratified distribution.
        Each day-of-week is treated as its own hypergeometric distribution, then the
        seven daily PMFs are convolved to obtain the total cluster count distribution. */
    double getStratifiedProbabilityFor(
        Parameters::ScanRateType scanrate,
        const CountByDay_t& totalCasesByDay,
        const CountByDay_t& spatialCasesByDay,
        const CountByDay_t& windowCasesByDay,
        count_t x
    ) const;
    /** Returns the probability at cases windows count, spatial count, and cases in cluster. */
    double getProbabilityFor(count_t T, const SpatialCases& S, count_t x) const {
        return S.getProbabilityAt(_T_index[T], x);
    }
    /** Returns the probability at cases windows count, spatial count, and cases in cluster. */
    double getProbabilityForChecked(count_t T, const SpatialCases& S, count_t x) const {
        return S.getProbabilityAtChecked(_T_index.at(T), x);
    }
    /** Returns the probability at cases windows count, spatial count, and cases in cluster. */
    double getProbabilityForChecked(count_t T, count_t S, count_t x) const {
        if (!_use_lookup_table) return getOnDemandProbabilityFor(_scanrate, T, S, x);
        if (!_spatial_cases.size() || static_cast<size_t>(S) > _spatial_cases.size() - 1U) return PROBABILITY_UNSET;
        return _spatial_cases[S].getProbabilityAtChecked(_T_index.at(T), x);
    }
    /** Returns the probability at cases windows count, spatial count, and cases in cluster. */
    double getProbabilityFor(count_t T, count_t S, count_t x) const {
        if (!_use_lookup_table) return getOnDemandProbabilityFor(_scanrate, T, S, x);
        return _spatial_cases[S].getProbabilityAt(_T_index[T], x);
    }
    /** Returns the probability at cases windows count, spatial count, and cases in cluster. */
    double getProbabilityFor_Checked(count_t T, count_t S, count_t x) const {
        return getProbabilityForChecked(T, S, x);
    }
    /** Exposed for tests and diagnostics; false means the large-C on-demand path is active. */
    bool isUsingLookupTable() const { return _use_lookup_table; }
    /** Returns the number of memoized on-demand probabilities and an estimated cache footprint. */
    ProbabilityCacheStatistics getProbabilityCacheStatistics() const;
    const SpatialCases& getSpatialCases(size_t S) const {
        return _spatial_cases[S];
    }
    void printHG(const std::set<count_t>& caseWindows) const;
    void clear() {
        _spatial_cases.clear();
        _T_index.clear();
        _case_window_count = 0;
        _probability_cache.clear();
        _probability_cache_hits = 0;
        _probability_cache_misses = 0;
        _probability_cache_invalid_requests = 0;
        _probability_cache_tail_evaluations = 0;
        _probability_cache_total_tail_terms = 0;
        _probability_cache_max_tail_terms = 0;
        _stratified_probability_cache.clear();
        _stratified_margin_set_cache.clear();
        _stratified_probability_cache_hits = 0;
        _stratified_probability_cache_misses = 0;
        _stratified_probability_cache_invalid_requests = 0;
        _stratified_probability_cache_out_of_support_requests = 0;
        _stratified_probability_cache_underflow_requests = 0;
        _stratified_probability_cache_no_distribution_requests = 0;
        _stratified_probability_cache_tail_evaluations = 0;
        _stratified_margin_set_reuse_opportunities = 0;
        _stratified_day_pmf_cache.clear();
        _stratified_day_pmf_requests = 0;
        _stratified_day_pmf_cache_hits = 0;
        _stratified_day_pmf_cache_misses = 0;
        _stratified_day_pmf_reuse_opportunities = 0;
        _stratified_day_pmf_total_terms = 0;
        _stratified_day_pmf_max_terms = 0;
        _stratified_convolution_count = 0;
        _stratified_convolution_input_terms = 0;
        _stratified_combined_pmf_total_size = 0;
        _stratified_combined_pmf_max_size = 0;
        _stratified_mass_conservation_checks = 0;
        _stratified_max_mass_deviation = 0.0;
        _stratified_min_combined_pmf_mass = std::numeric_limits<double>::infinity();
        _stratified_worst_mass_deviation_margins.clear();
        _cache_lock_acquisitions = 0;
        _cache_lock_contentions = 0;
        _cache_lock_wait_nanoseconds = 0;
        _cache_lock_max_wait_nanoseconds = 0;
        _cache_lock_hold_samples = 0;
        _cache_lock_hold_nanoseconds = 0;
        _cache_lock_max_hold_nanoseconds = 0;
    }
};
#endif
