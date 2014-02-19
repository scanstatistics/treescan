//***************************************************************************
#ifndef __IniParameterSpecification_H
#define __IniParameterSpecification_H
//***************************************************************************
#include "Parameters.h"
#include "Ini.h"

/** Defines interface for retrieving ini section and key name for parameters. */
class IniParameterSpecification {
  public:
    typedef std::vector<std::pair<const char*, const char*> > ParameterInfo_t;

  public:
    static const char * Input;
    static const char * AdvancedInput;
    static const char * Analysis;
    static const char * AdvancedAnalysis;
    static const char * Output;
    static const char * PowerSimulations;

    static const char * RunOptions;
    static const char * System;

  protected:
    ParameterInfo_t gvParameterInfo;

    void Build_1_1_x_ParameterList();

   public:
     IniParameterSpecification();
     IniParameterSpecification(const IniFile& SourceFile, Parameters& Parameters);
     virtual ~IniParameterSpecification();

    bool GetParameterIniInfo(Parameters::ParameterType eParameterType, const char ** sSectionName, const char ** sKey) const;
};
//***************************************************************************
#endif
