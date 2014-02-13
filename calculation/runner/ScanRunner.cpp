
#include <numeric>
#include <algorithm>
#include <boost/tokenizer.hpp>
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
        if (scanner.getParameters().getConditionalType() == Parameters::TOTALCASES)
            return _N * (scanner.getTotalC() / scanner.getTotalN());
        return _N * scanner.getParameters().getProbability();
    } else if (scanner.getParameters().getModelType() == Parameters::TEMPORALSCAN) {
        return _N * (getEndIdx() - getStartIdx() + 1) / scanner.getParameters().getDataTimeRangeSet().getTotalDaysAcrossRangeSets();
    } else {
        return _N;
    }
}

/** Returns cut's observed divided by expected. */
double CutStructure::getODE(const ScanRunner& scanner) {
    if (scanner.getParameters().getModelType() == Parameters::TEMPORALSCAN) {
        /* C/(N*W/D) where
                C = number of cases in the node as well as in the the temporal cluster found (below, 06.04 cases that are 7-11 days after vaccination) 
                N = number of cases in the node, through the whole time period (below, all 0604 cases) 
                W = number of days in the temporal cluster (below, 11-6=5) 
                D = number of days for which cases wee recorded (e.g. D=56 if a 1-56 time interval was used)
        */
        double C = static_cast<double>(_C);
        double N = _N;
        double W = static_cast<double>(_end_idx - _start_idx + 1.0);
        double D = static_cast<double>(scanner.getParameters().getDataTimeRangeSet().getTotalDaysAcrossRangeSets());
        double denominator = D ? N * W / D : 0.0;
        return denominator ? C/denominator : 0.0;
    } else {
        double expected = getExpected(scanner);
        return expected ? static_cast<double>(getC())/expected : 0.0;
    }
}

/** Returns cut's relative risk. */
double CutStructure::getRelativeRisk(const ScanRunner& scanner) {
    if (scanner.getParameters().getModelType() == Parameters::TEMPORALSCAN) {
        //RR = [ClusterCases / Cluster Window Length ] / [ (NodeCases-ClusterCases) / (StudyTime Length - ClusterWindowLength) ]
        double W = static_cast<double>(_end_idx - _start_idx + 1.0);
        double D = static_cast<double>(scanner.getParameters().getDataTimeRangeSet().getTotalDaysAcrossRangeSets());
        double diff_cases = _N - static_cast<double>(_C);
        double denominator = diff_cases ? ( diff_cases / (D - W) ) : 0.0;
        return denominator ? (static_cast<double>(_C) / W ) / denominator : 0.0;
    } 
    throw prg_error("Unknown model type (%d).", "getRelativeRisk()", scanner.getParameters().getModelType());
}

ScanRunner::ScanRunner(const Parameters& parameters, BasePrint& print) : _parameters(parameters), _print(print), _TotalC(0), _TotalControls(0), _TotalN(0) {
    // TODO: Eventually this will need refactoring once we implement multiple data time ranges.
    DataTimeRange min_max = _parameters.getModelType() == Parameters::TEMPORALSCAN ? _parameters.getDataTimeRangeSet().getMinMax() : DataTimeRange(0,0);
    // translate to ensure zero based additive
    _zero_translation_additive = (min_max.getStart() <= 0) ? std::abs(min_max.getStart()) : min_max.getStart() * -1;
}

/*
 Adds cases and measure through the tree from each node through the tree to all its parents, and so on, to all its ancestors as well.
 If a node is a decendant to an ancestor in more than one way, the cases and measure is only added once to that ancestor.
 */
void ScanRunner::addCN_C(int id, NodeStructure::CountContainer_t& c, NodeStructure::ExpectedContainer_t& n/*int c, double n*/) {
    _Ancestor.at(id) = 1;

    std::transform(c.begin(), c.end(), _Nodes.at(id)->refBrC_C().begin(), _Nodes.at(id)->refBrC_C().begin(), std::plus<NodeStructure::count_t>());
    std::transform(n.begin(), n.end(), _Nodes.at(id)->refBrN_C().begin(), _Nodes.at(id)->refBrN_C().begin(), std::plus<NodeStructure::expected_t>());
    for(size_t j=0; j < _Nodes.at(id)->getParents().size(); j++) {
        int parent = _Nodes.at(id)->getParents().at(j);
        if (_Ancestor.at(parent) == 0)
            addCN_C(parent, c, n);
        else
            _Ancestor.at(parent) = _Ancestor.at(parent) + 1;
    }
}

/*
 Returns pair<bool,size_t> - first value indicates node existance, second is index into class vector _Nodes.
 */
ScanRunner::Index_t ScanRunner::getNodeIndex(const std::string& identifier) const {
    std::auto_ptr<NodeStructure> node(new NodeStructure(identifier));
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
    _print.Printf("Reading case file ...\n", BasePrint::P_STDOUT);
    bool readSuccess=true;
    std::auto_ptr<DataSource> dataSource(DataSource::getNewDataSourceObject(filename));
    int expectedColumns = (_parameters.getModelType() == Parameters::TEMPORALSCAN ? 3 : 2) + (_parameters.isDuplicates() ? 1 : 0);    
    _caselessWindows.resize(_parameters.getModelType() == Parameters::TEMPORALSCAN ? _parameters.getDataTimeRangeSet().getTotalDaysAcrossRangeSets() : 1);

    int count=0, controls=0, duplicates=0, daysSinceIncidence=0;
    while (dataSource->readRecord()) {
        if (dataSource->getNumValues() != expectedColumns) {
            readSuccess = false;
            _print.Printf("Error: Record %ld in case file %s. Expecting <indentifier>, <count>,%s %s but found %ld value%s.\n", 
                          BasePrint::P_READERROR, dataSource->getCurrentRecordIndex(), 
                          (static_cast<int>(dataSource->getNumValues()) > expectedColumns) ? "has extra data" : "is missing data",
                          (_parameters.isDuplicates() ? " <duplicates>," : ""), 
                          (_parameters.getModelType() == Parameters::TEMPORALSCAN ? "<time>" : ""),
                          dataSource->getNumValues(), (dataSource->getNumValues() == 1 ? "" : "s"));
            continue;
        }
        ScanRunner::Index_t index = getNodeIndex(dataSource->getValueAt(0));
        if (!index.first) {
            readSuccess = false;
            _print.Printf("Error: Record %ld in case file references unknown node (%s).\n", BasePrint::P_READERROR, dataSource->getCurrentRecordIndex(), dataSource->getValueAt(0).c_str());
            continue;
        }
        NodeStructure * node = _Nodes.at(index.second);
        if  (!string_to_type<int>(dataSource->getValueAt(1).c_str(), count) || count < 0) {
            readSuccess = false;
            _print.Printf("Error: Record %ld in case file references negative number of cases in node '%s'.\n", BasePrint::P_READERROR, dataSource->getCurrentRecordIndex(), node->getIdentifier().c_str());
            continue;
        }
        if (_parameters.isDuplicates()) {
            if  (!string_to_type<int>(dataSource->getValueAt(2).c_str(), duplicates) || duplicates < 0) {
                readSuccess = false;
                _print.Printf("Error: Record %ld in case file references negative number of duplicates in node '%s'.\n", BasePrint::P_READERROR, dataSource->getCurrentRecordIndex(), node->getIdentifier().c_str());
                continue;
            } else if (duplicates > count) {
                readSuccess = false;
                _print.Printf("Error: Record %ld in case file references more duplicates than case in node '%s'.\n", BasePrint::P_READERROR, dataSource->getCurrentRecordIndex(), node->getIdentifier().c_str());
                continue;
            }
            node->refDuplicates() += duplicates;
        }
        // read model specific columns from data source
        if (_parameters.getModelType() == Parameters::POISSON) {
            node->refIntC_C().front() += count;
        } else if (_parameters.getModelType() == Parameters::BERNOULLI) {
            node->refIntC_C().front() += count;
        } else if (_parameters.getModelType() == Parameters::TEMPORALSCAN) {
            if  (!string_to_type<int>(dataSource->getValueAt(expectedColumns - 1).c_str(), daysSinceIncidence)) {
                readSuccess = false;
                _print.Printf("Error: Record %ld in case file references an invalid 'day since incidence' value in node '%s'.\n", BasePrint::P_READERROR, dataSource->getCurrentRecordIndex(), node->getIdentifier().c_str());
                continue;
            }
            // check that the 'daysSinceIncidence' is within a defined data time range
            DataTimeRangeSet::rangeset_index_t rangeIdx = _parameters.getDataTimeRangeSet().getDataTimeRangeIndex(daysSinceIncidence);
            if (rangeIdx.first == false) {
                readSuccess = false;
                _print.Printf("Error: Record %ld in case file references an invalid 'day since incidence' value in node '%s'.\n"
                              "The specified value is not within any of the data time ranges you have defined.",
                              BasePrint::P_READERROR, dataSource->getCurrentRecordIndex(), node->getIdentifier().c_str());
                continue;
            }
            node->refIntC_C().at(daysSinceIncidence + _zero_translation_additive) += count;
            _caselessWindows.set(daysSinceIncidence + _zero_translation_additive);
        } else throw prg_error("Unknown model type (%d).", "readCounts()", _parameters.getModelType());
    }

    _caselessWindows.flip(); // flip so that windows without cases are on instead of off
    if (_parameters.getModelType() == Parameters::TEMPORALSCAN && _caselessWindows.count() > 0) {
        std::string buffer;
        _print.Printf("Warning: The following days in the data time range do not have cases: %s\n", BasePrint::P_WARNING, getCaselessWindowsAsString(buffer).c_str());
    }

    return readSuccess;
}

/** Creates string detailing indexes and range of indexes which do not have cases. */
std::string & ScanRunner::getCaselessWindowsAsString(std::string& s) const {
    std::stringstream buffer;
    s.clear();

    boost::dynamic_bitset<>::size_type p = _caselessWindows.find_first();
    if (p == boost::dynamic_bitset<>::npos) return s;

    boost::dynamic_bitset<>::size_type pS=p, pE=p;
    do {
        p = _caselessWindows.find_next(p);
        if (p == boost::dynamic_bitset<>::npos || p > pE + 1) {
            // print range if at end of bit set or gap in range found
            if (pS == pE) {
                buffer << (static_cast<int>(pS) - getZeroTranslationAdditive());
            } else {
                buffer << (static_cast<int>(pS) - getZeroTranslationAdditive()) << " to " << (static_cast<int>(pE) - getZeroTranslationAdditive());
            }
            if (p != boost::dynamic_bitset<>::npos) {
                buffer << ", ";
            }
            pS=p;
            pE=p;
        } else {
            pE = p; // increase end point of range
        }
    } while (p != boost::dynamic_bitset<>::npos);

    s = buffer.str().c_str();
    return s;
}

/*
 Reads population data from passed file. The file format is: <node identifier>, <population>, <time>
 */
bool ScanRunner::readPopulation(const std::string& filename) {
    if (_parameters.getModelType() == Parameters::TEMPORALSCAN) return true; // population is not used with temporal scan

    _print.Printf("Reading population file ...\n", BasePrint::P_STDOUT);
    bool readSuccess=true;
    std::auto_ptr<DataSource> dataSource(DataSource::getNewDataSourceObject(filename));
    int expectedColumns = 2;

    while (dataSource->readRecord()) {
        if (dataSource->getNumValues() != expectedColumns) {
            readSuccess = false;
            _print.Printf("Error: Record %ld in population file %s. Expecting <indentifier>, <population> but found %ld value%s.\n", 
                          BasePrint::P_READERROR, dataSource->getCurrentRecordIndex(), (static_cast<int>(dataSource->getNumValues()) > (_parameters.isDuplicates() ? 4 : 3)) ? "has extra data" : "is missing data",
                          dataSource->getNumValues(), (dataSource->getNumValues() == 1 ? "" : "s"));
            continue;
        }
        ScanRunner::Index_t index = getNodeIndex(dataSource->getValueAt(0));
        if (!index.first) {
            readSuccess = false;
            _print.Printf("Error: Record %ld in count file references unknown node (%s).\n", BasePrint::P_READERROR, dataSource->getCurrentRecordIndex(), dataSource->getValueAt(0).c_str());
            continue;
        }
        NodeStructure * node = _Nodes.at(index.second);
        // read model specific columns from data source
        if (_parameters.getModelType() == Parameters::POISSON) {
            double population=0;
            if  (!string_to_type<double>(dataSource->getValueAt(expectedColumns - 1).c_str(), population) || population < 0) {
                readSuccess = false;
                _print.Printf("Error: Record %ld in population file references negative population in node '%s'.\n", BasePrint::P_READERROR, dataSource->getCurrentRecordIndex(), node->getIdentifier().c_str());
                continue;
            }
            node->refIntN_C().front() += population;
        } else if (_parameters.getModelType() == Parameters::BERNOULLI) {
            int population=0;
            if  (!string_to_type<int>(dataSource->getValueAt(expectedColumns - 1).c_str(), population) || population < 0) {
                readSuccess = false;
                _print.Printf("Error: Record %ld in population file references negative number of cases and controls in node '%s'.\n", BasePrint::P_READERROR, dataSource->getCurrentRecordIndex(), node->getIdentifier().c_str());
                continue;
            }
            node->refIntN_C().front() += population;
        } else throw prg_error("Unknown model type (%d).", "readPopulation()", _parameters.getModelType());
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

    // TODO: Eventually this will need refactoring once we implement multiple data time ranges.    
    size_t daysInDataTimeRange = _parameters.getModelType() == Parameters::TEMPORALSCAN ? _parameters.getDataTimeRangeSet().getTotalDaysAcrossRangeSets() : 1;

    // first collect all nodes -- this will allow the referencing of parent nodes not yet encountered
    while (dataSource->readRecord()) {
        std::string identifier = dataSource->getValueAt(0);
        std::auto_ptr<NodeStructure> node(new NodeStructure(trimString(identifier), _parameters.getCutType(), _parameters.getModelType(), daysInDataTimeRange));
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
    Parameters::cut_maps_t cut_type_maps = Parameters::getCutTypeMap();

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

            Parameters::cut_map_t::iterator itr_abbr = cut_type_maps.first.find(cut_type_string);
            Parameters::cut_map_t::iterator itr_full = cut_type_maps.second.find(cut_type_string);
            if (itr_abbr == cut_type_maps.first.end() && itr_full == cut_type_maps.second.end()) {
                readSuccess = false;
                _print.Printf("Error: Record %ld in cut file has unknown cut type (%s). Choices are simple, pairs, triplets and ordinal.\n", BasePrint::P_READERROR, dataSource->getCurrentRecordIndex(), dataSource->getValueAt(static_cast<long>(1)).c_str());
            } else {
                node->setCutType(itr_abbr == cut_type_maps.first.end() ? itr_full->second : itr_abbr->second);
            }
        }
    }
    return readSuccess;
}

/* REPORT RESULTS */
bool ScanRunner::reportResults(time_t start, time_t end) const {
    ResultsFileWriter resultsWriter(*this);

	bool status = resultsWriter.writeASCII(start, end);
	if (status && _parameters.isGeneratingHtmlResults()) {
		std::string buffer;
		status = resultsWriter.writeHTML(start, end);
	}
    // write cuts to supplemental reports file
	if (_parameters.isGeneratingTableResults()) {
		unsigned int k=0;
		CutsRecordWriter cutsWriter(*this);
		while(status && k < getCuts().size() && getCuts().at(k)->getC() > 0 && getCuts().at(k)->getRank() < _parameters.getNumReplicationsRequested() + 1) {
			cutsWriter.write(k);
			k++;
		}
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
    if (_print.GetIsCanceled()) return false;

    if (_parameters.getCutsFileName().length() && !readCuts(_parameters.getCutsFileName())) return false;
    if (_print.GetIsCanceled()) return false;

    if (!readCounts(_parameters.getCountFileName())) return false;
    if (_print.GetIsCanceled()) return false;

    if (_parameters.getModelType() != Parameters::TEMPORALSCAN) {
        if (!readPopulation(_parameters.getPopulationFileName())) return false;
    }
    if (_print.GetIsCanceled()) return false;

    if (!setupTree()) return false;
    if (_print.GetIsCanceled()) return false;

    if ((_parameters.getModelType() == Parameters::TEMPORALSCAN ? scanTreeTemporal() : scanTree())) {
        if (_print.GetIsCanceled()) return false;
        if (!runsimulations()) return false;
    }
    if (_print.GetIsCanceled()) return false;

    time(&gEndTime); //get end time
    if (!reportResults(gStartTime, gEndTime)) return false;

    return true;
}

/* DOING THE MONTE CARLO SIMULATIONS */
bool ScanRunner::runsimulations() {
    const char          * sReplicationFormatString = "The result of Monte Carlo replica #%u of %u replications is: %lf\n";
    unsigned long         ulParallelProcessCount = std::min(_parameters.getNumParallelProcessesToExecute(), _parameters.getNumReplicationsRequested());

    try {
        if (_parameters.getNumReplicationsRequested() == 0 || _Cut.size() == 0)
            return true;

        _print.Printf("Doing the %d Monte Carlo simulations ...\n", BasePrint::P_STDOUT, _parameters.getNumReplicationsRequested());

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

/* Determines number of cuts in all nodes, given each nodes cut type. */
size_t ScanRunner::calculateCutsCount() const {
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
            case Parameters::TRIPLETS: cuts += z * (z - 1)/2 + static_cast<size_t>(getNumCombinations(z, 3)); break;
            //case Parameters::COMBINATORIAL: cuts += std::pow(2.0,z) - z - 2.0; break;
            default: throw prg_error("Unknown cut type (%d).", "scanTree()", cutType);
        };
    }
    //for(unsigned int k=0; k < cuts; k++) _Cut.push_back(new CutStructure());
    return cuts;
}

/* SCANNING THE TREE */
bool ScanRunner::scanTree() {
    _print.Printf("Scanning the tree ...\n", BasePrint::P_STDOUT);
    ScanRunner::Loglikelihood_t calcLogLikelihood(AbstractLoglikelihood::getNewLoglikelihood(_parameters, _TotalC, _TotalN));

    //std::string buffer;
    //int hits=0;
    for (size_t n=0; n < _Nodes.size(); ++n) {
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
    if (_Cut.size()) {
        std::sort(_Cut.begin(), _Cut.end(), CompareCutsByLoglikelihood());
        _print.Printf("The log likelihood ratio of the most likely cut is %lf.\n", BasePrint::P_STDOUT, calcLogLikelihood->LogLikelihoodRatio(_Cut.at(0)->getLogLikelihood()));
    }
    return _Cut.size() != 0;
}

/* SCANNING THE TREE for temporal model */
bool ScanRunner::scanTreeTemporal() {
    _print.Printf("Scanning the tree.\n", BasePrint::P_STDOUT);
    ScanRunner::Loglikelihood_t calcLogLikelihood(AbstractLoglikelihood::getNewLoglikelihood(_parameters, _TotalC, _TotalN));

    // Define the start and end windows with the zero index offset already incorporated.
    DataTimeRange startWindow(_parameters.getTemporalStartRange().getStart() + _zero_translation_additive,
                              _parameters.getTemporalStartRange().getEnd() + _zero_translation_additive),
                  endWindow(_parameters.getTemporalEndRange().getStart() + _zero_translation_additive,
                            _parameters.getTemporalEndRange().getEnd() + _zero_translation_additive);
    //std::string buffer;
    //int hits=0;
    for (size_t n=0; n < _Nodes.size(); ++n) {
        if (_Nodes.at(n)->getBrC() > 1) {
            const NodeStructure& thisNode(*(_Nodes.at(n)));
            Parameters::CutType cutType = thisNode.getChildren().size() >= 2 ? thisNode.getCutType() : Parameters::SIMPLE;
            switch (cutType) {
                case Parameters::SIMPLE:
                    //printf("Evaluating cut [%s]\n", thisNode.getIdentifier().c_str());
                    for (DataTimeRange::index_t end=endWindow.getStart(); end < endWindow.getEnd(); ++end) {
                        for (DataTimeRange::index_t start=startWindow.getStart(); start < startWindow.getEnd(); ++start) {
                            updateCuts(n, thisNode.getBrC_C()[start] - thisNode.getBrC_C()[end], static_cast<NodeStructure::expected_t>(thisNode.getBrC()), calcLogLikelihood, start, end);
                        }
                    }
                    //++hits; printf("hits %d\n", hits);
                    break;
                case Parameters::ORDINAL:
                    for (DataTimeRange::index_t end=endWindow.getStart(); end < endWindow.getEnd(); ++end) {
                        for (DataTimeRange::index_t start=startWindow.getStart(); start < startWindow.getEnd(); ++start) {
                            for (size_t i=0; i < thisNode.getChildren().size() - 1; ++i) {
                                const NodeStructure& firstChildNode(*(_Nodes.at(thisNode.getChildren().at(i))));
                                //buffer = firstChildNode.getIdentifier().c_str();
                                NodeStructure::count_t branchWindow = firstChildNode.getBrC_C()[start] - firstChildNode.getBrC_C()[end];
                                NodeStructure::expected_t branchSum = static_cast<NodeStructure::expected_t>(firstChildNode.getBrC());
                                for (size_t j=i+1; j < thisNode.getChildren().size(); ++j) {
                                    const NodeStructure& childNode(*(_Nodes.at(thisNode.getChildren().at(j))));
                                    //buffer += ",";
                                    //buffer += childNode.getIdentifier();
                                    branchWindow += childNode.getBrC_C()[start] - childNode.getBrC_C()[end];
                                    branchSum += static_cast<NodeStructure::expected_t>(childNode.getBrC());
                                    //printf("Evaluating cut [%s]\n", buffer.c_str());
                                    updateCuts(n, branchWindow, branchSum, calcLogLikelihood, start, end);
                                    //++hits; printf("hits %d\n", hits);
                                }
                            }
                        } 
                    } break;
                case Parameters::PAIRS:
                    for (DataTimeRange::index_t end=endWindow.getStart(); end < endWindow.getEnd(); ++end) {
                        for (DataTimeRange::index_t start=startWindow.getStart(); start < startWindow.getEnd(); ++start) {
                            for (size_t i=0; i < thisNode.getChildren().size() - 1; ++i) {
                                const NodeStructure& startChildNode(*(_Nodes.at(thisNode.getChildren().at(i))));
                                NodeStructure::count_t startBranchWindow = startChildNode.getBrC_C()[start] - startChildNode.getBrC_C()[end];
                                NodeStructure::expected_t startBranchSum = static_cast<NodeStructure::expected_t>(startChildNode.getBrC());
                                for (size_t j=i+1; j < thisNode.getChildren().size(); ++j) {
                                    const NodeStructure& stopChildNode(*(_Nodes.at(thisNode.getChildren().at(j))));
                                    //printf("Evaluating cut [%s,%s]\n", startChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                    NodeStructure::count_t stopBranchWindow = stopChildNode.getBrC_C()[start] - stopChildNode.getBrC_C()[end];
                                    NodeStructure::expected_t stopBranchSum = static_cast<NodeStructure::expected_t>(stopChildNode.getBrC());
                                    updateCuts(n, startBranchWindow + stopBranchWindow, startBranchSum + stopBranchSum, calcLogLikelihood, start, end);
                                    //++hits; printf("hits %d\n", hits);
                                }
                            }
                        }
                    } break;
                case Parameters::TRIPLETS:
                    for (DataTimeRange::index_t end=endWindow.getStart(); end < endWindow.getEnd(); ++end) {
                        for (DataTimeRange::index_t start=startWindow.getStart(); start < startWindow.getEnd(); ++start) {
                            for (size_t i=0; i < thisNode.getChildren().size() - 1; ++i) {
                                const NodeStructure& startChildNode(*(_Nodes.at(thisNode.getChildren().at(i))));
                                NodeStructure::count_t startBranchWindow = startChildNode.getBrC_C()[start] - startChildNode.getBrC_C()[end];
                                NodeStructure::expected_t startBranchSum = static_cast<NodeStructure::expected_t>(startChildNode.getBrC());
                                for (size_t j=i+1; j < thisNode.getChildren().size(); ++j) {
                                    const NodeStructure& stopChildNode(*(_Nodes.at(thisNode.getChildren().at(j))));
                                    //printf("Evaluating cut [%s,%s]\n", startChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                    NodeStructure::count_t stopBranchWindow = stopChildNode.getBrC_C()[start] - stopChildNode.getBrC_C()[end];
                                    NodeStructure::expected_t stopBranchSum = static_cast<NodeStructure::expected_t>(stopChildNode.getBrC());
                                    updateCuts(n, startBranchWindow + stopBranchWindow, startBranchSum + stopBranchSum, calcLogLikelihood, start, end);
                                    //++hits;printf("hits %d\n", hits);
                                    for (size_t k=i+1; k < j; ++k) {
                                        const NodeStructure& middleChildNode(*(_Nodes.at(thisNode.getChildren().at(k))));
                                        NodeStructure::count_t middleBranchWindow = middleChildNode.getBrC_C()[start] - middleChildNode.getBrC_C()[end];
                                        NodeStructure::expected_t middleBranchSum = static_cast<NodeStructure::expected_t>(middleChildNode.getBrC());
                                        //printf("Evaluating cut [%s,%s,%s]\n", startChildNode.getIdentifier().c_str(), middleChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                        updateCuts(n, startBranchWindow + middleBranchWindow + stopBranchWindow, startBranchSum + middleBranchSum + stopBranchSum, calcLogLikelihood, start, end);
                                        //++hits; printf("hits %d\n", hits);
                                    }
                                }
                            }
                        }
                    } break;
                case Parameters::COMBINATORIAL:
                default: throw prg_error("Unknown cut type (%d).", "scanTreeTemporal()", cutType);
            }
        }
    }
    if (_Cut.size()) {
        std::sort(_Cut.begin(), _Cut.end(), CompareCutsByLoglikelihood());
        _print.Printf("The log likelihood ratio of the most likely cut is %lf.\n", BasePrint::P_STDOUT, calcLogLikelihood->LogLikelihoodRatio(_Cut.at(0)->getLogLikelihood()));
    }
    return _Cut.size() != 0;
}

void ScanRunner::updateCuts(int node_index, int BrC, double BrN, const Loglikelihood_t& logCalculator, DataTimeRange::index_t startIdx, DataTimeRange::index_t endIdx) {
    double loglikelihood = _parameters.getModelType() == Parameters::TEMPORALSCAN ? logCalculator->LogLikelihood(BrC, BrN, endIdx - startIdx + 1) : logCalculator->LogLikelihood(BrC, BrN); 
    if (loglikelihood == logCalculator->UNSET_LOGLIKELIHOOD) return;

    std::auto_ptr<CutStructure> cut(new CutStructure());
    cut->setLogLikelihood(loglikelihood);
    cut->setID(static_cast<int>(node_index));
    cut->setC(BrC);
    cut->setN(BrN);
    cut->setStartIdx(startIdx);
    cut->setEndIdx(endIdx);
    CutStructureContainer_t::iterator itr = std::lower_bound(_Cut.begin(), _Cut.end(), cut.get(), CompareCutsById());
    if (itr != _Cut.end() && (*itr)->getID() == cut->getID()) {
        if (cut->getLogLikelihood() > (*itr)->getLogLikelihood()) {
            size_t idx = std::distance(_Cut.begin(), itr);
            delete _Cut.at(idx); _Cut.at(idx)=0;
            _Cut.at(idx) = cut.release();
        }
    } else {
        _Cut.insert(itr, cut.release());
    }
}

/*
 SETTING UP THE TREE
 */
bool ScanRunner::setupTree() {
    double   adjustN;
    int     parent;

    _print.Printf("Setting up the tree ...\n", BasePrint::P_STDOUT);

    // Initialize variables
    _TotalC=0;_TotalN=0;
    for(NodeStructureContainer_t::iterator itr=_Nodes.begin(); itr != _Nodes.end(); ++itr) {
        std::fill((*itr)->refBrC_C().begin(), (*itr)->refBrC_C().end(), 0);
        std::fill((*itr)->refBrN_C().begin(), (*itr)->refBrN_C().end(), 0);
    }

    // Calculates the total number of cases and the total population at risk
    for(NodeStructureContainer_t::iterator itr=_Nodes.begin(); itr != _Nodes.end(); ++itr) {
        _TotalC = std::accumulate((*itr)->refIntC_C().begin(), (*itr)->refIntC_C().end(), _TotalC);
        _TotalN = std::accumulate((*itr)->refIntN_C().begin(), (*itr)->refIntN_C().end(), _TotalN);
    }
    // controls are read with population for Bernoulli -- so calculate total controls now
    if (_parameters.getModelType() == Parameters::BERNOULLI) {
        _TotalControls = std::max(static_cast<int>(_TotalN) - _TotalC, 0);
    }

    // Calculates the expected counts for each node and the total.
    if (_parameters.getModelType() == Parameters::POISSON && _parameters.getConditionalType() == Parameters::TOTALCASES) {
        adjustN = _TotalC/_TotalN;
        for (NodeStructureContainer_t::iterator itr=_Nodes.begin(); itr != _Nodes.end(); ++itr)
            std::transform((*itr)->getIntN_C().begin(), (*itr)->getIntN_C().end(), (*itr)->refIntN_C().begin(), std::bind1st(std::multiplies<double>(), adjustN)); // (*itr)->refIntN() *= adjustN;
        _TotalN = _TotalC;
    }

    // For each node, calculates the observed and expected number of cases for that
    // node together with all of its children, grandchildren, etc.
    // Checks whether anforlust is true or false for each node.
    // Also checks whether a node is an ancestor to itslef, which is not allowed.
    _Ancestor.resize(_Nodes.size(), 0);
    for (size_t i=0; i < _Nodes.size(); ++i) {
        std::fill(_Ancestor.begin(), _Ancestor.end(), 0);
        addCN_C(static_cast<int>(i), _Nodes.at(i)->refIntC_C(), _Nodes.at(i)->refIntN_C());
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

    if (_parameters.getModelType() == Parameters::POISSON || _parameters.getModelType() == Parameters::BERNOULLI) {
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
    }

    // Now we can set the data structures of NodeStructure to cumulative -- only relevant for temporal model since other models have one element arrays.
    std::for_each(_Nodes.begin(), _Nodes.end(), std::mem_fun(&NodeStructure::setCumulative));

    return true;
}
