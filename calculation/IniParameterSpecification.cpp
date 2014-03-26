//***************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//***************************************************************************
#include "IniParameterSpecification.h"

const char * IniParameterSpecification::Input                   = "Input";
const char * IniParameterSpecification::AdvancedInput           = "Advanced Input";
const char * IniParameterSpecification::Analysis                = "Analysis";
const char * IniParameterSpecification::Inference               = "Inference";
const char * IniParameterSpecification::Output                  = "Output";
const char * IniParameterSpecification::PowerSimulations        = "Power Simulations";

const char * IniParameterSpecification::RunOptions              = "Run Options";
const char * IniParameterSpecification::System                  = "System";

/** constructor -- builds specification for write process */
IniParameterSpecification::IniParameterSpecification() {
  Build_1_1_x_ParameterList();

  // Error check
  if (gvParameterInfo.size() != static_cast<size_t>(Parameters::giNumParameters))
      throw prg_error("Parameter list size (%u) not equal to number of parameters (%d).","GetParameterComment()", gvParameterInfo.size(), Parameters::giNumParameters);
}

/** constructor -- builds specification for read process */
IniParameterSpecification::IniParameterSpecification(const IniFile& SourceFile, Parameters& Parameters) {
  long                          lSectionIndex, lKeyIndex;
  Parameters::CreationVersion   Version = {atoi(VERSION_MAJOR), atoi(VERSION_MINOR), atoi(VERSION_RELEASE)};

  if ((lSectionIndex = SourceFile.GetSectionIndex(System)) > -1) {
    const IniSection * pSection = SourceFile.GetSection(lSectionIndex);
    if ((lKeyIndex = pSection->FindKey("Version")) > -1) {
      sscanf(pSection->GetLine(lKeyIndex)->GetValue(), "%u.%u.%u", &Version.iMajor, &Version.iMinor, &Version.iRelease);
    }
  }
  Parameters.setVersion(Version);
  Build_1_1_x_ParameterList();

  // Error check
  if (gvParameterInfo.size() != static_cast<size_t>(Parameters::giNumParameters))
      throw prg_error("Parameter list size (%u) not equal to number of parameters (%d).","GetParameterComment()", gvParameterInfo.size(), Parameters::giNumParameters);
}

/** destructor */
IniParameterSpecification::~IniParameterSpecification() {}

/** Version 1.1 parameter specifications. */
void IniParameterSpecification::Build_1_1_x_ParameterList() {
    // Order in vector is essential - should identical to ParameterType enumeration.
    gvParameterInfo.push_back(std::make_pair(Input, (const char*)"tree-filename"));
    gvParameterInfo.push_back(std::make_pair(Input, (const char*)"case-filename"));
    gvParameterInfo.push_back(std::make_pair(Input, (const char*)"population-filename"));
    gvParameterInfo.push_back(std::make_pair(Input, (const char*)"data-time-range"));
    gvParameterInfo.push_back(std::make_pair(AdvancedInput, (const char*)"cut-filename"));
    gvParameterInfo.push_back(std::make_pair(AdvancedInput, (const char*)"cut-type"));
    gvParameterInfo.push_back(std::make_pair(AdvancedInput, (const char*)"duplicates"));

    gvParameterInfo.push_back(std::make_pair(Analysis, (const char*)"scan-type"));
    gvParameterInfo.push_back(std::make_pair(Analysis, (const char*)"conditional-type"));
    gvParameterInfo.push_back(std::make_pair(Analysis, (const char*)"probability-model"));
    gvParameterInfo.push_back(std::make_pair(Analysis, (const char*)"event-probability"));
    gvParameterInfo.push_back(std::make_pair(Analysis, (const char*)"window-start-range"));
    gvParameterInfo.push_back(std::make_pair(Analysis, (const char*)"window-end-range"));
    gvParameterInfo.push_back(std::make_pair(Inference, (const char*)"monte-carlo-replications"));
    gvParameterInfo.push_back(std::make_pair(Inference, (const char*)"randomization-seed"));
    gvParameterInfo.push_back(std::make_pair(Inference, (const char*)"random-randomization-seed"));

    gvParameterInfo.push_back(std::make_pair(Output, (const char*)"results-filename"));
    gvParameterInfo.push_back(std::make_pair(Output, (const char*)"results-html"));
    gvParameterInfo.push_back(std::make_pair(Output, (const char*)"results-csv"));
    gvParameterInfo.push_back(std::make_pair(Output, (const char*)"results-llr"));

    gvParameterInfo.push_back(std::make_pair(PowerSimulations, (const char*)"input-simulations"));
    gvParameterInfo.push_back(std::make_pair(PowerSimulations, (const char*)"input-simulations-file"));
    gvParameterInfo.push_back(std::make_pair(PowerSimulations, (const char*)"output-simulations"));
    gvParameterInfo.push_back(std::make_pair(PowerSimulations, (const char*)"output-simulations-file"));

    gvParameterInfo.push_back(std::make_pair(RunOptions, (const char*)"parallel-processes"));

    gvParameterInfo.push_back(std::make_pair(System, (const char*)"parameters-version"));
}

/** For sepcified ParameterType, attempts to retrieve ini section and key name if ini file.
    Returns true if parameter found else false. */
bool IniParameterSpecification::GetParameterIniInfo(Parameters::ParameterType eParameterType,  const char ** sSectionName, const char ** sKey) const {
  size_t        tParamIndex = static_cast<size_t>(eParameterType); //remember that ParameterType enumeration starts at one
  bool          bReturn = false;

  if (tParamIndex > 0  && tParamIndex <= gvParameterInfo.size()) {
    *sSectionName = gvParameterInfo[tParamIndex - 1].first;
    *sKey = gvParameterInfo[tParamIndex - 1].second;
    bReturn = true;
  }
  return bReturn;
}
