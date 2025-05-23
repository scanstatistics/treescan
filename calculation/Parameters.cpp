//***************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//***************************************************************************
#include "Parameters.h"
#include "PrjException.h"
#include "RandomNumberGenerator.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/assign.hpp>

const int Parameters::giNumParameters = 85;

Parameters::cut_maps_t Parameters::getCutTypeMap() {
   cut_map_t cut_type_map_abbr = boost::assign::map_list_of("S",Parameters::SIMPLE) ("P",Parameters::PAIRS) ("T",Parameters::TRIPLETS) ("O",Parameters::ORDINAL);
   cut_map_t cut_type_map = boost::assign::map_list_of("simple",Parameters::SIMPLE) ("pairs",Parameters::PAIRS) ("triplets",Parameters::TRIPLETS) ("ordinal",Parameters::ORDINAL);
   return std::make_pair(cut_type_map_abbr, cut_type_map);
}

/* Returns the global event probability as decimal value. */
double Parameters::getProbability() const {
    double top, bottom;
    string_to_type<double>(_probablility_ratio.first.c_str(), top);
    string_to_type<double>(_probablility_ratio.second.c_str(), bottom);
    return top / bottom;
}

/* Returns the baseline event probability for the power evaluation as decimal value. */
double Parameters::getPowerBaselineProbability() const {
    double top, bottom;
    string_to_type<double>(_power_baseline_probablility_ratio.first.c_str(), top);
    string_to_type<double>(_power_baseline_probablility_ratio.second.c_str(), bottom);
    return top / bottom;
}

bool  Parameters::operator==(const Parameters& rhs) const {
  if (_replications != rhs._replications) return false;
  if (_treeFileNames != rhs._treeFileNames) return false;
  if (_cutsFileName != rhs._cutsFileName) return false;  
  if (_countFileName != rhs._countFileName) return false;
  if (_controlFileName != rhs._controlFileName) return false;
  if (_date_precision_type != rhs._date_precision_type) return false;
  //if (_dataTimeRangeSet.getDataTimeRangeSets() != rhs._dataTimeRangeSet.getDataTimeRangeSets()) return false;
  //if (_temporalStartRange != rhs._temporalStartRange) return false;
  //if (_temporalEndRange != rhs._temporalEndRange) return false;
  if (_outputFileName != rhs._outputFileName) return false;
  if (_resultsFormat != rhs._resultsFormat) return false;
  if (_parametersSourceFileName != rhs._parametersSourceFileName) return false;
  //if (_creationVersion != rhs._creationVersion) return false;
  //if (_randomizationSeed != rhs._randomizationSeed) return false;
  if (_numRequestedParallelProcesses != rhs._numRequestedParallelProcesses) return false;
  if (_randomlyGenerateSeed != rhs._randomlyGenerateSeed) return false;
  if (_generateHtmlResults != rhs._generateHtmlResults) return false;
  if (_generateTableResults != rhs._generateTableResults) return false;
  if (_generateNCBIAsnResults != rhs._generateNCBIAsnResults) return false;
  if (_generateNewickFile != rhs._generateNewickFile) return false;
  if (_printColumnHeaders != rhs._printColumnHeaders) return false;
  if (_modelType != rhs._modelType) return false;
  if (_probablility_ratio != rhs._probablility_ratio) return false;
  if (_variable_case_probablility != rhs._variable_case_probablility) return false;
  if (_cut_type != rhs._cut_type) return false;
  if (_conditional_type != rhs._conditional_type) return false;
  if (_scan_type != rhs._scan_type) return false;
  if (_maximum_window_percentage != rhs._maximum_window_percentage) return false;
  if (_minimum_window_length != rhs._minimum_window_length) return false;
  if (_generate_llr_results != rhs._generate_llr_results) return false;
  if (_read_simulations != rhs._read_simulations) return false;
  if (_input_sim_file != rhs._input_sim_file) return false;
  if (_write_simulations != rhs._write_simulations) return false;
  if (_output_sim_file != rhs._output_sim_file) return false;
  if (_maximum_window_percentage != rhs._maximum_window_percentage) return false;
  if (_maximum_window_length != rhs._maximum_window_length) return false;
  if (_maximum_window_type != rhs._maximum_window_type) return false;
  if (_minimum_window_length != rhs._minimum_window_length) return false;
  if (_report_critical_values != rhs._report_critical_values) return false;
  if (_perform_power_evaluations != rhs._perform_power_evaluations) return false;
  if (_critical_values_type != rhs._critical_values_type) return false;
  if (_critical_value_05 != rhs._critical_value_05) return false;
  if (_critical_value_01 != rhs._critical_value_01) return false;
  if (_critical_value_001 != rhs._critical_value_001) return false;
  if (_power_evaluation_type != rhs._power_evaluation_type) return false;
  if (_power_evaluation_totalcases != rhs._power_evaluation_totalcases) return false;
  if (_power_replica != rhs._power_replica) return false;
  if (_power_alt_hypothesis_filename != rhs._power_alt_hypothesis_filename) return false;
  if (_power_baseline_probablility_ratio != rhs._power_baseline_probablility_ratio) return false;
  if (_dayofweek_adjustment != rhs._dayofweek_adjustment) return false;
  if (_report_attributable_risk != rhs._report_attributable_risk) return false;
  if (_attributable_risk_exposed != rhs._attributable_risk_exposed) return false;
  if (_self_control_design != rhs._self_control_design) return false;
  if (_restrict_tree_levels != rhs._restrict_tree_levels) return false;
  if (_restricted_tree_levels != rhs._restricted_tree_levels) return false;
  if (_restrict_evaluated_tree_nodes != rhs._restrict_evaluated_tree_nodes) return false;
  if (_not_evaluated_nodes_filename != rhs._not_evaluated_nodes_filename) return false;
  //if (_input_sources != rhs._input_sources) return false;
  if (_sequential_scan != rhs._sequential_scan) return false;
  if (_sequential_min_signal != rhs._sequential_min_signal) return false;
  if (_sequential_max_signal != rhs._sequential_max_signal) return false;
  if (_sequential_file != rhs._sequential_file) return false;
  if (_sequential_alpha_overall != rhs._sequential_alpha_overall) return false;
  if (_sequential_alpha_spending != rhs._sequential_alpha_spending) return false;
  if (_power_z != rhs._power_z) return false;
  if (_apply_risk_window_restriction != rhs._apply_risk_window_restriction) return false;
  if (_risk_window_percentage != rhs._risk_window_percentage) return false;
  if (_forced_censored_algorithm != rhs._forced_censored_algorithm) return false;
  if (_apply_exclusion_ranges != rhs._apply_exclusion_ranges) return false;
  if (_exclusion_time_ranges != rhs._exclusion_time_ranges) return false;
  if (_minimum_censor_time != rhs._minimum_censor_time) return false;
  if (_minimum_censor_percentage != rhs._minimum_censor_percentage) return false;
  if (_risk_window_censor_alt_denominator != rhs._risk_window_censor_alt_denominator) return false;
  if (_apply_risk_window_restriction_censored != rhs._apply_risk_window_restriction_censored) return false;
  if (_prospective_frequency_type != rhs._prospective_frequency_type) return false;
  if (_prospective_frequency != rhs._prospective_frequency) return false;
  if (_prospective_analysis != rhs._prospective_analysis) return false;
  if (_restrict_temporal_windows != rhs._restrict_temporal_windows) return false;
  if (_data_time_range_str != rhs._data_time_range_str) return false;
  if (_temporal_start_range_str != rhs._temporal_start_range_str) return false;
  if (_temporal_end_range_str != rhs._temporal_end_range_str) return false;
  if (_exclusion_time_range_str != rhs._exclusion_time_range_str) return false;  
  if (_output_temporal_graph != rhs._output_temporal_graph) return false;
  if (_temporal_graph_report_type != rhs._temporal_graph_report_type) return false;
  if (_temporal_graph_report_count != rhs._temporal_graph_report_count) return false;
  if (_temporal_graph_report_cutoff != rhs._temporal_graph_report_cutoff) return false;
  if (_minimum_highrate_nodes_cases != rhs._minimum_highrate_nodes_cases) return false;
  if (_minimum_lowrate_nodes_cases != rhs._minimum_lowrate_nodes_cases) return false;
  if (_scan_rate_type != rhs._scan_rate_type) return false;
  if (_data_only_on_leaves != rhs._data_only_on_leaves) return false;
  if (_relaxed_study_data_period_checking != rhs._relaxed_study_data_period_checking) return false;
  if (_allow_multi_parent_nodes != rhs._allow_multi_parent_nodes) return false;
  if (_allow_multiple_roots != rhs._allow_multiple_roots) return false;
  if (_include_identical_parent_cuts != rhs._include_identical_parent_cuts) return false;
  if (_pvalue_reporting_type != rhs._pvalue_reporting_type) return false;
  if (_early_term_threshold != rhs._early_term_threshold) return false;

  return true;
}

/** If passed filename contains a slash, then assumes that path is complete and
    sInputFilename is not modified. If filename does not contain a slash, it is
    assumed that filename is located in same directory of parameter file.
    sInputFilename is reset to this location. Note that the primary reason for
    implementing this feature was to permit the program to be installed in any
    location and sample parameter files run immediately without having to edit
    input file paths. */
void Parameters::assignMissingPath(std::string & sInputFilename, bool bCheckWritable) {
    if (sInputFilename.empty()) return;
    // Windows somewhat supports both back slashes and forward slashes (for the most part) starting around Vista.
    // To facilitate the test for a file path, convert any forward slashes to back slash (standard Windows separator).
    #ifdef _WINDOWS_
    std::transform(sInputFilename.begin(), sInputFilename.end(), sInputFilename.begin(), [](char& ch) {
        if (ch == FileName::FORWARDSLASH) ch = FileName::BACKSLASH;
        return ch;
    });
    #endif
    // Assume that if slashes exist, then this is a complete file path, so
    // we'll make no attempts to determine what path might be otherwise.
    if (sInputFilename.find(FileName::getPathSeparator()) == sInputFilename.npos) {
        FileName fParameterFilename, fFilename;
        std::string buffer;
        // If no slashes, then this file is assumed to be in same directory as parameters file.
        fParameterFilename.setFullPath(getSourceFileName().c_str());
        fFilename.setFullPath(sInputFilename.c_str());
        fFilename.setLocation(fParameterFilename.getLocation(buffer).c_str());
        if (bCheckWritable && !validateFileAccess(fFilename.getFullPath(buffer), true, true)) {
            // if writability fails, then try setting to user documents directory
            std::string temp;
            fFilename.setLocation(GetUserDocumentsDirectory(buffer, fParameterFilename.getLocation(temp)).c_str());
        }
        fFilename.getFullPath(sInputFilename);
    }
}

/** Copies all class variables from the given Parameters object (rhs) into this one */
void Parameters::copy(const Parameters &rhs) {
    _treeFileNames = rhs._treeFileNames;
    _countFileName = rhs._countFileName;
    _controlFileName = rhs._controlFileName;
    _date_precision_type = rhs._date_precision_type;
    _dataTimeRangeSet = rhs._dataTimeRangeSet;
    _cutsFileName = rhs._cutsFileName;

    _scan_type = rhs._scan_type;
    _conditional_type = rhs._conditional_type;
    _modelType = rhs._modelType;
    _probablility_ratio = rhs._probablility_ratio;
    _variable_case_probablility = rhs._variable_case_probablility;
    _restrict_temporal_windows = rhs._restrict_temporal_windows;
    _temporalStartRange = rhs._temporalStartRange;
    _temporalEndRange = rhs._temporalEndRange;
    _replications = rhs._replications;
    _cut_type = rhs._cut_type;
    _maximum_window_percentage = rhs._maximum_window_percentage;
    _maximum_window_length = rhs._maximum_window_length;
    _maximum_window_type = rhs._maximum_window_type;
    _minimum_window_length = rhs._minimum_window_length;
    _minimum_highrate_nodes_cases = rhs._minimum_highrate_nodes_cases;
    _minimum_lowrate_nodes_cases = rhs._minimum_lowrate_nodes_cases;

    _outputFileName = rhs._outputFileName;
    _resultsFormat = rhs._resultsFormat;
    _generateHtmlResults = rhs._generateHtmlResults;
    _generateTableResults = rhs._generateTableResults;
    _generateNCBIAsnResults = rhs._generateNCBIAsnResults;
    _generateNewickFile = rhs._generateNewickFile;
    _printColumnHeaders = rhs._printColumnHeaders;
    _generate_llr_results = rhs._generate_llr_results;

    _perform_power_evaluations = rhs._perform_power_evaluations;
    _critical_values_type = rhs._critical_values_type;
    _critical_value_05 = rhs._critical_value_05;
    _critical_value_01 = rhs._critical_value_01;
    _critical_value_001 = rhs._critical_value_001;
    _power_evaluation_type = rhs._power_evaluation_type;
    _power_evaluation_totalcases = rhs._power_evaluation_totalcases;
    _power_replica = rhs._power_replica;
    _power_alt_hypothesis_filename = rhs._power_alt_hypothesis_filename;
    _power_baseline_probablility_ratio = rhs._power_baseline_probablility_ratio;
    _power_z = rhs._power_z;

    _read_simulations = rhs._read_simulations;
    _input_sim_file = rhs._input_sim_file;
    _write_simulations = rhs._write_simulations;
    _output_sim_file = rhs._output_sim_file;
    _report_critical_values = rhs._report_critical_values;

    _randomizationSeed = rhs._randomizationSeed;
    _numRequestedParallelProcesses = rhs._numRequestedParallelProcesses;
    _randomlyGenerateSeed = rhs._randomlyGenerateSeed;

    _parametersSourceFileName = rhs._parametersSourceFileName;
    _creationVersion = rhs._creationVersion;

    _dayofweek_adjustment = rhs._dayofweek_adjustment;

    _report_attributable_risk = rhs._report_attributable_risk;
    _attributable_risk_exposed = rhs._attributable_risk_exposed;
    _self_control_design = rhs._self_control_design;
    _input_sources = rhs._input_sources;

    _restrict_tree_levels = rhs._restrict_tree_levels;
    _restricted_tree_levels = rhs._restricted_tree_levels;
    _restrict_evaluated_tree_nodes = rhs._restrict_evaluated_tree_nodes;
    _not_evaluated_nodes_filename = rhs._not_evaluated_nodes_filename;

    _sequential_scan = rhs._sequential_scan;
    _sequential_min_signal = rhs._sequential_min_signal;
    _sequential_max_signal = rhs._sequential_max_signal;
    _sequential_file = rhs._sequential_file;
    _sequential_alpha_overall = rhs._sequential_alpha_overall;
    _sequential_alpha_spending = rhs._sequential_alpha_spending;

    _forced_censored_algorithm = rhs._forced_censored_algorithm;

    _apply_risk_window_restriction = rhs._apply_risk_window_restriction;
    _risk_window_percentage = rhs._risk_window_percentage;

    _apply_exclusion_ranges = rhs._apply_exclusion_ranges;
    _exclusion_time_ranges = rhs._exclusion_time_ranges;

    _minimum_censor_time = rhs._minimum_censor_time;
    _minimum_censor_percentage = rhs._minimum_censor_percentage;
    _risk_window_censor_alt_denominator = rhs._risk_window_censor_alt_denominator;
    _apply_risk_window_restriction_censored = rhs._apply_risk_window_restriction_censored;

    _prospective_frequency_type = rhs._prospective_frequency_type;
    _prospective_frequency = rhs._prospective_frequency;
    _prospective_analysis = rhs._prospective_analysis;

    _data_time_range_str = rhs._data_time_range_str;
    _temporal_start_range_str = rhs._temporal_start_range_str;
    _temporal_end_range_str = rhs._temporal_end_range_str;
    _exclusion_time_range_str = rhs._exclusion_time_range_str;

    _output_temporal_graph = rhs._output_temporal_graph;
    _temporal_graph_report_type = rhs._temporal_graph_report_type;
    _temporal_graph_report_count = rhs._temporal_graph_report_count;
    _temporal_graph_report_cutoff = rhs._temporal_graph_report_cutoff;
    _scan_rate_type = rhs._scan_rate_type;
    _data_only_on_leaves = rhs._data_only_on_leaves;
    _relaxed_study_data_period_checking = rhs._relaxed_study_data_period_checking;
    _allow_multi_parent_nodes = rhs._allow_multi_parent_nodes;
    _allow_multiple_roots = rhs._allow_multiple_roots;
    _include_identical_parent_cuts = rhs._include_identical_parent_cuts;
    _pvalue_reporting_type = rhs._pvalue_reporting_type;
    _early_term_threshold = rhs._early_term_threshold;
}

/** Returns whether early termination option is performed. */
bool Parameters::getTerminateSimulationsEarly() const {
    return getPValueReportingType() == TERMINATION_PVALUE && // early termination option
        getNumReplicationsRequested() >= MIN_REPLICA_RPT_PVALUE && // number of replications is at least at threshold for reporting p-value
        getPerformPowerEvaluations() == false && // we're not performing the power evaluation
        getSequentialScan() == false; // sequential scans don't early terminate
}

/** Returns threshold for early termination. If reporting default p-value, then threshold is determined by number of replications requested. */
unsigned int Parameters::getExecuteEarlyTermThreshold() const {
    return _early_term_threshold;
}

/** Returns whether analysis is tree-only, unconditioned Bernoulli,with self control design. */
bool Parameters::getIsSelfControlVariableBerounlli() const {
    return getScanType() == Parameters::TREEONLY && getConditionalType() == Parameters::UNCONDITIONAL &&
        getModelType() == Parameters::BERNOULLI_TREE && getVariableCaseProbability() && getSelfControlDesign();
}

/** Returns whether analysis is calculate test statistic. */
bool Parameters::getIsTestStatistic() const {
    return (
        (_scan_type == TREETIME && _conditional_type == NODEANDTIME) ||
        (_scan_type == TIMEONLY && _conditional_type == TOTALCASES && isPerformingDayOfWeekAdjustment()) ||
        (_scan_type == TREETIME && _conditional_type == NODE && isPerformingDayOfWeekAdjustment()) ||
        (_scan_type == TREEONLY && _conditional_type == UNCONDITIONAL && _modelType == BERNOULLI_TREE && getVariableCaseProbability())
    );
}

/** Returns the maximum temporal window in data time units. */
unsigned int Parameters::getMaximumWindowInTimeUnits() const {
    switch (_maximum_window_type) {
        case Parameters::PERCENTAGE_WINDOW :
            return static_cast<unsigned int>(std::floor(static_cast<double>(_dataTimeRangeSet.getTotalDaysAcrossRangeSets()) * _maximum_window_percentage / 100.0));
        case Parameters::FIXED_LENGTH : return _maximum_window_length;
        default: throw prg_error("Unknown maximum window type (%d).", "getMaximumWindowInTimeUnits()", _maximum_window_type);
    }
}

/** Returns number of parallel processes to run. */
unsigned int Parameters::getNumParallelProcessesToExecute() const {
#ifdef RPRTCMPT_RUNTIMES
  // reporting of run-time components is not thread safe at this time,
  // and has no useful purpose to be such - at this time
  return 1;
#else
  unsigned int  iNumProcessors;

  if (_numRequestedParallelProcesses <= 0)
    // parameter of zero or less indicates that we want all available processors
    iNumProcessors = GetNumSystemProcessors();
  else
    // else parameter indicates the maximum number of processors to use
    iNumProcessors = std::min(_numRequestedParallelProcesses, GetNumSystemProcessors());
    //iNumProcessors = giNumRequestedParallelProcesses;

  return iNumProcessors;
#endif  
}

/** If passed filename has same path as passed parameter filename, returns 'name.extension' else returns filename. */
const char * Parameters::getRelativeToParameterName(const FileName& fParameterName,
                                                     const std::string& sFilename,
                                                     std::string& sValue) const {
  FileName      fInputFilename(sFilename.c_str());
  std::string   buffer, buffer2;

  if (!stricmp(fInputFilename.getLocation(buffer).c_str(), fParameterName.getLocation(buffer2).c_str()))
    sValue = fInputFilename.getFileName() + fInputFilename.getExtension();
  else
    sValue = sFilename.c_str();
  return sValue.c_str();
}

bool Parameters::isSequentialScanPurelyTemporal() const {
    return _sequential_scan && _scan_type == Parameters::TIMEONLY && _conditional_type == Parameters::TOTALCASES;
}

bool Parameters::isSequentialScanBernoulli() const{
    return _sequential_scan && _modelType == Parameters::BERNOULLI_TREE && _conditional_type == Parameters::UNCONDITIONAL;
}

bool Parameters::isSequentialScanPoisson() const {
	return _sequential_scan && _modelType == Parameters::POISSON;
}

bool Parameters::isSequentialScanTreeOnly() const {
	return isSequentialScanBernoulli() || isSequentialScanPoisson();
}

/** Sets counts data file name.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file does not exist. */
void Parameters::setCountFileName(const char * sCountFileName, bool bCorrectForRelativePath) {
  _countFileName = sCountFileName;
  if (bCorrectForRelativePath) assignMissingPath(_countFileName);
}

/** Sets control data file name.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file does not exist. */
void Parameters::setControlFileName(const char * sControlFileName, bool bCorrectForRelativePath) {
    _controlFileName = sControlFileName;
    if (bCorrectForRelativePath) assignMissingPath(_controlFileName);
}

/** Sets file name which details node which will not be evaluated.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file does not exist. */
void Parameters::setNotEvaluatedNodesFileName(const char* filename, bool bCorrectForRelativePath) {
    _not_evaluated_nodes_filename = filename;
    if (bCorrectForRelativePath) assignMissingPath(_not_evaluated_nodes_filename);
}

/** Sets cuts data file name.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file does not exist. */
void Parameters::setCutsFileName(const char * sCutsFileName, bool bCorrectForRelativePath) {
  _cutsFileName = sCutsFileName;
  if (bCorrectForRelativePath) assignMissingPath(_cutsFileName);
}

/** Sets counts data file name.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file does not exist. */
void Parameters::setTreeFileName(const char * sTreeFileName, bool bCorrectForRelativePath, size_t treeIdx) {
  if (!treeIdx)
    throw prg_error("Index %d out of range [1,].", "setTreeFileNames()", treeIdx);

  if (treeIdx > _treeFileNames.size())
    _treeFileNames.resize(treeIdx);

  _treeFileNames[treeIdx - 1] = sTreeFileName;
  if (bCorrectForRelativePath)
    assignMissingPath(_treeFileNames[treeIdx - 1]);
}


/** Initializes global variables to default values */
void Parameters::setAsDefaulted() {
    _treeFileNames.resize(1);
    _treeFileNames.front() = "";

    _countFileName = "";
    _controlFileName = "";
    _date_precision_type = DataTimeRange::GENERIC;
    _dataTimeRangeSet = DataTimeRangeSet();
    _cutsFileName = "";

    _scan_type = TREEONLY;
    _conditional_type = UNCONDITIONAL;
    _modelType = POISSON;
    _probablility_ratio = ratio_t("1","2");
    _variable_case_probablility = false;
    _restrict_temporal_windows = false;
    _temporalStartRange = DataTimeRange();
    _temporalEndRange = DataTimeRange();
    _replications = 999;
    _cut_type = SIMPLE;
    _data_only_on_leaves = true;
    _relaxed_study_data_period_checking = false;
    _allow_multi_parent_nodes = false;
    _allow_multiple_roots = true;
    _maximum_window_percentage = 50.0;
    _maximum_window_length = 1;
    _maximum_window_type = PERCENTAGE_WINDOW;
    _minimum_window_length = 2;
    _minimum_highrate_nodes_cases = 2;
    _minimum_lowrate_nodes_cases = 0;
    _scan_rate_type = HIGHRATE;

    _outputFileName = "";
    _generateHtmlResults = false;
    _generateTableResults = false;
    _generateNCBIAsnResults = false;
    _generateNewickFile = false;
    _printColumnHeaders = true;
    _resultsFormat = TEXT;
    _generate_llr_results = false;

    _perform_power_evaluations = false;
    _critical_values_type = CV_MONTECARLO;
    _critical_value_05                       = 0.0;
    _critical_value_01                       = 0.0;
    _critical_value_001                      = 0.0;

    _read_simulations = false;
    _input_sim_file = "";
    _write_simulations = false;
    _output_sim_file = "";
    _report_critical_values = false;
    _power_evaluation_type = PE_WITH_ANALYSIS;
    _power_evaluation_totalcases = 600;
    _power_replica = _replications + 1;
    _power_alt_hypothesis_filename = "";
    _power_baseline_probablility_ratio = ratio_t("1","2");
    _power_z = 0.001;

    _creationVersion.iMajor = atoi(VERSION_MAJOR);
    _creationVersion.iMinor = atoi(VERSION_MINOR);
    _creationVersion.iRelease = atoi(VERSION_RELEASE);
    _randomizationSeed = RandomNumberGenerator::glDefaultSeed;
    _randomlyGenerateSeed = false;
    _numRequestedParallelProcesses = 0;
    _dayofweek_adjustment = false;

    _report_attributable_risk = false;
    _attributable_risk_exposed = 0;
    _include_identical_parent_cuts = false;
    _self_control_design = false;

    _input_sources.clear();

    _restrict_tree_levels = false;
    _restricted_tree_levels.clear();
    _restrict_evaluated_tree_nodes = false;
    _not_evaluated_nodes_filename = "";

    _sequential_scan = false;
    _sequential_min_signal=3;
    _sequential_max_signal=200;
    _sequential_file="";
    _sequential_alpha_overall = 0.05;
    _sequential_alpha_spending = 0.01;

    _apply_risk_window_restriction = true;
    _risk_window_percentage = 20.0;

    _forced_censored_algorithm = false;

    _apply_exclusion_ranges = false;
    _exclusion_time_ranges = DataTimeRangeSet();

    _minimum_censor_time = 50;
    _minimum_censor_percentage = 10;
    _risk_window_censor_alt_denominator = 2.0;
    _apply_risk_window_restriction_censored = false;

    _prospective_frequency_type = DAILY;
    _prospective_frequency = 1;
    _prospective_analysis = false;

    _data_time_range_str = "";
    _temporal_start_range_str = "";
    _temporal_end_range_str = "";
    _exclusion_time_range_str = "";

    _output_temporal_graph = false;
    _temporal_graph_report_cutoff = 0.05;
    _temporal_graph_report_count = 1;
    _temporal_graph_report_type = MLC_ONLY;

    _pvalue_reporting_type = STANDARD_PVALUE;
    _early_term_threshold = 50;
}

/** Sets output data file name.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file
    does not exist. */
void Parameters::setOutputFileName(const char * sOutPutFileName, bool bCorrectForRelativePath) {
  _outputFileName = sOutPutFileName;
  if (bCorrectForRelativePath) assignMissingPath(_outputFileName, true);
}

/** Sets sequential scan file name.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file
    does not exist. */
void Parameters::setSequentialFilename(const char * s, bool bCorrectForRelativePath) {
  _sequential_file = s;
  if (bCorrectForRelativePath) assignMissingPath(_sequential_file, false);
}

/** Sets power evaluation alternative hypothesis data file name.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file
    does not exist. */
void Parameters::setPowerEvaluationAltHypothesisFilename(const char * s, bool bCorrectForRelativePath) {
  _power_alt_hypothesis_filename = s;
  if (bCorrectForRelativePath) assignMissingPath(_power_alt_hypothesis_filename, false);
}

/** Sets p-value reporting type. Throws exception if out of range. */
void Parameters::setPValueReportingType(PValueReportingType e) {
    if (e < STANDARD_PVALUE || e > TERMINATION_PVALUE)
        throw prg_error("Enumeration %d out of range [%d,%d].", "setPValueReportingType()", e, STANDARD_PVALUE, TERMINATION_PVALUE);
    _pvalue_reporting_type = e;
}

/** Sets filename of file used to load parameters. */
void Parameters::setSourceFileName(const char * sParametersSourceFileName) {
  // Use FileName class to ensure that a relative path is expanded to absolute path.
  std::string buffer;
  _parametersSourceFileName = FileName(sParametersSourceFileName).getFullPath(buffer);
}

/** Sets temporal graph report type. Throws exception if out of range. */
void Parameters::setTemporalGraphReportType(TemporalGraphReportType e) {
    if (e < MLC_ONLY || e > SIGNIFICANT_ONLY)
        throw prg_error("Enumeration %d out of range [%d,%d].", "setTemporalGraphReportType()", e, MLC_ONLY, SIGNIFICANT_ONLY);
    _temporal_graph_report_type = e;
}

void Parameters::setInputSimulationsFilename(const char * s, bool bCorrectForRelativePath) {
  _input_sim_file = s;
  if (bCorrectForRelativePath) assignMissingPath(_input_sim_file);
}

void Parameters::setOutputSimulationsFilename(const char * s, bool bCorrectForRelativePath) {
  _output_sim_file = s;
  if (bCorrectForRelativePath) assignMissingPath(_output_sim_file);
}

void Parameters::read(const std::string &filename, ParametersFormat type) {
    using boost::property_tree::ptree;
    ptree pt;
    std::string buffer;

    switch (type) {
        case JSON: read_json(filename, pt); break;
        case XML: 
        default: read_xml(filename, pt);
    }
    setSourceFileName(filename.c_str());

    // Input
    setTreeFileName(pt.get<std::string>("parameters.input.tree-filename", "").c_str(), true);
    setCountFileName(pt.get<std::string>("parameters.input.count-filename", "").c_str(), true);
    setControlFileName(pt.get<std::string>("parameters.input.control-filename", "").c_str(), true);
    _date_precision_type = static_cast<DataTimeRange::DatePrecisionType>(pt.get<unsigned int>("parameters.analysis.date-precision", DataTimeRange::GENERIC));
    _dataTimeRangeSet.assign(pt.get<std::string>("parameters.input.data-time-range", "0,0"), _date_precision_type, boost::optional<boost::gregorian::date>());
    // Advanced Input
    setCutsFileName(pt.get<std::string>("parameters.input.advanced.input.cuts-filename", "").c_str(), true);
    _cut_type = static_cast<CutType>(pt.get<unsigned int>("parameters.input.advanced.input.cuts-type", SIMPLE));
    _apply_risk_window_restriction = pt.get<bool>("parameters.input.advanced.input.apply-risk-window-restriction", false);
    _risk_window_percentage = pt.get<double>("parameters.input.advanced.input.risk-window-percentage", 50.0);
    // Analysis
    _scan_type = static_cast<ScanType>(pt.get<unsigned int>("parameters.analysis.scan", TREEONLY));
    _conditional_type = static_cast<ConditionalType>(pt.get<unsigned int>("parameters.analysis.conditional", UNCONDITIONAL));
    _modelType = static_cast<ModelType>(pt.get<unsigned int>("parameters.analysis.probability-model", POISSON));
    _probablility_ratio.first = pt.get<std::string>("parameters.analysis.event-probability.numerator", "1");
    _probablility_ratio.second = pt.get<std::string>("parameters.analysis.event-probability.denominator", "2");
    _variable_case_probablility = pt.get<bool>("parameters.analysis.event-probability.variable-case-probability", false);
     _self_control_design = pt.get<bool>("parameters.analysis.self-control-design", false);
     _scan_rate_type = static_cast<ScanRateType>(pt.get<unsigned int>("parameters.analysis.scanrate", HIGHRATE));
     // Advanced Analysis - Temporal Window
    _maximum_window_percentage = pt.get<double>("parameters.analysis.advanced.temporal-window.maximum-window-percentage", 50);
    _maximum_window_length = pt.get<unsigned int>("parameters.analysis.advanced.temporal-window.maximum-window-length", 1);
    _maximum_window_type = static_cast<MaximumWindowType>(pt.get<unsigned int>("parameters.analysis.advanced.temporal-window.maximum-window-type", PERCENTAGE_WINDOW));
    _minimum_window_length = pt.get<unsigned int>("parameters.analysis.advanced.temporal-window.minimum-window-length", 2);
    _prospective_analysis = pt.get<bool>("parameters.analysis.advanced.temporal-window.prospective-analysis", false);
    _restrict_temporal_windows = pt.get<bool>("parameters.analysis.advanced.temporal-window.restrict-temporal-windows", false);
    _temporalStartRange.assign(pt.get<std::string>("parameters.analysis.advanced.temporal-window.start-range", "0,0"), _date_precision_type, _dataTimeRangeSet.getDataTimeRangeSets().front().getDateStart());
    _temporalEndRange.assign(pt.get<std::string>("parameters.analysis.advanced.temporal-window.end-range", "0,0"), _date_precision_type, _dataTimeRangeSet.getDataTimeRangeSets().front().getDateStart());
    // Advanced Analysis - Adjustments Window
    _dayofweek_adjustment = pt.get<bool>("parameters.analysis.advanced.adjustments.perform-day-of-week-adjustments", false);
    _apply_exclusion_ranges = pt.get<bool>("parameters.analysis.advanced.adjustments.apply-exclusion-time-ranges", false);
    _exclusion_time_ranges.assign(pt.get<std::string>("parameters.analysis.advanced.adjustments.exclusion-time-ranges", "0,0"), _date_precision_type, _dataTimeRangeSet.getDataTimeRangeSets().front().getDateStart());
    // Advanced Analysis - Inference
    _replications = pt.get<unsigned int>("parameters.analysis.advanced.inference.replications", 999);
    _randomizationSeed = pt.get<unsigned int>("parameters.analysis.advanced.inference.seed", static_cast<unsigned int>(RandomNumberGenerator::glDefaultSeed));
    _randomlyGenerateSeed = pt.get<bool>("parameters.analysis.advanced.inference.generate-seed", false);
    _restrict_tree_levels = pt.get<bool>("parameters.analysis.advanced.inference.restrict-tree-levels", false);
    buffer = pt.get<std::string>("parameters.analysis.advanced.inference.restricted-tree-levels", "");
    _restrict_evaluated_tree_nodes = pt.get<bool>("parameters.analysis.advanced.inference.restrict-evaluated-tree-nodes", false);
    setNotEvaluatedNodesFileName(pt.get<std::string>("parameters.analysis.advanced.inference.not-evaluated-nodes-filename", "").c_str(), true);
    csv_string_to_typelist<unsigned int>(buffer.c_str(), _restricted_tree_levels);
    // Advanced Analysis - Inference
    _sequential_scan = pt.get<bool>("parameters.analysis.advanced.sequential-scan.sequential-scan", false);
    _sequential_max_signal = pt.get<unsigned int>("parameters.analysis.advanced.sequential-scan.sequential-maximum-signal", 200);
    _sequential_min_signal = pt.get<unsigned int>("parameters.analysis.advanced.sequential-scan.squential-minimum-signal", 3);
    setSequentialFilename(pt.get<std::string>("parameters.analysis.advanced.sequential-scan.sequential-filename", "").c_str(), true);
    _sequential_alpha_overall = pt.get<double>("parameters.analysis.advanced.sequential-scan.sequential-alpha-overall", 0.05);
    _sequential_alpha_spending = pt.get<double>("parameters.analysis.advanced.sequential-scan.sequential-alpha-spending", 0.01);
    _minimum_highrate_nodes_cases = pt.get<unsigned int>("parameters.analysis.advanced.inference.minimum-node-cases", 2);
    // Advanced Analysis - Power Evaluations
    _perform_power_evaluations = pt.get<bool>("parameters.analysis.advanced.power-evaluations.perform-power-evaluations", false);
    _power_evaluation_type = static_cast<PowerEvaluationType>(pt.get<unsigned int>("parameters.analysis.advanced.power-evaluations.power-evaluation-type", PE_WITH_ANALYSIS));
    _critical_values_type = static_cast<CriticalValuesType>(pt.get<unsigned int>("parameters.analysis.advanced.power-evaluations.critical-values-type", CV_MONTECARLO));
    _critical_value_05 = pt.get<double>("parameters.analysis.advanced.power-evaluations.critical-value-05", 0);
    _critical_value_01 = pt.get<double>("parameters.analysis.advanced.power-evaluations.critical-value-01", 0);
    _critical_value_001 = pt.get<double>("parameters.analysis.advanced.power-evaluations.critical-value-001", 0);
    _power_evaluation_totalcases = pt.get<int>("parameters.analysis.advanced.power-evaluations.totalcases", 0);
    _power_replica = pt.get<int>("parameters.analysis.advanced.power-evaluations.replications", _replications + 1);
    _power_baseline_probablility_ratio.first = pt.get<std::string>("parameters.analysis.advanced.power-evaluations.baseline-probability.numerator", "1");
    _power_baseline_probablility_ratio.second = pt.get<std::string>("parameters.analysis.advanced.power-evaluations.baseline-probability.denominator", "2");
    setPowerEvaluationAltHypothesisFilename(pt.get<std::string>("parameters.analysis.advanced.power-evaluations.alternative-hypothesis-file", "").c_str(), true);
    _power_z = pt.get<double>("parameters.analysis.advanced.power-evaluations.power-z", 0.001);
    // Advanced Analysis - Miscellaneous
    _prospective_frequency_type = static_cast<ProspectiveFrequency>(pt.get<unsigned int>("parameters.analysis.advanced.miscellaneous.prospective-frequency-type", DAILY));
    _prospective_frequency = pt.get<unsigned int>("parameters.analysis.advanced.miscellaneous.prospective-frequency", 1);
    // Output
    setOutputFileName(pt.get<std::string>("parameters.output.results-file", "").c_str(), true);
    _generateHtmlResults = pt.get<bool>("parameters.output.generate-html-results", true);
    _generateTableResults = pt.get<bool>("parameters.output.generate-table-results", true);
    _generateNCBIAsnResults = pt.get<bool>("parameters.output.generate-ncbi-asn-results", true);
    _generateNewickFile = pt.get<bool>("parameters.output.generate-newick-file", true);
    // Advanced Output - Additional Output
    _generate_llr_results = pt.get<bool>("parameters.output.advanced.additional-output.generate-llr-results", true);
    _report_critical_values = pt.get<bool>("parameters.output.advanced.additional-output.report-critical-values", false);
    _report_attributable_risk = pt.get<bool>("parameters.output.advanced.additional-output.report-attributable-risk", false);
    _attributable_risk_exposed = pt.get<unsigned int>("parameters.output.advanced.additional-output.attributable-risk-exposed", 0);
    // Power Simulations
    _read_simulations = pt.get<bool>("parameters.power-simulations.input-simulations", true);
    setInputSimulationsFilename(pt.get<std::string>("parameters.power-simulations.input-simulations-file", "").c_str(), true);
    _write_simulations = pt.get<bool>("parameters.power-simulations.output-simulations", true);
    setOutputSimulationsFilename(pt.get<std::string>("parameters.power-simulations.output-simulations-file", "").c_str(), true);
    // Run Options
    _numRequestedParallelProcesses = pt.get<unsigned int>("parameters.run-options.processors", 0);
}

void Parameters::write(const std::string &filename, ParametersFormat type) const {
    using boost::property_tree::ptree;
    using boost::property_tree::xml_writer_settings;
    ptree pt;
    std::string buffer;

    // Input
    pt.put("parameters.input.tree-file", _treeFileNames.front());
    pt.put("parameters.input.count-file", _countFileName);
    pt.put("parameters.input.control-file", _controlFileName);
    pt.put("parameters.analysis.date-precision", static_cast<unsigned int>(_date_precision_type));
    pt.put("parameters.input.data-time-range", _dataTimeRangeSet.toString(buffer, _date_precision_type));
    // Advanced Input
    pt.put("parameters.input.advanced.input.cuts-file", _cutsFileName);
    pt.put("parameters.input-advanced.input.cuts-type", static_cast<unsigned int>(_cut_type));
    pt.put("parameters.input.advanced.input.apply-risk-window-restriction", _apply_risk_window_restriction);
    pt.put("parameters.input.advanced.input.risk-window-percentage", _risk_window_percentage);
    // Analysis
    pt.put("parameters.analysis.scan", static_cast<unsigned int>(_scan_type));
    pt.put("parameters.analysis.conditional", static_cast<unsigned int>(_conditional_type));
    pt.put("parameters.analysis.probability-model", static_cast<unsigned int>(_modelType));
    pt.put("parameters.analysis.self-control-design", _self_control_design);
    pt.put("parameters.analysis.event-probability.numerator", _probablility_ratio.first);
    pt.put("parameters.analysis.event-probability.denominator", _probablility_ratio.second);
    pt.put("parameters.analysis.event-probability.variable-case-probability", _variable_case_probablility);
    pt.put("parameters.analysis.scanrate", static_cast<unsigned int>(_scan_rate_type));
    // Advanced Analysis - Temporal Window
    pt.put("parameters.analysis.advanced.temporal-window.maximum-window-percentage", _maximum_window_percentage);
    pt.put("parameters.analysis.advanced.temporal-window.maximum-window-length", _maximum_window_length);
    pt.put("parameters.analysis.advanced.temporal-window.maximum-window-type", static_cast<unsigned int>(_maximum_window_type));
    pt.put("parameters.analysis.advanced.temporal-window.minimum-window-length", _minimum_window_length);
    pt.put("parameters.analysis.advanced.temporal-window.prospective-analysis", _prospective_analysis);
    pt.put("parameters.analysis.advanced.temporal-window.restrict-temporal-windows", _restrict_temporal_windows);
    pt.put("parameters.analysis.advanced.temporal-window.start-range", _temporalStartRange.toString(buffer, _date_precision_type));
    pt.put("parameters.analysis.advanced.temporal-window.end-range", _temporalEndRange.toString(buffer, _date_precision_type));
    // Advanced Analysis - Adjustments
    pt.put("parameters.analysis.advanced.adjustments.perform-day-of-week-adjustments", _dayofweek_adjustment);
    pt.put("parameters.analysis.advanced.adjustments.apply-exclusion-time-ranges", _apply_exclusion_ranges);
    pt.put("parameters.analysis.advanced.adjustments.exclusion-time-ranges", _exclusion_time_ranges.toString(buffer, _date_precision_type));
    // Advanced Analysis - Inference
    pt.put("parameters.analysis.advanced.inference.replications", _replications);
    pt.put("parameters.analysis.advanced.inference.seed", _randomizationSeed);
    pt.put("parameters.analysis.advanced.inference.generate-seed", _randomlyGenerateSeed);
    pt.put("parameters.analysis.advanced.inference.restrict-tree-levels", _restrict_tree_levels);
    typelist_csv_string<unsigned int>(_restricted_tree_levels, buffer);	
    pt.put("parameters.analysis.advanced.inference.restrict-evaluated-tree-nodes", _restrict_evaluated_tree_nodes);
    pt.put("parameters.analysis.advanced.inference.not-evaluated-nodes-filename", _not_evaluated_nodes_filename);
    pt.put("parameters.analysis.advanced.inference.tree-levels", buffer);
    pt.put("parameters.analysis.advanced.inference.minimum-node-cases", _minimum_highrate_nodes_cases);
    // Advanced Analysis - Sequential Scan
    pt.put("parameters.analysis.advanced.sequential-scan.sequential-scan", _sequential_scan);
    pt.put("parameters.analysis.advanced.sequential-scan.sequential-maximum-signal", _sequential_max_signal);
    pt.put("parameters.analysis.advanced.sequential-scan.squential-minimum-signal", _sequential_min_signal);
    pt.put("parameters.analysis.advanced.sequential-scan.sequential-filename", _sequential_file);
    pt.put("parameters.analysis.advanced.sequential-scan.sequential-alpha-overall", _sequential_alpha_overall);
    pt.put("parameters.analysis.advanced.sequential-scan.sequential-alpha-spending", _sequential_alpha_spending);
    // Advanced Analysis - Miscellaneous
    pt.put("parameters.analysis.advanced.miscellaneous.prospective-frequency-type", static_cast<unsigned int>(_prospective_frequency_type));
    pt.put("parameters.analysis.advanced.miscellaneous.prospective-frequenc", _prospective_frequency);
    // Output
    pt.put("parameters.output.results-file", _outputFileName);
    pt.put("parameters.output.generate-html-results", _generateHtmlResults);
    pt.put("parameters.output.generate-table-results", _generateTableResults);
    pt.put("parameters.output.generate-ncbi-asn-results", _generateNCBIAsnResults);
    pt.put("parameters.output.generate-newick-file", _generateNewickFile);
    // Advanced Output - Additional Output
    pt.put("parameters.output.generate-llr-results", _generate_llr_results);
    pt.put("parameters.output.report-critical-values", _report_critical_values);
    pt.put("parameters.output.advanced.additional-output.report-attributable-risk", _report_attributable_risk);
    pt.put("parameters.output.advanced.additional-output.attributable-risk-exposed", _attributable_risk_exposed);
    // Power Evaluations
    pt.put("parameters.analysis.advanced.power-evaluations.perform-power-evaluations", _perform_power_evaluations);
    pt.put("parameters.analysis.advanced.power-evaluations.power-evaluation-type", static_cast<unsigned int>(_power_evaluation_type));
    pt.put("parameters.analysis.advanced.power-evaluations.critical-values-type", static_cast<unsigned int>(_critical_values_type));
    pt.put("parameters.analysis.advanced.power-evaluations.critical-value-05", _critical_value_05);
    pt.put("parameters.analysis.advanced.power-evaluations.critical-value-01", _critical_value_01);
    pt.put("parameters.analysis.advanced.power-evaluations.critical-value-001", _critical_value_001);
    pt.put("parameters.analysis.advanced.power-evaluations.totalcases", _power_evaluation_totalcases);
    pt.put("parameters.analysis.advanced.power-evaluations.replications", _power_replica);
    pt.put("parameters.analysis.advanced.power-evaluations.alternative-hypothesis-file", _power_alt_hypothesis_filename);
    pt.put("parameters.analysis.advanced.power-evaluations.baseline-probability.numerator", _power_baseline_probablility_ratio.first);
    pt.put("parameters.analysis.advanced.power-evaluations.baseline-probability.denominator", _power_baseline_probablility_ratio.second);
    pt.put("parameters.analysis.advanced.power-evaluations.power-z", _power_z);
    // Power Simulations
    pt.put("parameters.power-simulations.input-simulations", _read_simulations);
    pt.put("parameters.power-simulations.input-simulations-file", _input_sim_file);
    pt.put("parameters.power-simulations.output-simulations", _write_simulations);
    pt.put("parameters.power-simulations.output-simulations-file", _output_sim_file);
    // Run Options
    pt.put("parameters.run-options.processors", _numRequestedParallelProcesses);

    switch (type) {
        case JSON: write_json(filename, pt); break;
        case XML:  
        default: {
            //const xml_writer_settings<char> w(' ', 4);
            write_xml(filename, pt, std::locale());
        }
    }
}
