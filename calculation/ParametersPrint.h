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
    const Parameters &  _parameters;

    void                PrintInferenceParameters(std::ofstream& out) const;
    void                PrintInputParameters(std::ofstream& out) const;
    void                PrintOutputParameters(std::ofstream& out) const;
    void                PrintRunOptionsParameters(std::ofstream& out) const;
    void                PrintSystemParameters(std::ofstream& out) const;

    void                WriteSettingsContainer(const SettingContainer_t& settings, const std::string& section, std::ofstream& out) const;

  public:
    ParametersPrint(const Parameters& Parameters);
    ~ParametersPrint();

    void                Print(std::ofstream& out) const;
};
//******************************************************************************
#endif

