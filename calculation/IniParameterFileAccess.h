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

    const IniParameterSpecification   & GetSpecifications() const;
    void                                ReadIniParameter(const IniFile& SourceFile, Parameters::ParameterType eParameterType);
    std::vector<std::string>          & ReadIniParameter(const IniFile& SourceFile, Parameters::ParameterType eParameterType, std::vector<std::string>& vParameters, size_t iSuffixIndex) const;

    void                                ReadInputSourceSettings(const IniFile& SourceFile);
    bool                                ReadInputSourceSection(const IniFile& SourceFile, const char* sectionName, const char* keyPrefix, Parameters::InputSource& source);
    void                                ReadAdditionalTreeFileNameSettings(const IniFile& SourceFile);

    void                                WriteInputSource(IniFile& WriteFile, Parameters::ParameterType eParameterType, const Parameters::InputSource * source);
    void                                WriteInputSource(IniFile& WriteFile, IniSection& section, const std::string& basekey, const Parameters::InputSource * source);

    void                                WriteIniParameter(IniFile& WriteFile, Parameters::ParameterType eParameterType, const char* sValue, const char* sComment=0);
    void                                WriteIniParameterAsKey(IniFile& WriteFile, const char* sSectionName, const char* sKey, const char* sValue, const char* sComment=0);

    void                                WriteInputSettings(IniFile& WriteFile);
    void                                WriteAnalysisSettings(IniFile& WriteFile);
    void                                WriteOutputSettings(IniFile& WriteFile);
    void                                WriteAdvancedInputSettings(IniFile& WriteFile);
    void                                WriteAdvancedAnalysisTemporalWindowSettings(IniFile& WriteFile);
    void                                WriteAdvancedAnalysisAdjustmentsSettings(IniFile& WriteFile);
    void                                WriteAdvancedAnalysisInferenceSettings(IniFile& WriteFile);
    void                                WriteAdvancedAnalysisMiscellaneousSettings(IniFile& WriteFile);
    void                                WriteAdvancedOutputSettings(IniFile& WriteFile);
    void                                WriteRunOptionSettings(IniFile& WriteFile);
    void                                WriteSequentialScanSettings(IniFile& WriteFile);
    void                                WritePowerEvaluationsSettings(IniFile& WriteFile);
    void                                WritePowerSimulationsSettings(IniFile& WriteFile);
    void                                WriteSystemSettings(IniFile& WriteFile);

  public:
     IniParameterFileAccess(Parameters& Parameters, BasePrint& PrintDirection);
     virtual ~IniParameterFileAccess();

     virtual const char               * GetParameterLabel(Parameters::ParameterType eParameterType) const;

     virtual bool                       Read(const char* szFilename);
     virtual void                       Write(const char * szFilename);

     void                               writeSections(IniFile& ini, const IniParameterSpecification& specification);
};
//***************************************************************************
#endif
