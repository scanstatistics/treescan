// Boost unit test header
#include <boost/test/unit_test.hpp>

#include "TreeScan.h"
#include "ChartGenerator.h"
#include "ScanRunner.h"
#include "PrintScreen.h"
#include "UtilityFunctions.h"
#include "RandomDistribution.h"

#include <memory>
#include <numeric>
#include <vector>

/** Grants unit tests access to the private helpers of TemporalChartGenerator. */
class TemporalChartGeneratorTester {
public:
    typedef TemporalChartGenerator::DayTally DayTally;
    typedef TemporalChartGenerator::NodeCasesDayTally_t NodeCasesDayTally_t;

    static NodeCasesDayTally_t getNodeCasesByDayOfWeek(const TemporalChartGenerator& generator, const CutStructure& cluster) {
        return generator.getNodeCasesByDayOfWeek(cluster);
    }
};

typedef TemporalChartGeneratorTester::DayTally DayTally;
typedef TemporalChartGeneratorTester::NodeCasesDayTally_t NodeCasesDayTally_t;

class DayOfWeekTallyRunner : public ScanRunner {
public:
    DayOfWeekTallyRunner(const Parameters& parameters, BasePrint& print) : ScanRunner(parameters, print) {}

    /** Adds node with exact (non-cumulative) internal and branch counts per day, then updates the total cases. */
    NodeStructure& addNode(const std::vector<int>& internal_counts, const std::vector<int>& branch_counts) {
        NodeStructure* node = new NodeStructure("node", getParameters(), internal_counts.size() + 1U);
        node->setID(static_cast<int>(_Nodes.size()));
        node->refIntC_C() = internal_counts;
        node->refIntC_C().push_back(0);
        TreeScan::cumulative_backward(node->refIntC_C());
        node->refBrC_C() = branch_counts;
        node->refBrC_C().push_back(0);
        TreeScan::cumulative_backward(node->refBrC_C());
        _TotalC = std::accumulate(internal_counts.begin(), internal_counts.end(), _TotalC);
        _Nodes.push_back(node);
        return *_Nodes.back();
    }

    /** Adds node without descendants - internal and branch counts are the same. */
    NodeStructure& addLeafNode(const std::vector<int>& counts) {
        return addNode(counts, counts);
    }
};

struct day_of_week_tally_fixture {
    PrintScreen print{ true };
    std::unique_ptr<DayOfWeekTallyRunner> runner;

    /** Creates tree-time scan runner with day precision data time range of 'numDays' days, starting on date 'start'. */
    DayOfWeekTallyRunner& makeRunner(boost::gregorian::date start, int numDays) {
        Parameters parameters;
        parameters.setScanType(Parameters::TREETIME);
        parameters.setDatePrecisionType(DataTimeRange::DAY);
        DataTimeRangeSet rangeSet;
        rangeSet.add(DataTimeRange(0, numDays - 1, start));
        parameters.setDataTimeRangeSet(rangeSet);
        runner.reset(new DayOfWeekTallyRunner(parameters, print));
        return *runner;
    }

    /** Returns day of week tallies for cluster on node 'nodeID'. Nodes must be added to runner prior to calling. */
    NodeCasesDayTally_t tally(int nodeID, int startIdx = 0, int endIdx = 0) const {
        CutStructure cluster;
        cluster.setID(nodeID);
        cluster.setStartIdx(startIdx);
        cluster.setEndIdx(endIdx);
        TemporalChartGenerator generator(*runner);
        return TemporalChartGeneratorTester::getNodeCasesByDayOfWeek(generator, cluster);
    }

    /** Ratio as reported in the 'Node By Day-Of-Week Interaction Effect' table of TemporalChartGenerator::generateChart(). */
    static double ratio(const DayTally& day) {
        return day._all_node_percent ? day._node_percent / day._all_node_percent : 0.0;
    }

    static void checkDay(const DayTally& day, const std::string& shortname, unsigned short ordinal,
                         unsigned int nodeCount, double nodeTotal, unsigned int allNodeCount, double allNodeTotal) {
        BOOST_TEST_CONTEXT("day " << shortname) {
            BOOST_CHECK_EQUAL(day._shortname, shortname);
            BOOST_CHECK_EQUAL(day._ordinal, ordinal);
            BOOST_CHECK_EQUAL(day._node_count, nodeCount);
            BOOST_CHECK_CLOSE(day._node_percent, static_cast<double>(nodeCount) / nodeTotal, 0.000001);
            BOOST_CHECK_EQUAL(day._all_node_count, allNodeCount);
            BOOST_CHECK_CLOSE(day._all_node_percent, static_cast<double>(allNodeCount) / allNodeTotal, 0.000001);
        }
    }
};

BOOST_FIXTURE_TEST_SUITE(temporal_chart_node_cases_by_day_of_week_suite, day_of_week_tally_fixture)

/* Data time range starting on a Monday, covering two full weeks, with cluster node and one other node. */
BOOST_AUTO_TEST_CASE(tallies_node_and_all_nodes_for_full_weeks_starting_monday) {
    makeRunner(boost::gregorian::date(2024, 1, 1), 14); // 2024/01/01 is a Monday
    runner->addLeafNode({ 3, 1, 0, 2, 0, 1, 1,   1, 0, 2, 0, 0, 1, 0 }); // 12 cases
    runner->addLeafNode({ 5, 4, 3, 2, 1, 3, 2,   5, 6, 1, 2, 3, 1, 2 }); // 40 cases

    NodeCasesDayTally_t days = tally(0);
    BOOST_REQUIRE_EQUAL(days.size(), 7U);
    checkDay(days[0], "Mon", 0, 4, 12.0, 14, 52.0);
    checkDay(days[1], "Tue", 1, 1, 12.0, 11, 52.0);
    checkDay(days[2], "Wed", 2, 2, 12.0, 6, 52.0);
    checkDay(days[3], "Thu", 3, 2, 12.0, 6, 52.0);
    checkDay(days[4], "Fri", 4, 0, 12.0, 4, 52.0);
    checkDay(days[5], "Sat", 5, 2, 12.0, 6, 52.0);
    checkDay(days[6], "Sun", 6, 1, 12.0, 5, 52.0);

    // Ratio of node percent to all nodes percent.
    BOOST_CHECK_CLOSE(ratio(days[0]), (4.0 / 12.0) / (14.0 / 52.0), 0.000001);
    BOOST_CHECK_CLOSE(ratio(days[2]), (2.0 / 12.0) / (6.0 / 52.0), 0.000001);
    BOOST_CHECK_EQUAL(ratio(days[4]), 0.0); // no node cases on Friday
}

/* Data time range starting mid-week - first day of data is not the first day reported (Monday). */
BOOST_AUTO_TEST_CASE(reorders_days_monday_through_sunday_when_range_starts_midweek) {
    makeRunner(boost::gregorian::date(2024, 1, 3), 7); // 2024/01/03 is a Wednesday
    runner->addLeafNode({ 1, 2, 3, 4, 5, 6, 7 }); // Wed, Thu, Fri, Sat, Sun, Mon, Tue

    NodeCasesDayTally_t days = tally(0);
    BOOST_REQUIRE_EQUAL(days.size(), 7U);
    checkDay(days[0], "Mon", 0, 6, 28.0, 6, 28.0);
    checkDay(days[1], "Tue", 1, 7, 28.0, 7, 28.0);
    checkDay(days[2], "Wed", 2, 1, 28.0, 1, 28.0);
    checkDay(days[3], "Thu", 3, 2, 28.0, 2, 28.0);
    checkDay(days[4], "Fri", 4, 3, 28.0, 3, 28.0);
    checkDay(days[5], "Sat", 5, 4, 28.0, 4, 28.0);
    checkDay(days[6], "Sun", 6, 5, 28.0, 5, 28.0);
    // Only one node in tree, so the node distribution equals the distribution of all nodes.
    for (const auto& day : days)
        BOOST_CHECK_CLOSE(ratio(day), 1.0, 0.000001);
}

/* Data time range starting on a Sunday - Sunday is reported last. */
BOOST_AUTO_TEST_CASE(reports_sunday_last_when_range_starts_sunday) {
    makeRunner(boost::gregorian::date(2023, 12, 31), 7); // 2023/12/31 is a Sunday
    runner->addLeafNode({ 7, 1, 2, 3, 4, 5, 6 }); // Sun, Mon, Tue, Wed, Thu, Fri, Sat

    NodeCasesDayTally_t days = tally(0);
    BOOST_REQUIRE_EQUAL(days.size(), 7U);
    checkDay(days[0], "Mon", 0, 1, 28.0, 1, 28.0);
    checkDay(days[5], "Sat", 5, 6, 28.0, 6, 28.0);
    checkDay(days[6], "Sun", 6, 7, 28.0, 7, 28.0);
}

/* Data time range which isn't a multiple of 7 days - some days of the week occur more often than others. */
BOOST_AUTO_TEST_CASE(tallies_partial_week) {
    makeRunner(boost::gregorian::date(2024, 1, 5), 10); // 2024/01/05 is a Friday
    runner->addLeafNode({ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }); // Fri - Thu, then Fri, Sat, Sun again

    NodeCasesDayTally_t days = tally(0);
    BOOST_REQUIRE_EQUAL(days.size(), 7U);
    checkDay(days[0], "Mon", 0, 1, 10.0, 1, 10.0);
    checkDay(days[1], "Tue", 1, 1, 10.0, 1, 10.0);
    checkDay(days[2], "Wed", 2, 1, 10.0, 1, 10.0);
    checkDay(days[3], "Thu", 3, 1, 10.0, 1, 10.0);
    checkDay(days[4], "Fri", 4, 2, 10.0, 2, 10.0);
    checkDay(days[5], "Sat", 5, 2, 10.0, 2, 10.0);
    checkDay(days[6], "Sun", 6, 2, 10.0, 2, 10.0);
}

/* Data time range shorter than a week - all 7 days are still reported, days without data have zero values. */
BOOST_AUTO_TEST_CASE(reports_all_days_when_range_shorter_than_week) {
    makeRunner(boost::gregorian::date(2024, 1, 1), 3); // Mon, Tue, Wed
    runner->addLeafNode({ 2, 1, 1 });
    runner->addLeafNode({ 2, 3, 1 });

    NodeCasesDayTally_t days = tally(0);
    BOOST_REQUIRE_EQUAL(days.size(), 7U);
    checkDay(days[0], "Mon", 0, 2, 4.0, 4, 10.0);
    checkDay(days[1], "Tue", 1, 1, 4.0, 4, 10.0);
    checkDay(days[2], "Wed", 2, 1, 4.0, 2, 10.0);
    checkDay(days[3], "Thu", 3, 0, 4.0, 0, 10.0);
    checkDay(days[4], "Fri", 4, 0, 4.0, 0, 10.0);
    checkDay(days[5], "Sat", 5, 0, 4.0, 0, 10.0);
    checkDay(days[6], "Sun", 6, 0, 4.0, 0, 10.0);
    BOOST_CHECK_CLOSE(ratio(days[0]), (2.0 / 4.0) / (4.0 / 10.0), 0.000001);
    // Ratio is reported as zero when there are no cases in any node on that day.
    for (size_t d = 3; d < days.size(); ++d)
        BOOST_CHECK_EQUAL(ratio(days[d]), 0.0);
}

/* Cluster node tallies use branch counts (node and descendants), while all nodes tallies use internal counts of every node. */
BOOST_AUTO_TEST_CASE(uses_branch_counts_for_cluster_node) {
    makeRunner(boost::gregorian::date(2024, 1, 1), 7);
    std::vector<int> child1 = { 1, 0, 2, 0, 1, 0, 0 };
    std::vector<int> child2 = { 0, 3, 0, 1, 0, 2, 1 };
    std::vector<int> parentBranch(7);
    for (size_t i = 0; i < parentBranch.size(); ++i)
        parentBranch[i] = child1[i] + child2[i];
    runner->addNode(std::vector<int>(7, 0), parentBranch); // node 0: parent without internal cases
    runner->addLeafNode(child1);                           // node 1
    runner->addLeafNode(child2);                           // node 2

    // Cluster on parent node - node distribution is the distribution of all nodes.
    NodeCasesDayTally_t days = tally(0);
    BOOST_REQUIRE_EQUAL(days.size(), 7U);
    for (size_t d = 0; d < days.size(); ++d) {
        BOOST_CHECK_EQUAL(days[d]._node_count, static_cast<unsigned int>(parentBranch[d]));
        BOOST_CHECK_EQUAL(days[d]._all_node_count, static_cast<unsigned int>(parentBranch[d]));
        if (parentBranch[d]) BOOST_CHECK_CLOSE(ratio(days[d]), 1.0, 0.000001);
    }

    // Cluster on child node - node counts are from child only, all nodes counts unchanged.
    days = tally(1);
    BOOST_REQUIRE_EQUAL(days.size(), 7U);
    for (size_t d = 0; d < days.size(); ++d) {
        BOOST_CHECK_EQUAL(days[d]._node_count, static_cast<unsigned int>(child1[d]));
        BOOST_CHECK_CLOSE(days[d]._node_percent, child1[d] / 4.0, 0.000001);
        BOOST_CHECK_EQUAL(days[d]._all_node_count, static_cast<unsigned int>(parentBranch[d]));
        BOOST_CHECK_CLOSE(days[d]._all_node_percent, parentBranch[d] / 11.0, 0.000001);
    }
}

/* Tallies are over the entire study period - the cluster's temporal window has no effect. Percentages sum to one. */
BOOST_AUTO_TEST_CASE(tallies_entire_study_period_regardless_of_cluster_window) {
    const int numDays = 60;
    makeRunner(boost::gregorian::date(2024, 2, 14), numDays);
    RandomNumberGenerator rng;
    std::vector<int> nodeCounts(numDays), otherCounts(numDays);
    for (int i = 0; i < numDays; ++i) {
        nodeCounts[i] = Equilikely(0L, 5L, rng);
        otherCounts[i] = Equilikely(0L, 10L, rng);
    }
    runner->addLeafNode(nodeCounts);
    runner->addLeafNode(otherCounts);

    NodeCasesDayTally_t fullWindow = tally(0, 0, numDays - 1);
    NodeCasesDayTally_t firstWeek = tally(0, 0, 6);
    NodeCasesDayTally_t lastDay = tally(0, numDays - 1, numDays - 1);
    BOOST_REQUIRE_EQUAL(fullWindow.size(), 7U);
    BOOST_REQUIRE_EQUAL(firstWeek.size(), 7U);
    BOOST_REQUIRE_EQUAL(lastDay.size(), 7U);

    unsigned int nodeTotal = 0, allNodeTotal = 0;
    double nodePercentTotal = 0.0, allNodePercentTotal = 0.0;
    for (size_t d = 0; d < fullWindow.size(); ++d) {
        BOOST_CHECK_EQUAL(fullWindow[d]._ordinal, d);
        BOOST_CHECK_EQUAL(fullWindow[d]._node_count, firstWeek[d]._node_count);
        BOOST_CHECK_EQUAL(fullWindow[d]._node_count, lastDay[d]._node_count);
        BOOST_CHECK_EQUAL(fullWindow[d]._all_node_count, firstWeek[d]._all_node_count);
        BOOST_CHECK_EQUAL(fullWindow[d]._all_node_count, lastDay[d]._all_node_count);
        nodeTotal += fullWindow[d]._node_count;
        allNodeTotal += fullWindow[d]._all_node_count;
        nodePercentTotal += fullWindow[d]._node_percent;
        allNodePercentTotal += fullWindow[d]._all_node_percent;
    }
    BOOST_CHECK_EQUAL(nodeTotal, static_cast<unsigned int>(runner->getNodes()[0]->getBrC()));
    BOOST_CHECK_EQUAL(allNodeTotal, static_cast<unsigned int>(runner->getTotalC()));
    BOOST_CHECK_CLOSE(nodePercentTotal, 1.0, 0.000001);
    BOOST_CHECK_CLOSE(allNodePercentTotal, 1.0, 0.000001);
}

BOOST_AUTO_TEST_SUITE_END()
