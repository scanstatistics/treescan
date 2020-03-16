//***************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//***************************************************************************
#include "IniParameterSpecification.h"

const char * IniParameterSpecification::Input                   = "Input";
const char * IniParameterSpecification::AdvancedInput           = "Advanced Input";
const char * IniParameterSpecification::Analysis                = "Analysis";
const char * IniParameterSpecification::TemporalWindow          = "Temporal Window";
const char * IniParameterSpecification::Adjustments             = "Adjustments";
const char * IniParameterSpecification::Inference               = "Inference";
const char * IniParameterSpecification::Output                  = "Output";
const char * IniParameterSpecification::AdditionalOutput        = "Additional Output";
const char * IniParameterSpecification::PowerEvaluations        = "Power Evaluations";
const char * IniParameterSpecification::PowerSimulations        = "Power Simulations";
const char * IniParameterSpecification::SequentialScan          = "Sequential Scan";

const char * IniParameterSpecification::RunOptions              = "Run Options";
const char * IniParameterSpecification::System                  = "System";

const char * IniParameterSpecification::SourceType              = "SourceType";
const char * IniParameterSpecification::SourceDelimiter         = "SourceDelimiter";
const char * IniParameterSpecification::SourceGrouper           = "SourceGrouper";
const char * IniParameterSpecification::SourceSkip              = "SourceSkip";
const char * IniParameterSpecification::SourceFieldMap          = "SourceFieldMap";
const char * IniParameterSpecification::SourceFirstRowHeader    = "SourceFirstRowHeader";


/** constructor -- builds specification for write process */
IniParameterSpecification::IniParameterSpecification() {
    // default to current version
    Parameters::CreationVersion  version = {std::atoi(VERSION_MAJOR), std::atoi(VERSION_MINOR), std::atoi(VERSION_RELEASE)};
    setup(version);
}

/** constructor -- builds specification for read process */
IniParameterSpecification::IniParameterSpecification(const IniFile& SourceFile, Parameters& Parameters) {
    Parameters::CreationVersion version = getIniVersion(SourceFile);
    Parameters.setVersion(version);
    setup(version);
}

/** constructor - builds specification to version specified in argument */
IniParameterSpecification::IniParameterSpecification(Parameters::CreationVersion version, Parameters& Parameters) {
    Parameters.setVersion(version);
    setup(version);
}

/** constructor - builds specification to version specified in argument
     - argument version must agree with version of ini file. */
IniParameterSpecification::IniParameterSpecification(const IniFile& SourceFile, Parameters::CreationVersion version, Parameters& Parameters) {
    // first get the Version setting from the source Ini file
    Parameters::CreationVersion ini_version = getIniVersion(SourceFile);
    // confirm that ini file and specified version are the same
    if (version != ini_version) {
        throw resolvable_error("Parameter file version (%u.%u.%u) does not match override version (%u.%u.%u).",
                               ini_version.iMajor, ini_version.iMinor, ini_version.iRelease,
                               version.iMajor, version.iMinor, version.iRelease);
    }
    setup(version);
}

/** define ini sections and section parameters base on passed version */
void IniParameterSpecification::setup(Parameters::CreationVersion version) {
    // define sections in which parameters belong
    _input_section = SectionInfo(Input, 100);
    _analysis_section = SectionInfo(Analysis, 200);
    _output_section = SectionInfo(Output, 300);
    _advanced_input_section = SectionInfo(AdvancedInput, 400);
    _temporal_window_section = SectionInfo(TemporalWindow, 500);
    _adjustments_section = SectionInfo(Adjustments, 600);
    _inference_section = SectionInfo(Inference, 700);
    _sequential_scan_section = SectionInfo(SequentialScan, 750);
    _power_evaluations_section = SectionInfo(PowerEvaluations, 800);
    _additional_output_section = SectionInfo(AdditionalOutput, 900);
    _power_simulations_section = SectionInfo(PowerSimulations, 1000);
    _run_options_section = SectionInfo(RunOptions, 1100);
    _system_section = SectionInfo(System, 1200);

    if (version.iMajor == 1 && version.iMinor == 1)
        Build_1_1_x_ParameterList();
    else if (version.iMajor == 1 && version.iMinor == 2)
        Build_1_2_x_ParameterList();
    else if (version.iMajor == 1 && version.iMinor == 3)
        Build_1_3_x_ParameterList();
    else if (version.iMajor == 1 && version.iMinor == 4)
        Build_1_4_x_ParameterList();
    else
		Build_2_0_x_ParameterList();
}

/* Returns ini version setting or default. */
Parameters::CreationVersion IniParameterSpecification::getIniVersion(const IniFile& SourceFile) {
    long lSectionIndex, lKeyIndex;
    Parameters::CreationVersion  version = {std::atoi(VERSION_MAJOR), std::atoi(VERSION_MINOR), std::atoi(VERSION_RELEASE)};
    bool bHasVersionKey=false;

    // search ini for version setting
    if ((lSectionIndex = SourceFile.GetSectionIndex(System)) > -1) {
        const IniSection * pSection = SourceFile.GetSection(lSectionIndex);
        if ((lKeyIndex = pSection->FindKey("parameters-version")) > -1) {
            sscanf(pSection->GetLine(lKeyIndex)->GetValue(), "%u.%u.%u", &version.iMajor, &version.iMinor, &version.iRelease);
            bHasVersionKey = true;
        }
    }
    return version;
}

/** Version 1.1 parameter specifications. */
void IniParameterSpecification::Build_1_1_x_ParameterList() {
    _parameter_info[Parameters::SCAN_TYPE] = ParamInfo(Parameters::SCAN_TYPE, "scan-type", 1, _analysis_section);
    _parameter_info[Parameters::CONDITIONAL_TYPE] = ParamInfo(Parameters::CONDITIONAL_TYPE, "conditional-type", 2, _analysis_section);
    _parameter_info[Parameters::MODEL_TYPE] = ParamInfo(Parameters::MODEL_TYPE, "probability-model", 3, _analysis_section);
    _parameter_info[Parameters::EVENT_PROBABILITY] = ParamInfo(Parameters::EVENT_PROBABILITY, "event-probability", 5, _analysis_section);
    _parameter_info[Parameters::START_DATA_TIME_RANGE] = ParamInfo(Parameters::START_DATA_TIME_RANGE, "window-start-range", 8, _analysis_section);
    _parameter_info[Parameters::END_DATA_TIME_RANGE] = ParamInfo(Parameters::END_DATA_TIME_RANGE, "window-end-range", 9, _analysis_section);

    _parameter_info[Parameters::TREE_FILE] = ParamInfo(Parameters::TREE_FILE, "tree-filename", 1, _input_section);
    _parameter_info[Parameters::COUNT_FILE] = ParamInfo(Parameters::COUNT_FILE, "count-filename", 2, _input_section);
    _parameter_info[Parameters::DATA_TIME_RANGES] = ParamInfo(Parameters::DATA_TIME_RANGES, "data-time-range", 3, _input_section);

    _parameter_info[Parameters::RESULTS_FILE] = ParamInfo(Parameters::RESULTS_FILE, "results-filename", 1, _output_section);
    _parameter_info[Parameters::RESULTS_HTML] = ParamInfo(Parameters::RESULTS_HTML, "results-html", 2, _output_section);
    _parameter_info[Parameters::RESULTS_CSV] = ParamInfo(Parameters::RESULTS_CSV, "results-csv", 3, _output_section);

    _parameter_info[Parameters::CUT_FILE] = ParamInfo(Parameters::CUT_FILE, "cut-filename", 1, _advanced_input_section);
    _parameter_info[Parameters::CUT_TYPE] = ParamInfo(Parameters::CUT_TYPE, "cut-type", 2, _advanced_input_section);

    _parameter_info[Parameters::MAXIMUM_WINDOW_PERCENTAGE] = ParamInfo(Parameters::MAXIMUM_WINDOW_PERCENTAGE, "maximum-window-percentage", 1, _temporal_window_section);
    _parameter_info[Parameters::MAXIMUM_WINDOW_FIXED] = ParamInfo(Parameters::MAXIMUM_WINDOW_FIXED, "maximum-window-fixed", 2, _temporal_window_section);
    _parameter_info[Parameters::MAXIMUM_WINDOW_TYPE] = ParamInfo(Parameters::MAXIMUM_WINDOW_TYPE, "maximum-window-type", 3, _temporal_window_section);
    _parameter_info[Parameters::MINIMUM_WINDOW_FIXED] = ParamInfo(Parameters::MINIMUM_WINDOW_FIXED, "minimum-window-fixed", 4, _temporal_window_section);

    _parameter_info[Parameters::REPLICATIONS] = ParamInfo(Parameters::REPLICATIONS, "monte-carlo-replications", 1, _inference_section);
    _parameter_info[Parameters::RANDOMIZATION_SEED] = ParamInfo(Parameters::RANDOMIZATION_SEED, "randomization-seed", 2, _inference_section);
    _parameter_info[Parameters::RANDOMLY_GENERATE_SEED] = ParamInfo(Parameters::RANDOMLY_GENERATE_SEED, "random-randomization-seed", 3, _inference_section);

    _parameter_info[Parameters::POWER_EVALUATIONS] = ParamInfo(Parameters::POWER_EVALUATIONS, "perform-power-evaluations", 1, _power_evaluations_section);
    _parameter_info[Parameters::POWER_EVALUATION_TYPE] = ParamInfo(Parameters::POWER_EVALUATION_TYPE, "power-evaluation-type", 2, _power_evaluations_section);
    _parameter_info[Parameters::CRITICAL_VALUES_TYPE] = ParamInfo(Parameters::CRITICAL_VALUES_TYPE, "critical-values-type", 3, _power_evaluations_section);
    _parameter_info[Parameters::CRITICAL_VALUE_05] = ParamInfo(Parameters::CRITICAL_VALUE_05, "critical-value-05", 4, _power_evaluations_section);
    _parameter_info[Parameters::CRITICAL_VALUE_01] = ParamInfo(Parameters::CRITICAL_VALUE_01, "critical-value-01", 5, _power_evaluations_section);
    _parameter_info[Parameters::CRITICAL_VALUE_001] = ParamInfo(Parameters::CRITICAL_VALUE_001, "critical-value-001", 6, _power_evaluations_section);
    _parameter_info[Parameters::POWER_EVALUATION_TOTALCASES] = ParamInfo(Parameters::POWER_EVALUATION_TOTALCASES, "power-evaluation-totalcases", 7, _power_evaluations_section);
    _parameter_info[Parameters::POWER_EVALUATIONS_REPLICA] = ParamInfo(Parameters::POWER_EVALUATIONS_REPLICA, "power-evaluation-replications", 8, _power_evaluations_section);
    _parameter_info[Parameters::POWER_EVALUATIONS_FILE] = ParamInfo(Parameters::POWER_EVALUATIONS_FILE, "alternative-hypothesis-filename", 9, _power_evaluations_section);

    _parameter_info[Parameters::RESULTS_LLR] = ParamInfo(Parameters::RESULTS_LLR, "results-llr", 1, _additional_output_section);
    _parameter_info[Parameters::REPORT_CRITICAL_VALUES] = ParamInfo(Parameters::REPORT_CRITICAL_VALUES, "report-critical-values", 2, _additional_output_section);

    _parameter_info[Parameters::READ_SIMULATIONS] = ParamInfo(Parameters::READ_SIMULATIONS, "input-simulations", 1, _power_simulations_section);
    _parameter_info[Parameters::INPUT_SIM_FILE] = ParamInfo(Parameters::INPUT_SIM_FILE, "input-simulations-file", 2, _power_simulations_section);
    _parameter_info[Parameters::WRITE_SIMULATIONS] = ParamInfo(Parameters::WRITE_SIMULATIONS, "output-simulations", 3, _power_simulations_section);
    _parameter_info[Parameters::OUTPUT_SIM_FILE] = ParamInfo(Parameters::OUTPUT_SIM_FILE, "output-simulations-file", 4, _power_simulations_section);

    _parameter_info[Parameters::PARALLEL_PROCESSES] = ParamInfo(Parameters::PARALLEL_PROCESSES, "parallel-processes", 1, _run_options_section);

    _parameter_info[Parameters::CREATION_VERSION] = ParamInfo(Parameters::CREATION_VERSION, "parameters-version", 1, _system_section);

    assert(_parameter_info.size() == 38);
}

/** Version 1.2 parameter specifications. */
void IniParameterSpecification::Build_1_2_x_ParameterList() {
    Build_1_1_x_ParameterList();
    _parameter_info[Parameters::DAYOFWEEK_ADJUSTMENT] = ParamInfo(Parameters::DAYOFWEEK_ADJUSTMENT, "perform-day-of-week-adjustments", 1, _adjustments_section);
    _parameter_info[Parameters::REPORT_ATTR_RISK] = ParamInfo(Parameters::REPORT_ATTR_RISK, "report-attributable-risk", 3, _additional_output_section);
    _parameter_info[Parameters::ATTR_RISK_NUM_EXPOSED] = ParamInfo(Parameters::ATTR_RISK_NUM_EXPOSED, "attributable-risk-exposed", 4, _additional_output_section);
    _parameter_info[Parameters::SELF_CONTROL_DESIGN] = ParamInfo(Parameters::SELF_CONTROL_DESIGN, "self-control-design", 4, _analysis_section);

    assert(_parameter_info.size() == 42);
}

/** Version 1.3 parameter specifications. */
void IniParameterSpecification::Build_1_3_x_ParameterList() {
    Build_1_2_x_ParameterList();

    _parameter_info[Parameters::POWER_BASELINE_PROBABILITY] = ParamInfo(Parameters::POWER_BASELINE_PROBABILITY, "baseline-probability", 10, _power_evaluations_section);
    _multiple_parameter_info[Parameters::TREE_FILE] = ParamInfo(Parameters::TREE_FILE, "tree-filename", 3, _advanced_input_section);

    _parameter_info[Parameters::RESTRICT_TREE_LEVELS] = ParamInfo(Parameters::RESTRICT_TREE_LEVELS, "restrict-tree-levels", 4, _inference_section);
    _parameter_info[Parameters::RESTRICTED_TREE_LEVELS] = ParamInfo(Parameters::RESTRICTED_TREE_LEVELS, "excluded-tree-levels", 5, _inference_section);

    assert(_parameter_info.size() == 45);
}

/** Version 1.4 parameter specifications. */
void IniParameterSpecification::Build_1_4_x_ParameterList() {
    Build_1_3_x_ParameterList();

    _parameter_info[Parameters::SEQUENTIAL_SCAN] = ParamInfo(Parameters::SEQUENTIAL_SCAN, "sequential-scan", 1, _sequential_scan_section);
    _parameter_info[Parameters::SEQUENTIAL_MAX_SIGNAL] = ParamInfo(Parameters::SEQUENTIAL_MAX_SIGNAL, "sequential-maximum-signal", 2, _sequential_scan_section);
    _parameter_info[Parameters::SEQUENTIAL_MIN_SIGNAL] = ParamInfo(Parameters::SEQUENTIAL_MIN_SIGNAL, "sequential-minimum-signal", 3, _sequential_scan_section);
    _parameter_info[Parameters::SEQUENTIAL_FILE] = ParamInfo(Parameters::SEQUENTIAL_FILE, "sequential-filename", 4, _sequential_scan_section);
    _parameter_info[Parameters::POWER_Z] = ParamInfo(Parameters::POWER_Z, "power-z", 10, _power_evaluations_section);
    _parameter_info[Parameters::APPLY_RISK_WINDOW_RESTRICTION] = ParamInfo(Parameters::APPLY_RISK_WINDOW_RESTRICTION, "apply-risk-window-restriction", 5, _temporal_window_section);
    _parameter_info[Parameters::RISK_WINDOW_PERCENTAGE] = ParamInfo(Parameters::RISK_WINDOW_PERCENTAGE, "risk-window-percentage", 6, _temporal_window_section);
    _parameter_info[Parameters::APPLY_EXCLUSION_RANGES] = ParamInfo(Parameters::APPLY_EXCLUSION_RANGES, "apply-exclusion-data-ranges", 2, _adjustments_section);
    _parameter_info[Parameters::EXCLUSION_RANGES] = ParamInfo(Parameters::EXCLUSION_RANGES, "exclusion-data-ranges", 3, _adjustments_section);

    _parameter_info[Parameters::MINIMUM_CENSOR_TIME] = ParamInfo(Parameters::MINIMUM_CENSOR_TIME, "minimum-censor-time", 5, _advanced_input_section);
    _parameter_info[Parameters::MINIMUM_CENSOR_PERCENTAGE] = ParamInfo(Parameters::MINIMUM_CENSOR_PERCENTAGE, "min-censor-percentage", 6, _advanced_input_section);
    _parameter_info[Parameters::RSK_WND_CENSOR] = ParamInfo(Parameters::RSK_WND_CENSOR, "risk-window-restriction-censor", 7, _advanced_input_section);
    _parameter_info[Parameters::RSK_WND_ALT_CENSOR_DENOM] = ParamInfo(Parameters::RSK_WND_ALT_CENSOR_DENOM, "risk-window-alt-censor-denominator", 8, _advanced_input_section);

    assert(_parameter_info.size() == 58);
}

/** Version 2.0 parameter specifications. */
void IniParameterSpecification::Build_2_0_x_ParameterList() {
    Build_1_4_x_ParameterList();

    _parameter_info[Parameters::SEQUENTIAL_ALPHA_OVERALL] = ParamInfo(Parameters::SEQUENTIAL_ALPHA_OVERALL, "sequential-alpha-overall", 5, _sequential_scan_section);
    _parameter_info[Parameters::SEQUENTIAL_ALPHA_SPENDING] = ParamInfo(Parameters::SEQUENTIAL_ALPHA_SPENDING, "sequential-alpha-spending", 6, _sequential_scan_section);

	_parameter_info[Parameters::CONTROL_FILE] = ParamInfo(Parameters::CONTROL_FILE, "control-filename", 4, _input_section);

    assert(_parameter_info.size() == 61);
}

/** For sepcified ParameterType, attempts to retrieve ini section and key name if ini file.
    Returns true if parameter found else false. */
bool IniParameterSpecification::GetParameterIniInfo(Parameters::ParameterType eParameterType,  const char ** sSectionName, const char ** sKey) const {
    ParameterInfoMap_t::const_iterator itr = _parameter_info.find(eParameterType);
    if (itr != _parameter_info.end()) {
        *sSectionName = itr->second._section->_label;
        *sKey = itr->second._label;
        return true;
    }
    return false;
}

/** For sepcified ParameterType, attempts to retrieve ini section and key name if ini file.
    Returns true if parameter found else false. */
bool IniParameterSpecification::GetMultipleParameterIniInfo(Parameters::ParameterType eParameterType,  const char ** sSectionName, const char ** sKey) const {
    MultipleParameterInfoMap_t::const_iterator itr = _multiple_parameter_info.find(eParameterType);
    if (itr != _multiple_parameter_info.end()) {
        *sSectionName = itr->second._section->_label;
        *sKey = itr->second._label;
        return true;
    }
    return false;
}

IniParameterSpecification::ParameterInfoCollection_t & IniParameterSpecification::getParameterInfoCollection(ParameterInfoCollection_t& collection) const {
    collection.clear();
    for (ParameterInfoMap_t::const_iterator itr=_parameter_info.begin(); itr != _parameter_info.end(); ++itr) {
        collection.push_back(ParamInfo(itr->second));
    }
    std::sort(collection.begin(), collection.end());
    return collection;
}
