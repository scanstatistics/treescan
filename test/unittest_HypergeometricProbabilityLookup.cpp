// Boost unit test header
#include <boost/test/unit_test.hpp>

#include <iostream>

#include "HypergeometricProbabilityLookup.h"

#include <set>
#include <vector>

BOOST_AUTO_TEST_SUITE( test_hypergeometric_probability_lookup )

namespace {
    const double TOLERANCE = 0.000001;

    void check_probability(double actual, double expected) {
        BOOST_CHECK_CLOSE(actual, expected, TOLERANCE);
    }
    // Small direct combination helpers for hand-checking the expected values in
    // the C=10000 test. They avoid pulling the production probability code into
    // the expected-value side of the test.
    double choose4(double n) {
        return n * (n - 1.0) * (n - 2.0) * (n - 3.0) / 24.0;
    }

    double choose3(double n) {
        return n * (n - 1.0) * (n - 2.0) / 6.0;
    }
}

BOOST_AUTO_TEST_CASE( high_rate_lookup_returns_negative_upper_tail_probabilities ) {
    HypergeometricProbabilityLookup lookup;
    std::set<count_t> caseWindows;
    caseWindows.insert(4);

    lookup.calculateHG(Parameters::HIGHRATE, caseWindows, 10);

    // This exercises the dense lookup path, where all probabilities are
    // precomputed for a small total case count C.
    // hypergeometric formula: P(X=x) = choose(S, x) * choose(C - S, T - x) / choose(C, T)
    // With C=10 total cases, S=5 spatial cases, and T=4 window cases:    
    //   P(X=x) = choose(5, x) * choose(5, 4 - x) / choose(10, 4)
    // The denominator is choose(10, 4) = 210. For x=0..4 the numerators are:
    //   5, 50, 100, 50, 5
    // Reducing by 5 gives probabilities:
    //   1/42, 10/42, 20/42, 10/42, 1/42
    // High-rate lookups store negative upper tails: -P(X >= x).
    check_probability(lookup.getProbabilityFor_Checked(4, 5, 2), -31.0 / 42.0);
    check_probability(lookup.getProbabilityFor_Checked(4, 5, 3), -11.0 / 42.0);
    check_probability(lookup.getProbabilityFor_Checked(4, 5, 4), -1.0 / 42.0);
}

BOOST_AUTO_TEST_CASE( low_rate_lookup_returns_negative_lower_tail_probabilities ) {
    HypergeometricProbabilityLookup lookup;
    std::set<count_t> caseWindows;
    caseWindows.insert(4);

    lookup.calculateHG(Parameters::LOWRATE, caseWindows, 10);

    // This uses the same dense distribution as the high-rate test, but verifies
    // that the low-rate scan reads the opposite tail of the distribution.
    // Low-rate lookups use the same distribution and store negative lower
    // tails: -P(X <= x), so x=1 is -(1/42 + 10/42) = -11/42.
    check_probability(lookup.getProbabilityFor_Checked(4, 5, 0), -1.0 / 42.0);
    check_probability(lookup.getProbabilityFor_Checked(4, 5, 1), -11.0 / 42.0);
}

BOOST_AUTO_TEST_CASE( high_or_low_rate_lookup_keeps_both_tails_in_one_range ) {
    HypergeometricProbabilityLookup lookup;
    std::set<count_t> caseWindows;
    caseWindows.insert(4);

    lookup.calculateHG(Parameters::HIGHORLOWRATE, caseWindows, 10);

    // HIGHORLOWRATE is the case where both tails can be queried for the same S
    // and T. This protects the trimmed storage range from dropping either side.
    // Combined-rate lookup keeps both negative tails in the same trimmed range.
    check_probability(lookup.getProbabilityFor_Checked(4, 5, 0), -1.0 / 42.0);
    check_probability(lookup.getProbabilityFor_Checked(4, 5, 1), -11.0 / 42.0);
    check_probability(lookup.getProbabilityFor_Checked(4, 5, 2), -31.0 / 42.0);
    check_probability(lookup.getProbabilityFor_Checked(4, 5, 3), -11.0 / 42.0);
    check_probability(lookup.getProbabilityFor_Checked(4, 5, 4), -1.0 / 42.0);
}


BOOST_AUTO_TEST_CASE( large_case_count_uses_on_demand_probability_cache ) {
    HypergeometricProbabilityLookup lookup;
    std::set<count_t> caseWindows;
    caseWindows.insert(4);

    lookup.calculateHG(Parameters::HIGHRATE, caseWindows, 10000);

    // C=10000 is above the dense table threshold, so the lookup should switch
    // to the memoized on-demand path instead of allocating a full S/T/x table.
    BOOST_CHECK(!lookup.isUsingLookupTable());

    // With T=4 and S=5000, the upper tail for x=2 is P(2)+P(3)+P(4).
    // These direct formulas independently compute the expected tail values.
    double denominator = choose4(10000.0);
    double p2 = (5000.0 * 4999.0 / 2.0) * (5000.0 * 4999.0 / 2.0) / denominator;
    double p3 = choose3(5000.0) * 5000.0 / denominator;
    double p4 = choose4(5000.0) / denominator;

    check_probability(lookup.getProbabilityFor_Checked(4, 5000, 2), -(p2 + p3 + p4));
    check_probability(lookup.getProbabilityFor_Checked(4, 5000, 4), -p4);
}

BOOST_AUTO_TEST_CASE( on_demand_probability_handles_extreme_upper_tail_without_zero_underflow ) {
    HypergeometricProbabilityLookup lookup;
    std::set<count_t> caseWindows;
    caseWindows.insert(4372);

    lookup.calculateHG(Parameters::HIGHRATE, caseWindows, 7000);

    // Regression case from a very concentrated cluster. The true upper-tail
    // probability is smaller than ordinary double arithmetic can represent, so
    // the on-demand log-space calculation should clamp to a tiny negative value
    // instead of returning zero or PROBABILITY_UNSET.
    double probability = lookup.getProbabilityFor_Checked(4372, 4200, 3800);
    BOOST_CHECK(!lookup.isUsingLookupTable());
    BOOST_CHECK(probability < 0.0);
    BOOST_CHECK(probability != HypergeometricProbabilityLookup::PROBABILITY_UNSET);
    BOOST_CHECK(std::isfinite(-std::log(-probability)));
}

BOOST_AUTO_TEST_CASE( on_demand_probability_cache_statistics_report_entries_and_memory ) {
    HypergeometricProbabilityLookup lookup;
    std::set<count_t> caseWindows;
    caseWindows.insert(4);

    lookup.calculateHG(Parameters::HIGHRATE, caseWindows, 10000);

    // The on-demand cache starts empty after initialization. It should grow only
    // when a probability is actually requested.
    HypergeometricProbabilityLookup::ProbabilityCacheStatistics statistics = lookup.getProbabilityCacheStatistics();
    BOOST_CHECK_EQUAL(statistics.cacheType, HypergeometricProbabilityLookup::ProbabilityCacheStatistics::NONE);
    BOOST_CHECK_EQUAL(statistics.entries, 0U);
    BOOST_CHECK_EQUAL(statistics.requests, 0U);
    BOOST_CHECK_EQUAL(statistics.cacheHits, 0U);
    BOOST_CHECK_EQUAL(statistics.cacheMisses, 0U);
    BOOST_CHECK_EQUAL(statistics.invalidRequests, 0U);
    BOOST_CHECK_EQUAL(statistics.tailEvaluations, 0U);
    BOOST_CHECK_EQUAL(statistics.totalTailTerms, 0U);
    BOOST_CHECK_EQUAL(statistics.maxTailTerms, 0U);
    BOOST_CHECK(statistics.estimatedMemoryBytes >= sizeof(void*));

    lookup.getProbabilityFor_Checked(4, 5000, 2);
    statistics = lookup.getProbabilityCacheStatistics();
    BOOST_CHECK_EQUAL(statistics.cacheType, HypergeometricProbabilityLookup::ProbabilityCacheStatistics::SCALAR_ON_DEMAND);
    BOOST_CHECK_EQUAL(statistics.entries, 1U);
    BOOST_CHECK_EQUAL(statistics.requests, 1U);
    BOOST_CHECK_EQUAL(statistics.cacheHits, 0U);
    BOOST_CHECK_EQUAL(statistics.cacheMisses, 1U);
    BOOST_CHECK_EQUAL(statistics.tailEvaluations, 1U);
    BOOST_CHECK_EQUAL(statistics.totalTailTerms, 3U);
    BOOST_CHECK_EQUAL(statistics.maxTailTerms, 3U);
    BOOST_CHECK(statistics.estimatedMemoryBytes > sizeof(double));

    // Repeating the same request should hit the memoized result, not add a
    // second entry or another tail evaluation for the same (scanrate, T, S, x) key.
    lookup.getProbabilityFor_Checked(4, 5000, 2);
    statistics = lookup.getProbabilityCacheStatistics();
    BOOST_CHECK_EQUAL(statistics.cacheType, HypergeometricProbabilityLookup::ProbabilityCacheStatistics::SCALAR_ON_DEMAND);
    BOOST_CHECK_EQUAL(statistics.entries, 1U);
    BOOST_CHECK_EQUAL(statistics.requests, 2U);
    BOOST_CHECK_EQUAL(statistics.cacheHits, 1U);
    BOOST_CHECK_EQUAL(statistics.cacheMisses, 1U);
    BOOST_CHECK_EQUAL(statistics.tailEvaluations, 1U);
    BOOST_CHECK_EQUAL(statistics.totalTailTerms, 3U);

    lookup.getProbabilityFor_Checked(4, 5000, 4);
    statistics = lookup.getProbabilityCacheStatistics();
    BOOST_CHECK_EQUAL(statistics.cacheType, HypergeometricProbabilityLookup::ProbabilityCacheStatistics::SCALAR_ON_DEMAND);
    BOOST_CHECK_EQUAL(statistics.entries, 2U);
    BOOST_CHECK_EQUAL(statistics.requests, 3U);
    BOOST_CHECK_EQUAL(statistics.cacheHits, 1U);
    BOOST_CHECK_EQUAL(statistics.cacheMisses, 2U);
    BOOST_CHECK_EQUAL(statistics.tailEvaluations, 2U);
    BOOST_CHECK_EQUAL(statistics.totalTailTerms, 4U);
    BOOST_CHECK_EQUAL(statistics.maxTailTerms, 3U);
}
BOOST_AUTO_TEST_CASE( stratified_probability_cache_statistics_report_entries_and_hits ) {
    HypergeometricProbabilityLookup lookup;
    HypergeometricProbabilityLookup::CountByDay_t totalCasesByDay = { 10, 10, 10, 10, 10, 10, 10 };
    HypergeometricProbabilityLookup::CountByDay_t spatialCasesByDay = { 5, 5, 5, 5, 5, 5, 5 };
    HypergeometricProbabilityLookup::CountByDay_t windowCasesByDay = { 4, 4, 4, 4, 4, 4, 4 };

    // This asks for the final day-of-week adjusted upper tail. The first request
    // must convolve the seven daily PMFs and store the finished tail probability.
    double firstProbability = lookup.getStratifiedProbabilityFor(
        Parameters::HIGHRATE, totalCasesByDay, spatialCasesByDay, windowCasesByDay, 15
    );
    HypergeometricProbabilityLookup::ProbabilityCacheStatistics statistics = lookup.getProbabilityCacheStatistics();
    BOOST_CHECK(firstProbability < 0.0);
    BOOST_CHECK_EQUAL(statistics.cacheType, HypergeometricProbabilityLookup::ProbabilityCacheStatistics::STRATIFIED_DAY_OF_WEEK);
    BOOST_CHECK_EQUAL(statistics.entries, 1U);
    BOOST_CHECK_EQUAL(statistics.requests, 1U);
    BOOST_CHECK_EQUAL(statistics.cacheHits, 0U);
    BOOST_CHECK_EQUAL(statistics.cacheMisses, 1U);
    BOOST_CHECK_EQUAL(statistics.invalidRequests, 0U);
    BOOST_CHECK_EQUAL(statistics.tailEvaluations, 1U);
    BOOST_CHECK_EQUAL(statistics.dayPmfRequests, 7U);
    BOOST_CHECK_EQUAL(statistics.dayPmfCacheEntries, 1U);
    BOOST_CHECK_EQUAL(statistics.dayPmfCacheHits, 6U);
    BOOST_CHECK_EQUAL(statistics.dayPmfCacheMisses, 1U);
    BOOST_CHECK_EQUAL(statistics.uniqueDayPmfMargins, 1U);
    BOOST_CHECK_EQUAL(statistics.dayPmfReuseOpportunities, 6U);
    BOOST_CHECK_EQUAL(statistics.uniqueStratifiedMarginSets, 1U);
    BOOST_CHECK_EQUAL(statistics.stratifiedMarginSetReuseOpportunities, 0U);
    BOOST_CHECK_EQUAL(statistics.totalDayPmfTerms, 35U);
    BOOST_CHECK_EQUAL(statistics.maxDayPmfTerms, 5U);
    BOOST_CHECK_EQUAL(statistics.convolutionCount, 6U);
    BOOST_CHECK_EQUAL(statistics.totalConvolutionInputTerms, 450U);
    BOOST_CHECK_EQUAL(statistics.totalCombinedPmfSize, 114U);
    BOOST_CHECK_EQUAL(statistics.maxCombinedPmfSize, 29U);
    BOOST_CHECK(statistics.estimatedMemoryBytes > sizeof(double));

    // Repeating the exact same DOW margins and x should reuse the cached final
    // tail value instead of rebuilding and reconvolving the weekday PMFs.
    double secondProbability = lookup.getStratifiedProbabilityFor(
        Parameters::HIGHRATE, totalCasesByDay, spatialCasesByDay, windowCasesByDay, 15
    );
    statistics = lookup.getProbabilityCacheStatistics();
    BOOST_CHECK_EQUAL(firstProbability, secondProbability);
    BOOST_CHECK_EQUAL(statistics.cacheType, HypergeometricProbabilityLookup::ProbabilityCacheStatistics::STRATIFIED_DAY_OF_WEEK);
    BOOST_CHECK_EQUAL(statistics.entries, 1U);
    BOOST_CHECK_EQUAL(statistics.requests, 2U);
    BOOST_CHECK_EQUAL(statistics.cacheHits, 1U);
    BOOST_CHECK_EQUAL(statistics.cacheMisses, 1U);
    BOOST_CHECK_EQUAL(statistics.tailEvaluations, 1U);
    BOOST_CHECK_EQUAL(statistics.dayPmfRequests, 7U);
    BOOST_CHECK_EQUAL(statistics.dayPmfCacheEntries, 1U);
    BOOST_CHECK_EQUAL(statistics.dayPmfCacheHits, 6U);
    BOOST_CHECK_EQUAL(statistics.dayPmfCacheMisses, 1U);
    BOOST_CHECK_EQUAL(statistics.uniqueDayPmfMargins, 1U);
    BOOST_CHECK_EQUAL(statistics.dayPmfReuseOpportunities, 6U);
    BOOST_CHECK_EQUAL(statistics.uniqueStratifiedMarginSets, 1U);
    BOOST_CHECK_EQUAL(statistics.stratifiedMarginSetReuseOpportunities, 0U);
    BOOST_CHECK_EQUAL(statistics.convolutionCount, 6U);

    // Changing only x misses the final tail cache, but it reuses the same seven
    // day-of-week margins. This is the diagnostic that tells us whether a future
    // combined PMF/CDF cache could answer several x values from one distribution.
    lookup.getStratifiedProbabilityFor(
        Parameters::HIGHRATE, totalCasesByDay, spatialCasesByDay, windowCasesByDay, 16
    );
    statistics = lookup.getProbabilityCacheStatistics();
    BOOST_CHECK_EQUAL(statistics.entries, 2U);
    BOOST_CHECK_EQUAL(statistics.requests, 3U);
    BOOST_CHECK_EQUAL(statistics.cacheHits, 1U);
    BOOST_CHECK_EQUAL(statistics.cacheMisses, 2U);
    BOOST_CHECK_EQUAL(statistics.tailEvaluations, 2U);
    BOOST_CHECK_EQUAL(statistics.dayPmfRequests, 7U);
    BOOST_CHECK_EQUAL(statistics.uniqueStratifiedMarginSets, 1U);
    BOOST_CHECK_EQUAL(statistics.stratifiedMarginSetReuseOpportunities, 1U);
    BOOST_CHECK_EQUAL(statistics.convolutionCount, 6U);
}
BOOST_AUTO_TEST_CASE( stratified_probability_uses_positive_support_offsets ) {
    HypergeometricProbabilityLookup lookup;
    HypergeometricProbabilityLookup::CountByDay_t totalCasesByDay = { 10, 10, 10, 10, 10, 10, 10 };
    HypergeometricProbabilityLookup::CountByDay_t spatialCasesByDay = { 8, 8, 8, 8, 8, 8, 8 };
    HypergeometricProbabilityLookup::CountByDay_t windowCasesByDay = { 7, 7, 7, 7, 7, 7, 7 };

    // Each day has support x_d=5..7, so the seven-day total has support 35..49.
    // Asking for P(X>=35) should return the whole distribution, not treat 35 as
    // an out-of-range vector index after trimming impossible leading values.
    double probability = lookup.getStratifiedProbabilityFor(
        Parameters::HIGHRATE, totalCasesByDay, spatialCasesByDay, windowCasesByDay, 35
    );
    check_probability(probability, -1.0);
}
BOOST_AUTO_TEST_CASE( spatial_cases_trim_unset_values_and_preserve_x_offset ) {
    HypergeometricProbabilityLookup::SpatialCases spatialCases;
    std::vector<double> probabilities;
    probabilities.push_back(HypergeometricProbabilityLookup::PROBABILITY_UNSET);
    probabilities.push_back(HypergeometricProbabilityLookup::PROBABILITY_UNSET);
    probabilities.push_back(-0.25);
    probabilities.push_back(-0.75);
    probabilities.push_back(HypergeometricProbabilityLookup::PROBABILITY_UNSET);

    // Dense lookup storage trims leading/trailing unset values. The first saved
    // x value must be remembered so later requests for x=2 and x=3 map back to
    // array indexes 0 and 1, not to negative or shifted indexes.
    spatialCases.addNegativeProbabilitiesFor(0, probabilities);

    const auto& data = spatialCases.getProbabilitiesOfXAtT();
    BOOST_REQUIRE_EQUAL(data.size(), 1);
    BOOST_CHECK_EQUAL(data[0].first, 2);
    BOOST_REQUIRE_EQUAL(data[0].second.size(), 2);
    BOOST_CHECK_EQUAL(spatialCases.getProbabilityAtChecked(0, 2), -0.25);
    BOOST_CHECK_EQUAL(spatialCases.getProbabilityAtChecked(0, 3), -0.75);
}

BOOST_AUTO_TEST_CASE( spatial_cases_require_probabilities_to_be_added_in_order ) {
    HypergeometricProbabilityLookup::SpatialCases spatialCases;
    std::vector<double> probabilities;
    probabilities.push_back(-0.5);

    // SpatialCases stores one probability vector per T index. Adding T=1 first
    // would leave a gap at T=0, so this should fail fast instead of creating a
    // sparse structure that getProbabilityAtChecked cannot index safely.
    BOOST_REQUIRE_THROW(spatialCases.addNegativeProbabilitiesFor(1, probabilities), prg_error);
}

BOOST_AUTO_TEST_SUITE_END()


