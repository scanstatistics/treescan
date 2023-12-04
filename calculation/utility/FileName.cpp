//******************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//******************************************************************************
#include "FileName.h"
#include "UtilityFunctions.h"
#include "PrjException.h"

const char * FileName::UNC_TAG = "\\\\";

const char FileName::BACKSLASH = '\\';
const char FileName::FORWARDSLASH = '/';

/** The constructor will break up sName into four parts by calling SetFullName().  If NULL is passed
    in, zero length strings are stored in the data members. */
FileName::FileName(const char* sName) {
  setFullPath(sName);
}

/** Copy Constructor */
FileName::FileName(const FileName &rhs) {
  *this = rhs;
}

FileName &FileName::operator=(const FileName &rhs) {
  if (this != &rhs) {
    setDrive(rhs.gsDrive.c_str());
    setDirectory(rhs.gsDirectory.c_str());
    setFileName(rhs.gsFileName.c_str());
    setExtension(rhs.gsExtension.c_str());
  }
  return (*this);
}

/** Overloaded equality operator */
bool FileName::operator==(const FileName& rhs) const {
  if (gsDrive != rhs.gsDrive) return false;
  if (gsDirectory != rhs.gsDirectory) return false;
  if (gsFileName != rhs.gsFileName) return false;
  if (gsExtension != rhs.gsExtension) return false;
  return true;
}

/** Returns the platform file path separator character. */
char FileName::getPathSeparator() {
#ifdef _WINDOWS_
    return BACKSLASH;
#else
    return FORWARDSLASH;
#endif
}


/** This function converts the name stored in this FileName into a filename
    with an absolute path. (i.e. the path will not have ".", ".." or any
    symbolic links in it). */
void FileName::convertToAbsolutePath() {
  std::string fullPath;

#ifdef _WINDOWS_
  char sBuffer[MAX_PATH];
  if (GetFullPathName((LPCTSTR)getFullPath(fullPath).c_str(), MAX_PATH, (LPTSTR)sBuffer, 0))
#else
  char sBuffer[PATH_MAX];
  if (realpath(getFullPath(fullPath).c_str(), sBuffer))
#endif
     setFullPath(sBuffer);
}

/** This function returns the current working directory. */
const char* FileName::getCurDirectory(std::string& theDirectory) {
#ifdef _WINDOWS_
      theDirectory = FileName("test.ini").getDirectory();
#else
      theDirectory = "./";
#endif
   return theDirectory.c_str();
}

/** This function returns the current working drive. */
const char* FileName::getCurDrive(std::string & theDrive) {
#ifdef _WINDOWS_
      theDrive = FileName("test.ini").getDrive();
#else
      theDrive = "";
#endif
   return theDrive.c_str();
}

/** This function returns the fully qualified file name. */
std::string & FileName::getFullPath(std::string& fullPath) const {
   fullPath = gsDrive + gsDirectory + gsFileName + gsExtension;
   return fullPath;
}

/** Returns the location of the file. ( i.e. <drive>:\<directory> ) */
std::string & FileName::getLocation(std::string& location) const {
   location = gsDrive + gsDirectory;
   return location;
}

/** These are Windows reserved special characters - this will have to be ifndef'd in */
bool FileName::isSpecialCharacter(char c) const {
#ifdef _WINDOWS_
   return (c=='<' || c=='>' || c==':' || c=='"' || c=='/' || c=='\\' || c=='|');
#else
   return (c=='/' || c=='\\');
#endif
}

/** This function will set the directory. */
void FileName::setDirectory(const char * sNewDirectory) {
  if (!sNewDirectory) return;
  // copy directory and append slash
  gsDirectory = sNewDirectory;
  if (gsDirectory.size())
    if (gsDirectory.find_last_of(getPathSeparator()) != gsDirectory.size() - 1)
       gsDirectory += getPathSeparator();
}

void FileName::setDrive(const char * sNewDrive) {
  if (!sNewDrive) return;
  gsDrive = sNewDrive;
  // if not UNC name and does not end in colon, add one
  if (gsDrive.size())
    if (gsDrive.find(UNC_TAG) != 0 && gsDrive.find(":") == gsDrive.npos)
      gsDrive += ":";
}

/**  This function will set the extension. */
void FileName::setExtension(const char* sNewExtension) {
  if (!sNewExtension) return;
  gsExtension = sNewExtension;
  if (sNewExtension[0] != '.' && strlen(sNewExtension))
    gsExtension.insert(0, ".");
}

// This function will set the file name and only the filename.
void FileName::setFileName(const char* sNewFile) {
  if (!sNewFile) return;
  gsFileName = sNewFile;
}

/** This function will assign the class to this file name.  This function will
    parse the filename according to the following rules:
    #   An extension is set after the last '.' in sFileName.
    #   A filename is set after the last slash and before the extension (if any)
    #   The drive is set from the start of sFileName to the first slash.  UNC is allowed.
    #     For a UNC name the drive would be the entire share (i.e. \\nfsc\imsdev)
    #   The directory is set to the string between the drive and the filename. */
void FileName::setFullPath(const char* sFileName) {
  if (!sFileName) return;

  std::string  sWorkPath;
  // Copy the path to a work area
#ifdef _WINDOWS_
  char *pTrash;
  char sFullName[MAX_PATH];

  sFullName[0] = '\0';
  // This will add on the drive and directory if not already specified
  if (sFileName[0])
    if (!GetFullPathName((LPCTSTR)sFileName, MAX_PATH, (LPTSTR)sFullName, (LPTSTR*)&pTrash))
      throw prg_error("Could not get full path name", "setFullPath");
  sWorkPath = sFullName;
#else
  sWorkPath = sFileName;
#endif
  trimString(sWorkPath);
  // parse the extension (everything after the last ".", unless there is a special character
  size_t lPosition = sWorkPath.rfind('.');   // find last "."
  // if found, make sure no special character in extension
  for (size_t i=lPosition; i < sWorkPath.size() && (lPosition != std::string::npos); ++i)
     if (isSpecialCharacter(sWorkPath[i]))
       lPosition = std::string::npos;
  // set extension if one exists
  if (lPosition != std::string::npos) {
    setExtension(sWorkPath.substr(lPosition).c_str());
    sWorkPath.erase(lPosition);  // remove the extension
  } else
    setExtension("");
   // Parse the filename
   lPosition = sWorkPath.rfind(getPathSeparator());
   if (lPosition == std::string::npos)  // not found
     lPosition = sWorkPath.rfind(':');
   if (lPosition != std::string::npos) { // found slash or :
     setFileName(sWorkPath.substr(++lPosition).c_str());
     sWorkPath.erase(lPosition);  // remove the filename
   } else {
     setFileName(sWorkPath.c_str());
     sWorkPath = "";    // no drive or directory, so clear the filename
   }
   setLocation(sWorkPath.c_str());
}

/** This function will assign the class to this file name.  This function will
    parse the filename according to the following rules:
    #   The drive is set from the start of sFileName to the first slash.  UNC is allowed.
    #     For a UNC name the drive would be the entire share (i.e. \\nfsc\imsdev)
    #   The directory is set to the string following the drive. */
void FileName::setLocation(const char* sLocation) {
  if (!sLocation) return;

  std::string  sWorkPath = sLocation;
  trimString(sWorkPath);
  // Parse the drive
  if (sWorkPath.find(UNC_TAG) == 0) { // UNC name
    size_t lPosition = sWorkPath.find("\\", 2);
    if (lPosition == std::string::npos || lPosition < 1) // drive must be at least 1 character
      throw prg_error("Invalid UNC name","setLocation");
    // goto next slash to get full sharename
    size_t lPosition2 = sWorkPath.find("\\", 2 + lPosition + 1);
    if (lPosition2 == std::string::npos || lPosition2 < 1) // share name must be at least 1 character
      throw prg_error("Invalid UNC name","setLocation");
    setDrive(sWorkPath.substr(0, lPosition2).c_str());
    sWorkPath.erase(0, lPosition2); // remove the drive name
  } else  { // DOS Name
    size_t lPosition = sWorkPath.find(":");
    if (lPosition == 1) { // drives are 1 character long
      setDrive(sWorkPath.substr(0, 2).c_str());
      sWorkPath.erase(0, 2);
    } else
      setDrive("");
  }
  setDirectory(sWorkPath.c_str()); // set the directory to what is left
}

