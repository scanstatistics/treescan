//***************************************************************************
#ifndef __ParameterFileAccess_H
#define __ParameterFileAccess_H
//***************************************************************************
#include "Parameters.h"
#include "PrjException.h" 

/** Coordinates the reading/writing of parameters to file. */
class ParameterAccessCoordinator {
  protected:
    Parameters & _parameters;

  public:
    ParameterAccessCoordinator(Parameters& parameters) : _parameters(parameters) {}

    bool read(const std::string& sFilename, BasePrint& print);
    void write(const std::string& sFilename, BasePrint& print);
};

/** Abstract base class for reading/writing parameters from file. */
class AbtractParameterFileAccess {
  protected:
    Parameters & _parameters;

    BasePrint                 & gPrintDirection;
    std::vector<int>            _missing_defaulted;      // collection of missing ParameterTypes on read from file
    bool                        _read_error;             // marker of errors encountered while reading parameters from file
    bool                        _write_boolean_as_digit;

  public:
     AbtractParameterFileAccess(Parameters& Parameters, BasePrint& PrintDirection, bool bWriteBooleanAsDigit=false);
     virtual ~AbtractParameterFileAccess();

     virtual const char        * GetParameterLabel(Parameters::ParameterType e) const = 0;

     bool                        ReadBoolean(const std::string& sValue, Parameters::ParameterType e) const;
     double                      ReadDouble(const std::string& sValue, Parameters::ParameterType e) const;
     int                         ReadEnumeration(int iValue, Parameters::ParameterType e, int iLow, int iHigh) const;
     int                         ReadInt(const std::string& sValue, Parameters::ParameterType e) const;
     int                         ReadUnsignedInt(const std::string& sValue, Parameters::ParameterType e) const;
     Parameters::CreationVersion ReadVersion(const std::string& sValue) const;
     Parameters::ratio_t         ReadRatio(const std::string& sValue, Parameters::ParameterType e) const;

     static std::string        & AsString(std::string& ref, int i) {printString(ref, "%d", i); return ref;}
     static std::string        & AsString(std::string& ref, unsigned int i) {printString(ref, "%u", i); return ref;}
     static std::string        & AsString(std::string& ref, float f) {printString(ref, "%g", f); return ref;}
     static std::string        & AsString(std::string& ref, double d) {printString(ref, "%g", d); return ref;}
     std::string               & AsString(std::string& ref, bool b) const {printString(ref, "%s", (b ? (_write_boolean_as_digit ? "1" : "y") : (_write_boolean_as_digit ? "0" : "n"))); return ref;}
     static std::string        & AsString(std::string& ref, const Parameters::CreationVersion& v) {printString(ref, "%d.%d.%d", v.iMajor, v.iMinor, v.iRelease); return ref;}
     static std::string        & AsString(std::string& ref, const Parameters::ratio_t& r);
     const char                * GetParameterComment(Parameters::ParameterType e) const;
     std::string               & GetParameterString(Parameters::ParameterType e, std::string& s) const;

     virtual bool               Read(const char* szFilename) = 0;
     void                       SetParameter(Parameters::ParameterType e, const std::string& sParameter, BasePrint& PrintDirection);
     Parameters::InputSource &  setInputSource(Parameters::InputSource & source,
                                               const std::string& typeStr,
                                               const std::string& mapStr,
                                               const std::string& delimiterStr,
                                               const std::string& groupStr,
                                               const std::string& skipStr,
                                               const std::string& headerStr,
                                               BasePrint& PrintDirection);
     virtual void               Write(const char * szFilename) = 0;
};

/** Execption class of invalid parameters */
class parameter_error : public resolvable_error {
  public:
   parameter_error(const char * format, ...);
};
//***************************************************************************
#endif
