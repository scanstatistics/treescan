//***************************************************************************
#ifndef __IniParameterFileAccess_H
#define __IniParameterFileAccess_H
//***************************************************************************
#include "ParameterFileAccess.h"
#include "IniParameterSpecification.h"
#include "Ini.h"

/** Implements class that reading/writing parameter settings of an INI file,
    where each parameter is maintained in a key/value pair of a section. */
class IniParameterFileAccess : public AbtractParameterFileAccess  {
  private:
    const IniParameterSpecification   * gpSpecifications;

    virtual const char                * GetParameterLabel(Parameters::ParameterType eParameterType) const;
    const IniParameterSpecification   & GetSpecifications() const;
    void                                ReadIniParameter(const IniFile& SourceFile, Parameters::ParameterType eParameterType);

    void                                WriteIniParameter(IniFile& WriteFile, Parameters::ParameterType eParameterType, const char* sValue, const char* sComment=0);
    void                                WriteIniParameterAsKey(IniFile& WriteFile, const char* sSectionName, const char* sKey, const char* sValue, const char* sComment=0);

    void                                WriteInputSettings(IniFile& WriteFile);
    void                                WriteAnalysisSettings(IniFile& WriteFile);
    void                                WriteOutputSettings(IniFile& WriteFile);
    void                                WriteAdvancedInputSettings(IniFile& WriteFile);
    void                                WriteAdvancedAnalysisSettings(IniFile& WriteFile);
    void                                WriteAdvancedOutputSettings(IniFile& WriteFile);
    void                                WriteRunOptionSettings(IniFile& WriteFile);
    void                                WritePowerEvaluationsSettings(IniFile& WriteFile);
    void                                WriteSystemSettings(IniFile& WriteFile);

  public:
     IniParameterFileAccess(Parameters& Parameters, BasePrint& PrintDirection);
     virtual ~IniParameterFileAccess();

     virtual bool                       Read(const char* szFilename);
     virtual void                       Write(const char * szFilename);
};
//***************************************************************************
#endif
