//***************************************************************************
#ifndef __IniParameterSpecification_H
#define __IniParameterSpecification_H
//***************************************************************************
#include "Parameters.h"
#include "Ini.h"

/** Defines interface for retrieving ini section and key name for parameters. */
class IniParameterSpecification {
  public:
    class SectionInfo {
        public:
            const char *  _label;
            unsigned int  _ordinal;

        public:
            SectionInfo():_label(0), _ordinal(0) {}
            SectionInfo(const char * label, unsigned int ordinal):_label(label), _ordinal(ordinal) {}
    };

    class ParamInfo {
        public:
            Parameters::ParameterType _type;
            const char *  _label;
            unsigned int  _ordinal;
            const SectionInfo * _section;

        public:
            ParamInfo() : _label(0), _section(0) {}
            ParamInfo(const ParamInfo& other):
              _type(other._type), _label(other._label), _ordinal(other._ordinal), _section(other._section) {}
            ParamInfo(Parameters::ParameterType type, const char * label, unsigned int ordinal, const SectionInfo& section):
                _type(type), _label(label), _ordinal(ordinal), _section(&section) {}

            bool operator<(const ParamInfo& pinfo) const {
                if (_section->_ordinal == pinfo._section->_ordinal) {
                    return this->_ordinal < pinfo._ordinal;
                } else {
                    return _section->_ordinal < pinfo._section->_ordinal;
                }
            }
    };

  public:
    typedef std::map<Parameters::ParameterType, ParamInfo> ParameterInfoMap_t;
    typedef std::map<Parameters::ParameterType, ParamInfo > MultipleParameterInfoMap_t;
    typedef std::vector<ParamInfo> ParameterInfoCollection_t;

  public:
    SectionInfo                 _analysis_section;
    SectionInfo                 _input_section;
    SectionInfo                 _output_section;
    SectionInfo                 _advanced_input_section;
    SectionInfo                 _temporal_window_section;
    SectionInfo                 _adjustments_section;
    SectionInfo                 _inference_section;
    SectionInfo                 _sequential_scan_section;
    SectionInfo                 _miscellaneous_analysis_section;
    SectionInfo                 _power_evaluations_section;
    SectionInfo                 _additional_output_section;
    SectionInfo                 _power_simulations_section;
    SectionInfo                 _run_options_section;
    SectionInfo                 _system_section;

    static const char * Input;
    static const char * AdvancedInput;
    static const char * Analysis;
    static const char * TemporalWindow;
    static const char * Adjustments;
    static const char * Inference;
    static const char * MiscellaneousAnalysis;
    static const char * Output;
    static const char * AdditionalOutput;
    static const char * PowerEvaluations;
    static const char * PowerSimulations;
    static const char * SequentialScan;

    static const char * RunOptions;
    static const char * System;

    static const char * SourceType;
    static const char * SourceDelimiter;
    static const char * SourceGrouper;
    static const char * SourceSkip;
    static const char * SourceFirstRowHeader;
    static const char * SourceFieldMap;
    static const char * SourceFieldMapOneCount;

    void setup(Parameters::CreationVersion version);

  protected:
    ParameterInfoMap_t _parameter_info;
    MultipleParameterInfoMap_t  _multiple_parameter_info;

    void Build_1_1_x_ParameterList();
    void Build_1_2_x_ParameterList();
    void Build_1_3_x_ParameterList();
    void Build_1_4_x_ParameterList();
    void Build_2_0_x_ParameterList();
    void Build_2_1_x_ParameterList();
    void Build_2_2_x_ParameterList();
    void Build_2_3_x_ParameterList();

   public:
     IniParameterSpecification();
     IniParameterSpecification(const IniFile& SourceFile, Parameters& Parameters);
     IniParameterSpecification(Parameters::CreationVersion version, Parameters& Parameters);
     IniParameterSpecification(const IniFile& SourceFile, Parameters::CreationVersion version, Parameters& Parameters);
     virtual ~IniParameterSpecification() {}

    static Parameters::CreationVersion getIniVersion(const IniFile& SourceFile);
    bool GetParameterIniInfo(Parameters::ParameterType eParameterType, const char ** sSectionName, const char ** sKey) const;
    bool GetMultipleParameterIniInfo(Parameters::ParameterType eParameterType, const char ** sSectionName, const char ** sKey) const;
    ParameterInfoCollection_t & getParameterInfoCollection(ParameterInfoCollection_t& collection) const;
};
//***************************************************************************
#endif
