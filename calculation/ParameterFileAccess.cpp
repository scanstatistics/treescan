//***************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//***************************************************************************
#include "ParameterFileAccess.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/detail/xml_parser_error.hpp>

/** Determines format of parameter file and invokes particular parameter reader class to read parameters from file. */
bool ParameterAccessCoordinator::read(const std::string& filename) {
    bool  success=false;
    try {
        if (access(filename.c_str(), 04) == -1)
            throw resolvable_error("Unable to open settings file '%s'.\n", filename.c_str());

        // first try to open file as XML
        Parameters::ParametersFormat file_format(Parameters::XML);
        while (!success && file_format <= Parameters::JSON) {
            try {
                _parameters.read(filename, file_format);
                success=true;
            } catch (boost::property_tree::xml_parser::xml_parser_error&) {file_format = Parameters::ParametersFormat(file_format + 1);}
        }
        success=true;
    } catch (prg_exception& x) {
        throw resolvable_error("Unable to read parameters from file '%s'.\n", filename.c_str());
    }
    return success;
}

/** Writes parameters to file in most recent format. */
void ParameterAccessCoordinator::write(const std::string& filename) {
    _parameters.write(filename, Parameters::XML);
}

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

