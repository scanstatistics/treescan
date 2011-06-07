//******************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//******************************************************************************
#include "ParametersPrint.h"
#include "PrjException.h"
#include "RandomNumberGenerator.h"

/** constructor*/
ParametersPrint::ParametersPrint(const Parameters& Parameters) : _parameters(Parameters) {}

/** destructor */
ParametersPrint::~ParametersPrint() {}


/** Prints parameters, in a particular format, to passed ascii file. */
void ParametersPrint::Print(std::ofstream& out) const {
  try {
    AsciiPrintFormat::PrintSectionSeparatorString(out, 0, 2);
    out << "PARAMETER SETTINGS" << std::endl; //fprintf(fp, "PARAMETER SETTINGS\n");

    //print 'Input' tab settings
    PrintInputParameters(out);
    // print 'Inference' tab settings
    PrintInferenceParameters(out);
    //print 'Output' tab settings
    PrintOutputParameters(out);
    //print 'RunOptions' settings
    PrintRunOptionsParameters(out);
    //print 'System' parameters
    PrintSystemParameters(out);
    AsciiPrintFormat::PrintSectionSeparatorString(out, 0, 1);
  }
  catch (prg_exception& x) {
    x.addTrace("Print()","ParametersPrint");
    throw;
  }
}

/** Prints 'Analysis' tab parameters to file stream. */
void ParametersPrint::PrintInferenceParameters(std::ofstream& out) const {
  SettingContainer_t settings;
  std::string        buffer;

  try {
    buffer = (_parameters.isConditional() ? "Conditional" : "Unconditional");
    settings.push_back(std::make_pair("Analysis",buffer));
    printString(buffer, "%u", _parameters.getNumReplicationsRequested());  
    settings.push_back(std::make_pair("Number of Replications",buffer));
    buffer = (_parameters.isDuplicates() ? "Yes" : "No");
    settings.push_back(std::make_pair("Duplicates",buffer));
    WriteSettingsContainer(settings, "Inference", out);
  }
  catch (prg_exception& x) {
    x.addTrace("PrintInferenceParameters()","ParametersPrint");
    throw;
  }
}

/** Prints 'Input' tab parameters to file stream. */
void ParametersPrint::PrintInputParameters(std::ofstream& out) const {
  SettingContainer_t    settings;

  try {
    settings.push_back(std::make_pair("Tree File",_parameters.getTreeFileName()));
    settings.push_back(std::make_pair("Count File",_parameters.getCountFileName()));
    WriteSettingsContainer(settings, "Input", out);
  }
  catch (prg_exception& x) {
    x.addTrace("PrintInputParameters()","ParametersPrint");
    throw;
  }
}

/** Prints 'Output' tab parameters to file stream. */
void ParametersPrint::PrintOutputParameters(std::ofstream& out) const {
  SettingContainer_t  settings;

  try {
    settings.push_back(std::make_pair("Results File",_parameters.getOutputFileName()));
    WriteSettingsContainer(settings, "Output", out);
  }
  catch (prg_exception& x) {
    x.addTrace("PrintOutputParameters()","ParametersPrint");
    throw;
  }
}

/** Prints 'Run Options' parameters to file stream. */
void ParametersPrint::PrintRunOptionsParameters(std::ofstream& out) const {
  SettingContainer_t settings;
  std::string        buffer;

  try {
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
    WriteSettingsContainer(settings, "Run Options", out);
  }
  catch (prg_exception& x) {
    x.addTrace("PrintRunOptionsParameters()","ParametersPrint");
    throw;
  }
}

/** Prints 'System' parameters to file stream. */
void ParametersPrint::PrintSystemParameters(std::ofstream& out) const {
  const Parameters::CreationVersion  & IniVersion = _parameters.getCreationVersion();
  Parameters::CreationVersion          Current = {atoi(VERSION_MAJOR), atoi(VERSION_MINOR), atoi(VERSION_RELEASE)};
  SettingContainer_t                    settings;
  std::string                           buffer;

  if (IniVersion.iMajor != Current.iMajor ||
      IniVersion.iMinor != Current.iMinor ||
      IniVersion.iRelease != Current.iRelease) {
    printString(buffer, "%u.%u.%u", IniVersion.iMajor, IniVersion.iMinor, IniVersion.iRelease);
    settings.push_back(std::make_pair("Parameters Version",buffer));
    WriteSettingsContainer(settings, "System", out);
  }
}

/** Writes settings container to file stream. */
void ParametersPrint::WriteSettingsContainer(const SettingContainer_t& settings, const std::string& section, std::ofstream& out) const {
  try {
      if (!settings.size()) return;

      //print section label
      out << std::endl; //fprintf(fp, "\n");  
      out << section; //fprintf(fp, section.c_str());  
      out << std::endl; //fprintf(fp, "\n");  
      for (size_t t=0; t < section.size(); ++t)
          out << "-";  //fprintf(fp, "-"); 
      out << std::endl; //fprintf(fp, "\n");  

      SettingContainer_t::const_iterator itr=settings.begin();
      //first calculate maximum label length
      size_t tMaxLabel=0;
      for (; itr != settings.end(); ++itr) {
            tMaxLabel = std::max(tMaxLabel, itr->first.size());
      }
      //print settings
      for (itr=settings.begin(); itr != settings.end(); ++itr) {
          out << "  "; //fprintf(fp, "  "); 
          out << itr->first; //fprintf(fp, itr->first.c_str()); 
          for (size_t t=itr->first.size(); t < tMaxLabel; ++t)
            out << " "; //fprintf(fp, " "); 
          out << " : "; //fprintf(fp, " : "); 
          out << itr->second; //fprintf(fp, itr->second.c_str()); 
          out << std::endl; //fprintf(fp, "\n"); 
      }
  }
  catch (prg_exception& x) {
    x.addTrace("WriteSettingsContainer()","ParametersPrint");
    throw;
  }
}
