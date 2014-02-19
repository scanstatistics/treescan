//******************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//******************************************************************************
#include "MonteCarloSimFunctor.h"
#include "PrjException.h"

/* constructor */
MCSimSuccessiveFunctor::MCSimSuccessiveFunctor(boost::mutex& mutex, 
                                               boost::shared_ptr<AbstractRandomizer> randomizer,
                                               const ScanRunner& scanRunner) : _mutex(mutex), _randomizer(randomizer), _scanRunner(scanRunner) {

    // TODO: Eventually this will need refactoring once we implement multiple data time ranges.
    const Parameters& parameters = _scanRunner.getParameters();
    size_t daysInDataTimeRange = parameters.getModelType() == Parameters::TEMPORALSCAN ?parameters.getDataTimeRangeSet().getTotalDaysAcrossRangeSets() : 1;
    _treeSimNodes.resize(_scanRunner.getNodes().size(), SimulationNode(daysInDataTimeRange));
    _loglikelihood.reset(AbstractLoglikelihood::getNewLoglikelihood(_scanRunner.getParameters(), _scanRunner.getTotalC(), _scanRunner.getTotalN()));
}

MCSimSuccessiveFunctor::result_type MCSimSuccessiveFunctor::operator() (MCSimSuccessiveFunctor::param_type const & param) {
    result_type temp_result;
    try {
        const Parameters& parameters = _scanRunner.getParameters();
        if (_scanRunner.getParameters().getModelType() == Parameters::TEMPORALSCAN)
            temp_result.dSuccessfulResult = scanTreeTemporal(param);
        else
            temp_result.dSuccessfulResult = scanTree(param);
        temp_result.bUnExceptional = true;
    } catch (memory_exception & e) {
        temp_result.eException_type = MCSimJobSource::result_type::memory;
        temp_result.Exception = prg_exception(e.what(), "MCSimSuccessiveFunctor");
        temp_result.bUnExceptional = false;
    } catch (resolvable_error & e) {
        temp_result.eException_type = MCSimJobSource::result_type::resolvable;
        temp_result.Exception = prg_exception(e.what(), "MCSimSuccessiveFunctor");
        temp_result.bUnExceptional = false;
    } catch (prg_exception & e) {
        temp_result.eException_type = MCSimJobSource::result_type::prg;
        temp_result.Exception = e;
        temp_result.bUnExceptional = false;
    } catch (std::exception & e) {
        temp_result.eException_type = MCSimJobSource::result_type::std;
        temp_result.Exception = prg_exception(e.what(), "MCSimSuccessiveFunctor");
        temp_result.bUnExceptional = false;
    } catch (...) {
        temp_result.eException_type = MCSimJobSource::result_type::unknown;
        temp_result.Exception = prg_exception("(...) -- unknown error", "MCSimSuccessiveFunctor");
        temp_result.bUnExceptional = false;
    }
    if (!temp_result.bUnExceptional) {
        temp_result.Exception.addTrace("operator()", "MCSimSuccessiveFunctor");
    }
    return temp_result;
}

/* This function randomizes data and scans tree for either the Poisson or Bernoulli model. */
MCSimSuccessiveFunctor::successful_result_type MCSimSuccessiveFunctor::scanTree(MCSimSuccessiveFunctor::param_type const & param) {
    //randomize data
    int TotalSimC = _randomizer.get()->RandomizeData(param, _scanRunner.getNodes(), _mutex, _treeSimNodes);

    //--------------------- SCANNING THE TREE, SIMULATIONS -------------------------
    const ScanRunner::NodeStructureContainer_t& nodes = _scanRunner.getNodes();
    double simLogLikelihood = -std::numeric_limits<double>::max();
    for (size_t i=0; i < nodes.size(); ++i) {
        if (_treeSimNodes.at(i).getBrC() > 1) { //if (_Nodes.at(i)->_SimBrC > 1)
            const NodeStructure& thisNode(*(nodes.at(i)));
            Parameters::CutType cutType = thisNode.getChildren().size() >= 2 ? thisNode.getCutType() : Parameters::SIMPLE;
            switch (cutType) {
                case Parameters::SIMPLE:
                    simLogLikelihood = std::max(simLogLikelihood, _loglikelihood->LogLikelihood(_treeSimNodes.at(i).getBrC()/*_Nodes.at(i)->_SimBrC*/, nodes.at(i)->getBrN())); 
                    break;
                case Parameters::ORDINAL:
                    for (size_t i=0; i < thisNode.getChildren().size() - 1; ++i) {
                        const NodeStructure& firstChildNode(*(nodes.at(thisNode.getChildren().at(i))));
                        //buffer = firstChildNode.getIdentifier().c_str();
                        int sumBranchC = _treeSimNodes.at(i).getBrC();
                        double sumBranchN = firstChildNode.getBrN();
                        for (size_t j=i+1; j < thisNode.getChildren().size(); ++j) {
                            const NodeStructure& childNode(*(nodes.at(thisNode.getChildren().at(j))));
                            //buffer += ",";
                            //buffer += childNode.getIdentifier();
                            sumBranchC += _treeSimNodes.at(static_cast<size_t>(childNode.getID())).getBrC();
                            sumBranchN += childNode.getBrN();
                            //printf("Evaluating cut [%s]\n", buffer.c_str());
                            simLogLikelihood = std::max(simLogLikelihood, _loglikelihood->LogLikelihood(sumBranchC, sumBranchN)); 
                            //++hits; printf("hits %d\n", hits);
                        }
                    } break;
                case Parameters::PAIRS:
                    for (size_t i=0; i < thisNode.getChildren().size() - 1; ++i) {
                        const NodeStructure& startChildNode(*(nodes.at(thisNode.getChildren().at(i))));
                        for (size_t j=i+1; j < thisNode.getChildren().size(); ++j) {
                            const NodeStructure& stopChildNode(*(nodes.at(thisNode.getChildren().at(j))));
                            //printf("Evaluating cut [%s,%s]\n", startChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                            simLogLikelihood = std::max(simLogLikelihood, 
                                                        _loglikelihood->LogLikelihood(_treeSimNodes.at(static_cast<size_t>(startChildNode.getID())).getBrC() + 
                                                                                      _treeSimNodes.at(static_cast<size_t>(stopChildNode.getID())).getBrC(), 
                                                                                       startChildNode.getBrN() + stopChildNode.getBrN())); 
                            //++hits; printf("hits %d\n", hits);
                        }
                    } break;
                case Parameters::TRIPLETS:
                    for (size_t i=0; i < thisNode.getChildren().size() - 1; ++i) {
                        const NodeStructure& startChildNode(*(nodes.at(thisNode.getChildren().at(i))));
                        for (size_t j=i+1; j < thisNode.getChildren().size(); ++j) {
                            const NodeStructure& stopChildNode(*(nodes.at(thisNode.getChildren().at(j))));
                            //printf("Evaluating cut [%s,%s]\n", startChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                            simLogLikelihood = std::max(simLogLikelihood, 
                                                        _loglikelihood->LogLikelihood(_treeSimNodes.at(static_cast<size_t>(startChildNode.getID())).getBrC() + 
                                                                                      _treeSimNodes.at(static_cast<size_t>(stopChildNode.getID())).getBrC(), 
                                                                                      startChildNode.getBrN() + stopChildNode.getBrN())); 
                            //++hits;printf("hits %d\n", hits);
                            for (size_t k=i+1; k < j; ++k) {
                                const NodeStructure& middleChildNode(*(nodes.at(thisNode.getChildren().at(k))));
                                //printf("Evaluating cut [%s,%s,%s]\n", startChildNode.getIdentifier().c_str(), middleChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                simLogLikelihood = std::max(simLogLikelihood, 
                                                            _loglikelihood->LogLikelihood(_treeSimNodes.at(static_cast<size_t>(startChildNode.getID())).getBrC() + 
                                                                                          _treeSimNodes.at(static_cast<size_t>(middleChildNode.getID())).getBrC() + 
                                                                                          _treeSimNodes.at(static_cast<size_t>(stopChildNode.getID())).getBrC(), 
                                                                                          startChildNode.getBrN() + middleChildNode.getBrN() + stopChildNode.getBrN())); 
                                //++hits; printf("hits %d\n", hits);
                            }
                        }
                    } break;
                case Parameters::COMBINATORIAL: default: throw prg_error("Unknown cut type (%d).", "scanTree()", cutType);
            };
         }
    } // for i<nNodes
    return std::make_pair(simLogLikelihood,TotalSimC);
}

/* This function randomizes data and scans tree for either the temporal model. */
MCSimSuccessiveFunctor::successful_result_type MCSimSuccessiveFunctor::scanTreeTemporal(MCSimSuccessiveFunctor::param_type const & param) {
    //randomize data
    int TotalSimC = _randomizer.get()->RandomizeData(param, _scanRunner.getNodes(), _mutex, _treeSimNodes);

    //--------------------- SCANNING THE TREE, SIMULATIONS -------------------------
    DataTimeRange::index_t idxAdditive = _scanRunner.getZeroTranslationAdditive();
    // Define the start and end windows with the zero index offset already incorporated.
    DataTimeRange startWindow(_scanRunner.getParameters().getTemporalStartRange().getStart() + idxAdditive,
                              _scanRunner.getParameters().getTemporalStartRange().getEnd() + idxAdditive),
                  endWindow(_scanRunner.getParameters().getTemporalEndRange().getStart() + idxAdditive,
                              _scanRunner.getParameters().getTemporalEndRange().getEnd() + idxAdditive);
    const ScanRunner::NodeStructureContainer_t& nodes = _scanRunner.getNodes();
    double simLogLikelihood = -std::numeric_limits<double>::max();
    for (size_t i=0; i < nodes.size(); ++i) {
        if (_treeSimNodes.at(i).getBrC() > 1) { //if (_Nodes.at(i)->_SimBrC > 1)
            const NodeStructure& thisNode(*(nodes.at(i)));
            const SimulationNode& thisSimNode(_treeSimNodes.at(i));
            Parameters::CutType cutType = thisNode.getChildren().size() >= 2 ? thisNode.getCutType() : Parameters::SIMPLE;
            switch (cutType) {
                case Parameters::SIMPLE:
                    for (DataTimeRange::index_t end=endWindow.getStart(); end < endWindow.getEnd(); ++end) {
                        for (DataTimeRange::index_t start=startWindow.getStart(); start < startWindow.getEnd(); ++start) {
                            NodeStructure::count_t branchWindow = thisSimNode.getBrC_C()[start] - thisSimNode.getBrC_C()[end];
                            NodeStructure::expected_t branchSum = static_cast<NodeStructure::expected_t>(thisSimNode.getBrC());
                            simLogLikelihood = std::max(simLogLikelihood, _loglikelihood->LogLikelihood(branchWindow, branchSum, end - start + 1)); 
                        }
                    } break;
                case Parameters::ORDINAL:
                    for (DataTimeRange::index_t end=endWindow.getStart(); end < endWindow.getEnd(); ++end) {
                        for (DataTimeRange::index_t start=startWindow.getStart(); start < startWindow.getEnd(); ++start) {
                            for (size_t i=0; i < thisNode.getChildren().size() - 1; ++i) {
                                const SimulationNode& firstSimChildNode(_treeSimNodes.at(thisNode.getChildren().at(i)));
                                //buffer = firstChildNode.getIdentifier().c_str();
                                NodeStructure::count_t branchWindow = firstSimChildNode.getBrC_C()[start] - firstSimChildNode.getBrC_C()[end];
                                NodeStructure::expected_t branchSum = static_cast<NodeStructure::expected_t>(firstSimChildNode.getBrC());
                                for (size_t j=i+1; j < thisNode.getChildren().size(); ++j) {
                                    //const NodeStructure& childNode(*(nodes.at(thisNode.getChildren().at(j))));
                                    const SimulationNode& childSimNode(_treeSimNodes.at(thisNode.getChildren().at(j)));
                                    //buffer += ",";
                                    //buffer += childNode.getIdentifier();
                                    NodeStructure::count_t childBranchC = childSimNode.getBrC_C()[start] - childSimNode.getBrC_C()[end];
                                    branchWindow += childBranchC;
                                    branchSum += static_cast<NodeStructure::expected_t>(childSimNode.getBrC());
                                    //printf("Evaluating cut [%s]\n", buffer.c_str());
                                    simLogLikelihood = std::max(simLogLikelihood, _loglikelihood->LogLikelihood(branchWindow, branchSum, end - start + 1)); 
                                    //++hits; printf("hits %d\n", hits);
                                }
                            }
                        }
                    }
                    break;
                case Parameters::PAIRS:
                    for (DataTimeRange::index_t end=endWindow.getStart(); end < endWindow.getEnd(); ++end) {
                        for (DataTimeRange::index_t start=startWindow.getStart(); start < startWindow.getEnd(); ++start) {
                            for (size_t i=0; i < thisNode.getChildren().size() - 1; ++i) {
                                const SimulationNode& startSimChildNode(_treeSimNodes.at(thisNode.getChildren().at(i)));
                                NodeStructure::count_t startBranchWindow = startSimChildNode.getBrC_C()[start] - startSimChildNode.getBrC_C()[end];
                                NodeStructure::expected_t startBranchSum = static_cast<NodeStructure::expected_t>(startSimChildNode.getBrC());
                                for (size_t j=i+1; j < thisNode.getChildren().size(); ++j) {
                                    const SimulationNode& stopSimChildNode(_treeSimNodes.at(thisNode.getChildren().at(j)));
                                    //printf("Evaluating cut [%s,%s]\n", startChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                    NodeStructure::count_t stopBranchWindow = stopSimChildNode.getBrC_C()[start] - stopSimChildNode.getBrC_C()[end];
                                    NodeStructure::expected_t stopBranchSum = static_cast<NodeStructure::expected_t>(stopSimChildNode.getBrC());
                                    simLogLikelihood = std::max(simLogLikelihood, _loglikelihood->LogLikelihood(startBranchWindow + stopBranchWindow, startBranchSum + stopBranchSum, end - start + 1)); 
                                    //++hits; printf("hits %d\n", hits);
                                }
                            } 
                        }
                    }
                    break;
                case Parameters::TRIPLETS:
                    for (DataTimeRange::index_t end=endWindow.getStart(); end < endWindow.getEnd(); ++end) {
                        for (DataTimeRange::index_t start=startWindow.getStart(); start < startWindow.getEnd(); ++start) {
                            for (size_t i=0; i < thisNode.getChildren().size() - 1; ++i) {
                                const SimulationNode& startSimChildNode(_treeSimNodes.at(thisNode.getChildren().at(i)));
                                NodeStructure::count_t startBranchWindow = startSimChildNode.getBrC_C()[start] - startSimChildNode.getBrC_C()[end];
                                NodeStructure::expected_t startBranchSum = static_cast<NodeStructure::expected_t>(startSimChildNode.getBrC());
                                for (size_t j=i+1; j < thisNode.getChildren().size(); ++j) {
                                    const SimulationNode& stopSimChildNode(_treeSimNodes.at(thisNode.getChildren().at(j)));
                                    //printf("Evaluating cut [%s,%s]\n", startChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                    NodeStructure::count_t stopBranchWindow = stopSimChildNode.getBrC_C()[start] - stopSimChildNode.getBrC_C()[end];
                                    NodeStructure::expected_t stopBranchSum = static_cast<NodeStructure::expected_t>(stopSimChildNode.getBrC());
                                    //printf("Evaluating cut [%s,%s]\n", startChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                    simLogLikelihood = std::max(simLogLikelihood, 
                                                                _loglikelihood->LogLikelihood(startBranchWindow + stopBranchWindow, startBranchSum + stopBranchSum, end - start + 1)); 
                                    //++hits;printf("hits %d\n", hits);
                                    for (size_t k=i+1; k < j; ++k) {
                                        const SimulationNode& middleSimChildNode(_treeSimNodes.at(thisNode.getChildren().at(k)));
                                        //printf("Evaluating cut [%s,%s,%s]\n", startChildNode.getIdentifier().c_str(), middleChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                        NodeStructure::count_t middleBranchWindow = middleSimChildNode.getBrC_C()[start] - middleSimChildNode.getBrC_C()[end];
                                        NodeStructure::expected_t middleBranchSum = static_cast<NodeStructure::expected_t>(middleSimChildNode.getBrC());
                                        //printf("Evaluating cut [%s,%s,%s]\n", startChildNode.getIdentifier().c_str(), middleChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                        simLogLikelihood = std::max(simLogLikelihood, 
                                                                    _loglikelihood->LogLikelihood(startBranchWindow + middleBranchWindow + stopBranchWindow, 
                                                                                                  startBranchSum + middleBranchSum + stopBranchSum, end - start + 1)); 
                                        //++hits; printf("hits %d\n", hits);
                                    }
                                }
                            }
                        }
                    }
                    break;
                case Parameters::COMBINATORIAL: default: throw prg_error("Unknown cut type (%d).", "scanTree()", cutType);
            };
         }
    } // for i<nNodes
    return std::make_pair(simLogLikelihood,TotalSimC);
}
