//***************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//***************************************************************************
#include "ParameterProgramOptions.h"
#include "PrjException.h"
#include "UtilityFunctions.h"

const char * ParameterProgramOptions::getOption(Parameters::ParameterType e, bool withShortName) const {
    const char  * sSectionName, * sKey;

    if (!_specifications.GetParameterIniInfo(e, &sSectionName, &sKey))
        throw prg_error("Unknown parameter type '%d'.", "getOption()", e);
    return sKey;
}

/** Returns program options for CParameter class. All program options are defiend as std::string we that we can have reading and errors
    between program options and file based parameter reading. */
ParameterProgramOptions::ParamOptContainer_t & ParameterProgramOptions::getOptions(ParamOptContainer_t& opt_descriptions) {
    const char * OPT_FORMAT = "%s options";
    const int LINE_WIDTH = 150;
    std::string buffer, buffer2, buffer3;

    /* Analysis tab options */
    opt_descriptions.push_back(ParamOptItem_t(new ParamOpt_t(po::options_description(printString(buffer, OPT_FORMAT, IniParameterSpecification::Analysis), LINE_WIDTH, LINE_WIDTH/2),true,std::string())));
    opt_descriptions.back()->get<0>().add_options()
        (getOption(Parameters::SCAN_TYPE, true), po::value<std::string>(), GetParameterComment(Parameters::SCAN_TYPE))
        (getOption(Parameters::CONDITIONAL_TYPE, true), po::value<std::string>(), GetParameterComment(Parameters::CONDITIONAL_TYPE))
        (getOption(Parameters::MODEL_TYPE, true), po::value<std::string>(), GetParameterComment(Parameters::MODEL_TYPE))
        (getOption(Parameters::EVENT_PROBABILITY, true), po::value<std::string>(), GetParameterComment(Parameters::EVENT_PROBABILITY))
        (getOption(Parameters::START_DATA_TIME_RANGE, true), po::value<std::string>(), GetParameterComment(Parameters::START_DATA_TIME_RANGE))
        (getOption(Parameters::END_DATA_TIME_RANGE, true), po::value<std::string>(), GetParameterComment(Parameters::END_DATA_TIME_RANGE));

    /* Input tab options */
    opt_descriptions.push_back(ParamOptItem_t(new ParamOpt_t(po::options_description(printString(buffer, OPT_FORMAT, IniParameterSpecification::Input), LINE_WIDTH, LINE_WIDTH/2),true,std::string())));
    opt_descriptions.back()->get<0>().add_options()
        (getOption(Parameters::TREE_FILE, true), po::value<std::string>(), GetParameterComment(Parameters::TREE_FILE))
        (getOption(Parameters::COUNT_FILE, true), po::value<std::string>(), GetParameterComment(Parameters::COUNT_FILE))
        (getOption(Parameters::DATA_TIME_RANGES, true), po::value<std::string>(), GetParameterComment(Parameters::DATA_TIME_RANGES));

    /* Output tab options */
    opt_descriptions.push_back(ParamOptItem_t(new ParamOpt_t(po::options_description(printString(buffer, OPT_FORMAT, IniParameterSpecification::Output), LINE_WIDTH, LINE_WIDTH/2),true,std::string())));
    opt_descriptions.back()->get<0>().add_options()
        (getOption(Parameters::RESULTS_FILE, true), po::value<std::string>(), GetParameterComment(Parameters::RESULTS_FILE))
        (getOption(Parameters::RESULTS_HTML, true), po::value<std::string>(), GetParameterComment(Parameters::RESULTS_HTML))
        (getOption(Parameters::RESULTS_CSV, true), po::value<std::string>(), GetParameterComment(Parameters::RESULTS_CSV));

    /* Temporal Window tab options */
    opt_descriptions.push_back(ParamOptItem_t(new ParamOpt_t(po::options_description(printString(buffer, OPT_FORMAT, IniParameterSpecification::TemporalWindow), LINE_WIDTH, LINE_WIDTH/2),true,std::string())));
    opt_descriptions.back()->get<0>().add_options()
        (getOption(Parameters::MAXIMUM_WINDOW_PERCENTAGE, true), po::value<std::string>(), GetParameterComment(Parameters::MAXIMUM_WINDOW_PERCENTAGE))
        (getOption(Parameters::MAXIMUM_WINDOW_FIXED, true), po::value<std::string>(), GetParameterComment(Parameters::MAXIMUM_WINDOW_FIXED))
        (getOption(Parameters::MAXIMUM_WINDOW_TYPE, true), po::value<std::string>(), GetParameterComment(Parameters::MAXIMUM_WINDOW_TYPE))
        (getOption(Parameters::MINIMUM_WINDOW_FIXED, true), po::value<std::string>(), GetParameterComment(Parameters::MINIMUM_WINDOW_FIXED));

    /* Inference tab options */
    opt_descriptions.push_back(ParamOptItem_t(new ParamOpt_t(po::options_description(printString(buffer, OPT_FORMAT, IniParameterSpecification::Inference), LINE_WIDTH, LINE_WIDTH/2),true,std::string())));
    opt_descriptions.back()->get<0>().add_options()
        (getOption(Parameters::REPLICATIONS, true), po::value<std::string>(), GetParameterComment(Parameters::REPLICATIONS))
        (getOption(Parameters::RANDOMIZATION_SEED, true), po::value<std::string>(), GetParameterComment(Parameters::RANDOMIZATION_SEED))
        (getOption(Parameters::RANDOMLY_GENERATE_SEED, true), po::value<std::string>(), GetParameterComment(Parameters::RANDOMLY_GENERATE_SEED));

    /* Power Evaluations options */
    opt_descriptions.push_back(ParamOptItem_t(new ParamOpt_t(po::options_description(printString(buffer, OPT_FORMAT, IniParameterSpecification::PowerEvaluations), LINE_WIDTH, LINE_WIDTH/2),true,std::string())));
    opt_descriptions.back()->get<0>().add_options()
        (getOption(Parameters::POWER_EVALUATIONS, true), po::value<std::string>(), GetParameterComment(Parameters::POWER_EVALUATIONS))
        (getOption(Parameters::POWER_EVALUATION_TYPE, true), po::value<std::string>(), GetParameterComment(Parameters::POWER_EVALUATION_TYPE))
        (getOption(Parameters::CRITICAL_VALUES_TYPE, true), po::value<std::string>(), GetParameterComment(Parameters::CRITICAL_VALUES_TYPE))
        (getOption(Parameters::CRITICAL_VALUE_05, true), po::value<std::string>(), GetParameterComment(Parameters::CRITICAL_VALUE_05))
        (getOption(Parameters::CRITICAL_VALUE_01, true), po::value<std::string>(), GetParameterComment(Parameters::CRITICAL_VALUE_01))
        (getOption(Parameters::CRITICAL_VALUE_001, true), po::value<std::string>(), GetParameterComment(Parameters::CRITICAL_VALUE_001))
        (getOption(Parameters::POWER_EVALUATION_TOTALCASES, true), po::value<std::string>(), GetParameterComment(Parameters::POWER_EVALUATION_TOTALCASES))
        (getOption(Parameters::POWER_EVALUATIONS_REPLICA, true), po::value<std::string>(), GetParameterComment(Parameters::POWER_EVALUATIONS_REPLICA))
        (getOption(Parameters::POWER_EVALUATIONS_FILE, true), po::value<std::string>(), GetParameterComment(Parameters::POWER_EVALUATIONS_FILE));

    /* Advanced Input tab options */
    opt_descriptions.push_back(ParamOptItem_t(new ParamOpt_t(po::options_description(printString(buffer, OPT_FORMAT, IniParameterSpecification::AdvancedInput), LINE_WIDTH, LINE_WIDTH/2),true,std::string())));
    opt_descriptions.back()->get<0>().add_options()
        (getOption(Parameters::CUT_FILE, true), po::value<std::string>(), GetParameterComment(Parameters::CUT_FILE))
        (getOption(Parameters::CUT_TYPE, true), po::value<std::string>(), GetParameterComment(Parameters::CUT_TYPE))
        (getOption(Parameters::DUPLICATES, true), po::value<std::string>(), GetParameterComment(Parameters::DUPLICATES));

    /* Additional Output tab options */
    opt_descriptions.push_back(ParamOptItem_t(new ParamOpt_t(po::options_description(printString(buffer, OPT_FORMAT, IniParameterSpecification::AdditionalOutput), LINE_WIDTH, LINE_WIDTH/2),true,std::string())));
    opt_descriptions.back()->get<0>().add_options()
        (getOption(Parameters::RESULTS_LLR, true), po::value<std::string>(), GetParameterComment(Parameters::RESULTS_LLR))
        (getOption(Parameters::REPORT_CRITICAL_VALUES, true), po::value<std::string>(), GetParameterComment(Parameters::REPORT_CRITICAL_VALUES));

    /* Power Simulations options */
    opt_descriptions.push_back(ParamOptItem_t(new ParamOpt_t(po::options_description(printString(buffer, OPT_FORMAT, IniParameterSpecification::PowerSimulations), LINE_WIDTH, LINE_WIDTH/2),true,std::string())));
    opt_descriptions.back()->get<0>().add_options()
        (getOption(Parameters::READ_SIMULATIONS, true), po::value<std::string>(), GetParameterComment(Parameters::READ_SIMULATIONS))
        (getOption(Parameters::INPUT_SIM_FILE, true), po::value<std::string>(), GetParameterComment(Parameters::INPUT_SIM_FILE))
        (getOption(Parameters::WRITE_SIMULATIONS, true), po::value<std::string>(), GetParameterComment(Parameters::WRITE_SIMULATIONS))
        (getOption(Parameters::OUTPUT_SIM_FILE, true), po::value<std::string>(), GetParameterComment(Parameters::OUTPUT_SIM_FILE));

    /* Run Options tab options */
    opt_descriptions.push_back(ParamOptItem_t(new ParamOpt_t(po::options_description(printString(buffer, OPT_FORMAT, IniParameterSpecification::RunOptions), LINE_WIDTH, LINE_WIDTH/2),true,std::string())));
    opt_descriptions.back()->get<0>().add_options()
        (getOption(Parameters::PARALLEL_PROCESSES, true), po::value<std::string>(), GetParameterComment(Parameters::PARALLEL_PROCESSES));

    return opt_descriptions;
}

/** Overrides settigs  */
bool ParameterProgramOptions::setParameterOverrides(const po::variables_map& vm) {
    std::string buffer;
    for (Parameters::ParameterType eType=Parameters::TREE_FILE; eType <= _parameters.giNumParameters; eType = Parameters::ParameterType(eType + 1)) {
        if (vm.count(getOption(eType)))
            SetParameter(eType, vm[getOption(eType)].as<std::string>(), gPrintDirection);
    }
    return !_read_error;
}

void ParameterProgramOptions::listOptions(FILE * fp) {
    std::string buffer;
    for (Parameters::ParameterType e=Parameters::TREE_FILE; e <= _parameters.giNumParameters; e = Parameters::ParameterType(e + 1)) {
        if (e != Parameters::CREATION_VERSION || e != Parameters::RANDOMIZATION_SEED || e != Parameters::RANDOMLY_GENERATE_SEED) { // skip certain parameters
            const char * option = getOption(e, true);
            fprintf(fp, " --%s \"%s\" ", option, GetParameterString(e, buffer).c_str());
        }
    }
}

bool ParameterProgramOptions::Read(const char* szFilename) {
    throw prg_error("ParameterProgramOptions::Read(const char*) not implemented.", "Read()");
    return false;
}
void ParameterProgramOptions::Write(const char * szFilename) {
    throw prg_error("ParameterProgramOptions::Write(const char*) not implemented.", "Write()");
}
