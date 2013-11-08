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
            case Parameters::COUNT_FILE              : return "case data filename";
            case Parameters::POPULATION_FILE         : return "population data filename";
            case Parameters::DATA_TIME_RANGES        : return "data time ranges (integer - integer)";
            /* Advanced Input */
            case Parameters::CUT_FILE                : return "cuts filename";
            case Parameters::CUT_TYPE                : return "default cuts type (SIMPLE=0, PAIRS=1, TRIPLETS=2, ORDINAL=3, COMBINATORIAL=4)";
            case Parameters::DUPLICATES              : return "duplicates in case data records";
            /* Analysis */
            case Parameters::SCAN_TYPE               : return "scan type (TREEONLY=0, TREETIME)";
            case Parameters::CONDITIONAL_TYPE        : return "conditional type (UNCONDITIONAL=0, TOTALCASES, CASESEACHBRANCH)";
            case Parameters::MODEL_TYPE              : return "probability model type (POISSON=0, BERNOULLI=1, TEMPORALSCAN=3)";
            case Parameters::EVENT_PROBABILITY       : return "event probability (integer / integer)";
            case Parameters::START_DATA_TIME_RANGE   : return "start data time range (integer - integer)";
            case Parameters::END_DATA_TIME_RANGE     : return "end data time range (integer - integer)";
            /* Advanced Analysis */
            case Parameters::REPLICATIONS            : return "number of simulation replications (0,9,999, n999)";
            case Parameters::RANDOMIZATION_SEED      : return "randomization seed (integer)";
            case Parameters::RANDOMLY_GENERATE_SEED  : return "generate randomization seed (y/n)";
            /* Output */
            case Parameters::RESULTS_FILE            : return "results filename";
            case Parameters::RESULTS_HTML            : return "create HTML results";
            case Parameters::RESULTS_CSV             : return "create CSV results";
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
            case Parameters::POPULATION_FILE          : s = _parameters.getPopulationFileName(); return s;
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
            /* Advanced Analysis */
            case Parameters::REPLICATIONS             : return AsString(s, _parameters.getNumReplicationsRequested());
            case Parameters::RANDOMIZATION_SEED       : return AsString(s, _parameters.getRandomizationSeed());
            case Parameters::RANDOMLY_GENERATE_SEED   : return AsString(s, _parameters.isRandomlyGeneratingSeed());
            /* Output */
            case Parameters::RESULTS_FILE             : s = _parameters.getOutputFileName(); return s;
            case Parameters::RESULTS_HTML             : return AsString(s, _parameters.isGeneratingHtmlResults());
            case Parameters::RESULTS_CSV              : return AsString(s, _parameters.isGeneratingTableResults());
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
            case Parameters::TREE_FILE                : _parameters.setTreeFileName(value.c_str()); break;
            case Parameters::COUNT_FILE               : _parameters.setCountFileName(value.c_str()); break;
            case Parameters::POPULATION_FILE          : _parameters.setPopulationFileName(value.c_str()); break;
            case Parameters::DATA_TIME_RANGES         : _parameters.setDataTimeRangeSet(DataTimeRangeSet(value)); break;
            /* Advanced Input */
            case Parameters::CUT_FILE                 : _parameters.setCutsFileName(value.c_str()); break;
            case Parameters::CUT_TYPE                 : iValue = ReadEnumeration(ReadInt(value, e), e, Parameters::SIMPLE, Parameters::COMBINATORIAL);
                                                        _parameters.setCutType((Parameters::CutType)iValue); break;
            case Parameters::DUPLICATES               : _parameters.setDuplicates(ReadBoolean(value, e)); break;
            /* Analysis */
            case Parameters::SCAN_TYPE                : iValue = ReadEnumeration(ReadInt(value, e), e, Parameters::TREEONLY, Parameters::TREETIME);
                                                       _parameters.setScanType((Parameters::ScanType)iValue); break;
            case Parameters::CONDITIONAL_TYPE         : iValue = ReadEnumeration(ReadInt(value, e), e, Parameters::UNCONDITIONAL, Parameters::CASESEACHBRANCH);
                                                       _parameters.setConditionalType((Parameters::ConditionalType)iValue); break;
            case Parameters::MODEL_TYPE               : iValue = ReadEnumeration(ReadInt(value, e), e, Parameters::POISSON, Parameters::TEMPORALSCAN);
                                                       _parameters.setModelType((Parameters::ModelType)iValue); break;
            case Parameters::EVENT_PROBABILITY        : _parameters.setProbabilityRatio(ReadRatio(value)); break;
            case Parameters::START_DATA_TIME_RANGE    : _parameters.setTemporalStartRange(DataTimeRange(value)); break;
            case Parameters::END_DATA_TIME_RANGE      : _parameters.setTemporalEndRange(DataTimeRange(value)); break;
            /* Advanced Analysis */
            case Parameters::REPLICATIONS             : _parameters.setNumReplications(ReadUnsignedInt(value, e)); break;
            case Parameters::RANDOMIZATION_SEED       : _parameters.setRandomizationSeed(ReadInt(value, e)); break;
            case Parameters::RANDOMLY_GENERATE_SEED   : _parameters.setRandomlyGeneratingSeed(ReadBoolean(value, e)); break;
            /* Output */
            case Parameters::RESULTS_FILE             : _parameters.setOutputFileName(value.c_str()); break;
            case Parameters::RESULTS_HTML             : _parameters.setGeneratingHtmlResults(ReadBoolean(value, e)); break;
            case Parameters::RESULTS_CSV              : _parameters.setGeneratingTableResults(ReadBoolean(value, e)); break;
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

