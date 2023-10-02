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
        ReadAdditionalTreeFileNameSettings(SourceFile);
        ReadInputSourceSettings(SourceFile);

        /* In version 2.0.x we moved the temporal window settings to the advanced tab and added an option to toggle this window restriction - default off.
           To keep the behavior of the prior versions, we need to toggle this feature on. */
        if (_parameters.getCreationVersion().iMajor < 2) {
            if (Parameters::isTemporalScanType(_parameters.getScanType())) {
                _parameters.setRestrictTemporalWindows(true);
                _parameters.setDatePrecisionType(DataTimeRange::GENERIC);
            } else {
                _parameters.setDatePrecisionType(DataTimeRange::NONE);
            }
        }
    } catch (prg_exception& x) {
        x.addTrace("Read()","IniParameterFileAccess");
        throw;
    }
    return !_read_error;
}

/** Reads parameter from ini file and returns all found key values in vector for
    those parameters that have optional additional keys, such as files with
    multiple datasets. */
std::vector<std::string>& IniParameterFileAccess::ReadIniParameter(const IniFile& SourceFile, Parameters::ParameterType eParameterType, std::vector<std::string>& vParameters, size_t iSuffixIndex) const {
    long lSectionIndex, lKeyIndex;
    std::string sNextKey;
    const char * sSectionName, * sKey;

    vParameters.clear();
    if (GetSpecifications().GetMultipleParameterIniInfo(eParameterType, &sSectionName, &sKey)) {
        //read possibly other dataset case source
        if ((lSectionIndex = SourceFile.GetSectionIndex(sSectionName)) > -1) {
            const IniSection  * pSection = SourceFile.GetSection(lSectionIndex);
            printString(sNextKey, "%s%i", sKey, iSuffixIndex);
            while ((lKeyIndex = pSection->FindKey(sNextKey.c_str())) > -1) {
                vParameters.push_back(std::string(pSection->GetLine(lKeyIndex)->GetValue()));
                printString(sNextKey, "%s%i", sKey, ++iSuffixIndex);
            }
        }
    }
    return vParameters;
}

/** Reads additional tree file parameter settings. */
void IniParameterFileAccess::ReadAdditionalTreeFileNameSettings(const IniFile& SourceFile) {
    std::vector<std::string> vFilenames;
    try {
        ReadIniParameter(SourceFile, Parameters::TREE_FILE, vFilenames, 2);
        for (size_t t=0; t < vFilenames.size(); ++t)
            _parameters.setTreeFileName(vFilenames[t].c_str(), true, t + 2);
    } catch (prg_exception& x) {
        x.addTrace("ReadAdditionalTreeFileNameSettings()","IniParameterFileAccess");
        throw;
    }
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

/* Reads optional input source settings. */
void IniParameterFileAccess::ReadInputSourceSettings(const IniFile& SourceFile) {
    const char * section, * advancedinput, * key;
    std::string buffer;

    try {
        // tree file
        if (GetSpecifications().GetParameterIniInfo(Parameters::TREE_FILE, &section, &key)) {
            Parameters::InputSource source;
            if (ReadInputSourceSection(SourceFile, section, key, source))
                _parameters.defineInputSource(Parameters::TREE_FILE, source);

            // section name for advanced input settings
            if (!GetSpecifications().GetParameterIniInfo(Parameters::CUT_FILE, &advancedinput, &key))
                throw prg_error("Unable to determine section for advanced input settings.", "ReadInputSourceSettings()");

            // now define input sources for additional tree files
            for (unsigned int setIdx=1; setIdx < _parameters.getTreeFileNames().size(); ++setIdx) {
                printString(buffer, "%s%u", key, (setIdx + 1));
                Parameters::InputSource sourceset;
                if (ReadInputSourceSection(SourceFile, advancedinput, buffer.c_str(), sourceset))
                    _parameters.defineInputSource(Parameters::TREE_FILE, sourceset, setIdx + 1);
            }

        }
        // case file
        if (GetSpecifications().GetParameterIniInfo(Parameters::COUNT_FILE, &section, &key)) {
            Parameters::InputSource source;
            if (ReadInputSourceSection(SourceFile, section, key, source))
                _parameters.defineInputSource(Parameters::COUNT_FILE, source);
        }
        // control file
        if (GetSpecifications().GetParameterIniInfo(Parameters::CONTROL_FILE, &section, &key)) {
            Parameters::InputSource source;
            if (ReadInputSourceSection(SourceFile, section, key, source))
                _parameters.defineInputSource(Parameters::CONTROL_FILE, source);
        }
        // cut file
        if (GetSpecifications().GetParameterIniInfo(Parameters::CUT_FILE, &section, &key)) {
            Parameters::InputSource source;
            if (ReadInputSourceSection(SourceFile, section, key, source))
                _parameters.defineInputSource(Parameters::CUT_FILE, source);
        }
        // power evaluations file
        if (GetSpecifications().GetParameterIniInfo(Parameters::POWER_EVALUATIONS_FILE, &section, &key)) {
            Parameters::InputSource source;
            if (ReadInputSourceSection(SourceFile, section, key, source))
                _parameters.defineInputSource(Parameters::POWER_EVALUATIONS_FILE, source);
        }
    } catch (prg_exception& x) {
        x.addTrace("ReadInputSourceSettings()","IniParameterFileAccess");
        throw;
    }
}

/* Reads key/values in passed source section. */
bool IniParameterFileAccess::ReadInputSourceSection(const IniFile& SourceFile, const char* sectionName, const char* keyPrefix, Parameters::InputSource& source) {
    long lSectionIndex, lKeyIndex=-1;
    std::string key, buffer;
    bool anykeys=false;
    std::string type, map, delimiter, group, skip, header;

    if ((lSectionIndex = SourceFile.GetSectionIndex(sectionName)) > -1) {
        const IniSection  * pSection = SourceFile.GetSection(lSectionIndex);
        // source file type
        printString(key, "%s-%s", keyPrefix, IniParameterSpecification::SourceType);
        if ((lKeyIndex = pSection->FindKey(key.c_str())) > -1) {
            anykeys=true;
            type = pSection->GetLine(lKeyIndex)->GetValue();
        }
        // fields map
        printString(key, "%s-%s", keyPrefix, IniParameterSpecification::SourceFieldMap);
        source.clearFieldsMap();
        if ((lKeyIndex = pSection->FindKey(key.c_str())) > -1) {
            anykeys=true;
            map = pSection->GetLine(lKeyIndex)->GetValue();
        }
        // delimiter
        printString(key, "%s-%s", keyPrefix, IniParameterSpecification::SourceDelimiter);
        if ((lKeyIndex = pSection->FindKey(key.c_str())) > -1) {
            anykeys=true;
            delimiter = pSection->GetLine(lKeyIndex)->GetValue();
        } 
        // grouper
        printString(key, "%s-%s", keyPrefix, IniParameterSpecification::SourceGrouper);
        if ((lKeyIndex = pSection->FindKey(key.c_str())) > -1) {
            anykeys=true;
            group = pSection->GetLine(lKeyIndex)->GetValue();
        } 
        // skip
        printString(key, "%s-%s", keyPrefix, IniParameterSpecification::SourceSkip);
        if ((lKeyIndex = pSection->FindKey(key.c_str())) > -1) {
            anykeys=true;
            skip = pSection->GetLine(lKeyIndex)->GetValue();
         }
        // first row header
        printString(key, "%s-%s", keyPrefix, IniParameterSpecification::SourceFirstRowHeader);
        if ((lKeyIndex = pSection->FindKey(key.c_str())) > -1) {
            anykeys=true;
            header = pSection->GetLine(lKeyIndex)->GetValue();
        }
        if (anykeys) {
            setInputSource(source, trimString(type), trimString(map), trimString(delimiter), trimString(group), trimString(skip), trimString(header), gPrintDirection);
        }
    }
    return anykeys;
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

void IniParameterFileAccess::writeSections(IniFile& ini, const IniParameterSpecification& specification) {
    const IniParameterSpecification * hold = gpSpecifications;
    try {
        gpSpecifications = &specification;
        WriteAnalysisSettings(ini);
        WriteInputSettings(ini);
        WriteOutputSettings(ini);
        WriteAdvancedInputSettings(ini);
        WriteAdvancedAnalysisTemporalWindowSettings(ini);
        WriteAdvancedAnalysisAdjustmentsSettings(ini);
        WriteAdvancedAnalysisInferenceSettings(ini);
        WriteSequentialScanSettings(ini);
        WritePowerEvaluationsSettings(ini);
        WriteAdvancedAnalysisMiscellaneousSettings(ini);
        WriteAdvancedOutputSettings(ini);
        WritePowerSimulationsSettings(ini);
        WriteRunOptionSettings(ini);
        WriteSystemSettings(ini);
        gpSpecifications = hold;
    } catch (...) {
        gpSpecifications = hold;
        throw;
    }
}

/** Writes parameters of associated CParameters object to ini file, of most recent format specification. */
void IniParameterFileAccess::Write(const char* sFilename) {
    try {
        IniFile WriteFile;
        _parameters.setSourceFileName(sFilename);
        gpSpecifications = new IniParameterSpecification();
        writeSections(WriteFile, *gpSpecifications);
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
        GetParameterString(Parameters::TREE_FILE, s);
        WriteIniParameter(WriteFile, Parameters::TREE_FILE, s.c_str(), GetParameterComment(Parameters::TREE_FILE));
        if (s.size()) WriteInputSource(WriteFile, Parameters::TREE_FILE, _parameters.getInputSource(Parameters::TREE_FILE));

        GetParameterString(Parameters::COUNT_FILE, s);
        WriteIniParameter(WriteFile, Parameters::COUNT_FILE, s.c_str(), GetParameterComment(Parameters::COUNT_FILE));
        if (s.size()) WriteInputSource(WriteFile, Parameters::COUNT_FILE, _parameters.getInputSource(Parameters::COUNT_FILE));

        GetParameterString(Parameters::CONTROL_FILE, s);
        WriteIniParameter(WriteFile, Parameters::CONTROL_FILE, s.c_str(), GetParameterComment(Parameters::CONTROL_FILE));
        if (s.size()) WriteInputSource(WriteFile, Parameters::CONTROL_FILE, _parameters.getInputSource(Parameters::CONTROL_FILE));

        WriteIniParameter(WriteFile, Parameters::DATE_PRECISION, GetParameterString(Parameters::DATE_PRECISION, s).c_str(), GetParameterComment(Parameters::DATE_PRECISION));
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
        WriteIniParameter(WriteFile, Parameters::SELF_CONTROL_DESIGN, GetParameterString(Parameters::SELF_CONTROL_DESIGN, s).c_str(), GetParameterComment(Parameters::SELF_CONTROL_DESIGN));
        WriteIniParameter(WriteFile, Parameters::EVENT_PROBABILITY, GetParameterString(Parameters::EVENT_PROBABILITY, s).c_str(), GetParameterComment(Parameters::EVENT_PROBABILITY));
        WriteIniParameter(WriteFile, Parameters::SCAN_RATE_TYPE, GetParameterString(Parameters::SCAN_RATE_TYPE, s).c_str(), GetParameterComment(Parameters::SCAN_RATE_TYPE));
    } catch (prg_exception& x) {
        x.addTrace("WriteAnalysisSettings()","IniParameterFileAccess");
        throw;
    }
}

/** Writes parameter settings grouped under 'Sequential Scan'. */
void IniParameterFileAccess::WriteSequentialScanSettings(IniFile& WriteFile) {
    std::string s;
    try {
        WriteIniParameter(WriteFile, Parameters::SEQUENTIAL_SCAN, GetParameterString(Parameters::SEQUENTIAL_SCAN, s).c_str(), GetParameterComment(Parameters::SEQUENTIAL_SCAN));
        WriteIniParameter(WriteFile, Parameters::SEQUENTIAL_MAX_SIGNAL, GetParameterString(Parameters::SEQUENTIAL_MAX_SIGNAL, s).c_str(), GetParameterComment(Parameters::SEQUENTIAL_MAX_SIGNAL));
        WriteIniParameter(WriteFile, Parameters::SEQUENTIAL_MIN_SIGNAL, GetParameterString(Parameters::SEQUENTIAL_MIN_SIGNAL, s).c_str(), GetParameterComment(Parameters::SEQUENTIAL_MIN_SIGNAL));
        WriteIniParameter(WriteFile, Parameters::SEQUENTIAL_FILE, GetParameterString(Parameters::SEQUENTIAL_FILE, s).c_str(), GetParameterComment(Parameters::SEQUENTIAL_FILE));
        WriteIniParameter(WriteFile, Parameters::SEQUENTIAL_ALPHA_OVERALL, GetParameterString(Parameters::SEQUENTIAL_ALPHA_OVERALL, s).c_str(), GetParameterComment(Parameters::SEQUENTIAL_ALPHA_OVERALL));
        WriteIniParameter(WriteFile, Parameters::SEQUENTIAL_ALPHA_SPENDING, GetParameterString(Parameters::SEQUENTIAL_ALPHA_SPENDING, s).c_str(), GetParameterComment(Parameters::SEQUENTIAL_ALPHA_SPENDING));
    } catch (prg_exception& x) {
        x.addTrace("WriteSequentialScanSettings()","IniParameterFileAccess");
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
    WriteIniParameter(WriteFile, Parameters::RESULTS_ASN, GetParameterString(Parameters::RESULTS_ASN, s).c_str(), GetParameterComment(Parameters::RESULTS_ASN));
    WriteIniParameter(WriteFile, Parameters::RESULTS_NWK, GetParameterString(Parameters::RESULTS_NWK, s).c_str(), GetParameterComment(Parameters::RESULTS_NWK));
  } catch (prg_exception& x) {
    x.addTrace("WriteOutputSettings()","IniParameterFileAccess");
    throw;
  }
}

/** Writes parameter settings grouped under 'Advanced Input'. */
void IniParameterFileAccess::WriteAdvancedInputSettings(IniFile& WriteFile) {
    std::string  s;
    try {
        GetParameterString(Parameters::CUT_FILE, s);
        WriteIniParameter(WriteFile, Parameters::CUT_FILE, s.c_str(), GetParameterComment(Parameters::CUT_FILE));
        if (s.size()) WriteInputSource(WriteFile, Parameters::CUT_FILE, _parameters.getInputSource(Parameters::CUT_FILE));
        WriteIniParameter(WriteFile, Parameters::CUT_TYPE, GetParameterString(Parameters::CUT_TYPE, s).c_str(), GetParameterComment(Parameters::CUT_TYPE));
        WriteIniParameter(WriteFile, Parameters::DATA_ONLY_ON_LEAVES, GetParameterString(Parameters::DATA_ONLY_ON_LEAVES, s).c_str(), GetParameterComment(Parameters::DATA_ONLY_ON_LEAVES));
        WriteIniParameter(WriteFile, Parameters::RELAXED_STUDY_DATA_PERIOD_CHECKING, GetParameterString(Parameters::RELAXED_STUDY_DATA_PERIOD_CHECKING, s).c_str(), GetParameterComment(Parameters::RELAXED_STUDY_DATA_PERIOD_CHECKING));
        WriteIniParameter(WriteFile, Parameters::DISALLOW_MULTI_PARENT_NODES, GetParameterString(Parameters::DISALLOW_MULTI_PARENT_NODES, s).c_str(), GetParameterComment(Parameters::DISALLOW_MULTI_PARENT_NODES));
        WriteIniParameter(WriteFile, Parameters::DISALLOW_MULTIPLE_ROOTS, GetParameterString(Parameters::DISALLOW_MULTIPLE_ROOTS, s).c_str(), GetParameterComment(Parameters::DISALLOW_MULTIPLE_ROOTS));
        WriteIniParameter(WriteFile, Parameters::MINIMUM_CENSOR_TIME, GetParameterString(Parameters::MINIMUM_CENSOR_TIME, s).c_str(), GetParameterComment(Parameters::MINIMUM_CENSOR_TIME));
        WriteIniParameter(WriteFile, Parameters::MINIMUM_CENSOR_PERCENTAGE, GetParameterString(Parameters::MINIMUM_CENSOR_PERCENTAGE, s).c_str(), GetParameterComment(Parameters::MINIMUM_CENSOR_PERCENTAGE));
        WriteIniParameter(WriteFile, Parameters::RSK_WND_CENSOR, GetParameterString(Parameters::RSK_WND_CENSOR, s).c_str(), GetParameterComment(Parameters::RSK_WND_CENSOR));
        WriteIniParameter(WriteFile, Parameters::RSK_WND_ALT_CENSOR_DENOM, GetParameterString(Parameters::RSK_WND_ALT_CENSOR_DENOM, s).c_str(), GetParameterComment(Parameters::RSK_WND_ALT_CENSOR_DENOM));
    } catch (prg_exception& x) {
        x.addTrace("WriteAdvancedInputSettings()","IniParameterFileAccess");
        throw;
    }

    std::string   sComment;
    const char  * sSectionName, * sBaseKey;
    try {
        for (size_t t=1; t < _parameters.getTreeFileNames().size(); ++t) {
            if (GetSpecifications().GetMultipleParameterIniInfo(Parameters::TREE_FILE, &sSectionName, &sBaseKey)) {
                printString(s, "%s%i", sBaseKey, t + 1);
                printString(sComment, " tree filename (additional %i)", t + 1);
                WriteIniParameterAsKey(WriteFile, sSectionName, s.c_str(), _parameters.getTreeFileNames()[t].c_str(), sComment.c_str());
                if (_parameters.getTreeFileNames()[t].size()) WriteInputSource(WriteFile, *(WriteFile.GetSection(sSectionName)), s, _parameters.getInputSource(Parameters::TREE_FILE, t+1));
            }
        }
    } catch (prg_exception& x) {
        x.addTrace("WriteAdvancedInputSettings()","IniParameterFileAccess");
        throw;
    }
}

/** Writes parameter settings grouped under 'Advanced Analysis - Temporal Window'. */
void IniParameterFileAccess::WriteAdvancedAnalysisTemporalWindowSettings(IniFile& WriteFile) {
    std::string s;
    try {
        WriteIniParameter(WriteFile, Parameters::MAXIMUM_WINDOW_PERCENTAGE, GetParameterString(Parameters::MAXIMUM_WINDOW_PERCENTAGE, s).c_str(), GetParameterComment(Parameters::MAXIMUM_WINDOW_PERCENTAGE));
        WriteIniParameter(WriteFile, Parameters::MAXIMUM_WINDOW_FIXED, GetParameterString(Parameters::MAXIMUM_WINDOW_FIXED, s).c_str(), GetParameterComment(Parameters::MAXIMUM_WINDOW_FIXED));
        WriteIniParameter(WriteFile, Parameters::MAXIMUM_WINDOW_TYPE, GetParameterString(Parameters::MAXIMUM_WINDOW_TYPE, s).c_str(), GetParameterComment(Parameters::MAXIMUM_WINDOW_TYPE));
        WriteIniParameter(WriteFile, Parameters::MINIMUM_WINDOW_FIXED, GetParameterString(Parameters::MINIMUM_WINDOW_FIXED, s).c_str(), GetParameterComment(Parameters::MINIMUM_WINDOW_FIXED));
        WriteIniParameter(WriteFile, Parameters::APPLY_RISK_WINDOW_RESTRICTION, GetParameterString(Parameters::APPLY_RISK_WINDOW_RESTRICTION, s).c_str(), GetParameterComment(Parameters::APPLY_RISK_WINDOW_RESTRICTION));
        WriteIniParameter(WriteFile, Parameters::RISK_WINDOW_PERCENTAGE, GetParameterString(Parameters::RISK_WINDOW_PERCENTAGE, s).c_str(), GetParameterComment(Parameters::RISK_WINDOW_PERCENTAGE));
        WriteIniParameter(WriteFile, Parameters::PROSPECTIVE_ANALYSIS, GetParameterString(Parameters::PROSPECTIVE_ANALYSIS, s).c_str(), GetParameterComment(Parameters::PROSPECTIVE_ANALYSIS));
        WriteIniParameter(WriteFile, Parameters::RESTRICTED_TIME_RANGE, GetParameterString(Parameters::RESTRICTED_TIME_RANGE, s).c_str(), GetParameterComment(Parameters::RESTRICTED_TIME_RANGE));
        WriteIniParameter(WriteFile, Parameters::START_DATA_TIME_RANGE, GetParameterString(Parameters::START_DATA_TIME_RANGE, s).c_str(), GetParameterComment(Parameters::START_DATA_TIME_RANGE));
        WriteIniParameter(WriteFile, Parameters::END_DATA_TIME_RANGE, GetParameterString(Parameters::END_DATA_TIME_RANGE, s).c_str(), GetParameterComment(Parameters::END_DATA_TIME_RANGE));
    } catch (prg_exception& x) {
        x.addTrace("WriteAdvancedAnalysisTemporalWindowSettings()","IniParameterFileAccess");
        throw;
    }
}

/** Writes parameter settings grouped under 'Advanced Analysis - Adjustments'. */
void IniParameterFileAccess::WriteAdvancedAnalysisAdjustmentsSettings(IniFile& WriteFile) {
    std::string s;
    try {
        WriteIniParameter(WriteFile, Parameters::DAYOFWEEK_ADJUSTMENT, GetParameterString(Parameters::DAYOFWEEK_ADJUSTMENT, s).c_str(), GetParameterComment(Parameters::DAYOFWEEK_ADJUSTMENT));
        WriteIniParameter(WriteFile, Parameters::APPLY_EXCLUSION_RANGES, GetParameterString(Parameters::APPLY_EXCLUSION_RANGES, s).c_str(), GetParameterComment(Parameters::APPLY_EXCLUSION_RANGES));
        WriteIniParameter(WriteFile, Parameters::EXCLUSION_RANGES, GetParameterString(Parameters::EXCLUSION_RANGES, s).c_str(), GetParameterComment(Parameters::EXCLUSION_RANGES));
    } catch (prg_exception& x) {
        x.addTrace("WriteAdvancedAnalysisAdjustmentsSettings()","IniParameterFileAccess");
        throw;
    }
}

/** Writes parameter settings grouped under 'Advanced Analysis - Inference'. */
void IniParameterFileAccess::WriteAdvancedAnalysisInferenceSettings(IniFile& WriteFile) {
    std::string s;
    try {
        WriteIniParameter(WriteFile, Parameters::REPLICATIONS, GetParameterString(Parameters::REPLICATIONS, s).c_str(), GetParameterComment(Parameters::REPLICATIONS));
        WriteIniParameter(WriteFile, Parameters::RESTRICT_TREE_LEVELS, GetParameterString(Parameters::RESTRICT_TREE_LEVELS, s).c_str(), GetParameterComment(Parameters::RESTRICT_TREE_LEVELS));
        WriteIniParameter(WriteFile, Parameters::RESTRICTED_TREE_LEVELS, GetParameterString(Parameters::RESTRICTED_TREE_LEVELS, s).c_str(), GetParameterComment(Parameters::RESTRICTED_TREE_LEVELS));
        WriteIniParameter(WriteFile, Parameters::RANDOMIZATION_SEED, GetParameterString(Parameters::RANDOMIZATION_SEED, s).c_str(), GetParameterComment(Parameters::RANDOMIZATION_SEED));
        WriteIniParameter(WriteFile, Parameters::RANDOMLY_GENERATE_SEED, GetParameterString(Parameters::RANDOMLY_GENERATE_SEED, s).c_str(), GetParameterComment(Parameters::RANDOMLY_GENERATE_SEED));
        WriteIniParameter(WriteFile, Parameters::MINIMUM_CASES_NODE, GetParameterString(Parameters::MINIMUM_CASES_NODE, s).c_str(), GetParameterComment(Parameters::MINIMUM_CASES_NODE));
    } catch (prg_exception& x) {
        x.addTrace("WriteAdvancedAnalysisInferenceSettings()","IniParameterFileAccess");
        throw;
    }
}

/** Writes parameter settings grouped under 'Advanced Analysis - Miscellaneous'. */
void IniParameterFileAccess::WriteAdvancedAnalysisMiscellaneousSettings(IniFile& WriteFile) {
    std::string s;
    try {
        WriteIniParameter(WriteFile, Parameters::PROSPECTIVE_FREQ_TYPE, GetParameterString(Parameters::PROSPECTIVE_FREQ_TYPE, s).c_str(), GetParameterComment(Parameters::PROSPECTIVE_FREQ_TYPE));
        WriteIniParameter(WriteFile, Parameters::PROSPECTIVE_FREQ, GetParameterString(Parameters::PROSPECTIVE_FREQ, s).c_str(), GetParameterComment(Parameters::PROSPECTIVE_FREQ));
    } catch (prg_exception& x) {
        x.addTrace("WriteAdvancedAnalysisMiscellaneousSettings()", "IniParameterFileAccess");
        throw;
    }
}

/** Writes parameter settings grouped under 'Advanced Output'. */
void IniParameterFileAccess::WriteAdvancedOutputSettings(IniFile& WriteFile) {
    std::string s;
    try {
        WriteIniParameter(WriteFile, Parameters::RESULTS_LLR, GetParameterString(Parameters::RESULTS_LLR, s).c_str(), GetParameterComment(Parameters::RESULTS_LLR));
        WriteIniParameter(WriteFile, Parameters::REPORT_CRITICAL_VALUES, GetParameterString(Parameters::REPORT_CRITICAL_VALUES, s).c_str(), GetParameterComment(Parameters::REPORT_CRITICAL_VALUES));
        WriteIniParameter(WriteFile, Parameters::REPORT_ATTR_RISK, GetParameterString(Parameters::REPORT_ATTR_RISK, s).c_str(), GetParameterComment(Parameters::REPORT_ATTR_RISK));
        WriteIniParameter(WriteFile, Parameters::ATTR_RISK_NUM_EXPOSED, GetParameterString(Parameters::ATTR_RISK_NUM_EXPOSED, s).c_str(), GetParameterComment(Parameters::ATTR_RISK_NUM_EXPOSED));
        WriteIniParameter(WriteFile, Parameters::OUTPUT_TEMPORAL_GRAPH, GetParameterString(Parameters::OUTPUT_TEMPORAL_GRAPH, s).c_str(), GetParameterComment(Parameters::OUTPUT_TEMPORAL_GRAPH));
        WriteIniParameter(WriteFile, Parameters::TEMPORAL_GRAPH_REPORT_TYPE, GetParameterString(Parameters::TEMPORAL_GRAPH_REPORT_TYPE, s).c_str(), GetParameterComment(Parameters::TEMPORAL_GRAPH_REPORT_TYPE));
        WriteIniParameter(WriteFile, Parameters::TEMPORAL_GRAPH_MLC_COUNT, GetParameterString(Parameters::TEMPORAL_GRAPH_MLC_COUNT, s).c_str(), GetParameterComment(Parameters::TEMPORAL_GRAPH_MLC_COUNT));
        WriteIniParameter(WriteFile, Parameters::TEMPORAL_GRAPH_CUTOFF, GetParameterString(Parameters::TEMPORAL_GRAPH_CUTOFF, s).c_str(), GetParameterComment(Parameters::TEMPORAL_GRAPH_CUTOFF));
    } catch (prg_exception& x) {
        x.addTrace("WriteAdvancedOutputSettings()","IniParameterFileAccess");
        throw;
    }
}

/** Writes parameter settings grouped under 'Power Evaluations'. */
void IniParameterFileAccess::WritePowerEvaluationsSettings(IniFile& WriteFile) {
    std::string s;
    try {
        WriteIniParameter(WriteFile, Parameters::POWER_EVALUATIONS, GetParameterString(Parameters::POWER_EVALUATIONS, s).c_str(), GetParameterComment(Parameters::POWER_EVALUATIONS));
        WriteIniParameter(WriteFile, Parameters::POWER_EVALUATION_TYPE, GetParameterString(Parameters::POWER_EVALUATION_TYPE, s).c_str(), GetParameterComment(Parameters::POWER_EVALUATION_TYPE));
        WriteIniParameter(WriteFile, Parameters::CRITICAL_VALUES_TYPE, GetParameterString(Parameters::CRITICAL_VALUES_TYPE, s).c_str(), GetParameterComment(Parameters::CRITICAL_VALUES_TYPE));
        WriteIniParameter(WriteFile, Parameters::CRITICAL_VALUE_05, GetParameterString(Parameters::CRITICAL_VALUE_05, s).c_str(), GetParameterComment(Parameters::CRITICAL_VALUE_05));
        WriteIniParameter(WriteFile, Parameters::CRITICAL_VALUE_01, GetParameterString(Parameters::CRITICAL_VALUE_01, s).c_str(), GetParameterComment(Parameters::CRITICAL_VALUE_01));
        WriteIniParameter(WriteFile, Parameters::CRITICAL_VALUE_001, GetParameterString(Parameters::CRITICAL_VALUE_001, s).c_str(), GetParameterComment(Parameters::CRITICAL_VALUE_001));
        WriteIniParameter(WriteFile, Parameters::POWER_EVALUATION_TOTALCASES, GetParameterString(Parameters::POWER_EVALUATION_TOTALCASES, s).c_str(), GetParameterComment(Parameters::POWER_EVALUATION_TOTALCASES));
        WriteIniParameter(WriteFile, Parameters::POWER_EVALUATIONS_REPLICA, GetParameterString(Parameters::POWER_EVALUATIONS_REPLICA, s).c_str(), GetParameterComment(Parameters::POWER_EVALUATIONS_REPLICA));
        GetParameterString(Parameters::POWER_EVALUATIONS_FILE, s);
        WriteIniParameter(WriteFile, Parameters::POWER_EVALUATIONS_FILE, s.c_str(), GetParameterComment(Parameters::POWER_EVALUATIONS_FILE));
        if (s.size()) WriteInputSource(WriteFile, Parameters::POWER_EVALUATIONS_FILE, _parameters.getInputSource(Parameters::POWER_EVALUATIONS_FILE));
        WriteIniParameter(WriteFile, Parameters::POWER_BASELINE_PROBABILITY, GetParameterString(Parameters::POWER_BASELINE_PROBABILITY, s).c_str(), GetParameterComment(Parameters::POWER_BASELINE_PROBABILITY));
        WriteIniParameter(WriteFile, Parameters::POWER_Z, GetParameterString(Parameters::POWER_Z, s).c_str(), GetParameterComment(Parameters::POWER_Z));
    } catch (prg_exception& x) {
        x.addTrace("WritePowerEvaluationsSettings()","IniParameterFileAccess");
        throw;
    }
}

/** Writes parameter settings grouped under 'Power Simulations'. */
void IniParameterFileAccess::WritePowerSimulationsSettings(IniFile& WriteFile) {
    std::string s;
    try {
        WriteIniParameter(WriteFile, Parameters::READ_SIMULATIONS, GetParameterString(Parameters::READ_SIMULATIONS, s).c_str(), GetParameterComment(Parameters::READ_SIMULATIONS));
        WriteIniParameter(WriteFile, Parameters::INPUT_SIM_FILE, GetParameterString(Parameters::INPUT_SIM_FILE, s).c_str(), GetParameterComment(Parameters::INPUT_SIM_FILE));
        WriteIniParameter(WriteFile, Parameters::WRITE_SIMULATIONS, GetParameterString(Parameters::WRITE_SIMULATIONS, s).c_str(), GetParameterComment(Parameters::WRITE_SIMULATIONS));
        WriteIniParameter(WriteFile, Parameters::OUTPUT_SIM_FILE, GetParameterString(Parameters::OUTPUT_SIM_FILE, s).c_str(), GetParameterComment(Parameters::OUTPUT_SIM_FILE));
    } catch (prg_exception& x) {
        x.addTrace("WritePowerSimulationsSettings()","IniParameterFileAccess");
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

void IniParameterFileAccess::WriteInputSource(IniFile& WriteFile, Parameters::ParameterType eParameterType, const Parameters::InputSource * source) {
    const char  * sSectionName, * sKey;
    std::string buffer, key;

    try {
        if (source) {
            if (GetSpecifications().GetParameterIniInfo(eParameterType, &sSectionName, &sKey)) {
                IniSection * pSection = WriteFile.GetSection(sSectionName);
                WriteInputSource(WriteFile, *(WriteFile.GetSection(sSectionName)), sKey, source);
            }
        }
    } catch (prg_exception& x) {
        x.addTrace("WriteInputSource()","IniParameterFileAccess");
        throw;
    }
}

/** Write InputSource object to ini file. */
void IniParameterFileAccess::WriteInputSource(IniFile& WriteFile, IniSection& section, const std::string& basekey, const Parameters::InputSource * source) {
    std::string buffer, key;

    try {
        if (source) {
            section.AddComment("source type (CSV=0)");
            printString(key, "%s-%s", basekey.c_str(), IniParameterSpecification::SourceType);
            section.AddLine(key.c_str(), AsString(buffer, source->getSourceType()).c_str());

            if (source->getFieldsMap().size()) {
                printString(buffer, "source field map (comma separated list of integers)");
                section.AddComment(buffer.c_str());
                std::stringstream s;
                for (FieldMapContainer_t::const_iterator itr=source->getFieldsMap().begin(); itr != source->getFieldsMap().end(); ++itr) {
                    if (itr->type() == typeid(long)) {
                        s << boost::any_cast<long>(*itr);
                    } else {
                        throw prg_error("Unknown type '%s'.", "WriteInputSource()", itr->type().name());
                    }
                    if ((itr+1) != source->getFieldsMap().end()) {s << ",";}
                }
                printString(key, "%s-%s", basekey.c_str(), IniParameterSpecification::SourceFieldMap);
                section.AddLine(key.c_str(), s.str().c_str());
            }
            if (source->getSourceType() == CSV) {
                section.AddComment("csv source delimiter (leave empty for space or tab delimiter)");
                printString(key, "%s-%s", basekey.c_str(), IniParameterSpecification::SourceDelimiter);
                section.AddLine(key.c_str(), source->getDelimiter().c_str());

                section.AddComment("csv source group character");
                printString(key, "%s-%s", basekey.c_str(), IniParameterSpecification::SourceGrouper);
                section.AddLine(key.c_str(), source->getGroup().c_str());

                section.AddComment("csv source skip initial lines (i.e. meta data)");
                printString(key, "%s-%s", basekey.c_str(), IniParameterSpecification::SourceSkip);
                section.AddLine(key.c_str(), AsString(buffer, source->getSkip()).c_str());

                section.AddComment("csv source first row column header");
                printString(key, "%s-%s", basekey.c_str(), IniParameterSpecification::SourceFirstRowHeader);
                section.AddLine(key.c_str(), AsString(buffer, source->getFirstRowHeader()).c_str());
            }
        }
    } catch (prg_exception& x) {
        x.addTrace("WriteInputSource()","IniParameterFileAccess");
        throw;
    }
}
