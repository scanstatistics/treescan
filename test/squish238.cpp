#include "fixture_examples.h"
#include "SignedRankRandomizer.h"
#include "Randomization.h"
#include "ScanRunner.h"
#include "SampleSiteData.h"
#include "Parameters.h"
#include "BasePrint.h"
#include "RandomNumberGenerator.h"
#include "Loglikelihood.h"
#include "MonteCarloSimFunctor.h"

#include <string>
#include <vector>
#include <fstream>
#include <cstdio>
#include <memory>
#include <mutex>
#include <cmath>

// Test helper: minimal ScanRunner subclass that allows controlled construction of nodes
class TestScanRunner : public ScanRunner {
public:
    TestScanRunner(const Parameters& params, BasePrint& print) : ScanRunner(params, print) {}

    // Expose a way to set sample-site identifiers used by constructor
    void setSampleSiteIdentifiers(const std::vector<std::string>& ids) {
        _sample_site_identifiers = ids;
    }

    // Add a node with given identifier and returns pointer to the created NodeStructure.
    // container_size is set to 1 (enough for SimulationNode internals used in these tests).
    NodeStructure* addNode(const std::string& identifier, unsigned int sample_sites = 0) {
        // NodeStructure constructor (identifier, parameters, container_size)
        NodeStructure* pnode = new NodeStructure(identifier, getParameters(), 1);
        pnode->setID(static_cast<int>(_Nodes.size())); // assign sequential ID
        // ensure sample site containers present if requested by test
        if (sample_sites) {
            // initialize sample-site maps empty; tests will populate refSampleSiteDataBr()
        }
        _Nodes.push_back(pnode);
        return pnode;
    }

    // Public wrapper to invoke protected scanTreeSignedRank for unit testing
    bool invoke_scanTreeSignedRank() {
        return scanTreeSignedRank();
    }
};

// Test wrapper subclass to expose protected read/write/randomize/addSimDiffs for unit testing
class TestSignedRankRandomizer : public SignedRankRandomizer {
public:
    TestSignedRankRandomizer(const ScanRunner& scanner, long seed = RandomNumberGenerator::glDefaultSeed)
        : SignedRankRandomizer(scanner, seed) {}

    // Public wrappers to call protected methods for unit testing
    int invoke_randomize(unsigned int iSimulation, const AbstractNodesProxy& proxy, SimNodeContainer_t& simNodes) {
        return randomize(iSimulation, proxy, simNodes);
    }
    void invoke_addSimDiffs(size_t target_id, const SimulationNode::SampleSiteDiff_t& diffs, SimNodeContainer_t& treeSimNodes, const ScanRunner::NodeStructureContainer_t& treeNodes) {
        addSimDiffs(target_id, diffs, treeSimNodes, treeNodes);
    }
    int invoke_read(const std::string& filename, unsigned int simulation, const ScanRunner::NodeStructureContainer_t& treeNodes, SimNodeContainer_t& treeSimNodes, boost::mutex& mutex) {
        return read(filename, simulation, treeNodes, treeSimNodes, mutex);
    }
    void invoke_write(const std::string& filename, const SimNodeContainer_t& treeSimNodes) {
        write(filename, treeSimNodes);
    }
};

// Test wrapper to expose protected `MCSimSuccessiveFunctor::scanTreeSignedRank` for unit testing.
class TestMCSimSuccessiveFunctor : public MCSimSuccessiveFunctor {
public:
    TestMCSimSuccessiveFunctor(boost::mutex& mutex, boost::shared_ptr<AbstractRandomizer> randomizer, const ScanRunner& scanRunner)
        : MCSimSuccessiveFunctor(mutex, randomizer, scanRunner) {}

    MCSimSuccessiveFunctor::successful_result_type invoke_scanTreeSignedRank(MCSimSuccessiveFunctor::param_type const& param) {
        return scanTreeSignedRank(param);
    }

    void setTreeSimNodes(SimNodeContainer_t& simNodes) {
        _treeSimNodes = simNodes;
    }
};

// Helper to create a SimulationNode container sized for N nodes, each with sample_sites slots.
static SimNodeContainer_t makeSimNodes(size_t num_nodes, unsigned int sample_sites) {
    SimNodeContainer_t v;
    v.reserve(num_nodes);
    for (size_t i = 0; i < num_nodes; ++i)
        v.emplace_back(1u, 1u, sample_sites); // container_size=1, level=1, sample_sites
    return v;
}

/* Tests for `SignedRankRandomizer` behaviour:
   - preserving magnitudes and applying random signs
   - propagation of simulated differences up the tree
   - serialization (write/read) round-trip of simulation data
   - ensuring branch sums update end-to-end via `RandomizeData`
*/
BOOST_AUTO_TEST_SUITE(signed_rank_randomize_suite)

BOOST_AUTO_TEST_CASE(PreservesMagnitudesAndAppliesSigns)
{
    Parameters params;
    PrintNull print(true);
    TestScanRunner runner(params, print);

    // Two sample sites total. Tell runner about identifiers (used by randomizer ctor).
    runner.setSampleSiteIdentifiers(std::vector<std::string>{"SS1", "SS2"});

    // Create two leaf nodes (both are leaves since no parent/child relationships)
    NodeStructure* n0 = runner.addNode("Node0", 2);
    NodeStructure* n1 = runner.addNode("Node1", 2);

    // Populate branch sample-site maps (getSampleSiteDataBr)
    // use baseline/current values so difference() = current - baseline
    n0->refSampleSiteDataBr().emplace(0u, SampleSiteData(0.0, 1.0)); // diff = 1.0
    n0->refSampleSiteDataBr().emplace(1u, SampleSiteData(0.0, 2.0)); // diff = 2.0
    n1->refSampleSiteDataBr().emplace(0u, SampleSiteData(0.0, 3.0)); // diff = 3.0
    n1->refSampleSiteDataBr().emplace(1u, SampleSiteData(0.0, 4.0)); // diff = 4.0

    // Instantiate real randomizer using real ScanRunner (our TestScanRunner)
    TestSignedRankRandomizer randomizer(runner, 12345);

    // Create simulation node container matching node count and sample site count
    SimNodeContainer_t simNodes = makeSimNodes(2, 2);

    // Use real NodesProxy to expose the tree to randomizer (data_only_on_leaves defaults in params)
    NodesProxy proxy(runner.getNodes(), params.getDataOnlyOnLeaves());

    // Run randomize (deterministic based on seed and simulation index)
    int total = randomizer.invoke_randomize(1u, proxy, simNodes);
    BOOST_TEST(total == 0); // randomize returns 0 in implementation

    // Check absolute magnitudes preserved and signs applied (at least one negative expected)
    bool found_negative = false;
    // node 0
    BOOST_TEST(std::abs(simNodes[0].getSampleSiteDifferences()[0]) == 1.0);
    BOOST_TEST(std::abs(simNodes[0].getSampleSiteDifferences()[1]) == 2.0);
    if (simNodes[0].getSampleSiteDifferences()[0] < 0 || simNodes[0].getSampleSiteDifferences()[1] < 0) found_negative = true;
    // node 1
    BOOST_TEST(std::abs(simNodes[1].getSampleSiteDifferences()[0]) == 3.0);
    BOOST_TEST(std::abs(simNodes[1].getSampleSiteDifferences()[1]) == 4.0);
    if (simNodes[1].getSampleSiteDifferences()[0] < 0 || simNodes[1].getSampleSiteDifferences()[1] < 0) found_negative = true;

    BOOST_TEST(found_negative);
}

BOOST_AUTO_TEST_CASE(AddSimDiffs_PropagatesToParents)
{
    Parameters params;
    PrintNull print(true);
    TestScanRunner runner(params, print);
    runner.setSampleSiteIdentifiers(std::vector<std::string>{"SS1","SS2"});

    // Create three nodes: parent (0) and two children (1,2). We'll propagate diffs from child 2 up to parent 0.
    NodeStructure* parent = runner.addNode("Parent", 2);
    NodeStructure* child1 = runner.addNode("Child1", 2);
    NodeStructure* child2 = runner.addNode("Child2", 2);

    // Link children to parent via addAsParent (this will populate parents/children lists)
    child1->addAsParent(*parent, "");
    child2->addAsParent(*parent, "");

    // Prepare simulation nodes (3 nodes)
    SimNodeContainer_t simNodes = makeSimNodes(3, 2);

    // Prepare diffs at child2
    SimulationNode::SampleSiteDiff_t diffs = { 1.5, -2.5 };
    // set these diffs into the sample-site differences for child2 as if produced by randomize
    simNodes[2].refSampleSiteDifferences() = diffs;

    TestSignedRankRandomizer randomizer(runner, 12345);

    // Invoke addSimDiffs for target=2 (child2). It should add to child's branch array and propagate to parent.
    randomizer.invoke_addSimDiffs(2, diffs, simNodes, runner.getNodes());

    // child2 branch should have the diffs
    BOOST_TEST(simNodes[2].getSampleSiteDifferencesBr()[0] == 1.5);
    BOOST_TEST(simNodes[2].getSampleSiteDifferencesBr()[1] == -2.5);

    // parent should have accumulated the same diffs (propagated from child2)
    // parent index is 0 (we assigned IDs sequentially)
    bool parent_received = (simNodes[0].getSampleSiteDifferencesBr()[0] == 1.5 && simNodes[0].getSampleSiteDifferencesBr()[1] == -2.5);
    BOOST_TEST(parent_received);
}

BOOST_AUTO_TEST_CASE(WriteAndRead_RoundTrip)
{
    Parameters params;
    PrintNull print(true);
    TestScanRunner runner(params, print);
    runner.setSampleSiteIdentifiers(std::vector<std::string>{"SS1","SS2"});

    // Create two nodes
    runner.addNode("A", 2);
    runner.addNode("B", 2);

    // Prepare simulation nodes and populate values
    SimNodeContainer_t simNodes = makeSimNodes(2, 2);
    simNodes[0].refSampleSiteDifferences() = { 1.1, -2.2 };
    simNodes[1].refSampleSiteDifferences() = { 3.3, 4.4 };

    TestSignedRankRandomizer randomizer(runner, 12345);

    // Create temporary filename
    std::string filename = "SignedRankTest_simdata.tmp";
    // Ensure no leftover file
    std::remove(filename.c_str());

    // Write data
    randomizer.invoke_write(filename, simNodes);

    // Prepare fresh container to read into
    SimNodeContainer_t loaded = makeSimNodes(2, 2);
    boost::mutex mtx;
    int rc = randomizer.invoke_read(filename, 1u, runner.getNodes(), loaded, mtx);
    BOOST_TEST(rc == 0);

    // Verify round-trip values
    BOOST_TEST(loaded[0].getSampleSiteDifferences()[0] == simNodes[0].refSampleSiteDifferences()[0]);
    BOOST_TEST(loaded[0].getSampleSiteDifferences()[1] == simNodes[0].refSampleSiteDifferences()[1]);
    BOOST_TEST(loaded[1].getSampleSiteDifferences()[0] == simNodes[1].refSampleSiteDifferences()[0]);
    BOOST_TEST(loaded[1].getSampleSiteDifferences()[1] == simNodes[1].refSampleSiteDifferences()[1]);

    // cleanup
    std::remove(filename.c_str());
}

BOOST_AUTO_TEST_CASE(RandomizeData_UpdatesBranchSums)
{
    Parameters params;
    PrintNull print(true);
    TestScanRunner runner(params, print);
    // single sample site
    runner.setSampleSiteIdentifiers(std::vector<std::string>{"SS1"});

    // Build simple tree: root(0) parent of leaf(1)
    NodeStructure* root = runner.addNode("Root", 1);
    NodeStructure* leaf = runner.addNode("Leaf", 1);
    leaf->addAsParent(*root, "");

    // set sample-site data on leaf only
    leaf->refSampleSiteDataBr().emplace(0u, SampleSiteData(0.0, 7.0)); // difference 7.0

    // Prepare simulation nodes (2)
    SimNodeContainer_t simNodes = makeSimNodes(2, 1);

    TestSignedRankRandomizer randomizer(runner, 999);

    // Call RandomizeData which will call randomize() and then addSimDiffs across tree
    boost::mutex mtx;
    int rc = randomizer.RandomizeData(1u, runner.getNodes(), mtx, simNodes);
    BOOST_TEST(rc == 0);

    // After RandomizeData, branch sums should reflect propagation: root.refSampleSiteDifferencesBr should equal leaf's differences (maybe sign-flipped)
    double leaf_val = simNodes[1].getSampleSiteDifferences()[0];
    double root_br_val = simNodes[0].getSampleSiteDifferencesBr()[0];
    BOOST_TEST(std::abs(root_br_val) == std::abs(leaf_val));
}

BOOST_AUTO_TEST_SUITE_END()

/* Tests for `ScanRunner::scanTreeSignedRank()`:
   - basic creation of `CutStructure` objects from sample-site data
   - ensuring root nodes are skipped when appropriate
*/
BOOST_AUTO_TEST_SUITE(signed_rank_scantree)

BOOST_AUTO_TEST_CASE(BasicCreatesCuts)
{
    Parameters params;
    // Ensure parameters configured for signed-rank tree scan
    params.setModelType(Parameters::SIGNED_RANK);
    params.setScanType(Parameters::TREEONLY);

    PrintNull print(true);
    TestScanRunner runner(params, print);

    // Two sample sites total. Tell runner about identifiers (used by scan implementation).
    runner.setSampleSiteIdentifiers(std::vector<std::string>{"SS1", "SS2"});

    // Create two leaf nodes (both are leaves)
    NodeStructure* n0 = runner.addNode("Node0", 2);
    NodeStructure* n1 = runner.addNode("Node1", 2);

    // Populate sample-site data (model: baseline, current)
    n0->refSampleSiteDataBr().emplace(0u, SampleSiteData(0.0, 1.0)); // diff 1.0
    n0->refSampleSiteDataBr().emplace(1u, SampleSiteData(0.0, 2.0)); // diff 2.0
    n1->refSampleSiteDataBr().emplace(0u, SampleSiteData(0.0, 3.0)); // diff 3.0
    n1->refSampleSiteDataBr().emplace(1u, SampleSiteData(0.0, 4.0)); // diff 4.0

    // Invoke the scanTreeSignedRank method via public wrapper
    bool ok = runner.invoke_scanTreeSignedRank();
    BOOST_TEST(ok == true);

    // Expect at least one cut created for the provided sample-site data
    auto const& cuts = runner.getCuts();
    BOOST_TEST(cuts.size() > 0u);

    // Cut should contain sample-site data and define a test statistic greater than zero
    for (auto const& cut : cuts) {
        BOOST_TEST(cut->getSampleSiteData().size() > 0);
        BOOST_TEST(cut->getLogLikelihood() > 0);
    }
}

BOOST_AUTO_TEST_CASE(SkipsRootNodes)
{
    Parameters params;
    params.setModelType(Parameters::SIGNED_RANK);
    params.setScanType(Parameters::TREEONLY);

    PrintNull print(true);
    TestScanRunner runner(params, print);

    // single sample site
    runner.setSampleSiteIdentifiers(std::vector<std::string>{"SS1"});

    // Build simple tree: root(0) parent of leaf(1)
    NodeStructure* root = runner.addNode("Root", 1);
    NodeStructure* leaf = runner.addNode("Leaf", 1);
    leaf->addAsParent(*root, "");

    // add sample-site data only to leaf
    leaf->refSampleSiteDataBr().emplace(0u, SampleSiteData(0.0, 7.0)); // diff 7.0

    bool ok = runner.invoke_scanTreeSignedRank();
    BOOST_TEST(ok == true);

    // Cuts should include leaf node but not root node (root nodes are skipped for signed-rank evaluation)
    bool found_root_cut = false;
    bool found_leaf_cut = false;
    for (auto const& cut : runner.getCuts()) {
        if (cut->getID() == root->getID()) found_root_cut = true;
        if (cut->getID() == leaf->getID()) found_leaf_cut = true;
    }
    BOOST_TEST(found_leaf_cut);
    BOOST_TEST(!found_root_cut);
}

BOOST_AUTO_TEST_CASE(CutTypes_CreateVariousCuts)
{
    // This test creates a parent with four children and checks that
    // scanTreeSignedRank generates cuts for ORDINAL, PAIRS and TRIPLETS.
    Parameters params;
    params.setModelType(Parameters::SIGNED_RANK);
    params.setScanType(Parameters::TREEONLY);

    PrintNull print(true);

    // We'll run three independent runner setups (one per cut type) to avoid state carry-over.
    for (Parameters::CutType ct : { Parameters::ORDINAL, Parameters::PAIRS, Parameters::TRIPLETS }) {
        TestScanRunner runner(params, print);
        // Two sample sites per child to keep data simple.
        runner.setSampleSiteIdentifiers(std::vector<std::string>{"SS1", "SS2"});

        // Create children first so their IDs are contiguous starting at 0.
        NodeStructure* c0 = runner.addNode("Child0", 2);
        NodeStructure* c1 = runner.addNode("Child1", 2);
        NodeStructure* c2 = runner.addNode("Child2", 2);
        NodeStructure* c3 = runner.addNode("Child3", 2);

        // Populate sample-site branch data for children (baseline, current) to yield non-zero differences.
        c0->refSampleSiteDataBr().emplace(0u, SampleSiteData(0.0, 1.0)); c0->refSampleSiteDataBr().emplace(1u, SampleSiteData(0.0, -10.0));
        c1->refSampleSiteDataBr().emplace(0u, SampleSiteData(0.0, 2.0)); c1->refSampleSiteDataBr().emplace(1u, SampleSiteData(0.0, 20.0));
        c2->refSampleSiteDataBr().emplace(0u, SampleSiteData(0.0, 3.0)); c2->refSampleSiteDataBr().emplace(1u, SampleSiteData(0.0, 30.0));
        c3->refSampleSiteDataBr().emplace(0u, SampleSiteData(0.0, 4.0)); c3->refSampleSiteDataBr().emplace(1u, SampleSiteData(0.0, -40.0));

        // Add parent after children
        NodeStructure* parent = runner.addNode("Parent", 2);

        // Link children to parent
        c0->addAsParent(*parent, "");
        c1->addAsParent(*parent, "");
        c2->addAsParent(*parent, "");
        c3->addAsParent(*parent, "");

        // Set the parent's cut type for this iteration.
        parent->setCutType(ct);

        // Invoke scan and expect at least one cut for the parent.
        bool ok = runner.invoke_scanTreeSignedRank();
        BOOST_TEST(ok == true);

        bool found_parent_cut = false;
        for (auto const& cut : runner.getCuts()) {
            if (cut->getID() == parent->getID()) {
                found_parent_cut = true;
                // cut should contain sample-site data and a positive test statistic
                BOOST_TEST(cut->getSampleSiteData().size() > 0);
                BOOST_TEST(cut->getLogLikelihood() > 0);
                BOOST_TEST(cut->getCutChildren().size() > 0);
            } else {
                BOOST_TEST(cut->getCutChildren().size() == 0);
            }
        }
        BOOST_TEST(found_parent_cut);
    }
}

BOOST_AUTO_TEST_SUITE_END()

/* Tests for `SignedRankLoglikelihood`:
   - behaviour on all-zero inputs
   - ranking, tied ranks and ignoring zeros
*/
BOOST_AUTO_TEST_SUITE(signed_rank_loglikelihood)

BOOST_AUTO_TEST_CASE(LogLikelihood_AllZeros)
{
    Parameters params;
    SignedRankLoglikelihood ll(params, 3u);
    std::vector<double> diffs = { 0.0, 0.0, 0.0 };
    SampleSiteVectorDifferenceProxy proxy(diffs);
    double res = ll.LogLikelihoodRatio(proxy);
    BOOST_TEST(res == 0);
}

BOOST_AUTO_TEST_CASE(LogLikelihood_SimpleRanks)
{
    Parameters params;
    // create instance sized to number of sample sites
    SignedRankLoglikelihood ll(params, 2u);
    std::vector<double> diffs = { 1.0, -2.0 }; // ranks 1 and 2 => llr = 1 - 2 = -1
    SampleSiteVectorDifferenceProxy proxy(diffs);
    double res = ll.LogLikelihoodRatio(proxy);
    BOOST_TEST(res == -1);
}

BOOST_AUTO_TEST_CASE(LogLikelihood_Ties)
{
    Parameters params;
    SignedRankLoglikelihood ll(params, 3u);
    // abs values: [1,1,2] => tied first two get average rank (1+2)/2 = 1.5
    // contributions: +1.5, -1.5, +3 => total 3
    std::vector<double> diffs = { 1.0, -1.0, 2.0 };
    SampleSiteVectorDifferenceProxy proxy(diffs);
    double res = ll.LogLikelihoodRatio(proxy);
    BOOST_TEST(res == 3);
}

BOOST_AUTO_TEST_CASE(LogLikelihood_WithZeroIgnored)
{
    Parameters params;
    SignedRankLoglikelihood ll(params, 3u);
    // zero is ignored; remaining abs [1,2] => ranks 1 and 2; contributions -1 and +2 => total 1
    std::vector<double> diffs = { 0.0, -1.0, 2.0 };
    SampleSiteVectorDifferenceProxy proxy(diffs);
    double res = ll.LogLikelihoodRatio(proxy);
    BOOST_TEST(res == 1);
}

BOOST_AUTO_TEST_SUITE_END()

/* Tests for `SampleSiteData` helpers:
   - baseline/current/difference semantics
   - accumulation via `add()`
   - `getAverage()` and `combine()` helpers
*/
BOOST_AUTO_TEST_SUITE(signed_rank_sample_site_data)

BOOST_AUTO_TEST_CASE(SampleSiteData_BaselineCurrentDifferenceAndEquality)
{
    SampleSiteData a(1.5, 3.0);
    BOOST_TEST(a.baseline() == 1.5);
    BOOST_TEST(a.current() == 3.0);
    BOOST_TEST(a.difference() == 1.5);
    SampleSiteData b(1.5, 3.0);
    BOOST_TEST(a == b);
}

BOOST_AUTO_TEST_CASE(SampleSiteData_AddAccumulates)
{
    SampleSiteData a(1.0, 2.0);
    SampleSiteData b(0.5, 0.25);
    a.add(b);
    BOOST_TEST(a.baseline() == 1.5);
    BOOST_TEST(a.current() == 2.25);
}

BOOST_AUTO_TEST_CASE(GetAverage_ComputesBaselineAndCurrentMeans)
{
    SampleSiteMap_t map;
    map.emplace(0u, SampleSiteData(1.0, 2.0));
    map.emplace(1u, SampleSiteData(3.0, 5.0));
    auto avg = getAverage(map);
    BOOST_TEST(avg.first == 2.0);
    BOOST_TEST(avg.second == 3.5);
}

BOOST_AUTO_TEST_CASE(Combine_MergesAndAccumulatesEntries)
{
    SampleSiteMap_t acc;
    acc.emplace(0u, SampleSiteData(1.0, 1.0));
    SampleSiteMap_t src;
    src.emplace(0u, SampleSiteData(2.0, 3.0));
    src.emplace(1u, SampleSiteData(4.0, 5.0));

    // non-clearing combine: should accumulate key 0 and add key 1
    combine(acc, src, false);
    BOOST_TEST(acc.size() == 2u);
    BOOST_TEST(acc[0].baseline() == 3.0);
    BOOST_TEST(acc[0].current() == 4.0);
    BOOST_TEST(acc[1].baseline() == 4.0);
    BOOST_TEST(acc[1].current() == 5.0);

    // clearing combine: should replace accumulation with src
    SampleSiteMap_t acc2;
    acc2.emplace(0u, SampleSiteData(9.0, 9.0));
    combine(acc2, src, true);
    BOOST_TEST(acc2.size() == src.size());
    BOOST_TEST(acc2[0].baseline() == 2.0);
    BOOST_TEST(acc2[0].current() == 3.0);
    BOOST_TEST(acc2[1].baseline() == 4.0);
    BOOST_TEST(acc2[1].current() == 5.0);
}

BOOST_AUTO_TEST_SUITE_END()

/* Tests for MCSimSuccessiveFunctor::scanTreeSignedRank():
   - basic Monte Carlo functor invocation produces a non-negative LLR
   - operator() wraps execution and reports unexceptional result
*/
BOOST_AUTO_TEST_SUITE(mcsim_successive_signed_rank)

BOOST_AUTO_TEST_CASE(Functor_BasicProducesLLR)
{
    Parameters params;
    params.setModelType(Parameters::SIGNED_RANK);
    params.setScanType(Parameters::TREEONLY);

    PrintNull print(true);
    TestScanRunner runner(params, print);

    // Two sample sites total. Tell runner about identifiers.
    runner.setSampleSiteIdentifiers(std::vector<std::string>{"SS1", "SS2", "SS3", "SS4", "SS5", "SS6", "SS7"});

    // Create two leaf nodes and populate sample-site branch data
    NodeStructure* n0 = runner.addNode("Node0", 7);
    NodeStructure* n1 = runner.addNode("Node1", 7);
    n0->refSampleSiteDataBr().emplace(0u, SampleSiteData(0.0, 1.0));
    n0->refSampleSiteDataBr().emplace(1u, SampleSiteData(0.0, 2.0));
    n0->refSampleSiteDataBr().emplace(2u, SampleSiteData(0.0, 3.0));
    n0->refSampleSiteDataBr().emplace(3u, SampleSiteData(0.0, 4.0));
    n0->refSampleSiteDataBr().emplace(4u, SampleSiteData(0.0, 5.0));
    n0->refSampleSiteDataBr().emplace(5u, SampleSiteData(0.0, 6.0));
    n0->refSampleSiteDataBr().emplace(6u, SampleSiteData(0.0, -7.0));
    n1->refSampleSiteDataBr().emplace(0u, SampleSiteData(0.0, 3.0));
    n1->refSampleSiteDataBr().emplace(1u, SampleSiteData(0.0, 4.0));
    n1->refSampleSiteDataBr().emplace(2u, SampleSiteData(0.0, 5.0));
    n1->refSampleSiteDataBr().emplace(3u, SampleSiteData(0.0, 6.0));
    n1->refSampleSiteDataBr().emplace(4u, SampleSiteData(0.0, -7.0));
    n1->refSampleSiteDataBr().emplace(5u, SampleSiteData(0.0, 8.0));
    n1->refSampleSiteDataBr().emplace(6u, SampleSiteData(0.0, -9.0));

    // Prepare functor with TestSignedRankRandomizer
    boost::mutex mtx;
    boost::shared_ptr<AbstractRandomizer> rnd(new TestSignedRankRandomizer(runner, 12345));
    TestMCSimSuccessiveFunctor functor(mtx, rnd, runner);

    auto res = functor.invoke_scanTreeSignedRank(1u);
    // Expect the pre-computed result for this sample arrangement
    BOOST_TEST(res.first == 4);
    BOOST_TEST(res.second == 0);
}

BOOST_AUTO_TEST_CASE(Functor_Operator_UnExceptional)
{
    Parameters params;
    params.setModelType(Parameters::SIGNED_RANK);
    params.setScanType(Parameters::TREEONLY);

    PrintNull print(true);
    TestScanRunner runner(params, print);

    // single sample site; root + leaf
    runner.setSampleSiteIdentifiers(std::vector<std::string>{"SS1"});
    NodeStructure* root = runner.addNode("Root", 1);
    NodeStructure* leaf = runner.addNode("Leaf", 1);
    leaf->addAsParent(*root, "");
    leaf->refSampleSiteDataBr().emplace(0u, SampleSiteData(0.0, 7.0));

    boost::mutex mtx;
    boost::shared_ptr<AbstractRandomizer> rnd(new TestSignedRankRandomizer(runner, 999));
    MCSimSuccessiveFunctor functor(mtx, rnd, runner);

    auto opres = functor(1u); // calls operator()
    BOOST_TEST(opres.bUnExceptional);
    BOOST_TEST(opres.dSuccessfulResult.first >= 0.0);
}

BOOST_AUTO_TEST_SUITE_END()
