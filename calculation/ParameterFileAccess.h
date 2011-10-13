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

    bool read(const std::string& sFilename);
    void write(const std::string& sFilename);
};

/** Execption class of invalid parameters */
class parameter_error : public resolvable_error {
  public:
   parameter_error(const char * format, ...);
};
//***************************************************************************
#endif
