//******************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//******************************************************************************
#include "MonteCarloSimFunctor.h"
#include "PrjException.h"
#include "WindowLength.h"
#include "DataSource.h"

AbstractMeasureList::AbstractMeasureList(const ScanRunner & scanRunner, Loglikelihood_t loglikelihood) :
    _scanRunner(scanRunner), _loglikelihood(loglikelihood) {}

/* Returns new AbstractMeasureList object based on settings. */
AbstractMeasureList * AbstractMeasureList::getNewMeasureList(const ScanRunner& scanner, Loglikelihood_t loglikelihood) {
    switch (scanner.getParameters().getScanRateType()) {
        case Parameters::LOWRATE: return new MaximumMeasureList(scanner, loglikelihood);
        case Parameters::HIGHORLOWRATE: return new MinimumMaximumMeasureList(scanner, loglikelihood);
        case Parameters::HIGHRATE: return new MinimumMeasureList(scanner, loglikelihood);
        default: throw prg_error("Unknown scan rate type'%d'.", "getNewMeasureList()", scanner.getParameters().getScanRateType());
    }
}

/* constructor */
MCSimSuccessiveFunctor::MCSimSuccessiveFunctor(boost::mutex& mutex,
                                               boost::shared_ptr<AbstractRandomizer> randomizer,
                                               const ScanRunner& scanRunner) : _mutex(mutex), _randomizer(randomizer), _scanRunner(scanRunner) {
    // TODO: Eventually this will need refactoring once we implement multiple data time ranges.
    const Parameters& parameters = _scanRunner.getParameters();
    size_t daysInDataTimeRange = Parameters::isTemporalScanType(parameters.getScanType()) ?parameters.getDataTimeRangeSet().getTotalDaysAcrossRangeSets() + 1 : 1;
    for (auto node: _scanRunner.getNodes())
        _treeSimNodes.push_back(SimulationNode(daysInDataTimeRange, node->getLevel()));
    //_treeSimNodes.resize(_scanRunner.getNodes().size(), SimulationNode(daysInDataTimeRange));
    _loglikelihood.reset(AbstractLoglikelihood::getNewLoglikelihood(_scanRunner.getParameters(), _scanRunner.getTotalC(), _scanRunner.getTotalN(), _scanRunner.isCensoredData()));

    if ((parameters.getScanType() == Parameters::TREETIME && parameters.getConditionalType() == Parameters::NODEANDTIME) ||
        (parameters.getScanType() == Parameters::TIMEONLY && parameters.isPerformingDayOfWeekAdjustment()) ||
        (parameters.getScanType() == Parameters::TREETIME && parameters.getConditionalType() == Parameters::NODE && parameters.isPerformingDayOfWeekAdjustment()))
        _measure_list.reset(AbstractMeasureList::getNewMeasureList(_scanRunner, _loglikelihood));
}

MCSimSuccessiveFunctor::result_type MCSimSuccessiveFunctor::operator() (MCSimSuccessiveFunctor::param_type const & param) {
    result_type temp_result;
    try {
        const Parameters& parameters = _scanRunner.getParameters();
        if ((parameters.getScanType() == Parameters::TREETIME && parameters.getConditionalType() == Parameters::NODEANDTIME) ||
            (parameters.getScanType() == Parameters::TIMEONLY && parameters.isPerformingDayOfWeekAdjustment()) ||
            (parameters.getScanType() == Parameters::TREETIME && parameters.getConditionalType() == Parameters::NODE && parameters.isPerformingDayOfWeekAdjustment()))
            temp_result.dSuccessfulResult = scanTreeTemporalConditionNodeTime(param);
        else if (parameters.getModelType() == Parameters::UNIFORM)
            if (_scanRunner.isCensoredData())
                temp_result.dSuccessfulResult = scanTreeTemporalConditionNodeCensored(param);
            else
                temp_result.dSuccessfulResult = scanTreeTemporalConditionNode(param);
        else if (parameters.getModelType() == Parameters::BERNOULLI_TIME)
            temp_result.dSuccessfulResult = scanTreeTemporalConditionNodeCensored(param);
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

/* Returns true if NodeStructure/SimulationNode is evaluated in scanning processing. */
bool MCSimSuccessiveFunctor::isEvaluated(const NodeStructure& node, const SimulationNode& simNode) const {
    // If the node branch does not have the minimum number of cases in branch, it is not evaluated.
    if (static_cast<unsigned int>(simNode.getBrC()) < _scanRunner.getNodeEvaluationMinimum()) return false;
    if (_scanRunner.getParameters().getScanType() != Parameters::TIMEONLY && _scanRunner.getParameters().getRestrictTreeLevels())
        return std::find(_scanRunner.getParameters().getRestrictedTreeLevels().begin(), 
                         _scanRunner.getParameters().getRestrictedTreeLevels().end(), 
                         node.getLevel()) == _scanRunner.getParameters().getRestrictedTreeLevels().end();
    return true;
}

/* This function randomizes data and scans tree for either the Poisson or Bernoulli model. */
MCSimSuccessiveFunctor::successful_result_type MCSimSuccessiveFunctor::scanTree(MCSimSuccessiveFunctor::param_type const & param) {
    //randomize data
    int TotalSimC = _randomizer.get()->RandomizeData(param, _scanRunner.getNodes(), _mutex, _treeSimNodes);

    //--------------------- SCANNING THE TREE, SIMULATIONS -------------------------
    const ScanRunner::NodeStructureContainer_t& nodes = _scanRunner.getNodes();
    int sumBranchC = 0, sumBranchC2 = 0;
    double simLogLikelihood = -std::numeric_limits<double>::max(), minimum_cases = _scanRunner.getNodeEvaluationMinimum(), sumBranchN=0.0;
    for (size_t i=0; i < nodes.size(); ++i) {
        const NodeStructure& thisNode(*(nodes[i]));
        if (isEvaluated(thisNode, _treeSimNodes[i])) {
            // always do simple cut
            simLogLikelihood = std::max(simLogLikelihood, _loglikelihood->LogLikelihood(_treeSimNodes[i].getBrC(), nodes[i]->getBrN()));

            Parameters::CutType cutType = thisNode.getChildren().size() >= 2 ? thisNode.getCutType() : Parameters::SIMPLE;
            switch (cutType) {
                case Parameters::SIMPLE: break; // already done
                case Parameters::ORDINAL:
                    // Ordinal cuts: ABCD -> AB, ABC, ABCD, BC, BCD, CD
                    for (size_t i=0; i < thisNode.getChildren().size() - 1; ++i) {
                        const NodeStructure& firstChildNode(*(thisNode.getChildren()[i]));
                        //buffer = firstChildNode.getIdentifier().c_str();
                        sumBranchC = _treeSimNodes[i].getBrC();
                        sumBranchN = firstChildNode.getBrN();
                        for (size_t j=i+1; j < thisNode.getChildren().size(); ++j) {
                            const NodeStructure& childNode(*(thisNode.getChildren()[j]));
                            //buffer += ",";
                            //buffer += childNode.getIdentifier();
                            sumBranchC += _treeSimNodes[static_cast<size_t>(childNode.getID())].getBrC();
                            sumBranchN += childNode.getBrN();
                            //printf("Evaluating cut [%s]\n", buffer.c_str());
                            if (sumBranchC >= minimum_cases)
                                simLogLikelihood = std::max(simLogLikelihood, _loglikelihood->LogLikelihood(sumBranchC, sumBranchN));
                            //++hits; printf("hits %d\n", hits);
                        }
                    } break;
                case Parameters::PAIRS:
                    // Pair cuts: ABCD -> AB, AC, AD, BC, BD, CD
                    for (size_t i=0; i < thisNode.getChildren().size() - 1; ++i) {
                        const NodeStructure& startChildNode(*(thisNode.getChildren()[i]));
                        for (size_t j=i+1; j < thisNode.getChildren().size(); ++j) {
                            const NodeStructure& stopChildNode(*(thisNode.getChildren()[j]));
                            //printf("Evaluating cut [%s,%s]\n", startChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                            sumBranchC = _treeSimNodes[static_cast<size_t>(startChildNode.getID())].getBrC() + _treeSimNodes[static_cast<size_t>(stopChildNode.getID())].getBrC();
                            if (sumBranchC >= minimum_cases)
                                simLogLikelihood = std::max(simLogLikelihood, _loglikelihood->LogLikelihood(sumBranchC, startChildNode.getBrN() + stopChildNode.getBrN()));
                            //++hits; printf("hits %d\n", hits);
                        }
                    } break;
                case Parameters::TRIPLETS:
                    // Triple cuts: ABCD -> AB, AC, ABC, AD, ABD, ACD, BC, BD, BCD, CD
                    for (size_t i=0; i < thisNode.getChildren().size() - 1; ++i) {
                        const NodeStructure& startChildNode(*(thisNode.getChildren()[i]));
                        for (size_t j=i+1; j < thisNode.getChildren().size(); ++j) {
                            const NodeStructure& stopChildNode(*(thisNode.getChildren()[j]));
                            //printf("Evaluating cut [%s,%s]\n", startChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                            sumBranchC = _treeSimNodes[static_cast<size_t>(startChildNode.getID())].getBrC() + _treeSimNodes[static_cast<size_t>(stopChildNode.getID())].getBrC();
                            if (sumBranchC >= minimum_cases)
                                simLogLikelihood = std::max(simLogLikelihood, _loglikelihood->LogLikelihood(sumBranchC, startChildNode.getBrN() + stopChildNode.getBrN()));
                            //++hits;printf("hits %d\n", hits);
                            for (size_t k=i+1; k < j; ++k) {
                                const NodeStructure& middleChildNode(*(thisNode.getChildren()[k]));
                                //printf("Evaluating cut [%s,%s,%s]\n", startChildNode.getIdentifier().c_str(), middleChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                sumBranchC2 = sumBranchC + _treeSimNodes[static_cast<size_t>(middleChildNode.getID())].getBrC();
                                if (sumBranchC2 >= minimum_cases)
                                    simLogLikelihood = std::max(simLogLikelihood, _loglikelihood->LogLikelihood(sumBranchC2, startChildNode.getBrN() + middleChildNode.getBrN() + stopChildNode.getBrN()));
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
    DataTimeRange startWindow(_scanRunner.temporalStartRange().getStart() + idxAdditive, _scanRunner.temporalStartRange().getEnd() + idxAdditive),
                  endWindow(_scanRunner.temporalEndRange().getStart() + idxAdditive, _scanRunner.temporalEndRange().getEnd() + idxAdditive);
    // Define the minimum and maximum window lengths.
    boost::shared_ptr<AbstractWindowLength> window(_scanRunner.getNewWindowLength());
    int  iWindowStart, iMinWindowStart, iWindowEnd, iMaxEndWindow;

    const ScanRunner::NodeStructureContainer_t& nodes = _scanRunner.getNodes();
    int branchSum = 0, branchSum2 = 0;
    double simLogLikelihood = -std::numeric_limits<double>::max(), minimum_cases = _scanRunner.getNodeEvaluationMinimum();
    for (size_t i=0; i < nodes.size(); ++i) {
        const NodeStructure& thisNode(*(nodes[i]));
        const SimulationNode& thisSimNode(_treeSimNodes[i]);
        if (isEvaluated(thisNode, thisSimNode)) {

            // always do simple cut
            iMaxEndWindow = std::min(endWindow.getEnd(), startWindow.getEnd() + window->maximum());
            for (iWindowEnd=endWindow.getStart(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
                window->windowstart(startWindow, iWindowEnd, iMinWindowStart, iWindowStart);
                for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
                    NodeStructure::count_t branchWindow = thisSimNode.getBrC_C()[iWindowStart] - thisSimNode.getBrC_C()[iWindowEnd + 1];
                    if (branchWindow >= minimum_cases)
                        simLogLikelihood = std::max(simLogLikelihood,
                            _loglikelihood->LogLikelihood(branchWindow, static_cast<NodeStructure::expected_t>(thisSimNode.getBrC()), iWindowEnd - iWindowStart + 1)
                        );
                }
            }

            Parameters::CutType cutType = thisNode.getChildren().size() >= 2 ? thisNode.getCutType() : Parameters::SIMPLE;
            switch (cutType) {
                case Parameters::SIMPLE: break; // already done
                case Parameters::ORDINAL:
                    // Ordinal cuts: ABCD -> AB, ABC, ABCD, BC, BCD, CD
                    iMaxEndWindow = std::min(endWindow.getEnd(), startWindow.getEnd() + window->maximum());
                    for (iWindowEnd=endWindow.getStart(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
                        window->windowstart(startWindow, iWindowEnd, iMinWindowStart, iWindowStart);
                        for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
                            for (size_t i=0; i < thisNode.getChildren().size() - 1; ++i) {
                                const SimulationNode& firstSimChildNode(_treeSimNodes[thisNode.getChildren()[i]->getID()]);
                                //buffer = firstChildNode.getIdentifier().c_str();
                                NodeStructure::count_t branchWindow = firstSimChildNode.getBrC_C()[iWindowStart] - firstSimChildNode.getBrC_C()[iWindowEnd + 1];
                                NodeStructure::expected_t branchSum = static_cast<NodeStructure::expected_t>(firstSimChildNode.getBrC());
                                for (size_t j=i+1; j < thisNode.getChildren().size(); ++j) {
                                    const SimulationNode& childSimNode(_treeSimNodes[thisNode.getChildren()[j]->getID()]);
                                    //buffer += ",";
                                    //buffer += childNode.getIdentifier();
                                    branchWindow += childSimNode.getBrC_C()[iWindowStart] - childSimNode.getBrC_C()[iWindowEnd + 1];
                                    branchSum += static_cast<NodeStructure::expected_t>(childSimNode.getBrC());
                                    //printf("Evaluating cut [%s]\n", buffer.c_str());
                                    if (branchWindow >= minimum_cases)
                                        simLogLikelihood = std::max(simLogLikelihood,
                                            _loglikelihood->LogLikelihood(branchWindow, branchSum, iWindowEnd - iWindowStart + 1)
                                        );
                                    //++hits; printf("hits %d\n", hits);
                                }
                            }
                        }
                    }
                    break;
                case Parameters::PAIRS:
                    // Pair cuts: ABCD -> AB, AC, AD, BC, BD, CD
                    iMaxEndWindow = std::min(endWindow.getEnd(), startWindow.getEnd() + window->maximum());
                    for (iWindowEnd=endWindow.getStart(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
                        window->windowstart(startWindow, iWindowEnd, iMinWindowStart, iWindowStart);
                        for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
                            for (size_t i=0; i < thisNode.getChildren().size() - 1; ++i) {
                                const SimulationNode& startSimChildNode(_treeSimNodes[thisNode.getChildren()[i]->getID()]);
                                NodeStructure::count_t startBranchWindow = startSimChildNode.getBrC_C()[iWindowStart] - startSimChildNode.getBrC_C()[iWindowEnd + 1];
                                NodeStructure::expected_t startBranchSum = static_cast<NodeStructure::expected_t>(startSimChildNode.getBrC());
                                for (size_t j=i+1; j < thisNode.getChildren().size(); ++j) {
                                    const SimulationNode& stopSimChildNode(_treeSimNodes[thisNode.getChildren()[j]->getID()]);
                                    //printf("Evaluating cut [%s,%s]\n", startChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                    branchSum = startBranchWindow + (stopSimChildNode.getBrC_C()[iWindowStart] - stopSimChildNode.getBrC_C()[iWindowEnd + 1]);
                                    if (branchSum >= minimum_cases)
                                        simLogLikelihood = std::max(simLogLikelihood,
                                            _loglikelihood->LogLikelihood(branchSum, startBranchSum + static_cast<NodeStructure::expected_t>(stopSimChildNode.getBrC()), iWindowEnd - iWindowStart + 1)
                                        );
                                    //++hits; printf("hits %d\n", hits);
                                }
                            }
                        }
                    }
                    break;
                case Parameters::TRIPLETS:
                    // Triple cuts: ABCD -> AB, AC, ABC, AD, ABD, ACD, BC, BD, BCD, CD
                    iMaxEndWindow = std::min(endWindow.getEnd(), startWindow.getEnd() + window->maximum());
                    for (iWindowEnd=endWindow.getStart(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
                        window->windowstart(startWindow, iWindowEnd, iMinWindowStart, iWindowStart);
                        for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
                            for (size_t i=0; i < thisNode.getChildren().size() - 1; ++i) {
                                const SimulationNode& startSimChildNode(_treeSimNodes[thisNode.getChildren()[i]->getID()]);
                                NodeStructure::count_t startBranchWindow = startSimChildNode.getBrC_C()[iWindowStart] - startSimChildNode.getBrC_C()[iWindowEnd + 1];
                                NodeStructure::expected_t startBranchSum = static_cast<NodeStructure::expected_t>(startSimChildNode.getBrC());
                                for (size_t j=i+1; j < thisNode.getChildren().size(); ++j) {
                                    const SimulationNode& stopSimChildNode(_treeSimNodes[thisNode.getChildren()[j]->getID()]);
                                    //printf("Evaluating cut [%s,%s]\n", startChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                    branchSum = startBranchWindow + (stopSimChildNode.getBrC_C()[iWindowStart] - stopSimChildNode.getBrC_C()[iWindowEnd + 1]);
                                    //printf("Evaluating cut [%s,%s]\n", startChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                    if (branchSum >= minimum_cases)
                                        simLogLikelihood = std::max(simLogLikelihood,
                                            _loglikelihood->LogLikelihood(branchSum, startBranchSum + static_cast<NodeStructure::expected_t>(stopSimChildNode.getBrC()), iWindowEnd - iWindowStart + 1)
                                        );
                                    //++hits;printf("hits %d\n", hits);
                                    for (size_t k=i+1; k < j; ++k) {
                                        const SimulationNode& middleSimChildNode(_treeSimNodes[thisNode.getChildren()[k]->getID()]);
                                        //printf("Evaluating cut [%s,%s,%s]\n", startChildNode.getIdentifier().c_str(), middleChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                        branchSum2 = branchSum + (middleSimChildNode.getBrC_C()[iWindowStart] - middleSimChildNode.getBrC_C()[iWindowEnd + 1]);
                                        //printf("Evaluating cut [%s,%s,%s]\n", startChildNode.getIdentifier().c_str(), middleChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                        if (branchSum2 >= minimum_cases)
                                            simLogLikelihood = std::max(simLogLikelihood,
                                                _loglikelihood->LogLikelihood(branchSum2,
                                                    startBranchSum + static_cast<NodeStructure::expected_t>(middleSimChildNode.getBrC()) + static_cast<NodeStructure::expected_t>(stopSimChildNode.getBrC()),
                                                    iWindowEnd - iWindowStart + 1
                                                )
                                            );
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
MCSimSuccessiveFunctor::successful_result_type MCSimSuccessiveFunctor::scanTreeTemporalConditionNodeCensored(MCSimSuccessiveFunctor::param_type const & param) {
    //randomize data
    int TotalSimC = _randomizer.get()->RandomizeData(param, _scanRunner.getNodes(), _mutex, _treeSimNodes);

    //--------------------- SCANNING THE TREE, SIMULATIONS -------------------------
    DataTimeRange::index_t idxAdditive = _scanRunner.getZeroTranslationAdditive();
    // Define the start and end windows with the zero index offset already incorporated.
    DataTimeRange startWindow(_scanRunner.temporalStartRange().getStart() + idxAdditive,
                              _scanRunner.temporalStartRange().getEnd() + idxAdditive),
                  endWindow(_scanRunner.temporalEndRange().getStart() + idxAdditive,
                            _scanRunner.temporalEndRange().getEnd() + idxAdditive);
    // Define the minimum and maximum window lengths.
    boost::shared_ptr<AbstractWindowLength> window(_scanRunner.getNewWindowLength());
    int  iWindowStart, iMinWindowStart, iWindowEnd, iMaxEndWindow;

    const ScanRunner::NodeStructureContainer_t& nodes = _scanRunner.getNodes();
    int branchWindowSum = 0, branchWindowSum2 = 0;
    double simLogLikelihood = -std::numeric_limits<double>::max(), minimum_cases = _scanRunner.getNodeEvaluationMinimum();
    for (size_t i = 0; i < nodes.size(); ++i) {
        const NodeStructure& thisNode(*(nodes[i]));
        const SimulationNode& thisSimNode(_treeSimNodes[i]);
        if (isEvaluated(thisNode, thisSimNode)) {

            // always do simple cut
            iMaxEndWindow = std::min(endWindow.getEnd(), startWindow.getEnd() + window->maximum());
            for (iWindowEnd = endWindow.getStart(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
                window->windowstart(startWindow, iWindowEnd, iMinWindowStart, iWindowStart, thisNode.getMinCensoredBr());
                for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
                    branchWindowSum = thisSimNode.getBrC_C()[iWindowStart] - thisSimNode.getBrC_C()[iWindowEnd + 1];
                    if (branchWindowSum >= minimum_cases)
                        simLogLikelihood = std::max(simLogLikelihood,
                            _loglikelihood->LogLikelihood(branchWindowSum, thisNode.getBrN_C()[iWindowStart] - thisNode.getBrN_C()[iWindowEnd + 1], thisSimNode.getBrC(), thisNode.getBrN())
                        );
                }
            }

            Parameters::CutType cutType = thisNode.getChildren().size() >= 2 ? thisNode.getCutType() : Parameters::SIMPLE;
            switch (cutType) {
            case Parameters::SIMPLE: break; // already done
            case Parameters::ORDINAL:
                // Ordinal cuts: ABCD -> AB, ABC, ABCD, BC, BCD, CD
                iMaxEndWindow = std::min(endWindow.getEnd(), startWindow.getEnd() + window->maximum());
                for (iWindowEnd = endWindow.getStart(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
                    window->windowstart(startWindow, iWindowEnd, iMinWindowStart, iWindowStart, thisNode.getMinCensoredBr());
                    for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
                        for (size_t i = 0; i < thisNode.getChildren().size() - 1; ++i) {
                            const NodeStructure& firstChildNode(*(thisNode.getChildren()[i]));
                            const SimulationNode& firstSimChildNode(_treeSimNodes[firstChildNode.getID()]);
                            //buffer = firstChildNode.getIdentifier().c_str();
                            NodeStructure::count_t branchWindow = firstSimChildNode.getBrC_C()[iWindowStart] - firstSimChildNode.getBrC_C()[iWindowEnd + 1];
                            NodeStructure::expected_t branchSum = firstChildNode.getBrN_C()[iWindowStart] - firstChildNode.getBrN_C()[iWindowEnd + 1];
                            NodeStructure::count_t branchCTotal = firstSimChildNode.getBrC();
                            NodeStructure::expected_t branchNTotal = firstChildNode.getBrN();
                            for (size_t j = i + 1; j < thisNode.getChildren().size(); ++j) {
                                const NodeStructure& childNode(*(thisNode.getChildren()[j]));
                                const SimulationNode& childSimNode(_treeSimNodes[childNode.getID()]);
                                //buffer += ",";
                                //buffer += childNode.getIdentifier();
                                branchWindow += childSimNode.getBrC_C()[iWindowStart] - childSimNode.getBrC_C()[iWindowEnd + 1];
                                branchSum += childNode.getBrN_C()[iWindowStart] - childNode.getBrN_C()[iWindowEnd + 1];
                                branchCTotal += childSimNode.getBrC();
                                branchNTotal += childNode.getBrN();
                                //printf("Evaluating cut [%s]\n", buffer.c_str());
                                if (branchWindow >= minimum_cases)
                                    simLogLikelihood = std::max(simLogLikelihood, _loglikelihood->LogLikelihood(branchWindow, branchSum, branchCTotal, branchNTotal));
                                //++hits; printf("hits %d\n", hits);
                            }
                        }
                    }
                }
                break;
            case Parameters::PAIRS:
                // Pair cuts: ABCD -> AB, AC, AD, BC, BD, CD
                iMaxEndWindow = std::min(endWindow.getEnd(), startWindow.getEnd() + window->maximum());
                for (iWindowEnd = endWindow.getStart(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
                    window->windowstart(startWindow, iWindowEnd, iMinWindowStart, iWindowStart, thisNode.getMinCensoredBr());
                    for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
                        for (size_t i = 0; i < thisNode.getChildren().size() - 1; ++i) {
                            const NodeStructure& startChildNode(*(thisNode.getChildren()[i]));
                            const SimulationNode& startSimChildNode(_treeSimNodes[startChildNode.getID()]);
                            NodeStructure::count_t startBranchWindow = startSimChildNode.getBrC_C()[iWindowStart] - startSimChildNode.getBrC_C()[iWindowEnd + 1];
                            NodeStructure::expected_t startBranchSum = startChildNode.getBrN_C()[iWindowStart] - startChildNode.getBrN_C()[iWindowEnd + 1];
                            NodeStructure::count_t startBranchTotalC = startSimChildNode.getBrC();
                            NodeStructure::expected_t startBranchTotalN = startChildNode.getBrN();
                            for (size_t j = i + 1; j < thisNode.getChildren().size(); ++j) {
                                const NodeStructure& stopChildNode(*(thisNode.getChildren()[j]));
                                const SimulationNode& stopSimChildNode(_treeSimNodes[stopChildNode.getID()]);
                                //printf("Evaluating cut [%s,%s]\n", startChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                branchWindowSum = startBranchWindow + (stopSimChildNode.getBrC_C()[iWindowStart] - stopSimChildNode.getBrC_C()[iWindowEnd + 1]);
                                if (branchWindowSum >= minimum_cases)
                                    simLogLikelihood = std::max(simLogLikelihood,
                                        _loglikelihood->LogLikelihood(branchWindowSum,
                                            startBranchSum + (stopChildNode.getBrN_C()[iWindowStart] - stopChildNode.getBrN_C()[iWindowEnd + 1]), startBranchTotalC + stopSimChildNode.getBrC(),
                                            startBranchTotalN + stopChildNode.getBrN()
                                        )
                                    );
                                //++hits; printf("hits %d\n", hits);
                            }
                        }
                    }
                }
                break;
            case Parameters::TRIPLETS:
                // Triple cuts: ABCD -> AB, AC, ABC, AD, ABD, ACD, BC, BD, BCD, CD
                iMaxEndWindow = std::min(endWindow.getEnd(), startWindow.getEnd() + window->maximum());
                for (iWindowEnd = endWindow.getStart(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
                    window->windowstart(startWindow, iWindowEnd, iMinWindowStart, iWindowStart, thisNode.getMinCensoredBr());
                    for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
                        for (size_t i = 0; i < thisNode.getChildren().size() - 1; ++i) {
                            const NodeStructure& startChildNode(*(thisNode.getChildren()[i]));
                            const SimulationNode& startSimChildNode(_treeSimNodes[startChildNode.getID()]);
                            NodeStructure::count_t startBranchWindow = startSimChildNode.getBrC_C()[iWindowStart] - startSimChildNode.getBrC_C()[iWindowEnd + 1];
                            NodeStructure::expected_t startBranchSum = startChildNode.getBrN_C()[iWindowStart] - startChildNode.getBrN_C()[iWindowEnd + 1];
                            NodeStructure::count_t startBranchTotalC = startSimChildNode.getBrC();
                            NodeStructure::expected_t startBranchTotalN = startChildNode.getBrN();
                            for (size_t j = i + 1; j < thisNode.getChildren().size(); ++j) {
                                const NodeStructure& stopChildNode(*(thisNode.getChildren()[j]));
                                const SimulationNode& stopSimChildNode(_treeSimNodes[stopChildNode.getID()]);
                                //printf("Evaluating cut [%s,%s]\n", startChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                branchWindowSum = startBranchWindow + (stopSimChildNode.getBrC_C()[iWindowStart] - stopSimChildNode.getBrC_C()[iWindowEnd + 1]);
                                NodeStructure::expected_t stopBranchSum = stopChildNode.getBrN_C()[iWindowStart] - stopChildNode.getBrN_C()[iWindowEnd + 1];
                                NodeStructure::count_t stopBranchTotalC = stopSimChildNode.getBrC();
                                NodeStructure::expected_t stopBranchTotalN = stopChildNode.getBrN();
                                //printf("Evaluating cut [%s,%s]\n", startChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                if (branchWindowSum >= minimum_cases)
                                    simLogLikelihood = std::max(simLogLikelihood,
                                        _loglikelihood->LogLikelihood(branchWindowSum,
                                            startBranchSum + stopBranchSum, startBranchTotalC + stopBranchTotalC,
                                            startBranchTotalN + stopBranchTotalN
                                        )
                                    );
                                //++hits;printf("hits %d\n", hits);
                                for (size_t k = i + 1; k < j; ++k) {
                                    const NodeStructure& middleChildNode(*(thisNode.getChildren()[k]));
                                    const SimulationNode& middleSimChildNode(_treeSimNodes[middleChildNode.getID()]);
                                    //printf("Evaluating cut [%s,%s,%s]\n", startChildNode.getIdentifier().c_str(), middleChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                    branchWindowSum2 = branchWindowSum + (middleSimChildNode.getBrC_C()[iWindowStart] - middleSimChildNode.getBrC_C()[iWindowEnd + 1]);
                                    NodeStructure::expected_t middleBranchSum = middleChildNode.getBrN_C()[iWindowStart] - middleChildNode.getBrN_C()[iWindowEnd + 1];
                                    //printf("Evaluating cut [%s,%s,%s]\n", startChildNode.getIdentifier().c_str(), middleChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                    if (branchWindowSum2 >= minimum_cases)
                                        simLogLikelihood = std::max(simLogLikelihood,
                                            _loglikelihood->LogLikelihood(branchWindowSum2,
                                                startBranchSum + middleBranchSum + stopBranchSum,
                                                startBranchTotalC + middleSimChildNode.getBrC() + stopBranchTotalC,
                                                startBranchTotalN + middleChildNode.getBrN() + stopBranchTotalN
                                            )
                                        );
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
    return std::make_pair(simLogLikelihood, TotalSimC);
}

/* This function randomizes data and scans tree for either the temporal model. */
MCSimSuccessiveFunctor::successful_result_type MCSimSuccessiveFunctor::scanTreeTemporalConditionNodeTime(MCSimSuccessiveFunctor::param_type const & param) {
    //randomize data
    int TotalSimC = _randomizer.get()->RandomizeData(param, _scanRunner.getNodes(), _mutex, _treeSimNodes);
    _measure_list->initialize();

    //--------------------- SCANNING THE TREE, SIMULATIONS -------------------------
    DataTimeRange::index_t idxAdditive = _scanRunner.getZeroTranslationAdditive();
    // Define the start and end windows with the zero index offset already incorporated.
    DataTimeRange startWindow(_scanRunner.temporalStartRange().getStart() + idxAdditive, _scanRunner.temporalStartRange().getEnd() + idxAdditive),
                  endWindow(_scanRunner.temporalEndRange().getStart() + idxAdditive, _scanRunner.temporalEndRange().getEnd() + idxAdditive);
    // Define the minimum and maximum window lengths.

    boost::shared_ptr<AbstractWindowLength> window(_scanRunner.getNewWindowLength());
    int  iWindowStart, iMinWindowStart, iWindowEnd, iMaxEndWindow;

    const ScanRunner::NodeStructureContainer_t& nodes = _scanRunner.getNodes();
    for (size_t i=0; i < nodes.size(); ++i) {
        const NodeStructure& thisNode(*(nodes[i]));
        const SimulationNode& thisSimNode(_treeSimNodes[i]);
        if (isEvaluated(thisNode, thisSimNode)) {

            // always do simple cut
            iMaxEndWindow = std::min(endWindow.getEnd(), startWindow.getEnd() + window->maximum());
            for (iWindowEnd=endWindow.getStart(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
                window->windowstart(startWindow, iWindowEnd, iMinWindowStart, iWindowStart);
                for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
                    NodeStructure::count_t branchWindow = thisSimNode.getBrC_C()[iWindowStart] - thisSimNode.getBrC_C()[iWindowEnd + 1];
                    NodeStructure::expected_t branchExpected = thisNode.getBrN_C()[iWindowStart] - thisNode.getBrN_C()[iWindowEnd + 1];
                    _measure_list->add(branchWindow, branchExpected);
                }
            }

            Parameters::CutType cutType = thisNode.getChildren().size() >= 2 ? thisNode.getCutType() : Parameters::SIMPLE;
            switch (cutType) {
                case Parameters::SIMPLE: break; // already done
                case Parameters::ORDINAL:
                    // Ordinal cuts: ABCD -> AB, ABC, ABCD, BC, BCD, CD
                    iMaxEndWindow = std::min(endWindow.getEnd(), startWindow.getEnd() + window->maximum());
                    for (iWindowEnd=endWindow.getStart(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
                        window->windowstart(startWindow, iWindowEnd, iMinWindowStart, iWindowStart);
                        for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
                            for (size_t i=0; i < thisNode.getChildren().size() - 1; ++i) {
                                const NodeStructure& firstChildNode(*(thisNode.getChildren()[i]));
                                const SimulationNode& firstSimChildNode(_treeSimNodes[thisNode.getChildren()[i]->getID()]);
                                //buffer = firstChildNode.getIdentifier().c_str();
                                NodeStructure::count_t branchWindow = firstSimChildNode.getBrC_C()[iWindowStart] - firstSimChildNode.getBrC_C()[iWindowEnd + 1];
                                NodeStructure::expected_t branchExpected = firstChildNode.getBrN_C()[iWindowStart] - firstChildNode.getBrN_C()[iWindowEnd + 1];
                                for (size_t j=i+1; j < thisNode.getChildren().size(); ++j) {
                                    const NodeStructure& childNode(*(thisNode.getChildren()[j]));
                                    const SimulationNode& childSimNode(_treeSimNodes[thisNode.getChildren()[j]->getID()]);
                                    //buffer += ",";
                                    //buffer += childNode.getIdentifier();
                                    NodeStructure::count_t childBranchC = childSimNode.getBrC_C()[iWindowStart] - childSimNode.getBrC_C()[iWindowEnd + 1];
                                    branchWindow += childBranchC;
                                    branchExpected += childNode.getBrN_C()[iWindowStart] - childNode.getBrN_C()[iWindowEnd + 1];
                                    //printf("Evaluating cut [%s]\n", buffer.c_str());
                                    _measure_list->add(branchWindow, branchExpected);
                                    //++hits; printf("hits %d\n", hits);
                                }
                            }
                        }
                    }
                    break;
                case Parameters::PAIRS:
                    // Pair cuts: ABCD -> AB, AC, AD, BC, BD, CD
                    iMaxEndWindow = std::min(endWindow.getEnd(), startWindow.getEnd() + window->maximum());
                    for (iWindowEnd=endWindow.getStart(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
                        window->windowstart(startWindow, iWindowEnd, iMinWindowStart, iWindowStart);
                        for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
                            for (size_t i=0; i < thisNode.getChildren().size() - 1; ++i) {
                                const NodeStructure& startChildNode(*(thisNode.getChildren()[i]));
                                const SimulationNode& startSimChildNode(_treeSimNodes[thisNode.getChildren()[i]->getID()]);
                                NodeStructure::count_t startBranchWindow = startSimChildNode.getBrC_C()[iWindowStart] - startSimChildNode.getBrC_C()[iWindowEnd + 1];
                                NodeStructure::expected_t startBranchExpected = startChildNode.getBrN_C()[iWindowStart] - startChildNode.getBrN_C()[iWindowEnd + 1];
                                for (size_t j=i+1; j < thisNode.getChildren().size(); ++j) {
                                    const NodeStructure& stopChildNode(*(thisNode.getChildren()[j]));
                                    const SimulationNode& stopSimChildNode(_treeSimNodes[thisNode.getChildren()[j]->getID()]);
                                    //printf("Evaluating cut [%s,%s]\n", startChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                    NodeStructure::count_t stopBranchWindow = stopSimChildNode.getBrC_C()[iWindowStart] - stopSimChildNode.getBrC_C()[iWindowEnd + 1];
                                    NodeStructure::expected_t stopBranchExpected = stopChildNode.getBrN_C()[iWindowStart] - stopChildNode.getBrN_C()[iWindowEnd + 1];
                                    _measure_list->add(startBranchWindow + stopBranchWindow, startBranchExpected + stopBranchExpected);
                                    //++hits; printf("hits %d\n", hits);
                                }
                            }
                        }
                    }
                    break;
                case Parameters::TRIPLETS:
                    // Triple cuts: ABCD -> AB, AC, ABC, AD, ABD, ACD, BC, BD, BCD, CD
                    iMaxEndWindow = std::min(endWindow.getEnd(), startWindow.getEnd() + window->maximum());
                    for (iWindowEnd=endWindow.getStart(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
                        window->windowstart(startWindow, iWindowEnd, iMinWindowStart, iWindowStart);
                        for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
                            for (size_t i=0; i < thisNode.getChildren().size() - 1; ++i) {
                                const NodeStructure& startChildNode(*(thisNode.getChildren()[i]));
                                const SimulationNode& startSimChildNode(_treeSimNodes[thisNode.getChildren()[i]->getID()]);
                                NodeStructure::count_t startBranchWindow = startSimChildNode.getBrC_C()[iWindowStart] - startSimChildNode.getBrC_C()[iWindowEnd + 1];
                                NodeStructure::expected_t startBranchExpected = startChildNode.getBrN_C()[iWindowStart] - startChildNode.getBrN_C()[iWindowEnd + 1];
                                for (size_t j=i+1; j < thisNode.getChildren().size(); ++j) {
                                    const NodeStructure& stopChildNode(*(thisNode.getChildren()[j]));
                                    const SimulationNode& stopSimChildNode(_treeSimNodes[thisNode.getChildren()[j]->getID()]);
                                    //printf("Evaluating cut [%s,%s]\n", startChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                    NodeStructure::count_t stopBranchWindow = stopSimChildNode.getBrC_C()[iWindowStart] - stopSimChildNode.getBrC_C()[iWindowEnd + 1];
                                    NodeStructure::expected_t stopBranchExpected = stopChildNode.getBrN_C()[iWindowStart] - stopChildNode.getBrN_C()[iWindowEnd + 1];
                                    //printf("Evaluating cut [%s,%s]\n", startChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                    _measure_list->add(startBranchWindow + stopBranchWindow, startBranchExpected + stopBranchExpected);
                                    //++hits;printf("hits %d\n", hits);
                                    for (size_t k=i+1; k < j; ++k) {
                                        const NodeStructure& middleChildNode(*(thisNode.getChildren()[k]));
                                        const SimulationNode& middleSimChildNode(_treeSimNodes[thisNode.getChildren()[k]->getID()]);
                                        //printf("Evaluating cut [%s,%s,%s]\n", startChildNode.getIdentifier().c_str(), middleChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                        NodeStructure::count_t middleBranchWindow = middleSimChildNode.getBrC_C()[iWindowStart] - middleSimChildNode.getBrC_C()[iWindowEnd + 1];
                                        NodeStructure::expected_t middleBranchExpected = middleChildNode.getBrN_C()[iWindowStart] - middleChildNode.getBrN_C()[iWindowEnd + 1];
                                        //printf("Evaluating cut [%s,%s,%s]\n", startChildNode.getIdentifier().c_str(), middleChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                        _measure_list->add(startBranchWindow + middleBranchWindow + stopBranchWindow, startBranchExpected + middleBranchExpected + stopBranchExpected);
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

    return std::make_pair(_measure_list->loglikelihood(),TotalSimC);
}

/////////////////////////// SequentialMCSimSuccessiveFunctor //////////////////////////////////////

/* constructor */
SequentialMCSimSuccessiveFunctor::SequentialMCSimSuccessiveFunctor(boost::mutex& mutex, const ScanRunner& scanner, boost::shared_ptr<SequentialScanLoglikelihoodRatioWriter> writer) 
    : _mutex(mutex), _scanRunner(scanner), _sequential_writer(writer) {
    // TODO: Eventually this will need refactoring once we implement multiple data time ranges.
    const Parameters& parameters = _scanRunner.getParameters();
    _treeSimNode.reset(new SimulationNode(parameters.getDataTimeRangeSet().getTotalDaysAcrossRangeSets() + 1, 1));

    // TODO: Eventually this will need refactoring once we implement multiple data time ranges.
    DataTimeRange min_max = parameters.getDataTimeRangeSet().getMinMax();
    DataTimeRange::index_t zero_translation_additive = (min_max.getStart() <= 0) ? std::abs(min_max.getStart()) : min_max.getStart() * -1;
    const DataTimeRange& actual_range = parameters.getDataTimeRangeSet().getDataTimeRangeSets().front(); // TODO: for now, only take the first
    _range = DataTimeRange(actual_range.getStart() + zero_translation_additive, actual_range.getEnd() + zero_translation_additive);

    // Define the start and end windows with the zero index offset already incorporated.
    DataTimeRange::index_t idxAdditive = _scanRunner.getZeroTranslationAdditive();
    _startWindow = DataTimeRange(_scanRunner.temporalStartRange().getStart() + idxAdditive, _scanRunner.temporalStartRange().getEnd() + idxAdditive),
    _endWindow = DataTimeRange(_scanRunner.temporalEndRange().getStart() + idxAdditive, _scanRunner.temporalEndRange().getEnd() + idxAdditive);

    // Define the minimum and maximum window lengths.
    _window = _scanRunner.getNewWindowLength();
    _loglikelihood.reset(AbstractLoglikelihood::getNewLoglikelihood(parameters, _scanRunner.getTotalC(), _scanRunner.getTotalN(), _scanRunner.isCensoredData()));
}

SequentialMCSimSuccessiveFunctor::result_type SequentialMCSimSuccessiveFunctor::operator() (SequentialMCSimSuccessiveFunctor::param_type const & param) {
    result_type temp_result;

    try {
        int iWindowStart, iMinWindowStart, iWindowEnd, iMaxEndWindow;
        unsigned int min_cases_to_signal = _scanRunner.getParameters().getSequentialMinimumSignal();
        unsigned int total_sequential_cases = _scanRunner.getParameters().getSequentialMaximumSignal();
        unsigned int total_cases = 0;
        double simLogLikelihood = -std::numeric_limits<double>::max();

        // initialize randomizer seed for this simulation
        _random_number_generator.SetSeedOffset(param);

        // clear data from last simulation
        _treeSimNode->clear();

        // First assign cases up to the minimum signal.
        for (unsigned int cases=0; cases < min_cases_to_signal - 1; ++cases) {
            DataTimeRange::index_t idx = static_cast<DataTimeRange::index_t>(Equilikely(static_cast<long>(_range.getStart()), static_cast<long>(_range.getEnd()), _random_number_generator));
            ++(_treeSimNode->refBrC_C()[idx]);
            ++total_cases;
        }
        _treeSimNode->setCumulative();
        // Once at the minimum number of cases, start adding cases one at a time -- calculating log likelihood ratio and keeping maximum.
        for (unsigned int cases=min_cases_to_signal; cases <= total_sequential_cases; ++cases) {
            DataTimeRange::index_t idx = static_cast<DataTimeRange::index_t>(Equilikely(static_cast<long>(_range.getStart()), static_cast<long>(_range.getEnd()), _random_number_generator));
            // add new case to cumulative collection
            for (size_t w=idx; ; --w) {
                ++(_treeSimNode->refBrC_C()[w]);
                if (w == 0) break;
            }
            ++total_cases;
            // Now calculate the maximum log likelihood for current number of cases.
            NodeStructure::expected_t branchSum = static_cast<NodeStructure::expected_t>(_treeSimNode->getBrC());
            iMaxEndWindow = std::min(_endWindow.getEnd(), _startWindow.getEnd() + _window->maximum());
            for (iWindowEnd=_endWindow.getStart(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
                _window->windowstart(_startWindow, iWindowEnd, iMinWindowStart, iWindowStart);
                for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
                    NodeStructure::count_t branchWindow = _treeSimNode->getBrC_C()[iWindowStart] - _treeSimNode->getBrC_C()[iWindowEnd + 1];
                    simLogLikelihood = std::max(simLogLikelihood, _loglikelihood->LogLikelihood(branchWindow, branchSum, iWindowEnd - iWindowStart + 1));
                }
            }
        }
        temp_result.dSuccessfulResult = std::make_pair(simLogLikelihood, total_cases);
        temp_result.bUnExceptional = true;

        // write maximum log likelihood to sequential simulations data file
        _sequential_writer->write(simLogLikelihood, _mutex);
    } catch (memory_exception & e) {
        temp_result.eException_type = MCSimJobSource::result_type::memory;
        temp_result.Exception = prg_exception(e.what(), "SequentialMCSimSuccessiveFunctor");
        temp_result.bUnExceptional = false;
    } catch (resolvable_error & e) {
        temp_result.eException_type = MCSimJobSource::result_type::resolvable;
        temp_result.Exception = prg_exception(e.what(), "SequentialMCSimSuccessiveFunctor");
        temp_result.bUnExceptional = false;
    } catch (prg_exception & e) {
        temp_result.eException_type = MCSimJobSource::result_type::prg;
        temp_result.Exception = e;
        temp_result.bUnExceptional = false;
    } catch (std::exception & e) {
        temp_result.eException_type = MCSimJobSource::result_type::std;
        temp_result.Exception = prg_exception(e.what(), "SequentialMCSimSuccessiveFunctor");
        temp_result.bUnExceptional = false;
    } catch (...) {
        temp_result.eException_type = MCSimJobSource::result_type::unknown;
        temp_result.Exception = prg_exception("(...) -- unknown error", "SequentialMCSimSuccessiveFunctor");
        temp_result.bUnExceptional = false;
    }
    if (!temp_result.bUnExceptional) {
        temp_result.Exception.addTrace("operator()", "SequentialMCSimSuccessiveFunctor");
    }
    return temp_result;
}

/////////////////////////// SequentialReadMCSimSuccessiveFunctor //////////////////////////////////////

/* constructor */
SequentialReadMCSimSuccessiveFunctor::SequentialReadMCSimSuccessiveFunctor(boost::mutex& mutex, const ScanRunner& scanner, boost::shared_ptr<SequentialFileDataSource> source) 
    : _mutex(mutex), _scanRunner(scanner), _source(source) {}

SequentialReadMCSimSuccessiveFunctor::result_type SequentialReadMCSimSuccessiveFunctor::operator() (SequentialReadMCSimSuccessiveFunctor::param_type const & param) {
    result_type temp_result;

    try {
        /* read next log likelihood from the data source */
        boost::optional<double> simLogLikelihood = _source->nextLLR();
        if (simLogLikelihood) {
            temp_result.dSuccessfulResult = std::make_pair(simLogLikelihood.get(), _scanRunner.getTotalC());
            temp_result.bUnExceptional = true;
        } else
            throw prg_error("Simulation %u could not read LLR value from file.", "operator()", param);

    } catch (memory_exception & e) {
        temp_result.eException_type = MCSimJobSource::result_type::memory;
        temp_result.Exception = prg_exception(e.what(), "SequentialReadMCSimSuccessiveFunctor");
        temp_result.bUnExceptional = false;
    } catch (resolvable_error & e) {
        temp_result.eException_type = MCSimJobSource::result_type::resolvable;
        temp_result.Exception = prg_exception(e.what(), "SequentialReadMCSimSuccessiveFunctor");
        temp_result.bUnExceptional = false;
    } catch (prg_exception & e) {
        temp_result.eException_type = MCSimJobSource::result_type::prg;
        temp_result.Exception = e;
        temp_result.bUnExceptional = false;
    } catch (std::exception & e) {
        temp_result.eException_type = MCSimJobSource::result_type::std;
        temp_result.Exception = prg_exception(e.what(), "SequentialReadMCSimSuccessiveFunctor");
        temp_result.bUnExceptional = false;
    } catch (...) {
        temp_result.eException_type = MCSimJobSource::result_type::unknown;
        temp_result.Exception = prg_exception("(...) -- unknown error", "SequentialReadMCSimSuccessiveFunctor");
        temp_result.bUnExceptional = false;
    }
    if (!temp_result.bUnExceptional) {
        temp_result.Exception.addTrace("operator()", "SequentialReadMCSimSuccessiveFunctor");
    }
    return temp_result;
}