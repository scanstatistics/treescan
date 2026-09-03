#include <boost/test/unit_test.hpp>

#include "ScanRunner.h"
#include "PrintScreen.h"
#include "UtilityFunctions.h"
#include "RandomDistribution.h"

#include <cmath>
#include <limits>
#include <vector>

class DayOfWeekAdjustedRRRunner : public ScanRunner {
public:
    DayOfWeekAdjustedRRRunner(const Parameters& parameters, BasePrint& print)
        : ScanRunner(parameters, print) {}

    NodeStructure& addNodeWithExactBranchCounts(const std::vector<int>& exact_counts) {
        NodeStructure* node = new NodeStructure("node", getParameters(), exact_counts.size() + 1U);
        node->setID(static_cast<int>(_Nodes.size()));
        node->refBrC_C() = exact_counts;
        node->refBrC_C().push_back(0);
        TreeScan::cumulative_backward(node->refBrC_C());
        _Nodes.push_back(node);
        return *_Nodes.back();
    }

    void setExactTreeCounts(const std::vector<unsigned int>& exact_counts) {
        _totalcases_by_timeinterval = exact_counts;
        _totalcases_by_dayofweek.assign(7U, 0);
        for (std::size_t t = 0U; t < exact_counts.size(); ++t)
            _totalcases_by_dayofweek[t % 7U] += exact_counts[t];
    }
};

struct day_of_week_adjusted_rr_fixture {
    day_of_week_adjusted_rr_fixture() : runner(parameters, print) {}

    Parameters parameters;
    PrintScreen print{ true };
    DayOfWeekAdjustedRRRunner runner;
};

BOOST_FIXTURE_TEST_SUITE(day_of_week_adjusted_node_and_time_rr_suite, day_of_week_adjusted_rr_fixture)

BOOST_AUTO_TEST_CASE(calculates_mantel_haenszel_common_ratio_by_weekday) {
    runner.addNodeWithExactBranchCounts({ 4, 1, 2, 0, 0, 0, 0, 1, 3, 0, 0, 0, 0, 0 });
    runner.setExactTreeCounts({ 10, 5, 4, 1, 1, 1, 1, 10, 7, 2, 1, 1, 1, 1 });
    BOOST_CHECK_CLOSE(getDayOfWeekAdjustedNodeAndTimeRR(runner, 0, 0, 6), 2.1538461538461537, 0.000001);
}

BOOST_AUTO_TEST_CASE(calculates_mantel_haenszel_common_ratio_by_weekday_practical_test) {
    RandomNumberGenerator rng;
    const size_t intervals = 100;
    std::vector<int> nodecounts(intervals, 0);
    for (auto& c: nodecounts)
        c = Equilikely(0L, 5L, rng);
    runner.addNodeWithExactBranchCounts(nodecounts);
    std::vector<unsigned int> treecounts(intervals, 0);
    for (size_t t=0; t < nodecounts.size(); ++t)
        treecounts[t] = Equilikely(nodecounts[t], 10L, rng);
    runner.setExactTreeCounts(treecounts);

	// Calculate Mantel-Haenszel odds ratio by weekday for cluster in the first week (0-6).
    BOOST_CHECK_CLOSE(getDayOfWeekAdjustedNodeAndTimeRR(runner, 0, 0, 6), 1.2631384013815841, 0.000001);

    // Calculate Mantel-Haenszel odds ratio by weekday for cluster in the last week (93-99).
    BOOST_CHECK_CLOSE(getDayOfWeekAdjustedNodeAndTimeRR(runner, 0, 93, intervals - 1), 1.5004892764135693, 0.000001);

    // Calculate Mantel-Haenszel odds ratio by weekday for cluster in the last 3 days (97-99).
    BOOST_CHECK_CLOSE(getDayOfWeekAdjustedNodeAndTimeRR(runner, 0, 97, intervals - 1), 1.3683529563979815, 0.000001);

    // Calculate Mantel-Haenszel odds ratio by weekday for cluster in the last few weeks (76-99).
    BOOST_CHECK_CLOSE(getDayOfWeekAdjustedNodeAndTimeRR(runner, 0, 76, intervals - 1), 0.79900837534079738, 0.000001);

    // Calculate Mantel-Haenszel odds ratio by weekday for cluster in the last day (99-99).
    BOOST_CHECK_CLOSE(getDayOfWeekAdjustedNodeAndTimeRR(runner, 0, intervals - 1, intervals - 1), 0.39705882352941174, 0.000001);

    // Calculate Mantel-Haenszel odds ratio by weekday for cluster in the middle of range (retrospective) (40-70).
    BOOST_CHECK_CLOSE(getDayOfWeekAdjustedNodeAndTimeRR(runner, 0, 40, 70), 1.4255215852743046, 0.000001);
}

BOOST_AUTO_TEST_CASE(returns_nan_for_invalid_node_id) {
    runner.addNodeWithExactBranchCounts({ 1, 0, 0, 0, 0, 0, 0 });
    runner.setExactTreeCounts({ 1, 0, 0, 0, 0, 0, 0 });
    BOOST_CHECK_THROW(getDayOfWeekAdjustedNodeAndTimeRR(runner, 1, 0, 0), prg_error);
}

BOOST_AUTO_TEST_CASE(throws_when_window_is_outside_observed_times) {
    runner.addNodeWithExactBranchCounts({ 1, 0, 0, 0, 0, 0, 0 });
    runner.setExactTreeCounts({ 1, 0, 0, 0, 0, 0, 0 });
    BOOST_CHECK_THROW(getDayOfWeekAdjustedNodeAndTimeRR(runner, 0, 0, 7), prg_error);
}

BOOST_AUTO_TEST_CASE(returns_infinity_for_positive_numerator_and_zero_denominator) {
    runner.addNodeWithExactBranchCounts({ 2, 0, 0, 0, 0, 0, 0, 0 });
    runner.setExactTreeCounts({ 5, 0, 0, 0, 0, 0, 0, 5 });
    BOOST_CHECK_EQUAL(getDayOfWeekAdjustedNodeAndTimeRR(runner, 0, 0, 0), std::numeric_limits<double>::infinity());
}

BOOST_AUTO_TEST_CASE(returns_nan_when_marginals_are_inconsistent) {
    runner.addNodeWithExactBranchCounts({ 6, 0, 0, 0, 0, 0, 0 });
    runner.setExactTreeCounts({ 5, 0, 0, 0, 0, 0, 0 });
    BOOST_CHECK_THROW(getDayOfWeekAdjustedNodeAndTimeRR(runner, 0, 0, 0), prg_error);
}

BOOST_AUTO_TEST_SUITE_END()

class NodeAndTimeExcessCasesRunner : public ScanRunner {
public:
    NodeAndTimeExcessCasesRunner(const Parameters& parameters, BasePrint& print)
        : ScanRunner(parameters, print) {
        addNode();
    }

    NodeStructure& addNode() {
        NodeStructure* node = new NodeStructure("node", getParameters(), 2U);
        node->setID(static_cast<int>(_Nodes.size()));
        _Nodes.push_back(node);
        return *_Nodes.back();
    }
};

BOOST_AUTO_TEST_SUITE(node_and_time_excess_cases_suite)

BOOST_AUTO_TEST_CASE(uses_observed_minus_expected_for_node_and_time_conditioning) {
    Parameters parameters;
    parameters.setScanType(Parameters::TREETIME);
    parameters.setConditionalType(Parameters::NODEANDTIME);
    parameters.setModelType(Parameters::MODEL_NOT_APPLICABLE);

    PrintScreen print(true);
    NodeAndTimeExcessCasesRunner runner(parameters, print);

    const int observed = 10;
    const double expected = 4.25;
    const double excess = getExcessCasesFor(runner, 0, observed, expected, MatchedSets(), 0, 0);

    BOOST_CHECK_CLOSE(excess, 5.75, 0.000001);
}

BOOST_AUTO_TEST_SUITE_END()

