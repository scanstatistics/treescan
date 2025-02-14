//***************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//***************************************************************************
#include "ParameterFileAccess.h"
#include "IniParameterFileAccess.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/detail/xml_parser_error.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

//////////// ParameterAccessCoordinator //////////////////////////////////////

/** Determines format of parameter file and invokes particular parameter reader class to read parameters from file. */
bool ParameterAccessCoordinator::read(const std::string& filename, BasePrint& print) {
    if (access(filename.c_str(), 04) == -1)
        throw resolvable_error("Unable to open settings file '%s'.\n", filename.c_str());
    return IniParameterFileAccess(_parameters, print).Read(filename.c_str());
}

/** Writes parameters to file in most recent format. */
void ParameterAccessCoordinator::write(const std::string& filename, BasePrint& print) {
    IniParameterFileAccess(_parameters, print).Write(filename.c_str());
}

//////////////////// AbtractParameterFileAccess ////////////////////////////////

/** Constructor */
AbtractParameterFileAccess::AbtractParameterFileAccess(Parameters& Parameters, BasePrint& PrintDirection, bool bWriteBooleanAsDigit)
                           :_parameters(Parameters), gPrintDirection(PrintDirection), _read_error(false), _write_boolean_as_digit(bWriteBooleanAsDigit) {}

/** Destructor */
AbtractParameterFileAccess::~AbtractParameterFileAccess() {}

/* Converts ratio data to string for wrting. */
std::string& AbtractParameterFileAccess::AsString(std::string& ref, const Parameters::ratio_t& r) {
    double test;
    string_to_type<double>(r.second.c_str(), test);
    if (test == 1.0)
        ref = r.first;
    else
        printString(ref, "%s/%s", r.first.c_str(), r.second.c_str());
    return ref; 
}

/** Returns constant char pointer to parameters comment string. */
const char * AbtractParameterFileAccess::GetParameterComment(Parameters::ParameterType e) const {
    try {
        switch (e) {
            // Input
            case Parameters::TREE_FILE               : return "tree structure filename";
            case Parameters::COUNT_FILE              : return "count data filename";
            case Parameters::CONTROL_FILE            : return "control data filename";
            case Parameters::DATE_PRECISION          : return "date precision type (NONE=0, GENERIC=1, YEAR=2, MONTH=3, DAY=4)";
            case Parameters::DATA_TIME_RANGES        : return "data time ranges: [integer,integer] or [yyyy/mm/dd,yyyy/mm/dd]";
            // Advanced Input
            case Parameters::CUT_FILE                : return "cuts filename";
            case Parameters::CUT_TYPE                : return "default cuts type (SIMPLE=0, PAIRS=1, TRIPLETS=2, ORDINAL=3, COMBINATORIAL=4)";
            case Parameters::DATA_ONLY_ON_LEAVES     : return "allow data only on tree leaves - (y/n)";
            case Parameters::RELAXED_STUDY_DATA_PERIOD_CHECKING: return "ignore cases outside study period - (y/n)";
            case Parameters::ALLOW_MULTI_PARENT_NODES: return "allow multi-parent nodes - (y/n)";
            case Parameters::ALLOW_MULTIPLE_ROOTS    : return "allow multiple root nodes - (y/n)";
            case Parameters::APPLY_RISK_WINDOW_RESTRICTION : return "apply risk window restriction - (y/n)";
            case Parameters::RISK_WINDOW_PERCENTAGE  : return "risk window percentage (0 < x <= 100.0)";
            case Parameters::MINIMUM_CENSOR_TIME     : return "minimum censor time (2 <= x)";
            case Parameters::MINIMUM_CENSOR_PERCENTAGE : return "minimum censor time percentage of study period (0 < x <= 100.0)";
            case Parameters::RSK_WND_CENSOR          : return "apply risk window restriction due to censoring - (y/n)";
            case Parameters::RSK_WND_ALT_CENSOR_DENOM: return "risk window alternative censor denominator (integer)";
            // Analysis
            case Parameters::SCAN_TYPE               : return "scan type (TREEONLY=0, TREETIME=1, TIMEONLY=2)";
            case Parameters::CONDITIONAL_TYPE        : return "conditional type (UNCONDITIONAL=0, TOTALCASES=1, NODE=2, NODEANDTIME=3)";
            case Parameters::MODEL_TYPE              : return "probability model type (POISSON=0, BERNOULLI_TREE=1, UNIFORM=2, Not-Applicable=3)";
            case Parameters::SELF_CONTROL_DESIGN     : return "self control design - unconditional Bernoulli only (y/n)";
            case Parameters::EVENT_PROBABILITY       : return "case probability (integer/integer)";
            case Parameters::VARIABLE_CASE_PROBABILITY: return "variable case probability - unconditional Bernoulli only (y/n)";
            case Parameters::SEQUENTIAL_SCAN         : return "perform sequential scan - time-only scan (y/n)";
            case Parameters::SEQUENTIAL_MAX_SIGNAL   : return "sequential scan maximum cases for signal (integer)";
            case Parameters::RESTRICTED_TIME_RANGE   : return "restrict temporal windows (y/n)";
            case Parameters::START_DATA_TIME_RANGE   : return "start data time range: [integer,integer] or [yyyy/mm/dd,yyyy/mm/dd]";
            case Parameters::END_DATA_TIME_RANGE     : return "end data time range: [integer,integer] or [yyyy/mm/dd,yyyy/mm/dd]";
            case Parameters::SCAN_RATE_TYPE          : return "scan rate type (HIGHRATE=0, LOWRATE=1, HIGHORLOWRATE=2)";
            // Advanced Analysis - Temporal Window
            case Parameters::MAXIMUM_WINDOW_PERCENTAGE : return "maximum temporal size as percentage of data time range (0 < x <= 50.0)";
            case Parameters::MAXIMUM_WINDOW_FIXED    : return "maximum temporal size as fixed time length (integer)";
            case Parameters::MAXIMUM_WINDOW_TYPE     : return "maximum temporal size selection (PERCENTAGE_WINDOW=0, FIXED_LENGTH=1)";
            case Parameters::MINIMUM_WINDOW_FIXED    : return "minimum temporal size as fixed time length (integer)";
            case Parameters::PROSPECTIVE_ANALYSIS    : return "prospective analysis (y/n)";
            case Parameters::SEQUENTIAL_MIN_SIGNAL   : return "sequential scan - minimum cases to signal (integer)";
            case Parameters::SEQUENTIAL_FILE         : return "sequential scan filename";
            case Parameters::SEQUENTIAL_ALPHA_OVERALL: return "sequential alpha overall";
            case Parameters::SEQUENTIAL_ALPHA_SPENDING: return "sequential alpha spending";
            // Advanced Analysis - Adjustments
            case Parameters::DAYOFWEEK_ADJUSTMENT    : return "perform day of week adjustments (y/n)";
            case Parameters::APPLY_EXCLUSION_RANGES  : return "apply exclusion time ranges (y/n)";
            case Parameters::EXCLUSION_RANGES        : return "exclusion time ranges (semi-colon separated list of ranges: [integer,integer];[integer,integer] or [yyyy/mm/dd,yyyy/mm/dd];[yyyy/mm/dd,yyyy/mm/dd])";
            // Advanced Analysis - Inference
            case Parameters::REPLICATIONS            : return "number of simulation replications (0, 9, 999, n999)";
            case Parameters::RANDOMIZATION_SEED      : return "randomization seed (integer)";
            case Parameters::RANDOMLY_GENERATE_SEED  : return "generate randomization seed (y/n)";
            case Parameters::RESTRICT_TREE_LEVELS    : return "restrict tree levels evaluated (y/n)";
            case Parameters::RESTRICTED_TREE_LEVELS  : return "tree levels excluded from evaluation (csv list of unsigned integers, root level is 1)";
            case Parameters::RESTRICT_EVALUATED_NODES: return "restrict tree nodes evaluated (y/n)";
            case Parameters::NOT_EVALUATED_NODES_FILE: return "not evaluated tree nodes filename";
            case Parameters::MINIMUM_CASES_NODE      : return "minimum number of cases in a node (integer)";
            case Parameters::PVALUE_REPORT_TYPE      : return "p-value reporting type (STANDARD_PVALUE=0, TERMINATION_PVALUE)";
            case Parameters::EARLY_TERM_THRESHOLD    : return "early termination threshold (> 0)";
                /* Output */
            case Parameters::RESULTS_FILE            : return "results filename";
            case Parameters::RESULTS_HTML            : return "create HTML results (y/n)";
            case Parameters::RESULTS_CSV             : return "create CSV results (y/n)";
            case Parameters::RESULTS_ASN             : return "create NCBI Asn results (y/n)";
            case Parameters::RESULTS_NWK             : return "create Newick File (y/n)";
            // Advanced Output - Additional Output
            case Parameters::RESULTS_LLR             : return "create LLR results (y/n)";
            case Parameters::REPORT_CRITICAL_VALUES  : return "report critical values (y/n)";
            case Parameters::REPORT_ATTR_RISK        : return "report attributable risk (y/n)";
            case Parameters::ATTR_RISK_NUM_EXPOSED   : return "number of exposed attributable risk is based upon (positive integer)";
            case Parameters::INCLUDE_IDENTICAL_PARENT_CUTS: return "report parent cuts that match child cuts (y/n)";
            case Parameters::OUTPUT_TEMPORAL_GRAPH   : return "output temporal graph HTML file (y/n)";
            case Parameters::TEMPORAL_GRAPH_REPORT_TYPE: return "temporal graph cluster reporting type (0=Only most likely cluster, 1=X most likely clusters, 2=Only significant clusters)";
            case Parameters::TEMPORAL_GRAPH_MLC_COUNT: return "number of most likely clusters to report in temporal graph (positive integer)";
            case Parameters::TEMPORAL_GRAPH_CUTOFF   : return "significant clusters p-value cutoff to report in temporal graph (0.000-1.000)";
            // Power Evaluations
            case Parameters::POWER_EVALUATIONS       : return "perform power evaluations (y/n)";
            case Parameters::POWER_EVALUATION_TYPE   : return "power evaluation type (0=Analysis And Power Evaluation Together, 1=Only Power Evaluation With Count File, 2=Only Power Evaluation With Defined Total Cases)";
            case Parameters::CRITICAL_VALUES_TYPE    : return "critical values type (0=Monte Carlo, 1=User Specified Values)";
            case Parameters::CRITICAL_VALUE_05       : return "power evaluation critical value .05 (> 0)";
            case Parameters::CRITICAL_VALUE_01       : return "power evaluation critical value .01 (> 0)";
            case Parameters::CRITICAL_VALUE_001      : return "power evaluation critical value .001 (> 0)";
            case Parameters::POWER_EVALUATION_TOTALCASES : return "total cases in power evaluation (integer)";
            case Parameters::POWER_EVALUATIONS_REPLICA : return "number of replications in power step (integer)";
            case Parameters::POWER_EVALUATIONS_FILE : return "power evaluation alternative hypothesis filename";
            case Parameters::POWER_BASELINE_PROBABILITY : return "power baseline probability (integer/integer)";
            case Parameters::POWER_Z                 : return "power z value (0 < z <= 0.01)";
            // Power Simulations
            case Parameters::READ_SIMULATIONS        : return "input simulation data (y/n)";
            case Parameters::INPUT_SIM_FILE          : return "input simulation filename";
            case Parameters::WRITE_SIMULATIONS       : return "output simulation data (y/n)";
            case Parameters::OUTPUT_SIM_FILE         : return "output simulation filename";
            // Runtime Options
            case Parameters::PARALLEL_PROCESSES      : return "number of parallel processes to execute (0=All Processors, x=At Most X Processors)";
            // System
            case Parameters::CREATION_VERSION        : return "parameters version - do not modify";
            case Parameters::PROSPECTIVE_FREQ_TYPE   : return "frequency of prospective analyses type (0=Daily, 1=Weekly, 2=Monthy, 3=Quarterly, 4=Yearly)";
            case Parameters::PROSPECTIVE_FREQ        : return "frequency of prospective (integer)";
            default : throw prg_error("Unknown parameter enumeration %d.","GetParameterComment()", e);
        };
    } catch (prg_exception& x) {
        x.addTrace("GetParameterComment()","AbtractParameterFileAccess");
        throw;
    }
}

/** Assigns string representation to passed string class for parameter. */
std::string & AbtractParameterFileAccess::GetParameterString(Parameters::ParameterType e, std::string& s) const {
    std::string worker;

    try {
        switch (e) {
            // Input
            case Parameters::TREE_FILE                : s = _parameters.getTreeFileNames().front(); return s;
            case Parameters::COUNT_FILE               : s = _parameters.getCountFileName(); return s;
            case Parameters::CONTROL_FILE             : s = _parameters.getControlFileName(); return s;
            case Parameters::DATE_PRECISION           : return AsString(s, _parameters.getDatePrecisionType());
            case Parameters::DATA_TIME_RANGES         : s =  _parameters.getDataTimeRangeStr(); return s;
            // Advanced Input
            case Parameters::CUT_FILE                 : s = _parameters.getCutsFileName(); return s;
            case Parameters::CUT_TYPE                 : return AsString(s, _parameters.getCutType());
            case Parameters::DATA_ONLY_ON_LEAVES      : return AsString(s, _parameters.getDataOnlyOnLeaves());
            case Parameters::RELAXED_STUDY_DATA_PERIOD_CHECKING: return AsString(s, _parameters.getRelaxedStudyDataPeriodChecking());
            case Parameters::ALLOW_MULTI_PARENT_NODES : return AsString(s, _parameters.getAllowMultiParentNodes());
            case Parameters::ALLOW_MULTIPLE_ROOTS     : return AsString(s, _parameters.getAllowMultipleRoots());
            case Parameters::APPLY_RISK_WINDOW_RESTRICTION : return AsString(s, _parameters.isApplyingRiskWindowRestriction());
            case Parameters::RISK_WINDOW_PERCENTAGE   : return AsString(s, _parameters.getRiskWindowPercentage());
            case Parameters::MINIMUM_CENSOR_TIME      : return AsString(s, _parameters.getMinimumCensorTime());
            case Parameters::MINIMUM_CENSOR_PERCENTAGE: return AsString(s, _parameters.getMinimumCensorPercentage());
            case Parameters::RSK_WND_CENSOR: return AsString(s, _parameters.isApplyingRiskWindowRestrictionCensored());
            case Parameters::RSK_WND_ALT_CENSOR_DENOM : return AsString(s, _parameters.getRiskWindowAltCensorDenominator());
            // Analysis
            case Parameters::SCAN_TYPE                : return AsString(s, _parameters.getScanType());
            case Parameters::CONDITIONAL_TYPE         : return AsString(s, _parameters.getConditionalType());
            case Parameters::MODEL_TYPE               : return AsString(s, _parameters.getModelType());
            case Parameters::SELF_CONTROL_DESIGN      : return AsString(s, _parameters.getSelfControlDesign());
            case Parameters::EVENT_PROBABILITY        : return AsString(s, _parameters.getProbabilityRatio());
            case Parameters::VARIABLE_CASE_PROBABILITY : return AsString(s, _parameters.getVariableCaseProbability());
            case Parameters::SEQUENTIAL_SCAN          : return AsString(s, _parameters.getSequentialScan());
            case Parameters::SEQUENTIAL_MAX_SIGNAL    : return AsString(s, _parameters.getSequentialMaximumSignal());
            case Parameters::SEQUENTIAL_MIN_SIGNAL    : return AsString(s, _parameters.getSequentialMinimumSignal());
            case Parameters::SEQUENTIAL_FILE          : s = _parameters.getSequentialFilename(); return s;
            case Parameters::SEQUENTIAL_ALPHA_OVERALL : return AsString(s, _parameters.getSequentialAlphaOverall());
            case Parameters::SEQUENTIAL_ALPHA_SPENDING: return AsString(s, _parameters.getSequentialAlphaSpending());
            case Parameters::RESTRICTED_TIME_RANGE    : return AsString(s, _parameters.getRestrictTemporalWindows());
            case Parameters::START_DATA_TIME_RANGE    : return s = _parameters.getTemporalStartRangeStr(); return s;
            case Parameters::END_DATA_TIME_RANGE      : return s = _parameters.getTemporalEndRangeStr(); return s;
            case Parameters::SCAN_RATE_TYPE           : return AsString(s, _parameters.getScanRateType());
            // Advanced Analysis - Temporal Window
            case Parameters::MAXIMUM_WINDOW_PERCENTAGE: return AsString(s, _parameters.getMaximumWindowPercentage());
            case Parameters::MAXIMUM_WINDOW_FIXED     : return AsString(s, _parameters.getMaximumWindowLength());
            case Parameters::MAXIMUM_WINDOW_TYPE      : return AsString(s, _parameters.getMaximumWindowType());
            case Parameters::MINIMUM_WINDOW_FIXED     : return AsString(s, _parameters.getMinimumWindowLength());
            case Parameters::PROSPECTIVE_ANALYSIS     : return AsString(s, _parameters.getIsProspectiveAnalysis());
            // Advanced Analysis - Adjustments
            case Parameters::DAYOFWEEK_ADJUSTMENT     : return AsString(s, _parameters.getPerformDayOfWeekAdjustment());
            case Parameters::APPLY_EXCLUSION_RANGES   : return AsString(s, _parameters.isApplyingExclusionTimeRanges());
            case Parameters::EXCLUSION_RANGES         : s = _parameters.getExclusionTimeRangeStr(); return s;
            // Advanced Analysis - Inference
            case Parameters::REPLICATIONS             : return AsString(s, _parameters.getNumReplicationsRequested());
            case Parameters::RANDOMIZATION_SEED       : return AsString(s, static_cast<unsigned int>(_parameters.getRandomizationSeed()));
            case Parameters::RANDOMLY_GENERATE_SEED   : return AsString(s, _parameters.isRandomlyGeneratingSeed());
            case Parameters::RESTRICT_TREE_LEVELS     : return AsString(s, _parameters.getRestrictTreeLevels());
            case Parameters::RESTRICTED_TREE_LEVELS   : typelist_csv_string<unsigned int>(_parameters.getRestrictedTreeLevels(), s); return s;
            case Parameters::RESTRICT_EVALUATED_NODES : return AsString(s, _parameters.getRestrictEvaluatedTreeNodes());
            case Parameters::NOT_EVALUATED_NODES_FILE : s = _parameters.getNotEvaluatedNodesFileName(); return s;
            case Parameters::MINIMUM_CASES_NODE       : return AsString(s, _parameters.getMinimumHighRateNodeCases());
            case Parameters::PVALUE_REPORT_TYPE       : return AsString(s, _parameters.getPValueReportingType());
            case Parameters::EARLY_TERM_THRESHOLD     : return AsString(s, _parameters.getEarlyTermThreshold());
            /* Output */
            case Parameters::RESULTS_FILE             : s = _parameters.getOutputFileName(); return s;
            case Parameters::RESULTS_HTML             : return AsString(s, _parameters.isGeneratingHtmlResults());
            case Parameters::RESULTS_CSV              : return AsString(s, _parameters.isGeneratingTableResults());
            case Parameters::RESULTS_ASN              : return AsString(s, _parameters.isGeneratingNCBIAsnResults());
            case Parameters::RESULTS_NWK              : return AsString(s, _parameters.isGeneratingNewickFile());
            // Advanced Output - Additional Output
            case Parameters::RESULTS_LLR              : return AsString(s, _parameters.isGeneratingLLRResults());
            case Parameters::REPORT_CRITICAL_VALUES   : return AsString(s, _parameters.getReportCriticalValues());
            case Parameters::REPORT_ATTR_RISK         : return AsString(s, _parameters.getReportAttributableRisk());
            case Parameters::ATTR_RISK_NUM_EXPOSED    : return AsString(s, _parameters.getAttributableRiskExposed());
            case Parameters::INCLUDE_IDENTICAL_PARENT_CUTS: return AsString(s, _parameters.getIncludeIdenticalParentCuts());
            case Parameters::OUTPUT_TEMPORAL_GRAPH: return AsString(s, _parameters.getOutputTemporalGraphFile());
            case Parameters::TEMPORAL_GRAPH_REPORT_TYPE: return AsString(s, _parameters.getTemporalGraphReportType());
            case Parameters::TEMPORAL_GRAPH_MLC_COUNT: return AsString(s, _parameters.getTemporalGraphMostLikelyCount());
            case Parameters::TEMPORAL_GRAPH_CUTOFF: return AsString(s, _parameters.getTemporalGraphSignificantCutoff());
            // Power Evaluations
            case Parameters::POWER_EVALUATIONS        : return AsString(s, _parameters.getPerformPowerEvaluations());
            case Parameters::POWER_EVALUATION_TYPE    : return AsString(s, _parameters.getPowerEvaluationType());
            case Parameters::CRITICAL_VALUES_TYPE     : return AsString(s, _parameters.getCriticalValuesType());
            case Parameters::CRITICAL_VALUE_05        : return AsString(s, _parameters.getCriticalValue05());
            case Parameters::CRITICAL_VALUE_01        : return AsString(s, _parameters.getCriticalValue01());
            case Parameters::CRITICAL_VALUE_001       : return AsString(s, _parameters.getCriticalValue001());
            case Parameters::POWER_EVALUATION_TOTALCASES : return AsString(s, _parameters.getPowerEvaluationTotalCases());
            case Parameters::POWER_EVALUATIONS_REPLICA : return AsString(s, _parameters.getPowerEvaluationReplications());
            case Parameters::POWER_EVALUATIONS_FILE    : s = _parameters.getPowerEvaluationAltHypothesisFilename(); return s;
            case Parameters::POWER_BASELINE_PROBABILITY : return AsString(s, _parameters.getPowerBaselineProbabilityRatio());
            case Parameters::POWER_Z                   : return AsString(s, _parameters.getPowerZ());
            // Power Simulations
            case Parameters::READ_SIMULATIONS         : return AsString(s, _parameters.isReadingSimulationData());
            case Parameters::INPUT_SIM_FILE           : s = _parameters.getInputSimulationsFilename(); return s;
            case Parameters::WRITE_SIMULATIONS        : return AsString(s, _parameters.isWritingSimulationData());
            case Parameters::OUTPUT_SIM_FILE          : s = _parameters.getOutputSimulationsFilename(); return s;
            // Runtime Options
            case Parameters::PARALLEL_PROCESSES       : return AsString(s, _parameters.getNumRequestedParallelProcesses());
            // System
            case Parameters::CREATION_VERSION         : printString(s, "%s.%s.%s", VERSION_MAJOR, VERSION_MINOR, VERSION_RELEASE); return s;
            case Parameters::PROSPECTIVE_FREQ_TYPE: return AsString(s, _parameters.getProspectiveFrequencyType());
            case Parameters::PROSPECTIVE_FREQ: return AsString(s, _parameters.getProspectiveFrequency());
            default : throw prg_error("Unknown parameter enumeration %d.","GetParameterComment()", e);
        };
    } catch (prg_exception& x) {
        x.addTrace("GetParameterComment()","AbtractParameterFileAccess");
        throw;
    }
}

/** Attempts to interpret passed string as a boolean value. Throws parameter_error. */
bool AbtractParameterFileAccess::ReadBoolean(const std::string& sValue, Parameters::ParameterType e) const {
  bool          bReadResult;

  if (sValue.size() == 0) {
    throw parameter_error("Invalid Parameter Setting:\nParameter '%s' is not set.\n", GetParameterLabel(e));
  }
  else if (!(!stricmp(sValue.c_str(),"y")   || !stricmp(sValue.c_str(),"n") ||
             !strcmp(sValue.c_str(),"1")    || !strcmp(sValue.c_str(),"0")   ||
             !strcmp(sValue.c_str(),"true")    || !strcmp(sValue.c_str(),"false")   ||
             !stricmp(sValue.c_str(),"yes")  || !stricmp(sValue.c_str(),"no"))) {
    throw parameter_error("Invalid Parameter Setting:\nFor parameter '%s', setting '%s' is invalid. Valid values are 'y' or 'n'.\n",
                          GetParameterLabel(e), sValue.c_str());
  }
  else
    bReadResult = (!stricmp(sValue.c_str(),"y") || !stricmp(sValue.c_str(),"yes") || !strcmp(sValue.c_str(),"1"));
  return bReadResult;
}

/** Attempts to interpret passed string as a double value. Throws parameter_error. */
double AbtractParameterFileAccess::ReadDouble(const std::string & sValue, Parameters::ParameterType e) const {
  double        dReadResult;

  if (sValue.size() == 0) {
    throw parameter_error("Invalid Parameter Setting:\nParameter '%s' is not set.\n", GetParameterLabel(e));
  }
  if (sscanf(sValue.c_str(), "%lf", &dReadResult) != 1) {
    throw parameter_error("Invalid Parameter Setting:\nFor parameter '%s', setting '%s' is not a valid real number.\n",
                          GetParameterLabel(e), sValue.c_str());
  }
  return dReadResult;
}


/** Attempts to validate integer as enumeration within specified range. Throws InvalidParameterException. */
int AbtractParameterFileAccess::ReadEnumeration(int iValue, Parameters::ParameterType e, int iLow, int iHigh) const {
  if (iValue < iLow || iValue > iHigh)
    throw parameter_error("Invalid Parameter Setting:\nFor parameter '%s', setting '%d' is out of range [%d,%d].\n",
                          GetParameterLabel(e), iValue, iLow, iHigh);
  return iValue;
}

/** Attempts to interpret passed string as an integer value. Throws InvalidParameterException. */
int AbtractParameterFileAccess::ReadInt(const std::string& sValue, Parameters::ParameterType e) const {
  int           iReadResult;

  if (sValue.size() == 0) {
    throw parameter_error("Invalid Parameter Setting:\nParameter '%s' is not set.\n", GetParameterLabel(e));
  }
  else if (sscanf(sValue.c_str(), "%i", &iReadResult) != 1) {
    throw parameter_error("Invalid Parameter Setting:\nFor parameter '%s', setting '%s' is not a valid integer.\n",
                          GetParameterLabel(e), sValue.c_str());
  }
  return iReadResult;
}

/** Attempts to interpret passed string as an integer value. Throws InvalidParameterException. */
int AbtractParameterFileAccess::ReadUnsignedInt(const std::string& sValue, Parameters::ParameterType e) const {
  int           iReadResult;

  if (sValue.size() == 0) {
    throw parameter_error("Invalid Parameter Setting:\nParameter '%s' is not set.\n", GetParameterLabel(e));
  }
  else if (sscanf(sValue.c_str(), "%u", &iReadResult) != 1) {
    throw parameter_error("Invalid Parameter Setting:\nFor parameter '%s', setting '%s' is not a valid integer.\n",
                          GetParameterLabel(e), sValue.c_str());
  }
  else if (iReadResult < 0) {
    throw parameter_error("Invalid Parameter Setting:\nFor parameter '%s', setting '%s' is not a positive integer.\n",
                          GetParameterLabel(e), sValue.c_str());
  }
  return iReadResult;
}

/** Attempts to interpret passed string as version number of format '#.#.#'. Throws InvalidParameterException. */
Parameters::CreationVersion AbtractParameterFileAccess::ReadVersion(const std::string& sValue) const {
  Parameters::CreationVersion v;

   if (sValue.size() == 0)
     throw parameter_error("Invalid Parameter Setting:\nParameter '%s' is not set.\n", GetParameterLabel(Parameters::CREATION_VERSION));
   else if (sscanf(sValue.c_str(), "%u.%u.%u", &v.iMajor, &v.iMinor, &v.iRelease) < 3)
     throw parameter_error("Invalid Parameter Setting:\nParameter '%s' is not set.\n", GetParameterLabel(Parameters::CREATION_VERSION));
   return v;
}

/** Attempts to interpret passed string as ratio of format '#/#' or decimal value. */
Parameters::ratio_t AbtractParameterFileAccess::ReadRatio(const std::string& sValue, Parameters::ParameterType e) const {
    if (sValue.size() == 0) throw parameter_error("Invalid Parameter Setting:\nParameter '%s' is not set.\n", GetParameterLabel(e));
    Parameters::ratio_t r;
    auto sepPos = sValue.find("/");
    double top;
    unsigned int bottom;
    if (sepPos == sValue.npos) { // No separator in value, try parsing as just a decimal value.
        if (!string_to_type<double>(sValue.c_str(), top))
            throw parameter_error("Invalid Parameter Setting:\nParameter '%s' could not be read as numeric value.\n", GetParameterLabel(e));
        r = Parameters::ratio_t(sValue, "1");
    } else {
        r = Parameters::ratio_t(sValue.substr(0, sepPos), sValue.substr(sepPos + 1, sValue.size()));
        if (!string_to_type<double>(r.first.c_str(), top) || !string_to_type<unsigned int>(r.second.c_str(), bottom))
            throw parameter_error("Invalid Parameter Setting:\nParameter '%s' could not be read as a ratio.\n", GetParameterLabel(e));
    }
    return r;
}

/** Calls appropriate read and set function for parameter type. */
void AbtractParameterFileAccess::SetParameter(Parameters::ParameterType e, const std::string& value, BasePrint& PrintDirection) {
    int iValue;

    try {
        switch (e) {
            // Input
            case Parameters::TREE_FILE                : _parameters.setTreeFileName(value.c_str(), true); break;
            case Parameters::COUNT_FILE               : _parameters.setCountFileName(value.c_str(), true); break;
            case Parameters::CONTROL_FILE             : _parameters.setControlFileName(value.c_str(), true); break;
            case Parameters::DATE_PRECISION           : iValue = ReadEnumeration(ReadInt(value, e), e, DataTimeRange::NONE, DataTimeRange::DAY);
                                                        _parameters.setDatePrecisionType((DataTimeRange::DatePrecisionType)iValue); break;
            case Parameters::DATA_TIME_RANGES         : _parameters.setDataTimeRangeStr(value.c_str()); break;
            // Advanced Input
            case Parameters::CUT_FILE                 : _parameters.setCutsFileName(value.c_str(), true); break;
            case Parameters::CUT_TYPE                 : iValue = ReadEnumeration(ReadInt(value, e), e, Parameters::SIMPLE, Parameters::COMBINATORIAL);
                                                        _parameters.setCutType((Parameters::CutType)iValue); break;
            case Parameters::DATA_ONLY_ON_LEAVES      : _parameters.setDataOnlyOnLeaves(ReadBoolean(value, e)); break;
            case Parameters::RELAXED_STUDY_DATA_PERIOD_CHECKING: _parameters.setRelaxedStudyDataPeriodChecking(ReadBoolean(value, e)); break;
            case Parameters::ALLOW_MULTI_PARENT_NODES : _parameters.setAllowMultiParentNodes(ReadBoolean(value, e)); break;
            case Parameters::ALLOW_MULTIPLE_ROOTS     : _parameters.setAllowMultipleRoots(ReadBoolean(value, e)); break;
            case Parameters::APPLY_RISK_WINDOW_RESTRICTION : _parameters.setApplyingRiskWindowRestriction(ReadBoolean(value, e)); break;
            case Parameters::RISK_WINDOW_PERCENTAGE   : _parameters.setRiskWindowPercentage(ReadDouble(value, e)); break;
            case Parameters::MINIMUM_CENSOR_TIME      : _parameters.setMinimumCensorTime(ReadUnsignedInt(value, e)); break;
            case Parameters::MINIMUM_CENSOR_PERCENTAGE: _parameters.setMinimumCensorPercentage(ReadUnsignedInt(value, e)); break;
            case Parameters::RSK_WND_CENSOR           : _parameters.setApplyingRiskWindowRestrictionCensored(ReadBoolean(value, e)); break;
            case Parameters::RSK_WND_ALT_CENSOR_DENOM : _parameters.setRiskWindowAltCensorDenominator(ReadDouble(value, e)); break;
            // Analysis
            case Parameters::SCAN_TYPE                : iValue = ReadEnumeration(ReadInt(value, e), e, Parameters::TREEONLY, Parameters::TIMEONLY);
                                                        _parameters.setScanType((Parameters::ScanType)iValue); break;
            case Parameters::CONDITIONAL_TYPE         : iValue = ReadEnumeration(ReadInt(value, e), e, Parameters::UNCONDITIONAL, Parameters::NODEANDTIME);
                                                        _parameters.setConditionalType((Parameters::ConditionalType)iValue); break;
            case Parameters::MODEL_TYPE               : iValue = ReadEnumeration(ReadInt(value, e), e, Parameters::POISSON, Parameters::BERNOULLI_TIME);
                                                        _parameters.setModelType((Parameters::ModelType)iValue); break;
            case Parameters::SELF_CONTROL_DESIGN      : _parameters.setSelfControlDesign(ReadBoolean(value, e)); break;
            case Parameters::EVENT_PROBABILITY        : _parameters.setProbabilityRatio(ReadRatio(value, Parameters::EVENT_PROBABILITY)); break;
            case Parameters::VARIABLE_CASE_PROBABILITY : _parameters.setVariableCaseProbability(ReadBoolean(value, e)); break;
            case Parameters::SEQUENTIAL_SCAN          : _parameters.setSequentialScan(ReadBoolean(value, e)); break;
            case Parameters::SEQUENTIAL_MAX_SIGNAL    : _parameters.setSequentialMaximumSignal(ReadUnsignedInt(value, e)); break;
            case Parameters::SEQUENTIAL_MIN_SIGNAL    : _parameters.setSequentialMinimumSignal(ReadUnsignedInt(value, e)); break;
            case Parameters::SEQUENTIAL_FILE          : _parameters.setSequentialFilename(value.c_str(), true); break;
            case Parameters::SEQUENTIAL_ALPHA_OVERALL : _parameters.setSequentialAlphaOverall(ReadDouble(value, e)); break;
            case Parameters::SEQUENTIAL_ALPHA_SPENDING: _parameters.setSequentialAlphaSpending(ReadDouble(value, e)); break;
            case Parameters::RESTRICTED_TIME_RANGE    : _parameters.setRestrictTemporalWindows(ReadBoolean(value, e)); break;
            case Parameters::START_DATA_TIME_RANGE    : _parameters.setTemporalStartRangeStr(value.c_str()); break;
            case Parameters::END_DATA_TIME_RANGE      : _parameters.setTemporalEndRangeStr(value.c_str()); break;
            case Parameters::SCAN_RATE_TYPE           : iValue = ReadEnumeration(ReadInt(value, e), e, Parameters::HIGHRATE, Parameters::HIGHORLOWRATE);
                                                        _parameters.setScanRateType((Parameters::ScanRateType)iValue); break;
            // Advanced Analysis - Temporal Window
            case Parameters::MAXIMUM_WINDOW_PERCENTAGE: _parameters.setMaximumWindowPercentage(ReadDouble(value, e)); break;
            case Parameters::MAXIMUM_WINDOW_FIXED     : _parameters.setMaximumWindowLength(ReadUnsignedInt(value, e)); break;
            case Parameters::MAXIMUM_WINDOW_TYPE      : iValue = ReadEnumeration(ReadInt(value, e), e, Parameters::PERCENTAGE_WINDOW, Parameters::FIXED_LENGTH);
                                                        _parameters.setMaximumWindowType((Parameters::MaximumWindowType)iValue); break;
            case Parameters::MINIMUM_WINDOW_FIXED     : _parameters.setMinimumWindowLength(ReadUnsignedInt(value, e)); break;
            case Parameters::PROSPECTIVE_ANALYSIS     : _parameters.setIsProspectiveAnalysis(ReadBoolean(value, e)); break;
            // Advanced Analysis - Adjustments
            case Parameters::DAYOFWEEK_ADJUSTMENT     : _parameters.setPerformDayOfWeekAdjustment(ReadBoolean(value, e)); break;
            case Parameters::APPLY_EXCLUSION_RANGES   : _parameters.setApplyingExclusionTimeRanges(ReadBoolean(value, e)); break;
            case Parameters::EXCLUSION_RANGES         : _parameters.setExclusionTimeRangeStr(value); break;
            // Advanced Analysis Inference
            case Parameters::REPLICATIONS             : _parameters.setNumReplications(ReadUnsignedInt(value, e)); break;
            case Parameters::RANDOMIZATION_SEED       : _parameters.setRandomizationSeed(static_cast<long>(ReadInt(value, e))); break;
            case Parameters::RANDOMLY_GENERATE_SEED   : _parameters.setRandomlyGeneratingSeed(ReadBoolean(value, e)); break;
            case Parameters::RESTRICT_TREE_LEVELS     : _parameters.setRestrictTreeLevels(ReadBoolean(value, e)); break;
            case Parameters::RESTRICTED_TREE_LEVELS   : {
                                                        std::vector<unsigned int> list;
                                                        if (!csv_string_to_typelist<unsigned int>(value.c_str(), list))
                                                            throw parameter_error("Invalid Parameter Setting:\nFor parameter '%s', unable to read as comma separated list of integers.\n", GetParameterLabel(e), value.c_str());
                                                            _parameters.setRestrictedTreeLevels(list);
                                                        }
                                                        break;
            case Parameters::RESTRICT_EVALUATED_NODES : _parameters.setRestrictEvaluatedTreeNodes(ReadBoolean(value, e)); break;
            case Parameters::NOT_EVALUATED_NODES_FILE : _parameters.setNotEvaluatedNodesFileName(value.c_str(), true); break;
            case Parameters::MINIMUM_CASES_NODE       : _parameters.setMinimumHighRateNodeCases(ReadUnsignedInt(value, e)); break;
            case Parameters::PVALUE_REPORT_TYPE       : iValue = ReadEnumeration(ReadInt(value, e), e, Parameters::STANDARD_PVALUE, Parameters::TERMINATION_PVALUE);
                                                        _parameters.setPValueReportingType((Parameters::PValueReportingType)iValue); break;
            case Parameters::EARLY_TERM_THRESHOLD     : _parameters.setEarlyTermThreshold(ReadUnsignedInt(value, e)); break;
            /* Output */
            case Parameters::RESULTS_FILE             : _parameters.setOutputFileName(value.c_str(), true); break;
            case Parameters::RESULTS_HTML             : _parameters.setGeneratingHtmlResults(ReadBoolean(value, e)); break;
            case Parameters::RESULTS_CSV              : _parameters.setGeneratingTableResults(ReadBoolean(value, e)); break;
            case Parameters::RESULTS_ASN              : _parameters.setGeneratingNCBIAsnResults(ReadBoolean(value, e)); break;
            case Parameters::RESULTS_NWK              : _parameters.setGeneratingNewickFile(ReadBoolean(value, e)); break;
            // Advanced Output - Additional Output
            case Parameters::RESULTS_LLR              : _parameters.setGeneratingLLRResults(ReadBoolean(value, e)); break;
            case Parameters::REPORT_CRITICAL_VALUES   : _parameters.setReportCriticalValues(ReadBoolean(value, e)); break;
            case Parameters::REPORT_ATTR_RISK         : _parameters.setReportAttributableRisk(ReadBoolean(value, e)); break;
            case Parameters::ATTR_RISK_NUM_EXPOSED    : _parameters.setAttributableRiskExposed(ReadUnsignedInt(value, e)); break;
            case Parameters::INCLUDE_IDENTICAL_PARENT_CUTS: _parameters.setIncludeIdenticalParentCuts(ReadBoolean(value, e)); break;
            case Parameters::OUTPUT_TEMPORAL_GRAPH:     _parameters.setOutputTemporalGraphFile(ReadBoolean(value, e)); break;
            case Parameters::TEMPORAL_GRAPH_REPORT_TYPE: iValue = ReadEnumeration(ReadInt(value, e), e, Parameters::MLC_ONLY, Parameters::SIGNIFICANT_ONLY);
                                                         _parameters.setTemporalGraphReportType((Parameters::TemporalGraphReportType)iValue); break;
            case Parameters::TEMPORAL_GRAPH_MLC_COUNT : _parameters.setTemporalGraphMostLikelyCount(ReadUnsignedInt(value, e)); break;
            case Parameters::TEMPORAL_GRAPH_CUTOFF    : _parameters.setTemporalGraphSignificantCutoff(ReadDouble(value, e)); break;
            // Power Evaluations
            case Parameters::POWER_EVALUATIONS        : _parameters.setPerformPowerEvaluations(ReadBoolean(value, e)); break;
            case Parameters::POWER_EVALUATION_TYPE    : iValue = ReadEnumeration(ReadInt(value, e), e, Parameters::PE_WITH_ANALYSIS, Parameters::PE_ONLY_SPECIFIED_CASES);
                                                        _parameters.setPowerEvaluationType((Parameters::PowerEvaluationType)iValue); break;
            case Parameters::CRITICAL_VALUES_TYPE     : iValue = ReadEnumeration(ReadInt(value, e), e, Parameters::CV_MONTECARLO, Parameters::CV_POWER_VALUES);
                                                        _parameters.setCriticalValuesType((Parameters::CriticalValuesType)iValue); break;
            case Parameters::CRITICAL_VALUE_05        : _parameters.setCriticalValue05(ReadDouble(value, e)); break;
            case Parameters::CRITICAL_VALUE_01        : _parameters.setCriticalValue01(ReadDouble(value, e)); break;
            case Parameters::CRITICAL_VALUE_001       : _parameters.setCriticalValue001(ReadDouble(value, e)); break;
            case Parameters::POWER_EVALUATION_TOTALCASES : _parameters.setPowerEvaluationTotalCases(ReadInt(value, e)); break;
            case Parameters::POWER_EVALUATIONS_REPLICA : _parameters.setPowerEvaluationReplications(ReadUnsignedInt(value, e)); break;
            case Parameters::POWER_EVALUATIONS_FILE   : _parameters.setPowerEvaluationAltHypothesisFilename(value.c_str(), true); break;
            case Parameters::POWER_BASELINE_PROBABILITY : _parameters.setPowerBaselineProbabilityRatio(ReadRatio(value, Parameters::POWER_BASELINE_PROBABILITY)); break;
            case Parameters::POWER_Z                  : _parameters.setPowerZ(ReadDouble(value, e)); break;
            // Power Simulations
            case Parameters::READ_SIMULATIONS         : _parameters.setReadingSimulationData(ReadBoolean(value, e)); break;
            case Parameters::INPUT_SIM_FILE           : _parameters.setInputSimulationsFilename(value.c_str(), true); break;
            case Parameters::WRITE_SIMULATIONS        : _parameters.setWritingSimulationData(ReadBoolean(value, e)); break;
            case Parameters::OUTPUT_SIM_FILE          : _parameters.setOutputSimulationsFilename(value.c_str(), true); break;
            // Run Options
            case Parameters::PARALLEL_PROCESSES       : _parameters.setNumProcesses(ReadUnsignedInt(value, e)); break;
            // System
            case Parameters::CREATION_VERSION         : _parameters.setVersion(ReadVersion(value)); break;
            case Parameters::PROSPECTIVE_FREQ_TYPE    : iValue = ReadEnumeration(ReadInt(value, e), e, Parameters::DAILY, Parameters::YEARLY);
                                                        _parameters.setProspectiveFrequencyType((Parameters::ProspectiveFrequency)iValue); break;
            case Parameters::PROSPECTIVE_FREQ         : _parameters.setProspectiveFrequency(ReadUnsignedInt(value, e)); break;
            default : throw parameter_error("Unknown parameter enumeration %d.", e);
        };
    } catch (parameter_error &x) {
        _read_error = true;
        PrintDirection.Printf(x.what(), BasePrint::P_PARAMERROR);
    } catch (prg_exception &x) {
        x.addTrace("SetParameter()","AbtractParameterFileAccess");
        throw;
    }
}

/** Attempts to write values to InputSource. */
Parameters::InputSource & AbtractParameterFileAccess::setInputSource(Parameters::InputSource & source,
                                                                     const std::string& typeStr, 
                                                                     const std::string& mapStr, 
                                                                     const std::string& delimiterStr, 
                                                                     const std::string& groupStr, 
                                                                     const std::string& skipStr,
                                                                     const std::string& headerStr,
                                                                     BasePrint& PrintDirection) {
    try {
        // set defaults
        source.setSourceType(CSV);
        source.clearFieldsMap();
        source.setDelimiter(" ");
        source.setGroup("\"");
        source.setSkip(0);
        source.setFirstRowHeader(false);

        // source file type
        if (typeStr.size()) {
            int type;
            if (!string_to_type<int>(typeStr.c_str(), type))
                throw resolvable_error("Unable to read parameter value '%s' as %s.", typeStr.c_str(), IniParameterSpecification::SourceType);
            if (type < CSV || type > CSV)
                throw resolvable_error("Parameter value '%d' out of range [%d,%d] for %s.", type, CSV, CSV, IniParameterSpecification::SourceType);
            source.setSourceType((SourceType)type);
        }
        // fields map
        if (mapStr.size()) {
            int column;
            FieldMapContainer_t fields_map;
            boost::escaped_list_separator<char> separator('\\', ',', '\"');
            boost::tokenizer<boost::escaped_list_separator<char> > mappings(mapStr, separator);
            for (boost::tokenizer<boost::escaped_list_separator<char> >::const_iterator itr=mappings.begin(); itr != mappings.end(); ++itr) {
                std::string token(*itr);
                trimString(token);
                if (string_to_type<int>(token.c_str(), column)) {
                    fields_map.push_back((long)column);
                } else {
                    throw resolvable_error("Unable to read parameter value '%s' as %s item.", token.c_str(), IniParameterSpecification::SourceFieldMap);
                }
            }
            source.setFieldsMap(fields_map);
        }
        if (source.getSourceType() == CSV) {
            source.setDelimiter(delimiterStr.size() == 0 ? " " : delimiterStr);
            if (source.getDelimiter().size() > 1)
                throw resolvable_error("The %s value settings is limited to 1 character. Values specified is '%s'.", IniParameterSpecification::SourceDelimiter, source.getDelimiter().c_str());
            source.setGroup(groupStr.size() == 0 ? "\"" : groupStr);
            if (source.getGroup().size() > 1)
                throw resolvable_error("The %s value settings is limited to 1 character. Values specified is '%s'.", IniParameterSpecification::SourceDelimiter, source.getGroup().c_str());
            unsigned int skip=0;
            if (skipStr.size() > 0 && !string_to_type<unsigned int>(skipStr.c_str(), skip)) {
                throw resolvable_error("Unable to read parameter value '%s' as %s.", skipStr.c_str(), IniParameterSpecification::SourceSkip);
            }
            source.setSkip(skip);
            bool rowheader = false;
            if (headerStr.size()) {
                if (!(!stricmp(headerStr.c_str(),"y")   || !stricmp(headerStr.c_str(),"n") ||
                    !strcmp(headerStr.c_str(),"1")    || !strcmp(headerStr.c_str(),"0")   ||
                    !stricmp(headerStr.c_str(),"yes")  || !stricmp(headerStr.c_str(),"no"))) {
                    throw resolvable_error("Unable to read parameter value '%s' as %s.", headerStr.c_str(), IniParameterSpecification::SourceFirstRowHeader);
                }
                rowheader = (!stricmp(headerStr.c_str(),"y") || !stricmp(headerStr.c_str(),"yes") || !strcmp(headerStr.c_str(),"1"));
            }
            source.setFirstRowHeader(rowheader);
        }
    } catch (resolvable_error &x) {
        _read_error = true;
        PrintDirection.Printf(x.what(), BasePrint::P_PARAMERROR);
    } catch (prg_exception &x) {
        x.addTrace("setInputSource()","AbtractParameterFileAccess");
        throw;
    }
    return source;
}

/////////////////////  parameter_error ///////////////////////////////////////////////////////////////

parameter_error::parameter_error(const char * format, ...) : resolvable_error() {
    try {
    #ifdef _MSC_VER
        std::vector<char> temp(MSC_VSNPRINTF_DEFAULT_BUFFER_SIZE);
        va_list varArgs;
        va_start (varArgs, format);
        vsnprintf(&temp[0], temp.size() - 1, format, varArgs);
        va_end(varArgs);
    #else
        std::vector<char> temp(1);
        va_list varArgs;
        va_start(varArgs, format);
        size_t iStringLength = vsnprintf(&temp[0], temp.size(), format, varArgs);
        va_end(varArgs);
        temp.resize(iStringLength + 1);
        va_start(varArgs, format);
        vsnprintf(&temp[0], iStringLength + 1, format, varArgs);
        va_end(varArgs);
    #endif
        _what = &temp[0];
    } catch (...) {}
}
