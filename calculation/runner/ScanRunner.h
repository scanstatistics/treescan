//***************************************************************************
#ifndef __ScanRunner_H
#define __ScanRunner_H
//***************************************************************************
#include "TreeScan.h"
#include "ptr_vector.h"
#include "Loglikelihood.h"
#include <boost/shared_ptr.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/logic/tribool.hpp>
#include "SimulationVariables.h"
#include "Parameters.h"
#include "CriticalValues.h"
#include "WindowLength.h"
#include <iostream>
#include <fstream>
#include <limits>
#include <deque>
#include <map>
#include <list>
#include <numeric>
#include <iomanip>
#include <algorithm>

class ScanRunner;

double getExcessCasesFor(const ScanRunner& scanner, int nodeID, int _C, double _N, DataTimeRange::index_t _start_idx, DataTimeRange::index_t _end_idx);
double getExpectedFor(const ScanRunner& scanner, int nodeID, int _C, double _N, DataTimeRange::index_t _start_idx, DataTimeRange::index_t _end_idx);
double getAttributableRiskFor(const ScanRunner& scanner, int nodeID, int _C, double _N, DataTimeRange::index_t _start_idx, DataTimeRange::index_t _end_idx);
double getRelativeRiskFor(const ScanRunner& scanner, int nodeID, int _C, double _N, DataTimeRange::index_t _start_idx, DataTimeRange::index_t _end_idx);
std::string & AttributableRiskAsString(double ar, std::string& s);

class CutStructure {
public:
    typedef std::vector<int> CutChildContainer_t;

private:
    int                     _ID;            // NodeID
    int                     _C;             // Number of cases.
    double                  _N;             // Expected number of cases.
    double                  _LogLikelihood; // Loglikelihood value.
    unsigned int            _rank;
    unsigned int            _report_order;
    unsigned int            _branch_order;
    DataTimeRange::index_t _start_idx;      // temporal start window index
    DataTimeRange::index_t _end_idx;        // temporal end window index
    CutChildContainer_t    _cut_children;   // optional collection of children indexes

public:
    CutStructure() : _ID(0), _C(0), _N(0), _LogLikelihood(-std::numeric_limits<double>::max()), _rank(1), _start_idx(0), _end_idx(1), _report_order(0), _branch_order(0) {}

    void                    addCutChild(int cutID, bool clear=false) {if (clear) _cut_children.clear(); _cut_children.push_back(cutID);}
    int                     getC() const {return _C; /* Observed */}
    const CutChildContainer_t & getCutChildren() const {return _cut_children;}
    double                  getAttributableRisk(const ScanRunner& scanner) const;
    std::string           & getAttributableRiskAsString(const ScanRunner& scanner, std::string& s);
    int                     getID() const {return _ID;}
    double                  getLogLikelihood() const {return _LogLikelihood;}
    double                  getN() const {return _N;}
    double                  getExcessCases(const ScanRunner& scanner) const;
    double                  getExpected(const ScanRunner& scanner) const;
    unsigned int            getBranchOrder() const { return _branch_order; }
    std::string           & getParentIndentifiers(const ScanRunner& scanner, std::string& parents) const;
    unsigned int            getReportOrder() const { return _report_order; }
    unsigned int            getRank() const {return _rank;}
    double                  getRelativeRisk(const ScanRunner& scanner) const;
    DataTimeRange::index_t  getStartIdx() const {return _start_idx;}
    DataTimeRange::index_t  getEndIdx() const {return _end_idx;}
    unsigned int            incrementRank() {return ++_rank;}
    void                    setC(int i) {_C = i;}
    void                    setCutChildren(const CutChildContainer_t & c) {_cut_children = c;}
    void                    setBranchOrder(unsigned int u) { _branch_order = u; }
    void                    setReportOrder(unsigned int u) { _report_order = u; }
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
    typedef std::list<std::pair<int, count_t> > CensorDist_t;

private:
    std::string             _identifier;
    int                     _ID;                // The node ID.
    CountContainer_t        _IntC_C;            // Number of true and simulated cases internal to the node, respectively.
    CountContainer_t        _BrC_C;             // Number of true and simulated cases in the node and all decendants (children, grandchildren etc.)
    ExpectedContainer_t     _IntN_C;            // Expected number of cases internal to the node.
    ExpectedContainer_t     _IntN_C_Seq_New;    // Expected number cases in new sequential data set.
    ExpectedContainer_t     _BrN_C;             // Expected number of cases internal to the node and with all decendants.
    RelationContainer_t     _Child;             // List of node IDs of the children and parents
    RelationContainer_t     _Parent;
    Parameters::CutType     _cut_type;
    CumulativeStatus        _cumulative_status;
    unsigned int            _level;             // calculated node level

    CountContainer_t        _IntC_Censored;     // Number of censored cases internal to the ndoe.
    count_t                 _min_censored_Br;   // minimum censored on branch

    Ancestors_t             _ancestors;     // nodes which have this node in tree branch

    void initialize_containers(const Parameters& parameters, size_t container_size) {
        _IntC_C.resize(container_size);
        _BrC_C.resize(container_size);
        if (parameters.getModelType() == Parameters::POISSON ||
            parameters.getModelType() == Parameters::BERNOULLI_TREE ||
            parameters.getModelType() == Parameters::BERNOULLI_TIME ||
            (parameters.getConditionalType() == Parameters::NODEANDTIME) ||
            (parameters.getScanType() == Parameters::TIMEONLY && parameters.isPerformingDayOfWeekAdjustment()) ||
            (parameters.getConditionalType() == Parameters::NODE && parameters.isPerformingDayOfWeekAdjustment())) {
            _IntN_C.resize(container_size);
            _BrN_C.resize(container_size);
        }
        if (parameters.isSequentialScanTreeOnly()) {
            // Also initialize structures used to store existing data from previous sequential scans.
            _IntN_C_Seq_New.resize(container_size);
        }
    }

    /* Obtain the level of this node - giving consideration for multiple trees and potential for multiple parents.
       If multiple parents, use the shortest distance (https://www.squishlist.com/ims/treescan/29/). */
    static unsigned int getLevel(const NodeStructure& node) {
        /* If level already calculated, just return that level. */
        if (node.getLevel()) 
            return node.getLevel();

        /* If node doesn't have parents, then level is one. */
        if (node.getParents().empty()) return 1;

        unsigned int parent_level=std::numeric_limits<unsigned int>::max();
        for (NodeStructure::RelationContainer_t::const_iterator itr=node.getParents().begin(); itr != node.getParents().end(); ++itr) {
            parent_level = std::min(parent_level, getLevel(*(*itr)));
        }
        return parent_level + 1;
    }

public:
    NodeStructure(const std::string& identifier) 
        :_identifier(identifier), _ID(0), _cumulative_status(NON_CUMULATIVE), _level(0), _min_censored_Br(0) {}
    NodeStructure(const std::string& identifier, const Parameters& parameters, size_t container_size) 
        : _identifier(identifier), _ID(0), _cut_type(parameters.getCutType()), _cumulative_status(NON_CUMULATIVE), _level(0), _min_censored_Br(0) {
            initialize_containers(parameters, container_size);
    }

    void                          calcMinCensored() {
        _min_censored_Br = static_cast<int>(_IntC_C.size() - 1);
        for (size_t i=0; i < _IntC_Censored.size(); ++i) {
            if (_IntC_Censored[i]) {
                _min_censored_Br = static_cast<int>(i);
                break;
            }
        }
    }
    const Ancestors_t           & getAncestors() const {return _ancestors;}
    CensorDist_t                & getCensorDistribution(CensorDist_t& censor_distribution) const {
        censor_distribution.clear();
        NodeStructure::count_t nodeCensored=0, nodeCount = getIntC();
        const CountContainer_t& censored = getIntC_Censored();
        for (size_t z=0; z < censored.size(); ++z) {
            if (censored[z]) {
                censor_distribution.push_back(std::make_pair(z, censored[z]));
                nodeCensored += censored[z];
                if (nodeCensored == nodeCount) break;
            }
        }

        if (nodeCount - nodeCensored)
            censor_distribution.push_back(std::make_pair(this->_IntC_C.size() - 1, nodeCount - nodeCensored));

        return censor_distribution;
    }
    const std::string           & getIdentifier() const {return _identifier;}
    int                           getID() const {return _ID;}
    int                           getIntC() const {return _IntC_C.front();}
    const CountContainer_t      & getIntC_C() const {return _IntC_C;}
    int                           getBrC() const {return _BrC_C.front();}
    const CountContainer_t      & getBrC_C() const {return _BrC_C;}
    int                           getNChildren() const {return static_cast<int>(_Child.size());}
    unsigned int                  getLevel() const {return _level;}
    count_t                       getMinCensoredBr() const { return _min_censored_Br; }
    double                        getIntN() const {return _IntN_C.front();}
    double                        getIntN_Seq_New() const { return _IntN_C_Seq_New.front(); }
    const ExpectedContainer_t   & getIntN_C() const {return _IntN_C;}
    double                        getBrN() const {return _BrN_C.front();}
    const ExpectedContainer_t   & getBrN_C() const {return _BrN_C;}
    const RelationContainer_t   & getChildren() const {return _Child;}
    const RelationContainer_t   & getParents() const {return _Parent;}
    std::string                 & getParentIndentifiers(std::string& parents) const {
                                    std::stringstream buffer;
                                    for (auto itr = getParents().begin(); itr != getParents().end(); ++itr)
                                        buffer << (itr != getParents().begin() ? "," : "") << (*itr)->getIdentifier();
                                    parents = buffer.str();
                                    return parents;
                                  }
    Parameters::CutType           getCutType() const {return _cut_type;} 
    ExpectedContainer_t         & refIntN_C() {
                                    if (_IntN_C.size() == 0) {
                                        _IntN_C.resize(_IntC_C.size(), 0);
                                    }
                                    return _IntN_C;
                                 }
    ExpectedContainer_t         & refIntN_C_Seq_New() {
                                    if (_IntN_C_Seq_New.size() == 0) {
                                        _IntN_C_Seq_New.resize(_IntC_C.size(), 0);
                                    }
                                    return _IntN_C_Seq_New;
                                 }
    CountContainer_t            & refIntC_C() {return _IntC_C;}
    CountContainer_t            & refBrC_C() {return _BrC_C;}
    ExpectedContainer_t         & refBrN_C() {
                                    if (_BrN_C.size() == 0) {
                                        _BrN_C.resize(_IntC_C.size(), 0);
                                    }
                                    return _BrN_C;
                                  }
    RelationContainer_t         & refChildren() {return _Child;}

    const CountContainer_t      & getIntC_Censored() const { return _IntC_Censored; }
    CountContainer_t            & refIntC_Censored() {
                                    if (_IntC_Censored.size() == 0) {
                                        _IntC_Censored.resize(_IntC_C.size(), 0);
                                    }
                                    return _IntC_Censored; }
    void                          setIdentifier(const std::string& s) {_identifier = s;}
    void                          setID(int i) {_ID = i;}
    void                          setCutType(Parameters::CutType cut_type) {_cut_type = cut_type;}
    void                          setMinCensoredBr(count_t c) { _min_censored_Br = c; }

    void addAsParent(NodeStructure& parent) {
        if (getID() == parent.getID()) return; // skip adding self as parent
        // add node of collection parents
        if (_Parent.end() == std::find(_Parent.begin(), _Parent.end(), &parent))
            _Parent.push_back(&parent);
        // and add this node as child in parents collection
        if (parent.refChildren().end() == std::find(parent.refChildren().begin(), parent.refChildren().end(), this))
            parent.refChildren().push_back(this);
    }
    unsigned int assignLevel() {
        /* Warning - this method could cause infinite loop if check for circular dependency is not first performed. */
        _level = getLevel(*this);
        return _level;
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
    void getAncestoryString(std::stringstream& s, int padding) const {
        if (_Parent.size() == 0) {
            s << std::setfill('0') << std::setw(padding) << getLevel() << "-" << getIdentifier();
            return;
        }
        _Parent.front()->getAncestoryString(s, padding);
        s << "," << std::setfill('0') << std::setw(padding) << getLevel() << "-" << getIdentifier();
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

class CompareCutsByReportOrder {
public:
    bool operator() (const CutStructure * lhs, const CutStructure * rhs) {
        return lhs->getReportOrder() < rhs->getReportOrder();
    }
};

struct TreeStatistics {
    typedef std::map<unsigned int, unsigned int> NodesLevel_t;

    unsigned int _num_nodes;
    unsigned int _num_root;
    unsigned int _num_leaf;
    unsigned int _num_parent;
    NodesLevel_t _nodes_per_level;

    TreeStatistics() : _num_nodes(0), _num_root(0), _num_leaf(0), _num_parent(0) {}
};

class AbstractRandomizer;
class RelativeRiskAdjustmentHandler;

typedef std::pair<double, unsigned int> llr_sim_t;

class compare_llr_sim_t {
public:
    bool operator() (const llr_sim_t &lhs, const llr_sim_t &rhs) {
        return lhs.first > rhs.first;
    }
};

class SequentialStatistic {
    public:
        typedef std::vector<double>                     alpha_spending_container_t;
        typedef std::deque<llr_sim_t>                   llr_sim_container_t;
        typedef std::map<unsigned int, unsigned int>    signalled_cuts_container_t;

        static const char         * _file_suffix;
        static const char         * _accumulated_case_ext;
        static const char         * _accumulated_control_ext;
        static const char         * _accumulated_sim_ext;
        static const char         * _settings_ext;
		static const char         * _period;
		static const char         * _period_replace;

    protected:
        const Parameters          & _parameters;
        const ScanRunner          & _scanner;
        Parameters                  _statistic_parameters;
        unsigned int                _look_idx;
        double                      _alpha_spending;
        alpha_spending_container_t  _alpha_spendings;
        std::string                 _counts_filename;
        std::string                 _controls_filename;
        std::string                 _simulations_filename;
        std::string                 _write_simulations_filename;
        std::string                 _write_llr_filename;
        std::string                 _settings_filename;
        boost::dynamic_bitset<>     _alpha_simulations;
        signalled_cuts_container_t  _cuts_signaled;
        llr_sim_container_t         _llr_sims;
        std::string                 _tree_hash;

		std::string               & keyEscapeXML(std::string& key, const std::string& to, const std::string& from) const;
        std::string               & getTreeHash(std::string& treehash) const;
        void                        readSettings(const std::string &filename);
        void                        writeSettings(const std::string &filename);

    public:
        SequentialStatistic(const Parameters& parameters, const ScanRunner & scanner);

        static double               getAlphaSpentToDate(const std::string &filename);

        bool                        addSimulationLLR(double llr, unsigned int simIdx);
        double                      getAlphaSpending() const { return _alpha_spending; }
        unsigned int                getLook() const { return _look_idx; }
        bool                        isFirstLook() const { return _look_idx == 1; }
        bool                        isMarkedSimulation(unsigned int simIdx) const { return _alpha_simulations.test(simIdx - 1); }
        const std::string         & getCountDataFilename() const { return _counts_filename; }
        const std::string         & getControlDataFilename() const { return _controls_filename; }
        const std::string         & getSimulationDataFilename() const { return _simulations_filename; }
        const std::string         & getWriteSimulationDataFilename() const { return _write_simulations_filename; }
        void                        setCutSignaled(size_t cutIdx) {
            if (_cuts_signaled.find(static_cast<unsigned int>(cutIdx)) == _cuts_signaled.end())
                _cuts_signaled[static_cast<unsigned int>(cutIdx)] = _look_idx;
        }
        unsigned int                testCutSignaled(size_t cutIdx) const {
            signalled_cuts_container_t::const_iterator itr = _cuts_signaled.find(static_cast<unsigned int>(cutIdx));
            return itr != _cuts_signaled.end() ? itr->second : 0;
        }
        bool                        testRankSignalling(unsigned int rank) const {
            size_t max_rank_to_signal = static_cast<unsigned int>(ceil(static_cast<double>(_parameters.getNumReplicationsRequested() + 1) * _alpha_spending));
            return rank <= max_rank_to_signal;
        }
        bool                        testSignallingLLR(double llr) const {
            return llr >= _llr_sims.back().first;
            //llr_sim_t add_llr_sim(llr, 0/*not applicable*/);
            //llr_sim_container_t::const_iterator itr=std::upper_bound(_llr_sims.begin(), _llr_sims.end(), add_llr_sim, compare_llr_sim_t());
            //return itr != _llr_sims.end();
        }
        void                        write(const std::string &casefilename, const std::string &controlfilename);
};

class DataSource;

class ScanRunner {
public:
    typedef ptr_vector<NodeStructure>                           NodeStructureContainer_t;
    typedef ptr_vector<CutStructure>                            CutStructureContainer_t;
    typedef std::pair<bool,size_t>                              Index_t;
    typedef boost::shared_ptr<RelativeRiskAdjustmentHandler>    RiskAdjustments_t;
    typedef std::vector<RiskAdjustments_t>                      RiskAdjustmentsContainer_t;
    typedef boost::tuple<double, double, double>                PowerEstimationSet_t;
    typedef std::deque<PowerEstimationSet_t>                    PowerEstimationContainer_t;
    typedef std::vector<unsigned int>                           TimeIntervalContainer_t;
    typedef std::vector<TimeIntervalContainer_t>                DayOfWeekIndexes_t;
    typedef boost::shared_ptr<TreeStatistics>                   TreeStatistics_t;
    typedef boost::shared_ptr<SequentialStatistic>              SequentialStatistic_t;

protected:
    BasePrint                         & _print;
    NodeStructureContainer_t            _Nodes;
    NodeStructure::RelationContainer_t  _rootNodes;
    CutStructureContainer_t             _Cut;
    CutStructureContainer_t             _trimmed_cuts;
    int                                 _TotalC;
    int                                 _TotalControls;
    double                              _TotalN;
    SimulationVariables                 _simVars;
    Parameters                          _parameters;
    DataTimeRange::index_t              _zero_translation_additive;
    boost::dynamic_bitset<>             _caselessWindows;
    std::auto_ptr<CriticalValues>       _critical_values;
    PowerEstimationContainer_t          _power_estimations;
    DayOfWeekIndexes_t                  _day_of_week_indexes;
    mutable TreeStatistics_t            _tree_statistics;
    bool                                _has_multi_parent_nodes;
    bool                                _censored_data;
    NodeStructure::count_t              _num_censored_cases;
    DataTimeRange::index_t              _avg_censor_time;
    NodeStructure::count_t              _num_cases_excluded;
    mutable SequentialStatistic_t       _sequential_statistic;
    // cache for storing total cases in time window
    mutable std::map<std::pair<DataTimeRange::index_t, DataTimeRange::index_t>, double> _node_n_time_total_cases_cache;

    unsigned int                addCN_C(const NodeStructure& sourceNode, NodeStructure& destinationNode, boost::dynamic_bitset<>& ancestor_nodes);
    size_t                      calculateCutsCount() const;

    bool                        readRelativeRisksAdjustments(const std::string& srcfilename, RiskAdjustmentsContainer_t& rrAdjustments, bool consolidate);
    bool                        readCounts(const std::string& srcfilename, bool sequence_new_data);
    bool                        readControls(const std::string& srcfilename, bool sequence_new_data);
    bool                        readCuts(const std::string& filename);
    bool                        readDateColumn(DataSource& source, size_t columnIdx, int& dateIdx, const std::string& file_name, const std::string& column_name) const;
    bool                        readTree(const std::string& filename, unsigned int treeOrdinal);
    bool                        reportResults(time_t start, time_t end);
    bool                        runPowerEvaluations();
    bool                        runsimulations(boost::shared_ptr<AbstractRandomizer> randomizer, unsigned int num_relica, bool isPowerStep, unsigned int iteration=0);
    bool                        runsequentialsimulations(unsigned int num_relica);
    bool                        scanTree();
    bool                        scanTreeTemporalConditionNode();
    bool                        scanTreeTemporalConditionNodeCensored();
    bool                        scanTreeTemporalConditionNodeTime();
    bool                        setupTree();
    CutStructure *              calculateCut(size_t node_index, int BrC, double BrN, const Loglikelihood_t& logCalculator, DataTimeRange::index_t startIdx=0, DataTimeRange::index_t endIdx=1, int BrC_All=0, double BrN_All=0.0);
    CutStructure *              calculateCut(size_t node_index, int C, double N, int BrC, double BrN, const Loglikelihood_t& logCalculator, DataTimeRange::index_t startIdxa, DataTimeRange::index_t endIdx);
    CutStructure *              updateCut(std::auto_ptr<CutStructure>& cut);

public:
    ScanRunner(const Parameters& parameters, BasePrint& print);

    const NodeStructure::RelationContainer_t getCutChildNodes(const CutStructure& cut) const {
        // Returns the direct child nodes for cut. If this cut wasn't simple, then this might be a subset of the children.
        if (cut.getCutChildren().size()) {
            NodeStructure::RelationContainer_t children;
            for (auto itr=cut.getCutChildren().begin(); itr != cut.getCutChildren().end(); ++itr)
                children.push_back(_Nodes[*itr]);
            return children;
        }
        return _Nodes[cut.getID()]->getChildren();
    }
    Index_t                            getNodeIndex(const std::string& identifier) const;
    bool                               isCensoredData() const { return _censored_data; }
    DataTimeRange::index_t             getAvgCensorTime() const { return _avg_censor_time; }
    NodeStructure::count_t             getNumCensoredCases() const { return _num_censored_cases; }
    NodeStructure::count_t             getNumExcludedCases() const { return _num_cases_excluded; }
    const CriticalValues             & getCriticalValues() const {return *_critical_values;}
    std::string                      & getCaselessWindowsAsString(std::string& s) const;
    const CutStructureContainer_t    & getCuts() const {return _Cut;}
    const CutStructureContainer_t    & getTrimmedCuts() const { return _trimmed_cuts; }
    const DayOfWeekIndexes_t         & getDayOfWeekIndexes() const {return _day_of_week_indexes;}
    bool                               getMultiParentNodesExist() const {return _has_multi_parent_nodes;}
    const NodeStructureContainer_t   & getNodes() const {return _Nodes;}
    const NodeStructure::RelationContainer_t & getRootNodes() const { return _rootNodes; }
    const Parameters                 & getParameters() const {return _parameters;}
    const PowerEstimationContainer_t & getPowerEstimations() const {return _power_estimations;}
    BasePrint                        & getPrint() {return _print;}
    SequentialStatistic              & refSequentialStatistic() { return *_sequential_statistic; }
    const SequentialStatistic        & getSequentialStatistic() const { return *_sequential_statistic; }
    SimulationVariables              & getSimulationVariables() {return _simVars;}
    int                                getTotalC() const {return _TotalC;}
    int                                getTotalControls() const {return _TotalControls;}
    double                             getTotalN() const {return _TotalN;}
    const TreeStatistics             & getTreeStatistics() const;
    DataTimeRange::index_t             getZeroTranslationAdditive() const {return _zero_translation_additive;}
    bool                               isEvaluated(const NodeStructure& node) const;
    bool                               reportableCut(const CutStructure& cut) const;
    bool                               reportablePValue(const CutStructure& cut) const;
    bool                               reportableRecurrenceInterval(const CutStructure& cut) const;
    RecurrenceInterval_t               getRecurrenceInterval(const CutStructure& cut) const;
    boost::logic::tribool              isSignificant(const CutStructure& cut) const;
    static bool                        isSignificant(const RecurrenceInterval_t& ri, const Parameters& parameters);
    bool                               run();
    void                               updateCriticalValuesList(double llr) {if (_critical_values.get()) _critical_values->add(llr);}
    DataTimeRange                      temporalStartRange() const {
        // If restricting temporal windows, returns user specified temporal time range, otherwise the overall data time range.
        return _parameters.getRestrictTemporalWindows() ? _parameters.getTemporalStartRange() : _parameters.getDataTimeRangeSet().getDataTimeRangeSets().front();
    }
    DataTimeRange                       temporalEndRange() const {
        // If restricting temporal windows, returns user specified temporal time range, otherwise the overall data time range.
        if (_parameters.getIsProspectiveAnalysis())
            return DataTimeRange(
                _parameters.getDataTimeRangeSet().getDataTimeRangeSets().front().getEnd(),
                _parameters.getDataTimeRangeSet().getDataTimeRangeSets().front().getEnd(),
                _parameters.getDataTimeRangeSet().getDataTimeRangeSets().front().getDateStart()
            );
        return _parameters.getRestrictTemporalWindows() ? _parameters.getTemporalEndRange() : _parameters.getDataTimeRangeSet().getDataTimeRangeSets().front();
    }

    boost::shared_ptr<AbstractWindowLength> getNewWindowLength() const;
    double get_node_n_time_total_cases(DataTimeRange::index_t start_idx, DataTimeRange::index_t end_idx) const {
        /* Obtain the total number of cases in window range for all nodes. */
        auto test = std::make_pair(start_idx, end_idx);
        auto finder = _node_n_time_total_cases_cache.find(test);
        if (finder != _node_n_time_total_cases_cache.end())
            return finder->second;
        double Ct = 0.0;
        for (auto node : getNodes())
            Ct += static_cast<double>(node->getIntC_C()[start_idx]) - static_cast<double>(node->getIntC_C()[end_idx + 1]);
        _node_n_time_total_cases_cache[test] = Ct;
        return Ct;
    }
};

class CompareCutsByAncestoryString {
    public:
    const ScanRunner::NodeStructureContainer_t & _nodes;
    size_t _padding_size;

    public:
    CompareCutsByAncestoryString(const ScanRunner& scanner) : _nodes(scanner.getNodes()) {
        std::string buffer;
        size_t t = scanner.getTreeStatistics()._nodes_per_level.size();
        type_to_string<size_t>(t, buffer);
        _padding_size = buffer.size();
    }
    bool operator() (const CutStructure * lhs, const CutStructure * rhs) {
        std::stringstream _stream_buffer_lhs, _stream_buffer_rhs;
        std::string _buffer_lhs, _buffer_rhs;

        _nodes[lhs->getID()]->getAncestoryString(_stream_buffer_lhs, _padding_size);
        _buffer_lhs = _stream_buffer_lhs.str();
        _nodes[rhs->getID()]->getAncestoryString(_stream_buffer_rhs, _padding_size);
        _buffer_rhs = _stream_buffer_rhs.str();

        return _buffer_lhs < _buffer_rhs;
    }
};

//***************************************************************************
#endif
