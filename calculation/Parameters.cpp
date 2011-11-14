//***************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//***************************************************************************
#include "Parameters.h"
#include "PrjException.h"
#include "RandomNumberGenerator.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/json_parser.hpp>

const int Parameters::giNumParameters = 11;

bool  Parameters::operator==(const Parameters& rhs) const {
  if (_replications != rhs._replications) return false;
  if (_treeFileName != rhs._treeFileName) return false;
  if (_countFileName != rhs._countFileName) return false;
  if (_outputFileName != rhs._outputFileName) return false;
  if (_resultsFormat != rhs._resultsFormat) return false;
  if (_parametersSourceFileName != rhs._parametersSourceFileName) return false;
  //if (_creationVersion != rhs._creationVersion) return false;
  //if (_randomizationSeed != rhs._randomizationSeed) return false;
  if (_numRequestedParallelProcesses != rhs._numRequestedParallelProcesses) return false;
  if (_randomlyGenerateSeed != rhs._randomlyGenerateSeed) return false;
  if (_conditional != rhs._conditional) return false;
  if (_duplicates != rhs._duplicates) return false;  
  if (_printColumnHeaders != rhs._printColumnHeaders) return false; 
  if (_modelType != rhs._modelType) return false;
  if (_probablility_ratio != rhs._probablility_ratio) return false;

  return true;
}

/** If passed filename contains a slash, then assumes that path is complete and
    sInputFilename is not modified. If filename does not contain a slash, it is
    assumed that filename is located in same directory of parameter file.
    sInputFilename is reset to this location. Note that the primary reason for
    implementing this feature was to permit the program to be installed in any
    location and sample parameter files run immediately without having to edit
    input file paths. */
void Parameters::assignMissingPath(std::string & sInputFilename, bool bCheckWritable) {
  FileName      fParameterFilename, fFilename;
  std::string   buffer;

  if (! sInputFilename.empty()) {
    //Assume that if slashes exist, then this is a complete file path, so
    //we'll make no attempts to determine what path might be otherwise.
    if (sInputFilename.find(FileName::SLASH) == sInputFilename.npos) {
      //If no slashes, then this file is assumed to be in same directory as parameters file.
      fParameterFilename.setFullPath(getSourceFileName().c_str());

      fFilename.setFullPath(sInputFilename.c_str());
      fFilename.setLocation(fParameterFilename.getLocation(buffer).c_str());

      if (bCheckWritable && !ValidateFileAccess(fFilename.getFullPath(buffer), true)) {
        // if writability fails, then try setting to user documents directory
        std::string temp;
        fFilename.setLocation(GetUserDocumentsDirectory(buffer, fParameterFilename.getLocation(temp)).c_str());
      }

      fFilename.getFullPath(sInputFilename);
    }
  }
}

/** Copies all class variables from the given Parameters object (rhs) into this one */
void Parameters::copy(const Parameters &rhs) {
  _replications = rhs._replications;
  _treeFileName = rhs._treeFileName;
  _countFileName = rhs._countFileName;
  _outputFileName = rhs._outputFileName;
  _resultsFormat = rhs._resultsFormat;
  _creationVersion = rhs._creationVersion;
  _parametersSourceFileName = rhs._parametersSourceFileName;
  _randomizationSeed = rhs._randomizationSeed;
  _numRequestedParallelProcesses = rhs._numRequestedParallelProcesses;
  _randomlyGenerateSeed = rhs._randomlyGenerateSeed;
  _conditional = rhs._conditional;
  _duplicates = rhs._duplicates;
  _printColumnHeaders = rhs._printColumnHeaders;
  _modelType = rhs._modelType;
  _probablility_ratio = rhs._probablility_ratio;
}

/** Returns number of parallel processes to run. */
unsigned int Parameters::getNumParallelProcessesToExecute() const {
#ifdef RPRTCMPT_RUNTIMES
  // reporting of run-time components is not thread safe at this time,
  // and has no useful purpose to be such - at this time
  return 1;
#else
  unsigned int  iNumProcessors;

  if (_numRequestedParallelProcesses <= 0)
    //parameter of zero or less indicates that we want all available processors
    iNumProcessors = GetNumSystemProcessors();
  else
    //else parameter indicates the maximum number of processors to use
    iNumProcessors = std::min(_numRequestedParallelProcesses, GetNumSystemProcessors());
    //iNumProcessors = giNumRequestedParallelProcesses;

  return iNumProcessors;
#endif  
}

/** If passed filename has same path as passed parameter filename, returns 'name.extension' else returns filename. */
const char * Parameters::getRelativeToParameterName(const FileName& fParameterName,
                                                     const std::string& sFilename,
                                                     std::string& sValue) const {
  FileName      fInputFilename(sFilename.c_str());
  std::string   buffer, buffer2;

  #if defined(_WINDOWS_)
  if (!_stricmp(fInputFilename.getLocation(buffer).c_str(), fParameterName.getLocation(buffer2).c_str()))
  #else
  if (!stricmp(fInputFilename.getLocation(buffer).c_str(), fParameterName.getLocation(buffer2).c_str()))
  #endif
    sValue = fInputFilename.getFileName() + fInputFilename.getExtension();
  else
    sValue = sFilename.c_str();
  return sValue.c_str();
}


/** Sets counts data file name.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file does not exist. */
void Parameters::setCountFileName(const char * sCountFileName, bool bCorrectForRelativePath) {
  _countFileName = sCountFileName;
  if (bCorrectForRelativePath) assignMissingPath(_countFileName);
}

/** Sets counts data file name.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file does not exist. */
void Parameters::setTreeFileName(const char * sTreeFileName, bool bCorrectForRelativePath) {
  _treeFileName = sTreeFileName;
  if (bCorrectForRelativePath) assignMissingPath(_treeFileName);
}


/** initializes global variables to default values */
void Parameters::setAsDefaulted() {
  _countFileName = "";
  _treeFileName = "";
  _outputFileName = "";
  _resultsFormat = TEXT;
  _replications = 99999;
  _creationVersion.iMajor = atoi(VERSION_MAJOR);
  _creationVersion.iMinor = atoi(VERSION_MINOR);
  _creationVersion.iRelease = atoi(VERSION_RELEASE);
  _randomizationSeed = RandomNumberGenerator::glDefaultSeed;
  _randomlyGenerateSeed = false;
  _numRequestedParallelProcesses = 0;
  _conditional = false;
  _duplicates = false;
  _printColumnHeaders = true;
  _modelType = POISSON;
  _probablility_ratio = ratio_t(1,2);
}

/** Sets output data file name.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file
    does not exist. */
void Parameters::setOutputFileName(const char * sOutPutFileName, bool bCorrectForRelativePath) {
  _outputFileName = sOutPutFileName;
  if (bCorrectForRelativePath) assignMissingPath(_outputFileName, true);
}

/** Sets filename of file used to load parameters. */
void Parameters::setSourceFileName(const char * sParametersSourceFileName) {
  //Use FileName class to ensure that a relative path is expanded to absolute path.
  std::string buffer;
  _parametersSourceFileName = FileName(sParametersSourceFileName).getFullPath(buffer);
}

void Parameters::read(const std::string &filename, ParametersFormat type) {
    using boost::property_tree::ptree;
    ptree pt;

    switch (type) {
        case INI: read_ini(filename, pt); break;
        case JSON: read_json(filename, pt); break;
        case XML: 
        default: read_xml(filename, pt);
    }
    setSourceFileName(filename.c_str());
    setTreeFileName(pt.get<std::string>(type == INI ? "input.tree-file" : "parameters.input.tree-file").c_str(), true);
    setCountFileName(pt.get<std::string>(type == INI ? "input.count-file" : "parameters.input.count-file").c_str(), true);
    _duplicates = pt.get<bool>(type == INI ? "input.duplicates" : "parameters.input.count-file.<xmlattr>.duplicates", false);
    _modelType = static_cast<ModelType>(pt.get<unsigned int>(type == INI ? "analysis.model" : "parameters.analysis.model", POISSON));
    _probablility_ratio.first = pt.get<unsigned int>(type == INI ? "analysis.probability-numerator" : "parameters.analysis.probability-numerator", 1);
    _probablility_ratio.second = pt.get<unsigned int>(type == INI ? "analysis.probability-denominator" : "parameters.analysis.probability-denominator", 2);
    _replications = pt.get<unsigned int>(type == INI ? "analysis.replications" : "parameters.analysis.replications", 999);
    _conditional = pt.get<bool>(type == INI ? "analysis.conditional" : "parameters.analysis.conditional", false);
    setOutputFileName(pt.get<std::string>(type == INI ? "output.results-file" : "parameters.output.results-file").c_str(), true);
    _resultsFormat = pt.get<bool>(type == INI ? "output.html" : "parameters.output.results-file.<xmlattr>.html", true) ? HTML : TEXT;
    _printColumnHeaders = pt.get<bool>(type == INI ? "output.print-headers" : "parameters.output.print-headers", true);
    _numRequestedParallelProcesses = pt.get<unsigned int>(type == INI ? "execute-options.processors" : "parameters.execute-options.processors", 0);
    _randomizationSeed = pt.get<unsigned int>(type == INI ? "execute-options.seed" : "parameters.execute-options.seed", static_cast<unsigned int>(RandomNumberGenerator::glDefaultSeed));
    _randomlyGenerateSeed = pt.get<bool>(type == INI ? "execute-options.generate-seed" : "parameters.execute-options.generate-seed", false);
}

void Parameters::write(const std::string &filename, ParametersFormat type) const {
    using boost::property_tree::ptree;
    using boost::property_tree::xml_writer_settings;
    ptree pt;

    pt.put(type != XML ? "input.tree-file" : "parameters.input.tree-file", _treeFileName);
    pt.put(type != XML ? "input.count-file" : "parameters.input.count-file", _countFileName);
    pt.put(type != XML ? "input.duplicates" : "parameters.input.count-file.<xmlattr>.duplicates", _duplicates);
    pt.put(type != XML ? "analysis.model" : "parameters.analysis.model", static_cast<unsigned int>(_modelType));
    pt.put(type != XML ? "analysis.probability-numerator" : "parameters.analysis.probability-numerator", _probablility_ratio.first);
    pt.put(type != XML ? "analysis.probability-denominator" : "parameters.analysis.probability-denominator", _probablility_ratio.second);
    pt.put(type != XML ? "analysis.replications" : "parameters.analysis.replications", _replications);
    pt.put(type != XML ? "analysis.conditional" : "parameters.analysis.conditional", _conditional);
    pt.put(type != XML ? "output.results-file" : "parameters.output.results-file", _outputFileName);
    pt.put(type != XML ? "output.html" : "parameters.output.results-file.<xmlattr>.html", _resultsFormat == HTML);
    pt.put(type != XML ? "output.print-headers" : "parameters.output.print-headers", _printColumnHeaders);
    pt.put(type != XML ? "execute-options.processors" : "parameters.execute-options.processors", _numRequestedParallelProcesses);
    pt.put(type != XML ? "execute-options.seed" : "parameters.execute-options.seed", _randomizationSeed);
    pt.put(type != XML ? "execute-options.generate-seed" : "parameters.execute-options.generate-seed", _randomlyGenerateSeed);
    switch (type) {
        case INI: write_ini(filename, pt); break;
        case JSON: write_json(filename, pt); break;
        case XML:  
        default: {
            xml_writer_settings<char> w(' ', 4);
            write_xml(filename, pt, std::locale(), w);
        }
    }
}
