//******************************************************************************
#ifndef __WilcoxonSignedRankRandomizer_H
#define __WilcoxonSignedRankRandomizer_H
//******************************************************************************
#include "Randomization.h"
#include "RandomDistribution.h"

/* Wilcoxon signed rank data randomizer. */
class WilcoxonSignedRankRandomizer : public AbstractRandomizer {
protected:
    const ScanRunner& _scanner;
    boost::dynamic_bitset<> _sample_site_flips;

    virtual int read(const std::string& filename, unsigned int simulation, const ScanRunner::NodeStructureContainer_t& treeNodes, SimNodeContainer_t& treeSimNodes, boost::mutex& mutex);
    virtual void write(const std::string& filename, const SimNodeContainer_t& treeSimNodes);
    void addSimDiffs(size_t target_id, const SimulationNode::SampleSiteDiff_t& diffs, SimNodeContainer_t& treeSimNodes, const ScanRunner::NodeStructureContainer_t& treeNodes);
    virtual int randomize(unsigned int iSimulation, const AbstractNodesProxy& treeNodes, SimNodeContainer_t& treeSimNodes);

public:
    WilcoxonSignedRankRandomizer(const ScanRunner& scanner, long lInitialSeed = RandomNumberGenerator::glDefaultSeed);
    virtual ~WilcoxonSignedRankRandomizer() {}

    virtual WilcoxonSignedRankRandomizer* clone() const { return new WilcoxonSignedRankRandomizer(*this); }

    virtual int RandomizeData(unsigned int iSimulation, const ScanRunner::NodeStructureContainer_t& treeNodes, boost::mutex& mutex, SimNodeContainer_t& treeSimNodes);
};
//******************************************************************************
#endif
