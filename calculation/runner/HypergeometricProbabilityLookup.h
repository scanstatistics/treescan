#ifndef __HYPERGEOMETRICPROBABILITYLOOKUP_H
#define __HYPERGEOMETRICPROBABILITYLOOKUP_H

#include "MinimalGrowthArray.h"
#include "PrjException.h"
#include "Parameters.h"
#include <array>
#include <vector>
#include <set>
#include <algorithm>
#include <stdexcept>
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

    struct ProbabilityCacheStatistics {
        size_t C = 0;
        size_t T = 0;
        //size_t evaluatedMarginPairs = 0;
        //size_t estimatedMaxCacheEntries = 0;
        size_t entries = 0;
        size_t estimatedMemoryBytes = 0;
        size_t onDemandRequests = 0;
        size_t cacheHits = 0;
        size_t cacheMisses = 0;
        size_t invalidRequests = 0;
        size_t tailEvaluations = 0;
        size_t totalTailTerms = 0;
        size_t maxTailTerms = 0;
    };

    class SpatialCases {
    public:
        typedef std::vector<std::pair<size_t, MinimalGrowthArray<double>>> SpatialCasesData_t;

    private:
        SpatialCasesData_t _probabilities_of_x_at_T;

    public:
        const SpatialCasesData_t& getProbabilitiesOfXAtT() const {
            return _probabilities_of_x_at_T;
        }
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
    struct ProbabilityMarginKey {
        count_t T;
        count_t S;

        bool operator==(const ProbabilityMarginKey& other) const {
            return T == other.T && S == other.S;
        }
    };
    struct ProbabilityMarginKeyHash {
        size_t operator()(const ProbabilityMarginKey& key) const;
    };
    typedef std::unordered_map<ProbabilityKey, double, ProbabilityKeyHash> ProbabilityCache_t;
    //typedef std::unordered_set<ProbabilityMarginKey, ProbabilityMarginKeyHash> ProbabilityMarginSet_t;

    std::vector<SpatialCases> _spatial_cases; // spatial cases dimension
    mutable std::vector<unsigned int> _T_index; // index mapping for T values
    count_t _total_cases = 0;
    size_t _case_window_count = 0;
    Parameters::ScanRateType _scanrate = Parameters::HIGHRATE;
    bool _use_lookup_table = true;
    mutable boost::mutex _probability_cache_mutex;
    mutable ProbabilityCache_t _probability_cache;
    //mutable ProbabilityMarginSet_t _probability_margin_pairs;
    //mutable size_t _estimated_max_cache_entries = 0;
    mutable size_t _probability_cache_hits = 0;
    mutable size_t _probability_cache_misses = 0;
    mutable size_t _probability_cache_invalid_requests = 0;
    mutable size_t _probability_cache_tail_evaluations = 0;
    mutable size_t _probability_cache_total_tail_terms = 0;
    mutable size_t _probability_cache_max_tail_terms = 0;

    /** Calculates a scalar negative tail probability without using the dense
        lookup table. This is used for large C values and memoizes each result. */
    double getOnDemandProbabilityFor(Parameters::ScanRateType scanrate, count_t T, count_t S, count_t x) const;
    size_t estimateCacheEntriesForMargin(Parameters::ScanRateType scanrate, count_t T, count_t S) const;

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
        //_probability_margin_pairs.clear();
        //_estimated_max_cache_entries = 0;
        _probability_cache_hits = 0;
        _probability_cache_misses = 0;
        _probability_cache_invalid_requests = 0;
        _probability_cache_tail_evaluations = 0;
        _probability_cache_total_tail_terms = 0;
        _probability_cache_max_tail_terms = 0;
    }
};
#endif
