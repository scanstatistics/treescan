//******************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//******************************************************************************
#include "MonteCarloSimFunctor.h"
#include "PrjException.h"
#include "WindowLength.h"

/* constructor */
MCSimSuccessiveFunctor::MCSimSuccessiveFunctor(boost::mutex& mutex,
                                               boost::shared_ptr<AbstractRandomizer> randomizer,
                                               const ScanRunner& scanRunner) : _mutex(mutex), _randomizer(randomizer), _scanRunner(scanRunner) {

    // TODO: Eventually this will need refactoring once we implement multiple data time ranges.
    const Parameters& parameters = _scanRunner.getParameters();
    size_t daysInDataTimeRange = Parameters::isTemporalScanType(parameters.getScanType()) ?parameters.getDataTimeRangeSet().getTotalDaysAcrossRangeSets() + 1 : 1;
    _treeSimNodes.resize(_scanRunner.getNodes().size(), SimulationNode(daysInDataTimeRange));
    _loglikelihood.reset(AbstractLoglikelihood::getNewLoglikelihood(_scanRunner.getParameters(), _scanRunner.getTotalC(), _scanRunner.getTotalN()));
}

MCSimSuccessiveFunctor::result_type MCSimSuccessiveFunctor::operator() (MCSimSuccessiveFunctor::param_type const & param) {
    result_type temp_result;
    try {
        const Parameters& parameters = _scanRunner.getParameters();
        if ((parameters.getScanType() == Parameters::TREETIME && parameters.getConditionalType() == Parameters::NODEANDTIME) ||
            (parameters.getScanType() == Parameters::TIMEONLY && parameters.isPerformingDayOfWeekAdjustment()) ||
            (parameters.getScanType() == Parameters::TREETIME && parameters.getConditionalType() == Parameters::CASESEACHBRANCH && parameters.isPerformingDayOfWeekAdjustment()))
            temp_result.dSuccessfulResult = scanTreeTemporalConditionNodeTime(param);
        else if (parameters.getModelType() == Parameters::UNIFORM)
            temp_result.dSuccessfulResult = scanTreeTemporalConditionNode(param);
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
            // always do simple cut
            simLogLikelihood = std::max(simLogLikelihood, _loglikelihood->LogLikelihood(_treeSimNodes.at(i).getBrC()/*_Nodes.at(i)->_SimBrC*/, nodes.at(i)->getBrN()));
            Parameters::CutType cutType = thisNode.getChildren().size() >= 2 ? thisNode.getCutType() : Parameters::SIMPLE;
            switch (cutType) {
                case Parameters::SIMPLE: break; // already done
                case Parameters::ORDINAL:
                    // Ordinal cuts: ABCD -> AB, ABC, ABCD, BC, BCD, CD
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
                    // Pair cuts: ABCD -> AB, AC, AD, BC, BD, CD
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
                    // Triple cuts: ABCD -> AB, AC, ABC, AD, ABD, ACD, BC, BD, BCD, CD
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
MCSimSuccessiveFunctor::successful_result_type MCSimSuccessiveFunctor::scanTreeTemporalConditionNode(MCSimSuccessiveFunctor::param_type const & param) {
    //randomize data
    int TotalSimC = _randomizer.get()->RandomizeData(param, _scanRunner.getNodes(), _mutex, _treeSimNodes);

    //--------------------- SCANNING THE TREE, SIMULATIONS -------------------------
    DataTimeRange::index_t idxAdditive = _scanRunner.getZeroTranslationAdditive();
    // Define the start and end windows with the zero index offset already incorporated.
    DataTimeRange startWindow(_scanRunner.getParameters().getTemporalStartRange().getStart() + idxAdditive,
                              _scanRunner.getParameters().getTemporalStartRange().getEnd() + idxAdditive),
                  endWindow(_scanRunner.getParameters().getTemporalEndRange().getStart() + idxAdditive,
                              _scanRunner.getParameters().getTemporalEndRange().getEnd() + idxAdditive);
    // Define the minimum and maximum window lengths.
    WindowLength window(static_cast<int>(_scanRunner.getParameters().getMinimumWindowLength()) - 1,
                        static_cast<int>(_scanRunner.getParameters().getMaximumWindowInTimeUnits()) - 1);
    int  iWindowStart, iMinWindowStart, iWindowEnd, iMaxEndWindow;

    const ScanRunner::NodeStructureContainer_t& nodes = _scanRunner.getNodes();
    double simLogLikelihood = -std::numeric_limits<double>::max();
    for (size_t i=0; i < nodes.size(); ++i) {
        if (_treeSimNodes.at(i).getBrC() > 1) { //if (_Nodes.at(i)->_SimBrC > 1)
            const NodeStructure& thisNode(*(nodes.at(i)));
            const SimulationNode& thisSimNode(_treeSimNodes.at(i));

            // always do simple cut
            iMaxEndWindow = std::min(endWindow.getEnd(), startWindow.getEnd() + window.maximum());
            for (iWindowEnd=endWindow.getStart(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
                iMinWindowStart = std::max(iWindowEnd - window.maximum(), startWindow.getStart());
                iWindowStart = std::min(startWindow.getEnd(), iWindowEnd - window.minimum());
                for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
                    NodeStructure::count_t branchWindow = thisSimNode.getBrC_C()[iWindowStart] - thisSimNode.getBrC_C()[iWindowEnd + 1];
                    NodeStructure::expected_t branchSum = static_cast<NodeStructure::expected_t>(thisSimNode.getBrC());
                    simLogLikelihood = std::max(simLogLikelihood, _loglikelihood->LogLikelihood(branchWindow, branchSum, iWindowEnd - iWindowStart + 1));
                }
            }

            Parameters::CutType cutType = thisNode.getChildren().size() >= 2 ? thisNode.getCutType() : Parameters::SIMPLE;
            switch (cutType) {
                case Parameters::SIMPLE: break; // already done
                case Parameters::ORDINAL:
                    // Ordinal cuts: ABCD -> AB, ABC, ABCD, BC, BCD, CD
                    iMaxEndWindow = std::min(endWindow.getEnd(), startWindow.getEnd() + window.maximum());
                    for (iWindowEnd=endWindow.getStart(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
                        iMinWindowStart = std::max(iWindowEnd - window.maximum(), startWindow.getStart());
                        iWindowStart = std::min(startWindow.getEnd(), iWindowEnd - window.minimum());
                        for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
                            for (size_t i=0; i < thisNode.getChildren().size() - 1; ++i) {
                                const SimulationNode& firstSimChildNode(_treeSimNodes.at(thisNode.getChildren().at(i)));
                                //buffer = firstChildNode.getIdentifier().c_str();
                                NodeStructure::count_t branchWindow = firstSimChildNode.getBrC_C()[iWindowStart] - firstSimChildNode.getBrC_C()[iWindowEnd + 1];
                                NodeStructure::expected_t branchSum = static_cast<NodeStructure::expected_t>(firstSimChildNode.getBrC());
                                for (size_t j=i+1; j < thisNode.getChildren().size(); ++j) {
                                    //const NodeStructure& childNode(*(nodes.at(thisNode.getChildren().at(j))));
                                    const SimulationNode& childSimNode(_treeSimNodes.at(thisNode.getChildren().at(j)));
                                    //buffer += ",";
                                    //buffer += childNode.getIdentifier();
                                    NodeStructure::count_t childBranchC = childSimNode.getBrC_C()[iWindowStart] - childSimNode.getBrC_C()[iWindowEnd + 1];
                                    branchWindow += childBranchC;
                                    branchSum += static_cast<NodeStructure::expected_t>(childSimNode.getBrC());
                                    //printf("Evaluating cut [%s]\n", buffer.c_str());
                                    simLogLikelihood = std::max(simLogLikelihood, _loglikelihood->LogLikelihood(branchWindow, branchSum, iWindowEnd - iWindowStart + 1));
                                    //++hits; printf("hits %d\n", hits);
                                }
                            }
                        }
                    }
                    break;
                case Parameters::PAIRS:
                    // Pair cuts: ABCD -> AB, AC, AD, BC, BD, CD
                    iMaxEndWindow = std::min(endWindow.getEnd(), startWindow.getEnd() + window.maximum());
                    for (iWindowEnd=endWindow.getStart(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
                        iMinWindowStart = std::max(iWindowEnd - window.maximum(), startWindow.getStart());
                        iWindowStart = std::min(startWindow.getEnd(), iWindowEnd - window.minimum());
                        for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
                            for (size_t i=0; i < thisNode.getChildren().size() - 1; ++i) {
                                const SimulationNode& startSimChildNode(_treeSimNodes.at(thisNode.getChildren().at(i)));
                                NodeStructure::count_t startBranchWindow = startSimChildNode.getBrC_C()[iWindowStart] - startSimChildNode.getBrC_C()[iWindowEnd + 1];
                                NodeStructure::expected_t startBranchSum = static_cast<NodeStructure::expected_t>(startSimChildNode.getBrC());
                                for (size_t j=i+1; j < thisNode.getChildren().size(); ++j) {
                                    const SimulationNode& stopSimChildNode(_treeSimNodes.at(thisNode.getChildren().at(j)));
                                    //printf("Evaluating cut [%s,%s]\n", startChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                    NodeStructure::count_t stopBranchWindow = stopSimChildNode.getBrC_C()[iWindowStart] - stopSimChildNode.getBrC_C()[iWindowEnd + 1];
                                    NodeStructure::expected_t stopBranchSum = static_cast<NodeStructure::expected_t>(stopSimChildNode.getBrC());
                                    simLogLikelihood = std::max(simLogLikelihood, _loglikelihood->LogLikelihood(startBranchWindow + stopBranchWindow, startBranchSum + stopBranchSum, iWindowEnd - iWindowStart + 1));
                                    //++hits; printf("hits %d\n", hits);
                                }
                            }
                        }
                    }
                    break;
                case Parameters::TRIPLETS:
                    // Triple cuts: ABCD -> AB, AC, ABC, AD, ABD, ACD, BC, BD, BCD, CD
                    iMaxEndWindow = std::min(endWindow.getEnd(), startWindow.getEnd() + window.maximum());
                    for (iWindowEnd=endWindow.getStart(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
                        iMinWindowStart = std::max(iWindowEnd - window.maximum(), startWindow.getStart());
                        iWindowStart = std::min(startWindow.getEnd(), iWindowEnd - window.minimum());
                        for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
                            for (size_t i=0; i < thisNode.getChildren().size() - 1; ++i) {
                                const SimulationNode& startSimChildNode(_treeSimNodes.at(thisNode.getChildren().at(i)));
                                NodeStructure::count_t startBranchWindow = startSimChildNode.getBrC_C()[iWindowStart] - startSimChildNode.getBrC_C()[iWindowEnd + 1];
                                NodeStructure::expected_t startBranchSum = static_cast<NodeStructure::expected_t>(startSimChildNode.getBrC());
                                for (size_t j=i+1; j < thisNode.getChildren().size(); ++j) {
                                    const SimulationNode& stopSimChildNode(_treeSimNodes.at(thisNode.getChildren().at(j)));
                                    //printf("Evaluating cut [%s,%s]\n", startChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                    NodeStructure::count_t stopBranchWindow = stopSimChildNode.getBrC_C()[iWindowStart] - stopSimChildNode.getBrC_C()[iWindowEnd + 1];
                                    NodeStructure::expected_t stopBranchSum = static_cast<NodeStructure::expected_t>(stopSimChildNode.getBrC());
                                    //printf("Evaluating cut [%s,%s]\n", startChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                    simLogLikelihood = std::max(simLogLikelihood,
                                                                _loglikelihood->LogLikelihood(startBranchWindow + stopBranchWindow, startBranchSum + stopBranchSum, iWindowEnd - iWindowStart + 1));
                                    //++hits;printf("hits %d\n", hits);
                                    for (size_t k=i+1; k < j; ++k) {
                                        const SimulationNode& middleSimChildNode(_treeSimNodes.at(thisNode.getChildren().at(k)));
                                        //printf("Evaluating cut [%s,%s,%s]\n", startChildNode.getIdentifier().c_str(), middleChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                        NodeStructure::count_t middleBranchWindow = middleSimChildNode.getBrC_C()[iWindowStart] - middleSimChildNode.getBrC_C()[iWindowEnd + 1];
                                        NodeStructure::expected_t middleBranchSum = static_cast<NodeStructure::expected_t>(middleSimChildNode.getBrC());
                                        //printf("Evaluating cut [%s,%s,%s]\n", startChildNode.getIdentifier().c_str(), middleChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                        simLogLikelihood = std::max(simLogLikelihood,
                                                                    _loglikelihood->LogLikelihood(startBranchWindow + middleBranchWindow + stopBranchWindow,
                                                                                                  startBranchSum + middleBranchSum + stopBranchSum, iWindowEnd - iWindowStart + 1));
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

/* This function randomizes data and scans tree for either the temporal model. */
MCSimSuccessiveFunctor::successful_result_type MCSimSuccessiveFunctor::scanTreeTemporalConditionNodeTime(MCSimSuccessiveFunctor::param_type const & param) {
    //randomize data
    int TotalSimC = _randomizer.get()->RandomizeData(param, _scanRunner.getNodes(), _mutex, _treeSimNodes);

    //--------------------- SCANNING THE TREE, SIMULATIONS -------------------------
    DataTimeRange::index_t idxAdditive = _scanRunner.getZeroTranslationAdditive();
    // Define the start and end windows with the zero index offset already incorporated.
    DataTimeRange startWindow(_scanRunner.getParameters().getTemporalStartRange().getStart() + idxAdditive,
                              _scanRunner.getParameters().getTemporalStartRange().getEnd() + idxAdditive),
                  endWindow(_scanRunner.getParameters().getTemporalEndRange().getStart() + idxAdditive,
                              _scanRunner.getParameters().getTemporalEndRange().getEnd() + idxAdditive);
    // Define the minimum and maximum window lengths.
    WindowLength window(static_cast<int>(_scanRunner.getParameters().getMinimumWindowLength()) - 1,
                        static_cast<int>(_scanRunner.getParameters().getMaximumWindowInTimeUnits()) - 1);
    int  iWindowStart, iMinWindowStart, iWindowEnd, iMaxEndWindow;

    const ScanRunner::NodeStructureContainer_t& nodes = _scanRunner.getNodes();
    double simLogLikelihood = -std::numeric_limits<double>::max();
    for (size_t i=0; i < nodes.size(); ++i) {
        if (_treeSimNodes.at(i).getBrC() > 1) { //if (_Nodes.at(i)->_SimBrC > 1)
            const NodeStructure& thisNode(*(nodes.at(i)));
            const SimulationNode& thisSimNode(_treeSimNodes.at(i));

            // always do simple cut
            iMaxEndWindow = std::min(endWindow.getEnd(), startWindow.getEnd() + window.maximum());
            for (iWindowEnd=endWindow.getStart(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
                iMinWindowStart = std::max(iWindowEnd - window.maximum(), startWindow.getStart());
                iWindowStart = std::min(startWindow.getEnd(), iWindowEnd - window.minimum());
                for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
                    NodeStructure::count_t branchWindow = thisSimNode.getBrC_C()[iWindowStart] - thisSimNode.getBrC_C()[iWindowEnd + 1];
                    NodeStructure::expected_t branchExpected = thisNode.getBrN_C()[iWindowStart] - thisNode.getBrN_C()[iWindowEnd + 1];
                    simLogLikelihood = std::max(simLogLikelihood, _loglikelihood->LogLikelihood(branchWindow, branchExpected));
                }
            }

            Parameters::CutType cutType = thisNode.getChildren().size() >= 2 ? thisNode.getCutType() : Parameters::SIMPLE;
            switch (cutType) {
                case Parameters::SIMPLE: break; // already done
                case Parameters::ORDINAL:
                    // Ordinal cuts: ABCD -> AB, ABC, ABCD, BC, BCD, CD
                    iMaxEndWindow = std::min(endWindow.getEnd(), startWindow.getEnd() + window.maximum());
                    for (iWindowEnd=endWindow.getStart(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
                        iMinWindowStart = std::max(iWindowEnd - window.maximum(), startWindow.getStart());
                        iWindowStart = std::min(startWindow.getEnd(), iWindowEnd - window.minimum());
                        for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
                            for (size_t i=0; i < thisNode.getChildren().size() - 1; ++i) {
                                const NodeStructure& firstChildNode(*(nodes.at(thisNode.getChildren().at(i))));
                                const SimulationNode& firstSimChildNode(_treeSimNodes.at(thisNode.getChildren().at(i)));
                                //buffer = firstChildNode.getIdentifier().c_str();
                                NodeStructure::count_t branchWindow = firstSimChildNode.getBrC_C()[iWindowStart] - firstSimChildNode.getBrC_C()[iWindowEnd + 1];
                                NodeStructure::expected_t branchExpected = firstChildNode.getBrN_C()[iWindowStart] - firstChildNode.getBrN_C()[iWindowEnd + 1];
                                for (size_t j=i+1; j < thisNode.getChildren().size(); ++j) {
                                    //const NodeStructure& childNode(*(nodes.at(thisNode.getChildren().at(j))));
                                    const NodeStructure& childNode(*(nodes.at(thisNode.getChildren().at(j))));
                                    const SimulationNode& childSimNode(_treeSimNodes.at(thisNode.getChildren().at(j)));
                                    //buffer += ",";
                                    //buffer += childNode.getIdentifier();
                                    NodeStructure::count_t childBranchC = childSimNode.getBrC_C()[iWindowStart] - childSimNode.getBrC_C()[iWindowEnd + 1];
                                    branchWindow += childBranchC;
                                    branchExpected += childNode.getBrN_C()[iWindowStart] - childNode.getBrN_C()[iWindowEnd + 1];
                                    //printf("Evaluating cut [%s]\n", buffer.c_str());
                                    simLogLikelihood = std::max(simLogLikelihood, _loglikelihood->LogLikelihood(branchWindow, branchExpected));
                                    //++hits; printf("hits %d\n", hits);
                                }
                            }
                        }
                    }
                    break;
                case Parameters::PAIRS:
                    // Pair cuts: ABCD -> AB, AC, AD, BC, BD, CD
                    iMaxEndWindow = std::min(endWindow.getEnd(), startWindow.getEnd() + window.maximum());
                    for (iWindowEnd=endWindow.getStart(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
                        iMinWindowStart = std::max(iWindowEnd - window.maximum(), startWindow.getStart());
                        iWindowStart = std::min(startWindow.getEnd(), iWindowEnd - window.minimum());
                        for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
                            for (size_t i=0; i < thisNode.getChildren().size() - 1; ++i) {
                                const NodeStructure& startChildNode(*(nodes.at(thisNode.getChildren().at(i))));
                                const SimulationNode& startSimChildNode(_treeSimNodes.at(thisNode.getChildren().at(i)));
                                NodeStructure::count_t startBranchWindow = startSimChildNode.getBrC_C()[iWindowStart] - startSimChildNode.getBrC_C()[iWindowEnd + 1];
                                NodeStructure::expected_t startBranchExpected = startChildNode.getBrN_C()[iWindowStart] - startChildNode.getBrN_C()[iWindowEnd + 1];
                                for (size_t j=i+1; j < thisNode.getChildren().size(); ++j) {
                                    const NodeStructure& stopChildNode(*(nodes.at(thisNode.getChildren().at(j))));
                                    const SimulationNode& stopSimChildNode(_treeSimNodes.at(thisNode.getChildren().at(j)));
                                    //printf("Evaluating cut [%s,%s]\n", startChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                    NodeStructure::count_t stopBranchWindow = stopSimChildNode.getBrC_C()[iWindowStart] - stopSimChildNode.getBrC_C()[iWindowEnd + 1];
                                    NodeStructure::expected_t stopBranchExpected = stopChildNode.getBrN_C()[iWindowStart] - stopChildNode.getBrN_C()[iWindowEnd + 1];
                                    simLogLikelihood = std::max(simLogLikelihood, _loglikelihood->LogLikelihood(startBranchWindow + stopBranchWindow, startBranchExpected + stopBranchExpected));
                                    //++hits; printf("hits %d\n", hits);
                                }
                            }
                        }
                    }
                    break;
                case Parameters::TRIPLETS:
                    // Triple cuts: ABCD -> AB, AC, ABC, AD, ABD, ACD, BC, BD, BCD, CD
                    iMaxEndWindow = std::min(endWindow.getEnd(), startWindow.getEnd() + window.maximum());
                    for (iWindowEnd=endWindow.getStart(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
                        iMinWindowStart = std::max(iWindowEnd - window.maximum(), startWindow.getStart());
                        iWindowStart = std::min(startWindow.getEnd(), iWindowEnd - window.minimum());
                        for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
                            for (size_t i=0; i < thisNode.getChildren().size() - 1; ++i) {
                                const NodeStructure& startChildNode(*(nodes.at(thisNode.getChildren().at(i))));
                                const SimulationNode& startSimChildNode(_treeSimNodes.at(thisNode.getChildren().at(i)));
                                NodeStructure::count_t startBranchWindow = startSimChildNode.getBrC_C()[iWindowStart] - startSimChildNode.getBrC_C()[iWindowEnd + 1];
                                NodeStructure::expected_t startBranchExpected = startChildNode.getBrN_C()[iWindowStart] - startChildNode.getBrN_C()[iWindowEnd + 1];
                                for (size_t j=i+1; j < thisNode.getChildren().size(); ++j) {
                                    const NodeStructure& stopChildNode(*(nodes.at(thisNode.getChildren().at(j))));
                                    const SimulationNode& stopSimChildNode(_treeSimNodes.at(thisNode.getChildren().at(j)));
                                    //printf("Evaluating cut [%s,%s]\n", startChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                    NodeStructure::count_t stopBranchWindow = stopSimChildNode.getBrC_C()[iWindowStart] - stopSimChildNode.getBrC_C()[iWindowEnd + 1];
                                    NodeStructure::expected_t stopBranchExpected = stopChildNode.getBrN_C()[iWindowStart] - stopChildNode.getBrN_C()[iWindowEnd + 1];
                                    //printf("Evaluating cut [%s,%s]\n", startChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                    simLogLikelihood = std::max(simLogLikelihood,
                                                                _loglikelihood->LogLikelihood(startBranchWindow + stopBranchWindow, startBranchExpected + stopBranchExpected));
                                    //++hits;printf("hits %d\n", hits);
                                    for (size_t k=i+1; k < j; ++k) {
                                        const NodeStructure& middleChildNode(*(nodes.at(thisNode.getChildren().at(k))));
                                        const SimulationNode& middleSimChildNode(_treeSimNodes.at(thisNode.getChildren().at(k)));
                                        //printf("Evaluating cut [%s,%s,%s]\n", startChildNode.getIdentifier().c_str(), middleChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                        NodeStructure::count_t middleBranchWindow = middleSimChildNode.getBrC_C()[iWindowStart] - middleSimChildNode.getBrC_C()[iWindowEnd + 1];
                                        NodeStructure::expected_t middleBranchExpected = middleChildNode.getBrN_C()[iWindowStart] - middleChildNode.getBrN_C()[iWindowEnd + 1];
                                        //printf("Evaluating cut [%s,%s,%s]\n", startChildNode.getIdentifier().c_str(), middleChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                        simLogLikelihood = std::max(simLogLikelihood,
                                                                    _loglikelihood->LogLikelihood(startBranchWindow + middleBranchWindow + stopBranchWindow,
                                                                                                  startBranchExpected + middleBranchExpected + stopBranchExpected));
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
