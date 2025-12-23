//******************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//******************************************************************************
#include "SignedRankRandomizer.h"

/** Constructor */
SignedRankRandomizer::SignedRankRandomizer(const ScanRunner& scanner, long lInitialSeed)
    :AbstractRandomizer(scanner.getParameters(), scanner.getMultiParentNodesExist(), lInitialSeed), _scanner(scanner){
    _sample_site_flips.resize(scanner.getSampleSiteIdentifiers().size());
}

/** Internal method to perform the randomization. */
int SignedRankRandomizer::randomize(unsigned int iSimulation, const AbstractNodesProxy& treeNodes, SimNodeContainer_t& treeSimNodes) {
    int TotalSimC = 0;
    setSeed(iSimulation);
    // First flip coins for each sample site.
    for (size_t i = 0; i < _sample_site_flips.size(); ++i)
        _sample_site_flips.set(i, Bernoulli(0.5, _random_number_generator));
    // Now assign the randomized differences to each leaf node.
    for (size_t n=0; n < treeNodes.size(); ++n) {
        if (!treeNodes.randomized(n)) continue; // skip if not randomized
        SimulationNode& simNode(treeSimNodes[n]);
        size_t d = 0;
        for (auto& ss: treeNodes.getSampleSiteMap(n)) {
            simNode.refSampleSiteDifferences()[d] = ss.second.difference();
            if (_sample_site_flips.test(d))
                simNode.refSampleSiteDifferences()[d] *= -1;
            ++d;
        }
    }
    return TotalSimC;
}

/** Adds simulated cases up the tree from node/leaf to all its parents, and so on, for a node without anforlust. */
void SignedRankRandomizer::addSim(size_t source_id, size_t target_id, const SimulationNode::SampleSiteDiff_t& diffs, SimNodeContainer_t& treeSimNodes, const ScanRunner::NodeStructureContainer_t& treeNodes) {
    if (_multiparents)
        addSimDiffs_ancestor_list(source_id, diffs, treeSimNodes, treeNodes);
    else
        addSimDiffs_recursive(target_id, diffs, treeSimNodes, treeNodes);
}

/** Adds simulated cases up the tree from node/leaf to all its parents, and so on, for a node without anforlust. */
void SignedRankRandomizer::addSimDiffs_ancestor_list(size_t source_id, const SimulationNode::SampleSiteDiff_t& diffs, SimNodeContainer_t& treeSimNodes, const ScanRunner::NodeStructureContainer_t& treeNodes) {
    const NodeStructure* node = treeNodes[source_id];
    for (NodeStructure::Ancestors_t::const_iterator itr = node->getAncestors().begin(); itr != node->getAncestors().end(); ++itr) {
        for (size_t t = 0; t < diffs.size(); ++t)
            treeSimNodes[*itr].refSampleSiteDifferencesBr()[t] += diffs[t];
    }
}

/** Adds simulated differenced up the tree from node/leaf to all its parents, and so on. */
void SignedRankRandomizer::addSimDiffs_recursive(size_t target_id, const SimulationNode::SampleSiteDiff_t& diffs, SimNodeContainer_t& treeSimNodes, const ScanRunner::NodeStructureContainer_t& treeNodes) {
    for (size_t t = 0; t < diffs.size(); ++t)
        treeSimNodes[target_id].refSampleSiteDifferencesBr()[t] += diffs[t];
    for (size_t j = 0; j < treeNodes[target_id]->getParents().size(); ++j)
        addSimDiffs_recursive(treeNodes[target_id]->getParents()[j].first->getID(), diffs, treeSimNodes, treeNodes);
}

/** Creates randomized data under the null hypothesis for Poisson model, assigning data to DataSet objects structures.
    Random number generator seed initialized based upon 'iSimulation' index. */
int SignedRankRandomizer::RandomizeData(unsigned int iSimulation, const ScanRunner::NodeStructureContainer_t& treeNodes, boost::mutex& mutex, SimNodeContainer_t& treeSimNodes) {
    // clear simulation data
    std::for_each(treeSimNodes.begin(), treeSimNodes.end(), std::mem_fun_ref(&SimulationNode::clear));

    int TotalSimC = 0;
    if (_read_data) {
        boost::mutex::scoped_lock lock(mutex);
        TotalSimC = read(_read_filename, iSimulation, treeNodes, treeSimNodes, mutex);
    } else { // else standard randomization
        TotalSimC = randomize(iSimulation, NodesProxy(treeNodes, _parameters.getDataOnlyOnLeaves()), treeSimNodes);
    }
    // write simulation data to file if requested
    if (_write_data) {
        boost::mutex::scoped_lock lock(mutex);
        write(_write_filename, treeSimNodes);
    }
    //------------------------ UPDATING THE TREE -----------------------------------
    for (size_t i = 0; i < treeNodes.size(); i++)
        addSim(i, i, treeSimNodes[i].getSampleSiteDifferences(), treeSimNodes, treeNodes);
    //checkSewerShedDataConsistency(treeSimNodes, false);
    return TotalSimC;
}

int SignedRankRandomizer::read(
    const std::string& filename, unsigned int simulation, const ScanRunner::NodeStructureContainer_t& treeNodes,
    SimNodeContainer_t& treeSimNodes, boost::mutex& mutex
) {
    std::ifstream stream;
    if (!stream.is_open()) stream.open(filename.c_str());
    if (!stream) throw resolvable_error("Error: Could not open file '%s' to read the simulated data.\n", filename.c_str());

    // seek line offset for reading iSimulation'th simulation data
    unsigned int skip = static_cast<unsigned int>((simulation - 1) * treeSimNodes.size());
    for (unsigned int i = 0; i < skip; ++i)
        stream.ignore(std::numeric_limits<int>::max(), '\n');

    double diff;
    for (size_t i = 0; i < treeSimNodes.size(); ++i) {
        SimulationNode& simNode(treeSimNodes[i]);
        for (size_t s = 0; s < simNode.getSampleSiteDifferences().size(); ++s) {
            if (!(stream >> diff) && i < treeSimNodes.size()) {
                if (stream.eof()) // check for end of file yet we should have more to read
                    throw resolvable_error(
                        "Error: Simulated data file does not contain enough data in simulation %d. Expecting %u nodes with %u datum but could only read %d nodes with %d datum.", 
                        simulation, treeSimNodes.size(), simNode.getSampleSiteDifferences().size(), i + 1, s
                    );
                throw resolvable_error(
                    "Error: Simulated data file appears to contain invalid data in simulation %d. Datum could not be read as numeric vale for element %d of node %d.\n",
                    simulation, s, i + 1
                );
            }
            simNode.refSampleSiteDifferences()[s] = diff;
        }
    }
    stream.close();
    return 0;
}

void SignedRankRandomizer::write(const std::string& filename, const SimNodeContainer_t& treeSimNodes) {
    std::ofstream stream;

    // open output file
    stream.open(filename.c_str(), std::ios::ate | std::ios::app);
    if (!stream) throw resolvable_error("Error: Could not open the simulated data output file '%s'.\n", filename.c_str());
    for (size_t i = 0; i < treeSimNodes.size(); ++i) {
        const SimulationNode& simNode(treeSimNodes[i]);
        for (auto diff: simNode.getSampleSiteDifferences()) {
            stream << diff << " ";
        }
        stream << std::endl;
    }
    stream.close();
}