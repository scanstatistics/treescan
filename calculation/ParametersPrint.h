//******************************************************************************
#ifndef __ParametersPrint_H
#define __ParametersPrint_H
//******************************************************************************
#include "Parameters.h"
#include "AsciiPrintFormat.h"
#include <iostream>
#include <fstream>

class DataSetHandler; /* forward class declaration */

/** Provides methods to print Parameters in an organized manner to the main output
    file of an analysis. */
class ParametersPrint {
  private:
    typedef std::vector< std::pair<std::string, std::string> > SettingContainer_t;
    const Parameters & _parameters;

    SettingContainer_t & getInputParameters(SettingContainer_t & settings) const;
    SettingContainer_t & getAdvancedInputParameters(SettingContainer_t & settings) const;
    SettingContainer_t & getAnalysisParameters(SettingContainer_t & settings) const;
    SettingContainer_t & getInferenceParameters(SettingContainer_t & settings) const;
    SettingContainer_t & getOutputParameters(SettingContainer_t & settings) const;
    SettingContainer_t & getRunOptionsParameters(SettingContainer_t & settings) const;
    SettingContainer_t & getSystemParameters(SettingContainer_t & settings) const;

    void WriteSettingsContainer(const SettingContainer_t& settings, const std::string& section, std::ostream& out) const;
    void WriteSettingsContainerHTML(const SettingContainer_t& settings, const std::string& section, std::ostream& out) const;

  public:
    ParametersPrint(const Parameters& Parameters) : _parameters(Parameters) {}

    void Print(std::ostream& out) const;
    void PrintHTML(std::ostream& out) const;
};
//******************************************************************************
#endif

