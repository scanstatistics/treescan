//***************************************************************************
#include "TreeScan.h"
#pragma hdrstop                                                        
//***************************************************************************
#include "IniParameterFileAccess.h"
#include "RandomNumberGenerator.h"
                                                                       
/** constructor */
IniParameterFileAccess::IniParameterFileAccess(Parameters& Parameters, BasePrint& PrintDirection)
                       :AbtractParameterFileAccess(Parameters, PrintDirection), gpSpecifications(0) {}

/** destructor */
IniParameterFileAccess::~IniParameterFileAccess() {
    try {
        delete gpSpecifications;
    } catch (...){}
}

/** Returns key string for specified parameter type. */
const char * IniParameterFileAccess::GetParameterLabel(Parameters::ParameterType e) const {
    const char * sSectionName, * sKey;
    GetSpecifications().GetParameterIniInfo(e,  &sSectionName, &sKey);
    return sKey;
}

/** Return reference to IniParameterSpecification object. Throws exception if not allocated. */
const IniParameterSpecification & IniParameterFileAccess::GetSpecifications() const {
    try {
        if (!gpSpecifications)
            throw prg_error("Specifications object not allocated.", "GetSpecifications()");
    } catch (prg_exception& x) {
        x.addTrace("GetSpecifications()","IniParameterFileAccess");
        throw;
    }
    return *gpSpecifications;
}

/** Reads parameters from ini file and sets associated CParameter object. */
bool IniParameterFileAccess::Read(const char* sFilename) {
    try {
        _missing_defaulted.clear();
        _read_error = false;
        _parameters.setAsDefaulted();
        _parameters.setSourceFileName(sFilename);

        IniFile SourceFile;
        SourceFile.Read(_parameters.getSourceFileName());
        gpSpecifications = new IniParameterSpecification(SourceFile, _parameters);

        for (Parameters::ParameterType eType=Parameters::TREE_FILE; eType <= _parameters.giNumParameters; eType = Parameters::ParameterType(eType + 1))
            ReadIniParameter(SourceFile, eType);
    } catch (prg_exception& x) {
        x.addTrace("Read()","IniParameterFileAccess");
        throw;
    }
    return !_read_error;
}

/** Reads parameter from ini file and sets in CParameter object. If parameter specification not
    found or ini section/key not found in file, marks as defaulted. */
void IniParameterFileAccess::ReadIniParameter(const IniFile& SourceFile, Parameters::ParameterType e) {
    long lSectionIndex, lKeyIndex=-1;
    const char * sSectionName, * sKey;
    try {
        if (GetSpecifications().GetParameterIniInfo(e, &sSectionName, &sKey)) {
            if ((lSectionIndex = SourceFile.GetSectionIndex(sSectionName)) > -1) {
                const IniSection  * pSection = SourceFile.GetSection(lSectionIndex);
                if ((lKeyIndex = pSection->FindKey(sKey)) > -1)
                    SetParameter(e, std::string(pSection->GetLine(lKeyIndex)->GetValue()), gPrintDirection);
            }
        }
        //if (lKeyIndex == -1)
        //  MarkAsMissingDefaulted(e, gPrintDirection);
    } catch (prg_exception& x) {
        x.addTrace("ReadIniParameter()","IniParameterFileAccess");
        throw;
    }
}

/** Writes specified comment and value to file for parameter type. */
void IniParameterFileAccess::WriteIniParameter(IniFile& WriteFile, Parameters::ParameterType e, const char* sValue, const char* sComment) {
    const char  * sSectionName, * sKey;
    try {
        if (GetSpecifications().GetParameterIniInfo(e, &sSectionName, &sKey)) {
            IniSection *  pSection = WriteFile.GetSection(sSectionName);
            if (sComment) pSection->AddComment(sComment);
            pSection->AddLine(sKey, sValue);
        } else throw prg_error("Unknown parameter type '%d'.", "WriteIniParameters()", e);
    } catch (prg_exception& x) {
        x.addTrace("WriteIniParameters()","IniParameterFileAccess");
        throw;
    }
}

/** Writes specified comment and value to file as specified section/key names. */
void IniParameterFileAccess::WriteIniParameterAsKey(IniFile& WriteFile, const char* sSectionName, const char * sKey, const char* sValue, const char* sComment) {
    try {
        IniSection *  pSection = WriteFile.GetSection(sSectionName);
        if (sComment) pSection->AddComment(sComment);
        pSection->AddLine(sKey, sValue);
    } catch (prg_exception& x) {
        x.addTrace("WriteIniParameterAsKey()","IniParameterFileAccess");
        throw;
    }
}

/** Writes parameters of associated CParameters object to ini file, of most recent
    format specification. */
void IniParameterFileAccess::Write(const char* sFilename) {
    try {
        IniFile WriteFile;
        _parameters.setSourceFileName(sFilename);
        gpSpecifications = new IniParameterSpecification();

        //write settings as provided in main graphical interface
        WriteInputSettings(WriteFile);
        WriteAnalysisSettings(WriteFile);
        WriteOutputSettings(WriteFile);
        WriteAdvancedInputSettings(WriteFile);
        WriteAdvancedAnalysisSettings(WriteFile);
        WriteAdvancedOutputSettings(WriteFile);
        WritePowerEvaluationsSettings(WriteFile);
        WriteRunOptionSettings(WriteFile);
        WriteSystemSettings(WriteFile);

        WriteFile.Write(_parameters.getSourceFileName());
    } catch (prg_exception& x) {
        x.addTrace("Write()", "IniParameterFileAccess");
        throw;
    }
}

/** Writes parameter settings grouped under 'Input'. */
void IniParameterFileAccess::WriteInputSettings(IniFile& WriteFile) {
    std::string  s;
    try {
        WriteIniParameter(WriteFile, Parameters::TREE_FILE, GetParameterString(Parameters::TREE_FILE, s).c_str(), GetParameterComment(Parameters::TREE_FILE));
        WriteIniParameter(WriteFile, Parameters::COUNT_FILE, GetParameterString(Parameters::COUNT_FILE, s).c_str(), GetParameterComment(Parameters::COUNT_FILE));
        WriteIniParameter(WriteFile, Parameters::POPULATION_FILE, GetParameterString(Parameters::POPULATION_FILE, s).c_str(), GetParameterComment(Parameters::POPULATION_FILE));
        WriteIniParameter(WriteFile, Parameters::DATA_TIME_RANGES, GetParameterString(Parameters::DATA_TIME_RANGES, s).c_str(), GetParameterComment(Parameters::DATA_TIME_RANGES));
    } catch (prg_exception& x) {
        x.addTrace("WriteInputSettings()","IniParameterFileAccess");
        throw;
    }
}

/** Writes parameter settings grouped under 'Analysis'. */
void IniParameterFileAccess::WriteAnalysisSettings(IniFile& WriteFile) {
    std::string s;
    try {
        WriteIniParameter(WriteFile, Parameters::SCAN_TYPE, GetParameterString(Parameters::SCAN_TYPE, s).c_str(), GetParameterComment(Parameters::SCAN_TYPE));
        WriteIniParameter(WriteFile, Parameters::MODEL_TYPE, GetParameterString(Parameters::MODEL_TYPE, s).c_str(), GetParameterComment(Parameters::MODEL_TYPE));
        WriteIniParameter(WriteFile, Parameters::CONDITIONAL_TYPE, GetParameterString(Parameters::CONDITIONAL_TYPE, s).c_str(), GetParameterComment(Parameters::CONDITIONAL_TYPE));
        WriteIniParameter(WriteFile, Parameters::EVENT_PROBABILITY, GetParameterString(Parameters::EVENT_PROBABILITY, s).c_str(), GetParameterComment(Parameters::EVENT_PROBABILITY));
        WriteIniParameter(WriteFile, Parameters::START_DATA_TIME_RANGE, GetParameterString(Parameters::START_DATA_TIME_RANGE, s).c_str(), GetParameterComment(Parameters::START_DATA_TIME_RANGE));
        WriteIniParameter(WriteFile, Parameters::END_DATA_TIME_RANGE, GetParameterString(Parameters::END_DATA_TIME_RANGE, s).c_str(), GetParameterComment(Parameters::END_DATA_TIME_RANGE));
    } catch (prg_exception& x) {
        x.addTrace("WriteAnalysisSettings()","IniParameterFileAccess");
        throw;
    }
}

/** Writes parameter settings grouped under 'Output'. */
void IniParameterFileAccess::WriteOutputSettings(IniFile& WriteFile) {
  std::string s;
  try {
    WriteIniParameter(WriteFile, Parameters::RESULTS_FILE, GetParameterString(Parameters::RESULTS_FILE, s).c_str(), GetParameterComment(Parameters::RESULTS_FILE));
    WriteIniParameter(WriteFile, Parameters::RESULTS_HTML, GetParameterString(Parameters::RESULTS_HTML, s).c_str(), GetParameterComment(Parameters::RESULTS_HTML));
    WriteIniParameter(WriteFile, Parameters::RESULTS_CSV, GetParameterString(Parameters::RESULTS_CSV, s).c_str(), GetParameterComment(Parameters::RESULTS_CSV));
  } catch (prg_exception& x) {
    x.addTrace("WriteOutputSettings()","IniParameterFileAccess");
    throw;
  }
}

/** Writes parameter settings grouped under 'Advanced Input'. */
void IniParameterFileAccess::WriteAdvancedInputSettings(IniFile& WriteFile) {
    std::string  s;
    try {
        WriteIniParameter(WriteFile, Parameters::CUT_FILE, GetParameterString(Parameters::CUT_FILE, s).c_str(), GetParameterComment(Parameters::CUT_FILE));
        WriteIniParameter(WriteFile, Parameters::CUT_TYPE, GetParameterString(Parameters::CUT_TYPE, s).c_str(), GetParameterComment(Parameters::CUT_TYPE));
        WriteIniParameter(WriteFile, Parameters::DUPLICATES, GetParameterString(Parameters::DUPLICATES, s).c_str(), GetParameterComment(Parameters::DUPLICATES));
    } catch (prg_exception& x) {
        x.addTrace("WriteAdvancedInputSettings()","IniParameterFileAccess");
        throw;
    }
}

/** Writes parameter settings grouped under 'Advanced Analysis'. */
void IniParameterFileAccess::WriteAdvancedAnalysisSettings(IniFile& WriteFile) {
    std::string s;
    try {
        WriteIniParameter(WriteFile, Parameters::REPLICATIONS, GetParameterString(Parameters::REPLICATIONS, s).c_str(), GetParameterComment(Parameters::REPLICATIONS));
        WriteIniParameter(WriteFile, Parameters::RANDOMIZATION_SEED, GetParameterString(Parameters::RANDOMIZATION_SEED, s).c_str(), GetParameterComment(Parameters::RANDOMIZATION_SEED));
        WriteIniParameter(WriteFile, Parameters::RANDOMLY_GENERATE_SEED, GetParameterString(Parameters::RANDOMLY_GENERATE_SEED, s).c_str(), GetParameterComment(Parameters::RANDOMLY_GENERATE_SEED));
    } catch (prg_exception& x) {
        x.addTrace("WriteAdvancedAnalysisSettings()","IniParameterFileAccess");
        throw;
    }
}

/** Writes parameter settings grouped under 'Advanced Output'. */
void IniParameterFileAccess::WriteAdvancedOutputSettings(IniFile& WriteFile) {
    std::string s;
    try {
        WriteIniParameter(WriteFile, Parameters::RESULTS_LLR, GetParameterString(Parameters::RESULTS_LLR, s).c_str(), GetParameterComment(Parameters::RESULTS_LLR));
    } catch (prg_exception& x) {
        x.addTrace("WriteAdvancedOutputSettings()","IniParameterFileAccess");
        throw;
    }
}

/** Writes parameter settings grouped under 'Power Evaluations'. */
void IniParameterFileAccess::WritePowerEvaluationsSettings(IniFile& WriteFile) {
    std::string s;
    try {
        WriteIniParameter(WriteFile, Parameters::READ_SIMULATIONS, GetParameterString(Parameters::READ_SIMULATIONS, s).c_str(), GetParameterComment(Parameters::READ_SIMULATIONS));
        WriteIniParameter(WriteFile, Parameters::INPUT_SIM_FILE, GetParameterString(Parameters::INPUT_SIM_FILE, s).c_str(), GetParameterComment(Parameters::INPUT_SIM_FILE));
        WriteIniParameter(WriteFile, Parameters::WRITE_SIMULATIONS, GetParameterString(Parameters::WRITE_SIMULATIONS, s).c_str(), GetParameterComment(Parameters::WRITE_SIMULATIONS));
        WriteIniParameter(WriteFile, Parameters::OUTPUT_SIM_FILE, GetParameterString(Parameters::OUTPUT_SIM_FILE, s).c_str(), GetParameterComment(Parameters::OUTPUT_SIM_FILE));
    } catch (prg_exception& x) {
        x.addTrace("WritePowerEvaluationsSettings()","IniParameterFileAccess");
        throw;
    }
}

/** Writes parameter settings grouped under 'Run Options'. */
void IniParameterFileAccess::WriteRunOptionSettings(IniFile& WriteFile) {
    std::string s;
    try {
        WriteIniParameter(WriteFile, Parameters::PARALLEL_PROCESSES, GetParameterString(Parameters::PARALLEL_PROCESSES, s).c_str(), GetParameterComment(Parameters::PARALLEL_PROCESSES));
    } catch (prg_exception& x) {
        x.addTrace("WriteRunOptionSettings()","IniParameterFileAccess");
        throw;
    }
}

/** Writes parameter settings grouped under '[System]'. */
void IniParameterFileAccess::WriteSystemSettings(IniFile& WriteFile) {
    std::string s;
    try {
        WriteIniParameter(WriteFile, Parameters::CREATION_VERSION, GetParameterString(Parameters::CREATION_VERSION, s).c_str(), GetParameterComment(Parameters::CREATION_VERSION));
    } catch (prg_exception& x) {
        x.addTrace("WriteSystemSettings()","IniParameterFileAccess");
        throw;
    }
}
