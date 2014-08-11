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
        case Parameters::POISSON: {
            switch (parameters.getConditionalType()) {
                case Parameters::UNCONDITIONAL : return new PoissonRandomizer(false, TotalC, TotalN, parameters); break;
                case Parameters::TOTALCASES : return new PoissonRandomizer(true, TotalC, TotalN, parameters); break;
                case Parameters::CASESEACHBRANCH :
                default: throw prg_error("Unknown conditional type (%d).", "getNewRandomizer()", parameters.getConditionalType());
            }
        } break;
        case Parameters::BERNOULLI: {
            switch (parameters.getConditionalType()) {
                case Parameters::UNCONDITIONAL : return new BernoulliRandomizer(false, TotalC, TotalControls, TotalN, parameters); break;
                case Parameters::TOTALCASES : return new BernoulliRandomizer(true, TotalC, TotalControls, TotalN, parameters); break;
                case Parameters::CASESEACHBRANCH :
                default: throw prg_error("Unknown conditional type (%d).", "getNewRandomizer()", parameters.getConditionalType());
            }
        } break;
        case Parameters::TEMPORALSCAN : {
            switch (parameters.getConditionalType()) {
                case Parameters::CASESEACHBRANCH : return new TemporalRandomizer(TotalC, TotalN, parameters.getDataTimeRangeSet(), parameters); break;
                case Parameters::UNCONDITIONAL :
                case Parameters::TOTALCASES :
                default: throw prg_error("Unknown conditional type (%d).", "getNewRandomizer()", parameters.getConditionalType());
            }
        } break;
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
void AbstractRandomizer::setSeed(unsigned int iSimulationIndex) {
    try {
        //calculate seed as unsigned long
        unsigned long ulSeed = _random_number_generator.GetInitialSeed() + iSimulationIndex;
        //compare to max seed(declared as positive signed long)
        if (ulSeed >= static_cast<unsigned long>(_random_number_generator.GetMaxSeed()))
            throw prg_error("Calculated seed for simulation %u, exceeds defined limit of %i.", "setSeed()", iSimulationIndex, _random_number_generator.GetMaxSeed());
        _random_number_generator.SetSeed(static_cast<long>(ulSeed));
    } catch (prg_exception& x) {
        x.addTrace("setSeed()","AbstractRandomizer");
        throw;
    }
}
