//***************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//***************************************************************************
#include "ParameterFileAccess.h"
#include "IniParameterFileAccess.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/detail/xml_parser_error.hpp>

//////////// ParameterAccessCoordinator //////////////////////////////////////

/** Determines format of parameter file and invokes particular parameter reader class to read parameters from file. */
bool ParameterAccessCoordinator::read(const std::string& filename, BasePrint& print) {
    bool  success=false;
    try {
        if (access(filename.c_str(), 04) == -1)
            throw resolvable_error("Unable to open settings file '%s'.\n", filename.c_str());
        success = IniParameterFileAccess(_parameters, print).Read(filename.c_str());
    } catch (prg_exception& x) {
        throw resolvable_error("Unable to read parameters from file '%s'.\n", filename.c_str());
    }
    return success;
}

/** Writes parameters to file in most recent format. */
void ParameterAccessCoordinator::write(const std::string& filename, BasePrint& print) {
    IniParameterFileAccess(_parameters, print).Write(filename.c_str());
}

//////////////////// AbtractParameterFileAccess ////////////////////////////////

/** constructor */
AbtractParameterFileAccess::AbtractParameterFileAccess(Parameters& Parameters, BasePrint& PrintDirection, bool bWriteBooleanAsDigit)
                           :_parameters(Parameters), _read_error(false), gPrintDirection(PrintDirection), _write_boolean_as_digit(bWriteBooleanAsDigit) {}

/** destructor */
AbtractParameterFileAccess::~AbtractParameterFileAccess() {}

/** Returns constant char pointer to parameters comment string. */
const char * AbtractParameterFileAccess::GetParameterComment(Parameters::ParameterType e) const {
    try {
        switch (e) {
            /* Input */
            case Parameters::TREE_FILE               : return "tree structure filename";
            case Parameters::COUNT_FILE              : return "count data filename";
            case Parameters::DATA_TIME_RANGES        : return "data time ranges (integer - integer)";
            /* Advanced Input */
            case Parameters::CUT_FILE                : return "cuts filename";
            case Parameters::CUT_TYPE                : return "default cuts type (SIMPLE=0, PAIRS=1, TRIPLETS=2, ORDINAL=3, COMBINATORIAL=4)";
            case Parameters::DUPLICATES              : return "duplicates in case data records (y/n -- experimental)";
            /* Analysis */
            case Parameters::SCAN_TYPE               : return "scan type (TREEONLY=0, TREETIME=1, TIMEONLY=2)";
            case Parameters::CONDITIONAL_TYPE        : return "conditional type (UNCONDITIONAL=0, TOTALCASES=1, NODE=2, NODEANDTIME=3)";
            case Parameters::MODEL_TYPE              : return "probability model type (POISSON=0, BERNOULLI=1, UNIFORM=2, Not-Applicable=3)";
            case Parameters::EVENT_PROBABILITY       : return "case probability (integer / integer)";
            case Parameters::START_DATA_TIME_RANGE   : return "start data time range (integer - integer)";
            case Parameters::END_DATA_TIME_RANGE     : return "end data time range (integer - integer)";
            /* Advanced Analysis - Temporal Window */
            case Parameters::MAXIMUM_WINDOW_PERCENTAGE : return "maximum temporal size as percentage of data time range (0 < x <= 50.0)";
            case Parameters::MAXIMUM_WINDOW_FIXED    : return "maximum temporal size as fixed time length (integer)";
            case Parameters::MAXIMUM_WINDOW_TYPE     : return "maximum temporal size selection (PERCENTAGE_WINDOW=0, FIXED_LENGTH=1)";
            case Parameters::MINIMUM_WINDOW_FIXED    : return "minimum temporal size as fixed time length (integer)";
            /* Advanced Analysis - Adjustments */
            case Parameters::DAYOFWEEK_ADJUSTMENT    : return "perform day of week adjustments (y/n)";
            /* Advanced Analysis - Inference */
            case Parameters::REPLICATIONS            : return "number of simulation replications (0, 9, 999, n999)";
            case Parameters::RANDOMIZATION_SEED      : return "randomization seed (integer)";
            case Parameters::RANDOMLY_GENERATE_SEED  : return "generate randomization seed (y/n)";
            /* Output */
            case Parameters::RESULTS_FILE            : return "results filename";
            case Parameters::RESULTS_HTML            : return "create HTML results (y/n)";
            case Parameters::RESULTS_CSV             : return "create CSV results (y/n)";
            case Parameters::RESULTS_LLR             : return "create LLR results (y/n)";
            case Parameters::REPORT_CRITICAL_VALUES  : return "report critical values (y/n)";
            /* Power Evaluations */
            case Parameters::POWER_EVALUATIONS       : return "perform power evaluations (y/n)";
            case Parameters::POWER_EVALUATION_TYPE   : return "power evaluation type (0=Analysis And Power Evaluation Together, 1=Only Power Evaluation With Count File, 2=Only Power Evaluation With Defined Total Cases)";
            case Parameters::CRITICAL_VALUES_TYPE    : return "critical values type (0=Monte Carlo, 1=User Specified Values)";
            case Parameters::CRITICAL_VALUE_05       : return "power evaluation critical value .05 (> 0)";
            case Parameters::CRITICAL_VALUE_01       : return "power evaluation critical value .01 (> 0)";
            case Parameters::CRITICAL_VALUE_001      : return "power evaluation critical value .001 (> 0)";
            case Parameters::POWER_EVALUATION_TOTALCASES : return "total cases in power evaluation (integer)";
            case Parameters::POWER_EVALUATIONS_REPLICA : return "number of replications in power step (integer)";
            case Parameters::POWER_EVALUATIONS_FILE : return "power evaluation alternative hypothesis filename";
            /* Power Simulations */
            case Parameters::READ_SIMULATIONS        : return "input simulation data (y/n)";
            case Parameters::INPUT_SIM_FILE          : return "input simulation filename";
            case Parameters::WRITE_SIMULATIONS       : return "output simulation data (y/n)";
            case Parameters::OUTPUT_SIM_FILE         : return "output simulation filename";
            /* Runtime Options */
            case Parameters::PARALLEL_PROCESSES      : return "number of parallel processes to execute (0=All Processors, x=At Most X Processors)";
            /* System */
            case Parameters::CREATION_VERSION        : return "parameters version - do not modify";
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
            /* Input */
            case Parameters::TREE_FILE                : s = _parameters.getTreeFileName(); return s;
            case Parameters::COUNT_FILE               : s = _parameters.getCountFileName(); return s;
            case Parameters::DATA_TIME_RANGES         : return _parameters.getDataTimeRangeSet().toString(s);
            /* Advanced Input */
            case Parameters::CUT_FILE                 : s = _parameters.getCutsFileName(); return s;
            case Parameters::CUT_TYPE                 : return AsString(s, _parameters.getCutType());
            case Parameters::DUPLICATES               : return AsString(s, _parameters.isDuplicates());
            /* Analysis */
            case Parameters::SCAN_TYPE                : return AsString(s, _parameters.getScanType());
            case Parameters::CONDITIONAL_TYPE         : return AsString(s, _parameters.getConditionalType());
            case Parameters::MODEL_TYPE               : return AsString(s, _parameters.getModelType());
            case Parameters::EVENT_PROBABILITY        : return AsString(s, _parameters.getProbabilityRatio());
            case Parameters::START_DATA_TIME_RANGE    : return _parameters.getTemporalStartRange().toString(s);
            case Parameters::END_DATA_TIME_RANGE      : return _parameters.getTemporalEndRange().toString(s);
            /* Advanced Analysis - Temporal Window */
            case Parameters::MAXIMUM_WINDOW_PERCENTAGE: return AsString(s, _parameters.getMaximumWindowPercentage());
            case Parameters::MAXIMUM_WINDOW_FIXED     : return AsString(s, _parameters.getMaximumWindowLength());
            case Parameters::MAXIMUM_WINDOW_TYPE      : return AsString(s, _parameters.getMaximumWindowType());
            case Parameters::MINIMUM_WINDOW_FIXED     : return AsString(s, _parameters.getMinimumWindowLength());
            /* Advanced Analysis - Adjustments */
            case Parameters::DAYOFWEEK_ADJUSTMENT    : return AsString(s, _parameters.getPerformDayOfWeekAdjustment());
            /* Advanced Analysis - Inference */
            case Parameters::REPLICATIONS             : return AsString(s, _parameters.getNumReplicationsRequested());
            case Parameters::RANDOMIZATION_SEED       : return AsString(s, static_cast<unsigned int>(_parameters.getRandomizationSeed()));
            case Parameters::RANDOMLY_GENERATE_SEED   : return AsString(s, _parameters.isRandomlyGeneratingSeed());
            /* Output */
            case Parameters::RESULTS_FILE             : s = _parameters.getOutputFileName(); return s;
            case Parameters::RESULTS_HTML             : return AsString(s, _parameters.isGeneratingHtmlResults());
            case Parameters::RESULTS_CSV              : return AsString(s, _parameters.isGeneratingTableResults());
            case Parameters::RESULTS_LLR              : return AsString(s, _parameters.isGeneratingLLRResults());
            case Parameters::REPORT_CRITICAL_VALUES   : return AsString(s, _parameters.getReportCriticalValues());
            /* Power Evaluations */
            case Parameters::POWER_EVALUATIONS        : return AsString(s, _parameters.getPerformPowerEvaluations());
            case Parameters::POWER_EVALUATION_TYPE    : return AsString(s, _parameters.getPowerEvaluationType());
            case Parameters::CRITICAL_VALUES_TYPE     : return AsString(s, _parameters.getCriticalValuesType());
            case Parameters::CRITICAL_VALUE_05        : return AsString(s, _parameters.getCriticalValue05());
            case Parameters::CRITICAL_VALUE_01        : return AsString(s, _parameters.getCriticalValue01());
            case Parameters::CRITICAL_VALUE_001       : return AsString(s, _parameters.getCriticalValue001());
            case Parameters::POWER_EVALUATION_TOTALCASES : return AsString(s, _parameters.getPowerEvaluationTotalCases());
            case Parameters::POWER_EVALUATIONS_REPLICA : return AsString(s, _parameters.getPowerEvaluationReplications());
            case Parameters::POWER_EVALUATIONS_FILE    : s = _parameters.getPowerEvaluationAltHypothesisFilename(); return s;
            /* Power Simulations */
            case Parameters::READ_SIMULATIONS         : return AsString(s, _parameters.isReadingSimulationData());
            case Parameters::INPUT_SIM_FILE           : s = _parameters.getInputSimulationsFilename(); return s;
            case Parameters::WRITE_SIMULATIONS        : return AsString(s, _parameters.isWritingSimulationData());
            case Parameters::OUTPUT_SIM_FILE          : s = _parameters.getOutputSimulationsFilename(); return s;
            /* Runtime Options */
            case Parameters::PARALLEL_PROCESSES       : return AsString(s, _parameters.getNumRequestedParallelProcesses());
            /* System */
            case Parameters::CREATION_VERSION         : printString(s, "%s.%s.%s", VERSION_MAJOR, VERSION_MINOR, VERSION_RELEASE); return s;
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

/** Attempts to interpret passed string as ratio of format '#/#'. Throws InvalidParameterException. */
Parameters::ratio_t AbtractParameterFileAccess::ReadRatio(const std::string& sValue) const {
    Parameters::ratio_t r;

   if (sValue.size() == 0)
     throw parameter_error("Invalid Parameter Setting:\nParameter '%s' is not set.\n", GetParameterLabel(Parameters::EVENT_PROBABILITY));
   else if (sscanf(sValue.c_str(), "%u/%u", &r.first, &r.second) < 2)
     throw parameter_error("Invalid Parameter Setting:\nParameter '%s' is not set.\n", GetParameterLabel(Parameters::EVENT_PROBABILITY));
  return r;
}

/** Calls appropriate read and set function for parameter type. */
void AbtractParameterFileAccess::SetParameter(Parameters::ParameterType e, const std::string& value, BasePrint& PrintDirection) {
    int iValue;

    try {
        switch (e) {
            /* Input */
            case Parameters::TREE_FILE                : _parameters.setTreeFileName(value.c_str(), true); break;
            case Parameters::COUNT_FILE               : _parameters.setCountFileName(value.c_str(), true); break;
            case Parameters::DATA_TIME_RANGES         : _parameters.setDataTimeRangeSet(DataTimeRangeSet(value)); break;
            /* Advanced Input */
            case Parameters::CUT_FILE                 : _parameters.setCutsFileName(value.c_str(), true); break;
            case Parameters::CUT_TYPE                 : iValue = ReadEnumeration(ReadInt(value, e), e, Parameters::SIMPLE, Parameters::COMBINATORIAL);
                                                        _parameters.setCutType((Parameters::CutType)iValue); break;
            case Parameters::DUPLICATES               : _parameters.setDuplicates(ReadBoolean(value, e)); break;
            /* Analysis */
            case Parameters::SCAN_TYPE                : iValue = ReadEnumeration(ReadInt(value, e), e, Parameters::TREEONLY, Parameters::TIMEONLY);
                                                       _parameters.setScanType((Parameters::ScanType)iValue); break;
            case Parameters::CONDITIONAL_TYPE         : iValue = ReadEnumeration(ReadInt(value, e), e, Parameters::UNCONDITIONAL, Parameters::NODEANDTIME);
                                                       _parameters.setConditionalType((Parameters::ConditionalType)iValue); break;
            case Parameters::MODEL_TYPE               : iValue = ReadEnumeration(ReadInt(value, e), e, Parameters::POISSON, Parameters::MODEL_NOT_APPLICABLE);
                                                       _parameters.setModelType((Parameters::ModelType)iValue); break;
            case Parameters::EVENT_PROBABILITY        : _parameters.setProbabilityRatio(ReadRatio(value)); break;
            case Parameters::START_DATA_TIME_RANGE    : _parameters.setTemporalStartRange(DataTimeRange(value)); break;
            case Parameters::END_DATA_TIME_RANGE      : _parameters.setTemporalEndRange(DataTimeRange(value)); break;
            /* Advanced Analysis - Temporal Window */
            case Parameters::MAXIMUM_WINDOW_PERCENTAGE: _parameters.setMaximumWindowPercentage(ReadDouble(value, e)); break;
            case Parameters::MAXIMUM_WINDOW_FIXED     : _parameters.setMaximumWindowLength(ReadUnsignedInt(value, e)); break;
            case Parameters::MAXIMUM_WINDOW_TYPE      : iValue = ReadEnumeration(ReadInt(value, e), e, Parameters::PERCENTAGE_WINDOW, Parameters::FIXED_LENGTH);
                                                        _parameters.setMaximumWindowType((Parameters::MaximumWindowType)iValue); break;
            case Parameters::MINIMUM_WINDOW_FIXED     : _parameters.setMinimumWindowLength(ReadUnsignedInt(value, e)); break;
            /* Advanced Analysis - Adjustments */
            case Parameters::DAYOFWEEK_ADJUSTMENT    : _parameters.setPerformDayOfWeekAdjustment(ReadBoolean(value, e)); break;
            /* Advanced Analysis Inference */
            case Parameters::REPLICATIONS             : _parameters.setNumReplications(ReadUnsignedInt(value, e)); break;
            case Parameters::RANDOMIZATION_SEED       : _parameters.setRandomizationSeed(static_cast<long>(ReadInt(value, e))); break;
            case Parameters::RANDOMLY_GENERATE_SEED   : _parameters.setRandomlyGeneratingSeed(ReadBoolean(value, e)); break;
            /* Output */
            case Parameters::RESULTS_FILE             : _parameters.setOutputFileName(value.c_str(), true); break;
            case Parameters::RESULTS_HTML             : _parameters.setGeneratingHtmlResults(ReadBoolean(value, e)); break;
            case Parameters::RESULTS_CSV              : _parameters.setGeneratingTableResults(ReadBoolean(value, e)); break;
            case Parameters::RESULTS_LLR              : _parameters.setGeneratingLLRResults(ReadBoolean(value, e)); break;
            case Parameters::REPORT_CRITICAL_VALUES   : _parameters.setReportCriticalValues(ReadBoolean(value, e)); break;
            /* Power Evaluations */
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
            /* Power Simulations */
            case Parameters::READ_SIMULATIONS         : _parameters.setReadingSimulationData(ReadBoolean(value, e)); break;
            case Parameters::INPUT_SIM_FILE           : _parameters.setInputSimulationsFilename(value.c_str(), true); break;
            case Parameters::WRITE_SIMULATIONS        : _parameters.setWritingSimulationData(ReadBoolean(value, e)); break;
            case Parameters::OUTPUT_SIM_FILE          : _parameters.setOutputSimulationsFilename(value.c_str(), true); break;
            /* Run Options */
            case Parameters::PARALLEL_PROCESSES       : _parameters.setNumProcesses(ReadUnsignedInt(value, e)); break;
            /* System */
            case Parameters::CREATION_VERSION         : _parameters.setVersion(ReadVersion(value)); break;
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
