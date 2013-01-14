
#include<boost/tokenizer.hpp>
#include <boost/regex.hpp>
#include <boost/assign.hpp>

#include "ScanRunner.h"
#include "UtilityFunctions.h"
#include "FileName.h"
#include "MonteCarloSimFunctor.h"
#include "MCSimJobSource.h"
#include "contractor.h"
#include "Randomization.h"
#include "ParametersPrint.h"
#include "DataFileWriter.h"
#include "DataSource.h"
#include "ResultsFileWriter.h"

double CutStructure::getExpected(const ScanRunner& scanner) {
  if (scanner.getParameters().getModelType() == Parameters::BERNOULLI) {
    if (scanner.getParameters().isConditional())
		return _N * (scanner.getTotalC() / scanner.getTotalN());
	return _N * scanner.getParameters().getProbability();
  }
  return _N;
}

/*
 Adds cases and measure through the tree from each node through the tree to
 all its parents, and so on, to all its ancestors as well.
 If a node is a decendant to an ancestor in more than one way, the cases and
 measure is only added once to that ancestor.
 */
void ScanRunner::addCN(int id, int c, double n) {
    _Ancestor.at(id) = 1;
    _Nodes.at(id)->refBrC() += c;
    _Nodes.at(id)->refBrN() += n;
    for(size_t j=0; j < _Nodes.at(id)->getParents().size(); j++) {
      int parent = _Nodes.at(id)->getParents().at(j);
        if(_Ancestor.at(parent) == 0)
          addCN(parent,c,n);
        else
          _Ancestor.at(parent) = _Ancestor.at(parent) + 1;
    }
}

/*
 Returns pair<bool,size_t> - first value indicates node existance, second is index into class vector _Nodes.
 */
ScanRunner::Index_t ScanRunner::getNodeIndex(const std::string& identifier) const {
    std::auto_ptr<NodeStructure> node(new NodeStructure(identifier, _parameters.getCutType()));
    NodeStructureContainer_t::const_iterator itr = std::lower_bound(_Nodes.begin(), _Nodes.end(), node.get(), CompareNodeStructureByIdentifier());
    if (itr != _Nodes.end() && (*itr)->getIdentifier() == node.get()->getIdentifier()) {
      size_t tt = std::distance(_Nodes.begin(), itr);
      assert(tt == (*itr)->getID());
      return std::make_pair(true, std::distance(_Nodes.begin(), itr));
    } else
        return std::make_pair(false, 0);
}

/*
 Reads count and population data from passed file. The file format is: <node identifier>, <count>, <population>
 */
bool ScanRunner::readCounts(const std::string& filename) {
    _print.Printf("Reading count file ...\n", BasePrint::P_STDOUT);
    bool readSuccess=true;
    std::auto_ptr<DataSource> dataSource(DataSource::getNewDataSourceObject(filename));

    // first collect all nodes -- this will allow the referencing of parent nodes not yet encountered
    int count=0, controls=0, duplicates=0;
    double population=0;
    while (dataSource->readRecord()) {
        if (dataSource->getNumValues() != (_parameters.isDuplicates() ? 4 : 3)) {
            readSuccess = false;
            _print.Printf("Error: Record %ld in count file %s. Expecting <indentifier>, <count>,%s <population> but found %ld value%s.\n", 
                          BasePrint::P_READERROR, dataSource->getCurrentRecordIndex(), (static_cast<int>(dataSource->getNumValues()) > (_parameters.isDuplicates() ? 4 : 3)) ? "has extra data" : "is missing data",
                          (_parameters.isDuplicates() ? " <duplicates>," : ""), dataSource->getNumValues(), (dataSource->getNumValues() == 1 ? "" : "s"));
            continue;
        }
        ScanRunner::Index_t index = getNodeIndex(dataSource->getValueAt(0));
        if (!index.first) {
            readSuccess = false;
            _print.Printf("Error: Record %ld in count file references unknown node (%s).\n", BasePrint::P_READERROR, dataSource->getCurrentRecordIndex(), dataSource->getValueAt(0).c_str());
            continue;
        }
        NodeStructure * node = _Nodes.at(index.second);
        if  (!string_to_type<int>(dataSource->getValueAt(1).c_str(), count) || count < 0) {
            readSuccess = false;
            _print.Printf("Error: Record %ld in count file references negative number of cases in node '%s'.\n", BasePrint::P_READERROR, dataSource->getCurrentRecordIndex(), node->getIdentifier().c_str());
            continue;
        }
        node->refIntC() += count;
        if (_parameters.isDuplicates()) {
            if  (!string_to_type<int>(dataSource->getValueAt(2).c_str(), duplicates) || duplicates < 0) {
                readSuccess = false;
                _print.Printf("Error: Record %ld in count file references negative number of duplicates in node '%s'.\n", BasePrint::P_READERROR, dataSource->getCurrentRecordIndex(), node->getIdentifier().c_str());
                continue;
            } else if (duplicates > count) {
                readSuccess = false;
                _print.Printf("Error: Record %ld in count file references more duplicates than count in node '%s'.\n", BasePrint::P_READERROR, dataSource->getCurrentRecordIndex(), node->getIdentifier().c_str());
                continue;
            }
            node->refDuplicates() += duplicates;
        }
        if (_parameters.getModelType() == Parameters::POISSON) {
            if  (!string_to_type<double>(dataSource->getValueAt(_parameters.isDuplicates() ? 3 : 2).c_str(), population) || population < 0) {
                readSuccess = false;
                _print.Printf("Error: Record %ld in count file references negative population in node '%s'.\n", BasePrint::P_READERROR, dataSource->getCurrentRecordIndex(), node->getIdentifier().c_str());
                continue;
            }
            node->refIntN() += population;
        } else if (_parameters.getModelType() == Parameters::BERNOULLI) { 
            if  (!string_to_type<int>(dataSource->getValueAt(_parameters.isDuplicates() ? 3 : 2).c_str(), controls) || controls < 0) {
                readSuccess = false;
                _print.Printf("Error: Record %ld in count file references negative number of controls in node '%s'.\n", BasePrint::P_READERROR, dataSource->getCurrentRecordIndex(), node->getIdentifier().c_str());
                continue;
            }
            node->refIntN() += count + controls;
            _TotalControls += controls;
        } else throw prg_error("Unknown model type (%d).", "readCounts()", _parameters.getModelType());
    }
    return readSuccess;
}

/*
 Reads tree structure from passed file. The file format is: <node identifier>, <parent node identifier 1>, <parent node identifier 2>, ... <parent node identifier N>
 */
bool ScanRunner::readTree(const std::string& filename) {
    _print.Printf("Reading tree file ...\n", BasePrint::P_STDOUT);
    bool readSuccess=true;
    std::auto_ptr<DataSource> dataSource(DataSource::getNewDataSourceObject(filename));

    // first collect all nodes -- this will allow the referencing of parent nodes not yet encountered
    while (dataSource->readRecord()) {
        std::string identifier = dataSource->getValueAt(0);
        std::auto_ptr<NodeStructure> node(new NodeStructure(trimString(identifier), _parameters.getCutType()));
        NodeStructureContainer_t::iterator itr = std::lower_bound(_Nodes.begin(), _Nodes.end(), node.get(), CompareNodeStructureByIdentifier());
        if (itr == _Nodes.end() || (*itr)->getIdentifier() != node.get()->getIdentifier())
            _Nodes.insert(itr, node.release());
    }

    //reset node identifiers to ordinal position in vector -- this is to keep the original algorithm intact since it uses vector indexes heavily
    for (size_t i=0; i < _Nodes.size(); ++i) _Nodes.at(i)->setID(static_cast<int>(i));

    // now set parent nodes
    dataSource->gotoFirstRecord();
    while (dataSource->readRecord()) {
        NodeStructure * node = 0;
        // assign parent nodes to node
        for (size_t t=0; t < dataSource->getNumValues(); ++t) {
            std::string identifier = dataSource->getValueAt(static_cast<long>(t));
            ScanRunner::Index_t index = getNodeIndex(identifier);
            if (!index.first) {
                readSuccess = false;
                _print.Printf("Error: Record %ld in tree file has unknown parent node (%s).\n", BasePrint::P_READERROR, dataSource->getCurrentRecordIndex(), identifier.c_str());
            } else {
                if (node) {
                    // prevent nodes from having more than one parent, see https://www.squishlist.com/ims/treescan/13/
                    if (node->refParents().size()) {
                        readSuccess = false;
                        _print.Printf("Error: Record %ld in tree file has node with more than one parent node defined (%s).\n", BasePrint::P_READERROR, dataSource->getCurrentRecordIndex(), identifier.c_str());
                    } else node->refParents().push_back(_Nodes.at(index.second)->getID());
                } else node = _Nodes.at(index.second);
            }
        }
    }

    return readSuccess;
}

/*
 Reads tree node cuts from passed file. The file format is: <node identifier>, <parent node identifier 1>, <parent node identifier 2>, ... <parent node identifier N>
 */
bool ScanRunner::readCuts(const std::string& filename) {
    _print.Printf("Reading cuts file ...\n", BasePrint::P_STDOUT);
    bool readSuccess=true;
    std::auto_ptr<DataSource> dataSource(DataSource::getNewDataSourceObject(filename));
    Parameters::cut_map_t cut_type_map = Parameters::getCutTypeMap();

    while (dataSource->readRecord()) {
        if (dataSource->getNumValues() != 2) {
            readSuccess = false;
            _print.Printf("Error: Record %ld in cuts file %s. Expecting <indentifier>, <cut type [simple,pairs,triplets,ordinal]> but found %ld value%s.\n", 
                          BasePrint::P_READERROR, dataSource->getCurrentRecordIndex(), (static_cast<int>(dataSource->getNumValues()) > 2) ? "has extra data" : "is missing data",
                          dataSource->getNumValues(), (dataSource->getNumValues() == 1 ? "" : "s"));
            continue;
        }
        std::string identifier = dataSource->getValueAt(static_cast<long>(0));
        ScanRunner::Index_t index = getNodeIndex(identifier);
        if (!index.first) {
            readSuccess = false;
            _print.Printf("Error: Record %ld in cut file has unknown node (%s).\n", BasePrint::P_READERROR, dataSource->getCurrentRecordIndex(), identifier.c_str());
        } else {
            NodeStructure * node = _Nodes.at(index.second);
            std::string cut_type_string = lowerString(trimString(dataSource->getValueAt(static_cast<long>(1))));
            Parameters::cut_map_t::iterator itr = cut_type_map.find(cut_type_string);
            if (itr == cut_type_map.end()) {
                readSuccess = false;
                _print.Printf("Error: Record %ld in cut file has unknown cut type (%s). Choices are simple, pairs, triplets and ordinal.\n", BasePrint::P_READERROR, dataSource->getCurrentRecordIndex(), dataSource->getValueAt(static_cast<long>(1)).c_str());
            } else {
                node->setCutType(itr->second);
            }
        }
    }

    return readSuccess;
}

/* REPORT RESULTS */
bool ScanRunner::reportResults(const std::string& filename, Parameters::ResultsFormat rptfmt, time_t start, time_t end) const {
    ResultsFileWriter resultsWriter(*this);
    bool status = rptfmt == Parameters::HTML ? resultsWriter.writeHTML(filename, start, end) : resultsWriter.writeASCII(filename, start, end);
    // write cuts to supplemental reports file
    unsigned int k=0;
    CutsRecordWriter cutsWriter(*this);
    while(status && k < getCuts().size() && getCuts().at(k)->getC() > 0 && getCuts().at(k)->getRank() < _parameters.getNumReplicationsRequested() + 1) {
        cutsWriter.write(k);
        k++;
    }
    return status;
}

/*
 Run Scan.
 */
bool ScanRunner::run() {
    time_t gStartTime, gEndTime;
    time(&gStartTime); //get start time

    if (!readTree(_parameters.getTreeFileName())) return false;
    if (_parameters.getCutsFileName().length() && !readCuts(_parameters.getCutsFileName())) return false;
    if (!readCounts(_parameters.getCountFileName())) return false;
    if (!setupTree()) return false;
    if (!scanTree()) return false;
    if (!runsimulations()) return false;

    time(&gEndTime); //get end time
    if (!reportResults(_parameters.getOutputFileName(), _parameters.getResultsFormat(), gStartTime, gEndTime)) return false;

    return true;
}

/* DOING THE MONTE CARLO SIMULATIONS */
bool ScanRunner::runsimulations() {
    char                * sReplicationFormatString = "The result of Monte Carlo replica #%u of %u replications is: %lf\n";
    unsigned long         ulParallelProcessCount = std::min(_parameters.getNumParallelProcessesToExecute(), _parameters.getNumReplicationsRequested());

    try {
        if (_parameters.getNumReplicationsRequested() == 0)
            return true;

        _print.Printf("Doing the %d Monte Carlo simulations:\n", BasePrint::P_STDOUT, _parameters.getNumReplicationsRequested());

        {
            PrintQueue lclPrintDirection(_print, false);
            MCSimJobSource jobSource(::GetCurrentTime_HighResolution(), lclPrintDirection, sReplicationFormatString, *this);
            typedef contractor<MCSimJobSource> contractor_type;
            contractor_type theContractor(jobSource);

            //run threads:
            boost::thread_group tg;
            boost::mutex        thread_mutex;
            for (unsigned u=0; u < ulParallelProcessCount; ++u) {
                try {
                    MCSimSuccessiveFunctor mcsf(thread_mutex, boost::shared_ptr<AbstractRandomizer>(AbstractRandomizer::getNewRandomizer(_parameters, _TotalC, _TotalControls, _TotalN)), *this);
                    tg.create_thread(subcontractor<contractor_type,MCSimSuccessiveFunctor>(theContractor,mcsf));
                } catch (std::bad_alloc &) {
                    if (u == 0) throw; // if this is the first thread, re-throw exception
                    _print.Printf("Notice: Insufficient memory to create %u%s parallel simulation ... continuing analysis with %u parallel simulations.\n", BasePrint::P_NOTICE, u + 1, (u == 1 ? "nd" : (u == 2 ? "rd" : "th")), u);
                    break;
                } catch (prg_exception& x) {
                    if (u == 0) throw; // if this is the first thread, re-throw exception
                    _print.Printf("Error: Program exception occurred creating %u%s parallel simulation ... continuing analysis with %u parallel simulations.\nException:%s\n", BasePrint::P_ERROR, u + 1, (u == 1 ? "nd" : (u == 2 ? "rd" : "th")), u, x.what());
                    break;
                } catch (...) {
                    if (u == 0) throw prg_error("Unknown program error occurred.\n","PerformSuccessiveSimulations_Parallel()"); // if this is the first thread, throw exception
                    _print.Printf("Error: Unknown program exception occurred creating %u%s parallel simulation ... continuing analysis with %u parallel simulations.\n", BasePrint::P_ERROR, u + 1, (u == 1 ? "nd" : (u == 2 ? "rd" : "th")), u);
                    break;
                }
            }
            tg.join_all();

            //propagate exceptions if needed:
            theContractor.throw_unhandled_exception();
            jobSource.Assert_NoExceptionsCaught();
            if (jobSource.GetUnregisteredJobCount() > 0)
                throw prg_error("At least %d jobs remain uncompleted.", "ScanRunner", jobSource.GetUnregisteredJobCount());
        }
    } catch (prg_exception& x) {
        x.addTrace("runsimulations()","ScanRunner");
        throw;
    }
    return true;
}

/* SCANNING THE TREE */
bool ScanRunner::scanTree() {
    _print.Printf("Scanning the tree.\n", BasePrint::P_STDOUT);

    double loglikelihood=0;
    ScanRunner::Loglikelihood_t calcLogLikelihood(AbstractLoglikelihood::getNewLoglikelihood(_parameters, _TotalC, _TotalN));

    // determine number of cuts by each nodes cut type
    size_t cuts = 0;
    for (NodeStructureContainer_t::const_iterator itr=_Nodes.begin(); itr != _Nodes.end(); ++itr) {
        if ((*itr)->getParents().size() == 0) // skip root nodes
            continue;
        Parameters::CutType cutType = (*itr)->getChildren().size() >= 2 ? (*itr)->getCutType() : Parameters::SIMPLE;
        size_t z = (*itr)->getChildren().size();
        switch (cutType) {
            case Parameters::SIMPLE : ++cuts; break;
            case Parameters::ORDINAL: cuts += z * (z - 1)/2 - 1; break;
            case Parameters::PAIRS: cuts += z * (z - 1)/2; break;
            case Parameters::TRIPLETS: cuts += z * (z - 1)/2 + getNumCombinations(z, 3); break;
            //case Parameters::COMBINATORIAL: cuts += std::pow(2.0,z) - z - 2.0; break;
            default: throw prg_error("Unknown cut type (%d).", "scanTree()", cutType);
        };
    }
    //for(unsigned int k=0; k < cuts; k++) _Cut.push_back(new CutStructure());

    //std::string buffer;
    //int hits=0;
    for (size_t n=0; n < _Nodes.size(); n++) {
        if (_Nodes.at(n)->getBrC() > 1) {
            const NodeStructure& thisNode(*(_Nodes.at(n)));
            Parameters::CutType cutType = thisNode.getChildren().size() >= 2 ? thisNode.getCutType() : Parameters::SIMPLE;
            switch (cutType) {
                case Parameters::SIMPLE:
                    //printf("Evaluating cut [%s]\n", thisNode.getIdentifier().c_str());
                    updateCuts(n, thisNode.getBrC(), thisNode.getBrN(), calcLogLikelihood);
                    //++hits; printf("hits %d\n", hits);
                    break;
                case Parameters::ORDINAL:
                    for (size_t i=0; i < thisNode.getChildren().size() - 1; ++i) {
                        const NodeStructure& firstChildNode(*(_Nodes.at(thisNode.getChildren().at(i))));
                        //buffer = firstChildNode.getIdentifier().c_str();
                        int sumBranchC=firstChildNode.getBrC();
                        double sumBranchN=firstChildNode.getBrN();
                        for (size_t j=i+1; j < thisNode.getChildren().size(); ++j) {
                            const NodeStructure& childNode(*(_Nodes.at(thisNode.getChildren().at(j))));
                            //buffer += ",";
                            //buffer += childNode.getIdentifier();
                            sumBranchC += childNode.getBrC();
                            sumBranchN += childNode.getBrN();
                            //printf("Evaluating cut [%s]\n", buffer.c_str());
                            updateCuts(n, sumBranchC, sumBranchN, calcLogLikelihood);
                            //++hits; printf("hits %d\n", hits);
                        }
                    } break;
                case Parameters::PAIRS:
                    for (size_t i=0; i < thisNode.getChildren().size() - 1; ++i) {
                        const NodeStructure& startChildNode(*(_Nodes.at(thisNode.getChildren().at(i))));
                        for (size_t j=i+1; j < thisNode.getChildren().size(); ++j) {
                            const NodeStructure& stopChildNode(*(_Nodes.at(thisNode.getChildren().at(j))));
                            //printf("Evaluating cut [%s,%s]\n", startChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                            updateCuts(n, startChildNode.getBrC() + stopChildNode.getBrC(), startChildNode.getBrN() + stopChildNode.getBrN(), calcLogLikelihood);
                            //++hits; printf("hits %d\n", hits);
                        }
                    } break;
                case Parameters::TRIPLETS:
                    for (size_t i=0; i < thisNode.getChildren().size() - 1; ++i) {
                        const NodeStructure& startChildNode(*(_Nodes.at(thisNode.getChildren().at(i))));
                        for (size_t j=i+1; j < thisNode.getChildren().size(); ++j) {
                            const NodeStructure& stopChildNode(*(_Nodes.at(thisNode.getChildren().at(j))));
                            //printf("Evaluating cut [%s,%s]\n", startChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                            updateCuts(n, startChildNode.getBrC() + stopChildNode.getBrC(), startChildNode.getBrN() + stopChildNode.getBrN(), calcLogLikelihood);
                            //++hits;printf("hits %d\n", hits);
                            for (size_t k=i+1; k < j; ++k) {
                                const NodeStructure& middleChildNode(*(_Nodes.at(thisNode.getChildren().at(k))));
                                //printf("Evaluating cut [%s,%s,%s]\n", startChildNode.getIdentifier().c_str(), middleChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                updateCuts(n, startChildNode.getBrC() + middleChildNode.getBrC() + stopChildNode.getBrC(), startChildNode.getBrN() + middleChildNode.getBrN() + stopChildNode.getBrN(), calcLogLikelihood);
                                //++hits; printf("hits %d\n", hits);
                            }
                        }
                    } break;
                case Parameters::COMBINATORIAL:
                default: throw prg_error("Unknown cut type (%d).", "scanTree()", cutType);
            }
        }
    }
    _print.Printf("The log likelihood ratio of the most likely cut is %lf.\n", BasePrint::P_STDOUT, calcLogLikelihood->LogLikelihoodRatio(_Cut.at(0)->getLogLikelihood()));
    return true;
}

void ScanRunner::updateCuts(int node_index, int BrC, double BrN, const Loglikelihood_t& logCalculator) {
    double loglikelihood = logCalculator->LogLikelihood(BrC, BrN); 
    if (loglikelihood == logCalculator->UNSET_LOGLIKELIHOOD) return;

    std::auto_ptr<CutStructure> cut(new CutStructure());
    cut->setLogLikelihood(loglikelihood);
    cut->setID(static_cast<int>(node_index));
    cut->setC(BrC);
    cut->setN(BrN);
    CutStructureContainer_t::iterator itr = std::lower_bound(_Cut.begin(), _Cut.end(), cut.get(), CompareCutsByLoglikelihood());
    _Cut.insert(itr, cut.release());
}

/*
 SETTING UP THE TREE
 */
bool ScanRunner::setupTree() {
    double   adjustN;
    int     parent;

    _print.Printf("Setting up the tree.\n", BasePrint::P_STDOUT);

    // Initialize variables
    _TotalC=0;_TotalN=0;
    for(NodeStructureContainer_t::iterator itr=_Nodes.begin(); itr != _Nodes.end(); ++itr) {
        (*itr)->setBrC(0);
        (*itr)->setBrN(0);
    }

    // Calculates the total number of cases and the total population at risk
    for(NodeStructureContainer_t::iterator itr=_Nodes.begin(); itr != _Nodes.end(); ++itr) {
        _TotalC += (*itr)->getIntC();
        _TotalN += (*itr)->getIntN();
    }

    // Calculates the expected counts for each node and the total.
    if (_parameters.getModelType() == Parameters::POISSON && _parameters.isConditional()) {
        adjustN = _TotalC/_TotalN;
        for (NodeStructureContainer_t::iterator itr=_Nodes.begin(); itr != _Nodes.end(); ++itr)
            (*itr)->refIntN() *= adjustN;
        _TotalN = _TotalC;
    }

    // For each node, calculates the observed and expected number of cases for that
    // node together with all of its children, grandchildren, etc.
    // Checks whether anforlust is true or false for each node.
    // Also checks whether a node is an ancestor to itslef, which is not allowed.
    _Ancestor.resize(_Nodes.size(), 0);
    for (size_t i=0; i < _Nodes.size(); ++i) {
        for (size_t j=0; j < _Nodes.size(); ++j) _Ancestor[j]=0;
        addCN(static_cast<int>(i), _Nodes.at(i)->getIntC(), _Nodes.at(i)->getIntN());
        if (_Ancestor[i] > 1) {
            _print.Printf("Error: Node '%s' has itself as an ancestor.\n", BasePrint::P_ERROR, _Nodes.at(i)->getIdentifier().c_str());
            return false;
        } // if Ancestor[i]>1
        for (size_t j=0; j < _Nodes.size(); ++j) if(_Ancestor[j] > 1) _Nodes.at(i)->setAnforlust(true);
    } // for i<nNodes

    // For each node calculates the number of children and sets up the list of child IDs
    for (size_t i=0; i < _Nodes.size(); ++i) {
        for (size_t j=0; j < _Nodes.at(i)->getParents().size(); ++j) {
            parent = _Nodes.at(i)->getParents().at(j);
            _Nodes.at(parent)->refChildren().push_back(static_cast<int>(i));
        } // for j
    } // for i < nNodes


    // Checks that no node has negative expected cases or that a node with zero expected has observed cases.
    for (size_t i=0; i < _Nodes.size(); ++i) {
        // cout << "Node=" << i << ", BrC=" << Node[i].BrC << ", BrN=" << Node[i].BrN << endl;
        if (_Nodes.at(i)->getBrN() < 0 ) {
            _print.Printf("Error: Node '%s' has negative expected cases.\n", BasePrint::P_ERROR, _Nodes.at(i)->getIdentifier().c_str());
            return false;
        }
        if (_Nodes.at(i)->getBrN() == 0 && _Nodes.at(i)->getBrC() > 0) {
            _print.Printf("Error: Node '%s' has observed cases but zero expected.\n", BasePrint::P_ERROR, _Nodes.at(i)->getIdentifier().c_str());
            return false;
        }
    } // for i

    return true;
}
