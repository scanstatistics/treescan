//******************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//******************************************************************************
#include "ParametersPrint.h"
#include "PrjException.h"
#include "RandomNumberGenerator.h"

/** Prints parameters, in a particular format, to passed ascii file. */
void ParametersPrint::Print(std::ofstream& out) const {
  try {
    AsciiPrintFormat::PrintSectionSeparatorString(out, 0, 2);
    out << "PARAMETER SETTINGS" << std::endl;

    SettingContainer_t settings;
    //print 'Input' tab settings
    WriteSettingsContainer(getInputParameters(settings), "Input", out);
    // print 'Analysis' tab settings
    WriteSettingsContainer(getAnalysisParameters(settings), "Analysis", out);
    //print 'Output' tab settings
    WriteSettingsContainer(getOutputParameters(settings), "Output", out);
    //print 'RunOptions' settings
    WriteSettingsContainer(getRunOptionsParameters(settings), "Run Options", out);
    //print 'System' parameters
    WriteSettingsContainer(getSystemParameters(settings), "System", out);
    AsciiPrintFormat::PrintSectionSeparatorString(out, 0, 1);
  } catch (prg_exception& x) {
    x.addTrace("Print()","ParametersPrint");
    throw;
  }
}

/** Prints parameters, in a particular format, to passed ascii file. */
void ParametersPrint::PrintHTML(std::ofstream& out) const {
  SettingContainer_t settings;
  out << "<div id=\"parameter-settings\"><h4>Parameter Settings</h4>" << std::endl;
  //print 'Input' tab settings
  WriteSettingsContainerHTML(getInputParameters(settings), "Input", out);
  // print 'Analysis' tab settings
  WriteSettingsContainerHTML(getAnalysisParameters(settings), "Analysis", out);
  //print 'Output' tab settings
  WriteSettingsContainerHTML(getOutputParameters(settings), "Output", out);
  //print 'RunOptions' settings
  WriteSettingsContainerHTML(getRunOptionsParameters(settings), "Run Options", out);
  //print 'System' parameters
  WriteSettingsContainerHTML(getSystemParameters(settings), "System", out);
  out << std::endl << "</div>" << std::endl;
}

/** Prints 'Analysis' tab parameters to file stream. */
ParametersPrint::SettingContainer_t & ParametersPrint::getAnalysisParameters(SettingContainer_t & settings) const {
  std::string        buffer;
  settings.clear();
  buffer = (_parameters.getModelType() == Parameters::POISSON ? "Poisson" : "Bernoulli");
  settings.push_back(std::make_pair("Model",buffer));
  buffer = (_parameters.isConditional() ? "Conditional" : "Unconditional");
  settings.push_back(std::make_pair("Scan Statistic",buffer));
  if (_parameters.getModelType() == Parameters::BERNOULLI) {
    printString(buffer, "%u/%u", _parameters.getProbabilityRatio().first, _parameters.getProbabilityRatio().second);
    settings.push_back(std::make_pair("Event Probability",buffer));
  }
  printString(buffer, "%u", _parameters.getNumReplicationsRequested());  
  settings.push_back(std::make_pair("Number of Replications",buffer));
  //buffer = (_parameters.isDuplicates() ? "Yes" : "No");
  //settings.push_back(std::make_pair("Duplicates",buffer));
  return settings;
}

/** Prints 'Input' tab parameters to file stream. */
ParametersPrint::SettingContainer_t & ParametersPrint::getInputParameters(SettingContainer_t & settings) const {
  settings.clear();
  settings.push_back(std::make_pair("Tree File",_parameters.getTreeFileName()));
  settings.push_back(std::make_pair("Count File",_parameters.getCountFileName()));
  return settings;
}

/** Prints 'Output' tab parameters to file stream. */
ParametersPrint::SettingContainer_t & ParametersPrint::getOutputParameters(SettingContainer_t & settings) const {
  settings.clear();
  settings.push_back(std::make_pair("Results File",_parameters.getOutputFileName()));
  return settings;
}

/** Prints 'Run Options' parameters to file stream. */
ParametersPrint::SettingContainer_t & ParametersPrint::getRunOptionsParameters(SettingContainer_t & settings) const {
  std::string        buffer;
  settings.clear();
  if (_parameters.getNumRequestedParallelProcesses() == 0)
      settings.push_back(std::make_pair("Processer Usage","All Available Proccessors"));
  else {
      printString(buffer, "At Most %u Proccessors", _parameters.getNumRequestedParallelProcesses());
      settings.push_back(std::make_pair("Processer Usage",buffer));
  }
  if (_parameters.isRandomlyGeneratingSeed())
      settings.push_back(std::make_pair("Use Random Seed",(_parameters.isRandomlyGeneratingSeed() ? "Yes" : "No")));
  if (_parameters.getRandomizationSeed() != RandomNumberGenerator::glDefaultSeed) {
      printString(buffer, "%ld\n", _parameters.getRandomizationSeed());
      settings.push_back(std::make_pair("Randomization Seed",buffer));
  }
  return settings;
}

/** Prints 'System' parameters to file stream. */
ParametersPrint::SettingContainer_t & ParametersPrint::getSystemParameters(SettingContainer_t & settings) const {
  const Parameters::CreationVersion  & IniVersion = _parameters.getCreationVersion();
  Parameters::CreationVersion          Current = {atoi(VERSION_MAJOR), atoi(VERSION_MINOR), atoi(VERSION_RELEASE)};  
  std::string                           buffer;
  settings.clear();
  if (IniVersion.iMajor != Current.iMajor ||
      IniVersion.iMinor != Current.iMinor ||
      IniVersion.iRelease != Current.iRelease) {
    printString(buffer, "%u.%u.%u", IniVersion.iMajor, IniVersion.iMinor, IniVersion.iRelease);
    settings.push_back(std::make_pair("Parameters Version",buffer));
  }
  return settings;
}

/** Writes settings container to file stream. */
void ParametersPrint::WriteSettingsContainer(const SettingContainer_t& settings, const std::string& section, std::ofstream& out) const {
  try {
      if (!settings.size()) return;
      //print section label
      out << std::endl;
      out << section;
      out << std::endl;
      for (size_t t=0; t < section.size(); ++t)
          out << "-";
      out << std::endl;
      SettingContainer_t::const_iterator itr=settings.begin();
      //first calculate maximum label length
      size_t tMaxLabel=0;
      for (; itr != settings.end(); ++itr)
            tMaxLabel = std::max(tMaxLabel, itr->first.size());
      //print settings
      for (itr=settings.begin(); itr != settings.end(); ++itr) {
          out << "  ";
          out << itr->first;
          for (size_t t=itr->first.size(); t < tMaxLabel; ++t)
            out << " ";
          out << " : ";
          out << itr->second;
          out << std::endl;
      }
  } catch (prg_exception& x) {
    x.addTrace("WriteSettingsContainer()","ParametersPrint");
    throw;
  }
}

/** Writes settings container to file stream. */
void ParametersPrint::WriteSettingsContainerHTML(const SettingContainer_t& settings, const std::string& section, std::ofstream& out) const {
  try {
      if (!settings.size()) return;
      //print section label
      out << "<div class=\"parameter-section\">" << std::endl;
      out << "<h4>" << section << "</h4>" << std::endl << "<table><tbody>" << std::endl;
      SettingContainer_t::const_iterator itr=settings.begin();
      //print settings
      for (itr=settings.begin(); itr != settings.end(); ++itr)
          out << "<tr><th>" << itr->first << " :</th><td>" << itr->second << "</td></tr>" << std::endl;
      out << std::endl << "</tbody></table></div>";
  } catch (prg_exception& x) {
    x.addTrace("WriteSettingsContainerHTML()","ParametersPrint");
    throw;
  }
}
