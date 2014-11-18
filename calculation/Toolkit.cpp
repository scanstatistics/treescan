//******************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//******************************************************************************
#include "Toolkit.h"
#include "UtilityFunctions.h"
#include "FileName.h"
#include "PrjException.h"

/** debug file name */
const char * AppToolkit::gsDebugFileName = "_debug_";

/** Default website. */
const char * AppToolkit::gsDefaultTreeScanWebSite = "http://www.treescan.org/";
/** Default Substantive Support Email. */
const char * AppToolkit::gsDefaultSubstantiveSupportEmail = "kulldorff@treescan.org";
/** Default Technical Support Email. */
const char * AppToolkit::gsDefaultTechnicalSupportEmail = "techsupport@treescan.org";
AppToolkit * AppToolkit::gpToolKit = 0;

void AppToolkit::ToolKitCreate(const char * sApplicationFullPath) {
  AppToolkit::gpToolKit = new AppToolkit(sApplicationFullPath);
}

void AppToolkit::ToolKitDestroy() {
  try {delete AppToolkit::gpToolKit; AppToolkit::gpToolKit=0;}catch(...){}
}

/** constructor */
AppToolkit::AppToolkit(const char * sApplicationFullPath) : gpDebugLog(0) {
  try {
    Setup(sApplicationFullPath);
  }
  catch (prg_exception& x) {
    x.addTrace("constructor()", "AppToolkit");
    throw;
  }
}

/** destructor */
AppToolkit::~AppToolkit() {
  try {
    closeDebugFile();
  }
  catch (...){}
}

/** Close debug file handle. */
void AppToolkit::closeDebugFile() {
   if (gpDebugLog) fclose(gpDebugLog);
}

/** Returns acknowledgment statement indicating program version, website, and
    brief declaration of usage agreement. */
const char * AppToolkit::GetAcknowledgment(std::string & Acknowledgment) const {
  printString(Acknowledgment, "You are running TreeScan v%s%s.\n\nTreeScan is free, available for download from %s"
                              ".\nIt may be used free of charge as long as proper "
                              "citations are given\nto both the TreeScan software and the underlying "
                              "statistical methodology.\n\n", GetVersion(), is64Bit() ? " (64-bit)" : "",GetWebSite());
  return Acknowledgment.c_str();
}

/** Returns applications full path */
const char * AppToolkit::GetApplicationFullPath() const {
  return gsApplicationFullPath.c_str();
}

/** Returns substantive support email address. */
const char * AppToolkit::GetSubstantiveSupportEmail() const {
  //return gSession.GetProperty(gsSubstantiveSupportEmailProperty)->GetValue();
  return gsDefaultSubstantiveSupportEmail;
}
/** Returns substantive support email address. */
const char * AppToolkit::GetTechnicalSupportEmail() const {
  //return gSession.GetProperty(gsTechnicalSupportEmailProperty)->GetValue();
  return gsDefaultTechnicalSupportEmail;
}

/** Returns website URL. */
const char * AppToolkit::GetWebSite() const {
  //return gSession.GetProperty(gsTreeScanWebSiteProperty)->GetValue();
  return gsDefaultTreeScanWebSite;
}

/** Returns whether binary is 64-bit. */
bool AppToolkit::is64Bit() const {
  return sizeof(int *) == 8;
}

/** Returns file handle to global debug file. */
FILE * AppToolkit::openDebugFile() {
  try {
	if (!gpDebugLog) {
      std::string filename;
      filename = FileName(gsApplicationFullPath.c_str()).getLocation(filename);
      filename += gsDebugFileName;
      filename += ".log";
      if ((gpDebugLog = fopen(filename.c_str(), /*"a"*/"w")) == NULL)
        throw resolvable_error("Error: Debug file '%s' could not be created.\n", filename.c_str());
	}
  }
  catch (prg_exception& x) {
    x.addTrace("openDebugFile()", "AppToolkit");
    throw;
  }
  return gpDebugLog;
}


/** internal setup */
void AppToolkit::Setup(const char * sApplicationFullPath) {
  try {
    gbRunUpdateOnTerminate = false;
    //set application full path
    gsApplicationFullPath = sApplicationFullPath;
    //Set system ini located at same directory as executable.
    printString(gsVersion, "%s.%s%s%s%s%s", VERSION_MAJOR, VERSION_MINOR,
                          (!strcmp(VERSION_RELEASE, "0") ? "" : "."),
                          (!strcmp(VERSION_RELEASE, "0") ? "" : VERSION_RELEASE),
                          (strlen(VERSION_PHASE) ? " " : ""), VERSION_PHASE);
  }
  catch (prg_exception& x) {
    x.addTrace("Setup()", "AppToolkit");
    throw;
  }
}
