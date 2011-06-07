//***************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//***************************************************************************
#include "Parameters.h"
#include "PrjException.h"
#include "RandomNumberGenerator.h"

const int Parameters::giNumParameters = 9;

bool  Parameters::operator==(const Parameters& rhs) const {
  if (_replications                   != rhs._replications) return false;
  if (_cuts                           != rhs._cuts) return false;
  if (_treeFileName                   != rhs._treeFileName) return false;
  if (_countFileName                  != rhs._countFileName) return false;
  if (_outputFileName                 != rhs._outputFileName) return false;
  if (_parametersSourceFileName       != rhs._parametersSourceFileName) return false;
  //if (_creationVersion              != rhs._creationVersion) return false;
  //if (_randomizationSeed            != rhs._randomizationSeed) return false;
  if (_numRequestedParallelProcesses  != rhs._numRequestedParallelProcesses) return false;
  if (_randomlyGenerateSeed           != rhs._randomlyGenerateSeed) return false;
  if (_conditional                    != rhs._conditional) return false;
  if (_duplicates                     != rhs._duplicates) return false;  

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
  _replications                   = rhs._replications;
  _cuts                           = rhs._cuts;
  _treeFileName                   = rhs._treeFileName;
  _countFileName                  = rhs._countFileName;
  _outputFileName                 = rhs._outputFileName;
  _creationVersion                = rhs._creationVersion;
  _parametersSourceFileName       = rhs._parametersSourceFileName;
  _randomizationSeed              = rhs._randomizationSeed;
  _numRequestedParallelProcesses  = rhs._numRequestedParallelProcesses;
  _randomlyGenerateSeed           = rhs._randomlyGenerateSeed;
  _conditional                    = rhs._conditional;
  _duplicates                     = rhs._duplicates;
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

  if (!stricmp(fInputFilename.getLocation(buffer).c_str(), fParameterName.getLocation(buffer2).c_str()))
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
  _countFileName                  = "";
  _treeFileName                   = "";
  _outputFileName                 = "";
  _replications                   = 999;
  _cuts                           = 2000;
  _creationVersion.iMajor         = atoi(VERSION_MAJOR);
  _creationVersion.iMinor         = atoi(VERSION_MINOR);
  _creationVersion.iRelease       = atoi(VERSION_RELEASE);
  _randomizationSeed              = RandomNumberGenerator::glDefaultSeed;
  _numRequestedParallelProcesses  = 0;
  _randomlyGenerateSeed           = false;
  _conditional                    = false;
  _duplicates                     = false;
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
