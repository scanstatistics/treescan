
#include <iostream>
#include <string>
#include <fstream>
#include <iostream>
#include<boost/tokenizer.hpp>
#include <boost/regex.hpp>

#include "ScanRunner.h"
#include "UtilityFunctions.h"
#include "FileName.h"
#include "MonteCarloSimFunctor.h"
#include "MCSimJobSource.h"
#include "contractor.h"
#include "Randomization.h"

/*
 Adds cases and measure through the tree from each node through the tree to
 all its parents, and so on, to all its ancestors as well.
 If a node is a decendant to an ancestor in more than one way, the cases and
 measure is only added once to that ancestor.
 */
void ScanRunner::addCN(int id, int c, double n) {
    _Ancestor.at(id) = 1;
    _Nodes.at(id)->_BrC += c;
    _Nodes.at(id)->_BrN += n;
    for(size_t j=0; j < _Nodes.at(id)->_Parent.size(); j++) {
      int parent = _Nodes.at(id)->_Parent.at(j);
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
    _Nodes.at(id)->_SimBrC += c;
    for(size_t j=0; j < _Nodes.at(id)->_Parent.size(); j++) addSimC(_Nodes.at(id)->_Parent[j], c);
}

/*
 Adds simulated cases up the tree from branhes to all its parents, and so on,
 for a node with anforlust.
 Note: This code can be made more efficient by storing in memory the ancestral
 nodes that should be updated with additional simlated cases from the node
 with internal cases. To do sometime in the future.
 */
void ScanRunner::addSimCAnforlust(int id, int c) {
    _Nodes.at(id)->_SimBrC += c;
    for(size_t j=0; j < _Nodes.at(id)->_Parent.size();j++) addSimCAnforlust(_Nodes.at(id)->_Parent[j], c);
}

/*
 ------ Returns a binomial(n,p) distributed random variable -------------------
 Note: SaTScan has a faster way of doing this.
*/
int ScanRunner::BinomialGenerator(int n, double p) {
    int     j;
    int     binomial;

    if(p==0) return 0;
    binomial=0;
    for (j=1;j<=n;j++) {
        if (RandomUniform() < p) binomial += 1;
    }
    return binomial;
}

ScanRunner::Loglikelihood_t ScanRunner::getLoglikelihood() const {
    if (_Conditional)
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
    if (itr != _Nodes.end() && (*itr)->_identifier == node.get()->_identifier) {
      size_t tt = std::distance(_Nodes.begin(), itr);
      assert(tt == (*itr)->_ID);
      return std::make_pair(true, std::distance(_Nodes.begin(), itr));
    } else
        return std::make_pair(false, 0);
}

/*
 Returns a Poisson distributed random variable.
 */
int ScanRunner::PoissonGenerator(double lambda) {
    if (lambda==0) return 0;

    int     x=0;
    double  r=RandomUniform(), p=exp(-lambda), logfactorial=0;
    while (p < r) {
        x++;
        logfactorial = logfactorial + log(static_cast<double>(x));
        p = p + exp(-lambda + x * log(lambda) - logfactorial);
    }
    // cout << endl << "lambda=" << lambda << " r=" << r << " x=" << x << " rr=" << rr;
    return x;
}

/*
 Returns a uniform random number in the interval [0,1].
 Should be replaced by a better random number generator.
 */
double ScanRunner::RandomUniform() {
    //double rand_num = static_cast<double>(rand());
    //double return_value = (rand_num + 0.5) / static_cast<double>(RAND_MAX+1);
    //cout << "return_value " << return_value << endl;
    //return return_value;
    return double(rand()+0.5)/(RAND_MAX+1); // This needs a "+0.05" and "+1" or RandomUniform is zero and one too often.
}

/*
 Reads count and population data from passed file. The file format is: <node identifier>, <count>, <population>
 */
bool ScanRunner::readCounts(const std::string& filename) {
    _print.Printf("Reading count file ...\n", BasePrint::P_STDOUT);

    bool readSuccess=true;
    std::ifstream in;
    in.open(filename);

    if (!in) {
        _print.Printf("Error: Unable to open count file: %s.\n", BasePrint::P_READERROR, filename.c_str());
        return false;
    }

    // first collect all nodes -- this will allow the referencing of parent nodes not yet encountered
    std::string line;
    unsigned int record_number = 0;
    while (getlinePortable(in, line)) {
        ++record_number;
        boost::tokenizer<boost::escaped_list_separator<char> > csv(line);
        boost::tokenizer<boost::escaped_list_separator<char> >::const_iterator itr=csv.begin();
        std::vector<std::string> values;
        for (;itr != csv.end(); ++itr) {std::string identifier = (*itr); values.push_back(trimString(identifier));}
        if (values.size() == 0) continue; // skip records with no data
        if (values.size() < 3) {
            readSuccess = false;
            _print.Printf("Error: Record %ld in count file is missing data. Expecting <indentifier>, <count>, <population> but found only %ld value.\n", BasePrint::P_READERROR, record_number, values.size());
        }
        ScanRunner::Index_t index = getNodeIndex(values.at(0));
        if (!index.first) {
            readSuccess = false;
            _print.Printf("Error: Record %ld in count file references unknown node (%s).\n", BasePrint::P_READERROR, record_number, values.at(0).c_str());
        }
        NodeStructure * node = _Nodes.at(index.second);
        if  (!string_to_type<int>(values.at(1).c_str(), node->_IntC) || node->_IntC < 0) {
            readSuccess = false;
            _print.Printf("Error: Record %ld in count file references negative number of cases in node '%s'.\n", BasePrint::P_READERROR, record_number, node->_identifier.c_str());
        }
        if (_Duplicates) {
            if  (!string_to_type<int>(values.at(2).c_str(), node->_Duplicates) || node->_Duplicates < 0) {
                readSuccess = false;
                _print.Printf("Error: Record %ld in count file references negative number of duplicates in node '%s'.\n", BasePrint::P_READERROR, record_number, node->_identifier.c_str());
            }
        }
        if  (!string_to_type<double>(values.at(_Duplicates ? 3 : 2).c_str(), node->_IntN) || node->_IntN < 0) {
            readSuccess = false;
            _print.Printf("Error: Record %ld in count file references negative population in node '%s'.\n", BasePrint::P_READERROR, record_number, node->_identifier.c_str());
        }
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
    in.open(filename);

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
            if (itr == _Nodes.end() || (*itr)->_identifier != node.get()->_identifier)
                _Nodes.insert(itr, node.release());
        }
    }

    //reset node identifiers to ordinal position in vector -- this is to keep the original algorithm intact since it uses vector indexes heavily
    for (size_t i=0; i < _Nodes.size(); ++i) _Nodes.at(i)->_ID = i;

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
                if (node) node->_Parent.push_back(_Nodes.at(index.second)->_ID);
                else node = _Nodes.at(index.second);
            }
        }
    }

    return readSuccess;
}

/* REPORT RESULTS */
bool ScanRunner::reportResults(const std::string& filename) {
    std::ofstream outfile(filename);
    if (!outfile) {
        _print.Printf("Unable to create specified output file: %s\n", BasePrint::P_READERROR, filename.c_str());
        FileName currFilename(filename.c_str()), documentsfile(filename.c_str());
        std::string buffer, temp;
        documentsfile.setLocation(GetUserDocumentsDirectory(buffer, currFilename.getLocation(temp)).c_str());
        documentsfile.getFullPath(buffer);
        _print.Printf("Trying to create file in documents directory ...\n", BasePrint::P_STDOUT);
        outfile.open(documentsfile.getFullPath(buffer));
        if (!outfile) {
            _print.Printf("Unable to create output file: %s\n", BasePrint::P_READERROR, buffer.c_str());
            return false;
        }
        _print.Printf("Creating the output file in documents directory: %s\n", BasePrint::P_STDOUT, buffer.c_str());
    } else
        _print.Printf("Creating the output file: %s\n", BasePrint::P_STDOUT, filename.c_str());

    _print.Printf("\nRESULTS\n", BasePrint::P_STDOUT);
    outfile << "RESULTS" << std::endl;
    if (_Conditional==1) _print.Printf( "Conditional Analysis,", BasePrint::P_STDOUT);
    else _print.Printf( "Unconditional Analysis,", BasePrint::P_STDOUT);
    if (_Conditional==1) outfile    << "Conditional Analysis,";
    else outfile    << "Unconditional Analysis,";

    _print.Printf(" Total Cases:%d", BasePrint::P_STDOUT, _TotalC);
    outfile << " Total Cases:" << _TotalC;
    _print.Printf("  Total Measure:%lf", BasePrint::P_STDOUT, _TotalN);
    outfile << " Total Measure:" << _TotalN << std::endl;
    outfile << std::endl;
    outfile << "Cut# NodeID #Obs ";

    if (_Duplicates) outfile << "#CasesWithoutDuplicates ";
    outfile << "#Exp O/E ";
    if (_Duplicates) outfile << "O/EWithoutDuplicates ";
    outfile << "LLR pvalue" << std::endl;
    if (_Cut.at(0)->_C == 0) _print.Printf("No clusters were found.\n", BasePrint::P_STDOUT);
    if (_Cut.at(0)->_C == 0) outfile << "No clusters were found." << std::endl;

    int k=0;
    outfile.setf(std::ios::fixed);
    outfile.precision(5);
    while( k < _nCuts && _Cut.at(k)->_C > 0 && _Rank.at(k) < _nMCReplicas+1) {
        _print.Printf("\nMost Likely Cut #%d:\n", BasePrint::P_STDOUT, k+1);
        outfile << k+1;
        _print.Printf("Node ID =%s\n", BasePrint::P_STDOUT, _Nodes.at(_Cut.at(k)->_ID)->_identifier.c_str());
        outfile << " " << _Nodes.at(_Cut.at(k)->_ID)->_identifier.c_str();
        _print.Printf("Number of Cases =%d\n", BasePrint::P_STDOUT, _Cut.at(k)->_C);
        outfile << " " << _Cut.at(k)->_C;
        if (_Duplicates) {
            _print.Printf("Number of Cases (duplicates removed) =%ld\n", BasePrint::P_STDOUT, _Cut.at(k)->_C - _Nodes.at(_Cut.at(k)->_ID)->_Duplicates);
            outfile << " " << _Cut.at(k)->_C - _Nodes.at(_Cut.at(k)->_ID)->_Duplicates;
        }
        _print.Printf("Expected =%lf\n", BasePrint::P_STDOUT, _Cut.at(k)->_N);
        outfile << " " << _Cut.at(k)->_N;
        _print.Printf("O/E =%lf\n", BasePrint::P_STDOUT, _Cut.at(k)->_C/_Cut.at(k)->_N);
        outfile << " " << _Cut.at(k)->_C/_Cut.at(k)->_N;
        if (_Duplicates) {
            _print.Printf("O/E (duplicates removed) =%lf\n", BasePrint::P_STDOUT, (_Cut.at(k)->_C - _Nodes.at(_Cut.at(k)->_ID)->_Duplicates)/_Cut.at(k)->_N);
            outfile << " " << (_Cut.at(k)->_C - _Nodes.at(_Cut.at(k)->_ID)->_Duplicates)/_Cut.at(k)->_N << std::endl;
        }
        if (_Conditional) _print.Printf("Log Likelihood Ratio =%lf\n", BasePrint::P_STDOUT, _Cut.at(k)->_LogLikelihood - _TotalC * log(_TotalC/_TotalN));
        else _print.Printf("Log Likelihood Ratio =%lf\n", BasePrint::P_STDOUT, _Cut.at(k)->_LogLikelihood );
        if (_Conditional) outfile << " " << _Cut.at(k)->_LogLikelihood - _TotalC * log(_TotalC/_TotalN);
        else outfile << " " << _Cut.at(k)->_LogLikelihood;

        _print.Printf("P-value =%lf\n", BasePrint::P_STDOUT, (float)_Rank.at(k) /(_nMCReplicas+1));
        outfile << " " << (float)_Rank.at(k) /(_nMCReplicas+1) << std::endl;
        k++;
    }

    outfile << std::endl << std::endl;
    outfile << "Information About Each Node" << std::endl;
    outfile << "ID Obs Exp O/E" << std::endl;
    // outfile.width(10);
    for (size_t i=0; i < _Nodes.size(); i++)
        if (_Nodes.at(i)->_BrN > 0)
            outfile << "0 " << _Nodes.at(i)->_identifier.c_str() << " " << _Nodes.at(i)->_BrC << " " << _Nodes.at(i)->_BrN << " " << _Nodes.at(i)->_BrC/_Nodes.at(i)->_BrN << " 0 0 " << std::endl;

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
    if (!reportResults(outputfile)) return false;

    time(&gEndTime); //get start time
    _print.Printf("\nProgram run on: %sProgram finished on: %s\n", BasePrint::P_STDOUT, ctime(&gStartTime), ctime(&gEndTime));

    return true;
}

bool ScanRunner::runsimulations_development() {
    char                * sReplicationFormatString = "The result of Monte Carlo replica #%u of %u replications is: %lf (%lf)\n";
    unsigned long         ulParallelProcessCount = 1;//std::min(GetNumSystemProcessors(), (unsigned int)_nMCReplicas);

    try {
        if (_nMCReplicas == 0)
            return true;
        
        _print.Printf("Doing the %d Monte Carlo simulations (new):\n", BasePrint::P_STDOUT, _nMCReplicas);
        _Rank.resize(_nCuts, 1);

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
                    MCSimSuccessiveFunctor mcsf(thread_mutex, boost::shared_ptr<AbstractRandomizer>(new PoissonRandomizer(_Conditional, _TotalC, _TotalN)), *this);
                    tg.create_thread(subcontractor<contractor_type,MCSimSuccessiveFunctor>(theContractor,mcsf));
                } catch (std::bad_alloc &b) {             
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

/* DOING THE MONTE CARLO SIMULATIONS */
bool ScanRunner::runsimulations() {
    return runsimulations_development();


    _print.Printf("Doing the %d Monte Carlo simulation (old)s:\n", BasePrint::P_STDOUT, _nMCReplicas);

    _Rank.resize(_nCuts, 1);
    for (int replica=0; replica < _nMCReplicas; replica++) {
        //-------------------- GENERATING THE RANDOM DATA ------------------------------
        int cases, CasesLeft, TotalSimC;
        double  ExpectedLeft;

        if (_Conditional) {
            TotalSimC = _TotalC;
            CasesLeft = _TotalC;
            ExpectedLeft = _TotalN;
            for (size_t i=0; i < _Nodes.size(); i++) {
                cases = BinomialGenerator(CasesLeft, _Nodes.at(i)->_IntN / ExpectedLeft);
                //if(cases>0 && Node[i].IntN<0.1) cout << "node=" << i <<  ", CasesLeft=" << CasesLeft << ", c=" << cases << ", exp=" << Node[i].IntN << ", ExpLeft=" << ExpectedLeft << endl;
                _Nodes.at(i)->_SimIntC = cases;
                CasesLeft -= cases;
                ExpectedLeft -= _Nodes.at(i)->_IntN;
                _Nodes.at(i)->_SimBrC = 0; // Initilazing the branch cases with zero
            } // for i
        }  else { // if unconditional
            TotalSimC=0;
            ExpectedLeft = _TotalN;
            for(size_t i=0; i < _Nodes.size(); i++) {
                cases = PoissonGenerator(_Nodes.at(i)->_IntN);
                //if(cases>0 && Node[i].IntN<0.1) cout << "node=" << i <<  ",  c=" << cases << ", exp=" << Node[i].IntN << endl;
                _Nodes.at(i)->_SimIntC=cases;
                TotalSimC += cases;
                ExpectedLeft -= _Nodes.at(i)->_IntN;
                _Nodes.at(i)->_SimBrC = 0; // Initilazing the branch cases with zero
            }
        }

        //------------------------ UPDATING THE TREE -----------------------------------
        for (size_t i=0; i < _Nodes.size(); i++) {
            if (_Nodes.at(i)->_Anforlust==false) addSimC(i,_Nodes.at(i)->_SimIntC);
            else addSimCAnforlust(i,_Nodes.at(i)->_SimIntC);
        }

        //--------------------- SCANNING THE TREE, SIMULATIONS -------------------------
        double SimLogLikelihood=0;
        ScanRunner::Loglikelihood_t calcLogLikelihood = ScanRunner::getLoglikelihood();

        for (size_t i=0; i < _Nodes.size(); i++) {
            if (_Nodes.at(i)->_SimBrC > 1)
                SimLogLikelihood = std::max(SimLogLikelihood, calcLogLikelihood->LogLikelihood(_Nodes.at(i)->_SimBrC, _Nodes.at(i)->_BrN));
        } // for i<nNodes

        double result=0;
        if (_Conditional) result = SimLogLikelihood - TotalSimC * log(TotalSimC/_TotalN);
        else result = SimLogLikelihood;
        _print.Printf("The result of Monte Carlo replica #%d is: %lf (%lf)\n", BasePrint::P_STDOUT, replica+1, result, SimLogLikelihood);
        for (int k=0; k < _nCuts;k++)
            if (SimLogLikelihood > _Cut.at(k)->_LogLikelihood ) _Rank.at(k)++;
    } // for i<nMCReplicas

    return true;
}

/* SCANNING THE TREE */
bool ScanRunner::scanTree() {
    _print.Printf("Scanning the tree.\n", BasePrint::P_STDOUT);

    double loglikelihood=0;
    double LogLikelihoodRatio=0;
    ScanRunner::Loglikelihood_t calcLogLikelihood = ScanRunner::getLoglikelihood();

    for(int k=0; k < _nCuts; k++) _Cut.push_back(new CutStructure());

    for (size_t i=0; i < _Nodes.size(); i++) {
        if (_Nodes.at(i)->_BrC > 1) {
            if (_Duplicates)
                loglikelihood = calcLogLikelihood->LogLikelihood(_Nodes.at(i)->_BrC - _Nodes.at(i)->_Duplicates, _Nodes.at(i)->_BrN);
            else {
                loglikelihood = calcLogLikelihood->LogLikelihood(_Nodes.at(i)->_BrC, _Nodes.at(i)->_BrN);
            }

            int k = 0;
            while(loglikelihood < _Cut.at(k)->_LogLikelihood && k < _nCuts) k++;
            if (k < _nCuts) {
                for (int m = _nCuts-1; m > k ; m--) {
                    _Cut.at(m)->_LogLikelihood = _Cut.at(m-1)->_LogLikelihood;
                    _Cut.at(m)->_ID = _Cut.at(m-1)->_ID;
                    _Cut.at(m)->_C = _Cut.at(m-1)->_C;
                    _Cut.at(m)->_N = _Cut.at(m-1)->_N;
                }
                _Cut.at(k)->_LogLikelihood = loglikelihood;
                _Cut.at(k)->_ID = i;
                _Cut.at(k)->_C = _Nodes.at(i)->_BrC;
                _Cut.at(k)->_N = _Nodes.at(i)->_BrN;
            }
        }
    }
    if (_Conditional) LogLikelihoodRatio = _Cut.at(0)->_LogLikelihood- _TotalC * log(_TotalC/_TotalN);
    else LogLikelihoodRatio = _Cut.at(0)->_LogLikelihood;
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
        (*itr)->_BrC = 0;
        (*itr)->_BrN = 0;
        (*itr)->_nChildren = 0;
    }

    // Calculates the total number of cases and the total population at risk
    for(NodeStructureContainer_t::iterator itr=_Nodes.begin(); itr != _Nodes.end(); ++itr) {
        _TotalC += (*itr)->_IntC;
        _TotalN += (*itr)->_IntN;
    }

    // Calculates the expected counts for each node and the total.
    if (_Conditional) {
        adjustN = _TotalC/_TotalN;
    for (NodeStructureContainer_t::iterator itr=_Nodes.begin(); itr != _Nodes.end(); ++itr)
        (*itr)->_IntN *= adjustN;
        _TotalN = _TotalC;
    }

    // For each node, calculates the observed and expected number of cases for that
    // node together with all of its children, grandchildren, etc.
    // Checks whether anforlust is true or false for each node.
    // Also checks whether a node is an ancestor to itslef, which is not allowed.
    _Ancestor.resize(_Nodes.size(), 0);
    for (size_t i=0; i < _Nodes.size(); ++i) {
        for (size_t j=0; j < _Nodes.size(); ++j) _Ancestor[j]=0;
        addCN(i, _Nodes.at(i)->_IntC, _Nodes.at(i)->_IntN);
        if (_Ancestor[i] > 1) {
            _print.Printf("Error: Node '%s' has itself as an ancestor.\n", BasePrint::P_ERROR, _Nodes.at(i)->_identifier.c_str());
            return false;
        } // if Ancestor[i]>1
        for (size_t j=0; j < _Nodes.size(); ++j) if(_Ancestor[j] > 1) _Nodes.at(i)->_Anforlust=true;
    } // for i<nNodes

    // For each node calculates the number of children and sets up the list of child IDs
    for (size_t i=0; i < _Nodes.size(); ++i) {
        for (size_t j=0; j < _Nodes.at(i)->_Parent.size(); ++j) {
            parent = _Nodes.at(i)->_Parent.at(j);
            _Nodes.at(parent)->_Child .push_back(i);
            _Nodes.at(parent)->_nChildren += 1;
        } // for j
    } // for i < nNodes


    // Checks that no node has negative expected cases or that a node with zero expected has observed cases.
    for (size_t i=0; i < _Nodes.size(); ++i) {
        // cout << "Node=" << i << ", BrC=" << Node[i].BrC << ", BrN=" << Node[i].BrN << endl;
        if (_Nodes.at(i)->_BrN < 0 ) {
            _print.Printf("Error: Node '%s' has negative expected cases.\n", BasePrint::P_ERROR, _Nodes.at(i)->_identifier.c_str());
            return false;
        }
        if (_Nodes.at(i)->_BrN == 0 && _Nodes.at(i)->_BrC > 0) {
            _print.Printf("Error: Node '%s' has observed cases but zero expected.\n", BasePrint::P_ERROR, _Nodes.at(i)->_identifier.c_str());
            return false;
        }
    } // for i

    return true;
}
