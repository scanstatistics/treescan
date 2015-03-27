//******************************************************************************
#include "TreeScan.h"
//******************************************************************************
#include "Randomization.h"
#include "PrjException.h"
#include "PoissonRandomizer.h"
#include "BernoulliRandomizer.h"
#include "TemporalRandomizer.h"

/** returns new randomizer given parameter settings. */
AbstractRandomizer * AbstractRandomizer::getNewRandomizer(const ScanRunner& scanner) {
    const Parameters& parameters = scanner.getParameters();
    switch (parameters.getScanType()) {

        case Parameters::TREEONLY : {
            switch (parameters.getConditionalType()) {
                case Parameters::UNCONDITIONAL :
                    if (parameters.getModelType() == Parameters::POISSON)
                        return new PoissonRandomizer(false, scanner.getTotalC(), scanner.getTotalN(), parameters);
                    if (parameters.getModelType() == Parameters::BERNOULLI)
                        return new BernoulliRandomizer(false, scanner.getTotalC(), scanner.getTotalControls(), scanner.getTotalN(), parameters);
                    throw prg_error("Cannot determine randomizer: tree-only, unconditonal, model (%d).", "getNewRandomizer()", parameters.getModelType());
                case Parameters::TOTALCASES :
                    if (parameters.getModelType() == Parameters::POISSON)
                        return new PoissonRandomizer(true, scanner.getTotalC(), scanner.getTotalN(), parameters);
                    if (parameters.getModelType() == Parameters::BERNOULLI)
                        return new BernoulliRandomizer(true, scanner.getTotalC(), scanner.getTotalControls(), scanner.getTotalN(), parameters);
                    throw prg_error("Cannot determine randomizer: tree-only, total-cases, model (%d).", "getNewRandomizer()", parameters.getModelType());
                default: throw prg_error("Cannot determine randomizer: tree-only, condition type (%d).", "getNewRandomizer()", parameters.getConditionalType());
            }
        }

        case Parameters::TREETIME : {
            switch (parameters.getConditionalType()) {
                case Parameters::NODE :
                    if (parameters.getModelType() == Parameters::UNIFORM)
                        return new TemporalRandomizer(scanner);
                    throw prg_error("Cannot determine randomizer: tree-time, node, model (%d).", "getNewRandomizer()", parameters.getModelType());
                case Parameters::NODEANDTIME :
                    if (parameters.getModelType() == Parameters::MODEL_NOT_APPLICABLE)
                        return new ConditionalTemporalRandomizer(scanner);
                    throw prg_error("Cannot determine randomizer: tree-time, node-time, model (%d).", "getNewRandomizer()", parameters.getModelType());
                default: throw prg_error("Cannot determine randomizer: tree-time, condition type (%d).", "getNewRandomizer()", parameters.getConditionalType());
            }
        }

        case Parameters::TIMEONLY : { /* time-only, conditioned on total cases, is a special case of tree-time, conditioned on the node with only one node */
            switch (parameters.getConditionalType()) {
                case Parameters::TOTALCASES :
                    if (parameters.getModelType() == Parameters::UNIFORM)
                        return new TemporalRandomizer(scanner);
                    throw prg_error("Cannot determine randomizer: time-only, total-cases, model (%d).", "getNewRandomizer()", parameters.getModelType());
                default: throw prg_error("Cannot determine randomizer: time-only, condition type (%d).", "getNewRandomizer()", parameters.getConditionalType());
            }
        }

        default: throw prg_error("Unknown scan type (%d).", "getNewRandomizer()", parameters.getScanType());
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
