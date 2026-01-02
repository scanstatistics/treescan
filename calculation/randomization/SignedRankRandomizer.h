//******************************************************************************
#ifndef __SignedRankRandomizer_H
#define __SignedRankRandomizer_H
//******************************************************************************
#include "Randomization.h"
#include "RandomDistribution.h"

/* Signed rank data randomizer. */
class SignedRankRandomizer : public AbstractRandomizer {
protected:
    const ScanRunner& _scanner;
    boost::dynamic_bitset<> _sample_site_flips;

    virtual int read(const std::string& filename, unsigned int simulation, const ScanRunner::NodeStructureContainer_t& treeNodes, SimNodeContainer_t& treeSimNodes, boost::mutex& mutex);
    virtual void write(const std::string& filename, const SimNodeContainer_t& treeSimNodes);
    void addSim(size_t source_id, size_t target_id, const SimulationNode::SampleSiteDiff_t& diffs, SimNodeContainer_t& treeSimNodes, const ScanRunner::NodeStructureContainer_t& treeNodes);


    void addSimDiffs_ancestor_list(size_t source_id, const SimulationNode::SampleSiteDiff_t& diffs, SimNodeContainer_t& treeSimNodes, const ScanRunner::NodeStructureContainer_t& treeNodes);
    void addSimDiffs_recursive(size_t target_id, const SimulationNode::SampleSiteDiff_t& diffs, SimNodeContainer_t& treeSimNodes, const ScanRunner::NodeStructureContainer_t& treeNodes);
    virtual int randomize(unsigned int iSimulation, const AbstractNodesProxy& treeNodes, SimNodeContainer_t& treeSimNodes);

public:
    SignedRankRandomizer(const ScanRunner& scanner, long lInitialSeed = RandomNumberGenerator::glDefaultSeed);
    virtual ~SignedRankRandomizer() {}

    virtual SignedRankRandomizer* clone() const { return new SignedRankRandomizer(*this); }

    virtual int RandomizeData(unsigned int iSimulation, const ScanRunner::NodeStructureContainer_t& treeNodes, boost::mutex& mutex, SimNodeContainer_t& treeSimNodes);
};
//******************************************************************************
#endif
