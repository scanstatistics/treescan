//******************************************************************************
#ifndef __FILENAME_H
#define __FILENAME_H
//******************************************************************************
#include <string>

class FileName {
  private:
    std::string         gsDrive;
    std::string         gsDirectory;
    std::string         gsFileName;
    std::string         gsExtension;

    bool                isSpecialCharacter(char c) const;

  public:
    FileName(const char* sName=0);
    FileName(const FileName &rhs);
    virtual ~FileName() {}

    static const char   SLASH;
    static const char * UNC_TAG;
    static const char * getCurDirectory(std::string & theDirectory);
    static const char * getCurDrive(std::string & theDrive);

    FileName &operator=(const FileName &rhs);
    bool operator==(const FileName &rhs) const;

    void                convertToAbsolutePath();
    const std::string & getDirectory() const {return gsDirectory;}
    const std::string & getDrive() const {return gsDrive;}
    const std::string & getExtension() const {return gsExtension;}
    const std::string & getFileName() const {return gsFileName;}
    std::string       & getFullPath(std::string& fullPath) const;
    std::string       & getLocation(std::string& location) const;
    void                setDirectory(const char * sNewDirectory);
    void                setDrive(const char * sNewDrive);
    void                setExtension(const char* sNewExt);
    void                setFileName(const char* sNewFile);
    void                setFullPath(const char* sNewFullPath) ;
    void                setLocation(const char *sLocation);
};
//******************************************************************************
#endif

