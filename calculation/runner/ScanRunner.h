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
#include <map>
#include <list>

class ScanRunner;
class CutStructure {
public:
    typedef std::vector<int> CutChildContainer_t;

private:
    int                     _ID;            // NodeID
    int                     _C;             // Number of cases.
    double                  _N;             // Expected number of cases.
    double                  _LogLikelihood; // Loglikelihood value.
    unsigned int            _rank;
    DataTimeRange::index_t _start_idx;      // temporal start window index
    DataTimeRange::index_t _end_idx;        // temporal end window index
    CutChildContainer_t    _cut_children;   // optional collection of children indexes

public:
    CutStructure() : _ID(0), _C(0), _N(0), _LogLikelihood(-std::numeric_limits<double>::max()), _rank(1), _start_idx(0), _end_idx(1) {}

    void                    addCutChild(int cutID, bool clear=false) {if (clear) _cut_children.clear(); _cut_children.push_back(cutID);}
    int                     getC() const {return _C; /* Observed */}
    const CutChildContainer_t & getCutChildren() {return _cut_children;}
    double                  getAttributableRisk(const ScanRunner& scanner);
    std::string           & getAttributableRiskAsString(const ScanRunner& scanner, std::string& s);
    int                     getID() const {return _ID;}
    double                  getLogLikelihood() const {return _LogLikelihood;}
    double                  getN() const {return _N;}
    double                  getExcessCases(const ScanRunner& scanner) const;
    double                  getExpected(const ScanRunner& scanner) const;
    unsigned int            getRank() const {return _rank;}
    double                  getRelativeRisk(const ScanRunner& scanner) const;
    DataTimeRange::index_t  getStartIdx() const {return _start_idx;}
    DataTimeRange::index_t  getEndIdx() const {return _end_idx;}
    unsigned int            incrementRank() {return ++_rank;}
    void                    setC(int i) {_C = i;}
    void                    setCutChildren(const CutChildContainer_t & c) {_cut_children = c;}
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
    typedef std::vector<NodeStructure*> RelationContainer_t;
    typedef std::vector<count_t> CountContainer_t;
    typedef std::vector<expected_t> ExpectedContainer_t;
    enum CumulativeStatus {NON_CUMULATIVE=0, CUMULATIVE};
    typedef std::list<unsigned int> Ancestors_t;

private:
    std::string             _identifier;
    int                     _ID;                // The node ID.
    CountContainer_t        _IntC_C;            // Number of true and simulated cases internal to the node, respectively.
    CountContainer_t        _BrC_C;             // Number of true and simulated cases in the node and all decendants (children, grandchildren etc.)
    ExpectedContainer_t     _IntN_C, _BrN_C;    // Expected number of cases internal to the node, and with all decendants respectively.
    RelationContainer_t     _Child;             // List of node IDs of the children and parents
    RelationContainer_t     _Parent;
    Parameters::CutType     _cut_type;
    CumulativeStatus        _cumulative_status;

    Ancestors_t             _ancestors;     // nodes which have this node in tree branch

    void initialize_containers(const Parameters& parameters, size_t container_size) {
        _IntC_C.resize(container_size);
        _BrC_C.resize(container_size);
        if (parameters.getModelType() == Parameters::POISSON ||
            parameters.getModelType() == Parameters::BERNOULLI ||
            (parameters.getConditionalType() == Parameters::NODEANDTIME) ||
            (parameters.getScanType() == Parameters::TIMEONLY && parameters.isPerformingDayOfWeekAdjustment()) ||
            (parameters.getConditionalType() == Parameters::NODE && parameters.isPerformingDayOfWeekAdjustment())) {
            _IntN_C.resize(container_size);
            _BrN_C.resize(container_size);
        }
    }

public:
    NodeStructure(const std::string& identifier) 
        :_identifier(identifier), _ID(0), _cumulative_status(NON_CUMULATIVE) {}
    NodeStructure(const std::string& identifier, const Parameters& parameters, size_t container_size) 
        : _identifier(identifier), _ID(0), _cut_type(parameters.getCutType()), _cumulative_status(NON_CUMULATIVE) {
            initialize_containers(parameters, container_size);
    }

    const Ancestors_t           & getAncestors() const {return _ancestors;}
    const std::string           & getIdentifier() const {return _identifier;}
    int                           getID() const {return _ID;}
    int                           getIntC() const {return _IntC_C.front();}
    const CountContainer_t      & getIntC_C() const {return _IntC_C;}
    int                           getBrC() const {return _BrC_C.front();}
    const CountContainer_t      & getBrC_C() const {return _BrC_C;}
    int                           getNChildren() const {return static_cast<int>(_Child.size());}
    double                        getIntN() const {return _IntN_C.front();}
    const ExpectedContainer_t   & getIntN_C() const {return _IntN_C;}
    double                        getBrN() const {return _BrN_C.front();}
    const ExpectedContainer_t   & getBrN_C() const {return _BrN_C;}
    const RelationContainer_t   & getChildren() const {return _Child;}
    const RelationContainer_t   & getParents() const {return _Parent;}
    Parameters::CutType           getCutType() const {return _cut_type;} 
    ExpectedContainer_t         & refIntN_C() {return _IntN_C;}
    CountContainer_t            & refIntC_C() {return _IntC_C;}
    CountContainer_t            & refBrC_C() {return _BrC_C;}
    ExpectedContainer_t         & refBrN_C() {return _BrN_C;}
    RelationContainer_t         & refChildren() {return _Child;}

    void                          setIdentifier(const std::string& s) {_identifier = s;}
    void                          setID(int i) {_ID = i;}
    void                          setCutType(Parameters::CutType cut_type) {_cut_type = cut_type;}

    void addAsParent(NodeStructure& parent) {
        // add node of collection parents
        if (_Parent.end() == std::find(_Parent.begin(), _Parent.end(), &parent))
            _Parent.push_back(&parent);
        // and add this node as child in parents collection
        if (parent.refChildren().end() == std::find(parent.refChildren().begin(), parent.refChildren().end(), this))
            parent.refChildren().push_back(this);
    }
    void setAncestors(boost::dynamic_bitset<>& ancestor_nodes) {
        /* convert ON bits in set to indexes stored in _ancestors container */
        _ancestors.clear();
        for (boost::dynamic_bitset<>::size_type p=ancestor_nodes.find_first(); p != ancestor_nodes.npos; p = ancestor_nodes.find_next(p))
            _ancestors.push_back(static_cast<unsigned int>(p));
    }
    void setCumulative() {
        if (_cumulative_status == NON_CUMULATIVE) {
            TreeScan::cumulative_backward(_IntC_C);
            TreeScan::cumulative_backward(_BrC_C);
            TreeScan::cumulative_backward(_IntN_C);
            TreeScan::cumulative_backward(_BrN_C);
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

class CompareCutsByEndIdx {
public:
    bool operator() (const CutStructure * lhs, const CutStructure * rhs) {
        return lhs->getEndIdx() < rhs->getEndIdx();
    }
};

class CompareCutsByLoglikelihood {
public:
    bool operator() (const CutStructure * lhs, const CutStructure * rhs) {
        return lhs->getLogLikelihood() > rhs->getLogLikelihood();
    }
};

struct TreeStatistics {
    typedef std::map<unsigned int, unsigned int> NodesLevel_t;

    unsigned int _num_nodes;
    unsigned int _num_root;
    unsigned int _num_leaf;
    unsigned int _num_parent;
    NodesLevel_t _nodes_per_level;

    unsigned int getNodeLevel(const NodeStructure& node, const ScanRunner& scanner) const;

    TreeStatistics() : _num_nodes(0), _num_root(0), _num_leaf(0), _num_parent(0) {}
};

class AbstractRandomizer;
class RelativeRiskAdjustmentHandler;

class ScanRunner {
public:
    typedef ptr_vector<NodeStructure>                           NodeStructureContainer_t;
    typedef ptr_vector<CutStructure>                            CutStructureContainer_t;
    typedef std::pair<bool,size_t>                              Index_t;
    typedef boost::shared_ptr<AbstractLoglikelihood>            Loglikelihood_t;
    typedef boost::shared_ptr<RelativeRiskAdjustmentHandler>    RiskAdjustments_t;
    typedef std::vector<RiskAdjustments_t>                      RiskAdjustmentsContainer_t;
    typedef boost::tuple<double, double, double>                PowerEstimationSet_t;
    typedef std::deque<PowerEstimationSet_t>                    PowerEstimationContainer_t;
    typedef std::vector<unsigned int>                           TimeIntervalContainer_t;
    typedef std::vector<TimeIntervalContainer_t>                DayOfWeekIndexes_t;
    typedef boost::shared_ptr<TreeStatistics>                   TreeStatistics_t;

private:
    BasePrint                 & _print;
    NodeStructureContainer_t    _Nodes;
    CutStructureContainer_t     _Cut;
    int                         _TotalC;
    int                         _TotalControls;
    double                      _TotalN;
    SimulationVariables         _simVars;
    Parameters                  _parameters;
    DataTimeRange::index_t      _zero_translation_additive;
    boost::dynamic_bitset<>     _caselessWindows;
    std::auto_ptr<CriticalValues> _critical_values;
    PowerEstimationContainer_t  _power_estimations;
    DayOfWeekIndexes_t          _day_of_week_indexes;
    mutable TreeStatistics_t    _tree_statistics;
    bool                        _has_multi_parent_nodes;

    unsigned int                addCN_C(const NodeStructure& sourceNode, NodeStructure& destinationNode, boost::dynamic_bitset<>& ancestor_nodes);
    size_t                      calculateCutsCount() const;

    Index_t                     getNodeIndex(const std::string& identifier) const;
    bool                        readRelativeRisksAdjustments(const std::string& filename, RiskAdjustmentsContainer_t& rrAdjustments, bool consolidate);
    bool                        readCounts(const std::string& filename);
    bool                        readCuts(const std::string& filename);
    bool                        readTree(const std::string& filename, unsigned int treeOrdinal);
    bool                        reportResults(time_t start, time_t end) const;
    bool                        runPowerEvaluations();
    bool                        runsimulations(boost::shared_ptr<AbstractRandomizer> randomizer, unsigned int num_relica, bool isPowerStep, unsigned int iteration=0);
    bool                        runsequentialsimulations(unsigned int num_relica);
    bool                        scanTree();
    bool                        scanTreeTemporalConditionNode();
    bool                        scanTreeTemporalConditionNodeTime();
    bool                        setupTree();
    CutStructure *              updateCuts(size_t node_index, int BrC, double BrN, const Loglikelihood_t& logCalculator, DataTimeRange::index_t startIdx=0, DataTimeRange::index_t endIdx=1);

public:
    ScanRunner(const Parameters& parameters, BasePrint& print);

    const CriticalValues             & getCriticalValues() const {return *_critical_values;}
    std::string                      & getCaselessWindowsAsString(std::string& s) const;
    const CutStructureContainer_t    & getCuts() const {return _Cut;}
    const DayOfWeekIndexes_t         & getDayOfWeekIndexes() const {return _day_of_week_indexes;}
    bool                               getMultiParentNodesExist() const {return _has_multi_parent_nodes;}
    const NodeStructureContainer_t   & getNodes() const {return _Nodes;}
    const Parameters                 & getParameters() const {return _parameters;}
    const PowerEstimationContainer_t & getPowerEstimations() const {return _power_estimations;}
    BasePrint                        & getPrint() {return _print;}
    SimulationVariables              & getSimulationVariables() {return _simVars;}
    int                                getTotalC() const {return _TotalC;}
    int                                getTotalControls() const {return _TotalControls;}
    double                             getTotalN() const {return _TotalN;}
    const TreeStatistics             & getTreeStatistics() const;
    DataTimeRange::index_t             getZeroTranslationAdditive() const {return _zero_translation_additive;}
    bool                               reportableCut(const CutStructure& cut) const;
    bool                               run();
    void                               updateCriticalValuesList(double llr) {if (_critical_values.get()) _critical_values->add(llr);}
};
//***************************************************************************
#endif
