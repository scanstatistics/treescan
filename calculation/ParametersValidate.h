//******************************************************************************
#ifndef ParametersValidateH
#define ParametersValidateH
//******************************************************************************
#include "Parameters.h"
#include "BasePrint.h"

/** Validates CParameters settings in relation to program and each other, as well
    as available functionality in program. */
class ParametersValidate {
  public: 
      static const char * MSG_INVALID_PARAM;

  private:
    const Parameters & _parameters;


    bool checkFileExists(const std::string& filename, const std::string& filetype, BasePrint& PrintDirection, bool writeCheck=false) const;

    bool ValidateAnalysisParameters(BasePrint & PrintDirection) const;
    bool ValidateAdjustmentsParameters(BasePrint & PrintDirection) const;
    bool ValidateInputParameters(BasePrint & PrintDirection) const;
    bool ValidateInputSource(const Parameters::InputSource * source, const std::string& filename, const std::string& verbosename, BasePrint& PrintDirection) const;
    bool ValidateOutputParameters(BasePrint & PrintDirection) const;
    bool ValidateAdditionalOutputParameters(BasePrint & PrintDirection) const;
    bool ValidatePowerEvaluationParametersParameters(BasePrint & PrintDirection) const;
    bool ValidateRandomizationSeed(BasePrint& PrintDirection) const;
    bool ValidateTemporalWindowParameters(BasePrint & PrintDirection) const;

  public:
    ParametersValidate(const Parameters& parameters) : _parameters(parameters) {}

    bool Validate(BasePrint& printDirection) const;
};
//******************************************************************************
#endif

