//******************************************************************************
#ifndef ParametersValidateH
#define ParametersValidateH
//******************************************************************************
#include "Parameters.h"
#include "BasePrint.h"

/** Validates CParameters settings in relation to program and each other, as well
    as available functionality in program. */
class ParametersValidate {
  private:
    const Parameters & _parameters;

    bool ValidateInputParameters(BasePrint & PrintDirection) const;
    bool ValidateAnalysisParameters(BasePrint & PrintDirection) const;
    bool ValidateOutputParameters(BasePrint & PrintDirection) const;
    bool ValidateTemporalWindowParameters(BasePrint & PrintDirection) const;
    bool ValidateRandomizationSeed(BasePrint& PrintDirection) const;

  public:
    ParametersValidate(const Parameters& parameters) : _parameters(parameters) {}

    bool Validate(BasePrint& printDirection) const;
};
//******************************************************************************
#endif

