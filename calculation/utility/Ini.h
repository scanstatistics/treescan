//******************************************************************************
#ifndef __INI_H
#define __INI_H
//******************************************************************************
#include <vector>
#include <string>
#include "ptr_vector.h"

// Ini Line Class
class IniLine {
  private:
    std::string                 gsKey;
    std::string                 gsValue;

  public:
    IniLine(const char *sIniLine = 0);
    IniLine(const char *sKey, const char *sValue);
    IniLine(const IniLine &rhs);
    virtual ~IniLine();

    IniLine                   & operator=(const IniLine &rhs);
    bool                        operator==(const IniLine &rhs) const;

    virtual IniLine           * Clone() const;
    const char                * GetKey() const;
    const char                * GetValue() const;
    void                        SetKey(const char *sKey);
    void                        SetLine(const char *sLine);
    void                        SetValue(const char *sValue);
};

class IniSection {
  public:
    typedef std::vector< std::vector<std::string> > CommentContainer_t;

  private:
    ptr_vector<IniLine>         gaIniLines;
    CommentContainer_t          gvCommentLineVectors;//vector of vectors of "comment" lines: gvCommentLineVectors[0] contains the lines that precede gaIniLines[0].  gCommentLineVectors[gaIniLines.size()] contains the lines that follow gaIniLines[gaIniLines.size() - 1] (the last non-comment line).
    std::string                 gsSectionName;
    mutable bool                gbModified;

   void                         Copy(const IniSection &rhs);

  public:
    IniSection(const char *sSectionName=0);
    IniSection(const IniSection &rhs);
    virtual ~IniSection();

    IniSection                & operator=(const IniSection &rhs);
    void                        AddLine(const char *sLine);
    void                        AddLine(const char * sKey, const char * sValue);
    void                        AddComment(const char * sComment);
    void                        ClearSection();
    virtual IniSection        * Clone() const;
    long                        FindKey(const char *sKeyName) const;
    bool                        GetBool(const char *sKeyName, bool bDefault = false) const;
    int                         GetInt(const char *sKeyName, int iDefault = 0) const;
    bool                        GetIsModified() const;
    IniLine                   * GetLine(long lIndex) const;
    const char                * GetName() const;
    long                        GetNumLines() const;
    const char                * GetString(std::string& buffer, const char *sKeyName, const char *sDefault = "") const;
    void                        Read(std::ifstream& readstream);
    bool                        RemoveLine( const char *sKeyName, const char *sEntry = 0 );
    void                        RemoveLine( long lIndex );
    void                        SetBool(const char *sKeyName, bool bValue);
    void                        SetInt(const char *sKeyName, int iNumber);
    void                        SetName(const char *sSectionName);
    void                        SetString(const char *sKeyName, const char *sEntry);
    void                        Write(std::ofstream& writestream) const;
};

class IniFile {
  private:
    mutable ptr_vector<IniSection> gaSections;

    bool                        SeekToNextSection(std::ifstream& readstream) const;

  public:
    IniFile();
    virtual ~IniFile();

    void                        AddSection(IniSection *pSection);
    void                        AddSection(const IniSection& Section);
    IniSection                * AddSection(const char *sSectionName);
    void                        Clear();
    void                        DeleteSection(long lIndex);
    void                        DeleteSection(const char *sSectionName);
    IniSection                * FindFirstSection(const char *sKey, const char *sValue);
    long                        FindSectionLine(const char *SectionLine) const;
    bool                        GetIsModified() const;
    long                        GetNumSections() const;
    IniSection                * GetSection(long lIndex);
    const IniSection          * GetSection(long lIndex) const;
    IniSection                * GetSection(const char *sSectionName);
    long                        GetSectionIndex(const char *sSectionName, long lStartPosition = 0) const;
    const char                * GetSectionName(long lIndex);
    void                        Read(const std::string& file);
    void                        Write(const std::string& file) const;
};
//******************************************************************************
#endif

