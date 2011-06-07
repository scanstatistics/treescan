//***************************************************************************
#ifndef __ScanRunner_H
#define __ScanRunner_H
//***************************************************************************
#include "TreeScan.h"
#include "ptr_vector.h"
#include "Loglikelihood.h"
#include "boost/shared_ptr.hpp"
#include "SimulationVariables.h"
#include "Parameters.h"
#include <iostream>
#include <fstream>

class CutStructure {
public:
    int     _ID;            // NodeID
    int     _C;             // Number of cases.
    double  _N;             // Expected number of cases.
    double  _LogLikelihood; // Loglikelihood value.

    CutStructure() : _ID(0), _C(0), _N(0), _LogLikelihood(0)  {}
};

class NodeStructure {
public:
    const std::string   _identifier;
    int                 _ID;                // The node ID.
    int                 _IntC, _SimIntC;    // Number of true and simulated cases internal to the node, respectively.
    int                 _BrC, _SimBrC;      // Number of true and simulated cases in the node and all decendants (children, grandchildren etc.)
    double              _IntN, _BrN;        // Expexted number of cases internal to the node, and with all decendants respectively.
    int                 _nChildren;         // Number of children of that node, respectively.
    std::vector<int>    _Child;             // List of node IDs of the children and parents
    std::vector<int>    _Parent;
    bool                _Anforlust;         // =1 if at least one node is an ancestor in more than one way, otherwise =0
    int                 _Duplicates;        // Number of duplicates that needs to be removed.

    NodeStructure(const std::string& identifier) : _identifier(identifier), _ID(0), _IntC(0), _SimIntC(0), _BrC(0), _SimBrC(0), _IntN(0), _BrN(0), _nChildren(0), /*_nParents(0),*/ _Anforlust(false), _Duplicates(0) {}

};

class CompareNodeStructureByIdentifier {
public:
    bool operator() (const NodeStructure * lhs, const NodeStructure * rhs) {
        //return atoi(lhs->_identifier.c_str()) < atoi(rhs->_identifier.c_str());
        return lhs->_identifier < rhs->_identifier;
    }
};

class CompareNodeStructureById {
public:
    bool operator() (const NodeStructure * lhs, const NodeStructure * rhs) {
        return lhs->_ID < rhs->_ID;
    }
};

class ScanRunner {
public:
    typedef ptr_vector<NodeStructure>                   NodeStructureContainer_t;
    typedef ptr_vector<CutStructure>                    CutStructureContainer_t;
    typedef std::pair<bool,size_t>                      Index_t;
    typedef boost::shared_ptr<AbstractLoglikelihood>    Loglikelihood_t; 

private:
    BasePrint                 & _print;

    void                        addCN(int id, int c, double n);
    void                        addSimC(int id, int c);
    void                        addSimCAnforlust(int id, int c);
    Index_t                     getNodeIndex(const std::string& identifier) const;
    bool                        readCounts(const std::string& filename);
    bool                        readTree(const std::string& filename);
    bool                        reportResults(const std::string& filename, time_t start, time_t end) const;
    bool                        runsimulations();
    bool                        scanTree();
    bool                        setupTree();

public:
    NodeStructureContainer_t    _Nodes;
    CutStructureContainer_t     _Cut;
    std::vector<unsigned int>   _Rank;
    std::vector<int>            _Ancestor;
    int                         _TotalC;
    double                      _TotalN;
    SimulationVariables         _simVars;
    Parameters                  _parameters;

  ScanRunner(const Parameters& parameters, BasePrint& print) : _parameters(parameters), _print(print), _TotalC(0), _TotalN(0) {}

    Loglikelihood_t             getLoglikelihood() const;
    bool                        run(const std::string& treefile, const std::string& countfile, const std::string& outputfile);
};
//***************************************************************************
#endif
