
#include <numeric>
#include <boost/tokenizer.hpp>
#include <boost/regex.hpp>
#include <boost/assign.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
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
#include "AlternativeHypothesisRandomizer.h"
#include "RelativeRiskAdjustment.h"
#include "BernoulliRandomizer.h"
#include "TemporalRandomizer.h"
#include "Toolkit.h"
#include "ParameterFileAccess.h"
#include "ChartGenerator.h"

double getExcessCasesFor(const ScanRunner& scanner, int nodeID, int _C, double _N, DataTimeRange::index_t _start_idx, DataTimeRange::index_t _end_idx) {
    const Parameters& parameters = scanner.getParameters();
    double C = static_cast<double>(_C), totalC = static_cast<double>(scanner.getTotalC());
    
    switch (parameters.getScanType()) {
        case Parameters::TREEONLY: {
            switch (parameters.getConditionalType()) {
                case Parameters::UNCONDITIONAL:
                    if (parameters.getModelType() == Parameters::POISSON)
                        return C - _N;
                    if (parameters.getModelType() == Parameters::BERNOULLI_TREE)
                        if (parameters.getSelfControlDesign())
                            return C - scanner.getParameters().getProbability() * (_N - C) / (1.0 - scanner.getParameters().getProbability());
                    return C - _N * scanner.getParameters().getProbability();
                    throw prg_error("Cannot calculate excess cases: tree-only, unconditonal, model (%d).", "getExcessCases()", parameters.getModelType());
                case Parameters::TOTALCASES:
                    if (parameters.getModelType() == Parameters::POISSON) {
                        if (!(scanner.getTotalN() - _N)) throw prg_error("Program error detected: model=%d, totalN=%lf, n=%lf.", "getExcessCases()", parameters.getModelType(), scanner.getTotalN(), _N);
                        return C - _N * (totalC - C) / (scanner.getTotalN() - _N);
                    }
                    if (parameters.getModelType() == Parameters::BERNOULLI_TREE) {
                        if (!(scanner.getTotalN() - _N)) throw prg_error("Program error detected: model=%d, totalN=%lf, n=%lf.", "getExcessCases()", parameters.getModelType(), scanner.getTotalN(), _N);
                        return C - _N * (totalC - C) / (scanner.getTotalN() - _N);
                    }
                    throw prg_error("Cannot calculate excess cases: tree-only, total-cases, model (%d).", "getExcessCases()", parameters.getModelType());
                default: throw prg_error("Cannot calculate excess cases: tree-only, condition type (%d).", "getExcessCases()", parameters.getConditionalType());
            }
        }
        case Parameters::TIMEONLY: /* time-only, condtioned on total cases, is a special case of tree-time, conditioned on the node with only one node */
        case Parameters::TREETIME: {
            switch (parameters.getConditionalType()) {
                case Parameters::TOTALCASES: /* this option is really only for time-only */
                case Parameters::NODE:       /* this option is really only for tree-time */
                    if (parameters.getModelType() == Parameters::UNIFORM) {
                        if (parameters.isPerformingDayOfWeekAdjustment() || scanner.isCensoredData()) {
                            //Obs - Exp * [ (NodeCases-Obs) / (NodeCases-Exp) ]
                            double exp = getExpectedFor(scanner, nodeID, _C, _N, _start_idx, _end_idx);
                            double NodeCases = static_cast<double>(scanner.getNodes()[nodeID]->getBrC());
                            return C - exp * ((NodeCases - C) / (NodeCases - exp));
                        }
                        double W = static_cast<double>(_end_idx - _start_idx + 1.0);
                        double T = static_cast<double>(parameters.getDataTimeRangeSet().getTotalDaysAcrossRangeSets());
                        if (!(T - W)) throw prg_error("Program error detected: model=%d, T=%lf, W=%lf.", "getExcessCases()", parameters.getModelType(), T, W);
                        return C - W * (_N - C) / (T - W);
                    } if (parameters.getModelType() == Parameters::BERNOULLI_TIME) {
                        /* TODO -- is this correct or should it be in terms of node only? */

                        if (!(scanner.getTotalN() - _N)) throw prg_error("Program error detected: model=%d, totalN=%lf, n=%lf.", "getExcessCases()", parameters.getModelType(), scanner.getTotalN(), _N);
                        return C - _N * (totalC - C) / (scanner.getTotalN() - _N);
                    }
                    throw prg_error("Cannot calculate excess cases: tree-time/time-only, total-cases/node, model (%d).", "getExcessCases()", parameters.getModelType());
                case Parameters::NODEANDTIME: {
                    /* c = cases in detected cluster
                    C = total cases in the whole tree
                    C(n)=total cases in the cluster node, summed over the whole study time period
                    C(t)=total cases in the cluster time window, summed over all the nodes
                    Let E2 = (C(n)-c)*(C(t)-c) / (C-C(n)-C(t)+c) -- this is an alternative method for calculating expected counts
                    Excess Cases = c-E2
                    */
                    double Cn = static_cast<double>(scanner.getNodes()[nodeID]->getBrC());
                    double Ct = 0.0;
                    for (size_t t = 0; t < scanner.getNodes().size(); ++t)
                        Ct += static_cast<double>(scanner.getNodes()[t]->getIntC_C()[_start_idx]) - static_cast<double>(scanner.getNodes()[t]->getIntC_C()[_end_idx + 1]);
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

double getExpectedFor(const ScanRunner& scanner, int nodeID, int _C, double _N, DataTimeRange::index_t _start_idx, DataTimeRange::index_t _end_idx) {
    const Parameters& parameters = scanner.getParameters();
    double C = static_cast<double>(_C), totalC = static_cast<double>(scanner.getTotalC());
    switch (parameters.getScanType()) {
        case Parameters::TREEONLY: {
            switch (parameters.getConditionalType()) {
            case Parameters::UNCONDITIONAL:
                if (parameters.getModelType() == Parameters::POISSON)
                    return _N;
                if (parameters.getModelType() == Parameters::BERNOULLI_TREE)
                    return _N * scanner.getParameters().getProbability();
                throw prg_error("Cannot determine expected cases: tree-only, unconditonal, model (%d).", "getExpected()", parameters.getModelType());
            case Parameters::TOTALCASES:
                if (parameters.getModelType() == Parameters::POISSON)
                    return _N * totalC / scanner.getTotalN();
                if (parameters.getModelType() == Parameters::BERNOULLI_TREE)
                    return _N * (scanner.getTotalC() / scanner.getTotalN());
                throw prg_error("Cannot determine expected cases: tree-only, total-cases, model (%d).", "getExpected()", parameters.getModelType());
            default: throw prg_error("Cannot determine expected cases: tree-only, condition type (%d).", "getExpected()", parameters.getConditionalType());
            }
        }
        case Parameters::TREETIME: {
            switch (parameters.getConditionalType()) {
                case Parameters::NODE:
                    if (parameters.getModelType() == Parameters::UNIFORM) {
                        if (parameters.isPerformingDayOfWeekAdjustment() || scanner.isCensoredData()) {
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
                    } else if (parameters.getModelType() == Parameters::BERNOULLI_TIME) {
                        /* TODO -- is this correct or should it be in terms of node only? */
                        return static_cast<double>(scanner.getTotalC()) / scanner.getTotalN() * _N;
                    }
                    throw prg_error("Cannot determine expected cases: tree-time, total-cases, model (%d).", "getExpected()", parameters.getModelType());
                case Parameters::NODEANDTIME:
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
                        if (parameters.isPerformingDayOfWeekAdjustment() || scanner.isCensoredData()) {
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
                    } else if (parameters.getModelType() == Parameters::BERNOULLI_TIME) {
                        /* TODO -- is this correct or should it be in terms of node only? */
                        return static_cast<double>(scanner.getTotalC()) / scanner.getTotalN() * _N;
                    }
                    throw prg_error("Cannot determine expected cases: tree-time, total-cases, model (%d).", "getExpected()", parameters.getModelType());
                default: throw prg_error("Cannot determine expected cases: tree-time, condition type (%d).", "getExpected()", parameters.getConditionalType());
            }
       }
       default: throw prg_error("Unknown scan type (%d).", "getExpected()", parameters.getScanType());
    }
}

/* Calculates the attributable risk per person for cut. */
double getAttributableRiskFor(const ScanRunner& scanner, int nodeID, int _C, double _N, DataTimeRange::index_t _start_idx, DataTimeRange::index_t _end_idx) {
    const Parameters& parameters = scanner.getParameters();
    double C = static_cast<double>(_C), totalC = static_cast<double>(scanner.getTotalC());

    switch (parameters.getScanType()) {
        case Parameters::TREEONLY: {
            switch (parameters.getConditionalType()) {
                case Parameters::UNCONDITIONAL:
                case Parameters::TOTALCASES: return getExcessCasesFor(scanner, nodeID, _C, _N, _start_idx, _end_idx) / static_cast<double>(parameters.getAttributableRiskExposed());
                default: throw prg_error("Cannot calculate attributable risk: tree-only, condition type (%d).", "getAttributableRisk()", parameters.getConditionalType());
            }
        }
        case Parameters::TIMEONLY: /* time-only, condtioned on total cases, is a special case of tree-time, conditioned on the node with only one node */
        case Parameters::TREETIME: {
            switch (parameters.getConditionalType()) {
                case Parameters::TOTALCASES: /* this option is really only for time-only */
                case Parameters::NODE:       /* this option is really only for tree-time */
                    return getExcessCasesFor(scanner, nodeID, _C, _N, _start_idx, _end_idx) / static_cast<double>(parameters.getAttributableRiskExposed());
                case Parameters::NODEANDTIME: {
                    double exp = getExcessCasesFor(scanner, nodeID, _C, _N, _start_idx, _end_idx);
                    double NodeCases = static_cast<double>(scanner.getNodes()[nodeID]->getBrC());
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

/** Returns cut's relative risk. See user guide for formula explanation. */
double getRelativeRiskFor(const ScanRunner& scanner, int nodeID, int _C, double _N, DataTimeRange::index_t _start_idx, DataTimeRange::index_t _end_idx) {
    double relative_risk = 0;
    double C = static_cast<double>(_C), totalC = static_cast<double>(scanner.getTotalC());

    const Parameters& parameters = scanner.getParameters();
        switch (parameters.getScanType()) {
            case Parameters::TREEONLY: {
                switch (parameters.getConditionalType()) {
                    case Parameters::UNCONDITIONAL:
                        if (parameters.getModelType() == Parameters::POISSON) {
                            relative_risk = C / _N;
                            return relative_risk ? relative_risk : std::numeric_limits<double>::infinity();
                        }
                        if (parameters.getModelType() == Parameters::BERNOULLI_TREE) {
                            if (parameters.getSelfControlDesign())
                                relative_risk = (C / parameters.getProbability()) / ((_N - C) / (1.0 - parameters.getProbability()));
                            else
                                relative_risk = C / (_N * parameters.getProbability());
                            return relative_risk ? relative_risk : std::numeric_limits<double>::infinity();
                        }
                        throw prg_error("Cannot calculate relative risk: tree-only, unconditonal, model (%d).", "getRelativeRisk()", parameters.getModelType());
                    case Parameters::TOTALCASES:
                        if (parameters.getModelType() == Parameters::POISSON) {
                            double NN = scanner.getTotalN() - _N;
                            if (!NN) throw prg_error("Program error detected: model=%d, totalN=%lf, n=%lf.", "getRelativeRisk()", parameters.getModelType(), scanner.getTotalN(), _N);
                            double CC = totalC - C;
                            relative_risk = CC ? (C / _N) / (CC / NN) : 0;
                            return relative_risk ? relative_risk : std::numeric_limits<double>::infinity();
                        }
                        if (parameters.getModelType() == Parameters::BERNOULLI_TREE) {
                            double NN = scanner.getTotalN() - _N;
                            if (!NN) throw prg_error("Program error detected: model=%d, totalN=%lf, n=%lf.", "getRelativeRisk()", parameters.getModelType(), scanner.getTotalN(), _N);
                            double CC = totalC - C;
                            relative_risk = CC ? (C / _N) / (CC / NN) : 0;
                            return relative_risk ? relative_risk : std::numeric_limits<double>::infinity();
                        }
                        throw prg_error("Cannot calculate relative risk: tree-only, total-cases, model (%d).", "getRelativeRisk()", parameters.getModelType());
                    default: throw prg_error("Cannot calculate relative risk: tree-only, condition type (%d).", "getRelativeRisk()", parameters.getConditionalType());
                }
            }
            case Parameters::TIMEONLY: /* time-only, conditioned on total cases, is a special case of tree-time, conditioned on the node with only one node */
            case Parameters::TREETIME: {
                switch (parameters.getConditionalType()) {
                case Parameters::TOTALCASES: /* this option is really only for time-only */
                case Parameters::NODE:       /* this option is really only for tree-time */
                    if (parameters.getModelType() == Parameters::UNIFORM) {
                        if (parameters.isPerformingDayOfWeekAdjustment() || scanner.isCensoredData()) {
                            // (Obs/Exp) / [ (NodeCases-Obs) / (NodeCases-Exp) ]
                            double exp = getExpectedFor(scanner, nodeID, _C, _N, _start_idx, _end_idx);
                            double NodeCases = static_cast<double>(scanner.getNodes()[nodeID]->getBrC());
                            if (C == NodeCases)
                                relative_risk = std::numeric_limits<double>::infinity();
                            else
                                relative_risk = (C / exp) / ((NodeCases - C) / (NodeCases - exp));
                            return relative_risk ? relative_risk : std::numeric_limits<double>::infinity();
                        }
                        double W = static_cast<double>(_end_idx - _start_idx + 1.0);
                        double T = static_cast<double>(parameters.getDataTimeRangeSet().getTotalDaysAcrossRangeSets());
                        if (!(T - W)) throw prg_error("Program error detected: model=%d, T=%lf, W=%lf.", "getRelativeRisk()", parameters.getModelType(), T, W);
                        double CC = _N - static_cast<double>(_C);
                        relative_risk = CC ? (static_cast<double>(_C) / W) / (CC / (T - W)) : 0.0;
                        return relative_risk ? relative_risk : std::numeric_limits<double>::infinity();
                    }
                    else if (parameters.getModelType() == Parameters::BERNOULLI_TIME) {
                        /* TODO -- is this correct or should it be in terms of node only? */

                        //when all cases are inside cluster, relative risk goes to infinity
                        if (totalC == static_cast<double>(_C)) return -1;

                        if (_N && totalC - _N && ((totalC - _C) / (totalC - _N)))
                            return (_C / _N) / ((totalC - _C) / (totalC - _N));
                        return 0;
                    }
                    throw prg_error("Cannot calculate excess cases: tree-time/time-only, total-cases/node, model (%d).", "getRelativeRisk()", parameters.getModelType());
                case Parameters::NODEANDTIME: {
                    /* c = cases in detected cluster
                    C = total cases in the whole tree
                    C(n)=total cases in the cluster node, summed over the whole study time period
                    C(t)=total cases in the cluster time window, summed over all the nodes
                    Let E2 = (C(n)-c)*(C(t)-c) / (C-C(n)-C(t)+c) -- this is an alternative method for calculating expected counts
                    RR = c/E2
                    */
                    double Cn = static_cast<double>(scanner.getNodes()[nodeID]->getBrC());
                    double Ct = 0.0;
                    for (size_t t = 0; t < scanner.getNodes().size(); ++t)
                        Ct += static_cast<double>(scanner.getNodes()[t]->getIntC_C()[_start_idx]) - static_cast<double>(scanner.getNodes()[t]->getIntC_C()[_end_idx + 1]);
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

/** Returns the attributable risk as formatted string. */
std::string & AttributableRiskAsString(double ar, std::string& s) {
    std::stringstream ss;
    if (ar >= 0.001) {
        ss << getRoundAsString(ar * 1000.0, s, 1, true).c_str() << " per 1,000";
    } else {
        ss << getRoundAsString(ar * 1000000.0, s, 1, true).c_str() << " per 1,000,000";
    }
    s = ss.str().c_str();
    return s;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Calculates the attributable risk per person for cut. */
double CutStructure::getAttributableRisk(const ScanRunner& scanner) const {
    return getAttributableRiskFor(scanner, _ID, _C, _N, _start_idx, _end_idx);
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
    return getExcessCasesFor(scanner, _ID, _C, _N, _start_idx, _end_idx);
}

/** Returns cut's expected count. See user guide for formula explanation. */
double CutStructure::getExpected(const ScanRunner& scanner) const {
    return getExpectedFor(scanner, _ID, _C, _N, _start_idx, _end_idx);
}

/* Returns nodes parent(s) as csv list. */
std::string& CutStructure::getParentIndentifiers(const ScanRunner& scanner, std::string& parents) const {
    std::stringstream buffer;
    const NodeStructure * node = scanner.getNodes().at(getID());
    for (auto itr = node->getParents().begin(); itr != node->getParents().end(); ++itr)
        buffer << (itr != node->getParents().begin() ? "," : "") << (*itr)->getIdentifier();
    parents = buffer.str();
    return parents;
}

/** Returns cut's relative risk. See user guide for formula explanation. */
double CutStructure::getRelativeRisk(const ScanRunner& scanner) const {
    return getRelativeRiskFor(scanner, _ID, _C, _N, _start_idx, _end_idx);
}

////////////////////////// SequentialStatistic ///////////////////////////////

const char * SequentialStatistic::_file_suffix = "_sequential";
const char * SequentialStatistic::_accumulated_case_ext = ".casedata";
const char * SequentialStatistic::_accumulated_control_ext = ".controldata";
const char * SequentialStatistic::_accumulated_sim_ext = ".simulationdata";
const char * SequentialStatistic::_settings_ext = ".xml";

double SequentialStatistic::getAlphaSpentToDate(const std::string &output_filename) {
    std::string filename, buffer;
    getDerivedFilename(output_filename, _file_suffix, _settings_ext, filename);

    using boost::property_tree::ptree;
    ptree pt;

    if (!validateFileAccess(filename))
        return 0.0;

    read_xml(filename, pt);

    // Read alpha spendings from last looks.
    buffer = pt.get<std::string>("accumulation.alpha-spending", "0.0");
    std::vector<double> spending;
    csv_string_to_typelist<double>(buffer.c_str(), spending);
    return std::accumulate(spending.begin(), spending.end(), 0.0);
}

SequentialStatistic::SequentialStatistic(const Parameters& parameters, const ScanRunner & scanner) : _parameters(parameters), _scanner(scanner) {
    std::string buffer1, buffer2;
    // Expected filenames are derived from output filename.
    getDerivedFilename(_parameters.getOutputFileName(), _file_suffix, _accumulated_case_ext, _counts_filename);
    getDerivedFilename(_parameters.getOutputFileName(), _file_suffix, _accumulated_control_ext, _controls_filename);
    getDerivedFilename(_parameters.getOutputFileName(), _file_suffix, _accumulated_sim_ext, _simulations_filename);
    GetTemporaryFilename(_write_simulations_filename);
    GetTemporaryFilename(_write_llr_filename);
    getDerivedFilename(_parameters.getOutputFileName(), _file_suffix, _settings_ext, _settings_filename);
    // If case file doesn't exist yet, this is the first look of sequential statistic.
    _look_idx = validateFileAccess(_counts_filename) == true ? 2 : 1; // store look count in settings?
    parameters.setCurrentLook(_look_idx);
    // Get alpha spending for this look via user parameter setting.
    _alpha_spending = _parameters.getSequentialAlphaSpending();
    if (!isFirstLook()) {
        readSettings(_settings_filename);
        parameters.setCurrentLook(_look_idx);
        // Read calculate the look index.
        _alpha_spending += _statistic_parameters.getSequentialAlphaSpending();
        if (_parameters.getNumReplicationsRequested() != _statistic_parameters.getNumReplicationsRequested())
            throw resolvable_error("Error: Number of replications requested (%u) does match setting in previous sequential scan (%u).",
                                   _parameters.getNumReplicationsRequested(), 
                                   _statistic_parameters.getNumReplicationsRequested());
        if (_parameters.getProbabilityRatio() != _statistic_parameters.getProbabilityRatio())
            throw resolvable_error("Error: Event probability requested (%s) does match setting in previous sequential scan (%s).",
                                    AbtractParameterFileAccess::AsString(buffer1, _parameters.getProbabilityRatio()).c_str(),
                                    AbtractParameterFileAccess::AsString(buffer2, _statistic_parameters.getProbabilityRatio()).c_str());
        if (_parameters.getRestrictTreeLevels() != _statistic_parameters.getRestrictTreeLevels())
            throw resolvable_error("Error: Restricted tree levels (requested=%s) does match setting in previous sequential scan (requested=%s).",
                                   (_parameters.getRestrictTreeLevels() ? "y" : "n"),
                                   (_statistic_parameters.getRestrictTreeLevels() ? "y" : "n"));
        if (_parameters.getRestrictTreeLevels() && _parameters.getRestrictedTreeLevels() != _statistic_parameters.getRestrictedTreeLevels()) {
            typelist_csv_string<unsigned int>(_parameters.getRestrictedTreeLevels(), buffer1);
            typelist_csv_string<unsigned int>(_statistic_parameters.getRestrictedTreeLevels(), buffer2);
            throw resolvable_error("Error: Restricted tree levels (%s) does match setting in previous sequential scan (%s).", buffer1.c_str(), buffer2.c_str());
        }
        if (_parameters.getSequentialAlphaOverall() != _statistic_parameters.getSequentialAlphaOverall())
            throw resolvable_error("Error: Alpha overall specified for sequential analysis (%s) does match setting in previous alpha overall (%s).",
                AbtractParameterFileAccess::AsString(buffer1, _parameters.getSequentialAlphaOverall()).c_str(),
                AbtractParameterFileAccess::AsString(buffer2, _statistic_parameters.getSequentialAlphaOverall()).c_str());
        if (getTreeHash(buffer1) != _tree_hash)
            throw resolvable_error("Error: The tree file and/or cut file are not the same as prior sequential scan(s). The tree structure must remain the same for each look.");

        // Calculate the size of _llr_sims based upon this looks alpha spending.
        for (boost::dynamic_bitset<>::size_type i = 0; i < _alpha_simulations.size(); ++i) {
            // Test whether simulation was marked in last look as being within previous alpha spending.
            if (_alpha_simulations.test(i))
                // Once a simulation is marked, it remains marked. So add to list with max double as LLR.
                _llr_sims.push_back(std::make_pair(std::numeric_limits<double>::max(), i + 1));
        }
    }
    _alpha_simulations.resize(_parameters.getNumReplicationsRequested());
    size_t alpha_spending_size = static_cast<size_t>(ceil(static_cast<double>(_parameters.getNumReplicationsRequested() + 1) * _alpha_spending));
    _llr_sims.resize(alpha_spending_size, std::make_pair(0,0));
    _alpha_spendings.push_back(_parameters.getSequentialAlphaSpending());
}

bool SequentialStatistic::addSimulationLLR(double llr, unsigned int simIdx) {
    // Skip this simulation if already marked in previous look. This llr is already in _llr_sims but assigned as max double.
    if (isMarkedSimulation(simIdx))
        return true;

    // Attempt to insert current simulation llr into top ranking while maintaining the alpha spending limit.
    llr_sim_t add_llr_sim(llr, simIdx);
    llr_sim_container_t::iterator itr = std::upper_bound(_llr_sims.begin(), _llr_sims.end(), add_llr_sim, compare_llr_sim_t());
    if (itr != _llr_sims.end()) {
        _llr_sims.insert(itr, add_llr_sim);
        _llr_sims.pop_back();
        return true;
    } return false;
}

/* Creates a hash of the structure -- does not include data. */
std::string & SequentialStatistic::getTreeHash(std::string& treehash) const {
    md5 hash;
    md5::digest_type digest;
    std::stringstream identifiers;
    std::string buffer;

    ScanRunner::NodeStructureContainer_t::const_iterator itr = _scanner.getNodes().begin(), itrend = _scanner.getNodes().end();
    for (; itr != itrend; ++itr) {
        identifiers.str("");
        const NodeStructure& node = *(*itr);
        identifiers << "n:" <<node.getIdentifier() << ":c" << node.getCutType();
        for (size_t i = 0; i < node.getChildren().size(); ++i) {
            const NodeStructure& child = *(node.getChildren()[i]);
            identifiers << "n:" << child.getIdentifier() << ":c" << child.getCutType();
        }
        buffer = identifiers.str();
        lowerString(buffer);
        hash.process_bytes(buffer.data(), buffer.size());
    }
    hash.get_digest(digest);    
    treehash = toString(digest);
    return treehash;
}

void SequentialStatistic::readSettings(const std::string &filename) {
    using boost::property_tree::ptree;
    ptree pt;
    std::string buffer;

    // Read configuration file into tree.
    read_xml(filename, pt);

    // Read parameters section and store in _statistic_parameters class variable.
    _statistic_parameters.setNumReplications(static_cast<unsigned int>(pt.get<unsigned int>("parameters.replications", _parameters.getNumReplicationsRequested())));
    _statistic_parameters.setProbabilityRatio(Parameters::ratio_t(pt.get<unsigned int>("parameters.event-probability-numerator", _parameters.getProbabilityRatio().first),
                                                                  pt.get<unsigned int>("parameters.event-probability-denominator", _parameters.getProbabilityRatio().second)));
    _statistic_parameters.setRestrictTreeLevels(pt.get<bool>("parameters.restrict-tree-levels", false));
    buffer = pt.get<std::string>("parameters.restricted-tree-levels", "");
    Parameters::RestrictTreeLevels_t restricted_tree_levels;
    csv_string_to_typelist<unsigned int>(buffer.c_str(), restricted_tree_levels);
    _statistic_parameters.setRestrictedTreeLevels(restricted_tree_levels);
    _statistic_parameters.setSequentialAlphaOverall(static_cast<double>(pt.get<double>("parameters.sequential-alpha-overall", _parameters.getSequentialAlphaOverall())));
    _tree_hash = pt.get<std::string>("parameters.tree-hash", "");

    // Read alpha spendings from last looks.
    buffer = pt.get<std::string>("accumulation.alpha-spending", "0.0");
    csv_string_to_typelist<double>(buffer.c_str(), _alpha_spendings);
    _statistic_parameters.setSequentialAlphaSpending(std::accumulate(_alpha_spendings.begin(), _alpha_spendings.end(), 0.0));

    // better determination of look iteration through alpha spending collection
    _look_idx = _alpha_spendings.size() + 1;

    // Read collection of simulation indexes that were in alpha spending from previous looks and store in class variable.
    std::vector<unsigned int> indexes;
    buffer = pt.get<std::string>("accumulation.alpha-simulations", "");
    csv_string_to_typelist<unsigned int>(buffer.c_str(), indexes);
    _alpha_simulations.resize(_parameters.getNumReplicationsRequested());
    for (std::vector<unsigned int>::const_iterator itr = indexes.begin(); itr != indexes.end(); ++itr)
        _alpha_simulations.set(*itr - 1);

    // Read collection of cuts that signalled in previous looks and store in class variable.
    ptree & cuts = pt.get_child("accumulation.signalled-cuts");
    for (ptree::iterator it = cuts.begin(); it != cuts.end(); ++it) {
        ScanRunner::Index_t index = _scanner.getNodeIndex(it->first);
        if (!index.first)
            throw resolvable_error("Unknown node identifier in sequential configuration file: '%s'", it->first.c_str());
        _cuts_signaled.insert(std::make_pair(index.second, it->second.get_value<unsigned int>()));
    }
}

void SequentialStatistic::writeSettings(const std::string &filename) {
    using boost::property_tree::ptree;
    //using boost::property_tree::xml_writer_settings;
    ptree pt;
    std::string buffer;
    std::stringstream streambuffer;

    // Write parameter settings.
    pt.put("parameters.replications", _parameters.getNumReplicationsRequested());
    pt.put("parameters.event-probability-numerator", _parameters.getProbabilityRatio().first);
    pt.put("parameters.event-probability-denominator", _parameters.getProbabilityRatio().second);
    pt.put("parameters.restrict-tree-levels", _parameters.getRestrictTreeLevels());
    typelist_csv_string<unsigned int>(_parameters.getRestrictedTreeLevels(), buffer);
    pt.put("parameters.restricted-tree-levels", buffer);
    streambuffer << _parameters.getSequentialAlphaOverall();
    pt.put("parameters.sequential-alpha-overall", streambuffer.str().c_str());
    if (_tree_hash.size() == 0) getTreeHash(_tree_hash);
    pt.put("parameters.tree-hash", _tree_hash);

    // Write alpha spending for all looks.
    typelist_csv_string<double>(_alpha_spendings, buffer);
    pt.put("accumulation.alpha-spending", buffer);

    // Write simulation indexes that were in alpha spending for this and previous looks.
    std::vector<unsigned int> indexes;
    for (size_t t = 0; t < _alpha_simulations.size(); ++t) if (_alpha_simulations.test(t)) indexes.push_back(t + 1);
    typelist_csv_string<unsigned int>(indexes, buffer);
    pt.put("accumulation.alpha-simulations", buffer);

    // Write collection of cuts that signalled in this and previous looks.
    ptree signalled_cuts_node;
    for (signalled_cuts_container_t::iterator itr=_cuts_signaled.begin(); itr != _cuts_signaled.end(); ++itr) {
        buffer = _scanner.getNodes().at(itr->first)->getIdentifier().c_str();
        signalled_cuts_node.put(buffer, itr->second);
    }
    pt.add_child("accumulation.signalled-cuts", signalled_cuts_node);

    // Write to xml file.
    write_xml(filename, pt, std::locale(), boost::property_tree::xml_parser::xml_writer_settings<boost::property_tree::ptree::key_type>('\t', 1));
}

void SequentialStatistic::write(const std::string &casefilename, const std::string &controlfilename) {
    _alpha_simulations.reset();
    for (llr_sim_container_t::iterator itr=_llr_sims.begin(); itr != _llr_sims.end(); ++itr) {
        if (itr->second == 0)
            break;
        _alpha_simulations.set(itr->second - 1);
    }

    writeSettings(_settings_filename);
    // add case file to case accumulation.
    std::ifstream latest_cases(casefilename.c_str(), std::ios_base::binary);
    std::ofstream accumulated_cases(_counts_filename.c_str(), std::ios_base::app | std::ios_base::binary);
    accumulated_cases << latest_cases.rdbuf();
    // add control file to control accumulation.
    std::ifstream latest_controls(controlfilename.c_str(), std::ios_base::binary);
    std::ofstream accumulated_controls(_controls_filename.c_str(), std::ios_base::app | std::ios_base::binary);
    accumulated_controls << latest_controls.rdbuf();
    // Overwrite simulations file.
    std::ifstream latest_simulations(_write_simulations_filename.c_str(), std::ios_base::binary);
    std::ofstream accumulated_simulations(_simulations_filename.c_str(), std::ios_base::trunc | std::ios_base::binary);
    accumulated_simulations << latest_simulations.rdbuf();
    accumulated_simulations.close();
    latest_simulations.close();
    remove(_write_simulations_filename.c_str());
}

////////////////////////// ScanRunner ////////////////////////////////////////

/** class constructor */
ScanRunner::ScanRunner(const Parameters& parameters, BasePrint& print) : 
    _parameters(parameters), _print(print), _TotalC(0), _TotalControls(0), _TotalN(0),_has_multi_parent_nodes(false), _censored_data(false), _num_censored_cases(0), _avg_censor_time(0), _num_cases_excluded(0) {
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
    // Potentially force censored execution.
    _censored_data = parameters.isForcedCensoredAlgorithm();
}

boost::shared_ptr<AbstractWindowLength> ScanRunner::getNewWindowLength() const {
    if (isCensoredData() && _parameters.isApplyingRiskWindowRestrictionCensored())
        return boost::shared_ptr<AbstractWindowLength>(new CensoredRiskPercentageWindowLength(_parameters, 
                                                                                              static_cast<int>(_parameters.getMinimumWindowLength()) - 1, 
                                                                                              static_cast<int>(_parameters.getMaximumWindowInTimeUnits()) - 1, 
                                                                                              _zero_translation_additive,
                                                                                              _parameters.getRiskWindowAltCensorDenominator()
                                                                                             ));
    if (_parameters.isApplyingRiskWindowRestriction())
        return boost::shared_ptr<AbstractWindowLength>(new RiskPercentageWindowLength(_parameters, static_cast<int>(_parameters.getMinimumWindowLength()) - 1, static_cast<int>(_parameters.getMaximumWindowInTimeUnits()) - 1, _zero_translation_additive));
    return boost::shared_ptr<AbstractWindowLength>(new WindowLength(_parameters, static_cast<int>(_parameters.getMinimumWindowLength()) - 1, static_cast<int>(_parameters.getMaximumWindowInTimeUnits()) - 1));
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

    destinationNode.setMinCensoredBr(std::min(sourceNode.getMinCensoredBr(), destinationNode.getMinCensoredBr()));

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
        unsigned int level = node.getLevel();
        if (_tree_statistics->_nodes_per_level.find(level) == _tree_statistics->_nodes_per_level.end())
            _tree_statistics->_nodes_per_level.insert(std::make_pair(level, static_cast<unsigned int>(1)));
        else
            _tree_statistics->_nodes_per_level[level] += static_cast<unsigned int>(1);
    }
    return *_tree_statistics.get();
}

/* Returns true if NodeStructure is evaluated in scanning processing. */
bool ScanRunner::isEvaluated(const NodeStructure& node) const {
    // If node does not have cases in branch, it is not evaluated.
    if (node.getBrC() <= 1) return false;
    if (_parameters.getScanType() != Parameters::TIMEONLY && _parameters.getRestrictTreeLevels())
        return std::find(_parameters.getRestrictedTreeLevels().begin(), _parameters.getRestrictedTreeLevels().end(), node.getLevel()) == _parameters.getRestrictedTreeLevels().end();
    return true;
}

/** Read the relative risks file
    -- unlike other input files of system, records read from relative risks
       file are applied directly to the measure structure, just post calculation
       of measure and prior to temporal adjustments and making cumulative. */
bool ScanRunner::readRelativeRisksAdjustments(const std::string& srcfilename, RiskAdjustmentsContainer_t& rrAdjustments, bool consolidate) {
    _print.Printf("Reading alternative hypothesis file ...\n", BasePrint::P_STDOUT);

    bool bValid=true, bEmpty=true;
    ScanRunner::Index_t nodeIndex;
    const long nodeIdIdx=0, 
        uAdjustmentIndex = _parameters.getScanType() != Parameters::TIMEONLY ? 1 : 0, 
        startidx = _parameters.getScanType() != Parameters::TIMEONLY ? 2 : 1, 
        endidx = _parameters.getScanType() != Parameters::TIMEONLY ? 3 : 2;
    boost::dynamic_bitset<> nodeSet;
    std::auto_ptr<DataSource> dataSource(DataSource::getNewDataSourceObject(srcfilename, _parameters.getInputSource(Parameters::POWER_EVALUATIONS_FILE)));
    bool testMultipleNodeRecords(_parameters.getModelType() == Parameters::BERNOULLI_TREE);
    std::string nodeId("all"), filename("alternative hypothesis");
    std::string time_columnname(_parameters.getDatePrecisionType() == DataTimeRange::GENERIC ? "day since incidence" : "occurance date");

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
        if (_parameters.getScanType() != Parameters::TIMEONLY) {
            nodeId = dataSource->getValueAt(nodeIdIdx);
            if (lowerString(nodeId) != "all") {
                nodeIndex = getNodeIndex(dataSource->getValueAt(nodeIdIdx));
                if (!nodeIndex.first) {
                    bValid = false;
                    _print.Printf("Error: Record %ld in alternative hypothesis file references unknown node (%s).\n", BasePrint::P_READERROR, dataSource->getCurrentRecordIndex(), dataSource->getValueAt(nodeIdIdx).c_str());
                    continue;
                }
            }
        }
        //read alternative hypothesis value
        double alternative_hypothesis;
        if (dataSource->getValueAt(uAdjustmentIndex).size() < 1) {
            _print.Printf("Error: Record %ld of alternative hypothesis file missing %s.\n", BasePrint::P_READERROR, dataSource->getCurrentRecordIndex(),
                          _parameters.getModelType() == Parameters::BERNOULLI_TREE ? "event probability" : "relative risk");
            bValid = false;
            continue;
        }
        if (_parameters.getModelType() == Parameters::BERNOULLI_TREE) {
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
            if (!readDateColumn(*dataSource, startidx, start, filename, time_columnname)) {
                bValid = false;
                continue;
            }
            if (_parameters.getDataTimeRangeSet().getDataTimeRangeIndex(start).first == false) {
                bValid = false;
                _print.Printf("Error: Record %ld in alternative hypothesis file references an invalid start range index for node '%s'.\n"
                              "Value must be within the defined data time range.", BasePrint::P_READERROR, dataSource->getCurrentRecordIndex(), dataSource->getValueAt(nodeIdIdx).c_str());
                continue;
            }
            if (!readDateColumn(*dataSource, endidx, end, filename, time_columnname)) {
                bValid = false;
                continue;
            }
            if (_parameters.getDataTimeRangeSet().getDataTimeRangeIndex(end).first == false) {
                bValid = false;
                _print.Printf("Error: Record %ld in alternative hypothesis file references an invalid end range index for node '%s'.\n"
                              "Value must be within the defined data time range.", BasePrint::P_READERROR, dataSource->getCurrentRecordIndex(), dataSource->getValueAt(nodeIdIdx).c_str());
                continue;
            }
            // translate index to zero index
            start = start + _zero_translation_additive;
            end = end + _zero_translation_additive;
        }

        size_t iNumWords = dataSource->getNumValues();
        if (iNumWords > static_cast<size_t>(_parameters.getScanType() == Parameters::TREEONLY ? 2 : 4)) {
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

/* Reads count, control and population data from passed file. */
bool ScanRunner::readCounts(const std::string& srcfilename, bool sequence_new_data) {
    /* Special Case: We won't actually read the counts file in this situation but define the total from user specificied value. */
    if (_parameters.getScanType() == Parameters::TIMEONLY && _parameters.getConditionalType() == Parameters::TOTALCASES && 
        _parameters.getPerformPowerEvaluations() && _parameters.getPowerEvaluationType() == Parameters::PE_ONLY_SPECIFIED_CASES) {
        _Nodes.front()->refIntC_C().front() = _parameters.getPowerEvaluationTotalCases();
        return true;
    }

    if (!sequence_new_data)
        _print.Printf("Reading count data from prior analyses ...\n", BasePrint::P_STDOUT);
    else
        _print.Printf("Reading count file ...\n", BasePrint::P_STDOUT);
    bool readSuccess=true;
    double population = 0;
    int count = 0, controls = 0, daysSinceIncidence = 0, censortime = 0;
    long identifierIdx = _parameters.getScanType() == Parameters::TIMEONLY ? -1 : 0;
    long countIdx = _parameters.getScanType() == Parameters::TIMEONLY ? 0 : 1;
    DataTimeRange::index_t censortimetotal = 0;
    std::string filename("count"), time_columnname(_parameters.getDatePrecisionType() == DataTimeRange::GENERIC ? "day since incidence" : "occurance date");
    std::auto_ptr<DataSource> dataSource(DataSource::getNewDataSourceObject(srcfilename, _parameters.getInputSource(Parameters::COUNT_FILE)));

    /* In TreeScan version 2.0, we switched to a separate control data file and removed the control column from counts file.
       But to keep backwards compatibility, expect controls in count file if user has not specified a control file with a 
       tree-only scan (not including sequential). */
    bool bernoulliExpectingControl = _parameters.getControlFileName().empty();

    // Determine number of expected columns based on user settings.
    std::string col_id("<identifier>"), col_count("<count>"), col_pop("<population>"), col_controls("<controls>"), col_time("<time>");
    std::vector<std::string> expectedColumns;
    if (_parameters.getModelType() == Parameters::POISSON)
        expectedColumns = boost::assign::list_of (col_id) (col_count) (col_pop);
    else if (_parameters.getModelType() == Parameters::BERNOULLI_TREE) {
        expectedColumns = boost::assign::list_of (col_id) (col_count);
        if (bernoulliExpectingControl) expectedColumns.push_back(col_controls);
    } else if (_parameters.getModelType() == Parameters::BERNOULLI_TIME) {
        if (_parameters.getScanType() == Parameters::TREETIME) expectedColumns = boost::assign::list_of (col_id) (col_count) (col_time);
        else expectedColumns = boost::assign::list_of (col_count) (col_time);
    } else {
        if (_parameters.getScanType() != Parameters::TIMEONLY) expectedColumns = boost::assign::list_of (col_id) (col_count) (col_time);
        else expectedColumns = boost::assign::list_of (col_count) (col_time);
    }

    /* Iinitialize bitset which is used to track data time range days with cases (or controls). */
    _caselessWindows.resize(Parameters::isTemporalScanType(_parameters.getScanType()) ? _parameters.getDataTimeRangeSet().getTotalDaysAcrossRangeSets() : 1);
    /* Read records of control file, verifying the expected columns and data type then adding to data structures. */
    while (dataSource->readRecord()) {
        // Note: The uniform model has an optional column - censor time.
        if (!(dataSource->getNumValues() == expectedColumns.size() || 
            (_parameters.getModelType() == Parameters::UNIFORM && dataSource->getNumValues() == (expectedColumns.size() + 1)))) {
            readSuccess = false;
            std::string buffer;
            typelist_csv_string<std::string>(expectedColumns, buffer);
            _print.Printf(
                "Error: Record %ld in count file %s. Expecting %s%s but found %ld value%s.\n",
                BasePrint::P_READERROR, dataSource->getCurrentRecordIndex(),
                (static_cast<int>(dataSource->getNumValues()) > expectedColumns.size()) ? "has extra data" : "is missing data",
                buffer.c_str(),
                (_parameters.getModelType() == Parameters::UNIFORM ? ", <censor time>" : ""),
                dataSource->getNumValues(), 
                (dataSource->getNumValues() == 1 ? "" : "s")
            );
            continue;
        }
        /* Read and verifiy identifier column - if not time-only scan type. */
        ScanRunner::Index_t index(true, 0);
        if (_parameters.getScanType() != Parameters::TIMEONLY) {
            index = getNodeIndex(dataSource->getValueAt(identifierIdx));
            if (!index.first) {
                readSuccess = false;
                _print.Printf(
                    "Error: Record %ld in count file references unknown node (%s).\n", 
                    BasePrint::P_READERROR, 
                    dataSource->getCurrentRecordIndex(), 
                    dataSource->getValueAt(identifierIdx).c_str()
                );
                continue;
            }
        }
        /* Read and verify number of cases column. */
        if  (!string_to_numeric_type<int>(dataSource->getValueAt(countIdx).c_str(), count) || count < 0) {
            readSuccess = false;
            _print.Printf(
                "Error: Record %ld in count file references an invalid number of cases.\n"
                "       The number of cases must be an integer value greater than or equal to zero.\n", 
                BasePrint::P_READERROR, 
                dataSource->getCurrentRecordIndex()
            );
            continue;
        }
        // Now read model specific columns from data source.
        NodeStructure * node = _Nodes[index.second];
        if (_parameters.getModelType() == Parameters::POISSON) {
            node->refIntC_C().front() += count;
            if  (!string_to_numeric_type<double>(dataSource->getValueAt(expectedColumns.size() - 1).c_str(), population) || population < 0) {
                readSuccess = false;
                _print.Printf(
                    "Error: Record %ld in count file references an invalid population.\n"
                    "       The population must be a numeric value greater than or equal to zero.\n", 
                    BasePrint::P_READERROR, 
                    dataSource->getCurrentRecordIndex()
                );
                continue;
            }
            node->refIntN_C().front() += population;
        } else if (_parameters.getModelType() == Parameters::BERNOULLI_TREE) {
            node->refIntC_C().front() += count;
            node->refIntN_C().front() += count;
            if (sequence_new_data) node->refIntN_C_Seq_New().front() += count;
            /* Skip to the next record in data source if not expecting controls column in this file. */
            if (!bernoulliExpectingControl)	continue;
            /* Otherwise read and verify data from controls column. */
            if  (!string_to_numeric_type<int>(dataSource->getValueAt(expectedColumns.size() - 1).c_str(), controls) || controls < 0) {
                readSuccess = false;
                _print.Printf(
                    "Error: Record %ld in count file references an invalid number of controls.\n"
                    "       The controls must be an integer value greater than or equal to zero.\n", 
                    BasePrint::P_READERROR, 
                    dataSource->getCurrentRecordIndex()
                );
                continue;
            }
            node->refIntN_C().front() += controls;
        } else if (_parameters.getModelType() == Parameters::BERNOULLI_TIME) {
            /* First read and verify the days since incidance column - we might not be reading controls from this file. */
            if (!readDateColumn(*dataSource, expectedColumns.size() - 1, daysSinceIncidence, filename, time_columnname)) {
                readSuccess = false;
                continue;
            }
            DataTimeRangeSet::rangeset_index_t rangeIdx = _parameters.getDataTimeRangeSet().getDataTimeRangeIndex(daysSinceIncidence);
            if (rangeIdx.first == false) {
                readSuccess = false;
                _print.Printf(
                    "Error: Record %ld in count file references an invalid '%s' value.\n"
                    "       The specified value is not within any of the data time ranges you have defined.",
                    BasePrint::P_READERROR, 
                    dataSource->getCurrentRecordIndex(),
                    time_columnname.c_str()
                );
                continue;
            }
            node->refIntC_C()[daysSinceIncidence + _zero_translation_additive] += count;
            node->refIntN_C()[daysSinceIncidence + _zero_translation_additive] += count;
            if (count) _caselessWindows.set(daysSinceIncidence + _zero_translation_additive);
        } else if (Parameters::isTemporalScanType(_parameters.getScanType())) {
            /* Other temporal scan type. */
            if (!readDateColumn(*dataSource, expectedColumns.size() - 1, daysSinceIncidence, filename, time_columnname)) {
                readSuccess = false;
                continue;
            }
            // check that the 'daysSinceIncidence' is within a defined data time range
            DataTimeRangeSet::rangeset_index_t rangeIdx = _parameters.getDataTimeRangeSet().getDataTimeRangeIndex(daysSinceIncidence);
            if (rangeIdx.first == false) {
                readSuccess = false;
                _print.Printf(
                    "Error: Record %ld in count file references an invalid '%s' value.\n"
                    "       The specified value is not within any of the data time ranges you have defined.",
                    BasePrint::P_READERROR, 
                    dataSource->getCurrentRecordIndex(),
                    time_columnname.c_str()
                );
                continue;
            }
            // If applying exclusion time range and this event is in one of the exclusion ranges, skip this record (TreeTime conditioned on Node/Time only).
            if (_parameters.isApplyingExclusionTimeRanges()) {
                DataTimeRangeSet::rangeset_index_t rangeIdxExclusion = _parameters.getExclusionTimeRangeSet().getDataTimeRangeIndex(daysSinceIncidence);
                if (rangeIdxExclusion.first == true) {
                    ++_num_cases_excluded;
                    continue;
                }
            }
            // If the probably model is uniform, there is possibly another column - censored time.
            if (_parameters.getModelType() == Parameters::UNIFORM) {
                // If this column is missing or blank, just ignore the column in this record.
                if ((dataSource->getNumValues() == expectedColumns.size() + 1) && dataSource->getValueAt(expectedColumns.size()).size() != 0) {
                    if (!readDateColumn(*dataSource, expectedColumns.size(), censortime, filename, "censoring time")) {
                        readSuccess = false;
                        continue;
                    }
                    DataTimeRangeSet::rangeset_index_t rangeIdx = _parameters.getDataTimeRangeSet().getDataTimeRangeIndex(censortime);
                    if (rangeIdx.first == false) {
                        readSuccess = false;
                        _print.Printf(
                            "Error: Record %ld in count file references an invalid 'censoring time' value.\n"
                            "       The specified value is not within any of the data time ranges you have defined.",
                            BasePrint::P_READERROR, 
                            dataSource->getCurrentRecordIndex()
                        );
                        continue;
                    }
                    if (censortime < static_cast<int>(_parameters.getMinimumCensorTime())) {
                        _print.Printf(
                            "Warning: Record %ld in count file references an invalid 'censoring time' value.\n"
                            "         The censoring time is less than user specified minimum of %u. This observation will be ignored.",
                            BasePrint::P_WARNING, 
                            dataSource->getCurrentRecordIndex(), 
                            _parameters.getMinimumCensorTime()
                        );
                        continue;
                    }
                    DataTimeRange minmax = _parameters.getDataTimeRangeSet().getMinMax();
                    if (censortime == minmax.getStart()) {
                        _print.Printf(
                            "Warning: Record %ld in count file references an invalid 'censoring time' value.\n"
                            "         The censoring time is not allowed to equal the data time range start. This observation will be ignored.",
                            BasePrint::P_WARNING, 
                            dataSource->getCurrentRecordIndex()
                        );
                        continue;
                    }
                    DataTimeRange::index_t positive_range_days = minmax.numDaysInPositiveRange();
                    if (censortime < positive_range_days * (static_cast<double>(_parameters.getMinimumCensorPercentage()) / 100.0)) {
                        _print.Printf(
                            "Warning: Record %ld in count file references an invalid 'censoring time' value.\n"
                            "         The censoring time is less than the %d%% of the positive data time range - which is %d units long. This observation will be ignored.",
                            BasePrint::P_WARNING, 
                            dataSource->getCurrentRecordIndex(), 
                            _parameters.getMinimumCensorPercentage(), 
                            positive_range_days
                        );
                        continue;
                    }
                    if (censortime < daysSinceIncidence) {
                        _print.Printf(
                            "Warning: Record %ld in count file references an invalid 'censoring time' value.\n"
                            "         The censoring time is less than the 'day since incidence' value. This observation will be ignored.",
                            BasePrint::P_WARNING, 
                            dataSource->getCurrentRecordIndex()
                        );
                        continue;
                    }
                    // Skip this record now if the number of cases is zero -- otherwise we might falsely indicate that this data set is censoring data.
                    if (count == 0) continue;
                    // If the censor time equals the data time range end, then this record isn't really censoring.
                    if (censortime < minmax.getEnd()) {
                        // Now that we know there is censored data, check parameter settings are valid.
                        if (_parameters.isPerformingDayOfWeekAdjustment())
                            throw resolvable_error("Error: The day of week adjustment is not implemented for censored data.");
                        if (_parameters.getSequentialScan())
                            throw resolvable_error("Error: The sequential scan is not implemented for censored data.");
                        if (_parameters.getPerformPowerEvaluations())
                            throw resolvable_error("Error: The power evaluations option is not implemented for censored data.");
                        // Now mark the parameter settings to say that this data set has censored data.
                        _censored_data = true;
                        // Note the censored case in the censored data array for this node.
                        node->refIntC_Censored()[censortime + _zero_translation_additive] += count;
                        // Assign expected counts up to time of censoring.
                        for (size_t t = 0; t <= static_cast<size_t>(censortime + _zero_translation_additive); ++t)
                            node->refIntN_C()[t] += static_cast<double>(count) / static_cast<double>(censortime + _zero_translation_additive + 1);
                        // Keep track of totals for summary information.
                        censortimetotal += censortime * count;
                        _num_censored_cases += count;
                    }
                }
            }

            // Since we might have skipped adding this record for censored times equal to data time range start, wait until now to add cases.
            node->refIntC_C()[daysSinceIncidence + _zero_translation_additive] += count;
            _caselessWindows.set(daysSinceIncidence + _zero_translation_additive);

        } else throw prg_error("Unknown condition encountered: scan (%d), model (%d).", "readCounts()", _parameters.getScanType(), _parameters.getModelType());
    }

    /* Report to user if any days in the data time range do not contains cases (or controls). */
    if (readSuccess && Parameters::isTemporalScanType(_parameters.getScanType()) && !((_parameters.getModelType() == Parameters::BERNOULLI_TIME) && bernoulliExpectingControl == false)) {
        _caselessWindows.flip(); // flip so that windows without cases are on instead of off
        if (_caselessWindows.count() > 0) {
            std::string buffer;
            _print.Printf(
                "Warning: The following %s in the data time range do not have cases%s: %s\n",
                BasePrint::P_WARNING,
                (_parameters.getDatePrecisionType() == DataTimeRange::GENERIC ? "days" : "dates"),
                (_parameters.getModelType() == Parameters::BERNOULLI_TIME ? " or controls" : ""),
                getCaselessWindowsAsString(buffer).c_str()
            );
        }
    }

    /* Now that all case data has been read, calculate the average censor time for censored data. */
    if (_censored_data) _avg_censor_time = censortimetotal / std::max(1, _num_censored_cases);

    return readSuccess;
}

/* Reads input column from source based on user date precision. */
bool ScanRunner::readDateColumn(DataSource& source, size_t columnIdx, int& dateIdx, const std::string& file_name, const std::string& column_name) const {
    if (_parameters.getDatePrecisionType() == DataTimeRange::NONE)
        throw prg_error("readDateColumn() should nor be called with date precision of NONE.", "readControls()");

    DateStringParser::ParserStatus eStatus = DateStringParser(_parameters.getDatePrecisionType()).Parse(
        source.getValueAt(columnIdx).c_str(), dateIdx, _parameters.getDataTimeRangeSet().getDataTimeRangeSets().front().getDateStart()
    );
    switch (eStatus) {
        case  DateStringParser::OUT_OF_RANGE:
            _print.Printf(
                "Error: Record %ld in %s file references an invalid '%s' value.\n       The value precedes Data Time Range start date.\n",
                BasePrint::P_READERROR, source.getCurrentRecordIndex(), file_name.c_str(), column_name.c_str()
            ); return false;
        case  DateStringParser::LESSER_PRECISION:
            _print.Printf(
                "Error: Record %ld in %s file references an invalid '%s' value.\n       The value must match precision setting 'Time Precision' setting.\n",
                BasePrint::P_READERROR, source.getCurrentRecordIndex(), file_name.c_str(), column_name.c_str()
            ); return false;
        case  DateStringParser::INVALID_DATE:
            _print.Printf(
                "Error: Record %ld in %s file references an invalid '%s' value.\n       The value must be %s.\n",
                BasePrint::P_READERROR, source.getCurrentRecordIndex(), file_name.c_str(), column_name.c_str(),
                _parameters.getDatePrecisionType() == DataTimeRange::GENERIC ? "an integer" : "a date"
            ); return false;
        case DateStringParser::VALID_DATE:
        default: break;
    }
    return true;
}

/* Reads control data from passed file for Bernoulli models. 
    We currently have two ways for control data to be read from data files. Earlier versions of TreeScan had the case and control data is the
    count file only. With the addition of the Bernoulli Time model, it became apparent breaking cases and controls into two files made more sense.
    To keep backward compatability, we're maintaining both options. */
bool ScanRunner::readControls(const std::string& srcfilename, bool sequence_new_data) {
    if (!sequence_new_data)
        _print.Printf("Reading control data from prior analyses ...\n", BasePrint::P_STDOUT);
    else
        _print.Printf("Reading control file ...\n", BasePrint::P_STDOUT);
    bool readSuccess = true;
    std::string filename("control"), time_columnname(_parameters.getDatePrecisionType() == DataTimeRange::GENERIC ? "day since incidence" : "occurance date");
    std::auto_ptr<DataSource> dataSource(DataSource::getNewDataSourceObject(srcfilename, _parameters.getInputSource(Parameters::CONTROL_FILE)));
    int controls = 0, daysSinceIncidence = 0, expectedColumns = (_parameters.getScanType() == Parameters::TREETIME ? 3 : 2);
    long identifierIdx = _parameters.getScanType() == Parameters::TIMEONLY ? -1 : 0;
    long controlIdx = _parameters.getScanType() == Parameters::TIMEONLY ? 0 : 1;
    /* Read records of control file, verifying the expected columns and data type then adding to data structures. */
    while (dataSource->readRecord()) {
        if (!(dataSource->getNumValues() == expectedColumns)) {
            readSuccess = false;
            _print.Printf(
                "Error: Record %ld in control file %s. Expecting %s<controls>%s but found %ld value%s.\n",
                BasePrint::P_READERROR, dataSource->getCurrentRecordIndex(),
                (static_cast<int>(dataSource->getNumValues()) > expectedColumns) ? "has extra data" : "is missing data",
                (_parameters.getScanType() == Parameters::TIMEONLY ? "" : "<identifier>, "),
                (Parameters::isTemporalScanType(_parameters.getScanType()) ? ", <time>" : ""),
                dataSource->getNumValues(), 
                (dataSource->getNumValues() == 1 ? "" : "s")
            );
            continue;
        }
        ScanRunner::Index_t index(true, 0);
        if (_parameters.getScanType() != Parameters::TIMEONLY) {
            index = getNodeIndex(dataSource->getValueAt(identifierIdx));
            if (!index.first) {
                readSuccess = false;
                _print.Printf(
                    "Error: Record %ld in control file references unknown node (%s).\n", 
                    BasePrint::P_READERROR, 
                    dataSource->getCurrentRecordIndex(), 
                    dataSource->getValueAt(identifierIdx).c_str()
                );
                continue;
            }
        }
        if (!string_to_numeric_type<int>(dataSource->getValueAt(controlIdx).c_str(), controls) || controls < 0) {
            readSuccess = false;
            _print.Printf(
                "Error: Record %ld in control file references an invalid number of controls.\n"
                "       The controls must be an integer value greater than or equal to zero.\n",
                BasePrint::P_READERROR,
                dataSource->getCurrentRecordIndex()
            );
            continue;
        }
        /* Now read remainder of data as expected for specific Bernoulli model type.*/
        NodeStructure * node = _Nodes[index.second];
        if (_parameters.getModelType() == Parameters::BERNOULLI_TREE) {
            node->refIntN_C().front() += controls;
            if (sequence_new_data) node->refIntN_C_Seq_New().front() += controls;
        } else if (_parameters.getModelType() == Parameters::BERNOULLI_TIME) {
            if (!readDateColumn(*dataSource, controlIdx + 1, daysSinceIncidence, filename, time_columnname)) {
                readSuccess = false;
                continue;
            }
            /* Check that the 'daysSinceIncidence' is within a defined data time range. */
            DataTimeRangeSet::rangeset_index_t rangeIdx = _parameters.getDataTimeRangeSet().getDataTimeRangeIndex(daysSinceIncidence);
            if (rangeIdx.first == false) {
                readSuccess = false;
                _print.Printf(
                    "Error: Record %ld in control file references an invalid '%s' value.\n"
                    "The specified value is not within any of the data time ranges you have defined.",
                    BasePrint::P_READERROR, 
                    dataSource->getCurrentRecordIndex(),
                    time_columnname.c_str()
                );
                continue;
            }
            node->refIntN_C()[daysSinceIncidence + _zero_translation_additive] += controls;
            if (controls) _caselessWindows.set(daysSinceIncidence + _zero_translation_additive);
        } else throw prg_error("Unknown condition encountered: scan (%d), model (%d).", "readControls()", _parameters.getScanType(), _parameters.getModelType());
    }

    /* Report to user if any days in the data time range do not contains cases or controls. */
    _caselessWindows.flip(); // flip so that windows without cases/controls are on instead of off
    if (Parameters::isTemporalScanType(_parameters.getScanType()) && _caselessWindows.count() > 0) {
        std::string buffer;
        _print.Printf(
            "Warning: The following %s in the data time range do not have cases or controls: %s\n",
            BasePrint::P_WARNING, 
            (_parameters.getDatePrecisionType() == DataTimeRange::GENERIC ? "days" : "dates"),
            getCaselessWindowsAsString(buffer).c_str()
        );
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
                if (_parameters.getDatePrecisionType() == DataTimeRange::GENERIC)
                    buffer << (static_cast<int>(pS) - getZeroTranslationAdditive());
                else
                    buffer << _parameters.getDataTimeRangeSet().getDataTimeRangeSets().front().rangeIdxToGregorianString(
                        static_cast<int>(pS) - getZeroTranslationAdditive(), _parameters.getDatePrecisionType()
                    );
            } else {
                if (_parameters.getDatePrecisionType() == DataTimeRange::GENERIC)
                    buffer << (static_cast<int>(pS) - getZeroTranslationAdditive()) << " to " << (static_cast<int>(pE) - getZeroTranslationAdditive());
                else {
                    std::pair<std::string, std::string> rangeDates = _parameters.getDataTimeRangeSet().getDataTimeRangeSets().front().rangeToGregorianStrings(
                        static_cast<int>(pS) - getZeroTranslationAdditive(),
                        static_cast<int>(pE) - getZeroTranslationAdditive(),
                        _parameters.getDatePrecisionType()
                    );
                    buffer << rangeDates.first << " to " << rangeDates.second;
                }
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
                    // test that node does not reference itself as parent
                    if (node->getID() == static_cast<int>(index.second)) {
                        readSuccess = false;
                        _print.Printf("Error: Record %ld in tree file has node referencing self as parent (%s).\n", BasePrint::P_READERROR, dataSource->getCurrentRecordIndex(), identifier.c_str());
                        continue;
                    }
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
           (_parameters.getNumReplicationsRequested() == 0 && static_cast<double>(cut.getC()) > cut.getExpected(*this)) ||
           /* If Bernoulli sequential scan -- a cut reported in previous look is always reportable. */
           (_parameters.isSequentialScanBernoulli() && getSequentialStatistic().testCutSignaled(static_cast<size_t>(cut.getID())) != 0);
}

bool ScanRunner::reportablePValue(const CutStructure& cut) const {
    return _parameters.getNumReplicationsRequested() > MIN_REPLICA_RPT_PVALUE && !_parameters.isSequentialScanBernoulli();
}

bool ScanRunner::reportableRecurrenceInterval(const CutStructure& cut) const {
    return _parameters.getIsProspectiveAnalysis() && _parameters.getNumReplicationsRequested() > MIN_REPLICA_RPT_PVALUE;
}

RecurrenceInterval_t ScanRunner::getRecurrenceInterval(const CutStructure& cut) const {
    //if (!parameters.GetIsProspectiveAnalysis())
    //   throw prg_error("GetRecurrenceInterval() called for non-prospective analysis.", "GetRecurrenceInterval()");

    //if (!reportableCut(cut))
    //    throw prg_error("Recurrence Interval cannot be obtained for this cut.", "getRecurrenceInterval()");

    size_t daysInDataTimeRange = Parameters::isTemporalScanType(_parameters.getScanType()) ? _parameters.getDataTimeRangeSet().getTotalDaysAcrossRangeSets() + 1 : 1;
    double p_value = (double)cut.getRank() / (_parameters.getNumReplicationsRequested() + 1);
    // Determine the number of units in occurrence per user selection.
    double dUnitsInOccurrence = std::max(static_cast<double>(_parameters.getProspectiveFrequency()) / p_value, 1.0);
    // Now calculate recurrance interval as years and days based on frequency.
    switch (_parameters.getProspectiveFrequencyType()) {
        case Parameters::YEARLY: return std::make_pair(dUnitsInOccurrence, std::max(dUnitsInOccurrence * AVERAGE_DAYS_IN_YEAR, 1.0));
        case Parameters::QUARTERLY: std::make_pair(dUnitsInOccurrence / 4.0, std::max((dUnitsInOccurrence / 4.0) * AVERAGE_DAYS_IN_YEAR, 1.0));
        case Parameters::MONTHLY: return std::make_pair(dUnitsInOccurrence / 12.0, std::max((dUnitsInOccurrence / 12.0) * AVERAGE_DAYS_IN_YEAR, 1.0));
        case Parameters::WEEKLY:  return std::make_pair(dUnitsInOccurrence / 52.0, std::max((dUnitsInOccurrence / 52.0) * AVERAGE_DAYS_IN_YEAR, 1.0));
        case Parameters::DAILY: return std::make_pair(dUnitsInOccurrence / AVERAGE_DAYS_IN_YEAR, std::max(dUnitsInOccurrence, 1.0));
        default: throw prg_error("Invalid enum '%d' for prospective analysis frequency type.", "getRecurrenceInterval()", _parameters.getProspectiveFrequencyType());
    }
}

/** Returns whether cut is significant. */
boost::logic::tribool ScanRunner::isSignificant(const CutStructure& cut) const {
    boost::logic::tribool significance(boost::logic::indeterminate);
    if (reportableRecurrenceInterval(cut)) {
        significance = isSignificant(getRecurrenceInterval(cut), _parameters);
    } else if (reportablePValue(cut)) {
        // p-value  less than 0.05 is significant
        significance = ((double)cut.getRank() / (double)(_parameters.getNumReplicationsRequested() + 1)) < 0.05;
    } //else {// If both recurrence interval and p-value are not reportable, so we do not have information to say whether this cluster is significant or not.}
    return significance;
}

bool ScanRunner::isSignificant(const RecurrenceInterval_t& ri, const Parameters& parameters) {
    const double SIGNIFICANCE_MULTIPLIER = 100.0; // I'm not sure how Martin picked this number.
    double frequency_length = static_cast<double>(parameters.getProspectiveFrequency()); // defaulted to one.
    switch (parameters.getProspectiveFrequencyType()) {
        case Parameters::YEARLY: return (ri.first > SIGNIFICANCE_MULTIPLIER * frequency_length);
        case Parameters::QUARTERLY: return (ri.first * 4.0 > SIGNIFICANCE_MULTIPLIER * frequency_length);
        case Parameters::MONTHLY: return (ri.first * 12.0 > SIGNIFICANCE_MULTIPLIER * frequency_length);
        case Parameters::WEEKLY: return (ri.first * 52.0 > SIGNIFICANCE_MULTIPLIER * frequency_length);
        case Parameters::DAILY: return (ri.second > std::max(365.0, SIGNIFICANCE_MULTIPLIER * frequency_length));
        default: throw prg_error("Invalid enum '%d' for prospective analysis frequency type.", "isSignificant()", parameters.getProspectiveFrequencyType());
    }
}

/* REPORT RESULTS */
bool ScanRunner::reportResults(time_t start, time_t end) {
    ResultsFileWriter resultsWriter(*this);

    // Assign the report order for each cut -- this allows use to sort in different ways during reporting, yet return to the
    // order reported in the main results file. 
    unsigned int i = 1;
    for (CutStructureContainer_t::iterator itr = _Cut.begin(); itr != _Cut.end(); ++itr, ++i)
        (*itr)->setReportOrder(i);

    // Create the main text output file.
    if (!resultsWriter.writeASCII(start, end))
        return false;

    if (_parameters.isGeneratingHtmlResults() || _parameters.isGeneratingTableResults()) {
        /* First widdle the cut list down to those that are reportable. */
        CutStructureContainer_t::iterator itr = _Cut.begin();
        for (; itr != _Cut.end(); ++itr) {
            if (!reportableCut(*(*itr))) {
                break; // Found first cut that was is not reportable. Remove cuts here and after.
            }
        }
        // Move non-reportable cuts to separate vector. We'll need to reference these trimmed cuts in a different way in the html output.
        if (itr != _Cut.end()) {
            CutStructureContainer_t::iterator itrTemp = itr;
            for (; itr != _Cut.end(); ++itr) {
                _trimmed_cuts.push_back(*itr);
                *itr = 0;
            }
            _Cut.erase(itrTemp, _Cut.end()); // Erase, not kill, the elements from this vector.
        }
        if (_parameters.getScanType() != Parameters::TIMEONLY) {
            // Sort by ancestry string and update cuts.
            std::sort(_Cut.begin(), _Cut.end(), CompareCutsByAncestoryString(*this));
            i = 1;
            for (CutStructureContainer_t::iterator itr = _Cut.begin(); itr != _Cut.end(); ++itr, ++i)
                (*itr)->setBranchOrder(i);
            // Now return to report order.
            std::sort(_Cut.begin(), _Cut.end(), CompareCutsByReportOrder());
        }
    }
    // write cuts to html file
    if (_parameters.isGeneratingHtmlResults()) {
        if (!resultsWriter.writeHTML(start, end))
            return false;
    }

    // generate temporal chart - if requested
    if (_parameters.getOutputTemporalGraphFile())
        TemporalChartGenerator(*this, this->_simVars).generateChart();

    // write cuts to csv file
    if (_parameters.isGeneratingTableResults()) {
        CutsRecordWriter cutsWriter(*this);
        bool reportCutCSV = true;
        if (_parameters.isSequentialScanPurelyTemporal())
            // This is a sequential scan and we haven't reached the maximum cases, we will report cuts.
            reportCutCSV &= !(static_cast<unsigned int>(getTotalC()) > _parameters.getSequentialMaximumSignal());
        if (_parameters.getPerformPowerEvaluations())
            // If this is a power evaluation with an analysis, we will report cuts.
            reportCutCSV &= _parameters.getPowerEvaluationType() == Parameters::PE_WITH_ANALYSIS;
        for (CutStructureContainer_t::iterator itr = _Cut.begin(); itr != _Cut.end() && reportCutCSV; ++itr)
            cutsWriter.write(*(*itr));
    }
    // write power evaluation results to separate file
    if (_parameters.getPerformPowerEvaluations()) {
        PowerEstimationRecordWriter(*this).write();
    }
    return true;
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

    // create SequentialStatistic object if Bernoulli sequential
    if (_parameters.isSequentialScanBernoulli()) {
        _sequential_statistic.reset(new SequentialStatistic(_parameters, *this));
        if (macro_less_than(_parameters.getSequentialAlphaOverall(), _sequential_statistic->getAlphaSpending(), DBL_CMP_TOLERANCE)) {
            std::stringstream buffer;
            double remaining = _parameters.getSequentialAlphaOverall() - (_sequential_statistic->getAlphaSpending() - _parameters.getSequentialAlphaSpending());
            if (macro_less_than(0.0, remaining, DBL_CMP_TOLERANCE)) {
                buffer << "\nThe overall alpha has only " << remaining << " remaining to be spent but user settings are requesting " << _parameters.getSequentialAlphaSpending() << " in current look.";
                throw resolvable_error(buffer.str().c_str());
            }
            buffer << "The alpha spending for sequential scan reached the specified alpha overall and the analysis is over.\n";
            _print.Printf(buffer.str().c_str(), BasePrint::P_STDOUT);
            return false;
        }
    }

    if (!readCounts(_parameters.getCountFileName(), true))
        throw resolvable_error("\nProblem encountered when reading the data from the case file.");
    if (_print.GetIsCanceled()) return false;

    if (!_parameters.getControlFileName().empty() && (_parameters.getModelType() == Parameters::BERNOULLI_TREE || _parameters.getModelType() == Parameters::BERNOULLI_TIME))
        if (!readControls(_parameters.getControlFileName(), true))
            throw resolvable_error("\nProblem encountered when reading the data from the control file.");

    if (_parameters.isSequentialScanBernoulli() && !_sequential_statistic->isFirstLook()) {
        if (!readCounts(_sequential_statistic->getCountDataFilename(), false))
            throw resolvable_error("\nProblem encountered when reading the sequential case data file.");
        if (!readControls(_sequential_statistic->getControlDataFilename(), false))
            throw resolvable_error("\nProblem encountered when reading the sequential control data file.");
        if (_print.GetIsCanceled()) return false;
    }

    if (!setupTree())
       throw resolvable_error("\nProblem encountered when setting up tree.");
    if (_print.GetIsCanceled()) return false;

    if (_parameters.isSequentialScanPurelyTemporal() && static_cast<unsigned int>(_TotalC) > _parameters.getSequentialMaximumSignal()) {
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
        else if (_parameters.getModelType() == Parameters::UNIFORM) {
            if (_censored_data)
                scan_success = scanTreeTemporalConditionNodeCensored();
            else
                scan_success = scanTreeTemporalConditionNode();
        } else if (_parameters.getModelType() == Parameters::BERNOULLI_TIME)
            scan_success = scanTreeTemporalConditionNodeCensored();
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
                if (_parameters.isSequentialScanPurelyTemporal()) {
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

    // create SequentialStatistic object if Bernoulli sequential
    if (_parameters.isSequentialScanBernoulli())
        _sequential_statistic->write(_parameters.getCountFileName(), _parameters.getControlFileName());

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
        if (_parameters.getModelType() == Parameters::BERNOULLI_TREE && _parameters.getConditionalType() == Parameters::TOTALCASES) {
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
            double z = _parameters.getPowerZ();
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
                                       "Current values are: n1 = %d, p1 = %g, N = %d, p0 = %g.\n",
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
        if (validateFileAccess(SequentialScanLoglikelihoodRatioWriter::getFilename(_parameters, buffer), false)) {
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
    Loglikelihood_t calcLogLikelihood(AbstractLoglikelihood::getNewLoglikelihood(_parameters, _TotalC, _TotalN, _censored_data));

    //std::string buffer;
    //int hits=0;
    for (size_t n=0; n < _Nodes.size(); ++n) {
        const NodeStructure& thisNode(*_Nodes[n]);
        if (isEvaluated(thisNode)) {

            // Always do simple cut for each node
            //printf("Evaluating cut [%s]\n", thisNode.getIdentifier().c_str());
            calculateCut(n, thisNode.getBrC(), thisNode.getBrN(), calcLogLikelihood);
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
                            CutStructure * cut = calculateCut(n, sumBranchC, sumBranchN, calcLogLikelihood);
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
                            CutStructure * cut = calculateCut(n, startChildNode.getBrC() + stopChildNode.getBrC(), startChildNode.getBrN() + stopChildNode.getBrN(), calcLogLikelihood);
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
                            CutStructure * cut = calculateCut(n, startChildNode.getBrC() + stopChildNode.getBrC(), startChildNode.getBrN() + stopChildNode.getBrN(), calcLogLikelihood);
                            if (cut) {
                                cut->addCutChild(startChildNode.getID(), true);
                                cut->addCutChild(stopChildNode.getID());
                            }
                            //++hits;printf("hits %d\n", hits);
                            for (size_t k=i+1; k < j; ++k) {
                                const NodeStructure& middleChildNode(*(thisNode.getChildren()[k]));
                                //printf("Evaluating cut [%s,%s,%s]\n", startChildNode.getIdentifier().c_str(), middleChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                CutStructure * cut = calculateCut(n, startChildNode.getBrC() + middleChildNode.getBrC() + stopChildNode.getBrC(), startChildNode.getBrN() + middleChildNode.getBrN() + stopChildNode.getBrN(), calcLogLikelihood);
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
    Loglikelihood_t calcLogLikelihood(AbstractLoglikelihood::getNewLoglikelihood(_parameters, _TotalC, _TotalN, _censored_data));

    // Define the start and end windows with the zero index offset already incorporated.
    DataTimeRange startWindow(temporalStartRange().getStart() + _zero_translation_additive, temporalStartRange().getEnd() + _zero_translation_additive),
                  endWindow(temporalEndRange().getStart() + _zero_translation_additive, temporalEndRange().getEnd() + _zero_translation_additive);
    // Define the minimum and maximum window lengths.
    boost::shared_ptr<AbstractWindowLength> window(getNewWindowLength());
    int  iWindowStart, iMinWindowStart, iWindowEnd, iMaxEndWindow;

    //std::string buffer;
    //int hits=0;
    for (size_t n=0; n < _Nodes.size(); ++n) {
        if (isEvaluated(*_Nodes[n])) {
            const NodeStructure& thisNode(*(_Nodes[n]));

            // always do simple cut
            //printf("Evaluating cut [%s]\n", thisNode.getIdentifier().c_str());
            iMaxEndWindow = std::min(endWindow.getEnd(), startWindow.getEnd() + window->maximum());
            for (iWindowEnd=endWindow.getStart(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
                window->windowstart(startWindow, iWindowEnd, iMinWindowStart, iWindowStart);
                for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
                    //_print.Printf("%d to %d\n", BasePrint::P_STDOUT,iWindowStart, iWindowEnd);
                    calculateCut(n, thisNode.getBrC_C()[iWindowStart] - thisNode.getBrC_C()[iWindowEnd + 1], static_cast<NodeStructure::expected_t>(thisNode.getBrC()), calcLogLikelihood, iWindowStart, iWindowEnd);
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
                    iMaxEndWindow = std::min(endWindow.getEnd(), startWindow.getEnd() + window->maximum());
                    for (iWindowEnd=endWindow.getStart(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
                        window->windowstart(startWindow, iWindowEnd, iMinWindowStart, iWindowStart);
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
                                    CutStructure * cut = calculateCut(n, branchWindow, branchSum, calcLogLikelihood, iWindowStart, iWindowEnd);
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
                    iMaxEndWindow = std::min(endWindow.getEnd(), startWindow.getEnd() + window->maximum());
                    for (iWindowEnd=endWindow.getStart(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
                        window->windowstart(startWindow, iWindowEnd, iMinWindowStart, iWindowStart);
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
                                    CutStructure * cut = calculateCut(n, startBranchWindow + stopBranchWindow, startBranchSum + stopBranchSum, calcLogLikelihood, iWindowStart, iWindowEnd);
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
                    iMaxEndWindow = std::min(endWindow.getEnd(), startWindow.getEnd() + window->maximum());
                    for (iWindowEnd=endWindow.getStart(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
                        window->windowstart(startWindow, iWindowEnd, iMinWindowStart, iWindowStart);
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
                                    CutStructure * cut = calculateCut(n, startBranchWindow + stopBranchWindow, startBranchSum + stopBranchSum, calcLogLikelihood, iWindowStart, iWindowEnd);
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
                                        CutStructure * cut = calculateCut(n, startBranchWindow + middleBranchWindow + stopBranchWindow, startBranchSum + middleBranchSum + stopBranchSum, calcLogLikelihood, iWindowStart, iWindowEnd);
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


/* SCANNING THE TREE for temporal model conditioned on node and censored. */
bool ScanRunner::scanTreeTemporalConditionNodeCensored() {
    _print.Printf("Scanning the tree.\n", BasePrint::P_STDOUT);
   Loglikelihood_t calcLogLikelihood(AbstractLoglikelihood::getNewLoglikelihood(_parameters, _TotalC, _TotalN, _censored_data));

    // Define the start and end windows with the zero index offset already incorporated.
    DataTimeRange startWindow(temporalStartRange().getStart() + _zero_translation_additive, temporalStartRange().getEnd() + _zero_translation_additive),
                  endWindow(temporalEndRange().getStart() + _zero_translation_additive, temporalEndRange().getEnd() + _zero_translation_additive);
    // Define the minimum and maximum window lengths.
    boost::shared_ptr<AbstractWindowLength> window(getNewWindowLength());
    int  iWindowStart, iMinWindowStart, iWindowEnd, iMaxEndWindow;

    //std::string buffer;
    //int hits=0;
    for (size_t n = 0; n < _Nodes.size(); ++n) {
        if (isEvaluated(*_Nodes[n])) {
            const NodeStructure& thisNode(*(_Nodes[n]));

            // always do simple cut
            //printf("Evaluating cut [%s]\n", thisNode.getIdentifier().c_str());
            iMaxEndWindow = std::min(endWindow.getEnd(), startWindow.getEnd() + window->maximum());
            for (iWindowEnd = endWindow.getStart(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
                window->windowstart(startWindow, iWindowEnd, iMinWindowStart, iWindowStart, thisNode.getMinCensoredBr());
                for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
                    //_print.Printf("%d to %d\n", BasePrint::P_STDOUT,iWindowStart, iWindowEnd);
                    calculateCut(n, thisNode.getBrC_C()[iWindowStart] - thisNode.getBrC_C()[iWindowEnd + 1], 
                                    thisNode.getBrN_C()[iWindowStart] - thisNode.getBrN_C()[iWindowEnd + 1], 
                                    thisNode.getBrC(), thisNode.getBrN(), calcLogLikelihood, iWindowStart, iWindowEnd);
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
                iMaxEndWindow = std::min(endWindow.getEnd(), startWindow.getEnd() + window->maximum());
                for (iWindowEnd = endWindow.getStart(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
                    window->windowstart(startWindow, iWindowEnd, iMinWindowStart, iWindowStart, thisNode.getMinCensoredBr());
                    for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
                        for (size_t i = 0; i < thisNode.getChildren().size() - 1; ++i) {
                            const NodeStructure& firstChildNode(*(thisNode.getChildren()[i]));
                            currentChildren.clear();
                            currentChildren.push_back(firstChildNode.getID());
                            //buffer = firstChildNode.getIdentifier().c_str();
                            NodeStructure::count_t branchWindow = firstChildNode.getBrC_C()[iWindowStart] - firstChildNode.getBrC_C()[iWindowEnd + 1];
                            NodeStructure::expected_t branchSum = firstChildNode.getBrN_C()[iWindowStart] - firstChildNode.getBrN_C()[iWindowEnd + 1];
                            NodeStructure::count_t branchCTotal = firstChildNode.getBrC();
                            NodeStructure::expected_t branchNTotal = firstChildNode.getBrN();
                            for (size_t j = i + 1; j < thisNode.getChildren().size(); ++j) {
                                const NodeStructure& childNode(*(thisNode.getChildren()[j]));
                                currentChildren.push_back(childNode.getID());
                                //buffer += ",";
                                //buffer += childNode.getIdentifier();
                                branchWindow += childNode.getBrC_C()[iWindowStart] - childNode.getBrC_C()[iWindowEnd + 1];
                                branchSum += childNode.getBrN_C()[iWindowStart] - childNode.getBrN_C()[iWindowEnd + 1];
                                branchCTotal += childNode.getBrC();
                                branchNTotal += childNode.getBrN();
                                //printf("Evaluating cut [%s]\n", buffer.c_str());
                                CutStructure * cut = calculateCut(n, branchWindow, branchSum, branchCTotal, branchNTotal, calcLogLikelihood, iWindowStart, iWindowEnd);
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
                iMaxEndWindow = std::min(endWindow.getEnd(), startWindow.getEnd() + window->maximum());
                for (iWindowEnd = endWindow.getStart(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
                    window->windowstart(startWindow, iWindowEnd, iMinWindowStart, iWindowStart, thisNode.getMinCensoredBr());
                    for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
                        for (size_t i = 0; i < thisNode.getChildren().size() - 1; ++i) {
                            const NodeStructure& startChildNode(*(thisNode.getChildren()[i]));
                            NodeStructure::count_t startBranchWindow = startChildNode.getBrC_C()[iWindowStart] - startChildNode.getBrC_C()[iWindowEnd + 1];
                            NodeStructure::expected_t startBranchSum = startChildNode.getBrN_C()[iWindowStart] - startChildNode.getBrN_C()[iWindowEnd + 1];
                            NodeStructure::count_t startBranchTotalC = startChildNode.getBrC();
                            NodeStructure::expected_t startBranchTotalN = startChildNode.getBrN();
                            for (size_t j = i + 1; j < thisNode.getChildren().size(); ++j) {
                                const NodeStructure& stopChildNode(*(thisNode.getChildren()[j]));
                                //printf("Evaluating cut [%s,%s]\n", startChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                NodeStructure::count_t stopBranchWindow = stopChildNode.getBrC_C()[iWindowStart] - stopChildNode.getBrC_C()[iWindowEnd + 1];
                                NodeStructure::expected_t stopBranchSum = stopChildNode.getBrN_C()[iWindowStart] - stopChildNode.getBrN_C()[iWindowEnd + 1];
                                CutStructure * cut = calculateCut(n, startBranchWindow + stopBranchWindow, startBranchSum + stopBranchSum, 
                                                                  startBranchTotalC + stopChildNode.getBrC(), startBranchTotalN + stopChildNode.getBrN(),
                                                                  calcLogLikelihood, iWindowStart, iWindowEnd);
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
                iMaxEndWindow = std::min(endWindow.getEnd(), startWindow.getEnd() + window->maximum());
                for (iWindowEnd = endWindow.getStart(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
                    window->windowstart(startWindow, iWindowEnd, iMinWindowStart, iWindowStart, thisNode.getMinCensoredBr());
                    for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
                        for (size_t i = 0; i < thisNode.getChildren().size() - 1; ++i) {
                            const NodeStructure& startChildNode(*(thisNode.getChildren()[i]));
                            NodeStructure::count_t startBranchWindow = startChildNode.getBrC_C()[iWindowStart] - startChildNode.getBrC_C()[iWindowEnd + 1];
                            NodeStructure::expected_t startBranchSum = startChildNode.getBrN_C()[iWindowStart] - startChildNode.getBrN_C()[iWindowEnd + 1];
                            NodeStructure::count_t startBranchTotalC = startChildNode.getBrC();
                            NodeStructure::expected_t startBranchTotalN = startChildNode.getBrN();
                            for (size_t j = i + 1; j < thisNode.getChildren().size(); ++j) {
                                const NodeStructure& stopChildNode(*(thisNode.getChildren()[j]));
                                //printf("Evaluating cut [%s,%s]\n", startChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                NodeStructure::count_t stopBranchWindow = stopChildNode.getBrC_C()[iWindowStart] - stopChildNode.getBrC_C()[iWindowEnd + 1];
                                NodeStructure::expected_t stopBranchSum = stopChildNode.getBrN_C()[iWindowStart] - stopChildNode.getBrN_C()[iWindowEnd + 1];
                                NodeStructure::count_t stopBranchTotalC = stopChildNode.getBrC();
                                NodeStructure::expected_t stopBranchTotalN = stopChildNode.getBrN();
                                CutStructure * cut = calculateCut(n, startBranchWindow + stopBranchWindow, startBranchSum + stopBranchSum, 
                                                                  startBranchTotalC + stopBranchTotalC, startBranchTotalN + stopBranchTotalN,
                                                                  calcLogLikelihood, iWindowStart, iWindowEnd);
                                if (cut) {
                                    cut->addCutChild(startChildNode.getID(), true);
                                    cut->addCutChild(stopChildNode.getID());
                                }
                                //++hits;printf("hits %d\n", hits);
                                for (size_t k = i + 1; k < j; ++k) {
                                    const NodeStructure& middleChildNode(*(thisNode.getChildren()[k]));
                                    NodeStructure::count_t middleBranchWindow = middleChildNode.getBrC_C()[iWindowStart] - middleChildNode.getBrC_C()[iWindowEnd + 1];
                                    NodeStructure::expected_t middleBranchSum = middleChildNode.getBrN_C()[iWindowStart] - middleChildNode.getBrN_C()[iWindowEnd + 1];
                                    NodeStructure::count_t middleBranchTotalC = middleChildNode.getBrC();
                                    NodeStructure::expected_t middleBranchTotalN = middleChildNode.getBrN();
                                    //printf("Evaluating cut [%s,%s,%s]\n", startChildNode.getIdentifier().c_str(), middleChildNode.getIdentifier().c_str(), stopChildNode.getIdentifier().c_str());
                                    CutStructure * cut = calculateCut(n, startBranchWindow + middleBranchWindow + stopBranchWindow, startBranchSum + middleBranchSum + stopBranchSum, 
                                                                      startBranchTotalC + middleBranchTotalC + stopBranchTotalC, startBranchTotalN + middleBranchTotalN + stopBranchTotalN,
                                                                      calcLogLikelihood, iWindowStart, iWindowEnd);
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
    Loglikelihood_t calcLogLikelihood(AbstractLoglikelihood::getNewLoglikelihood(_parameters, _TotalC, _TotalN, _censored_data));

    // Define the start and end windows with the zero index offset already incorporated.
    DataTimeRange startWindow(temporalStartRange().getStart() + _zero_translation_additive,
                              temporalStartRange().getEnd() + _zero_translation_additive),
                  endWindow(temporalEndRange().getStart() + _zero_translation_additive,
                            temporalEndRange().getEnd() + _zero_translation_additive);
    // Define the minimum and maximum window lengths.
    boost::shared_ptr<AbstractWindowLength> window(getNewWindowLength());
    int  iWindowStart, iMinWindowStart, iWindowEnd, iMaxEndWindow;

    //std::string buffer;
    //int hits=0;
    for (size_t n=0; n < _Nodes.size(); ++n) {
        if (isEvaluated(*_Nodes[n])) {
            const NodeStructure& thisNode(*(_Nodes[n]));

            // always do simple cut
            //printf("Evaluating cut [%s]\n", thisNode.getIdentifier().c_str());
            iMaxEndWindow = std::min(endWindow.getEnd(), startWindow.getEnd() + window->maximum());
            for (iWindowEnd=endWindow.getStart(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
                window->windowstart(startWindow, iWindowEnd, iMinWindowStart, iWindowStart);
                for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
                    //_print.Printf("%d to %d\n", BasePrint::P_STDOUT,iWindowStart, iWindowEnd);
                    calculateCut(n, thisNode.getBrC_C()[iWindowStart] - thisNode.getBrC_C()[iWindowEnd + 1],
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
                    iMaxEndWindow = std::min(endWindow.getEnd(), startWindow.getEnd() + window->maximum());
                    for (iWindowEnd=endWindow.getStart(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
                        window->windowstart(startWindow, iWindowEnd, iMinWindowStart, iWindowStart);
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
                                    CutStructure * cut = calculateCut(n, branchWindow, branchExpected, calcLogLikelihood, iWindowStart, iWindowEnd);
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
                    iMaxEndWindow = std::min(endWindow.getEnd(), startWindow.getEnd() + window->maximum());
                    for (iWindowEnd=endWindow.getStart(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
                        window->windowstart(startWindow, iWindowEnd, iMinWindowStart, iWindowStart);
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
                                    CutStructure * cut = calculateCut(n, startBranchWindow + stopBranchWindow, startBranchExpected + stopBranchExpected, calcLogLikelihood, iWindowStart, iWindowEnd);
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
                    iMaxEndWindow = std::min(endWindow.getEnd(), startWindow.getEnd() + window->maximum());
                    for (iWindowEnd=endWindow.getStart(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
                        window->windowstart(startWindow, iWindowEnd, iMinWindowStart, iWindowStart);
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
                                    CutStructure * cut = calculateCut(n, startBranchWindow + stopBranchWindow, startBranchExpected + stopBranchExpected, calcLogLikelihood, iWindowStart, iWindowEnd);
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
                                        CutStructure * cut = calculateCut(n, startBranchWindow + middleBranchWindow + stopBranchWindow, startBranchExpected + middleBranchExpected + stopBranchExpected, calcLogLikelihood, iWindowStart, iWindowEnd);
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

CutStructure * ScanRunner::calculateCut(size_t node_index, int BrC, double BrN, const Loglikelihood_t& logCalculator, DataTimeRange::index_t startIdx, DataTimeRange::index_t endIdx, int BrC_All, double BrN_All) {
    double loglikelihood = 0;
    
    if ((_parameters.getScanType() == Parameters::TREETIME && _parameters.getConditionalType() == Parameters::NODEANDTIME) ||
        (_parameters.getScanType() == Parameters::TIMEONLY && _parameters.isPerformingDayOfWeekAdjustment()) ||
        (_parameters.getScanType() == Parameters::TREETIME && _parameters.getConditionalType() == Parameters::NODE && _parameters.isPerformingDayOfWeekAdjustment()))
        loglikelihood = logCalculator->LogLikelihood(BrC, BrN);
    else if (_parameters.getModelType() == Parameters::UNIFORM)
        loglikelihood = logCalculator->LogLikelihood(BrC, BrN, endIdx - startIdx + 1);
    else if (_parameters.getModelType() == Parameters::BERNOULLI_TIME)
        loglikelihood = logCalculator->LogLikelihood(BrC, BrN, BrC_All, BrN_All);
    else
        loglikelihood = logCalculator->LogLikelihood(BrC, BrN);
    if (loglikelihood == logCalculator->UNSET_LOGLIKELIHOOD && !(_parameters.isSequentialScanBernoulli() && getSequentialStatistic().testCutSignaled(static_cast<int>(node_index)) != 0))
        // Exclude this cut if log likelihood is unset -- unless we're sequential scannig and this cut has signalled in prior looks.
        return 0;

    std::auto_ptr<CutStructure> cut(new CutStructure());
    cut->setLogLikelihood(loglikelihood);
    cut->setID(static_cast<int>(node_index));
    cut->setC(BrC);
    cut->setN(BrN);
    cut->setStartIdx(startIdx);
    cut->setEndIdx(endIdx);

    return updateCut(cut);
}

CutStructure * ScanRunner::calculateCut(size_t node_index, int C, double N, int BrC, double BrN, const Loglikelihood_t& logCalculator, DataTimeRange::index_t startIdx, DataTimeRange::index_t endIdx) {
    double loglikelihood = logCalculator->LogLikelihood(C, N, BrC, BrN);
    if (loglikelihood == logCalculator->UNSET_LOGLIKELIHOOD) return 0;

    std::auto_ptr<CutStructure> cut(new CutStructure());
    cut->setLogLikelihood(loglikelihood);
    cut->setID(static_cast<int>(node_index));
    cut->setC(C);
    cut->setN(N);
    cut->setStartIdx(startIdx);
    cut->setEndIdx(endIdx);

    return updateCut(cut);
}

CutStructure * ScanRunner::updateCut(std::auto_ptr<CutStructure>& cut) {
    CutStructureContainer_t::iterator itr;
    if (_parameters.getScanType() == Parameters::TIMEONLY) {
        // for time-only scans, we want to keep secondary clusters -- possibly one for each end date
        itr = std::lower_bound(_Cut.begin(), _Cut.end(), cut.get(), CompareCutsByEndIdx());
        if (!(itr != _Cut.end() && (*itr)->getEndIdx() == cut->getEndIdx()))
            return *(_Cut.insert(itr, cut.release()));
    }
    else {
        // we're keeping the best cut for each node
        itr = std::lower_bound(_Cut.begin(), _Cut.end(), cut.get(), CompareCutsById());
        if (!(itr != _Cut.end() && (*itr)->getID() == cut->getID()))
            return *(_Cut.insert(itr, cut.release()));
    }
    // at this point, we're replacing a cut with better log likeloihood cut
    if (cut->getLogLikelihood() > (*itr)->getLogLikelihood()) {
        size_t idx = std::distance(_Cut.begin(), itr);
        delete _Cut[idx]; _Cut[idx] = 0;
        _Cut[idx] = cut.release();
        return _Cut[idx];
    }
    return 0;
}

/* SETTING UP THE TREE */
bool ScanRunner::setupTree() {
    _print.Printf("Setting up the tree ...\n", BasePrint::P_STDOUT);

    // Initialize variables and calculates the total number of cases
    _TotalC = 0; _TotalN = 0;
    for(NodeStructureContainer_t::iterator itr=_Nodes.begin(); itr != _Nodes.end(); ++itr) {
        std::fill((*itr)->refBrC_C().begin(), (*itr)->refBrC_C().end(), 0);
        std::fill((*itr)->refBrN_C().begin(), (*itr)->refBrN_C().end(), 0);
        _TotalC = std::accumulate((*itr)->refIntC_C().begin(), (*itr)->refIntC_C().end(), _TotalC);
    }

    // If executing sequential scan and the number of cases in counts file is less than user specified total seqnential cases.
    if (_parameters.isSequentialScanPurelyTemporal() && static_cast<unsigned int>(_TotalC) > _parameters.getSequentialMaximumSignal()) {
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
                if (_censored_data) {
                    // If there exists censored data, the expected counts were added during the case file read. Now we need to add expected counts for cases that are not censored.
                    size_t daysInDataTimeRange = _parameters.getDataTimeRangeSet().getTotalDaysAcrossRangeSets();
                    DataTimeRange minmax = _parameters.getDataTimeRangeSet().getMinMax();
                    for (size_t n=0; n < _Nodes.size(); ++n) {
                        NodeStructure::count_t nodeCount=0, nodeNonCensored=0, nodeCensored=0;
                        NodeStructure& node = *(_Nodes[n]);
                        nodeCount = std::accumulate(node.getIntC_C().begin(), node.getIntC_C().end(), nodeCount);
                        nodeCensored = std::accumulate(node.getIntC_Censored().begin(), node.getIntC_Censored().end(), nodeCensored);
                        node.calcMinCensored();
                        nodeNonCensored = nodeCount - nodeCensored;
                        for (size_t t=0; t < daysInDataTimeRange && nodeNonCensored != 0; ++t)
                            node.refIntN_C()[t] += static_cast<double>(nodeNonCensored) / static_cast<double>(daysInDataTimeRange);
                    }
                }
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

    if (_parameters.getModelType() == Parameters::BERNOULLI_TIME) {
        _TotalControls = static_cast<int>(_TotalN) - _TotalC;
    } else if (_parameters.getModelType() == Parameters::BERNOULLI_TREE) {
        // controls are read with population for Bernoulli -- so calculate total controls now
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
        _parameters.getModelType() == Parameters::BERNOULLI_TREE ||
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
    // We can now set each nodes calculated level in tree structure.
    std::for_each(_Nodes.begin(), _Nodes.end(), std::mem_fun(&NodeStructure::assignLevel));
    for (NodeStructureContainer_t::iterator itr=_Nodes.begin(); itr != _Nodes.end(); ++itr) {
        (*itr)->setCumulative();
        if ((*itr)->assignLevel() == 1)
            _rootNodes.push_back((*itr));
    }

    return true;
}
