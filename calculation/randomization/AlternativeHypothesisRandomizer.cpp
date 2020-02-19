//******************************************************************************
#include "TreeScan.h"
//******************************************************************************
#include <numeric>
#include "AlternativeHypothesisRandomizer.h"

AlternativeHypothesisRandomizater::AlternativeHypothesisRandomizater(const ScanRunner::NodeStructureContainer_t& treeNodes,
                                                                     boost::shared_ptr<AbstractRandomizer> randomizer,
                                                                     const RelativeRiskAdjustmentHandler& adjustments,
                                                                     const Parameters& parameters, 
                                                                     int totalC,
                                                                     bool multiparents,
                                                                     long lInitialSeed)
                                  : AbstractRandomizer(parameters, multiparents, lInitialSeed), _randomizer(randomizer), _alternative_adjustments(adjustments) {
    _randomizer->_read_data = false;
    _randomizer->_write_data = false;
    if (_parameters.getModelType() == Parameters::POISSON) {
        for (size_t t=0; t < treeNodes.size(); ++t) {
            _nodes_IntN_C.push_back(NodeStructure::ExpectedContainer_t());
            _nodes_IntN_C.back().resize(treeNodes[t]->getIntN_C().size());
            std::copy(treeNodes[t]->getIntN_C().begin(), treeNodes[t]->getIntN_C().end(), _nodes_IntN_C[t].begin());
        }
        // apply adjustments
        _alternative_adjustments.apply(_nodes_IntN_C);
        if (_parameters.getConditionalType() == Parameters::TOTALCASES) {
            // now re-calibrate expected counts given adjustments
            double newTotalN=0.0;
            for(RelativeRiskAdjustmentHandler::NodesExpectedContainer_t::const_iterator itr=_nodes_IntN_C.begin(); itr != _nodes_IntN_C.end(); ++itr) {
                newTotalN = std::accumulate(itr->begin(), itr->end(), newTotalN);
            }
            double adjustN = static_cast<double>(totalC)/newTotalN;
            for(RelativeRiskAdjustmentHandler::NodesExpectedContainer_t::iterator itr=_nodes_IntN_C.begin(); itr != _nodes_IntN_C.end(); ++itr) {
                std::transform(itr->begin(), itr->end(), itr->begin(), std::bind1st(std::multiplies<double>(), adjustN));
            }
        }
        _nodes_proxy.reset(new AlternativeExpectedNodesProxy(treeNodes, _nodes_IntN_C));
    } else if  (_parameters.getModelType() == Parameters::BERNOULLI_TREE && _parameters.getConditionalType() == Parameters::UNCONDITIONAL) {
        _nodes_proxy.reset(new AlternativeProbabilityNodesProxy(treeNodes, _alternative_adjustments, _parameters.getProbability()));
    } else if  (_parameters.getModelType() == Parameters::BERNOULLI_TREE && _parameters.getConditionalType() == Parameters::TOTALCASES) {
        /* We won't actually use this NodesProxy instance. The ConditionalBernoulliAlternativeHypothesisRandomizer class will
           contain two different instances of NodesProxy - one for alternative hypothesis nodes, the other for the null hypothesis nodes. */
        _nodes_proxy.reset(new NodesProxy(treeNodes));
    } else if ((_parameters.getScanType() == Parameters::TREETIME && _parameters.getConditionalType() == Parameters::NODE) ||
               (_parameters.getScanType() == Parameters::TIMEONLY && _parameters.getConditionalType() == Parameters::TOTALCASES)) {
        size_t daysInDataTimeRange = _parameters.getDataTimeRangeSet().getTotalDaysAcrossRangeSets();
        for (size_t t=0; t < treeNodes.size(); ++t) {
            _nodes_IntN_C.push_back(NodeStructure::ExpectedContainer_t());
            // Assign one expected event to each node-time entry.
            _nodes_IntN_C.back().resize(daysInDataTimeRange, 1.0);
        }
        // For each node-time entry, multiply with the relative risk.
        _alternative_adjustments.apply(_nodes_IntN_C);
        // Set cumulative
        RelativeRiskAdjustmentHandler::NodesExpectedContainer_t::iterator itr=_nodes_IntN_C.begin();
        for (; itr != _nodes_IntN_C.end(); ++itr) {
            // Add zero to front -- so we have a lower limit when we're determining position during randomization.
            itr->insert(itr->begin(), 0);
            TreeScan::cumulative_forward<NodeStructure::ExpectedContainer_t>(*itr);
        }
        _nodes_proxy.reset(new AlternativeExpectedNodesProxy(treeNodes, _nodes_IntN_C));
    } else
        throw prg_exception("Unknown context for alternative hypothesis encountered.", "AlternativeHypothesisRandomizater()");
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
    for (size_t i=0; i < treeNodes.size(); i++)
        addSimC_C(i, i, treeSimNodes[i].getIntC_C(), treeSimNodes, treeNodes);
    return totalCases;
}

int AlternativeHypothesisRandomizater::read(const std::string& filename, unsigned int simulation, const ScanRunner::NodeStructureContainer_t& treeNodes, SimNodeContainer_t& treeSimNodes) {
    return _randomizer->read(filename, simulation, treeNodes, treeSimNodes);
}

void AlternativeHypothesisRandomizater::write(const std::string& filename, const SimNodeContainer_t& treeSimNodes) {
    _randomizer->write(filename, treeSimNodes);
}
