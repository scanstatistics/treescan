
#include<boost/tokenizer.hpp>
#include <boost/regex.hpp>

#include "ScanRunner.h"
#include "UtilityFunctions.h"
#include "FileName.h"
#include "MonteCarloSimFunctor.h"
#include "MCSimJobSource.h"
#include "contractor.h"
#include "Randomization.h"
#include "ParametersPrint.h"
#include "DataFileWriter.h"

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
    for(size_t j=0; j < _Nodes.at(id)->getParent().size(); j++) {
      int parent = _Nodes.at(id)->getParent().at(j);
        if(_Ancestor.at(parent) == 0)
          addCN(parent,c,n);
        else
          _Ancestor.at(parent) = _Ancestor.at(parent) + 1;
    }
}

/*
 Adds simulated cases up the tree from branhes to all its parents, and so on,
 for a node without anforlust.
 */
void ScanRunner::addSimC(int id, int c) {
    _Nodes.at(id)->refSimBrC() += c;
    for(size_t j=0; j < _Nodes.at(id)->getParent().size(); j++) addSimC(_Nodes.at(id)->getParent()[j], c);
}

/*
 Adds simulated cases up the tree from branhes to all its parents, and so on,
 for a node with anforlust.
 Note: This code can be made more efficient by storing in memory the ancestral
 nodes that should be updated with additional simlated cases from the node
 with internal cases. To do sometime in the future.
 */
void ScanRunner::addSimCAnforlust(int id, int c) {
    _Nodes.at(id)->refSimBrC() += c;
    for(size_t j=0; j < _Nodes.at(id)->getParent().size();j++) addSimCAnforlust(_Nodes.at(id)->getParent()[j], c);
}

ScanRunner::Loglikelihood_t ScanRunner::getLoglikelihood() const {
    if (_parameters.isConditional())
        return Loglikelihood_t(new PoissonLoglikelihood(_TotalC, _TotalN));
    else
        return Loglikelihood_t(new UnconditionalPoissonLoglikelihood());
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
    _print.Printf("Reading count file ...\n", BasePrint::P_STDOUT);

    bool readSuccess=true;
    std::ifstream in;
    in.open(filename.c_str());

    if (!in) {
        _print.Printf("Error: Unable to open count file: %s.\n", BasePrint::P_READERROR, filename.c_str());
        return false;
    }

    // first collect all nodes -- this will allow the referencing of parent nodes not yet encountered
    std::string line;
    unsigned int record_number=0;
    int count=0, duplicates=0;
    double population=0;
    while (getlinePortable(in, line)) {
        ++record_number;
        boost::tokenizer<boost::escaped_list_separator<char> > csv(line);
        boost::tokenizer<boost::escaped_list_separator<char> >::const_iterator itr=csv.begin();
        std::vector<std::string> values;
        for (;itr != csv.end(); ++itr) {std::string identifier = (*itr); values.push_back(trimString(identifier));}
        if (values.size() == 0) continue; // skip records with no data
        if (values.size() != (_parameters.isDuplicates() ? 4 : 3)) {
            readSuccess = false;
            _print.Printf("Error: Record %ld in count file %s. Expecting <indentifier>, <count>,%s <population> but found %ld value%s.\n", 
                          BasePrint::P_READERROR, record_number, (static_cast<int>(values.size()) > (_parameters.isDuplicates() ? 4 : 3)) ? "has extra data" : "is missing data",
                          (_parameters.isDuplicates() ? " <duplicates>," : ""), values.size(), (values.size() == 1 ? "" : "s"));
            continue;
        }
        ScanRunner::Index_t index = getNodeIndex(values.at(0));
        if (!index.first) {
            readSuccess = false;
            _print.Printf("Error: Record %ld in count file references unknown node (%s).\n", BasePrint::P_READERROR, record_number, values.at(0).c_str());
            continue;
        }
        NodeStructure * node = _Nodes.at(index.second);
        if  (!string_to_type<int>(values.at(1).c_str(), count) || count < 0) {
            readSuccess = false;
            _print.Printf("Error: Record %ld in count file references negative number of cases in node '%s'.\n", BasePrint::P_READERROR, record_number, node->getIdentifier().c_str());
            continue;
        }
        if (_parameters.isDuplicates()) {
            if  (!string_to_type<int>(values.at(2).c_str(), duplicates) || duplicates < 0) {
                readSuccess = false;
                _print.Printf("Error: Record %ld in count file references negative number of duplicates in node '%s'.\n", BasePrint::P_READERROR, record_number, node->getIdentifier().c_str());
                continue;
            } else if (duplicates > count) {
                readSuccess = false;
                _print.Printf("Error: Record %ld in count file references more duplicates than count in node '%s'.\n", BasePrint::P_READERROR, record_number, node->getIdentifier().c_str());
                continue;
            }
        }
        if  (!string_to_type<double>(values.at(_parameters.isDuplicates() ? 3 : 2).c_str(), population) || population < 0) {
            readSuccess = false;
            _print.Printf("Error: Record %ld in count file references negative population in node '%s'.\n", BasePrint::P_READERROR, record_number, node->getIdentifier().c_str());
            continue;
        }
        node->refIntC() += count;
        node->refDuplicates() += duplicates;
        node->refIntN() += population;
    }

    return readSuccess;
}

/*
 Reads tree structure from passed file. The file format is: <node identifier>, <parent node identifier 1>, <parent node identifier 2>, ... <parent node identifier N>
 */
bool ScanRunner::readTree(const std::string& filename) {
    _print.Printf("Reading tree file ...\n", BasePrint::P_STDOUT);

    bool readSuccess=true;
    std::ifstream in;
    in.open(filename.c_str());

    if (!in) {
        _print.Printf("Error: Unable to open tree file: %s.\n", BasePrint::P_READERROR, filename.c_str());
        return false;
    }

    // first collect all nodes -- this will allow the referencing of parent nodes not yet encountered
    std::string line;
    while (getlinePortable(in, line)) {
        boost::tokenizer<boost::escaped_list_separator<char> > csv(line);
        boost::tokenizer<boost::escaped_list_separator<char> >::const_iterator itr=csv.begin();
        if (itr != csv.end()) {
            std::string identifier = (*itr);
            std::auto_ptr<NodeStructure> node(new NodeStructure(trimString(identifier)));
            NodeStructureContainer_t::iterator itr = std::lower_bound(_Nodes.begin(), _Nodes.end(), node.get(), CompareNodeStructureByIdentifier());
            if (itr == _Nodes.end() || (*itr)->getIdentifier() != node.get()->getIdentifier())
                _Nodes.insert(itr, node.release());
        }
    }

    //reset node identifiers to ordinal position in vector -- this is to keep the original algorithm intact since it uses vector indexes heavily
    for (size_t i=0; i < _Nodes.size(); ++i) _Nodes.at(i)->setID(i);

    // now set parent nodes
    in.clear();
    in.seekg(0L, std::ios::beg);
    unsigned int record_number = 0;
    while (getlinePortable(in, line)) {
        ++record_number;
        boost::tokenizer<boost::escaped_list_separator<char> > csv(line);
        boost::tokenizer<boost::escaped_list_separator<char> >::const_iterator itr=csv.begin();
        NodeStructure * node = 0;
        // assign parent nodes to node
        for (;itr != csv.end(); ++itr) {
            std::string identifier = (*itr);
            trimString(identifier);
            ScanRunner::Index_t index = getNodeIndex(identifier);
            if (!index.first) {
                readSuccess = false;
                _print.Printf("Error: Record %ld in tree file has unknown parent node (%s).\n", BasePrint::P_READERROR, record_number, identifier.c_str());
            } else {
                if (node) node->refParent().push_back(_Nodes.at(index.second)->getID());
                else node = _Nodes.at(index.second);
            }
        }
    }

    return readSuccess;
}

/* REPORT RESULTS */
bool ScanRunner::reportResults(const std::string& filename, time_t start, time_t end) const {
    std::ofstream outfile(filename.c_str());
    if (!outfile) {
        _print.Printf("Unable to create specified output file: %s\n", BasePrint::P_READERROR, filename.c_str());
        FileName currFilename(filename.c_str()), documentsfile(filename.c_str());
        std::string buffer, temp;
        documentsfile.setLocation(GetUserDocumentsDirectory(buffer, currFilename.getLocation(temp)).c_str());
        documentsfile.getFullPath(buffer);
        _print.Printf("Trying to create file in documents directory ...\n", BasePrint::P_STDOUT);
        outfile.open(documentsfile.getFullPath(buffer).c_str());
        if (!outfile) {
            _print.Printf("Unable to create output file: %s\n", BasePrint::P_READERROR, buffer.c_str());
            return false;
        }
        const_cast<Parameters&>(_parameters).setOutputFileName(buffer.c_str());
        _print.Printf("Creating the output file in documents directory: %s\n", BasePrint::P_STDOUT, buffer.c_str());
    } else
        _print.Printf("Creating the output file: %s\n", BasePrint::P_STDOUT, filename.c_str());

    AsciiPrintFormat PrintFormat;

    PrintFormat.PrintVersionHeader(outfile);
    std::string buffer = ctime(&start);
    outfile << std::endl << "Program run on: " << buffer;

    PrintFormat.SetMarginsAsSummarySection();
    PrintFormat.PrintSectionSeparatorString(outfile);
    outfile << std::endl << "SUMMARY OF DATA" << std::endl << std::endl;
    PrintFormat.PrintSectionLabel(outfile, "Analysis", false);
    buffer = (_parameters.isConditional() ? "Conditional" : "Unconditional");
    PrintFormat.PrintAlignedMarginsDataString(outfile, buffer);
    PrintFormat.PrintSectionLabel(outfile, "Total Cases", false);
    PrintFormat.PrintAlignedMarginsDataString(outfile, printString(buffer, "%ld", _TotalC));
    PrintFormat.PrintSectionLabel(outfile, "Total Measure", false);
    PrintFormat.PrintAlignedMarginsDataString(outfile, printString(buffer, "%lf", _TotalN));
    PrintFormat.PrintSectionSeparatorString(outfile, 0, 2);

    // write detected cuts
    //if (_parameters.isDuplicates()) outfile << "#CasesWithoutDuplicates ";
    //outfile << "#Exp O/E ";
    //if (_parameters.isDuplicates()) outfile << "O/EWithoutDuplicates ";
    //outfile << "LLR pvalue" << std::endl;

    if (_Cut.at(0)->getC() == 0) {
        outfile << "No clusters were found." << std::endl;
        outfile.close();
        return true;
    }

    outfile << "Most Likely Cuts"<< std::endl << std::endl;

    std::string format, replicas;
    CutsRecordWriter  cutsWriter(*this);

    printString(replicas, "%u", _parameters.getNumReplicationsRequested());
    printString(format, "%%.%dlf", replicas.size());

    unsigned int k=0;
    outfile.setf(std::ios::fixed);
    outfile.precision(5);
    while( k < _parameters.getCuts() && _Cut.at(k)->getC() > 0 && _Rank.at(k) < _parameters.getNumReplicationsRequested() + 1) {
        PrintFormat.SetMarginsAsClusterSection( k + 1);
        outfile << k + 1 << ")";
        PrintFormat.PrintSectionLabel(outfile, "Node Identifier", false);
        PrintFormat.PrintAlignedMarginsDataString(outfile, _Nodes.at(_Cut.at(k)->getID())->getIdentifier());

        PrintFormat.PrintSectionLabel(outfile, "Number of Cases", true);
        printString(buffer, "%ld", _Cut.at(k)->getC());
        PrintFormat.PrintAlignedMarginsDataString(outfile, buffer);
        if (_parameters.isDuplicates()) {
            PrintFormat.PrintSectionLabel(outfile, "Cases (Duplicates Removed)", true);
            printString(buffer, "%ld", _Cut.at(k)->getC() - _Nodes.at(_Cut.at(k)->getID())->getDuplicates());
            PrintFormat.PrintAlignedMarginsDataString(outfile, buffer);
        }
        PrintFormat.PrintSectionLabel(outfile, "Expected", true);
        PrintFormat.PrintAlignedMarginsDataString(outfile, getValueAsString(_Cut.at(k)->getN(), buffer));
        PrintFormat.PrintSectionLabel(outfile, "Observed/Expected", true);
        PrintFormat.PrintAlignedMarginsDataString(outfile, getValueAsString(_Cut.at(k)->getC()/_Cut.at(k)->getN(), buffer));
        if (_parameters.isDuplicates()) {
            PrintFormat.PrintSectionLabel(outfile, "O/E (Duplicates Removed)", true);
            PrintFormat.PrintAlignedMarginsDataString(outfile, getValueAsString((_Cut.at(k)->getC() - _Nodes.at(_Cut.at(k)->getID())->getDuplicates())/_Cut.at(k)->getN(), buffer));
        }
        PrintFormat.PrintSectionLabel(outfile, "Log Likelihood Ratio", true);
        if (_parameters.isConditional())
            printString(buffer, "%lf", _Cut.at(k)->getLogLikelihood() - _TotalC * log(_TotalC/_TotalN));
        else
            printString(buffer, "%lf", _Cut.at(k)->getLogLikelihood());
        PrintFormat.PrintAlignedMarginsDataString(outfile, buffer);
        PrintFormat.PrintSectionLabel(outfile, "P-value", true);
        printString(buffer, format.c_str(), (double)_Rank.at(k) /(_parameters.getNumReplicationsRequested() + 1));
        PrintFormat.PrintAlignedMarginsDataString(outfile, buffer, 2);

        cutsWriter.write(k);
        k++;
    }

    //outfile << std::endl << std::endl;
    //outfile << "Information About Each Node" << std::endl;
    //outfile << "ID Obs Exp O/E" << std::endl;
    // outfile.width(10);
    //for (size_t i=0; i < _Nodes.size(); i++)
    //    if (_Nodes.at(i)->_BrN > 0)
    //        outfile << "0 " << _Nodes.at(i)->_identifier.c_str() << " " << _Nodes.at(i)->_BrC << " " << _Nodes.at(i)->_BrN << " " << _Nodes.at(i)->_BrC/_Nodes.at(i)->_BrN << " 0 0 " << std::endl;

    ParametersPrint(_parameters).Print(outfile);
    double nTotalTime = difftime(end, start);
    double nHours     = floor(nTotalTime/(60*60));
    double nMinutes   = floor((nTotalTime - nHours*60*60)/60);
    double nSeconds   = nTotalTime - (nHours*60*60) - (nMinutes*60);
    outfile << "Program completed  : " << ctime(&end);
    const char * szHours = (0 < nHours && nHours < 1.5 ? "hour" : "hours");
    const char * szMinutes = (0 < nMinutes && nMinutes < 1.5 ? "minute" : "minutes");
    const char * szSeconds = (0.5 <= nSeconds && nSeconds < 1.5 ? "second" : "seconds");
    if (nHours > 0) printString(buffer, "Total Running Time : %.0f %s %.0f %s %.0f %s", nHours, szHours, nMinutes, szMinutes, nSeconds, szSeconds);
    else if (nMinutes > 0) printString(buffer, "Total Running Time : %.0f %s %.0f %s", nMinutes, szMinutes, nSeconds, szSeconds);
    else printString(buffer, "Total Running Time : %.0f %s",nSeconds, szSeconds);
    outfile << buffer << std::endl;
    if (_parameters.getNumParallelProcessesToExecute() > 1) outfile << "Processor Usage    : " << _parameters.getNumParallelProcessesToExecute() << " processors";

    outfile.close();
    return true;
}

/*
 Run Scan.
 */
bool ScanRunner::run(const std::string& treefile, const std::string& countfile, const std::string& outputfile) {
    time_t gStartTime, gEndTime;
    time(&gStartTime); //get start time

    if (!readTree(treefile)) return false;
    if (!readCounts(countfile)) return false;
    if (!setupTree()) return false;
    if (!scanTree()) return false;
    if (!runsimulations()) return false;

    time(&gEndTime); //get end time
    if (!reportResults(outputfile, gStartTime, gEndTime)) return false;

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
        _Rank.resize(_parameters.getCuts(), 1);

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
                    MCSimSuccessiveFunctor mcsf(thread_mutex, boost::shared_ptr<AbstractRandomizer>(new PoissonRandomizer(_parameters.isConditional(), _TotalC, _TotalN)), *this);
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
    double LogLikelihoodRatio=0;
    ScanRunner::Loglikelihood_t calcLogLikelihood = ScanRunner::getLoglikelihood();

    for(unsigned int k=0; k < _parameters.getCuts(); k++) _Cut.push_back(new CutStructure());

    for (size_t i=0; i < _Nodes.size(); i++) {
        if (_Nodes.at(i)->getBrC() > 1) {
            if (_parameters.isDuplicates())
                loglikelihood = calcLogLikelihood->LogLikelihood(_Nodes.at(i)->getBrC() - _Nodes.at(i)->getDuplicates(), _Nodes.at(i)->getBrN());
            else {
                loglikelihood = calcLogLikelihood->LogLikelihood(_Nodes.at(i)->getBrC(), _Nodes.at(i)->getBrN());
            }

            unsigned int k = 0;
            while(loglikelihood < _Cut.at(k)->getLogLikelihood() && k < _parameters.getCuts()) k++;
            if (k < _parameters.getCuts()) {
                for (unsigned int m = _parameters.getCuts() - 1; m > k ; m--) {
                    _Cut.at(m)->setLogLikelihood(_Cut.at(m-1)->getLogLikelihood());
                    _Cut.at(m)->setID(_Cut.at(m-1)->getID());
                    _Cut.at(m)->setC(_Cut.at(m-1)->getC());
                    _Cut.at(m)->setN(_Cut.at(m-1)->getN());
                }
                _Cut.at(k)->setLogLikelihood(loglikelihood);
                _Cut.at(k)->setID(i);
                _Cut.at(k)->setC(_Nodes.at(i)->getBrC());
                _Cut.at(k)->setN(_Nodes.at(i)->getBrN());
            }
        }
    }
    if (_parameters.isConditional()) LogLikelihoodRatio = _Cut.at(0)->getLogLikelihood() - _TotalC * log(_TotalC/_TotalN);
    else LogLikelihoodRatio = _Cut.at(0)->getLogLikelihood();
    _print.Printf("The log likelihood ratio of the most likely cut is %lf.\n", BasePrint::P_STDOUT, LogLikelihoodRatio);

    return true;
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
    if (_parameters.isConditional()) {
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
        addCN(i, _Nodes.at(i)->getIntC(), _Nodes.at(i)->getIntN());
        if (_Ancestor[i] > 1) {
            _print.Printf("Error: Node '%s' has itself as an ancestor.\n", BasePrint::P_ERROR, _Nodes.at(i)->getIdentifier().c_str());
            return false;
        } // if Ancestor[i]>1
        for (size_t j=0; j < _Nodes.size(); ++j) if(_Ancestor[j] > 1) _Nodes.at(i)->setAnforlust(true);
    } // for i<nNodes

    // For each node calculates the number of children and sets up the list of child IDs
    for (size_t i=0; i < _Nodes.size(); ++i) {
        for (size_t j=0; j < _Nodes.at(i)->getParent().size(); ++j) {
            parent = _Nodes.at(i)->getParent().at(j);
            _Nodes.at(parent)->refChild().push_back(i);
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
