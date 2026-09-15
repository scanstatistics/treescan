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
BOOST_AUTO_TEST_CASE( stratified_probability_reports_convolved_pmf_mass_conservation ) {
    HypergeometricProbabilityLookup lookup;
    HypergeometricProbabilityLookup::CountByDay_t totalCasesByDay = { 10, 10, 10, 10, 10, 10, 10 };
    HypergeometricProbabilityLookup::CountByDay_t spatialCasesByDay = { 5, 5, 5, 5, 5, 5, 5 };
    HypergeometricProbabilityLookup::CountByDay_t windowCasesByDay = { 4, 4, 4, 4, 4, 4, 4 };

    HypergeometricProbabilityLookup::ProbabilityCacheStatistics statistics = lookup.getProbabilityCacheStatistics();
    BOOST_CHECK_EQUAL(statistics.massConservationChecks, 0U);
    BOOST_CHECK_EQUAL(statistics.minCombinedPmfMass, 0.0);
    BOOST_CHECK(statistics.worstMassDeviationMargins.empty());

    // These margins are small enough that no bin of the convolved distribution underflows,
    // so the combined PMF must still sum to one within summation noise.
    lookup.getStratifiedProbabilityFor(Parameters::HIGHRATE, totalCasesByDay, spatialCasesByDay, windowCasesByDay, 15);
    statistics = lookup.getProbabilityCacheStatistics();
    BOOST_CHECK_EQUAL(statistics.massConservationChecks, 1U);
    BOOST_CHECK(statistics.maxMassDeviation <= HypergeometricProbabilityLookup::MASS_CONSERVATION_TOLERANCE);
    BOOST_CHECK_CLOSE(statistics.minCombinedPmfMass, 1.0, 1e-6);

    // Repeating the same margins with a different x answers from the margin set cache, so
    // no convolution runs and no additional check is recorded.
    lookup.getStratifiedProbabilityFor(Parameters::HIGHRATE, totalCasesByDay, spatialCasesByDay, windowCasesByDay, 16);
    statistics = lookup.getProbabilityCacheStatistics();
    BOOST_CHECK_EQUAL(statistics.massConservationChecks, 1U);

    // A new margin set convolves again and is checked in its turn.
    windowCasesByDay = { 3, 3, 3, 3, 3, 3, 3 };
    lookup.getStratifiedProbabilityFor(Parameters::HIGHRATE, totalCasesByDay, spatialCasesByDay, windowCasesByDay, 12);
    statistics = lookup.getProbabilityCacheStatistics();
    BOOST_CHECK_EQUAL(statistics.massConservationChecks, 2U);
    BOOST_CHECK(statistics.maxMassDeviation <= HypergeometricProbabilityLookup::MASS_CONSERVATION_TOLERANCE);
    BOOST_CHECK_CLOSE(statistics.minCombinedPmfMass, 1.0, 1e-6);
}
BOOST_AUTO_TEST_CASE( stratified_probability_classifies_out_of_support_requests ) {
    HypergeometricProbabilityLookup lookup;
    HypergeometricProbabilityLookup::CountByDay_t totalCasesByDay = { 10, 10, 10, 10, 10, 10, 10 };
    HypergeometricProbabilityLookup::CountByDay_t spatialCasesByDay = { 8, 8, 8, 8, 8, 8, 8 };
    HypergeometricProbabilityLookup::CountByDay_t windowCasesByDay = { 7, 7, 7, 7, 7, 7, 7 };

    // Support is 35..49, so x=50 cannot occur. The sentinel is the correct answer here and
    // must be attributed to the support check rather than to underflow.
    double probability = lookup.getStratifiedProbabilityFor(
        Parameters::HIGHRATE, totalCasesByDay, spatialCasesByDay, windowCasesByDay, 50
    );
    BOOST_CHECK_EQUAL(probability, HypergeometricProbabilityLookup::PROBABILITY_UNSET);

    HypergeometricProbabilityLookup::ProbabilityCacheStatistics statistics = lookup.getProbabilityCacheStatistics();
    BOOST_CHECK_EQUAL(statistics.invalidRequests, 1U);
    BOOST_CHECK_EQUAL(statistics.outOfSupportRequests, 1U);
    BOOST_CHECK_EQUAL(statistics.underflowRequests, 0U);
    BOOST_CHECK_EQUAL(statistics.noDistributionRequests, 0U);
}
BOOST_AUTO_TEST_CASE( stratified_probability_clamps_underflowed_tail_instead_of_discarding_cluster ) {
    HypergeometricProbabilityLookup lookup;
    HypergeometricProbabilityLookup::CountByDay_t totalCasesByDay = { 1000, 1000, 1000, 1000, 1000, 1000, 1000 };
    HypergeometricProbabilityLookup::CountByDay_t spatialCasesByDay = { 200, 200, 200, 200, 200, 200, 200 };
    HypergeometricProbabilityLookup::CountByDay_t windowCasesByDay = { 200, 200, 200, 200, 200, 200, 200 };

    // Each day can contribute x_d=0..200, so x=1400 is the top of the support and is a
    // legitimate, maximally extreme cluster. Its true probability is about 1e-1521: each
    // day contributes about 1e-217, which is representable, but the convolution multiplies
    // them in ordinary space and the product collapses to exactly zero.
    double probability = lookup.getStratifiedProbabilityFor(
        Parameters::HIGHRATE, totalCasesByDay, spatialCasesByDay, windowCasesByDay, 1400
    );
    HypergeometricProbabilityLookup::ProbabilityCacheStatistics statistics = lookup.getProbabilityCacheStatistics();

    // The cluster must survive at the representable floor rather than being dropped, which
    // is the same contract the scalar path gets from negativeProbabilityFromLogTail. It is
    // still counted as a clamped tail, because ranking it against other clamped clusters is
    // not possible without carrying a log scale through the convolution.
    BOOST_CHECK_EQUAL(probability, -std::numeric_limits<double>::denorm_min());
    BOOST_CHECK(probability != HypergeometricProbabilityLookup::PROBABILITY_UNSET);
    BOOST_CHECK_EQUAL(statistics.underflowRequests, 1U);
    BOOST_CHECK_EQUAL(statistics.invalidRequests, 0U);
    BOOST_CHECK_EQUAL(statistics.outOfSupportRequests, 0U);
    BOOST_CHECK_EQUAL(statistics.noDistributionRequests, 0U);
    // A clamped result is a real answer, so it is cached and counted like any other.
    BOOST_CHECK_EQUAL(statistics.entries, 1U);
    BOOST_CHECK_EQUAL(statistics.tailEvaluations, 1U);

    // Mass conservation stays clean even though the far tail underflowed, which is exactly
    // why it cannot be used on its own to decide whether underflow is happening.
    BOOST_CHECK_EQUAL(statistics.massConservationChecks, 1U);
    BOOST_CHECK(statistics.maxMassDeviation <= HypergeometricProbabilityLookup::MASS_CONSERVATION_TOLERANCE);
}
BOOST_AUTO_TEST_CASE( stratified_cache_memory_estimate_includes_tail_array_payload ) {
    HypergeometricProbabilityLookup lookup;
    HypergeometricProbabilityLookup::CountByDay_t totalCasesByDay = { 500, 500, 500, 500, 500, 500, 500 };
    HypergeometricProbabilityLookup::CountByDay_t spatialCasesByDay = { 100, 100, 100, 100, 100, 100, 100 };
    HypergeometricProbabilityLookup::CountByDay_t windowCasesByDay = { 100, 100, 100, 100, 100, 100, 100 };

    lookup.getStratifiedProbabilityFor(
        Parameters::HIGHRATE, totalCasesByDay, spatialCasesByDay, windowCasesByDay, 300
    );
    HypergeometricProbabilityLookup::ProbabilityCacheStatistics statistics = lookup.getProbabilityCacheStatistics();

    // The margin set cache holds one entry whose tail array spans the 0..700 support. Those
    // doubles live on the heap, so counting only sizeof(value_type) would miss them and
    // understate the cache that dominates this path.
    const size_t oneTailArrayBytes = 701U * sizeof(double);
    BOOST_CHECK_EQUAL(statistics.uniqueStratifiedMarginSets, 1U);
    BOOST_CHECK(statistics.marginSetCacheBytes > oneTailArrayBytes);
    // The tiers must account for the whole estimate between them.
    BOOST_CHECK_EQUAL(statistics.estimatedMemoryBytes,
        statistics.tailCacheBytes + statistics.marginSetCacheBytes + statistics.dayPmfCacheBytes);

    // Entry-weighted term counts describe what is stored: one high tail over the 0..700
    // support, and one cached day PMF over each day's 0..100 support. The request-weighted
    // averages alongside them count the same day PMF once per request instead of once per
    // entry, so only these two track the memory.
    BOOST_CHECK_EQUAL(statistics.cachedMarginSetTailTerms, 701U);
    BOOST_CHECK_EQUAL(statistics.dayPmfCacheEntries, 1U);
    BOOST_CHECK_EQUAL(statistics.cachedDayPmfTerms, 101U);
}
BOOST_AUTO_TEST_CASE( stratified_margin_set_cache_entry_survives_being_moved_in ) {
    HypergeometricProbabilityLookup::CountByDay_t totalCasesByDay = { 10, 10, 10, 10, 10, 10, 10 };
    HypergeometricProbabilityLookup::CountByDay_t spatialCasesByDay = { 5, 5, 5, 5, 5, 5, 5 };
    HypergeometricProbabilityLookup::CountByDay_t windowCasesByDay = { 4, 4, 4, 4, 4, 4, 4 };

    // The tail arrays are moved into the margin set cache instead of copied, so the entry
    // must still hold them afterwards. A moved-out entry would leave an empty array behind
    // and every later x for these margins would come back wrong or unset.
    HypergeometricProbabilityLookup cachingLookup;
    cachingLookup.getStratifiedProbabilityFor(
        Parameters::HIGHRATE, totalCasesByDay, spatialCasesByDay, windowCasesByDay, 20
    );
    double fromCache = cachingLookup.getStratifiedProbabilityFor(
        Parameters::HIGHRATE, totalCasesByDay, spatialCasesByDay, windowCasesByDay, 21
    );

    // A fresh lookup convolves from scratch for the same x and must agree exactly.
    HypergeometricProbabilityLookup freshLookup;
    double fromConvolution = freshLookup.getStratifiedProbabilityFor(
        Parameters::HIGHRATE, totalCasesByDay, spatialCasesByDay, windowCasesByDay, 21
    );

    BOOST_CHECK(fromCache < 0.0);
    BOOST_CHECK(fromCache != HypergeometricProbabilityLookup::PROBABILITY_UNSET);
    BOOST_CHECK_EQUAL(fromCache, fromConvolution);

    HypergeometricProbabilityLookup::ProbabilityCacheStatistics statistics = cachingLookup.getProbabilityCacheStatistics();
    // Each day spans x_d=0..4, so the combined support is 0..28 and the one stored high tail
    // holds 29 terms. Reading zero here would mean the entry was left moved-out.
    BOOST_CHECK_EQUAL(statistics.uniqueStratifiedMarginSets, 1U);
    BOOST_CHECK_EQUAL(statistics.cachedMarginSetTailTerms, 29U);
    BOOST_CHECK_EQUAL(statistics.stratifiedMarginSetReuseOpportunities, 1U);
    BOOST_CHECK_EQUAL(statistics.convolutionCount, 6U); // the second x reused the cached tails
}
BOOST_AUTO_TEST_CASE( stratified_day_pmf_cache_serves_shared_day_margins_across_margin_sets ) {
    HypergeometricProbabilityLookup lookup;
    HypergeometricProbabilityLookup::CountByDay_t totalCasesByDay = { 10, 10, 10, 10, 10, 10, 10 };
    HypergeometricProbabilityLookup::CountByDay_t windowCasesByDay = { 4, 4, 4, 4, 4, 4, 4 };

    // Two different margin sets built from the same multiset of day margins: the odd day is
    // last in one and first in the other. The day PMF keys are (C_d, S_d, T_d) with no day
    // index, so the second set hits the day PMF cache for all seven days, which is the path
    // that now reads the cached PMF through a pointer after releasing the lock rather than
    // copying it out. Both sets convolve the same seven distributions, so the tails must
    // agree.
    //
    // They agree to within rounding rather than exactly. Both day margins here have support
    // 0..4, since min(S_d, T_d) is 4 either way, so all seven PMFs are the same length and
    // the sort by size leaves them in whatever order std::sort happens to produce for that
    // input - it is not stable. The two sets therefore multiply and add the same values in
    // different orders, and floating point is not associative. The result stays
    // deterministic for a given margin set, which is what the caches depend on.
    HypergeometricProbabilityLookup::CountByDay_t oddDayLast = { 5, 5, 5, 5, 5, 5, 6 };
    HypergeometricProbabilityLookup::CountByDay_t oddDayFirst = { 6, 5, 5, 5, 5, 5, 5 };

    double first = lookup.getStratifiedProbabilityFor(
        Parameters::HIGHRATE, totalCasesByDay, oddDayLast, windowCasesByDay, 21
    );
    double second = lookup.getStratifiedProbabilityFor(
        Parameters::HIGHRATE, totalCasesByDay, oddDayFirst, windowCasesByDay, 21
    );
    BOOST_CHECK(first < 0.0);
    BOOST_CHECK(first != HypergeometricProbabilityLookup::PROBABILITY_UNSET);
    check_probability(second, first);

    HypergeometricProbabilityLookup::ProbabilityCacheStatistics statistics = lookup.getProbabilityCacheStatistics();
    BOOST_CHECK_EQUAL(statistics.uniqueStratifiedMarginSets, 2U);
    // Only two distinct day margins exist: (10,5,4) and (10,6,4).
    BOOST_CHECK_EQUAL(statistics.dayPmfCacheEntries, 2U);
    BOOST_CHECK_EQUAL(statistics.dayPmfRequests, 14U);
    BOOST_CHECK_EQUAL(statistics.dayPmfCacheMisses, 2U);
    BOOST_CHECK_EQUAL(statistics.dayPmfCacheHits, 12U);
}
BOOST_AUTO_TEST_CASE( probability_cache_lock_records_acquisitions_without_contention ) {
    HypergeometricProbabilityLookup lookup;
    HypergeometricProbabilityLookup::CountByDay_t totalCasesByDay = { 10, 10, 10, 10, 10, 10, 10 };
    HypergeometricProbabilityLookup::CountByDay_t spatialCasesByDay = { 5, 5, 5, 5, 5, 5, 5 };
    HypergeometricProbabilityLookup::CountByDay_t windowCasesByDay = { 4, 4, 4, 4, 4, 4, 4 };

    HypergeometricProbabilityLookup::ProbabilityCacheStatistics statistics = lookup.getProbabilityCacheStatistics();
    const size_t acquisitionsForOneReport = statistics.lockAcquisitions;
    BOOST_CHECK_EQUAL(acquisitionsForOneReport, 1U); // reading the statistics takes the lock itself
    BOOST_CHECK_EQUAL(statistics.lockContentions, 0U);

    // One miss takes the lock to check the caches, once per weekday to look up the day PMF,
    // once per weekday to store it, and once to record the result. Nothing else is running,
    // so every acquisition must be uncontended and no wait time can accrue.
    lookup.getStratifiedProbabilityFor(Parameters::HIGHRATE, totalCasesByDay, spatialCasesByDay, windowCasesByDay, 15);
    statistics = lookup.getProbabilityCacheStatistics();
    BOOST_CHECK(statistics.lockAcquisitions > acquisitionsForOneReport);
    BOOST_CHECK_EQUAL(statistics.lockContentions, 0U);
    BOOST_CHECK_EQUAL(statistics.lockWaitNanoseconds, 0U);
    BOOST_CHECK_EQUAL(statistics.maxLockWaitNanoseconds, 0U);

    // Hold time is sampled sparsely, so a handful of acquisitions should produce none.
    BOOST_CHECK(statistics.lockAcquisitions < HypergeometricProbabilityLookup::CACHE_LOCK_HOLD_SAMPLE_INTERVAL);
    BOOST_CHECK_EQUAL(statistics.lockHoldSamples, 0U);

    // clear() has to reset the lock counters along with everything else, or a second scan
    // reports the first scan's contention.
    lookup.clear();
    statistics = lookup.getProbabilityCacheStatistics();
    BOOST_CHECK_EQUAL(statistics.lockAcquisitions, 1U);
    BOOST_CHECK_EQUAL(statistics.lockContentions, 0U);
    BOOST_CHECK_EQUAL(statistics.lockHoldSamples, 0U);
}
BOOST_AUTO_TEST_CASE( stratified_margin_set_cache_stores_only_the_tail_the_scan_rate_reads ) {
    HypergeometricProbabilityLookup::CountByDay_t totalCasesByDay = { 500, 500, 500, 500, 500, 500, 500 };
    HypergeometricProbabilityLookup::CountByDay_t spatialCasesByDay = { 100, 100, 100, 100, 100, 100, 100 };
    HypergeometricProbabilityLookup::CountByDay_t windowCasesByDay = { 100, 100, 100, 100, 100, 100, 100 };

    // A single-sided scan rate never consults the opposite cumulative direction, so storing
    // it doubles the largest cache for nothing. Both one-sided rates should cache about half
    // of what the two-sided rate needs, for the same margins and the same support.
    HypergeometricProbabilityLookup highRateLookup, lowRateLookup, bothTailsLookup;
    highRateLookup.getStratifiedProbabilityFor(
        Parameters::HIGHRATE, totalCasesByDay, spatialCasesByDay, windowCasesByDay, 300
    );
    lowRateLookup.getStratifiedProbabilityFor(
        Parameters::LOWRATE, totalCasesByDay, spatialCasesByDay, windowCasesByDay, 100
    );
    bothTailsLookup.getStratifiedProbabilityFor(
        Parameters::HIGHORLOWRATE, totalCasesByDay, spatialCasesByDay, windowCasesByDay, 300
    );

    const size_t highRateBytes = highRateLookup.getProbabilityCacheStatistics().marginSetCacheBytes;
    const size_t lowRateBytes = lowRateLookup.getProbabilityCacheStatistics().marginSetCacheBytes;
    const size_t bothTailsBytes = bothTailsLookup.getProbabilityCacheStatistics().marginSetCacheBytes;
    const size_t oneTailArrayBytes = 701U * sizeof(double);

    BOOST_CHECK_EQUAL(highRateBytes, lowRateBytes);
    BOOST_CHECK(bothTailsBytes >= highRateBytes + oneTailArrayBytes);
}
BOOST_AUTO_TEST_CASE( stratified_probability_agrees_across_scan_rates_storing_one_or_both_tails ) {
    HypergeometricProbabilityLookup::CountByDay_t totalCasesByDay = { 10, 10, 10, 10, 10, 10, 10 };
    HypergeometricProbabilityLookup::CountByDay_t spatialCasesByDay = { 5, 5, 5, 5, 5, 5, 5 };
    HypergeometricProbabilityLookup::CountByDay_t windowCasesByDay = { 4, 4, 4, 4, 4, 4, 4 };

    // Dropping the unread direction must not change any answer. The high-or-low rate stores
    // both tails and picks by side of the expectation, so for an x above the mean it has to
    // return exactly what the high rate returns from its single stored tail, and likewise
    // below the mean against the low rate.
    HypergeometricProbabilityLookup highRateLookup, lowRateLookup, bothTailsLookup;
    double high = highRateLookup.getStratifiedProbabilityFor(
        Parameters::HIGHRATE, totalCasesByDay, spatialCasesByDay, windowCasesByDay, 20
    );
    double low = lowRateLookup.getStratifiedProbabilityFor(
        Parameters::LOWRATE, totalCasesByDay, spatialCasesByDay, windowCasesByDay, 5
    );
    double bothHigh = bothTailsLookup.getStratifiedProbabilityFor(
        Parameters::HIGHORLOWRATE, totalCasesByDay, spatialCasesByDay, windowCasesByDay, 20
    );
    double bothLow = bothTailsLookup.getStratifiedProbabilityFor(
        Parameters::HIGHORLOWRATE, totalCasesByDay, spatialCasesByDay, windowCasesByDay, 5
    );

    BOOST_CHECK(high < 0.0);
    BOOST_CHECK(low < 0.0);
    BOOST_CHECK_EQUAL(high, bothHigh);
    BOOST_CHECK_EQUAL(low, bothLow);
}
BOOST_AUTO_TEST_CASE( stratified_high_or_low_rate_covers_x_at_the_expected_value ) {
    HypergeometricProbabilityLookup::CountByDay_t totalCasesByDay = { 10, 10, 10, 10, 10, 10, 10 };
    HypergeometricProbabilityLookup::CountByDay_t spatialCasesByDay = { 5, 5, 5, 5, 5, 5, 5 };
    HypergeometricProbabilityLookup::CountByDay_t windowCasesByDay = { 4, 4, 4, 4, 4, 4, 4 };

    // Each day expects 5*4/10 = 2 cases exactly, so the stratified expectation is exactly
    // 14.0 and x=14 lands precisely on it. Strict comparisons against the expectation
    // selected neither tail for that x, so a legitimate cluster fell through the tail
    // selection and was reported as unset. x at the mode has a large tail probability either
    // way, so this was quietly discarding an entirely ordinary result.
    HypergeometricProbabilityLookup bothTailsLookup;
    double atExpectation = bothTailsLookup.getStratifiedProbabilityFor(
        Parameters::HIGHORLOWRATE, totalCasesByDay, spatialCasesByDay, windowCasesByDay, 14
    );
    BOOST_CHECK(atExpectation != HypergeometricProbabilityLookup::PROBABILITY_UNSET);
    BOOST_CHECK(atExpectation < 0.0);

    HypergeometricProbabilityLookup::ProbabilityCacheStatistics statistics = bothTailsLookup.getProbabilityCacheStatistics();
    BOOST_CHECK_EQUAL(statistics.invalidRequests, 0U);
    BOOST_CHECK_EQUAL(statistics.outOfSupportRequests, 0U);

    // A tie goes to the high tail, matching the x >= ceil(S*T/C) convention the scalar and
    // dense-table paths use, and one below the expectation must still take the low tail.
    HypergeometricProbabilityLookup highRateLookup, lowRateLookup;
    double highRateAt14 = highRateLookup.getStratifiedProbabilityFor(
        Parameters::HIGHRATE, totalCasesByDay, spatialCasesByDay, windowCasesByDay, 14
    );
    double lowRateAt13 = lowRateLookup.getStratifiedProbabilityFor(
        Parameters::LOWRATE, totalCasesByDay, spatialCasesByDay, windowCasesByDay, 13
    );
    double belowExpectation = bothTailsLookup.getStratifiedProbabilityFor(
        Parameters::HIGHORLOWRATE, totalCasesByDay, spatialCasesByDay, windowCasesByDay, 13
    );
    BOOST_CHECK_EQUAL(atExpectation, highRateAt14);
    BOOST_CHECK_EQUAL(belowExpectation, lowRateAt13);
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


