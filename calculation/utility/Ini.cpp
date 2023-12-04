//******************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//******************************************************************************
#include "Ini.h"
#include "FileName.h"
#include "UtilityFunctions.h"
#include "PrjException.h"
#include <iostream>
#include <fstream>

/** Constructs an IniLine object from an ini line (or default) */
IniLine::IniLine(const char *sIniLine) {
  SetLine(sIniLine);
}

/** Constructs an IniLine object with an existing key and value */
IniLine::IniLine(const char *sKey, const char *sValue) {
  SetKey(sKey);
  SetValue(sValue);
}

/** Copy constructor */
IniLine::IniLine(const IniLine &rhs) {
  SetKey(rhs.gsKey.c_str());
  SetValue(rhs.gsValue.c_str());
}

/** Destructor */
IniLine::~IniLine() {}

/** Overloaded assignment operator */
IniLine &IniLine::operator=(const IniLine &rhs) {
  if (this != &rhs) {
    SetKey(rhs.gsKey.c_str());
    SetValue(rhs.gsValue.c_str());
  }
  return (*this);
}

/** Overloaded equality operator */
bool IniLine::operator==(const IniLine &rhs) const {
  return (!stricmp(gsKey.c_str(), rhs.gsKey.c_str()));
}

/** Pointer copy constructor */
IniLine *IniLine::Clone() const {
  return new IniLine(*this);
}

/** Returns the current key */
const char * IniLine::GetKey() const {
   return gsKey.c_str();
}

/** Returns the current value */
const char *IniLine::GetValue() const {
   return gsValue.c_str();
}

/** Sets the current key to the passed character string */
void IniLine::SetKey(const char *sString) {
  gsKey = sString;
  trimString(gsKey);
}

/** Sets the key and value based on the passed character string */
void IniLine::SetLine(const char *sString) {
  std::string sLine(sString);

  size_t pos = sLine.find('=');
  if (pos != sLine.npos) {
    gsKey = sLine.substr(0, pos);
    trimString(gsKey);
    gsValue = sLine.substr(pos+1);
    trimString(gsValue);
  }
  else {
    gsValue = "";
    gsKey = sLine;
    trimString(gsKey);
  }
  /** Remove quotes */
  if (gsValue.size() >= 2 && (gsValue[0] == '"' && gsValue[gsValue.size()-1] == '"')) {
    gsValue.erase(0, 1);
    gsValue.erase(gsValue.size() - 1);
  }

}

/** Sets the current value to the passed character string */
void IniLine::SetValue(const char *sString) {
  gsValue = sString;
}

/** ClassDesc Begin IniSection
    This class provides reading & writing of ini sections.
    The client can access each IniLine by index: GetLine(lIndex).
    <br>What about comments?  Comments are currently allowed only as "single line"
    comments: where a line in the file begins with a semicolon (';'), it is treated
    as a comment, i.e., it is not made accessible through this interface.
    Sometimes, however, you may want to build a section and include comments for the
    benefit of the human reader of the file.  In this case, you should build your
    IniSection from scratch, using AddComment() to append a comment line at the appropriate
    time.
ClassDesc End */

/** Cnstructor */
IniSection::IniSection(const char *sSectionName)
           : gaIniLines(0), gvCommentLineVectors(1), gbModified(false) {
  SetName(sSectionName);
  gbModified = false;
}

/** Copy constructor */
IniSection::IniSection(const IniSection &rhs)
           : gaIniLines(rhs.gaIniLines), gvCommentLineVectors(rhs.gvCommentLineVectors), gbModified(rhs.gbModified) {
  Copy(rhs);
}

/** Destructor - free memory */
IniSection::~IniSection() {}

/** Overloaded assignment operator */
IniSection & IniSection::operator= (const IniSection &rhs) {
  if (this != &rhs) {
    gaIniLines = rhs.gaIniLines;
    gvCommentLineVectors = rhs.gvCommentLineVectors;
    Copy(rhs);
  }  
  return (*this);
}

/** Append a "comment line" to this section. */
void IniSection::AddComment(const char * sComment) {
  if ( sComment && sComment[0] ) {
    gbModified = true;
    gvCommentLineVectors.back().push_back(sComment);
  }
}

/** Creates a new IniLine from sLine and Adds to End of Section */
void IniSection::AddLine(const char *sLine) {
  if ( sLine && sLine[0] ) {
    gbModified = true;
    gaIniLines.push_back(new IniLine(sLine));
    gvCommentLineVectors.resize(gvCommentLineVectors.size() + 1);
  }
}

/** Creates a new IniLine from sKey & sEntry and Adds to End of Section */
void IniSection::AddLine(const char * sKey, const char * sValue) {
  gbModified = true;
  gaIniLines.push_back(new IniLine(sKey, sValue));
  gvCommentLineVectors.resize(gvCommentLineVectors.size() + 1);
}

/** Clears all lines from section and set flag to write - used to delete section or
    if starting from scratch */
void IniSection::ClearSection() {
  gaIniLines.killAll();
  gvCommentLineVectors.resize(1);
  gvCommentLineVectors.back().clear();
  gbModified = true;
}

IniSection * IniSection::Clone() const {
   return new IniSection ( *this );
}

/** Copies the values of an existing IniSection object */
void IniSection::Copy(const IniSection &rhs) {
  SetName(rhs.gsSectionName.c_str());
  gbModified = rhs.gbModified;
}

/** Returns the index of sKey
    returns -1 if sKey is not found */
long IniSection::FindKey(const char *sKey) const {
   long      lIndex, lKeyIndex = -1, lNumLines = GetNumLines();

   if (sKey) {
     for (lIndex = 0;lIndex < lNumLines && lKeyIndex == -1;lIndex++)
        if (stricmp(sKey, gaIniLines[lIndex]->GetKey()) == 0)
          lKeyIndex = lIndex;
   }
   return lKeyIndex;
}

/** Gets the entry (right side of =) for sKey
    Returns true if entry is "true", "yes" or 1 (case insensitive)
    if not found - returns default (default is has false as its default) */
bool IniSection::GetBool(const char *sKeyName, bool bDefault) const {
   std::string sReturn;
   bool bReturn = bDefault;

   GetString(sReturn, sKeyName, "");
   if (sReturn.size())  {
     trimString(sReturn);
     bReturn = (!stricmp(sReturn.c_str(), "true") || !stricmp(sReturn.c_str(), "yes") || (atoi(sReturn.c_str()) == 1));
   }
   return bReturn;
}

/** Gets the entry (right side of =) for sKey
    If entry not a valid integer or sKey not found - returns Default */
int IniSection::GetInt(const char *sKeyName, int iDefault) const {
   std::string sReturn;
   int  iReturn = iDefault;

   GetString(sReturn, sKeyName, "");
   if (sReturn.size()) {
     trimString(sReturn);
     if (strspn(sReturn.c_str(), "-0123456789") == strlen(sReturn.c_str()))
       iReturn = atoi(sReturn.c_str());
     else if ( sReturn[0] == '0' && sReturn[1] == 'x' )
       sscanf ( sReturn.c_str(), "%x", &iReturn );
   }
   return iReturn;
}

/** Returns gbModified */
bool IniSection::GetIsModified() const {
   return gbModified;
}

IniLine * IniSection::GetLine(long lIndex) const {
   return gaIniLines[lIndex];
}

/** Get the Section Name - includes the brackets (ie. [FileInfo] */
const char * IniSection::GetName() const {
   return gsSectionName.c_str();
}

/** Returns the number of IniLines in the Section */
long IniSection::GetNumLines() const {
  return static_cast<long>(gaIniLines.size());
}

/** Get the entry (right side of = ) for sKey */
const char * IniSection::GetString(std::string& buffer, const char *sKeyName, const char *sDefault) const {
   long       lKey;

   lKey = FindKey(sKeyName);
   if (lKey >= 0)  // found key
     buffer = gaIniLines.at(lKey)->GetValue();
   else
     buffer = sDefault;
   return buffer.c_str();
}

/**  Read the ini-section - file pointer must be currently at the first line
     after the section header - file pointer will be pointing at the next
     section header or end of file after call
     sValue - must use ""s to retain leading or trailing blank (ie. 4="  sub-site"
     internal quotes will be retained - only if 1st and last non-blank chars are quotes are they removed */
void IniSection::Read(std::ifstream& readstream) {
  std::istream::pos_type        filePos = readstream.tellg();
  std::string                   buffer, sKey, sValue;
  bool     bDoneSection = false;


  while (!bDoneSection && getlinePortable(readstream, buffer)) {
       trimString(buffer);
       if (buffer.size() == 0) {
         filePos = readstream.tellg();
         continue;
       }
       bDoneSection = (buffer[0] == '[');
       if (!bDoneSection && buffer.size()) {
         if (buffer[0] == ';') {
           buffer.erase(0, 1); // strip the comment character.
           gvCommentLineVectors.back().push_back(buffer);
         }
         else
           AddLine(buffer.c_str());
       }
       else if (bDoneSection) {
         readstream.clear();
         readstream.seekg(filePos, std::ios::beg);
       }
       filePos = readstream.tellg();
  }
  gbModified = false;
}
 
/** Removes line lIndex from the section. If lIndex is not valid, an exception will
    be generated. */
void IniSection::RemoveLine ( long lIndex ) {
   std::vector<std::string>::iterator itrCurrentComment, itrEndOfComments;

   gaIniLines.kill( gaIniLines.begin() + lIndex );
   // tack the strings in gvCommentLineVectors.at(lIndex + 1) onto the end of
   // gvCommentLineVectors.at(lIndex) and then erase gvCommentLineVectors.at(lIndex + 1):
   itrCurrentComment = gvCommentLineVectors.at(lIndex + 1).begin();
   itrEndOfComments = gvCommentLineVectors.at(lIndex + 1).end();
   while (itrCurrentComment != itrEndOfComments) {
       gvCommentLineVectors.at(lIndex).push_back(*itrCurrentComment); // resize(gvCommentLineVectors.at(lIndex).size() + 1);
        ++itrCurrentComment;
    }
    gvCommentLineVectors.erase(gvCommentLineVectors.begin() + (lIndex + 1));
}

/** Removes line with key of sKeyName from the section, if it exists. If sEntry is
    supplied, the function checks to make sure that the value of the found key matches
    before it deletes it. Returns TRUE if the line was successfully deleted; FALSE
    otherwise. */
bool IniSection::RemoveLine ( const char *sKeyName, const char *sEntry ) {
   bool   bRetVal;
   long   lIndex;

   lIndex = FindKey ( sKeyName );

   // Depends on short-circuit evaluation. (i.e. "!sEntry" prevents stricmp from being called)
   bRetVal = ( lIndex != -1 ) && ( !sEntry || !stricmp ( sEntry, gaIniLines[lIndex]->GetValue() ) );

   if ( bRetVal )
     RemoveLine ( lIndex );
   return bRetVal;
}

/** Set the entry for this key to "true" or "false" (sets flag for write)
    if key not in section - it is added to end of section */
void IniSection::SetBool(const char *sKeyName, bool bValue) {
  gbModified = true;
  if (bValue)
    SetString(sKeyName, "true");
  else
    SetString(sKeyName, "false");
}

/** Set the entry for this key to iNumber (sets flag for write)
    if key not in section - it is added to end of section */
void IniSection::SetInt(const char *sKeyName, int iNumber){
   char sEntry[50];

   gbModified = true;
   sprintf(sEntry, "%d", iNumber);
   SetString(sKeyName, sEntry);
}

/** Adds brackets around section name and sets it */
void IniSection::SetName(const char *sSectionName) {
  if (sSectionName && (sSectionName != gsSectionName)) {
    gbModified = true;
    if (sSectionName[0] == '[')
      gsSectionName = sSectionName;
    else
      printString(gsSectionName, "[%s]", sSectionName);
  }    
}

/** Set the entry for this key to sEntry (sets flag for write)
    if key not in section - it is added to end of section */
void IniSection::SetString(const char *sKeyName, const char *sEntry) {
   long       lKey;

   if (sKeyName && sEntry) {
     lKey = FindKey(sKeyName);
     if (lKey >= 0) { // found key
       if (strlen(sEntry))
         gaIniLines[lKey]->SetValue(sEntry);
       else
         gaIniLines.kill(gaIniLines.begin() + lKey);
       gbModified = true;
     }
     else if (strlen(sEntry)) {
       AddLine(sKeyName, sEntry);
       gbModified = true;
     }
     // else do nothing - blank added
   }                                                     
}

/** Function to write the Ini section to disk */
void IniSection::Write(std::ofstream& writestream) const {
  long lNumLines = GetNumLines();
  if (lNumLines) {
    // write section header
    writestream << gsSectionName << std::endl;
    for (long i=0; i < lNumLines; ++i) {
       // write preceding comment lines first
       for (long j=0; (unsigned)j < gvCommentLineVectors.at(i).size(); ++j) {
          writestream << ";" << gvCommentLineVectors.at(i).at(j) << std::endl;
       }
       // then write the IniLine
       const IniLine * pIniLine = gaIniLines[i];
       std::string sTemp = pIniLine->GetKey();
       size_t lLength = sTemp.size();
       if ((lLength > 0) && (sTemp[0] == ' ' || sTemp[lLength-1] == ' ') ) {
         printString(sTemp, "\"%s\"", pIniLine->GetKey());
       }
       writestream << sTemp << "=";
       sTemp = pIniLine->GetValue();
       lLength = sTemp.size();
       if ((lLength > 0) && (sTemp[0] == ' ' || sTemp[lLength-1] == ' ') ) {
         printString(sTemp, "\"%s\"", pIniLine->GetValue());
       }
       writestream << sTemp << std::endl;
    }
    // write trailing comment lines
    for (long j=0; (unsigned)j < gvCommentLineVectors.back().size(); ++j) {
       writestream << ";" << gvCommentLineVectors.back().at(j) << std::endl;
    }
    writestream << std::endl;
  }
  gbModified = false;
}
 
/** ClassDesc Begin IniFile
    This class contains an entire ini file as an array of all IniSections. It
    is important to note that all sections associated with this IniFile will go
    out of scope when the file does. This _also_ applies to any sections you may
    have added to the file via AddSection(IniSection*).
    ClassDesc End */

//--- Member Functions

IniFile::IniFile() : gaSections(0) {}

/** Destructor - frees memory */
IniFile::~IniFile() {}

/** Add a section to the ini-file. This function absorbs pSection into the inifile
    don't delete pSection! */
void IniFile::AddSection(IniSection *pSection) {
  if (GetSectionIndex(pSection->GetName()) >= 0)
    throw prg_error("Section '%s' already exists.", "IniFile", pSection->GetName());
  gaSections.push_back(pSection);
}

/** Add a section to the ini-file */
void IniFile::AddSection(const IniSection& Section) {
   std::auto_ptr<IniSection> pSection(new IniSection(Section));
   AddSection(pSection.release());
}

/** Add a section to the ini-file */
IniSection * IniFile::AddSection(const char *sSectionName) {
   IniSection *pSection;
   pSection = new IniSection(sSectionName);
   AddSection(pSection);
   return pSection;
}

/** Clears out the ini file */
void IniFile::Clear() {
  gaSections.kill ( gaSections.begin(), gaSections.end() );
}

/** Deletes the specified section from the file. The file will not be actually removed on
    disk until it is written back. If the section is not in the file, nothing happens. */
void IniFile::DeleteSection ( const char *sSectionName ) {
   long     lIndex;
   lIndex = GetSectionIndex ( sSectionName );
   if ( lIndex >= 0 )
     gaSections.kill(gaSections.begin() + lIndex );
}

/** Deletes the specified section from the file. The file will not be actually removed on
    disk until it is written back. */
void IniFile::DeleteSection ( long lIndex ) {
  gaSections.kill(gaSections.begin() + lIndex );
}

/** FindFirstSection returns a pointer to the first Section which contains the key=value combination,
    returns null if no section is found */
IniSection * IniFile::FindFirstSection(const char *sKey, const char *sValue) {
   IniSection *pSection = 0;
   long         i, lNumSect;
   bool         bFound = false;
   std::string  buffer;

   lNumSect = GetNumSections();
   for (i = 0;i < lNumSect && !bFound;i++) {
     pSection = GetSection(i);
     bFound = (stricmp(pSection->GetString(buffer, sKey), sValue) == 0);
   }
   if (!bFound)
     pSection = 0;
   return pSection;
}

long IniFile::FindSectionLine(const char *SectionLine) const {
  std::string   __SectionLine = SectionLine;

  size_t _open = __SectionLine.find('[');
  size_t _close = __SectionLine.find("].");
  if (_open == __SectionLine.npos || _close == __SectionLine.npos)
    return -1;
  std::string SectionName = __SectionLine.substr(_open, _close);
  std::string LineKey = __SectionLine.substr(_close+2);
  long section_index = GetSectionIndex(SectionName.c_str());
  if (section_index != -1) {
    long key_index = GetSection(section_index)->FindKey(LineKey.c_str());
    if (key_index != -1)
      return key_index;
  }
  return -1;
}

/** Returns whether any of the sections have been modified */
bool IniFile::GetIsModified() const {
   long          lNumSections, i;
   bool          bAnyMod = false;

   lNumSections = GetNumSections();
   for (i = 0;i < lNumSections && !bAnyMod;i++)
     bAnyMod = gaSections[i]->GetIsModified();
   return bAnyMod;
}

/** Number of sections in file (not number loaded) */
long IniFile::GetNumSections() const {
   return static_cast<long>(gaSections.size());
}

/** GetSection returns a pointer to the Section, creating section if it does not exist */
IniSection * IniFile::GetSection(const char *sSectionName) {
   long lIndex = GetSectionIndex(sSectionName);
   return (lIndex >= 0 ? GetSection(lIndex) : AddSection(sSectionName));
}

/** GetSection will load the section if it has not already been loaded */
IniSection * IniFile::GetSection(long lIndex) {
   return gaSections.at(lIndex);
}

/** GetSection will load the section if it has not already been loaded */
const IniSection * IniFile::GetSection(long lIndex) const {
   return gaSections.at(lIndex);
}

/** Returns the index from gpIniSections of the SectionName passed in
    returns -1 if not found. */
long IniFile::GetSectionIndex(const char *sSectionName, long lStartPosition) const {
  long          lIndex = -1, i, lNumSections = GetNumSections();
  std::string   sNewName;

  if (sSectionName) {
    if (sSectionName[0] == '[')
      sNewName = sSectionName;
    else
      printString(sNewName, "[%s]", sSectionName);
     for (i=lStartPosition; i < lNumSections && lIndex == -1; ++i)
        if (!stricmp(gaSections[i]->GetName(), sNewName.c_str()))
          lIndex = i;
  }
  return lIndex;
}
 
/** GetSectionName */
const char * IniFile::GetSectionName(long lIndex) {
   IniSection *pIniSection;

   pIniSection = gaSections.at(lIndex);
   return pIniSection->GetName();
}

/** Internal function to move thru file and point to next section header - even
    if it is the current pos (i.e. calling multiple times in a row does nothing)
    This function requires the caller to have placed a READ lock on the file. */
bool IniFile::SeekToNextSection(std::ifstream& readstream) const {
  std::istream::pos_type        Pos = readstream.tellg();
  std::string                   buffer;

  while (getlinePortable(readstream, buffer)) {
       trimString(buffer);
       if (buffer.size() && buffer[0] == '[' && buffer[buffer.size()-1] == ']') {
         readstream.clear();
         readstream.seekg(Pos, std::ios::beg);
         return true;
       }
       Pos = readstream.tellg();
  }
  return false;
}

void IniFile::Read(const std::string& file) {
  std::ifstream  readstream;
  std::string    buffer;

  // open file stream
  readstream.open(file.c_str(), std::ios::binary);
  if (!readstream) throw resolvable_error("Error: Could not open file '%s'.\n", file.c_str());

  while (SeekToNextSection(readstream)) {
      getlinePortable(readstream, buffer);
      trimString(buffer);
      std::auto_ptr<IniSection> pIniSection(new IniSection(buffer.c_str()));
      pIniSection->Read(readstream);
      gaSections.push_back(pIniSection.release());
  }
}

void IniFile::Write(const std::string& file) const {
  std::ofstream writestream;

  // open output file
  writestream.open(file.c_str(), std::ios::trunc);
  if (!writestream) throw resolvable_error("Error: Could not open file '%s'.\n", file.c_str());

  for (long i=0; i < GetNumSections(); ++i)
     gaSections[i]->Write(writestream);
}

