//******************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//******************************************************************************
#include "ParametersValidate.h"
#include "RandomNumberGenerator.h"
#include "Randomization.h"
#include "ParametersPrint.h"
#include "PrjException.h"
#include "TimeStamp.h"
#include <boost/random/linear_congruential.hpp>
#include <boost/random/uniform_int.hpp>
#include "FileName.h"

/** Validates that given current state of settings, parameters and their relationships
    with other parameters are correct. Errors are sent to print direction and*/
bool ParametersValidate::Validate(BasePrint& printDirection) const {
  bool bValid=true;
  try {
    if (! ValidateInputParameters(printDirection))
      bValid = false;
    if (! ValidateAnalysisParameters(printDirection))
      bValid = false;
    if (! ValidateOutputParameters(printDirection))
      bValid = false;
    if (! ValidateRandomizationSeed(printDirection))
      bValid = false;
  }
  catch (prg_exception& x) {
    x.addTrace("Validate()","ParametersValidate");
    throw;
  }
  return bValid;
}


/** Validates input/output file parameters. */
bool ParametersValidate::ValidateInputParameters(BasePrint& PrintDirection) const {
    bool bValid=true;
    std::string buffer, buffer2;
    try {
        if (_parameters.getTreeFileName().empty()) {
            bValid = false;
            PrintDirection.Printf("Invalid Parameter Setting:\nNo tree file specified.\n", BasePrint::P_PARAMERROR);
        } else if (!ValidateFileAccess(_parameters.getTreeFileName())) {
            bValid = false;
            PrintDirection.Printf("Invalid Parameter Setting:\n"
                                   "The tree file '%s' could not be opened for reading. "
                                   "Please confirm that the path and/or file name are valid and that you "
                                   "have permissions to read from this directory and file.\n",
                                   BasePrint::P_PARAMERROR, _parameters.getTreeFileName().c_str());
        }
        if (_parameters.getCountFileName().empty()) {
            bValid = false;
            PrintDirection.Printf("Invalid Parameter Setting:\nNo count file specified.\n", BasePrint::P_PARAMERROR);
        } else if (!ValidateFileAccess(_parameters.getCountFileName())) {
            bValid = false;
            PrintDirection.Printf("Invalid Parameter Setting:\n"
                                   "The count file '%s' could not be opened for reading. "
                                   "Please confirm that the path and/or file name are valid and that you "
                                   "have permissions to read from this directory and file.\n",
                                   BasePrint::P_PARAMERROR, _parameters.getCountFileName().c_str());
        }
        if (!_parameters.getCutsFileName().empty() && !ValidateFileAccess(_parameters.getCutsFileName())) {
            bValid = false;
            PrintDirection.Printf("Invalid Parameter Setting:\n"
                                   "The cut file '%s' could not be opened for reading. "
                                   "Please confirm that the path and/or file name are valid and that you "
                                   "have permissions to read from this directory and file.\n",
                                   BasePrint::P_PARAMERROR, _parameters.getCutsFileName().c_str());
        }
        if (_parameters.getModelType() == Parameters::TEMPORALSCAN) {
            const DataTimeRangeSet::rangeset_t& rangeSets = _parameters.getDataTimeRangeSet().getDataTimeRangeSets();
            if (rangeSets.size() == 0) {
                bValid = false;
                PrintDirection.Printf("Invalid Parameter Setting:\nA data time range was not specified.\n", BasePrint::P_PARAMERROR);
            }
            if (rangeSets.size() > 1) {
                bValid = false;
                PrintDirection.Printf("Invalid Parameter Setting:\nA single data time range may be defined, you have %u data time ranges.\n"
                                      "(Multiple ranges are not implemented as of this version.)\n",
                                      BasePrint::P_PARAMERROR, rangeSets.size());
            }
            const DataTimeRange& startRange = _parameters.getTemporalStartRange();
            const DataTimeRange& endRange = _parameters.getTemporalEndRange();
            if (startRange.getStart() < 0) {
                bValid = false;
                PrintDirection.Printf("Invalid Parameter Setting:\nThe start scanning window range '%s' must be defined with a starting index of zero or more.\n",
                                      BasePrint::P_PARAMERROR, startRange.toString(buffer).c_str());
            }
            if (endRange.getStart() < 0) {
                bValid = false;
                PrintDirection.Printf("Invalid Parameter Setting:\nThe end scanning window range '%s' must be defined with a starting index of zero or more.\n",
                                      BasePrint::P_PARAMERROR, endRange.toString(buffer).c_str());
            }
            bool positiveRange=false, startWindowInRange=false, endWindowInRange=false;
            // validate the data time range indexes do not overlap
            // validate the the start and end temporal windows are within a data time range
            for (DataTimeRangeSet::rangeset_t::const_iterator itrOuter=rangeSets.begin(); itrOuter != rangeSets.end();++itrOuter) {
                startWindowInRange |= itrOuter->encloses(startRange);
                endWindowInRange |= itrOuter->encloses(endRange);
                positiveRange |= itrOuter->getEnd() >= 0;
                for (DataTimeRangeSet::rangeset_t::const_iterator itrInner=itrOuter+1; itrInner != rangeSets.end(); ++itrInner) {
                    if (itrOuter->overlaps(*itrInner)) {
                        bValid = false;
                        PrintDirection.Printf("Invalid Parameter Setting:\nThe data time range '%s' overlaps with other range '%s'.\n",
                                               BasePrint::P_PARAMERROR, itrOuter->toString(buffer).c_str(), itrInner->toString(buffer2).c_str());
                    }
                }
            }
            if (!positiveRange) {
                bValid = false;
                PrintDirection.Printf("Invalid Parameter Setting:\nA data time range with positve indices (zero or greater) was not defined.\n"
                                      "At least one range must cross into positive indices.\n",
                                      BasePrint::P_PARAMERROR, startRange.toString(buffer).c_str());
            }
            if (!startWindowInRange) {
                bValid = false;
                PrintDirection.Printf("Invalid Parameter Setting:\nThe start scanning window range '%s' is not within a defined data time range with a starting index of zero or more.\n",
                                      BasePrint::P_PARAMERROR, startRange.toString(buffer).c_str());
            }
            if (!endWindowInRange) {
                bValid = false;
                PrintDirection.Printf("Invalid Parameter Setting:\nThe end scanning window range '%s' is not within a defined data time range with a starting index of zero or more.\n",
                                      BasePrint::P_PARAMERROR, endRange.toString(buffer).c_str());
            }
            if (startRange.getEnd() > endRange.getEnd()) {
                bValid = false;
                PrintDirection.Printf("Invalid Parameter Setting:\n"
                                      "The upper index of the start scanning window range '%s' cannot be greater than upper index of the end scanning window range '%s'.\n",
                                      BasePrint::P_PARAMERROR, startRange.toString(buffer).c_str(), endRange.toString(buffer2).c_str());
            }
            if (endRange.getStart() < startRange.getStart()) {
                bValid = false;
                PrintDirection.Printf("Invalid Parameter Setting:\n"
                                      "The lower index of the end scanning window range '%s' cannot be greater than lower index of the start scanning window range '%s'.\n",
                                      BasePrint::P_PARAMERROR, endRange.toString(buffer).c_str(), startRange.toString(buffer2).c_str());
            }
        }
    } catch (prg_exception& x) {
        x.addTrace("ValidateFileParameters()","ParametersValidate");
        throw;
    }
    return bValid;
}

/** Validates input/output file parameters. */
bool ParametersValidate::ValidateAnalysisParameters(BasePrint& PrintDirection) const {
  bool bValid=true;
  try {
      if (_parameters.getModelType() == Parameters::BERNOULLI && 
          (_parameters.getProbabilityRatio().first == 0 || _parameters.getProbabilityRatio().second == 0 || 
          _parameters.getProbabilityRatio().first >= _parameters.getProbabilityRatio().second)) {
          bValid = false;
          PrintDirection.Printf("Invalid Parameter Setting:\nEvent probabilty must be between zero and one.\n", BasePrint::P_PARAMERROR);
      }
  }
  catch (prg_exception& x) {
    x.addTrace("ValidateAnalysisParameters()","ParametersValidate");
    throw;
  }
  return bValid;
}

/** Validates output options. */
bool ParametersValidate::ValidateOutputParameters(BasePrint & PrintDirection) const {
  bool bValid=true;
  try {
      //validate output file
      if (_parameters.getOutputFileName().empty()) {
        bValid = false;
        PrintDirection.Printf("Invalid Parameter Setting:\nNo results file specified.\n", BasePrint::P_PARAMERROR);
      } else if (!ValidateFileAccess(_parameters.getOutputFileName(), true)) {
        bValid = false;
        PrintDirection.Printf("Invalid Parameter Setting:\n"
                              "Results file '%s' could not be opened for writing. "
                              "Please confirm that the path and/or file name are valid and that you "
                              "have permissions to write to this directory and file.\n",
                              BasePrint::P_PARAMERROR, _parameters.getOutputFileName().c_str());
      }
  } catch (prg_exception& x) {
    x.addTrace("ValidateOutputOptionParameters()","ParametersValidate");
    throw;
  }
  return bValid;
}

/** Validates parameters parameters related to randomization seed.
    Prints errors to print direction and returns whether values are vaild. */
bool ParametersValidate::ValidateRandomizationSeed(BasePrint& PrintDirection) const {
  if (!_parameters.getNumReplicationsRequested()) return true;

  if (_parameters.isRandomlyGeneratingSeed()) {
      double dMaxSeed = (double)RandomNumberGenerator::glM - (double)_parameters.getNumReplicationsRequested();
      boost::minstd_rand generator(static_cast<int>(time(0)));
      const_cast<Parameters&>(_parameters).setRandomizationSeed(boost::uniform_int<>(1,static_cast<int>(dMaxSeed))(generator));
      return true;
  }
  //validate hidden parameter which specifies randomization seed
  if (!(0 < _parameters.getRandomizationSeed() && _parameters.getRandomizationSeed() < RandomNumberGenerator::glM)) {
      PrintDirection.Printf("Invalid Parameter Setting:\nRandomization seed out of range [1 - %ld].\n",
                            BasePrint::P_PARAMERROR, RandomNumberGenerator::glM - 1);
      return false;
  }
  //validate that generated seeds during randomization will not exceed defined range
  double dMaxRandomizationSeed = (double)_parameters.getRandomizationSeed() + (double)_parameters.getNumReplicationsRequested();
  if (dMaxRandomizationSeed >= static_cast<double>(RandomNumberGenerator::glM)) {
      //case #1 - glRandomizationSeed == RandomNumberGenerator::glDefaultSeed
      //    In this case, it is assumed that user accepted default - at this time
      //    changing the initial seed through parameter settings is a 'hidden' parameter;
      //    so no direction should be given regarding the alteration of seed value.
      if (_parameters.getRandomizationSeed() == RandomNumberGenerator::glDefaultSeed) {
        double dMaxReplications = (double)RandomNumberGenerator::glM - (double)_parameters.getRandomizationSeed();
        dMaxReplications = (floor((dMaxReplications)/1000) - 1)  * 1000 + 999;
        PrintDirection.Printf("Invalid Parameter Setting:\nRequested number of replications causes randomization seed to exceed defined limit. "
                              "Maximum number of replications is %.0lf.\n", BasePrint::P_PARAMERROR, dMaxReplications);
        return false;
      }
      //case #2 - user specified alternate randomization seed
      //    This alternate seed or the number of requested replications could be the problem.
      //    User has two options, either pick a lesser seed or request less replications.
      //calculate maximum seed for requested number of replications
      double dMaxSeed = (double)RandomNumberGenerator::glM - (double)_parameters.getNumReplicationsRequested();
      //calculate maximum number of replications for requested seed
      double dMaxReplications = (double)RandomNumberGenerator::glM - (double)_parameters.getRandomizationSeed();
      dMaxReplications = (floor((dMaxReplications)/1000) - 1)  * 1000 + 999;
      //check whether specified combination of seed and requested number of replications fights each other
      if (dMaxReplications < 9 && (dMaxSeed <= 0 || dMaxSeed > RandomNumberGenerator::glM)) {
        PrintDirection.Printf("Invalid Parameter Setting:\nRandomization seed will exceed defined limit. "
                              "The specified initial seed, in conjunction with the number of replications, "
                              "contend for numerical range in defined limits. Please modify the specified "
                              "initial seed and/or lessen the number of replications and try again.\n", BasePrint::P_PARAMERROR);
      }
      //check that randomization seed is not so large that we can't run any replications
      else if (dMaxReplications < 9) {
        PrintDirection.Printf("Invalid Parameter Setting:\nRandomization seed will exceed defined limit. "
                              "The intial seed specified prevents any replications from being performed. "
                              "With %ld replications, the initial seed can be [0 - %.0lf].\n",
                              BasePrint::P_PARAMERROR, _parameters.getNumReplicationsRequested(), dMaxSeed);
      }
      //check that number of replications isn't too large
      else if (dMaxSeed <= 0 || dMaxSeed > RandomNumberGenerator::glM) {
        PrintDirection.Printf("Invalid Parameter Setting:\nRequested number of replications causes randomization seed to exceed defined limit. "
                              "With initial seed of %i, maximum number of replications is %.0lf.\n",
                              BasePrint::P_PARAMERROR, _parameters.getRandomizationSeed(), dMaxReplications);
      }
      else {
        PrintDirection.Printf("Invalid Parameter Setting:\nRandomization seed will exceed defined limit. "
                              "Either limit the number of replications to %.0lf or "
                              "define the initial seed to a value less than %.0lf.\n",
                              BasePrint::P_PARAMERROR, dMaxReplications, dMaxSeed);
      }
      return false;
  }

  return true;
}

