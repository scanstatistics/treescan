//***************************************************************************
#ifndef __ScanRunner_H
#define __ScanRunner_H
//***************************************************************************
#include "TreeScan.h"
#include "ptr_vector.h"
#include "Loglikelihood.h"
#include <boost/shared_ptr.hpp>
#include <boost/dynamic_bitset.hpp>
#include "SimulationVariables.h"
#include "Parameters.h"
#include "CriticalValues.h"
#include <iostream>
#include <fstream>
#include <limits>
#include <deque>

class ScanRunner;
class CutStructure {
private:
    int                     _ID;            // NodeID
    int                     _C;             // Number of cases.
    double                  _N;             // Expected number of cases.
    double                  _LogLikelihood; // Loglikelihood value.
    unsigned int            _rank;
    DataTimeRange::index_t _start_idx;      // temporal start window index
    DataTimeRange::index_t _end_idx;        // temporal end window index

public:
    CutStructure() : _ID(0), _C(0), _N(0), _LogLikelihood(-std::numeric_limits<double>::max()), _rank(1), _start_idx(0), _end_idx(1) {}

    int                     getC() const {return _C;}
    int                     getID() const {return _ID;}
    double                  getLogLikelihood() const {return _LogLikelihood;}
    double                  getN() const {return _N;}
    double                  getExcessCases(const ScanRunner& scanner);
    double                  getExpected(const ScanRunner& scanner);
    double                  getODE(const ScanRunner& scanner);
    unsigned int            getRank() const {return _rank;}
    double                  getRelativeRisk(const ScanRunner& scanner);
    DataTimeRange::index_t  getStartIdx() const {return _start_idx;}
    DataTimeRange::index_t  getEndIdx() const {return _end_idx;}
    unsigned int            incrementRank() {return ++_rank;}
    void                    setC(int i) {_C = i;}
    void                    setID(int i) {_ID = i;}
    void                    setLogLikelihood(double d) {_LogLikelihood = d;}
    void                    setN(double d) {_N = d;}
    void                    setStartIdx(DataTimeRange::index_t idx) {_start_idx = idx;}
    void                    setEndIdx(DataTimeRange::index_t idx) {_end_idx = idx;}
};

class NodeStructure {
public:
    typedef int count_t;
    typedef double expected_t;
    typedef std::vector<int> RelationContainer_t;
    typedef std::vector<count_t> CountContainer_t;
    typedef std::vector<expected_t> ExpectedContainer_t;
    enum CumulativeStatus {NON_CUMULATIVE=0, CUMULATIVE};

private:
    std::string             _identifier;
    int                     _ID;                // The node ID.
    CountContainer_t        _IntC_C;            // Number of true and simulated cases internal to the node, respectively.
    CountContainer_t        _BrC_C;             // Number of true and simulated cases in the node and all decendants (children, grandchildren etc.)
    ExpectedContainer_t     _IntN_C, _BrN_C;    // Expected number of cases internal to the node, and with all decendants respectively.
    RelationContainer_t     _Child;             // List of node IDs of the children and parents
    RelationContainer_t     _Parent;
    bool                    _Anforlust;         // =1 if at least one node is an ancestor in more than one way, otherwise =0 (anforlust is Swedish for 'pedigree collapse')
    int                     _Duplicates;        // Number of duplicates that needs to be removed.
    Parameters::CutType     _cut_type;
    CumulativeStatus        _cumulative_status;

    void initialize_containers(Parameters::ModelType model_type, size_t container_size) {
        _IntC_C.resize(container_size);
        _BrC_C.resize(container_size);
        if (model_type == Parameters::POISSON || model_type == Parameters::BERNOULLI) {
            _IntN_C.resize(container_size);
            _BrN_C.resize(container_size);
        }
    }

public:
    NodeStructure(const std::string& identifier) 
        :_identifier(identifier), _ID(0), _Anforlust(false), _Duplicates(0), _cumulative_status(NON_CUMULATIVE) {}
    NodeStructure(const std::string& identifier, Parameters::CutType cut_type, Parameters::ModelType model_type, size_t container_size) 
        : _identifier(identifier), _ID(0), _Anforlust(false), _Duplicates(0), _cut_type(cut_type), _cumulative_status(NON_CUMULATIVE) {
            initialize_containers(model_type, container_size);
    }

    const std::string           & getIdentifier() const {return _identifier;}
    int                           getID() const {return _ID;}
    const CountContainer_t      & getIntC_C() const {return _IntC_C;}
    int                           getBrC() const {return _BrC_C.front();}
    const CountContainer_t      & getBrC_C() const {return _BrC_C;}
    int                           getNChildren() const {return static_cast<int>(_Child.size());}
    int                           getDuplicates() const {return _Duplicates;}
    double                        getIntN() const {return _IntN_C.front();}
    const ExpectedContainer_t   & getIntN_C() const {return _IntN_C;}
    double                        getBrN() const {return _BrN_C.front();}
    const ExpectedContainer_t   & getBrN_C() const {return _BrN_C;}
    bool                          getAnforlust() const {return _Anforlust;}
    const RelationContainer_t   & getChildren() const {return _Child;}
    const RelationContainer_t   & getParents() const {return _Parent;}
    Parameters::CutType           getCutType() const {return _cut_type;} 
    ExpectedContainer_t         & refIntN_C() {return _IntN_C;}
    CountContainer_t            & refIntC_C() {return _IntC_C;}
    CountContainer_t            & refBrC_C() {return _BrC_C;}
    ExpectedContainer_t         & refBrN_C() {return _BrN_C;}
    int                         & refDuplicates() {return _Duplicates;}
    RelationContainer_t         & refChildren() {return _Child;}
    RelationContainer_t         & refParents() {return _Parent;}

    void                          setIdentifier(const std::string& s) {_identifier = s;}
    void                          setID(int i) {_ID = i;}
    void                          setDuplicates(int i) {_Duplicates = i;}
    void                          setAnforlust(bool b) {_Anforlust = b;}
    void                          setCutType(Parameters::CutType cut_type) {_cut_type = cut_type;}

    void setCumulative() {
        if (_cumulative_status == NON_CUMULATIVE) {
            TreeScan::cumulative(_IntC_C);
            TreeScan::cumulative(_BrC_C);
            TreeScan::cumulative(_IntN_C);
            TreeScan::cumulative(_BrN_C);
        }
        _cumulative_status = CUMULATIVE;
    }
};

class CompareNodeStructureByIdentifier {
public:
    bool operator() (const NodeStructure * lhs, const NodeStructure * rhs) {
        return lhs->getIdentifier() < rhs->getIdentifier();
    }
};

class CompareNodeStructureById {
public:
    bool operator() (const NodeStructure * lhs, const NodeStructure * rhs) {
        return lhs->getID() < rhs->getID();
    }
};

class CompareCutsById {
public:
    bool operator() (const CutStructure * lhs, const CutStructure * rhs) {
        return lhs->getID() > rhs->getID();
    }
};

class CompareCutsByLoglikelihood {
public:
    bool operator() (const CutStructure * lhs, const CutStructure * rhs) {
        return lhs->getLogLikelihood() > rhs->getLogLikelihood();
    }
};

class AbstractRandomizer;
class RelativeRiskAdjustmentHandler;

class ScanRunner {
public:
    typedef ptr_vector<NodeStructure>                           NodeStructureContainer_t;
    typedef ptr_vector<CutStructure>                            CutStructureContainer_t;
    typedef std::vector<int>                                    AncestorContainer_t;
    typedef std::pair<bool,size_t>                              Index_t;
    typedef boost::shared_ptr<AbstractLoglikelihood>            Loglikelihood_t;
    typedef boost::shared_ptr<RelativeRiskAdjustmentHandler>    RiskAdjustments_t;
    typedef std::vector<RiskAdjustments_t>                      RiskAdjustmentsContainer_t;
    typedef boost::tuple<double, double, double>                PowerEstimationSet_t;
    typedef std::deque<PowerEstimationSet_t>                    PowerEstimationContainer_t;

private:
    BasePrint                 & _print;
    NodeStructureContainer_t    _Nodes;
    CutStructureContainer_t     _Cut;
    AncestorContainer_t         _Ancestor;
    int                         _TotalC;
    int                         _TotalControls;
    double                      _TotalN;
    SimulationVariables         _simVars;
    Parameters                  _parameters;
    DataTimeRange::index_t      _zero_translation_additive;
    boost::dynamic_bitset<>     _caselessWindows;
    std::auto_ptr<CriticalValues> _critical_values;
    PowerEstimationContainer_t    _power_estimations;

    void                        addCN_C(int id, NodeStructure::CountContainer_t& c, NodeStructure::ExpectedContainer_t& n);
    size_t                      calculateCutsCount() const;

    Index_t                     getNodeIndex(const std::string& identifier) const;
    bool                        readRelativeRisksAdjustments(const std::string& filename, RiskAdjustmentsContainer_t& rrAdjustments, bool consolidate);
    bool                        readCounts(const std::string& filename);
    bool                        readCuts(const std::string& filename);
    bool                        readTree(const std::string& filename);
    bool                        reportResults(time_t start, time_t end) const;
    bool                        runPowerEvaluations();
    bool                        runsimulations(boost::shared_ptr<AbstractRandomizer> randomizer, unsigned int num_relica, bool isPowerStep);
    bool                        scanTree();
    bool                        scanTreeTemporal();
    bool                        setupTree();
    void                        updateCuts(size_t node_index, int BrC, double BrN, const Loglikelihood_t& logCalculator, DataTimeRange::index_t startIdx=0, DataTimeRange::index_t endIdx=1);

public:
    ScanRunner(const Parameters& parameters, BasePrint& print);

    const CriticalValues             & getCriticalValues() const {return *_critical_values;}
    std::string                      & getCaselessWindowsAsString(std::string& s) const;
    const CutStructureContainer_t    & getCuts() const {return _Cut;}
    const NodeStructureContainer_t   & getNodes() const {return _Nodes;}
    const Parameters                 & getParameters() const {return _parameters;}
    const PowerEstimationContainer_t & getPowerEstimations() const {return _power_estimations;}
    BasePrint                        & getPrint() {return _print;}
    SimulationVariables              & getSimulationVariables() {return _simVars;}
    int                                getTotalC() const {return _TotalC;}
    double                             getTotalN() const {return _TotalN;}
    DataTimeRange::index_t             getZeroTranslationAdditive() const {return _zero_translation_additive;}
    bool                               run();
    void                               updateCriticalValuesList(double llr) {if (_critical_values.get()) _critical_values->add(llr);}
};
//***************************************************************************
#endif
