//***************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//***************************************************************************
#include "ParameterProgramOptions.h"
#include "PrjException.h"
#include "UtilityFunctions.h"

unsigned int ParameterProgramOptions::ADDITIONAL_TREEFILES = 50;

ParameterProgramOptions::ParameterProgramOptions(Parameters& Parameters, Parameters::CreationVersion version, BasePrint& Print)
    :AbtractParameterFileAccess(Parameters, Print), _specifications(version, Parameters) {}

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

    IniParameterSpecification::ParameterInfoCollection_t parameter_collection;
    _specifications.getParameterInfoCollection(parameter_collection);
    const char * curr_section = "";

    IniParameterSpecification::ParameterInfoCollection_t::iterator itr=parameter_collection.begin(), end=parameter_collection.end();
    for (;itr != end; ++itr) {
        if (!strcmp(itr->_section->_label, IniParameterSpecification::System))
            continue;
        buffer = itr->_label;
        buffer3 = "";
        if (strcmp(curr_section, itr->_section->_label)) {
            opt_descriptions.push_back(ParamOptItem_t(new ParamOpt_t(po::options_description(printString(buffer2, OPT_FORMAT, itr->_section->_label), LINE_WIDTH, LINE_WIDTH/2),true,buffer3)));
            curr_section = itr->_section->_label;
        }
        opt_descriptions.back()->get<0>().add_options()(buffer.c_str(), po::value<std::string>(), GetParameterComment(itr->_type));
    }

    /* multiple tree files (hidden) tab options */
    opt_descriptions.push_back(ParamOptItem_t(new ParamOpt_t(po::options_description(printString(buffer, OPT_FORMAT, "Additional Tree Files"), LINE_WIDTH, LINE_WIDTH/2),false,std::string())));
    for (size_t t=0; t < ADDITIONAL_TREEFILES; ++t) {
        opt_descriptions.back()->get<0>().add_options()
            (printString(buffer, "%s%d", getOption(Parameters::TREE_FILE), t+2).c_str(), po::value<std::string>(), GetParameterComment(Parameters::TREE_FILE));
    }

    return opt_descriptions;
}

/** Overrides settigs  */
bool ParameterProgramOptions::setParameterOverrides(const po::variables_map& vm) {
    std::string buffer;
    for (Parameters::ParameterType eType=Parameters::TREE_FILE; eType <= _parameters.giNumParameters; eType = Parameters::ParameterType(eType + 1)) {
        if (vm.count(getOption(eType)))
            SetParameter(eType, vm[getOption(eType)].as<std::string>(), gPrintDirection);
    }
    /* manually scan for multiple tree file parameters */
    for (size_t t=0; t < ADDITIONAL_TREEFILES; ++t) {
        printString(buffer, "%s%d", getOption(Parameters::TREE_FILE), t+2);
        if (vm.count(buffer.c_str())) {
            _parameters.setTreeFileName(vm[buffer.c_str()].as<std::string>().c_str(), true, t + 2);
        }
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
