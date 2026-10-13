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
#include <cstdint>
#include <limits>
#include <memory>
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
    /** Separation between independently locked shards, in bytes. The largest cache line
        across the supported targets: 64 on x86-64, 128 on Apple Silicon. */
    static constexpr size_t CACHE_LINE_ALIGNMENT = 128;

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
        /** How the load actually fell across the shards. If the key hashes distribute poorly
            the busiest shard carries far more than its share and the sharding underdelivers
            while looking like it should have worked, so the imbalance has to be visible.
            With an even spread, maxShardAcquisitions is close to lockAcquisitions/shards. */
        size_t shards = 0;
        size_t maxShardAcquisitions = 0;
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

    /** Counters for the stratified path. One copy lives in each shard and they are summed
        when statistics are requested, so no counter is ever shared between threads working
        on different shards. Making these atomics instead would put a hundred million
        read-modify-writes on one cache line, which is the contention this design removes. */
    struct StratifiedCounters {
        size_t probabilityCacheHits = 0;
        size_t probabilityCacheMisses = 0;
        size_t invalidRequests = 0;
        size_t outOfSupportRequests = 0;
        size_t underflowRequests = 0;
        size_t noDistributionRequests = 0;
        size_t tailEvaluations = 0;
        size_t marginSetReuseOpportunities = 0;
        size_t dayPmfRequests = 0;
        size_t dayPmfCacheHits = 0;
        size_t dayPmfCacheMisses = 0;
        size_t dayPmfReuseOpportunities = 0;
        size_t dayPmfTotalTerms = 0;
        size_t dayPmfMaxTerms = 0;
        size_t convolutionCount = 0;
        size_t convolutionInputTerms = 0;
        size_t combinedPmfTotalSize = 0;
        size_t combinedPmfMaxSize = 0;
        size_t massConservationChecks = 0;
        double maxMassDeviation = 0.0;
        double minCombinedPmfMass = std::numeric_limits<double>::infinity();
        std::string worstMassDeviationMargins;
        size_t lockAcquisitions = 0;
        size_t lockContentions = 0;
        unsigned long long lockWaitNanoseconds = 0;
        unsigned long long maxLockWaitNanoseconds = 0;
        size_t lockHoldSamples = 0;
        unsigned long long lockHoldNanoseconds = 0;
        unsigned long long maxLockHoldNanoseconds = 0;
    };

    /** One independently locked slice of the stratified caches.

        A single mutex over all of these serialized every probability lookup: measured at 16
        threads it was held about 60 seconds of CPU while threads lost roughly 1,750
        thread-seconds waiting for it, and adding processors past ten made the scan slower
        rather than faster. Splitting the caches into independently locked shards divides
        both the collision rate and the time any one lock is held.

        alignas keeps each shard's mutex and counters off the cache lines of its neighbours.
        Without that the lock word of one shard shares a line with the next and the line
        still ping-pongs between cores, which silently undoes the sharding. The alignment is
        128 rather than 64 because Apple Silicon uses 128-byte cache lines where x86-64 uses
        64; over-aligning on x86 costs a few kilobytes across the whole array and under-
        aligning on arm64 would reintroduce exactly the false sharing this avoids. */
    struct alignas(CACHE_LINE_ALIGNMENT) CacheShard {
        mutable boost::mutex mutex;
        mutable StratifiedProbabilityCache_t tailCache;
        mutable StratifiedMarginSetCache_t marginSetCache;
        mutable DayPmfCache_t dayPmfCache;
        mutable StratifiedCounters counters;
    };

    /** Scoped lock over one shard's mutex that measures its own contention.

        try_lock is attempted first so an uncontended acquire costs the same single atomic
        as a plain lock, and the clock reads are paid only by a thread that actually has to
        wait. Hold time is sampled rather than always measured, because timing every
        acquisition would cost two clock reads on a path taken hundreds of millions of times
        per scan.

        Two things to keep in mind when reading the numbers. try_lock is permitted to fail
        spuriously, so the contended count is an upper bound. And because a thread can be
        descheduled while holding a lock, hold times include any period it spent off a
        processor - on a fully subscribed machine that, not the work inside, is what produces
        the occasional hold of tens of microseconds. */
    class CacheLock {
        const CacheShard& _shard;
        std::chrono::steady_clock::time_point _held_from;
        bool _sample_hold_time = false;

    public:
        explicit CacheLock(const CacheShard& shard);
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
    std::unique_ptr<CacheShard[]> _shards;
    size_t _shard_count = 0;
    unsigned int _shard_shift = 0; // how far to shift a mixed hash down to get a shard index

    /** Calculates a scalar negative tail probability without using the dense
        lookup table. This is used for large C values and memoizes each result. */
    double getOnDemandProbabilityFor(Parameters::ScanRateType scanrate, count_t T, count_t S, count_t x) const;
    /** Picks a shard from the high bits of a key's hash.

        The high bits specifically. Every key routed to a shard shares whichever bits chose
        that shard, so those bits must not be the ones the map inside the shard buckets on.
        Microsoft's unordered_map uses power-of-two bucket counts and masks the low bits, so
        sharding on the low bits there would collapse each map into a handful of buckets;
        libstdc++ and libc++ use prime bucket counts and a modulo, which tolerates it better.
        Taking the high bits is correct for the worst of the three and costs nothing on the
        others. The multiply is a standard mixing step so that keys differing only in their
        low bits still spread across shards. */
    size_t shardIndexFor(size_t hash) const {
        // Mixed and shifted in a fixed 64-bit type rather than in size_t. On a 32-bit build
        // the multiply would still promote to 64 bits while a size_t-derived shift would
        // leave far more than log2(shards) significant bits, and the truncating cast would
        // then index past the end of the array.
        const uint64_t mixed = static_cast<uint64_t>(hash) * 0x9E3779B97F4A7C15ULL;
        return static_cast<size_t>(mixed >> _shard_shift);
    }
    /** Tier 1 and tier 2 are sharded on the margins alone, excluding x, so that a margin set
        and every x asked about it land in the same shard. That keeps the existing lookup -
        check the final tail, fall through to the margin set tails - as one lock acquisition
        rather than two. */
    CacheShard& shardForMargins(const StratifiedMarginSetKey& key) const {
        return _shards[shardIndexFor(StratifiedMarginSetKeyHash()(key))];
    }
    /** Tier 3 has its own key, so a day PMF lookup may land in a different shard than the
        margin set that needed it. That is safe only because the two locks are never held at
        the same time; see the note on getStratifiedProbabilityFor. */
    CacheShard& shardForDayMargins(const DayPmfMarginKey& key) const {
        return _shards[shardIndexFor(DayPmfMarginKeyHash()(key))];
    }
    void allocateShards(unsigned int threads);

public:
    /** Shards are allocated here as well as in calculateHG, so a lookup used without a scan
        setup - as tests do - is still usable. calculateHG reallocates them once it knows how
        many threads the scan will run. */
    HypergeometricProbabilityLookup() { allocateShards(1); }
    HypergeometricProbabilityLookup(const HypergeometricProbabilityLookup& other) {
        throw prg_error("copy constructor not implemented.", "HypergeometricProbabilityLookup");
    }

    /** Initializes hypergeometric evaluation for a scan.
        For small C, this precomputes the dense table of negative tail probabilities.
        For large C, this records C and scanrate, then evaluates probabilities on demand. */
    void calculateHG(Parameters::ScanRateType scanrate, const std::set<count_t>& caseWindows, count_t C, bool use_lookup_table, unsigned int threads=1);
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
        // Empties every shard's caches and counters but keeps the shards themselves, so a
        // lookup reused across scans stays sharded as it was allocated.
        for (size_t shard = 0; shard < _shard_count; ++shard) {
            _shards[shard].tailCache.clear();
            _shards[shard].marginSetCache.clear();
            _shards[shard].dayPmfCache.clear();
            _shards[shard].counters = StratifiedCounters();
        }
    }
};
#endif
