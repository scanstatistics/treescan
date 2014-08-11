//******************************************************************************
#include "TreeScan.h"
//******************************************************************************
#include "AlternativeHypothesisRandomizer.h"

AlternativeHypothesisRandomizater::AlternativeHypothesisRandomizater(const ScanRunner::NodeStructureContainer_t& treeNodes,
                                                                     boost::shared_ptr<AbstractRandomizer> randomizer,
                                                                     const RelativeRiskAdjustmentHandler& adjustments,
                                                                     const Parameters& parameters, 
                                                                     long lInitialSeed)
                                  : AbstractRandomizer(parameters, lInitialSeed), _randomizer(randomizer), _alternative_adjustments(adjustments) {

    assert(!(_parameters.getModelType() == Parameters::BERNOULLI && _parameters.getConditionalType() == Parameters::TOTALCASES)); // Not implemented for this model yet.
    assert(_parameters.getModelType() != Parameters::TEMPORALSCAN); // Not implemented for this model yet.

    _randomizer->_read_data = false;
    _randomizer->_write_data = false;

    if (_parameters.getModelType() == Parameters::POISSON) {
        for (size_t t=0; t < treeNodes.size(); ++t) {
            _nodes_IntN_C.push_back(NodeStructure::ExpectedContainer_t());
            _nodes_IntN_C.back().resize(treeNodes.at(t)->getIntN_C().size());
            std::copy(treeNodes.at(t)->getIntN_C().begin(), treeNodes.at(t)->getIntN_C().end(), _nodes_IntN_C.at(t).begin());
        }
        // apply adjustments
        _alternative_adjustments.apply(_nodes_IntN_C);
        _nodes_proxy.reset(new AlternativeExpectedNodesProxy(_nodes_IntN_C));
    } else if  (_parameters.getModelType() == Parameters::BERNOULLI && _parameters.getConditionalType() == Parameters::UNCONDITIONAL) {
        _nodes_proxy.reset(new AlternativeProbabilityNodesProxy(treeNodes, _alternative_adjustments, _parameters.getProbability()));
    }
}

int AlternativeHypothesisRandomizater::randomize(unsigned int iSimulation, const AbstractNodesProxy& treeNodes, SimNodeContainer_t& treeSimNodes) {
    return _randomizer->randomize(iSimulation, treeNodes, treeSimNodes);
}

int AlternativeHypothesisRandomizater::RandomizeData(unsigned int iSimulation, const ScanRunner::NodeStructureContainer_t& treeNodes, boost::mutex& mutex, SimNodeContainer_t& treeSimNodes) {
    int totalCases=0;
    // read simulation data to file if requested
    if (_read_data) {
        boost::mutex::scoped_lock lock(mutex);
        totalCases = read(_read_filename, iSimulation, treeNodes, treeSimNodes);
    }

    totalCases = randomize(iSimulation, *_nodes_proxy, treeSimNodes);

    // write simulation data to file if requested
    if (_write_data) {
        boost::mutex::scoped_lock lock(mutex);
        write(_write_filename, treeSimNodes);
    }
    //------------------------ UPDATING THE TREE -----------------------------------
    for (size_t i=0; i < treeNodes.size(); i++) {
        if (treeNodes.at(i)->getAnforlust()==false) 
            addSimC_C(i, treeSimNodes.at(i).refIntC_C()/*_Nodes.at(i)->_SimIntC*/, treeNodes, treeSimNodes);
        else
            addSimC_CAnforlust(i, treeSimNodes.at(i).refIntC_C()/*_Nodes.at(i)->_SimIntC*/, treeNodes, treeSimNodes);
    }
    return totalCases;
}

int AlternativeHypothesisRandomizater::read(const std::string& filename, unsigned int simulation, const ScanRunner::NodeStructureContainer_t& treeNodes, SimNodeContainer_t& treeSimNodes) {
    return _randomizer->read(filename, simulation, treeNodes, treeSimNodes);
}

void AlternativeHypothesisRandomizater::write(const std::string& filename, const SimNodeContainer_t& treeSimNodes) {
    _randomizer->write(filename, treeSimNodes);
}
