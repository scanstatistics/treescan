#include <boost/test/unit_test.hpp>

#include "Parameters.h"
BOOST_AUTO_TEST_SUITE(parameters_suite)

BOOST_AUTO_TEST_CASE(reports_odds_ratio_for_tree_time_conditioned_on_node_and_time) {
    Parameters parameters;
    parameters.setScanType(Parameters::TREETIME);
    parameters.setConditionalType(Parameters::NODEANDTIME);

    BOOST_CHECK(parameters.getIsOddsRatio());
    BOOST_CHECK_EQUAL(parameters.getRelativeRiskReportingLabel(), "Odds Ratio");

    parameters.setPerformDayOfWeekAdjustment(true);
    BOOST_CHECK(parameters.getIsOddsRatio());
    BOOST_CHECK_EQUAL(parameters.getRelativeRiskReportingLabel(), "Odds Ratio");
}

BOOST_AUTO_TEST_CASE(reports_relative_risk_for_other_scan_configurations) {
    Parameters parameters;

    parameters.setScanType(Parameters::TREETIME);
    parameters.setConditionalType(Parameters::NODE);
    BOOST_CHECK(!parameters.getIsOddsRatio());
    BOOST_CHECK_EQUAL(parameters.getRelativeRiskReportingLabel(), "Relative Risk");

    parameters.setScanType(Parameters::TIMEONLY);
    parameters.setConditionalType(Parameters::TOTALCASES);
    BOOST_CHECK(!parameters.getIsOddsRatio());
    BOOST_CHECK_EQUAL(parameters.getRelativeRiskReportingLabel(), "Relative Risk");

    parameters.setScanType(Parameters::TREEONLY);
    parameters.setConditionalType(Parameters::TOTALCASES);
    BOOST_CHECK(!parameters.getIsOddsRatio());
    BOOST_CHECK_EQUAL(parameters.getRelativeRiskReportingLabel(), "Relative Risk");

    parameters.setScanType(Parameters::TREEONLY);
    parameters.setConditionalType(Parameters::NODEANDTIME);
    BOOST_CHECK(!parameters.getIsOddsRatio());
    BOOST_CHECK_EQUAL(parameters.getRelativeRiskReportingLabel(), "Relative Risk");
}

BOOST_AUTO_TEST_SUITE_END()
