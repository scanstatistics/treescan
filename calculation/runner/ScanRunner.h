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
private:
    int     _ID;            // NodeID
    int     _C;             // Number of cases.
    double  _N;             // Expected number of cases.
    double  _LogLikelihood; // Loglikelihood value.

public:
    CutStructure() : _ID(0), _C(0), _N(0), _LogLikelihood(0)  {}

    int     getC() const {return _C;}
    int     getID() const {return _ID;}
    double  getLogLikelihood() const {return _LogLikelihood;}
    double  getN() const {return _N;}

    void    setC(int i) {_C = i;}
    void    setID(int i) {_ID = i;}
    void    setLogLikelihood(double d) {_LogLikelihood = d;}
    void    setN(double d) {_N = d;}
};

class NodeStructure {
public:
    typedef std::vector<int>  RelationContainer_t;

private:
    std::string         _identifier;
    int                 _ID;                // The node ID.
    int                 _IntC, _SimIntC;    // Number of true and simulated cases internal to the node, respectively.
    int                 _BrC, _SimBrC;      // Number of true and simulated cases in the node and all decendants (children, grandchildren etc.)
    double              _IntN, _BrN;        // Expexted number of cases internal to the node, and with all decendants respectively.
    RelationContainer_t _Child;             // List of node IDs of the children and parents
    RelationContainer_t _Parent;
    bool                _Anforlust;         // =1 if at least one node is an ancestor in more than one way, otherwise =0
    int                 _Duplicates;        // Number of duplicates that needs to be removed.

public:
    NodeStructure(const std::string& identifier) : _identifier(identifier), _ID(0), _IntC(0), _SimIntC(0), _BrC(0), _SimBrC(0), _IntN(0), _BrN(0), _Anforlust(false), _Duplicates(0) {}

    const std::string           & getIdentifier() const {return _identifier;}
    int                           getID() const {return _ID;}
    int                           getIntC() const {return _IntC;}
    int                           getSimIntC() const {return _SimIntC;}
    int                           getBrC() const {return _BrC;}
    int                           getSimBrC() const {return _SimBrC;}
    int                           getNChildren() const {return static_cast<int>(_Child.size());}
    int                           getDuplicates() const {return _Duplicates;}
    double                        getIntN() const {return _IntN;}
    double                        getBrN() const {return _BrN;}
    bool                          getAnforlust() const {return _Anforlust;}
    const RelationContainer_t   & getChild() const {return _Child;}
    const RelationContainer_t   & getParent() const {return _Parent;}

    double                      & refIntN() {return _IntN;}
    int                         & refIntC() {return _IntC;}
    int                         & refBrC() {return _BrC;}
    double                      & refBrN() {return _BrN;}
    int                         & refSimBrC() {return _SimBrC;}
    int                         & refDuplicates() {return _Duplicates;}
    RelationContainer_t         & refChild() {return _Child;}
    RelationContainer_t         & refParent() {return _Parent;}

    void                          setIdentifier(const std::string& s) {_identifier = s;}
    void                          setID(int i) {_ID = i;}
    void                          setIntC(int i) {_IntC = i;}
    void                          setSimIntC(int i) {_SimIntC = i;}
    void                          setBrC(int i) {_BrC = i;}
    void                          setSimBrC(int i) {_SimBrC = i;}
    void                          setDuplicates(int i) {_Duplicates = i;}
    void                          setIntN(double d) {_IntN = d;}
    void                          setBrN(double d) {_BrN = d;}
    void                          setAnforlust(bool b) {_Anforlust = b;}
};

class CompareNodeStructureByIdentifier {
public:
    bool operator() (const NodeStructure * lhs, const NodeStructure * rhs) {
        //return atoi(lhs->_identifier.c_str()) < atoi(rhs->_identifier.c_str());
        return lhs->getIdentifier() < rhs->getIdentifier();
    }
};

class CompareNodeStructureById {
public:
    bool operator() (const NodeStructure * lhs, const NodeStructure * rhs) {
        return lhs->getID() < rhs->getID();
    }
};

class ScanRunner {
public:
    typedef ptr_vector<NodeStructure>                   NodeStructureContainer_t;
    typedef ptr_vector<CutStructure>                    CutStructureContainer_t;
    typedef std::vector<unsigned int>                   RankContainer_t;
    typedef std::vector<int>                            AncestorContainer_t;
    typedef std::pair<bool,size_t>                      Index_t;
    typedef boost::shared_ptr<AbstractLoglikelihood>    Loglikelihood_t; 

private:
    BasePrint                 & _print;
    NodeStructureContainer_t    _Nodes;
    CutStructureContainer_t     _Cut;
    RankContainer_t             _Rank;
    AncestorContainer_t         _Ancestor;
    int                         _TotalC;
    double                      _TotalN;
    SimulationVariables         _simVars;
    Parameters                  _parameters;

    void                        addCN(int id, int c, double n);
    void                        addSimC(int id, int c);
    void                        addSimCAnforlust(int id, int c);
    Index_t                     getNodeIndex(const std::string& identifier) const;
    bool                        readCounts(const std::string& filename);
    bool                        readTree(const std::string& filename);
    bool                        reportResults(const std::string& filename, bool htmlPrint, time_t start, time_t end) const;
    bool                        runsimulations();
    bool                        scanTree();
    bool                        setupTree();

public:
  ScanRunner(const Parameters& parameters, BasePrint& print) : _parameters(parameters), _print(print), _TotalC(0), _TotalN(0) {}

    const CutStructureContainer_t    & getCuts() const {return _Cut;}
    const NodeStructureContainer_t   & getNodes() const {return _Nodes;}
    Loglikelihood_t                    getLoglikelihood() const;
    const Parameters                 & getParameters() const {return _parameters;}
    BasePrint                        & getPrint() {return _print;}
    const RankContainer_t            & getRanks() const {return _Rank;}
    RankContainer_t                  & getRanks() {return _Rank;}
    SimulationVariables              & getSimulationVariables() {return _simVars;}
    int                                getTotalC() const {return _TotalC;}
    double                             getTotalN() const {return _TotalN;}
    bool                               run(const std::string& treefile, const std::string& countfile, const std::string& outputfile, bool htmlPrint);
};
//***************************************************************************
#endif
