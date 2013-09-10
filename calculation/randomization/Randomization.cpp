//******************************************************************************
#include "TreeScan.h"
//******************************************************************************
#include "Randomization.h"
#include "PrjException.h"
#include "PoissonRandomizer.h"
#include "BernoulliRandomizer.h"
#include "TemporalRandomizer.h"

/** returns new randomizer given parameter settings. */
AbstractRandomizer * AbstractRandomizer::getNewRandomizer(const Parameters& parameters, int TotalC, int TotalControls, double TotalN) {
    switch (parameters.getModelType()) {
        case Parameters::POISSON: return new PoissonRandomizer(parameters.isConditional(), TotalC, TotalN);
        case Parameters::BERNOULLI: return new BernoulliRandomizer(parameters.getProbability(), parameters.isConditional(), TotalC, TotalControls, TotalN);
        case Parameters::TEMPORALSCAN : return new TemporalRandomizer(TotalC, TotalN, parameters.getDataTimeRangeSet());
        default: throw prg_error("Unknown model type (%d).", "getNewRandomizer()", parameters.getModelType());
    }
}

/*
 Adds simulated cases up the tree from branhes to all its parents, and so on, for a node without anforlust.
 */
void AbstractRandomizer::addSimC_C(size_t id, NodeStructure::CountContainer_t& c, const ScanRunner::NodeStructureContainer_t& treeNodes, SimNodeContainer_t& treeSimNodes) {
    std::transform(c.begin(), c.end(), treeSimNodes.at(id).refBrC_C().begin(), treeSimNodes.at(id).refBrC_C().begin(), std::plus<int>());
    for(size_t j=0; j < treeNodes.at(id)->getParents().size(); ++j) 
        addSimC_C(treeNodes.at(id)->getParents()[j], c, treeNodes, treeSimNodes);
}

/*
 Adds simulated cases up the tree from branhes to all its parents, and so on, for a node with anforlust.
 Note: This code can be made more efficient by storing in memory the ancestral
 nodes that should be updated with additional simlated cases from the node
 with internal cases. To do sometime in the future.
 */
void AbstractRandomizer::addSimC_CAnforlust(size_t id, NodeStructure::CountContainer_t& c, const ScanRunner::NodeStructureContainer_t& treeNodes, SimNodeContainer_t& treeSimNodes) {
    std::transform(c.begin(), c.end(), treeSimNodes.at(id).refBrC_C().begin(), treeSimNodes.at(id).refBrC_C().begin(), std::plus<int>());
    for (size_t j=0; j < treeNodes.at(id)->getParents().size(); ++j) 
        addSimC_CAnforlust(treeNodes.at(id)->getParents()[j], c, treeNodes, treeSimNodes);
}

/** Reset seed of randomizer for particular simulation index. */
void AbstractRandomizer::SetSeed(unsigned int iSimulationIndex) {
    try {
        //calculate seed as unsigned long
        unsigned long ulSeed = _randomNumberGenerator.GetInitialSeed() + iSimulationIndex;
        //compare to max seed(declared as positive signed long)
        if (ulSeed >= static_cast<unsigned long>(_randomNumberGenerator.GetMaxSeed()))
            throw prg_error("Calculated seed for simulation %u, exceeds defined limit of %i.", "SetSeed()", iSimulationIndex, _randomNumberGenerator.GetMaxSeed());
        _randomNumberGenerator.SetSeed(static_cast<long>(ulSeed));
    } catch (prg_exception& x) {
        x.addTrace("SetSeed()","AbstractRandomizer");
        throw;
    }
}
