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
}

BOOST_AUTO_TEST_CASE( high_rate_lookup_returns_negative_upper_tail_probabilities ) {
    HypergeometricProbabilityLookup lookup;
    std::set<count_t> caseWindows;
    caseWindows.insert(4);

    lookup.calculateHG(Parameters::HIGHRATE, caseWindows, 10);

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

    // Combined-rate lookup keeps both negative tails in the same trimmed range.
    check_probability(lookup.getProbabilityFor_Checked(4, 5, 0), -1.0 / 42.0);
    check_probability(lookup.getProbabilityFor_Checked(4, 5, 1), -11.0 / 42.0);
    check_probability(lookup.getProbabilityFor_Checked(4, 5, 2), -31.0 / 42.0);
    check_probability(lookup.getProbabilityFor_Checked(4, 5, 3), -11.0 / 42.0);
    check_probability(lookup.getProbabilityFor_Checked(4, 5, 4), -1.0 / 42.0);
}

BOOST_AUTO_TEST_CASE( spatial_cases_trim_unset_values_and_preserve_x_offset ) {
    HypergeometricProbabilityLookup::SpatialCases spatialCases;
    std::vector<double> probabilities;
    probabilities.push_back(HypergeometricProbabilityLookup::PROBABILITY_UNSET);
    probabilities.push_back(HypergeometricProbabilityLookup::PROBABILITY_UNSET);
    probabilities.push_back(-0.25);
    probabilities.push_back(-0.75);
    probabilities.push_back(HypergeometricProbabilityLookup::PROBABILITY_UNSET);

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

    BOOST_REQUIRE_THROW(spatialCases.addNegativeProbabilitiesFor(1, probabilities), prg_error);
}

BOOST_AUTO_TEST_SUITE_END()
