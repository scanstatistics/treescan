
#include <numeric>
#include <algorithm>
#include <boost/tokenizer.hpp>
#include <boost/regex.hpp>
#include <boost/assign.hpp>
#include <locale>

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
#include "WindowLength.h"
#include "AlternativeHypothesisRandomizer.h"
#include "RelativeRiskAdjustment.h"
#include "BernoulliRandomizer.h"
#include "TemporalRandomizer.h"

/* Calculates the attributable risk per person for cut. */
double CutStructure::getAttributableRisk(const ScanRunner& scanner) {
    const Parameters& parameters = scanner.getParameters();
    double C = static_cast<double>(_C);
    double totalC = static_cast<double>(scanner.getTotalC());

    switch (parameters.getScanType()) {

        case Parameters::TREEONLY: {
            switch (parameters.getConditionalType()) {
                case Parameters::UNCONDITIONAL :
                case Parameters::TOTALCASES : return getExcessCases(scanner) / static_cast<double>(parameters.getAttributableRiskExposed());
                default: throw prg_error("Cannot calculate attributable risk: tree-only, condition type (%d).", "getAttributableRisk()", parameters.getConditionalType());
            }
        }

        case Parameters::TIMEONLY: /* time-only, condtioned on total cases, is a special case of tree-time, conditioned on the node with only one node */
        case Parameters::TREETIME: {
            switch (parameters.getConditionalType()) {
                case Parameters::TOTALCASES : /* this option is really only for time-only */
                case Parameters::NODE :       /* this option is really only for tree-time */
                    return getExcessCases(scanner) / static_cast<double>(parameters.getAttributableRiskExposed());
                case Parameters::NODEANDTIME : {
                    double exp = getExpected(scanner);
                    double NodeCases = static_cast<double>(scanner.getNodes()[getID()]->getBrC());
                    // O/Eout = (NodeCases – CasesInWindow) / (NodeCases-  - Expected)
                    double o_eout = (NodeCases - C) / (NodeCases - exp);
                    // EHA = Expected * O/Eout
                    double eha = exp * o_eout;
                    // RR = CasesInWindow / EHA
                    double rr = C / eha;
                    // EC = CasesInWindow – EHA 
                    double ec = C - eha;
                    return ec / static_cast<double>(parameters.getAttributableRiskExposed());
                }
                default: throw prg_error("Cannot calculate excess cases: tree-time/time-only, condition type (%d).", "getAttributableRisk()", parameters.getConditionalType());
            }
        }

        default: throw prg_error("Unknown scan type (%d).", "getExcessCases()", parameters.getScanType());
    }
}

/** Returns the attributable risk as formatted string. */
std::string & CutStructure::getAttributableRiskAsString(const ScanRunner& scanner, std::string& s) {
    std::stringstream ss;
    double ar = getAttributableRisk(scanner);
    if (ar >= 0.001) {
        ss << getRoundAsString(ar * 1000.0, s, 1, true).c_str() << " per 1,000";
    } else {
        ss << getRoundAsString(ar * 1000000.0, s, 1, true).c_str() << " per 1,000,000";
    }
    s = ss.str().c_str();
    return s;
}

/* Calculates the excess number of cases. See user guide for formula explanation. */
double CutStructure::getExcessCases(const ScanRunner& scanner) const {
    const Parameters& parameters = scanner.getParameters();
    double C = static_cast<double>(_C);
    double totalC = static_cast<double>(scanner.getTotalC());
    switch (parameters.getScanType()) {

        case Parameters::TREEONLY: {
            switch (parameters.getConditionalType()) {
                case Parameters::UNCONDITIONAL :
                    if (parameters.getModelType() == Parameters::POISSON)
                        return C - _N;
                    if (parameters.getModelType() == Parameters::BERNOULLI)
                        if (parameters.getSelfControlDesign())
                            return C - scanner.getParameters().getProbability() * (_N - C)/(1.0 - scanner.getParameters().getProbability());
                        return C - _N * scanner.getParameters().getProbability();
                    throw prg_error("Cannot calculate excess cases: tree-only, unconditonal, model (%d).", "getExcessCases()", parameters.getModelType());
                case Parameters::TOTALCASES :
                    if (parameters.getModelType() == Parameters::POISSON) {
                        if (!(scanner.getTotalN() - _N)) throw prg_error("Program error detected: model=%d, totalN=%lf, n=%lf.", "getExcessCases()", parameters.getModelType(), scanner.getTotalN(), _N);
                        return C - _N * (totalC - C)/(scanner.getTotalN() - _N);
                    }
                    if (parameters.getModelType() == Parameters::BERNOULLI) {
                        if (!(scanner.getTotalN() - _N)) throw prg_error("Program error detected: model=%d, totalN=%lf, n=%lf.", "getExcessCases()", parameters.getModelType(), scanner.getTotalN(), _N);
                        return C - _N * (totalC - C)/(scanner.getTotalN() - _N);
                    }
                    throw prg_error("Cannot calculate excess cases: tree-only, total-cases, model (%d).", "getExcessCases()", parameters.getModelType());
                default: throw prg_error("Cannot calculate excess cases: tree-only, condition type (%d).", "getExcessCases()", parameters.getConditionalType());
            }
        }

        case Parameters::TIMEONLY: /* time-only, condtioned on total cases, is a special case of tree-time, conditioned on the node with only one node */
        case Parameters::TREETIME: {
            switch (parameters.getConditionalType()) {
                case Parameters::TOTALCASES : /* this option is really only for time-only */
                case Parameters::NODE :       /* this option is really only for tree-time */
                    if (parameters.getModelType() == Parameters::UNIFORM) {
                        if (parameters.isPerformingDayOfWeekAdjustment()) {
                            //Obs - Exp * [ (NodeCases-Obs) / (NodeCases-Exp) ]
                            double exp = getExpected(scanner);
                            double NodeCases = static_cast<double>(scanner.getNodes()[getID()]->getBrC());
                            return C - exp * ((NodeCases - C) / (NodeCases - exp));
                        }
                        double W = static_cast<double>(_end_idx - _start_idx + 1.0);
                        double T = static_cast<double>(parameters.getDataTimeRangeSet().getTotalDaysAcrossRangeSets());
                        if (!(T - W)) throw prg_error("Program error detected: model=%d, T=%lf, W=%lf.", "getExcessCases()", parameters.getModelType(), T, W);
                        return C - W * (_N - C)/(T - W);
                    }
                    throw prg_error("Cannot calculate excess cases: tree-time/time-only, total-cases/node, model (%d).", "getExcessCases()", parameters.getModelType());
                case Parameters::NODEANDTIME : {
                    /* c = cases in detected cluster
                       C = total cases in the whole tree
                       C(n)=total cases in the cluster node, summed over the whole study time period
                       C(t)=total cases in the cluster time window, summed over all the nodes
                       Let E2 = (C(n)-c)*(C(t)-c) / (C-C(n)-C(t)+c) -- this is an alternative method for calculating expected counts
                       Excess Cases = c-E2
                       */
                    double Cn =  static_cast<double>(scanner.getNodes()[getID()]->getBrC());
                    double Ct = 0.0;
                    for (size_t t=0; t < scanner.getNodes().size(); ++t)
                        Ct += static_cast<double>(scanner.getNodes()[t]->getIntC_C()[getStartIdx()]) - static_cast<double>(scanner.getNodes()[t]->getIntC_C()[getEndIdx() + 1]);
                    double denominator = totalC - Cn - Ct + C;
                    if (denominator == 0.0) // This will never happen when looking for clusters with high rates.
                        return std::numeric_limits<double>::quiet_NaN();
                    double e2 = (Cn - C) * (Ct - C) / denominator;
                    if (e2 == 0.0 && C == 0.0) // C == 0.0 will never happen when looking for clusters with high rates.
                        return std::numeric_limits<double>::quiet_NaN();
                    return C - e2;
                }
                default: throw prg_error("Cannot calculate excess cases: tree-time/time-only, condition type (%d).", "getExcessCases()", parameters.getConditionalType());
            }
        }
        default: throw prg_error("Unknown scan type (%d).", "getExcessCases()", parameters.getScanType());
    }
}

/** Returns cut's expected count. See user guide for formula explanation. */
double CutStructure::getExpected(const ScanRunner& scanner) const {
    const Parameters& parameters = scanner.getParameters();
    double C = static_cast<double>(_C);
    double totalC = static_cast<double>(scanner.getTotalC());
    switch (parameters.getScanType()) {

        case Parameters::TREEONLY: {
            switch (parameters.getConditionalType()) {
                case Parameters::UNCONDITIONAL :
                    if (parameters.getModelType() == Parameters::POISSON)
                        return _N;
                    if (parameters.getModelType() == Parameters::BERNOULLI)
                        return _N * scanner.getParameters().getProbability();
                    throw prg_error("Cannot determine expected cases: tree-only, unconditonal, model (%d).", "getExpected()", parameters.getModelType());
                case Parameters::TOTALCASES :
                    if (parameters.getModelType() == Parameters::POISSON)
                        return _N * totalC/scanner.getTotalN();
                    if (parameters.getModelType() == Parameters::BERNOULLI)
                        return _N * (scanner.getTotalC() / scanner.getTotalN());
                    throw prg_error("Cannot determine expected cases: tree-only, total-cases, model (%d).", "getExpected()", parameters.getModelType());
                default: throw prg_error("Cannot determine expected cases: tree-only, condition type (%d).", "getExpected()", parameters.getConditionalType());
            }
        }

        case Parameters::TREETIME: {
            switch (parameters.getConditionalType()) {
                case Parameters::NODE:
                    if (parameters.getModelType() == Parameters::UNIFORM) {
                        if (parameters.isPerformingDayOfWeekAdjustment()) {
                            return _N;
                        } else {
                            /*  (N*W/T)
                                N = number of cases in the node, through the whole time period (below, all 0604 cases)
                                W = number of days in the temporal cluster (below, 11-6=5)
                                T = number of days for which cases were recorded (e.g. D=56 if a 1-56 time interval was used)
                            */
                            double W = static_cast<double>(_end_idx - _start_idx + 1.0);
                            double T = static_cast<double>(parameters.getDataTimeRangeSet().getTotalDaysAcrossRangeSets());
                            return _N * W / T;
                        }
                    }
                    throw prg_error("Cannot determine expected cases: tree-time, total-cases, model (%d).", "getExpected()", parameters.getModelType());
                case Parameters::NODEANDTIME :
                    if (parameters.getModelType() == Parameters::MODEL_NOT_APPLICABLE) 
                        return _N;
                    throw prg_error("Cannot determine expected cases: tree-time, node-time, model (%d).", "getExpected()", parameters.getModelType());
                default: throw prg_error("Cannot determine expected cases: tree-time, condition type (%d).", "getExpected()", parameters.getConditionalType());
            }
        }

        case Parameters::TIMEONLY: { /* time-only, conditioned on total cases, is a special case of tree-time, conditioned on the node with only one node */
            switch (parameters.getConditionalType()) {
                case Parameters::TOTALCASES:
                    if (parameters.getModelType() == Parameters::UNIFORM) {
                        if (parameters.isPerformingDayOfWeekAdjustment()) {
                            return _N;
                        } else {
                            /*  (N*W/T)
                                N = number of cases in the node, through the whole time period (below, all 0604 cases)
                                W = number of days in the temporal cluster (below, 11-6=5)
                                T = number of days for which cases were recorded (e.g. D=56 if a 1-56 time interval was used)
                            */
                            double W = static_cast<double>(_end_idx - _start_idx + 1.0);
                            double T = static_cast<double>(parameters.getDataTimeRangeSet().getTotalDaysAcrossRangeSets());
                            return _N * W / T;
                        }
                    }
                    throw prg_error("Cannot determine expected cases: tree-time, total-cases, model (%d).", "getExpected()", parameters.getModelType());
                default: throw prg_error("Cannot determine expected cases: tree-time, condition type (%d).", "getExpected()", parameters.getConditionalType());
            }
        }
        default: throw prg_error("Unknown scan type (%d).", "getExpected()", parameters.getScanType());
    }
}

/** Returns cut's relative risk. See user guide for formula explanation. */
double CutStructure::getRelativeRisk(const ScanRunner& scanner) const {
    double relative_risk=0;
    double C = static_cast<double>(_C);
    double totalC = static_cast<double>(scanner.getTotalC());
    const Parameters& parameters = scanner.getParameters();

    switch (parameters.getScanType()) {

        case Parameters::TREEONLY: {
            switch (parameters.getConditionalType()) {
                case Parameters::UNCONDITIONAL :
                    if (parameters.getModelType() == Parameters::POISSON) {
                        relative_risk = C / _N;
                        return relative_risk ? relative_risk : std::numeric_limits<double>::infinity();
                    }
                    if (parameters.getModelType() == Parameters::BERNOULLI) {
                        if (parameters.getSelfControlDesign())
                            relative_risk = (C/parameters.getProbability()) / ((_N - C) / (1.0 - parameters.getProbability()));
                        else
                            relative_risk = C / (_N * parameters.getProbability());
                        return relative_risk ? relative_risk : std::numeric_limits<double>::infinity();
                    }
                    throw prg_error("Cannot calculate relative risk: tree-only, unconditonal, model (%d).", "getRelativeRisk()", parameters.getModelType());
                case Parameters::TOTALCASES :
                    if (parameters.getModelType() == Parameters::POISSON) {
                        double NN = scanner.getTotalN() - _N;
                        if (!NN) throw prg_error("Program error detected: model=%d, totalN=%lf, n=%lf.", "getRelativeRisk()", parameters.getModelType(), scanner.getTotalN(), _N);
                        double CC = totalC - C;
                        relative_risk = CC ? (C / _N) / (CC /  NN): 0;
                        return relative_risk ? relative_risk : std::numeric_limits<double>::infinity();
                    }
                    if (parameters.getModelType() == Parameters::BERNOULLI) {
                        double NN = scanner.getTotalN() - _N;
                        if (!NN) throw prg_error("Program error detected: model=%d, totalN=%lf, n=%lf.", "getRelativeRisk()", parameters.getModelType(), scanner.getTotalN(), _N);
                        double CC = totalC - C;
                        relative_risk = CC ? (C / _N) / (CC /  NN): 0;
                        return relative_risk ? relative_risk : std::numeric_limits<double>::infinity();
                    }
                    throw prg_error("Cannot calculate relative risk: tree-only, total-cases, model (%d).", "getRelativeRisk()", parameters.getModelType());
                default: throw prg_error("Cannot calculate relative risk: tree-only, condition type (%d).", "getRelativeRisk()", parameters.getConditionalType());
            }
        }

        case Parameters::TIMEONLY: /* time-only, conditioned on total cases, is a special case of tree-time, conditioned on the node with only one node */
        case Parameters::TREETIME: {
            switch (parameters.getConditionalType()) {
                case Parameters::TOTALCASES : /* this option is really only for time-only */
                case Parameters::NODE :       /* this option is really only for tree-time */
                    if (parameters.getModelType() == Parameters::UNIFORM) {
                        if (parameters.isPerformingDayOfWeekAdjustment()) {
                            // (Obs/Exp) / [ (NodeCases-Obs) / (NodeCases-Exp) ]
                            double exp = getExpected(scanner);
                            double NodeCases = static_cast<double>(scanner.getNodes()[getID()]->getBrC());
                            if (C == NodeCases)
                                relative_risk = std::numeric_limits<double>::infinity();
                            else
                                relative_risk = (C / exp) / ( (NodeCases - C) / (NodeCases - exp) );
                            return relative_risk ? relative_risk : std::numeric_limits<double>::infinity();
                        }
                        double W = static_cast<double>(_end_idx - _start_idx + 1.0);
                        double T = static_cast<double>(parameters.getDataTimeRangeSet().getTotalDaysAcrossRangeSets());
                        if (!(T - W)) throw prg_error("Program error detected: model=%d, T=%lf, W=%lf.", "getRelativeRisk()", parameters.getModelType(), T, W);
                        double CC = _N - static_cast<double>(_C);
                        relative_risk = CC ? (static_cast<double>(_C) / W ) / ( CC / (T - W) ) : 0.0;
                        return relative_risk ? relative_risk : std::numeric_limits<double>::infinity();
                    }
                    throw prg_error("Cannot calculate excess cases: tree-time/time-only, total-cases/node, model (%d).", "getRelativeRisk()", parameters.getModelType());
                case Parameters::NODEANDTIME : {
                    /* c = cases in detected cluster
                       C = total cases in the whole tree
                       C(n)=total cases in the cluster node, summed over the whole study time period
                       C(t)=total cases in the cluster time window, summed over all the nodes
                       Let E2 = (C(n)-c)*(C(t)-c) / (C-C(n)-C(t)+c) -- this is an alternative method for calculating expected counts
                       RR = c/E2
                       */
                    double Cn =  static_cast<double>(scanner.getNodes()[getID()]->getBrC());
                    double Ct = 0.0;
                    for (size_t t=0; t < scanner.getNodes().size(); ++t)
                        Ct += static_cast<double>(scanner.getNodes()[t]->getIntC_C()[getStartIdx()]) - static_cast<double>(scanner.getNodes()[t]->getIntC_C()[getEndIdx() + 1]);
                    double denominator = totalC - Cn - Ct + C;
                    if (denominator == 0.0) // This will never happen when looking for clusters with high rates.
                        return std::numeric_limits<double>::quiet_NaN();
                    double e2 = (Cn - C) * (Ct - C) / denominator;
                    if (e2 == 0.0) // C == 0.0 will never happen when looking for clusters with high rates.
                        return C == 0.0 ? std::numeric_limits<double>::quiet_NaN() : std::numeric_limits<double>::infinity()/*This will happen now and then.*/;
                    return C / e2;
                }
                default: throw prg_error("Cannot calculate excess cases: tree-time/time-only, condition type (%d).", "getRelativeRisk()", parameters.getConditionalType());
            }
        }

        default: throw prg_error("Unknown scan type (%d).", "getRelativeRisk()", parameters.getScanType());
    }
}

unsigned int TreeStatistics::getNodeLevel(const NodeStructure& node, const ScanRunner& scanner) const {
    unsigned int level=1;

    const NodeStructure * node_ptr = &node;
    while (!node_ptr->getParents().empty()) {
        ++level;
        // We're currently preventing nodes from having more than one parent, see https://www.squishlist.com/ims/treescan/13/.
        node_ptr = node_ptr->getParents().front();
    }
    return level;
}

/** class constructor */
ScanRunner::ScanRunner(const Parameters& parameters, BasePrint& print) : _parameters(parameters), _print(print), _TotalC(0), _TotalControls(0), _TotalN(0), _has_multi_parent_nodes(false) {
    // TODO: Eventually this will need refactoring once we implement multiple data time ranges.
    DataTimeRange min_max = Parameters::isTemporalScanType(_parameters.getScanType()) ? _parameters.getDataTimeRangeSet().getMinMax() : DataTimeRange(0,0);
    // translate to ensure zero based additive
    _zero_translation_additive = (min_max.getStart() <= 0) ? std::abs(min_max.getStart()) : min_max.getStart() * -1;

    if (parameters.isPerformingDayOfWeekAdjustment()) {
        _day_of_week_indexes.resize(7);
        size_t daysInDataTimeRange = _parameters.getDataTimeRangeSet().getTotalDaysAcrossRangeSets() + 1;
        for (size_t idx=0; idx < daysInDataTimeRange; ++idx) {
            _day_of_week_indexes[idx % 7].push_back(idx);
        }
    }
}

/*
 Adds cases and measure through the tree from each node through the tree to all its parents, and so on, to all its ancestors as well.
 If a node is a decendant to an ancestor in more than one way, the cases and measure is only added once to that ancestor.
 */
unsigned int ScanRunner::addCN_C(const NodeStructure& sourceNode, NodeStructure& destinationNode, boost::dynamic_bitset<>& ancestor_nodes) {
    unsigned int source_count=(sourceNode.getID() == destinationNode.getID() ? 1 : 0);

    /* Test corresponding bit set to see if destination node already includes data from the source node. Sets it if wasn't previously. */
    if (_has_multi_parent_nodes && ancestor_nodes.test_set(destinationNode.getID())) {
        //printf("'%s' already has data from '%s'\n", destinationNode.getIdentifier().c_str(), sourceNode.getIdentifier().c_str());
        return source_count;
    }

    // add source node's data to destination nodes branch totals
    std::transform(sourceNode.getIntC_C().begin(), sourceNode.getIntC_C().end(), destinationNode.refBrC_C().begin(), destinationNode.refBrC_C().begin(), std::plus<NodeStructure::count_t>());
    std::transform(sourceNode.getIntN_C().begin(), sourceNode.getIntN_C().end(), destinationNode.refBrN_C().begin(), destinationNode.refBrN_C().begin(), std::plus<NodeStructure::expected_t>());

    // continue walking up the tree
    NodeStructure::RelationContainer_t::const_iterator itr=destinationNode.getParents().begin(), itr_end=destinationNode.getParents().end();
    for (; itr != itr_end; ++itr)
        source_count += addCN_C(sourceNode, *(*itr), ancestor_nodes);
    return source_count;
}

/* Returns pair<bool,size_t> - first value indicates node existance, second is index into class vector _Nodes. */
ScanRunner::Index_t ScanRunner::getNodeIndex(const std::string& identifier) const {
    std::auto_ptr<NodeStructure> node(new NodeStructure(identifier));
    NodeStructureContainer_t::const_iterator itr = std::lower_bound(_Nodes.begin(), _Nodes.end(), node.get(), CompareNodeStructureByIdentifier());
    if (itr != _Nodes.end() && (*itr)->getIdentifier() == node.get()->getIdentifier()) {
        size_t tt = std::distance(_Nodes.begin(), itr);
        // sanity check
        if (tt != (*itr)->getID())
            throw prg_error("Calculated index (%u) does not match node ID (%u).", "RelativeRiskAdjustmentHandler::getAsProbabilities()", tt, (*itr)->getID());
        return std::make_pair(true, std::distance(_Nodes.begin(), itr));
    } else
        return std::make_pair(false, 0);
}

/* Returns tree statistics information. */
const TreeStatistics& ScanRunner::getTreeStatistics() const {
    if (_tree_statistics.get()) return *_tree_statistics.get();
    _tree_statistics.reset(new TreeStatistics());
    _tree_statistics->_num_nodes = static_cast<unsigned int>(_Nodes.size());
    NodeStructureContainer_t::const_iterator itr=_Nodes.begin();
    for (; itr != _Nodes.end(); ++itr) {
        const NodeStructure& node = *(*itr);
        if (node.getChildren().empty()) 
            ++_tree_statistics->_num_leaf;
        else
            ++_tree_statistics->_num_parent;
        if (node.getParents().empty()) ++_tree_statistics->_num_root;
        unsigned int level = _tree_statistics->getNodeLevel(node, *this);
        if (_tree_statistics->_nodes_per_level.find(level) == _tree_statistics->_nodes_per_level.end())
            _tree_statistics->_nodes_per_level.insert(std::make_pair(level, static_cast<unsigned int>(1)));
        else
            _tree_statistics->_nodes_per_level[level] += static_cast<unsigned int>(1);
    }
    return *_tree_statistics.get();
}

/** Read the relative risks file
    -- unlike other input files of system, records read from relative risks
       file are applied directly to the measure structure, just post calculation
       of measure and prior to temporal adjustments and making cumulative. */
bool ScanRunner::readRelativeRisksAdjustments(const std::string& filename, RiskAdjustmentsContainer_t& rrAdjustments, bool consolidate) {
    _print.Printf("Reading alternative hypothesis file ...\n", BasePrint::P_STDOUT);

    bool bValid=true, bEmpty=true;
    ScanRunner::Index_t nodeIndex;
    const long nodeIdIdx=0, uAdjustmentIndex=1, startidx=2, endidx=3;
    boost::dynamic_bitset<> nodeSet;
    std::auto_ptr<DataSource> dataSource(DataSource::getNewDataSourceObject(filename, _parameters.getInputSource(Parameters::POWER_EVALUATIONS_FILE)));
    bool testMultipleNodeRecords(_parameters.getModelType() == Parameters::BERNOULLI);

    // if unconditional/conditional Bernoulli, limit this file to a single entry for each node
    if (testMultipleNodeRecords)
        nodeSet.resize(_Nodes.size());

    // start with one collection of adjustments
    rrAdjustments.clear();
    rrAdjustments.push_back(RiskAdjustments_t(new RelativeRiskAdjustmentHandler()));
    RiskAdjustments_t adjustments = rrAdjustments.front();
    while (dataSource->readRecord()) {
        // if not consolidating adjustments, then we'll use blank record as trigger to create new adjustment collection
        if (!consolidate && dataSource->detectBlankRecordFlag()) {
            if (adjustments->get().size()) { // create new collection of adjustments
                rrAdjustments.push_back(RiskAdjustments_t(new RelativeRiskAdjustmentHandler()));
                adjustments = rrAdjustments.back();
                nodeSet.reset();
            }
            dataSource->clearBlankRecordFlag();
        }
        bEmpty=false;
        //read node identifier
        std::string nodeId = dataSource->getValueAt(nodeIdIdx);
        if (lowerString(nodeId) != "all") {
            nodeIndex = getNodeIndex(dataSource->getValueAt(nodeIdIdx));
            if (!nodeIndex.first) {
                bValid = false;
                _print.Printf("Error: Record %ld in alternative hypothesis file references unknown node (%s).\n", BasePrint::P_READERROR, dataSource->getCurrentRecordIndex(), dataSource->getValueAt(nodeIdIdx).c_str());
                continue;
            }
        }
        //read alternative hypothesis value
        double alternative_hypothesis;
        if (dataSource->getValueAt(uAdjustmentIndex).size() < 1) {
            _print.Printf("Error: Record %ld of alternative hypothesis file missing %s.\n", BasePrint::P_READERROR, dataSource->getCurrentRecordIndex(),
                          _parameters.getModelType() == Parameters::BERNOULLI ? "event probability" : "relative risk");
            bValid = false;
            continue;
        }
        if (_parameters.getModelType() == Parameters::BERNOULLI) {
            if (!string_to_numeric_type<double>(dataSource->getValueAt(uAdjustmentIndex).c_str(), alternative_hypothesis) || alternative_hypothesis < 0.0 || alternative_hypothesis > 1.0) {
                bValid = false;
                _print.Printf("Error: Record %ld in alternative hypothesis file references an invalid case probability for node '%s'.\n"
                              "       The event probability must be a numeric value between zero and one (inclusive).\n", BasePrint::P_READERROR, dataSource->getCurrentRecordIndex(), dataSource->getValueAt(nodeIdIdx).c_str());
                continue;
            } else if (_parameters.getConditionalType() == Parameters::UNCONDITIONAL && alternative_hypothesis < _parameters.getProbability()) {
                _print.Printf("%s: Record %ld in alternative hypothesis file references a case probability of %g, which is less than standard case probability of %g.\n",
                              BasePrint::P_READERROR, (_parameters.getConditionalType() == Parameters::UNCONDITIONAL ? "Warning" : "Error"),
                              dataSource->getCurrentRecordIndex(), alternative_hypothesis, _parameters.getProbability());
                bValid &= _parameters.getConditionalType() == Parameters::UNCONDITIONAL;
            }
        } else {
            if (!string_to_numeric_type<double>(dataSource->getValueAt(uAdjustmentIndex).c_str(), alternative_hypothesis) || alternative_hypothesis < 0) {
                bValid = false;
                _print.Printf("Error: Record %ld in alternative hypothesis file references an invalid relative risk for node '%s'.\n"
                              "       The relative risk must be a numeric value greater than or equal to zero.\n", BasePrint::P_READERROR, dataSource->getCurrentRecordIndex(), dataSource->getValueAt(nodeIdIdx).c_str());
                continue;
            }
        }
        DataTimeRange::index_t start=0, end=0;
        if (_parameters.getScanType() == Parameters::TIMEONLY || _parameters.getScanType() == Parameters::TREETIME) {
            if (!string_to_numeric_type<DataTimeRange::index_t>(dataSource->getValueAt(startidx).c_str(), start) || _parameters.getDataTimeRangeSet().getDataTimeRangeIndex(start).first == false) {
                bValid = false;
                _print.Printf("Error: Record %ld in alternative hypothesis file references an invalid start range index for node '%s'.\n"
                              "Value must be an integer within the defined data time range.", BasePrint::P_READERROR, dataSource->getCurrentRecordIndex(), dataSource->getValueAt(nodeIdIdx).c_str());
                continue;
            }
            if (!string_to_numeric_type<DataTimeRange::index_t>(dataSource->getValueAt(endidx).c_str(), end) || _parameters.getDataTimeRangeSet().getDataTimeRangeIndex(end).first == false) {
                bValid = false;
                _print.Printf("Error: Record %ld in alternative hypothesis file references an invalid end range index for node '%s'.\n"
                              "Value must be an integer within the defined data time range.", BasePrint::P_READERROR, dataSource->getCurrentRecordIndex(), dataSource->getValueAt(nodeIdIdx).c_str());
                continue;
            }
            // translate index to zero index
            start = start + _zero_translation_additive;
            end = end + _zero_translation_additive;
        }

        size_t iNumWords = dataSource->getNumValues();
        if (iNumWords > (_parameters.getScanType() == Parameters::TREEONLY ? 2 : 4)) {
            _print.Printf("Error: Record %i of alternative hypothesis file contains extra columns of data.\n", BasePrint::P_READERROR, dataSource->getCurrentRecordIndex());
            bValid = false;
            continue;
        }
        size_t nodeIdx = (nodeId == "all" ? 0 : nodeIndex.second);
        size_t nodeIdxStop = (nodeId == "all" ? _Nodes.size() : nodeIndex.second + 1);
        for (; nodeIdx < nodeIdxStop; ++nodeIdx) {
            if (testMultipleNodeRecords) {
                if (nodeSet.test(nodeIdx)) {
                    _print.Printf("Error: Multiple records specified for node '%s' in alternative hypothesis file. Each node is limited to one entry.\n", BasePrint::P_READERROR, getNodes()[nodeIdx]->getIdentifier().c_str());
                    bValid = false;
                    continue;
                } else {
                    nodeSet.set(nodeIdx);
                }
            }
            adjustments->add(nodeIdx, alternative_hypothesis, start, end);
        }
    }

    //if invalid at this point then read encountered problems with data format,
    //inform user of section to refer to in user guide for assistance
    if (!bValid)
      _print.Printf("Please see the 'Alternative Hypothesis File' section in the user guide for help.\n", BasePrint::P_ERROR);
    //print indication if file contained no data
    else if (bEmpty) {
      _print.Printf("Error: Alternative hypothesis file contains no data.\n", BasePrint::P_ERROR);
      bValid = false;
    } else {
        // safety check - remove any collections that have no adjustments
        for (RiskAdjustmentsContainer_t::iterator itr=rrAdjustments.begin(); itr != rrAdjustments.end(); ++itr) {
            if (itr->get()->get().size() == 0) itr = rrAdjustments.erase(itr);
        }
    }
    return bValid;
}

/* Reads count and population data from passed file. The file format is: <node identifier>, <count>, <population> */
bool ScanRunner::readCounts(const std::string& filename) {
    _print.Printf("Reading count file ...\n", BasePrint::P_STDOUT);
    bool readSuccess=true;
    std::auto_ptr<DataSource> dataSource(DataSource::getNewDataSourceObject(filename, _parameters.getInputSource(Parameters::COUNT_FILE)));
    int expectedColumns = (_parameters.getScanType() == Parameters::TIMEONLY ? 2 : 3);
    _caselessWindows.resize(Parameters::isTemporalScanType(_parameters.getScanType()) ? _parameters.getDataTimeRangeSet().getTotalDaysAcrossRangeSets() : 1);
    long identifierIdx = _parameters.getScanType() == Parameters::TIMEONLY ? -1 : 0;
    long countIdx = _parameters.getScanType() == Parameters::TIMEONLY ? 0 : 1;

    int count=0, controls=0, daysSinceIncidence=0;
    while (dataSource->readRecord()) {
        if (dataSource->getNumValues() != expectedColumns) {
            readSuccess = false;
            _print.Printf("Error: Record %ld in count file %s. Expecting %s<count>%s but found %ld value%s.\n",
                          BasePrint::P_READERROR, dataSource->getCurrentRecordIndex(),
                          (static_cast<int>(dataSource->getNumValues()) > expectedColumns) ? "has extra data" : "is missing data",
                          (_parameters.getScanType() == Parameters::TIMEONLY ? "" : "<identifier>, "),
                          (Parameters::isTemporalScanType(_parameters.getScanType()) ? ", <time>" : (_parameters.getModelType() == Parameters::POISSON ? ", <population>" : ", <controls>")),
                          dataSource->getNumValues(), (dataSource->getNumValues() == 1 ? "" : "s"));
            continue;
        }
        ScanRunner::Index_t index(true, 0);
        if (_parameters.getScanType() != Parameters::TIMEONLY) {
            index = getNodeIndex(dataSource->getValueAt(identifierIdx));
            if (!index.first) {
                readSuccess = false;
                _print.Printf("Error: Record %ld in count file references unknown node (%s).\n", BasePrint::P_READERROR, dataSource->getCurrentRecordIndex(), dataSource->getValueAt(identifierIdx).c_str());
                continue;
            }
        }
        NodeStructure * node = _Nodes[index.second];
        if  (!string_to_numeric_type<int>(dataSource->getValueAt(countIdx).c_str(), count) || count < 0) {
            readSuccess = false;
            _print.Printf("Error: Record %ld in count file references an invalid number of cases.\n"
                          "       The number of cases must be an integer value greater than or equal to zero.\n", BasePrint::P_READERROR, dataSource->getCurrentRecordIndex());
            continue;
        }
        // read model specific columns from data source
        if (_parameters.getModelType() == Parameters::POISSON) {
            node->refIntC_C().front() += count;
            // now read the population
            double population=0;
            if  (!string_to_numeric_type<double>(dataSource->getValueAt(expectedColumns - 1).c_str(), population) || population < 0) {
                readSuccess = false;
                _print.Printf("Error: Record %ld in count file references an invalid population.\n"
                              "       The population must be a numeric value greater than or equal to zero.\n", BasePrint::P_READERROR, dataSource->getCurrentRecordIndex());
                continue;
            }
            node->refIntN_C().front() += population;
        } else if (_parameters.getModelType() == Parameters::BERNOULLI) {
            node->refIntC_C().front() += count;
            int controls=0;
            if  (!string_to_numeric_type<int>(dataSource->getValueAt(expectedColumns - 1).c_str(), controls) || controls < 0) {
                readSuccess = false;
                _print.Printf("Error: Record %ld in count file references an invalid number of controls.\n"
                              "       The controls must be an integer value greater than or equal to zero.\n", BasePrint::P_READERROR, dataSource->getCurrentRecordIndex());
                continue;
            }
            node->refIntN_C().front() += count + controls;
        } else if (Parameters::isTemporalScanType(_parameters.getScanType())) {
            if  (!string_to_numeric_type<int>(dataSource->getValueAt(expectedColumns - 1).c_str(), daysSinceIncidence)) {
                readSuccess = false;
                _print.Printf("Error: Record %ld in count file references an invalid 'day since incidence' value.\n"
                              "       The 'day since incidence' variable must be an integer.\n", BasePrint::P_READERROR, dataSource->getCurrentRecordIndex());
                continue;
            }
            // check that the 'daysSinceIncidence' is within a defined data time range
            DataTimeRangeSet::rangeset_index_t rangeIdx = _parameters.getDataTimeRangeSet().getDataTimeRangeIndex(daysSinceIncidence);
            if (rangeIdx.first == false) {
                readSuccess = false;
                _print.Printf("Error: Record %ld in count file references an invalid 'day since incidence' value.\n"
                              "The specified value is not within any of the data time ranges you have defined.",
                              BasePrint::P_READERROR, dataSource->getCurrentRecordIndex());
                continue;
            }
            node->refIntC_C()[daysSinceIncidence + _zero_translation_additive] += count;
            _caselessWindows.set(daysSinceIncidence + _zero_translation_additive);
        } else throw prg_error("Unknown condition encountered: scan (%d), model (%d).", "readCounts()", _parameters.getScanType(), _parameters.getModelType());
    }

    _caselessWindows.flip(); // flip so that windows without cases are on instead of off
    if (Parameters::isTemporalScanType(_parameters.getScanType()) && _caselessWindows.count() > 0) {
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
 Reads tree structure from passed file. The file format is: <node identifier>, <parent node identifier 1>, <parent node identifier 2>, ... <parent node identifier N>
 */
bool ScanRunner::readTree(const std::string& filename, unsigned int treeOrdinal) {
    // TODO: Eventually this will need refactoring once we implement multiple data time ranges.
    size_t daysInDataTimeRange = Parameters::isTemporalScanType(_parameters.getScanType()) ? _parameters.getDataTimeRangeSet().getTotalDaysAcrossRangeSets() + 1 : 1;

    // tree only does not read the tree structure file, aggregate all data into one node
    if (_parameters.getScanType() == Parameters::TIMEONLY) {
        std::auto_ptr<NodeStructure> node(new NodeStructure("All", _parameters, daysInDataTimeRange));
        _Nodes.insert(_Nodes.begin(), node.release());
        return true;
    }

    if (_parameters.getTreeFileNames().size() == 1)
        _print.Printf("Reading tree file ...\n", BasePrint::P_STDOUT);
    else
        _print.Printf("Reading %u%s tree file ...\n", BasePrint::P_STDOUT, treeOrdinal, getOrdinalSuffix(treeOrdinal));
    bool readSuccess=true;
    std::auto_ptr<DataSource> dataSource(DataSource::getNewDataSourceObject(filename, _parameters.getInputSource(Parameters::TREE_FILE)));

    // first collect all nodes -- this will allow the referencing of parent nodes not yet encountered
    while (dataSource->readRecord()) {
        if (dataSource->getNumValues() > 2) {
            readSuccess = false;
            _print.Printf("Error: Record %ld in tree file has %ld values but expecting 2: <node id>, <parent node id>(optional).\n", BasePrint::P_READERROR, dataSource->getCurrentRecordIndex(), dataSource->getNumValues());
            continue;
        }
        std::string identifier = dataSource->getValueAt(0);
        std::auto_ptr<NodeStructure> node(new NodeStructure(trimString(identifier), _parameters, daysInDataTimeRange));
        NodeStructureContainer_t::iterator itr = std::lower_bound(_Nodes.begin(), _Nodes.end(), node.get(), CompareNodeStructureByIdentifier());
        if (itr == _Nodes.end() || (*itr)->getIdentifier() != node.get()->getIdentifier())
            _Nodes.insert(itr, node.release());
    }

    // stop reading tree file if we're already determined there are problems in the file.
    if (!readSuccess) return readSuccess;

    //reset node identifiers to ordinal position in vector -- this is to keep the original algorithm intact since it uses vector indexes heavily
    for (size_t i=0; i < _Nodes.size(); ++i) _Nodes[i]->setID(static_cast<int>(i));

    // create bitset which will help determine if there exists at least one root node
    boost::dynamic_bitset<> nodesWithParents(_Nodes.size());

    // now set parent/children nodes
    dataSource->gotoFirstRecord();
    while (dataSource->readRecord()) {
        NodeStructure * node = 0;
        // assign parent nodes to node
        for (size_t t=0; t < dataSource->getNumValues(); ++t) {
            std::string identifier = dataSource->getValueAt(static_cast<long>(t));
            ScanRunner::Index_t index = getNodeIndex(identifier);
            if (!index.first) {
                /* If this current node being read is a parent field and value is not blank and id is not known, then signal reading a bad record. */
                if (node != 0 && identifier.size() != 0) {
                    readSuccess = false;
                    _print.Printf("Error: Record %ld in tree file has unknown parent node (%s).\n", BasePrint::P_READERROR, dataSource->getCurrentRecordIndex(), identifier.c_str());
                }
            } else {
                if (node) {
                    node->addAsParent(*_Nodes[index.second]);
                    nodesWithParents.set(node->getID());
                    // detect nodes with multiple parents
                    _has_multi_parent_nodes |= node->getParents().size() > 1;
                } else node = _Nodes[index.second];
            }
        }
    }

    // confirm that there exists at least one root node
    if (nodesWithParents.count() == nodesWithParents.size()) {
        readSuccess = false;
        _print.Printf("Error: The tree file must contain at least one node which does not have a parent.\n", BasePrint::P_READERROR);
    }
    return readSuccess;
}

/*
 Reads tree node cuts from passed file. The file format is: <node identifier>, <parent node identifier 1>, <parent node identifier 2>, ... <parent node identifier N>
 */
bool ScanRunner::readCuts(const std::string& filename) {
    if (_parameters.getScanType() == Parameters::TIMEONLY) return true; // time only does not read the tree structure file

    _print.Printf("Reading cuts file ...\n", BasePrint::P_STDOUT);
    bool readSuccess=true;
    std::auto_ptr<DataSource> dataSource(DataSource::getNewDataSourceObject(filename, _parameters.getInputSource(Parameters::CUT_FILE)));
    Parameters::cut_maps_t cut_type_maps = Parameters::getCutTypeMap();

    while (dataSource->readRecord()) {
        if (dataSource->getNumValues() != 2) {
            readSuccess = false;
            _print.Printf("Error: Record %ld in cuts file %s. Expecting <identifier>, <cut type [simple,pairs,triplets,ordinal]> but found %ld value%s.\n",
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
            NodeStructure * node = _Nodes[index.second];
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

/** Returns whether cut is reportable. */
bool ScanRunner::reportableCut(const CutStructure& cut) const {
    return /* Does the top cut rank among replications? */
           (_parameters.getNumReplicationsRequested() > 0 && cut.getRank() < _parameters.getNumReplicationsRequested() + 1) ||
           /* If not performing replications, is the cut's observed greater than expected? (we're only scanning for high rates) */
           (_parameters.getNumReplicationsRequested() == 0 && static_cast<double>(cut.getC()) > cut.getExpected(*this));
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
        while (status && k < getCuts().size() && reportableCut(*getCuts()[k])) {
            cutsWriter.write(k);
            k++;
        }
    }
    // write power evaluation results to separate file
    if (_parameters.getPerformPowerEvaluations()) {
        PowerEstimationRecordWriter(*this).write();
    }
    return status;
}

/* Run Scan. */
bool ScanRunner::run() {
    time_t gStartTime, gEndTime;
    time(&gStartTime); //get start time

    for (Parameters::FileNameContainer_t::const_iterator itr=_parameters.getTreeFileNames().begin(); itr != _parameters.getTreeFileNames().end(); ++itr) {
        if (!readTree(*itr, static_cast<unsigned int>(std::distance(_parameters.getTreeFileNames().begin(), itr) + 1)))
            throw resolvable_error("\nProblem encountered when reading the data from the tree file: \n%s.", itr->c_str());
        if (_print.GetIsCanceled()) return false;
    }
    if (_print.GetIsCanceled()) return false;

    if (_parameters.getCutsFileName().length() && !readCuts(_parameters.getCutsFileName()))
        throw resolvable_error("\nProblem encountered when reading the data from the cut file.");
    if (_print.GetIsCanceled()) return false;

    if (!readCounts(_parameters.getCountFileName()))
        throw resolvable_error("\nProblem encountered when reading the data from the case file.");
    if (_print.GetIsCanceled()) return false;

     if (!setupTree())
        throw resolvable_error("\nProblem encountered when setting up tree.");
    if (_print.GetIsCanceled()) return false;

    if (_parameters.getSequentialScan() && static_cast<unsigned int>(_TotalC) > _parameters.getSequentialMaximumSignal()) {
        // The sequential scan reached target number of cases. Don't continue analysis - instead report such in results file.
        _parameters.setReportCriticalValues(false);
        time(&gEndTime); //get end time
        if (!reportResults(gStartTime, gEndTime)) return false;
        return true;
    }

    // Track critical values if requested or we're performing power evaluations and we're actually calculating the critical values.
    if (_parameters.getReportCriticalValues() || (_parameters.getPerformPowerEvaluations() && _parameters.getCriticalValuesType() == Parameters::CV_MONTECARLO)) {
        _critical_values.reset(new CriticalValues(_parameters.getNumReplicationsRequested()));
    }

    if (!_parameters.getPerformPowerEvaluations() || (_parameters.getPerformPowerEvaluations() && _parameters.getPowerEvaluationType() == Parameters::PE_WITH_ANALYSIS)) {
        bool scan_success=false;
        bool t = (_parameters.getScanType() == Parameters::TIMEONLY && _parameters.isPerformingDayOfWeekAdjustment());
        if ((_parameters.getScanType() == Parameters::TREETIME && _parameters.getConditionalType() == Parameters::NODEANDTIME) ||
            (_parameters.getScanType() == Parameters::TIMEONLY && _parameters.isPerformingDayOfWeekAdjustment()) ||
            (_parameters.getScanType() == Parameters::TREETIME && _parameters.getConditionalType() == Parameters::NODE && _parameters.isPerformingDayOfWeekAdjustment()))
            scan_success = scanTreeTemporalConditionNodeTime();
        else if (_parameters.getModelType() == Parameters::UNIFORM)
            scan_success = scanTreeTemporalConditionNode();
        else
            scan_success = scanTree();
        if (scan_success) {
            if (_print.GetIsCanceled()) return false;
            if (!(_parameters.getNumReplicationsRequested() == 0 || _Cut.size() == 0)) {
                _print.Printf("Doing the %d Monte Carlo simulations ...\n", BasePrint::P_STDOUT, _parameters.getNumReplicationsRequested());
                boost::shared_ptr<AbstractRandomizer> randomizer(AbstractRandomizer::getNewRandomizer(*this));
                if (_parameters.isReadingSimulationData())
                    randomizer->setReading(_parameters.getInputSimulationsFilename());
                if (_parameters.isWritingSimulationData()) {
                    remove(_parameters.getOutputSimulationsFilename().c_str());
                    randomizer->setWriting(_parameters.getOutputSimulationsFilename());
                }
                if (_parameters.getSequentialScan()) {
                    if (!runsequentialsimulations(_parameters.getNumReplicationsRequested())) return false;
                } else if (!runsimulations(randomizer, _parameters.getNumReplicationsRequested(), false)) return false;
            }
        }
    }

    if (_print.GetIsCanceled()) return false;
    if (_parameters.getPerformPowerEvaluations()) {
        if (!runPowerEvaluations()) return false;
    }

    if (_print.GetIsCanceled()) return false;
    time(&gEndTime); //get end time
    if (!reportResults(gStartTime, gEndTime)) return false;

    return true;
}

/* Runs power evaluations. */
bool ScanRunner::runPowerEvaluations() {
    switch (_parameters.getCriticalValuesType()) {
        case Parameters::CV_MONTECARLO:
            // if simulations not already done is analysis stage, perform them now
            if (!_simVars.get_sim_count()) {
                _print.Printf("Doing the %d Monte Carlo simulations ...\n", BasePrint::P_STDOUT, _parameters.getNumReplicationsRequested());
                boost::shared_ptr<AbstractRandomizer> randomizer(AbstractRandomizer::getNewRandomizer(*this));
                if (_parameters.isReadingSimulationData())
                    randomizer->setReading(_parameters.getInputSimulationsFilename());
                if (_parameters.isWritingSimulationData()) {
                    remove(_parameters.getOutputSimulationsFilename().c_str());
                    randomizer->setWriting(_parameters.getOutputSimulationsFilename());
                }
                if (!runsimulations(randomizer, _parameters.getNumReplicationsRequested(), false)) return false;
            }
            // sim vars to track those LLRs
            _simVars.reset(_critical_values->getAlpha05().second);
            _simVars.add_additional_mlc(_critical_values->getAlpha01().second);
            _simVars.add_additional_mlc(_critical_values->getAlpha001().second);
            break;
        case Parameters::CV_POWER_VALUES:
            // sim vars to track those LLRs
            _simVars.reset(_parameters.getCriticalValue05());
            _simVars.add_additional_mlc(_parameters.getCriticalValue01());
            _simVars.add_additional_mlc(_parameters.getCriticalValue001());
            break;
        default: throw prg_error("Unknown type '%d'.", "runPowerEvaluations()", _parameters.getCriticalValuesType());
    };
    if (_print.GetIsCanceled()) return false;
    // read power evaluation adjustments
    RiskAdjustmentsContainer_t riskAdjustments;
    if (!readRelativeRisksAdjustments(_parameters.getPowerEvaluationAltHypothesisFilename(), riskAdjustments, false))
        throw resolvable_error("There were problems reading the alternative hypothesis file.", "runPowerEvaluations()");
    if (!riskAdjustments.size())
        throw resolvable_error("Power evaluations can not be performed. No adjustments found in the alternative hypothesis file.");
    SimulationVariables simVarsCopy(_simVars);
    for (size_t t=0; t < riskAdjustments.size(); ++t) {
        _print.Printf("\nDoing the alternative replications for power set %d\n", BasePrint::P_STDOUT, t+1);
        boost::shared_ptr<AbstractRandomizer> core_randomizer;
        if (_parameters.getModelType() == Parameters::BERNOULLI && _parameters.getConditionalType() == Parameters::TOTALCASES) {
            /* Randomization is specialized for the conditional Bernoulli and power evaluations. */
            // calculate the number of individuals in the nodes with an excess risk
            unsigned int n1=0;
            const AdjustmentsContainer_t& adj = riskAdjustments[t]->get();
            double rsk_adjustment = adj.begin()->second.begin()->getRelativeRisk();
            for (AdjustmentsContainer_t::const_iterator itr=adj.begin(); itr != adj.end(); ++itr) {
                n1 += static_cast<unsigned int>(_Nodes[itr->first]->getIntN());
                if (!macro_equal(itr->second.begin()->getRelativeRisk(), rsk_adjustment, DBL_CMP_TOLERANCE))
                    throw resolvable_error("For the conditional Bernoulli model, the relative risks must be the same for all nodes in each power set in the alternative hypothesis file.");
            }
            // Verify that event probability is less than the alternative hypothesis.
            if (macro_less_than(adj.begin()->second.begin()->getRelativeRisk(), _parameters.getPowerBaselineProbability(), DBL_CMP_TOLERANCE))
                throw resolvable_error("The relative risks in power set %d, of the alternative hypothesis file, is less than the baseline event probability.", t+1);

            /* Check that values for p0, p1 and total cases is reasonable in terms of actual data. */
            double z = 0.001; // now we will set z=0.001, but we should experiment a little with different values
            double C = getTotalC();
            double N = static_cast<double>(getTotalC() + getTotalControls());
            double p0 = _parameters.getPowerBaselineProbability();
            double p1 = riskAdjustments[t]->get().begin()->second.begin()->getRelativeRisk();
            double p = (n1*p1+(N-n1)*p0)/N;
            BinomialGenerator bg;
            double z_check = bg.getBinomialDistributionProbability(static_cast<unsigned int>(C), static_cast<unsigned int>(N), p);
            if (!(z < z_check && z_check < 1.0 - z)) {
                double X = (n1 * p1 + (N - n1) * p0);
                throw resolvable_error("\nWith the specified null and alternative hypotheses in power set %d, the expected number of cases is %.1lf,\n"
                                       "but the observed number of cases is %d, which is unrealistic for the null and alternative hypotheses specified.\n"
                                       "Please specify either a total number of observed cases to be approximately %.1lf, or, change the null and/or\n"
                                       "alternative hypothesis so that the expected number of cases, n1 * p1 + (N - n1) * p0 is approximately %d.\n"
                                       "Current values are: n1 = %d, p1 = %g, N = %d, p0 = %g\n",
                                       t+1, X, getTotalC(), X, getTotalC(), n1, p1, static_cast<unsigned int>(N), p0);
            }
            core_randomizer.reset(new ConditionalBernoulliAlternativeHypothesisRandomizer(getNodes(), *riskAdjustments[t],
                                                                                          getTotalC(), getTotalControls(),
                                                                                          _parameters.getPowerBaselineProbability(), 
                                                                                          riskAdjustments[t]->get().begin()->second.begin()->getRelativeRisk()/*risk is required to be same for all nodes*/,
                                                                                          n1, _parameters, getMultiParentNodesExist()));
        } else if (Parameters::isTemporalScanType(_parameters.getScanType())) {
            /* Randomization is specialized for the tree-time/time-only scans and power evaluations. */
            core_randomizer.reset(new TemporalAlternativeHypothesisRandomizer(*this));
        } else {
            /* Use the same randomizer as the null hypothesis randomization. */
            core_randomizer.reset(AbstractRandomizer::getNewRandomizer(*this));
        }
        boost::shared_ptr<AbstractRandomizer> randomizer(new AlternativeHypothesisRandomizater(getNodes(), core_randomizer, *riskAdjustments[t], _parameters, _TotalC, _has_multi_parent_nodes));
        if (_parameters.isWritingSimulationData()) {
            if (t == 0 && _parameters.getCriticalValuesType() == Parameters::CV_POWER_VALUES)
                // if we didn't perform monte carlo simulations, truncate simulations write file on first power simulation
                remove(_parameters.getOutputSimulationsFilename().c_str());
            randomizer->setWriting(_parameters.getOutputSimulationsFilename());
        }
        if (!runsimulations(randomizer, _parameters.getPowerEvaluationReplications(), true, static_cast<unsigned int>(t))) return false;
        _power_estimations.push_back(PowerEstimationSet_t(static_cast<double>(_simVars.get_llr_counters().at(0).second)/static_cast<double>(_parameters.getPowerEvaluationReplications()),
                                                          static_cast<double>(_simVars.get_llr_counters().at(1).second)/static_cast<double>(_parameters.getPowerEvaluationReplications()),
                                                          static_cast<double>(_simVars.get_llr_counters().at(2).second)/static_cast<double>(_parameters.getPowerEvaluationReplications())));
        // reset simulation variables for next power estimation iteration
        _simVars = simVarsCopy;
    }
    return true;
}

/* DOING THE MONTE CARLO SIMULATIONS */
bool ScanRunner::runsimulations(boost::shared_ptr<AbstractRandomizer> randomizer, unsigned int num_relica, bool isPowerStep, unsigned int iteration) {
    const char * sReplicationFormatString = "The result of Monte Carlo replica #%u of %u replications is: %lf\n";
    unsigned long ulParallelProcessCount = std::min(_parameters.getNumParallelProcessesToExecute(), num_relica);

    try {
        PrintQueue lclPrintDirection(_print, false);
        MCSimJobSource jobSource(::GetCurrentTime_HighResolution(), lclPrintDirection, sReplicationFormatString, *this, num_relica, isPowerStep, iteration);
        typedef contractor<MCSimJobSource> contractor_type;
        contractor_type theContractor(jobSource);

        std::deque<boost::shared_ptr<AbstractRandomizer> > _randomizers;
        _randomizers.push_back(randomizer);
        for (unsigned u=1; u < ulParallelProcessCount; ++u) {
            _randomizers.push_back(boost::shared_ptr<AbstractRandomizer>(randomizer->clone()));
        }

        //run threads:
        boost::thread_group tg;
        boost::mutex        thread_mutex;
        for (unsigned u=0; u < ulParallelProcessCount; ++u) {
            try {
                MCSimSuccessiveFunctor mcsf(thread_mutex, _randomizers.at(u), *this);
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
                if (u == 0) throw prg_error("Unknown program error occurred.\n","runsimulations()"); // if this is the first thread, throw exception
                _print.Printf("Error: Unknown program exception occurred creating %u%s parallel simulation ... continuing analysis with %u parallel simulations.\n", BasePrint::P_ERROR, u + 1, (u == 1 ? "nd" : (u == 2 ? "rd" : "th")), u);
                break;
            }
        }
        tg.join_all();

        //propagate exceptions if needed
        theContractor.throw_unhandled_exception();
        jobSource.Assert_NoExceptionsCaught();
        if (jobSource.GetUnregisteredJobCount() > 0)
            throw prg_error("At least %d jobs remain uncompleted.", "ScanRunner", jobSource.GetUnregisteredJobCount());
    } catch (prg_exception& x) {
        x.addTrace("runsimulations()","ScanRunner");
        throw;
    }
    return true;
}

/* DOING THE MONTE CARLO SIMULATIONS */
bool ScanRunner::runsequentialsimulations(unsigned int num_relica) {
    const char * sReplicationFormatString = "The result of Monte Carlo replica #%u of %u replications is: %lf\n";
    unsigned long ulParallelProcessCount = std::min(_parameters.getNumParallelProcessesToExecute(), num_relica);

    try {
        PrintQueue lclPrintDirection(_print, false);
        MCSimJobSource jobSource(::GetCurrentTime_HighResolution(), lclPrintDirection, sReplicationFormatString, *this, num_relica, false);
        typedef contractor<MCSimJobSource> contractor_type;
        contractor_type theContractor(jobSource);

        boost::shared_ptr<SequentialFileDataSource> source;
        boost::shared_ptr<SequentialScanLoglikelihoodRatioWriter> sequential_writer;
        std::string buffer;
        if (ValidateFileAccess(SequentialScanLoglikelihoodRatioWriter::getFilename(_parameters, buffer), false)) {
            source.reset(new SequentialFileDataSource(buffer, _parameters));
            source->gotoFirstRecord();
            // If reading sequential LLR values from file data source, limit to one thread to simply reading file.
            ulParallelProcessCount = 1;
        } else {
            sequential_writer.reset(new SequentialScanLoglikelihoodRatioWriter(*this));
        }

        //run threads:
        boost::thread_group tg;
        boost::mutex thread_mutex;
        for (unsigned u=0; u < ulParallelProcessCount; ++u) {
            try {
                if (source.get()) {
                    SequentialReadMCSimSuccessiveFunctor mcsf(thread_mutex, *this, source);
                    tg.create_thread(subcontractor<contractor_type,SequentialReadMCSimSuccessiveFunctor>(theContractor,mcsf));
                } else {
                    SequentialMCSimSuccessiveFunctor mcsf(thread_mutex, *this, sequential_writer);
                    tg.create_thread(subcontractor<contractor_type,SequentialMCSimSuccessiveFunctor>(theContractor,mcsf));
                }
            } catch (std::bad_alloc &) {
                if (u == 0) throw; // if this is the first thread, re-throw exception
                _print.Printf("Notice: Insufficient memory to create %u%s parallel simulation ... continuing analysis with %u parallel simulations.\n", BasePrint::P_NOTICE, u + 1, (u == 1 ? "nd" : (u == 2 ? "rd" : "th")), u);
                break;
            } catch (prg_exception& x) {
                if (u == 0) throw; // if this is the first thread, re-throw exception
                _print.Printf("Error: Program exception occurred creating %u%s parallel simulation ... continuing analysis with %u parallel simulations.\nException:%s\n", BasePrint::P_ERROR, u + 1, (u == 1 ? "nd" : (u == 2 ? "rd" : "th")), u, x.what());
                break;
            } catch (...) {
                if (u == 0) throw prg_error("Unknown program error occurred.\n","runsimulations()"); // if this is the first thread, throw exception
                _print.Printf("Error: Unknown program exception occurred creating %u%s parallel simulation ... continuing analysis with %u parallel simulations.\n", BasePrint::P_ERROR, u + 1, (u == 1 ? "nd" : (u == 2 ? "rd" : "th")), u);
                break;
            }
        }
        tg.join_all();

        //propagate exceptions if needed
        theContractor.throw_unhandled_exception();
        jobSource.Assert_NoExceptionsCaught();
        if (jobSource.GetUnregisteredJobCount() > 0)
            throw prg_error("At least %d jobs remain uncompleted.", "ScanRunner", jobSource.GetUnregisteredJobCount());
    } catch (prg_exception& x) {
        x.addTrace("runsequentialsimulations()","ScanRunner");
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
        if (_Nodes[n]->getBrC() > 1) {
            const NodeStructure& thisNode(*_Nodes[n]);

            // Always do simple cut for each node
            //printf("Evaluating cut [%s]\n", thisNode.getIdentifier().c_str());
            updateCuts(n, thisNode.getBrC(), thisNode.getBrN(), calcLogLikelihood);
            //++hits; printf("hits %d\n", hits);

            //if (thisNode.getChildren().size() == 0) continue;
            Parameters::CutType cutType = thisNode.getChildren().size() >= 2 ? thisNode.getCutType() : Parameters::SIMPLE;
            switch (cutType) {
                case Parameters::SIMPLE: break; // already done, regardless of specified node cut
                case Parameters::ORDINAL: {
                    CutStructure::CutChildContainer_t currentChildren;
                    // Ordinal cuts: ABCD -> AB, ABC, ABCD, BC, BCD, CD
                    for (size_t i=0; i < thisNode.getChildren().size() - 1; ++i) {
                        const NodeStructure& firstChildNode(*(thisNode.getChildren()[i]));
                        currentChildren.clear();
                        currentChildren.push_back(firstChildNode.getID());
                        //buffer = firstChildNode.getIdentifier().c_str();
                        int sumBranchC=firstChildNode.getBrC();
                        double sumBranchN=firstChildNode.getBrN();
                        for (size_t j=i+1; j < thisNode.getChildren().size(); ++j) {
                            const NodeStructure& childNode(*(thisNode.getChildren()[j]));
                            currentChildren.push_back(childNode.getID());
                            //buffer += ",";
                            //buffer += childNode.getIdentifier();
                            sumBranchC += childNode.getBrC();
                            sumBranchN += childNode.getBrN();
                            //printf("Evaluating cut [%s]\n", buffer.c_str());
                            CutStructure * cut = updateCuts(n, sumBranchC, sumBranchN, calcLogLikelihood);
                            if (cut) {
                                cut->setCutChildren(currentChildren);
                            }
                            //++hits; printf("hits %d\n", hits);
                        }
                    }
                } break;
                case Parameters::PAIRS:
                    // Pair cuts: ABCD -> AB, AC, AD, BC, BD, CD
                    for (size_t i=0; i < thisNode.getChildren().size() - 1; ++i) {
                        const NodeStructure& startChildNode(*(thisNode.getChildren()[i]));
                        for (size_t j=i+1; j < thisNode.getChildren().size(); ++j) {
                            const NodeStructure& stopChildNode(*(thisNode.getChildren()[j]));
                            //printf("Evaluating cut [%s,%s]\n", startChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                            CutStructure * cut = updateCuts(n, startChildNode.getBrC() + stopChildNode.getBrC(), startChildNode.getBrN() + stopChildNode.getBrN(), calcLogLikelihood);
                            if (cut) {
                                cut->addCutChild(startChildNode.getID(), true);
                                cut->addCutChild(stopChildNode.getID());
                            }                            //++hits; printf("hits %d\n", hits);
                        }
                    } break;
                case Parameters::TRIPLETS:
                    // Triple cuts: ABCD -> AB, AC, ABC, AD, ABD, ACD, BC, BD, BCD, CD
                    for (size_t i=0; i < thisNode.getChildren().size() - 1; ++i) {
                        const NodeStructure& startChildNode(*(thisNode.getChildren()[i]));
                        for (size_t j=i+1; j < thisNode.getChildren().size(); ++j) {
                            const NodeStructure& stopChildNode(*(thisNode.getChildren()[j]));
                            //printf("Evaluating cut [%s,%s]\n", startChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                            CutStructure * cut = updateCuts(n, startChildNode.getBrC() + stopChildNode.getBrC(), startChildNode.getBrN() + stopChildNode.getBrN(), calcLogLikelihood);
                            if (cut) {
                                cut->addCutChild(startChildNode.getID(), true);
                                cut->addCutChild(stopChildNode.getID());
                            }
                            //++hits;printf("hits %d\n", hits);
                            for (size_t k=i+1; k < j; ++k) {
                                const NodeStructure& middleChildNode(*(thisNode.getChildren()[k]));
                                //printf("Evaluating cut [%s,%s,%s]\n", startChildNode.getIdentifier().c_str(), middleChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                CutStructure * cut = updateCuts(n, startChildNode.getBrC() + middleChildNode.getBrC() + stopChildNode.getBrC(), startChildNode.getBrN() + middleChildNode.getBrN() + stopChildNode.getBrN(), calcLogLikelihood);
                                if (cut) {
                                    cut->addCutChild(startChildNode.getID(), true);
                                    cut->addCutChild(middleChildNode.getID());
                                    cut->addCutChild(stopChildNode.getID());
                                }
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
        _print.Printf("The log likelihood ratio of the most likely cut is %lf.\n", BasePrint::P_STDOUT, calcLogLikelihood->LogLikelihoodRatio(_Cut[0]->getLogLikelihood()));
    }
    return _Cut.size() != 0;
}

/* SCANNING THE TREE for temporal model */
bool ScanRunner::scanTreeTemporalConditionNode() {
    _print.Printf("Scanning the tree.\n", BasePrint::P_STDOUT);
    ScanRunner::Loglikelihood_t calcLogLikelihood(AbstractLoglikelihood::getNewLoglikelihood(_parameters, _TotalC, _TotalN));

    // Define the start and end windows with the zero index offset already incorporated.
    DataTimeRange startWindow(_parameters.getTemporalStartRange().getStart() + _zero_translation_additive,
                              _parameters.getTemporalStartRange().getEnd() + _zero_translation_additive),
                  endWindow(_parameters.getTemporalEndRange().getStart() + _zero_translation_additive,
                            _parameters.getTemporalEndRange().getEnd() + _zero_translation_additive);
    // Define the minimum and maximum window lengths.
    WindowLength window(static_cast<int>(_parameters.getMinimumWindowLength()) - 1,
                        static_cast<int>(_parameters.getMaximumWindowInTimeUnits()) - 1);
    int  iWindowStart, iMinWindowStart, iWindowEnd, iMaxEndWindow;

    //std::string buffer;
    //int hits=0;
    for (size_t n=0; n < _Nodes.size(); ++n) {
        if (_Nodes[n]->getBrC() > 1) {
            const NodeStructure& thisNode(*(_Nodes[n]));

            // always do simple cut
            //printf("Evaluating cut [%s]\n", thisNode.getIdentifier().c_str());
            iMaxEndWindow = std::min(endWindow.getEnd(), startWindow.getEnd() + window.maximum());
            for (iWindowEnd=endWindow.getStart(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
                iMinWindowStart = std::max(iWindowEnd - window.maximum(), startWindow.getStart());
                iWindowStart = std::min(startWindow.getEnd(), iWindowEnd - window.minimum());
                for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
                    //_print.Printf("%d to %d\n", BasePrint::P_STDOUT,iWindowStart, iWindowEnd);
                    updateCuts(n, thisNode.getBrC_C()[iWindowStart] - thisNode.getBrC_C()[iWindowEnd + 1], static_cast<NodeStructure::expected_t>(thisNode.getBrC()), calcLogLikelihood, iWindowStart, iWindowEnd);
                }
            }
            //++hits; printf("hits %d\n", hits);

            //if (thisNode.getChildren().size() == 0) continue;
            Parameters::CutType cutType = thisNode.getChildren().size() >= 2 ? thisNode.getCutType() : Parameters::SIMPLE;
            switch (cutType) {
                case Parameters::SIMPLE: break;// already done, regardless of specified node cut
                case Parameters::ORDINAL: {
                    // Ordinal cuts: ABCD -> AB, ABC, ABCD, BC, BCD, CD
                    CutStructure::CutChildContainer_t currentChildren;
                    iMaxEndWindow = std::min(endWindow.getEnd(), startWindow.getEnd() + window.maximum());
                    for (iWindowEnd=endWindow.getStart(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
                        iMinWindowStart = std::max(iWindowEnd - window.maximum(), startWindow.getStart());
                        iWindowStart = std::min(startWindow.getEnd(), iWindowEnd - window.minimum());
                        for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
                            for (size_t i=0; i < thisNode.getChildren().size() - 1; ++i) {
                                const NodeStructure& firstChildNode(*(thisNode.getChildren()[i]));
                                currentChildren.clear();
                                currentChildren.push_back(firstChildNode.getID());
                                //buffer = firstChildNode.getIdentifier().c_str();
                                NodeStructure::count_t branchWindow = firstChildNode.getBrC_C()[iWindowStart] - firstChildNode.getBrC_C()[iWindowEnd + 1];
                                NodeStructure::expected_t branchSum = static_cast<NodeStructure::expected_t>(firstChildNode.getBrC());
                                for (size_t j=i+1; j < thisNode.getChildren().size(); ++j) {
                                    const NodeStructure& childNode(*(thisNode.getChildren()[j]));
                                    currentChildren.push_back(childNode.getID());
                                    //buffer += ",";
                                    //buffer += childNode.getIdentifier();
                                    branchWindow += childNode.getBrC_C()[iWindowStart] - childNode.getBrC_C()[iWindowEnd + 1];
                                    branchSum += static_cast<NodeStructure::expected_t>(childNode.getBrC());
                                    //printf("Evaluating cut [%s]\n", buffer.c_str());
                                    CutStructure * cut = updateCuts(n, branchWindow, branchSum, calcLogLikelihood, iWindowStart, iWindowEnd);
                                    if (cut) {
                                        cut->setCutChildren(currentChildren);
                                    }
                                    //++hits; printf("hits %d\n", hits);
                                }
                            }
                        }
                    }
                } break;
                case Parameters::PAIRS:
                    // Pair cuts: ABCD -> AB, AC, AD, BC, BD, CD
                    iMaxEndWindow = std::min(endWindow.getEnd(), startWindow.getEnd() + window.maximum());
                    for (iWindowEnd=endWindow.getStart(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
                        iMinWindowStart = std::max(iWindowEnd - window.maximum(), startWindow.getStart());
                        iWindowStart = std::min(startWindow.getEnd(), iWindowEnd - window.minimum());
                        for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
                            for (size_t i=0; i < thisNode.getChildren().size() - 1; ++i) {
                                const NodeStructure& startChildNode(*(thisNode.getChildren()[i]));
                                NodeStructure::count_t startBranchWindow = startChildNode.getBrC_C()[iWindowStart] - startChildNode.getBrC_C()[iWindowEnd + 1];
                                NodeStructure::expected_t startBranchSum = static_cast<NodeStructure::expected_t>(startChildNode.getBrC());
                                for (size_t j=i+1; j < thisNode.getChildren().size(); ++j) {
                                    const NodeStructure& stopChildNode(*(thisNode.getChildren()[j]));
                                    //printf("Evaluating cut [%s,%s]\n", startChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                    NodeStructure::count_t stopBranchWindow = stopChildNode.getBrC_C()[iWindowStart] - stopChildNode.getBrC_C()[iWindowEnd + 1];
                                    NodeStructure::expected_t stopBranchSum = static_cast<NodeStructure::expected_t>(stopChildNode.getBrC());
                                    CutStructure * cut = updateCuts(n, startBranchWindow + stopBranchWindow, startBranchSum + stopBranchSum, calcLogLikelihood, iWindowStart, iWindowEnd);
                                    if (cut) {
                                        cut->addCutChild(startChildNode.getID(), true);
                                        cut->addCutChild(stopChildNode.getID());
                                    }
                                    //++hits; printf("hits %d\n", hits);
                                }
                            }
                        }
                    } break;
                case Parameters::TRIPLETS:
                    // Triple cuts: ABCD -> AB, AC, ABC, AD, ABD, ACD, BC, BD, BCD, CD
                    iMaxEndWindow = std::min(endWindow.getEnd(), startWindow.getEnd() + window.maximum());
                    for (iWindowEnd=endWindow.getStart(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
                        iMinWindowStart = std::max(iWindowEnd - window.maximum(), startWindow.getStart());
                        iWindowStart = std::min(startWindow.getEnd(), iWindowEnd - window.minimum());
                        for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
                            for (size_t i=0; i < thisNode.getChildren().size() - 1; ++i) {
                                const NodeStructure& startChildNode(*(thisNode.getChildren()[i]));
                                NodeStructure::count_t startBranchWindow = startChildNode.getBrC_C()[iWindowStart] - startChildNode.getBrC_C()[iWindowEnd + 1];
                                NodeStructure::expected_t startBranchSum = static_cast<NodeStructure::expected_t>(startChildNode.getBrC());
                                for (size_t j=i+1; j < thisNode.getChildren().size(); ++j) {
                                    const NodeStructure& stopChildNode(*(thisNode.getChildren()[j]));
                                    //printf("Evaluating cut [%s,%s]\n", startChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                    NodeStructure::count_t stopBranchWindow = stopChildNode.getBrC_C()[iWindowStart] - stopChildNode.getBrC_C()[iWindowEnd + 1];
                                    NodeStructure::expected_t stopBranchSum = static_cast<NodeStructure::expected_t>(stopChildNode.getBrC());
                                    CutStructure * cut = updateCuts(n, startBranchWindow + stopBranchWindow, startBranchSum + stopBranchSum, calcLogLikelihood, iWindowStart, iWindowEnd);
                                    if (cut) {
                                        cut->addCutChild(startChildNode.getID(), true);
                                        cut->addCutChild(stopChildNode.getID());
                                    }
                                    //++hits;printf("hits %d\n", hits);
                                    for (size_t k=i+1; k < j; ++k) {
                                        const NodeStructure& middleChildNode(*(thisNode.getChildren()[k]));
                                        NodeStructure::count_t middleBranchWindow = middleChildNode.getBrC_C()[iWindowStart] - middleChildNode.getBrC_C()[iWindowEnd + 1];
                                        NodeStructure::expected_t middleBranchSum = static_cast<NodeStructure::expected_t>(middleChildNode.getBrC());
                                        //printf("Evaluating cut [%s,%s,%s]\n", startChildNode.getIdentifier().c_str(), middleChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                        CutStructure * cut = updateCuts(n, startBranchWindow + middleBranchWindow + stopBranchWindow, startBranchSum + middleBranchSum + stopBranchSum, calcLogLikelihood, iWindowStart, iWindowEnd);
                                        if (cut) {
                                            cut->addCutChild(startChildNode.getID(), true);
                                            cut->addCutChild(middleChildNode.getID());
                                            cut->addCutChild(stopChildNode.getID());
                                        }
                                        //++hits; printf("hits %d\n", hits);
                                    }
                                }
                            }
                        }
                    } break;
                case Parameters::COMBINATORIAL:
                default: throw prg_error("Unknown cut type (%d).", "scanTreeTemporalConditionNode()", cutType);
            }
        }
    }
    if (_Cut.size()) {
        std::sort(_Cut.begin(), _Cut.end(), CompareCutsByLoglikelihood());
        _print.Printf("The log likelihood ratio of the most likely cut is %lf.\n", BasePrint::P_STDOUT, calcLogLikelihood->LogLikelihoodRatio(_Cut[0]->getLogLikelihood()));
    }
    return _Cut.size() != 0;
}

/* SCANNING THE TREE for temporal model -- conditioned on the total cases across nodes and time. */
bool ScanRunner::scanTreeTemporalConditionNodeTime() {
    _print.Printf("Scanning the tree.\n", BasePrint::P_STDOUT);
    ScanRunner::Loglikelihood_t calcLogLikelihood(AbstractLoglikelihood::getNewLoglikelihood(_parameters, _TotalC, _TotalN));

    // Define the start and end windows with the zero index offset already incorporated.
    DataTimeRange startWindow(_parameters.getTemporalStartRange().getStart() + _zero_translation_additive,
                              _parameters.getTemporalStartRange().getEnd() + _zero_translation_additive),
                  endWindow(_parameters.getTemporalEndRange().getStart() + _zero_translation_additive,
                            _parameters.getTemporalEndRange().getEnd() + _zero_translation_additive);
    // Define the minimum and maximum window lengths.
    WindowLength window(static_cast<int>(_parameters.getMinimumWindowLength()) - 1,
                        static_cast<int>(_parameters.getMaximumWindowInTimeUnits()) - 1);
    int  iWindowStart, iMinWindowStart, iWindowEnd, iMaxEndWindow;

    //std::string buffer;
    //int hits=0;
    for (size_t n=0; n < _Nodes.size(); ++n) {
        if (_Nodes[n]->getBrC() > 1) {
            const NodeStructure& thisNode(*(_Nodes[n]));

            // always do simple cut
            //printf("Evaluating cut [%s]\n", thisNode.getIdentifier().c_str());
            iMaxEndWindow = std::min(endWindow.getEnd(), startWindow.getEnd() + window.maximum());
            for (iWindowEnd=endWindow.getStart(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
                iMinWindowStart = std::max(iWindowEnd - window.maximum(), startWindow.getStart());
                iWindowStart = std::min(startWindow.getEnd(), iWindowEnd - window.minimum());
                for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
                    //_print.Printf("%d to %d\n", BasePrint::P_STDOUT,iWindowStart, iWindowEnd);
                    updateCuts(n, thisNode.getBrC_C()[iWindowStart] - thisNode.getBrC_C()[iWindowEnd + 1],
                                  thisNode.getBrN_C()[iWindowStart] - thisNode.getBrN_C()[iWindowEnd + 1],
                                  calcLogLikelihood, iWindowStart, iWindowEnd);
                }
            }
            //++hits; printf("hits %d\n", hits);

            //if (thisNode.getChildren().size() == 0) continue;
            Parameters::CutType cutType = thisNode.getChildren().size() >= 2 ? thisNode.getCutType() : Parameters::SIMPLE;
            switch (cutType) {
                case Parameters::SIMPLE: break;// already done, regardless of specified node cut
                case Parameters::ORDINAL: {
                    // Ordinal cuts: ABCD -> AB, ABC, ABCD, BC, BCD, CD
                    CutStructure::CutChildContainer_t currentChildren;
                    iMaxEndWindow = std::min(endWindow.getEnd(), startWindow.getEnd() + window.maximum());
                    for (iWindowEnd=endWindow.getStart(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
                        iMinWindowStart = std::max(iWindowEnd - window.maximum(), startWindow.getStart());
                        iWindowStart = std::min(startWindow.getEnd(), iWindowEnd - window.minimum());
                        for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
                            for (size_t i=0; i < thisNode.getChildren().size() - 1; ++i) {
                                const NodeStructure& firstChildNode(*(thisNode.getChildren()[i]));
                                currentChildren.clear();
                                currentChildren.push_back(firstChildNode.getID());
                                //buffer = firstChildNode.getIdentifier().c_str();
                                NodeStructure::count_t branchWindow = firstChildNode.getBrC_C()[iWindowStart] - firstChildNode.getBrC_C()[iWindowEnd + 1];
                                NodeStructure::expected_t branchExpected = firstChildNode.getBrN_C()[iWindowStart] - firstChildNode.getBrN_C()[iWindowEnd + 1];
                                for (size_t j=i+1; j < thisNode.getChildren().size(); ++j) {
                                    const NodeStructure& childNode(*(thisNode.getChildren()[j]));
                                    currentChildren.push_back(childNode.getID());
                                    //buffer += ",";
                                    //buffer += childNode.getIdentifier();
                                    branchWindow += childNode.getBrC_C()[iWindowStart] - childNode.getBrC_C()[iWindowEnd + 1];
                                    branchExpected += childNode.getBrN_C()[iWindowStart] - childNode.getBrN_C()[iWindowEnd + 1];
                                    //printf("Evaluating cut [%s]\n", buffer.c_str());
                                    CutStructure * cut = updateCuts(n, branchWindow, branchExpected, calcLogLikelihood, iWindowStart, iWindowEnd);
                                    if (cut) {
                                        cut->setCutChildren(currentChildren);
                                    }
                                    //++hits; printf("hits %d\n", hits);
                                }
                            }
                        }
                    }
                } break;
                case Parameters::PAIRS:
                    // Pair cuts: ABCD -> AB, AC, AD, BC, BD, CD
                    iMaxEndWindow = std::min(endWindow.getEnd(), startWindow.getEnd() + window.maximum());
                    for (iWindowEnd=endWindow.getStart(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
                        iMinWindowStart = std::max(iWindowEnd - window.maximum(), startWindow.getStart());
                        iWindowStart = std::min(startWindow.getEnd(), iWindowEnd - window.minimum());
                        for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
                            for (size_t i=0; i < thisNode.getChildren().size() - 1; ++i) {
                                const NodeStructure& startChildNode(*(thisNode.getChildren()[i]));
                                NodeStructure::count_t startBranchWindow = startChildNode.getBrC_C()[iWindowStart] - startChildNode.getBrC_C()[iWindowEnd + 1];
                                NodeStructure::expected_t startBranchExpected = startChildNode.getBrN_C()[iWindowStart] - startChildNode.getBrN_C()[iWindowEnd + 1];
                                for (size_t j=i+1; j < thisNode.getChildren().size(); ++j) {
                                    const NodeStructure& stopChildNode(*(thisNode.getChildren()[j]));
                                    //printf("Evaluating cut [%s,%s]\n", startChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                    NodeStructure::count_t stopBranchWindow = stopChildNode.getBrC_C()[iWindowStart] - stopChildNode.getBrC_C()[iWindowEnd + 1];
                                    NodeStructure::expected_t stopBranchExpected = stopChildNode.getBrN_C()[iWindowStart] - stopChildNode.getBrN_C()[iWindowEnd + 1];
                                    CutStructure * cut = updateCuts(n, startBranchWindow + stopBranchWindow, startBranchExpected + stopBranchExpected, calcLogLikelihood, iWindowStart, iWindowEnd);
                                    if (cut) {
                                        cut->addCutChild(startChildNode.getID(), true);
                                        cut->addCutChild(stopChildNode.getID());
                                    }
                                    //++hits; printf("hits %d\n", hits);
                                }
                            }
                        }
                    } break;
                case Parameters::TRIPLETS:
                    // Triple cuts: ABCD -> AB, AC, ABC, AD, ABD, ACD, BC, BD, BCD, CD
                    iMaxEndWindow = std::min(endWindow.getEnd(), startWindow.getEnd() + window.maximum());
                    for (iWindowEnd=endWindow.getStart(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
                        iMinWindowStart = std::max(iWindowEnd - window.maximum(), startWindow.getStart());
                        iWindowStart = std::min(startWindow.getEnd(), iWindowEnd - window.minimum());
                        for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
                            for (size_t i=0; i < thisNode.getChildren().size() - 1; ++i) {
                                const NodeStructure& startChildNode(*(thisNode.getChildren()[i]));
                                NodeStructure::count_t startBranchWindow = startChildNode.getBrC_C()[iWindowStart] - startChildNode.getBrC_C()[iWindowEnd + 1];
                                NodeStructure::expected_t startBranchExpected = startChildNode.getBrN_C()[iWindowStart] - startChildNode.getBrN_C()[iWindowEnd + 1];
                                for (size_t j=i+1; j < thisNode.getChildren().size(); ++j) {
                                    const NodeStructure& stopChildNode(*(thisNode.getChildren()[j]));
                                    //printf("Evaluating cut [%s,%s]\n", startChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                    NodeStructure::count_t stopBranchWindow = stopChildNode.getBrC_C()[iWindowStart] - stopChildNode.getBrC_C()[iWindowEnd + 1];
                                    NodeStructure::expected_t stopBranchExpected = stopChildNode.getBrN_C()[iWindowStart] - stopChildNode.getBrN_C()[iWindowEnd + 1];
                                    CutStructure * cut = updateCuts(n, startBranchWindow + stopBranchWindow, startBranchExpected + stopBranchExpected, calcLogLikelihood, iWindowStart, iWindowEnd);
                                    if (cut) {
                                        cut->addCutChild(startChildNode.getID(), true);
                                        cut->addCutChild(stopChildNode.getID());
                                    }
                                    //++hits;printf("hits %d\n", hits);
                                    for (size_t k=i+1; k < j; ++k) {
                                        const NodeStructure& middleChildNode(*(thisNode.getChildren()[k]));
                                        NodeStructure::count_t middleBranchWindow = middleChildNode.getBrC_C()[iWindowStart] - middleChildNode.getBrC_C()[iWindowEnd + 1];
                                        NodeStructure::expected_t middleBranchExpected = middleChildNode.getBrN_C()[iWindowStart] - middleChildNode.getBrN_C()[iWindowEnd + 1];
                                        //printf("Evaluating cut [%s,%s,%s]\n", startChildNode.getIdentifier().c_str(), middleChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                        CutStructure * cut = updateCuts(n, startBranchWindow + middleBranchWindow + stopBranchWindow, startBranchExpected + middleBranchExpected + stopBranchExpected, calcLogLikelihood, iWindowStart, iWindowEnd);
                                        if (cut) {
                                            cut->addCutChild(startChildNode.getID(), true);
                                            cut->addCutChild(middleChildNode.getID());
                                            cut->addCutChild(stopChildNode.getID());
                                        }
                                        //++hits; printf("hits %d\n", hits);
                                    }
                                }
                            }
                        }
                    } break;
                case Parameters::COMBINATORIAL:
                default: throw prg_error("Unknown cut type (%d).", "scanTreeTemporalConditionNodeTime()", cutType);
            }
        }
    }
    if (_Cut.size()) {
        std::sort(_Cut.begin(), _Cut.end(), CompareCutsByLoglikelihood());
        _print.Printf("The log likelihood ratio of the most likely cut is %lf.\n", BasePrint::P_STDOUT, calcLogLikelihood->LogLikelihoodRatio(_Cut[0]->getLogLikelihood()));
    }
    return _Cut.size() != 0;
}

CutStructure * ScanRunner::updateCuts(size_t node_index, int BrC, double BrN, const Loglikelihood_t& logCalculator, DataTimeRange::index_t startIdx, DataTimeRange::index_t endIdx) {
    double loglikelihood=0;

    if ((_parameters.getScanType() == Parameters::TREETIME && _parameters.getConditionalType() == Parameters::NODEANDTIME) ||
        (_parameters.getScanType() == Parameters::TIMEONLY && _parameters.isPerformingDayOfWeekAdjustment()) ||
        (_parameters.getScanType() == Parameters::TREETIME && _parameters.getConditionalType() == Parameters::NODE && _parameters.isPerformingDayOfWeekAdjustment()))
        loglikelihood = logCalculator->LogLikelihood(BrC, BrN);
    else if (_parameters.getModelType() == Parameters::UNIFORM)
        loglikelihood = logCalculator->LogLikelihood(BrC, BrN, endIdx - startIdx + 1);
    else
        loglikelihood = logCalculator->LogLikelihood(BrC, BrN);
    if (loglikelihood == logCalculator->UNSET_LOGLIKELIHOOD) 0;

    std::auto_ptr<CutStructure> cut(new CutStructure());
    cut->setLogLikelihood(loglikelihood);
    cut->setID(static_cast<int>(node_index));
    cut->setC(BrC);
    cut->setN(BrN);
    cut->setStartIdx(startIdx);
    cut->setEndIdx(endIdx);

    CutStructureContainer_t::iterator itr;
    if (_parameters.getScanType() == Parameters::TIMEONLY) {
        // for time-only scans, we want to keep secondary clusters -- possibly one for each end date
        itr = std::lower_bound(_Cut.begin(), _Cut.end(), cut.get(), CompareCutsByEndIdx());
        if (!(itr != _Cut.end() && (*itr)->getEndIdx() == cut->getEndIdx()))
            return *(_Cut.insert(itr, cut.release()));
    } else {
        // we're keeping the best cut for each node
        itr = std::lower_bound(_Cut.begin(), _Cut.end(), cut.get(), CompareCutsById());
        if (!(itr != _Cut.end() && (*itr)->getID() == cut->getID()))
            return *(_Cut.insert(itr, cut.release()));
    }
    // at this point, we're replacing a cut with better log likeloihood cut
    if (cut->getLogLikelihood() > (*itr)->getLogLikelihood()) {
        size_t idx = std::distance(_Cut.begin(), itr);
        delete _Cut[idx]; _Cut[idx]=0;
        _Cut[idx] = cut.release();
        return _Cut[idx];
    }
    return 0;
}

/* SETTING UP THE TREE */
bool ScanRunner::setupTree() {
    _print.Printf("Setting up the tree ...\n", BasePrint::P_STDOUT);

    // Initialize variables
    _TotalC=0;_TotalN=0;
    for(NodeStructureContainer_t::iterator itr=_Nodes.begin(); itr != _Nodes.end(); ++itr) {
        std::fill((*itr)->refBrC_C().begin(), (*itr)->refBrC_C().end(), 0);
        std::fill((*itr)->refBrN_C().begin(), (*itr)->refBrN_C().end(), 0);
    }

    // Calculates the total number of cases
    for(NodeStructureContainer_t::iterator itr=_Nodes.begin(); itr != _Nodes.end(); ++itr) {
        _TotalC = std::accumulate((*itr)->refIntC_C().begin(), (*itr)->refIntC_C().end(), _TotalC);
    }
    // If executing sequential scan and the number of cases in counts file is less than user specified total seqnential cases.
    if (_parameters.getSequentialScan() && static_cast<unsigned int>(_TotalC) > _parameters.getSequentialMaximumSignal()) {
        /* abort this analysis -- return true so that returned to function won't think that an error occurred. */

        //_print.Printf("Error: For the sequential scan, the number of cases in the count file must be less than or equal to the total sequential cases.\n"
        //              "       The count file defines %i cases while the specified number of sequential cases is %u.", BasePrint::P_ERROR, _TotalC, _parameters.getSequentialMaximumSignal());
        return true;
    }
    // Check for minimum number of cases.
    if (!_parameters.getPerformPowerEvaluations() || !(_parameters.getPerformPowerEvaluations() && _parameters.getPowerEvaluationType() == Parameters::PE_ONLY_SPECIFIED_CASES)) {
        // The conditional Poisson should not be performed with less than cases.
        if (_parameters.getModelType() == Parameters::POISSON && _parameters.getConditionalType() == Parameters::TOTALCASES && _TotalC < 2/* minimum number of cases */) {
            _print.Printf("Error: The conditional Poison model requires at least 2 cases to perform analysis. %d counts defined in count file.\n", BasePrint::P_ERROR, _TotalC);
            return false;
        }
    }

    // calculate the number of expected cases
    if (Parameters::isTemporalScanType(_parameters.getScanType())) {
        NodeStructure::CountContainer_t totalcases_by_dayofweek;
        if (_parameters.isPerformingDayOfWeekAdjustment()) {
            // calculate the total number of cases for each day of the week
            totalcases_by_dayofweek.resize(7, 0);
            for (size_t n=0; n < _Nodes.size(); ++n) {
                NodeStructure& node = *(_Nodes[n]);
                const NodeStructure::CountContainer_t& cases = node.getIntC_C();
                for (size_t idx=0; idx < cases.size(); ++idx) {
                    totalcases_by_dayofweek[idx % 7] += cases[idx];
                }
            }
        }
        NodeStructure::CountContainer_t totalcases_by_node(_Nodes.size(), 0);
        for (size_t n=0; n < _Nodes.size(); ++n) {
            totalcases_by_node[n] = std::accumulate(_Nodes[n]->getIntC_C().begin(), _Nodes[n]->getIntC_C().end(), 0);
        }
        // calculate expected number of cases for tree-time scan
        switch (_parameters.getConditionalType()) {
            case Parameters::NODEANDTIME : {
                // calculate total cases by time interval -- we'll need this to calculate the expected cases for conditional tree-temporal scan
                size_t daysInDataTimeRange = _parameters.getDataTimeRangeSet().getTotalDaysAcrossRangeSets() + 1;
                TimeIntervalContainer_t totalcases_by_timeinterval(daysInDataTimeRange, 0);
                for (NodeStructureContainer_t::iterator itr=_Nodes.begin(); itr != _Nodes.end(); ++itr) {
                    for (size_t t=0; t < (*itr)->refIntC_C().size(); ++t) {
                        totalcases_by_timeinterval[t] += (*itr)->refIntC_C()[t];
                    }
                }
                // expected number of cases is calculated slightly different when performing day of week adjustment
                if (_parameters.isPerformingDayOfWeekAdjustment()) {
                    for (size_t n=0; n < _Nodes.size(); ++n) {
                        NodeStructure& node = *(_Nodes[n]);
                        const NodeStructure::CountContainer_t& cases = node.getIntC_C();
                        // first calculate the total number of cases for this node, for each day of the week
                        NodeStructure::CountContainer_t node_cases_by_dayofweek(7, 0);
                        for (size_t idx=0; idx < cases.size(); ++idx) {
                            node_cases_by_dayofweek[idx % 7] += cases[idx];
                        }
                        // now we can calculate the expected number of cases for this node
                        NodeStructure::ExpectedContainer_t& nodeExpected = node.refIntN_C();
                        for (size_t t=0; t < totalcases_by_timeinterval.size(); ++t) {
                            double cases_day_of_week = static_cast<double>(totalcases_by_dayofweek[t % 7]);
                            if (cases_day_of_week) nodeExpected[t] += static_cast<double>(totalcases_by_timeinterval[t]) * static_cast<double>(node_cases_by_dayofweek[t % 7]) / cases_day_of_week;
                        }
                    }
                } else {
                    // now we can calculate the expected number of cases
                    for (size_t n=0; n < _Nodes.size(); ++n) {
                        NodeStructure::ExpectedContainer_t& nodeExpected = _Nodes[n]->refIntN_C();
                        for (size_t t=0; t < totalcases_by_timeinterval.size(); ++t) {
                            nodeExpected[t] += static_cast<double>(totalcases_by_timeinterval[t]) * static_cast<double>(totalcases_by_node[n]) / static_cast<double>(_TotalC);
                        }
                    }
                }
            } break;
            case Parameters::TOTALCASES :
            case Parameters::NODE :
                if (_parameters.isPerformingDayOfWeekAdjustment()) {
                    double daysInDataTimeRange = static_cast<double>(_parameters.getDataTimeRangeSet().getTotalDaysAcrossRangeSets() + 1);
                    for (size_t n=0; n < _Nodes.size(); ++n) {
                        if (totalcases_by_node[n]) {
                            NodeStructure::ExpectedContainer_t& nodeExpected = _Nodes[n]->refIntN_C();
                            for (size_t t=0; t < nodeExpected.size(); ++t) {
                                nodeExpected[t] += static_cast<double>(totalcases_by_node[n]) * (static_cast<double>(totalcases_by_dayofweek[t % 7])/static_cast<double>(_TotalC) / static_cast<double>(_day_of_week_indexes[t % 7].size()));
                            }
                        }
                    }
                } break;
            case Parameters::UNCONDITIONAL :
            default : throw prg_error("Unknown conditional type (%d) for temporal scan type.", "setupTree()", _parameters.getConditionalType());
        }
    }

    // Calculates the total population at risk or total measure
    for(NodeStructureContainer_t::iterator itr=_Nodes.begin(); itr != _Nodes.end(); ++itr) {
        _TotalN = std::accumulate((*itr)->refIntN_C().begin(), (*itr)->refIntN_C().end(), _TotalN);
    }

    // controls are read with population for Bernoulli -- so calculate total controls now
    if (_parameters.getModelType() == Parameters::BERNOULLI) {
        if (_parameters.getPerformPowerEvaluations() && _parameters.getConditionalType() == Parameters::TOTALCASES &&_parameters.getPowerEvaluationType() == Parameters::PE_ONLY_SPECIFIED_CASES) {
            // check that the user specifed number of power cases is not greater than the total population (cases + controls)
            if (static_cast<double>(_parameters.getPowerEvaluationTotalCases()) > _TotalN) {
                _print.Printf("Error: The user specified number of total cases is greater than the total population defined in case file.\n", BasePrint::P_ERROR);
                return false;
            }
            _TotalC = _parameters.getPowerEvaluationTotalCases();
            _TotalControls = static_cast<int>(_TotalN) - _TotalC;
        } else {
            _TotalControls = std::max(static_cast<int>(_TotalN) - _TotalC, 0);
        }
    }

    // Calculates the expected counts for each node and the total.
    if (_parameters.getModelType() == Parameters::POISSON && _parameters.getConditionalType() == Parameters::TOTALCASES) {
        // If performing power calculations with user specified number of cases, override number of cases read from case file.
        // This is ok since this option can not be peformed in conjunction with analysis - so the cases in tree are not used.
        if (_parameters.getPerformPowerEvaluations() && _parameters.getPowerEvaluationType() == Parameters::PE_ONLY_SPECIFIED_CASES)
            _TotalC = _parameters.getPowerEvaluationTotalCases();
        double adjustN = _TotalC/_TotalN;
        for (NodeStructureContainer_t::iterator itr=_Nodes.begin(); itr != _Nodes.end(); ++itr)
            std::transform((*itr)->getIntN_C().begin(), (*itr)->getIntN_C().end(), (*itr)->refIntN_C().begin(), std::bind1st(std::multiplies<double>(), adjustN)); // (*itr)->refIntN() *= adjustN;
        _TotalN = _TotalC;
    }
    /* For each node, calculates the observed and expected number of cases for that
       node together with all of its children, grandchildren, etc. */
    boost::dynamic_bitset<> ancestor_nodes(_has_multi_parent_nodes ? _Nodes.size() : 0);
    for (NodeStructureContainer_t::iterator itr=_Nodes.begin(); itr < _Nodes.end(); ++itr) {
        ancestor_nodes.reset();
        if (addCN_C(*(*itr), *(*itr), ancestor_nodes) > 1) {
            _print.Printf("Error: Node '%s' has itself as an ancestor.\n", BasePrint::P_ERROR, (*itr)->getIdentifier().c_str());
            return false;
        }
        // If we're do replications and there exist nodes with multiple parents, set ancestor indexes for this node.
        if (_parameters.getNumReplicationsRequested() > 0 && _has_multi_parent_nodes)
            (*itr)->setAncestors(ancestor_nodes);
    }

    if (_parameters.getModelType() == Parameters::POISSON ||
        _parameters.getModelType() == Parameters::BERNOULLI ||
        (_parameters.getScanType() == Parameters::TREETIME && _parameters.getConditionalType() == Parameters::NODEANDTIME) ||
        (_parameters.getScanType() == Parameters::TIMEONLY && _parameters.isPerformingDayOfWeekAdjustment()) ||
        (_parameters.getScanType() == Parameters::TREETIME && _parameters.getConditionalType() == Parameters::NODE && _parameters.isPerformingDayOfWeekAdjustment())) {
        // Checks that no node has negative expected cases or that a node with zero expected has observed cases.
        for (size_t i=0; i < _Nodes.size(); ++i) {
            // cout << "Node=" << i << ", BrC=" << Node[i].BrC << ", BrN=" << Node[i].BrN << endl;
            if (_Nodes[i]->getBrN() < 0 ) {
                _print.Printf("Error: Node '%s' has negative expected cases.\n", BasePrint::P_ERROR, _Nodes[i]->getIdentifier().c_str());
                return false;
            }
            if (_Nodes[i]->getBrN() == 0 && _Nodes[i]->getBrC() > 0) {
                _print.Printf("Error: Node '%s' has observed cases but zero expected.\n", BasePrint::P_ERROR, _Nodes[i]->getIdentifier().c_str());
                return false;
            }
        } // for i
    }

    // Now we can set the data structures of NodeStructure to cumulative -- only relevant for temporal model since other models have one element arrays.
    std::for_each(_Nodes.begin(), _Nodes.end(), std::mem_fun(&NodeStructure::setCumulative));
    return true;
}
