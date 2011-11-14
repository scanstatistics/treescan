//******************************************************************************
#include "TreeScan.h"
//******************************************************************************
#include "Randomization.h"
#include "PrjException.h"
#include "PoissonRandomizer.h"
#include "BernoulliRandomizer.h"

/** returns new randomizer given parameter settings. */
AbstractRandomizer * AbstractRandomizer::getNewRandomizer(const Parameters& parameters, int TotalC, int TotalControls, double TotalN) {
    switch (parameters.getModelType()) {
    case Parameters::POISSON: return new PoissonRandomizer(parameters.isConditional(), TotalC, TotalN);
    case Parameters::BERNOULLI: return new BernoulliRandomizer(parameters.getProbability(), parameters.isConditional(), TotalC, TotalControls, TotalN);
    default: throw prg_error("Unknown model type (%d).", "getNewRandomizer()", parameters.getModelType());
    }
}

/*
 Adds simulated cases up the tree from branhes to all its parents, and so on,
 for a node without anforlust.
 */
void AbstractRandomizer::addSimC(size_t id, int c, const ScanRunner::NodeStructureContainer_t& treeNodes, SimDataContainer_t& simData) {
    simData.at(id).second += c;  //treeNodes.at(id)->_SimBrC += c;
    for(size_t j=0; j < treeNodes.at(id)->getParent().size(); j++) addSimC(treeNodes.at(id)->getParent()[j], c, treeNodes, simData);
}

/*
 Adds simulated cases up the tree from branhes to all its parents, and so on,
 for a node with anforlust.
 Note: This code can be made more efficient by storing in memory the ancestral
 nodes that should be updated with additional simlated cases from the node
 with internal cases. To do sometime in the future.
 */
void AbstractRandomizer::addSimCAnforlust(size_t id, int c, const ScanRunner::NodeStructureContainer_t& treeNodes, SimDataContainer_t& simData) {
    simData.at(id).second += c;  //treeNodes.at(id)->_SimBrC += c;
    for(size_t j=0; j < treeNodes.at(id)->getParent().size();j++) addSimCAnforlust(treeNodes.at(id)->getParent()[j], c, treeNodes, simData);
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
