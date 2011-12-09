//******************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//******************************************************************************
#include "MonteCarloSimFunctor.h"
#include "PrjException.h"

/* constructor */
MCSimSuccessiveFunctor::MCSimSuccessiveFunctor(boost::mutex& Mutex, 
                                               boost::shared_ptr<AbstractRandomizer> randomizer,
                                               const ScanRunner& scanRunner) : gMutex(Mutex), _randomizer(randomizer), _scanRunner(scanRunner) {
    _simData.resize(_scanRunner.getNodes().size(), std::make_pair(0,0));
    _loglikelihood.reset(AbstractLoglikelihood::getNewLoglikelihood(_scanRunner.getParameters(), _scanRunner.getTotalC(), _scanRunner.getTotalN()));
}

MCSimSuccessiveFunctor::result_type MCSimSuccessiveFunctor::operator() (MCSimSuccessiveFunctor::param_type const & param) {
    result_type temp_result;
    try {
        //randomize data
        int TotalSimC = _randomizer.get()->RandomizeData(param, _scanRunner.getNodes(), _simData);
        
        //print simulation data to file, if requested
        //if (gDataHub.GetParameters().GetOutputSimulationData()) {
        //    boost::mutex::scoped_lock     lock(gMutex);
        //    for (size_t t=0; t < gpSimulationDataContainer->size(); ++t)
        //        gDataWriter->write((*(*gpSimulationDataContainer)[t]), gDataHub.GetParameters());
        //}

        //--------------------- SCANNING THE TREE, SIMULATIONS -------------------------
        const Parameters& parameter = _scanRunner.getParameters();
        const ScanRunner::NodeStructureContainer_t& nodes = _scanRunner.getNodes();
        double simLogLikelihood = -std::numeric_limits<double>::max();
        for (size_t i=0; i < nodes.size(); ++i) {            
            if (_simData.at(i).second > 1) { //if (_Nodes.at(i)->_SimBrC > 1)
                const NodeStructure& thisNode(*(nodes.at(i)));
                Parameters::CutType cutType = thisNode.getChildren().size() >= 3 ? parameter.getCutType() : Parameters::SIMPLE;
                switch (cutType) {
                    case Parameters::SIMPLE:
                        simLogLikelihood = std::max(simLogLikelihood, _loglikelihood->LogLikelihood(_simData.at(i).second/*_Nodes.at(i)->_SimBrC*/, nodes.at(i)->getBrN())); 
                        break;
                    case Parameters::ORDINAL:
                        for (size_t i=0; i < thisNode.getChildren().size() - 1; ++i) {
                            const NodeStructure& firstChildNode(*(nodes.at(thisNode.getChildren().at(i))));
                            //buffer = firstChildNode.getIdentifier().c_str();
                            int sumBranchC = _simData.at(i).second;
                            double sumBranchN = firstChildNode.getBrN();
                            for (size_t j=i+1; j < thisNode.getChildren().size(); ++j) {
                                const NodeStructure& childNode(*(nodes.at(thisNode.getChildren().at(j))));
                                //buffer += ",";
                                //buffer += childNode.getIdentifier();
                                sumBranchC += _simData.at(static_cast<size_t>(childNode.getID())).second;
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
                                                            _loglikelihood->LogLikelihood(_simData.at(static_cast<size_t>(startChildNode.getID())).second + 
                                                                                          _simData.at(static_cast<size_t>(stopChildNode.getID())).second, 
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
                                                            _loglikelihood->LogLikelihood(_simData.at(static_cast<size_t>(startChildNode.getID())).second + 
                                                                                          _simData.at(static_cast<size_t>(stopChildNode.getID())).second, 
                                                                                          startChildNode.getBrN() + stopChildNode.getBrN())); 
                                //++hits;printf("hits %d\n", hits);
                                for (size_t k=i+1; k < j; ++k) {
                                    const NodeStructure& middleChildNode(*(nodes.at(thisNode.getChildren().at(k))));
                                    //printf("Evaluating cut [%s,%s,%s]\n", startChildNode.getIdentifier().c_str(), middleChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                    simLogLikelihood = std::max(simLogLikelihood, 
                                                                _loglikelihood->LogLikelihood(_simData.at(static_cast<size_t>(startChildNode.getID())).second + 
                                                                                              _simData.at(static_cast<size_t>(middleChildNode.getID())).second + 
                                                                                              _simData.at(static_cast<size_t>(stopChildNode.getID())).second, 
                                                                                              startChildNode.getBrN() + middleChildNode.getBrN() + stopChildNode.getBrN())); 
                                    //++hits; printf("hits %d\n", hits);
                                }
                            }
                        } break;
                    case Parameters::COMBINATORIAL:
                    default: throw prg_error("Unknown cut type (%d).", "scanTree()", cutType);
                };
            }
        } // for i<nNodes

        temp_result.dSuccessfulResult = std::make_pair(simLogLikelihood,TotalSimC);
        temp_result.bUnExceptional = true;
    } catch (memory_exception & e) {
        temp_result.eException_type = MCSimJobSource::result_type::memory;
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
