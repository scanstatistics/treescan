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

const char * ParametersValidate::MSG_INVALID_PARAM = "Invalid Parameter Setting";

/**/
bool ParametersValidate::checkFileExists(const std::string& filename, const std::string& filetype, BasePrint& PrintDirection, bool writeCheck) const {
    std::string buffer = filename;

    trimString(buffer);
    if (buffer.empty()) {
        PrintDirection.Printf("%s:\nThe %s file could not be opened. No filename was specified.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, filetype.c_str());
        return false;
    } else if (!validateFileAccess(buffer, writeCheck)) {
        PrintDirection.Printf("%s:\nThe %s file '%s' could not be opened for %s. "
                              "Please confirm that the path and/or file name are valid and that you "
                              "have permissions to %s from this directory and file.\n",
                              BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, filetype.c_str(), buffer.c_str(), 
                              (writeCheck ? "writing": "reading"), (writeCheck ? "write": "read"));
        return false;
    }
    return true;
}

/** Validates that given current state of settings, parameters and their relationships
    with other parameters are correct. Errors are sent to print direction and returns whether values are valid. */
bool ParametersValidate::Validate(BasePrint& printDirection) const {
    bool bValid=true;

    if (Parameters::isTemporalScanType(_parameters.getScanType()) && _parameters.getIsProspectiveAnalysis()) {
        const_cast<Parameters&>(_parameters).setRestrictTemporalWindows(false);
    }

    bValid &= ValidateInputParameters(printDirection);
    bValid &= ValidateAnalysisParameters(printDirection);
    bValid &= ValidateOutputParameters(printDirection);
    bValid &= ValidateAdditionalOutputParameters(printDirection);
    bValid &= ValidateTemporalWindowParameters(printDirection);
    bValid &= ValidateAdjustmentsParameters(printDirection);
    bValid &= ValidateInferenceParameters(printDirection);
    bValid &= ValidateSequentialScanParameters(printDirection);
    bValid &= ValidatePowerEvaluationParametersParameters(printDirection);
    bValid &= ValidateRandomizationSeed(printDirection);
    const_cast<Parameters&>(_parameters).setProspectiveFrequency(std::max(_parameters.getProspectiveFrequency(), 1u));
    return bValid;
}

/** Validates 'Adjustments' parameters. */
bool ParametersValidate::ValidateAdjustmentsParameters(BasePrint & PrintDirection) const {
    bool bValid=true;
    std::string buffer, buffer2;

    if (_parameters.getPerformDayOfWeekAdjustment()) {
        if (_parameters.getScanType() == Parameters::TREEONLY) {
            bValid = false;
            PrintDirection.Printf("Invalid Parameter Setting:\nThe day of week adjustment is not implemented for the 'Tree Only' scan type.\n", BasePrint::P_PARAMERROR);
        }
        if (_parameters.getModelType() == Parameters::BERNOULLI_TIME) {
            bValid = false;
            PrintDirection.Printf("Invalid Parameter Setting:\nThe day of week adjustment is not implemented for the 'Bernoulli' model type.\n", BasePrint::P_PARAMERROR);
        }
        if (_parameters.getModelType() == Parameters::BERNOULLI_TIME) {
            bValid = false;
            PrintDirection.Printf("Invalid Parameter Setting:\nThe day of week adjustment is not implemented for the 'Bernoulli' model type.\n", BasePrint::P_PARAMERROR);
        }
        if (!(_parameters.getDatePrecisionType() == DataTimeRange::GENERIC || _parameters.getDatePrecisionType() == DataTimeRange::DAY)) {
            bValid = false;
            PrintDirection.Printf("Invalid Parameter Setting:\nThe day of week adjustment is only implemented for the date precision of day or generic.\n", BasePrint::P_PARAMERROR);
        }
    }
    if (_parameters.isApplyingExclusionTimeRanges() && _parameters.getDataTimeRangeSet().getDataTimeRangeSets().size() > 0) {
        if (!(_parameters.getScanType() == Parameters::TREETIME && _parameters.getConditionalType() == Parameters::NODEANDTIME)) {
            bValid = false;
            PrintDirection.Printf("Invalid Parameter Setting:\nThe option to exclude specific time ranges is only implemented for 'Tree-Time' scans conditioned on both node and time.\n", BasePrint::P_PARAMERROR);
        }
        if (_parameters.getExclusionTimeRangeStr().empty()) {
            bValid = false;
            PrintDirection.Printf("Invalid Parameter Setting:\nThe option to exclude specific time ranges was selected but no exlusions are defined.\n", BasePrint::P_PARAMERROR);
        }
        try { // Attempt to parse exclusions string into data time ranges.
            const_cast<Parameters&>(_parameters).setExclusionTimeRangeSet(
                DataTimeRangeSet(_parameters.getExclusionTimeRangeStr(), _parameters.getDatePrecisionType(), _parameters.getDataTimeRangeSet().getDataTimeRangeSets().front().getDateStart())
            );
        } catch (std::exception& x) {
            PrintDirection.Printf(
                "Invalid Parameter Setting:\nUnable to parse exclusion time ranges value '%s' into time ranges.\n", 
                BasePrint::P_PARAMERROR, _parameters.getExclusionTimeRangeStr().c_str()
            );
            return false;
        }
        const DataTimeRange & data_time_range = _parameters.getDataTimeRangeSet().getDataTimeRangeSets().front();
        const DataTimeRangeSet::rangeset_t& exclusionSets = _parameters.getExclusionTimeRangeSet().getDataTimeRangeSets();
        if (exclusionSets.size() == 0) {
            bValid = false;
            PrintDirection.Printf("Invalid Parameter Setting:\nThe option to exclude specific time ranges is selected but no exlusions are defined.\n", BasePrint::P_PARAMERROR);
        }
        // Validate each exclusion range.
        for (DataTimeRangeSet::rangeset_t::const_iterator itrOuter = exclusionSets.begin(); itrOuter != exclusionSets.end(); ++itrOuter) {
            if (itrOuter->getStart() > itrOuter->getEnd()) {
                bValid = false;
                PrintDirection.Printf("The exclusion time range start '%d' must be before or equal the data time range end '%d'.\n",
                                      BasePrint::P_PARAMERROR, itrOuter->getStart(), itrOuter->getEnd());
            }
            // Validate that the exclusion time ranges are within the user defined data time range.
            if (!data_time_range.encloses(*itrOuter)) {
                bValid = false;
                PrintDirection.Printf(
                    "Invalid Parameter Setting:\nThe exclusion time range '%s' is not with the data time range '%s'.\n",
                    BasePrint::P_PARAMERROR, 
                    itrOuter->toString(buffer, _parameters.getDatePrecisionType()).c_str(),
                    data_time_range.toString(buffer2, _parameters.getDatePrecisionType()).c_str()
                );
            }
            // Validate that the current exclusion range does not overlap with other exclusion ranges.
            for (DataTimeRangeSet::rangeset_t::const_iterator itrInner=itrOuter+1; itrInner != exclusionSets.end(); ++itrInner) {
                if (itrOuter->overlaps(*itrInner)) {
                    bValid = false;
                    PrintDirection.Printf(
                        "Invalid Parameter Setting:\nThe data time range '%s' overlaps with other range '%s'.\n",
                        BasePrint::P_PARAMERROR, 
                        itrOuter->toString(buffer, _parameters.getDatePrecisionType()).c_str(),
                        itrInner->toString(buffer2, _parameters.getDatePrecisionType()).c_str()
                    );
                }
            }
        }
    }
    return bValid;
}

bool ParametersValidate::ValidateInputSource(const Parameters::InputSource * source, const std::string& filename, const std::string& verbosename, BasePrint& PrintDirection) const {
    FileName file(filename.c_str());

    std::string extension(file.getExtension());
    lowerString(extension);
    // First exclude file types that are not readable - namely, Excel.
    if (extension == ".xls" || extension == ".xlsx") {
        PrintDirection.Printf("%s:\nThe Excel file '%s' cannot be read as an input file.\n.TreeScan cannot read directly from Excel files.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, filename.c_str());
        return false;
    }

    if (source) {
        // Verify that the input source settings's source data file type matches extension.
        bool correct_filetype=true;
        switch (source->getSourceType()) {
            case CSV : {
                FieldMapContainer_t::const_iterator itrMap=source->getFieldsMap().begin();
                for (;itrMap != source->getFieldsMap().end(); ++itrMap) {
                     if (itrMap->type() == typeid(long) && boost::any_cast<long>(*itrMap) < 0) {
                        PrintDirection.Printf("%s:\nThe field mapping column indexes cannot be unless than zero, got value %ld.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, boost::any_cast<long>(*itrMap));
                        return false;
                     }
                }
                correct_filetype = !(extension == ".xls"); break;
            }
            case EXCEL : correct_filetype = extension == ".xls" || extension == ".xlsx"; break;
            default : throw prg_error("Unknown  source type: %d.", "ValidateInputSource()", source->getSourceType());
        }
        if (!correct_filetype) {
            PrintDirection.Printf("%s:\nThe file '%s' cannot be read as an input source. The specified source type does not match the file type.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, filename.c_str());
            return false;
        }
    } return true;
}

/** Validates input/output file parameters. */
bool ParametersValidate::ValidateInferenceParameters(BasePrint & PrintDirection) const {
    bool bValid = true;
    if (_parameters.getScanRateType() != Parameters::LOWRATE && _parameters.getMinimumHighRateNodeCases() < 2) {
        bValid = false;
        PrintDirection.Printf("Invalid Parameter Setting:\nThe minimum number of node cases cannot be less than 2 for high rate scans.\n", BasePrint::P_PARAMERROR);
    }
    if (!_parameters.isSequentialScanTreeOnly()) { // tree sequential scan does not report p-values
        if (_parameters.getNumReplicationsRequested() > 0 && _parameters.getPValueReportingType() == Parameters::TERMINATION_PVALUE &&
            (_parameters.getEarlyTermThreshold() < 1 || _parameters.getEarlyTermThreshold() > _parameters.getNumReplicationsRequested())) {
            bValid = false;
            PrintDirection.Printf("%s:\nThe threshold for early termination of simulations must be from 1 to number of replications.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
        }
    }
    if (_parameters.getScanType() == Parameters::TIMEONLY)
        const_cast<Parameters&>(_parameters).setRestrictEvaluatedTreeNodes(false);

    if (_parameters.getRestrictEvaluatedTreeNodes()) {
        if (_parameters.getNotEvaluatedNodesFileName().empty()) {
            bValid = false;
            PrintDirection.Printf("Invalid Parameter Setting:\nNo not evaluated nodes file specified.\n", BasePrint::P_PARAMERROR);
        } else if (!validateFileAccess(_parameters.getNotEvaluatedNodesFileName())) {
            bValid = false;
            PrintDirection.Printf("Invalid Parameter Setting:\n"
                "The not evaluated nodes file '%s' could not be opened for reading. "
                "Please confirm that the path and/or file name are valid and that you "
                "have permissions to read from this directory and file.\n",
                BasePrint::P_PARAMERROR, _parameters.getNotEvaluatedNodesFileName().c_str());
        }
    }
    return bValid;
}

/** Validates input/output file parameters. */
bool ParametersValidate::ValidateInputParameters(BasePrint& PrintDirection) const {
    bool bValid=true;
    std::string buffer, buffer2;
    try {
        if (_parameters.getScanType() != Parameters::TIMEONLY) {
            if (_parameters.getTreeFileNames().size() == 0) {
                bValid = false;
                PrintDirection.Printf("Invalid Parameter Setting:\nNo tree file specified.\n", BasePrint::P_PARAMERROR);
            }
            for (Parameters::FileNameContainer_t::const_iterator itr=_parameters.getTreeFileNames().begin(); itr != _parameters.getTreeFileNames().end(); ++itr) {
                bool exists = checkFileExists(*itr, "tree", PrintDirection);
                bValid &= exists;
                if (exists) {
                    bValid &= ValidateInputSource(
                        _parameters.getInputSource(Parameters::TREE_FILE, static_cast<unsigned int>(std::distance(_parameters.getTreeFileNames().begin(), itr)) + 1),
                        *itr, "tree", PrintDirection
                    );
                }
            }
        }
        if (_parameters.getCountFileName().empty()) {
            bValid = false;
            PrintDirection.Printf("Invalid Parameter Setting:\nNo count file specified.\n", BasePrint::P_PARAMERROR);
        } else if (!validateFileAccess(_parameters.getCountFileName())) {
            bValid = false;
            PrintDirection.Printf("Invalid Parameter Setting:\n"
                                   "The count file '%s' could not be opened for reading. "
                                   "Please confirm that the path and/or file name are valid and that you "
                                   "have permissions to read from this directory and file.\n",
                                   BasePrint::P_PARAMERROR, _parameters.getCountFileName().c_str());
        }

        if (_parameters.getModelType() == Parameters::BERNOULLI_TREE || _parameters.getModelType() == Parameters::BERNOULLI_TIME) {
            if (_parameters.getControlFileName().empty()) {
                if (_parameters.getModelType() == Parameters::BERNOULLI_TIME) {
                    bValid = false;
                    PrintDirection.Printf("Invalid Parameter Setting:\nNo control file specified.\n", BasePrint::P_PARAMERROR);
                } else if (!_parameters.isSequentialScanBernoulli())
                    /* The Bernoulli tree model in versions prior to 2.0 did not have control file, so allow file to be optional in order to facilitate backwards compatability.*/
                    PrintDirection.Printf("Notice:\nNo control file specified for tree-only Bernoulli scan, control data assumed to be in count file.\n", BasePrint::P_NOTICE);
            } else if (!validateFileAccess(_parameters.getControlFileName())) {
                bValid = false;
                PrintDirection.Printf("Invalid Parameter Setting:\n"
                    "The control file '%s' could not be opened for reading. "
                    "Please confirm that the path and/or file name are valid and that you "
                    "have permissions to read from this directory and file.\n",
                    BasePrint::P_PARAMERROR, _parameters.getControlFileName().c_str());
            }
        }

        if (_parameters.getScanType() != Parameters::TIMEONLY) {
            if (!_parameters.getCutsFileName().empty() && !validateFileAccess(_parameters.getCutsFileName())) {
                bValid = false;
                PrintDirection.Printf("Invalid Parameter Setting:\n"
                                       "The cut file '%s' could not be opened for reading. "
                                       "Please confirm that the path and/or file name are valid and that you "
                                       "have permissions to read from this directory and file.\n",
                                       BasePrint::P_PARAMERROR, _parameters.getCutsFileName().c_str());
            }
        }
        if (_parameters.isTemporalScanType(_parameters.getScanType())) {
            if (_parameters.getDataTimeRangeStr().empty()) {
                PrintDirection.Printf(
                    "Invalid Parameter Setting:\nNo data time range was specified for a temporal analysis.\n", BasePrint::P_PARAMERROR
                );
                return false;
            }
            try { // Attempt to parse data time range string into data time ranges.
                const_cast<Parameters&>(_parameters).setDataTimeRangeSet(
                    DataTimeRangeSet(_parameters.getDataTimeRangeStr(), _parameters.getDatePrecisionType(), boost::optional<boost::gregorian::date>())
                );
            } catch (std::exception& x) {
                PrintDirection.Printf(
                    "Invalid Parameter Setting:\nUnable to parse data time range value '%s' into time ranges.\n",
                    BasePrint::P_PARAMERROR, _parameters.getDataTimeRangeStr().c_str()
                );
                return false;
            }
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

            /*const DataTimeRange& startRange = _parameters.getTemporalStartRange();
            const DataTimeRange& endRange = _parameters.getTemporalEndRange();
            bool startWindowInRange=false, endWindowInRange=false;
            // validate the data time range indexes do not overlap
            // validate the the start and end temporal windows are within a data time range
            for (DataTimeRangeSet::rangeset_t::const_iterator itrOuter=rangeSets.begin(); itrOuter != rangeSets.end();++itrOuter) {
                if (itrOuter->getStart() >= itrOuter->getEnd()) {
                    bValid = false;
                    PrintDirection.Printf("The data time range start '%d' must be before the data time range end '%d'.\n",
                                          BasePrint::P_PARAMERROR, itrOuter->getStart(), itrOuter->getEnd());
                }
                startWindowInRange |= itrOuter->encloses(startRange);
                endWindowInRange |= itrOuter->encloses(endRange);
                for (DataTimeRangeSet::rangeset_t::const_iterator itrInner=itrOuter+1; itrInner != rangeSets.end(); ++itrInner) {
                    if (itrOuter->overlaps(*itrInner)) {
                        bValid = false;
                        PrintDirection.Printf(
                            "Invalid Parameter Setting:\nThe data time range '%s' overlaps with other range '%s'.\n", BasePrint::P_PARAMERROR, 
                            itrOuter->toString(buffer, _parameters.getDatePrecisionType()).c_str(),
                            itrInner->toString(buffer2, _parameters.getDatePrecisionType()).c_str()
                        );
                    }
                }
            }*/

            if (bValid && _parameters.getRestrictTemporalWindows()) {
                if (_parameters.getTemporalStartRangeStr().empty()) {
                    PrintDirection.Printf(
                        "Invalid Parameter Setting:\nNo start range was specified for a temporal analysis.\n", BasePrint::P_PARAMERROR
                    );
                    return false;
                }
                try { // Attempt to parse start range string into data time range.
                    const_cast<Parameters&>(_parameters).setTemporalStartRange(
                        DataTimeRange(_parameters.getTemporalStartRangeStr(), _parameters.getDatePrecisionType(), _parameters.getDataTimeRangeSet().getDataTimeRangeSets().front().getDateStart())
                    );
                } catch (std::exception& x) {
                    PrintDirection.Printf(
                        "Invalid Parameter Setting:\nUnable to parse start range value '%s' into time range.\n",
                        BasePrint::P_PARAMERROR, _parameters.getTemporalStartRangeStr().c_str()
                    );
                    return false;
                }
                if (_parameters.getTemporalEndRangeStr().empty()) {
                    PrintDirection.Printf(
                        "Invalid Parameter Setting:\nNo end range was specified for a temporal analysis.\n", BasePrint::P_PARAMERROR
                    );
                    return false;
                }
                try { // Attempt to parse end range string into data time range.
                    const_cast<Parameters&>(_parameters).setTemporalEndRange(
                        DataTimeRange(_parameters.getTemporalEndRangeStr(), _parameters.getDatePrecisionType(), _parameters.getDataTimeRangeSet().getDataTimeRangeSets().front().getDateStart())
                    );
                } catch (std::exception& x) {
                    PrintDirection.Printf(
                        "Invalid Parameter Setting:\nUnable to parse end range value '%s' into time range.\n",
                        BasePrint::P_PARAMERROR, _parameters.getTemporalEndRangeStr().c_str()
                    );
                    return false;
                }
                // Test temporal ranges with data time range.
                if (!_parameters.getDataTimeRangeSet().getDataTimeRangeSets().front().overlaps(_parameters.getTemporalStartRange())) {
                    bValid = false;
                    PrintDirection.Printf(
                        "Invalid Parameter Setting:\nThe start time window range '%s' is not within the data time range.\n", BasePrint::P_PARAMERROR,
                        _parameters.getTemporalStartRangeStr().c_str()
                    );
                }
                if (!_parameters.getDataTimeRangeSet().getDataTimeRangeSets().front().overlaps(_parameters.getTemporalEndRange())) {
                    bValid = false;
                    PrintDirection.Printf(
                        "Invalid Parameter Setting:\nThe end time window range '%s' is not within the defined data time range.\n", BasePrint::P_PARAMERROR,
                        _parameters.getTemporalEndRangeStr().c_str()
                    );
                }
                // Test temporal ranges against each other.
                if (_parameters.getTemporalEndRange().getEnd() < _parameters.getTemporalStartRange().getStart()) {
                    bValid = false;
                    PrintDirection.Printf(
                        "Invalid Parameter Setting:\nThe temporal window end time range '%s' completely precedes the start time range '%s'.\n",
                        BasePrint::P_PARAMERROR, 
                        _parameters.getTemporalEndRangeStr().c_str(),
                        _parameters.getTemporalStartRangeStr().c_str()
                    );
                }
                if (bValid && _parameters.getTemporalEndRange().getStart() < _parameters.getTemporalStartRange().getStart()) {
                    PrintDirection.Printf(
                        "Warning:\nThe temporal window end time range '%s' partially precedes the start time range '%s'; not all intervals will be evaluated.\n",
                        BasePrint::P_PARAMERROR, 
                        _parameters.getTemporalEndRangeStr().c_str(),
                        _parameters.getTemporalStartRangeStr().c_str()
                    );
                }
            }
        }
        if (_parameters.isTemporalScanType(_parameters.getScanType()) && _parameters.isApplyingRiskWindowRestriction()) {
            if (_parameters.getRiskWindowPercentage() < 0.0 || _parameters.getRiskWindowPercentage() > 100.0) {
                bValid = false;
                PrintDirection.Printf("Invalid Parameter Setting:\nThe risk window percentage must be greater than or equal to zero and less than or equal to 100.\n", BasePrint::P_PARAMERROR);
            }
        }
        if (_parameters.isForcedCensoredAlgorithm()) {
            if (_parameters.getModelType() != Parameters::UNIFORM) {
                bValid = false;
                PrintDirection.Printf("Invalid Parameter Setting:\nForced censored algorithm is only permitted with the uniform model.\n", BasePrint::P_PARAMERROR);
            }
            if (_parameters.isPerformingDayOfWeekAdjustment()) {
                bValid = false;
                PrintDirection.Printf("Invalid Parameter Setting:\nForced censored algorithm is not permitted with day of week adjustment.\n", BasePrint::P_PARAMERROR);
            }
            if (_parameters.getSequentialScan()) {
                bValid = false;
                PrintDirection.Printf("Invalid Parameter Setting:\nForced censored algorithm is not permitted with sequential scan.\n", BasePrint::P_PARAMERROR);
            }
            if (_parameters.getPerformPowerEvaluations()) {
                bValid = false;
                PrintDirection.Printf("Invalid Parameter Setting:\nForced censored algorithm is not permitted with power evaluations.\n", BasePrint::P_PARAMERROR);
            }
        }
        if (_parameters.getMinimumCensorTime() < 2) {
            bValid = false;
            PrintDirection.Printf("Invalid Parameter Setting:\nThe minimum allowed censor time can not be less than 2.\n", BasePrint::P_PARAMERROR);
        }
        if (_parameters.getMinimumCensorPercentage() <= 0 || _parameters.getMinimumCensorPercentage() > 100) {
            bValid = false;
            PrintDirection.Printf("Invalid Parameter Setting:\nThe minimum censor time, as a percentage of data time range, must be greater than zero and less than or equal to 100.\n", BasePrint::P_PARAMERROR);
        }
        if (_parameters.getRiskWindowAltCensorDenominator() < 2) {
            bValid = false;
            PrintDirection.Printf("Invalid Parameter Setting:\nThe risk window alernative censor denominator can not be less than 2.\n", BasePrint::P_PARAMERROR);
        }
    } catch (prg_exception& x) {
        x.addTrace("ValidateInputParameters()","ParametersValidate");
        throw;
    }
    return bValid;
}

/** Validates input/output file parameters. */
bool ParametersValidate::ValidateAnalysisParameters(BasePrint& PrintDirection) const {
    bool bValid=true;
    try {
        switch (_parameters.getScanType()) {
            case Parameters::TREEONLY:
                if (!(_parameters.getConditionalType() == Parameters::UNCONDITIONAL || _parameters.getConditionalType() == Parameters::TOTALCASES)) {
                    bValid = false;
                    PrintDirection.Printf("Invalid Parameter Setting:\nA scan type of 'Tree Only' can either be unconditioned or conditioned on the total cases.\n", BasePrint::P_PARAMERROR);
                }
                if (!(_parameters.getModelType() == Parameters::BERNOULLI_TREE || _parameters.getModelType() == Parameters::POISSON)) {
                    bValid = false;
                    PrintDirection.Printf("Invalid Parameter Setting:\nA scan type of 'Tree Only' is not implemented for selected model.\n", BasePrint::P_PARAMERROR);
                }
                break;
            case Parameters::TREETIME:
                if (!(_parameters.getConditionalType() == Parameters::NODE || _parameters.getConditionalType() == Parameters::NODEANDTIME)) {
                    bValid = false;
                    PrintDirection.Printf("Invalid Parameter Setting:\nA scan type of 'Tree and Time' can either be conditioned on each node or on each node plus time.\n", BasePrint::P_PARAMERROR);
                }
                // if conditioning on both node and time, there isn't a model type the user can select
                if (bValid && _parameters.getConditionalType() == Parameters::NODEANDTIME) {
                    const_cast<Parameters&>(_parameters).setModelType(Parameters::MODEL_NOT_APPLICABLE);
                }
                if (!(_parameters.getModelType() == Parameters::UNIFORM || 
                    _parameters.getModelType() == Parameters::MODEL_NOT_APPLICABLE ||
                    _parameters.getModelType() == Parameters::BERNOULLI_TIME)) {
                    bValid = false;
                    PrintDirection.Printf("Invalid Parameter Setting:\nA scan type of 'Tree and Time' is not implemented for the selected model type.\n", BasePrint::P_PARAMERROR);
                }
                if (_parameters.getConditionalType() == Parameters::NODE && 
                    !(_parameters.getModelType() == Parameters::UNIFORM || _parameters.getModelType() == Parameters::BERNOULLI_TIME)) {
                    bValid = false;
                    PrintDirection.Printf("Invalid Parameter Setting:\nA scan type of 'Tree and Time' conditioned on the node requires the Uniform or Bernoulli Time model to be selected as model type.\n", BasePrint::P_PARAMERROR);
                }
                break;
            case Parameters::TIMEONLY:
                if (_parameters.getConditionalType() != Parameters::TOTALCASES) {
                    bValid = false;
                    PrintDirection.Printf("Invalid Parameter Setting:\nA scan type of 'Time Only' can be conditioned on the total cases only.\n", BasePrint::P_PARAMERROR);
                }
                if (!(_parameters.getModelType() == Parameters::UNIFORM || _parameters.getModelType() == Parameters::BERNOULLI_TIME)) {
                    bValid = false;
                    PrintDirection.Printf("Invalid Parameter Setting:\nA scan type of 'Time Only' is not implemented for the selected model type.\n", BasePrint::P_PARAMERROR);
                }
                break;
            default: throw prg_error("Unknown scan type (%d).", "ValidateAnalysisParameters()", _parameters.getScanType());
        }
        if (_parameters.getSelfControlDesign() && !(_parameters.getModelType() == Parameters::BERNOULLI_TREE && _parameters.getConditionalType() == Parameters::UNCONDITIONAL)) {
            bValid = false;
            PrintDirection.Printf("Invalid Parameter Setting:\nSelf control design is implemented for the unconditional Bernoulli model only.\n", BasePrint::P_PARAMERROR);
        }
        if (_parameters.getModelType() == Parameters::BERNOULLI_TREE && _parameters.getConditionalType() == Parameters::UNCONDITIONAL && !_parameters.getVariableCaseProbability()) {
           double top, bottom;
           string_to_type<double>(_parameters.getProbabilityRatio().first.c_str(), top);
           string_to_type<double>(_parameters.getProbabilityRatio().second.c_str(), bottom);
           if (top == 0 || bottom == 0 || top >= bottom) {
                bValid = false;
                PrintDirection.Printf("Invalid Parameter Setting:\nCase probabilty must be between zero and one.\n", BasePrint::P_PARAMERROR);
           }
        }
    } catch (prg_exception& x) {
        x.addTrace("ValidateAnalysisParameters()","ParametersValidate");
        throw;
    }
    return bValid;
}

/** Validates output options. */
bool ParametersValidate::ValidateOutputParameters(BasePrint & PrintDirection) const {
    bool bValid=true;
    try {
        // validate output file
        if (_parameters.getOutputFileName().empty()) {
            bValid = false;
            PrintDirection.Printf("Invalid Parameter Setting:\nNo results file specified.\n", BasePrint::P_PARAMERROR);
        } else if (!validateFileAccess(_parameters.getOutputFileName(), true)) {
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

/** Validates additional output options. */
bool ParametersValidate::ValidateAdditionalOutputParameters(BasePrint & PrintDirection) const {
    bool bValid=true;
    try {
        // validate output file
        if (_parameters.getReportAttributableRisk()) {
            if (_parameters.getAttributableRiskExposed() == 0) {
                bValid = false;
                PrintDirection.Printf("Invalid Parameter Setting:\nThe number of exposed cases for the attributable risk must be greater than zero. ", BasePrint::P_PARAMERROR);
            }
        }
        if (_parameters.getOutputTemporalGraphFile() && !_parameters.isTemporalScanType(_parameters.getScanType())) {
            const_cast<Parameters&>(_parameters).setOutputTemporalGraphFile(false);
            PrintDirection.Printf("Parameter Setting Warning:\nThe temporal graph option is only available for temporal scans.\nThe option was disabled.\n", BasePrint::P_WARNING);
        }
    } catch (prg_exception& x) {
        x.addTrace("ValidateAdditionalOutputParameters()","ParametersValidate");
        throw;
    }
    return bValid;
}

/** Validates power evaluation options. */
bool ParametersValidate::ValidatePowerEvaluationParametersParameters(BasePrint & PrintDirection) const {
    if (!_parameters.getPerformPowerEvaluations()) return true;

    bool bValid=true;
    if (!(_parameters.getModelType() == Parameters::POISSON ||
          _parameters.getModelType() == Parameters::BERNOULLI_TREE ||
          (_parameters.getScanType() == Parameters::TIMEONLY && _parameters.getConditionalType() == Parameters::TOTALCASES) ||
          (_parameters.getScanType() == Parameters::TREETIME && _parameters.getConditionalType() == Parameters::NODE)
         )) {
        PrintDirection.Printf("%s:\nThe power evaluation is only available for the Poisson and Bernoulli models, Tree-Time scan conditioned on node and Time-Only scan.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
        bValid = false;
    }
    if (Parameters::isTemporalScanType(_parameters.getScanType()) && _parameters.isPerformingDayOfWeekAdjustment()) {
        PrintDirection.Printf("%s:\nThe day of week adjustment is not implemented for the power evaluation.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
        bValid = false;
    }
    if (_parameters.getNumReplicationsRequested() < 999) {
        PrintDirection.Printf("%s:\nThe minimum number of standard replications in the power evaluation is %u.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, 999);
        bValid = false;
    }
    if (_parameters.getPValueReportingType() == Parameters::TERMINATION_PVALUE) {
        PrintDirection.Printf("%s:\nThe power evaluation is not available with the Monte Carlo early termination.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
        bValid = false;
    }
    if (_parameters.getPowerEvaluationReplications() < 100) {
        PrintDirection.Printf("%s:\nThe minimum number of power replications in the power evaluation is %u.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, 100);
        bValid = false;
    }
    if (_parameters.getPowerEvaluationReplications() % 100) {
        PrintDirection.Printf("%s:\nThe number of power replications in the power evaluation must be a multiple of 100.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
        bValid = false;
    }
    if (_parameters.getPowerEvaluationReplications() > _parameters.getNumReplicationsRequested() + 1) {
        PrintDirection.Printf("%s:\nThe number of standard replications must be at most one less than the number of power replications (%u).\n", 
                              BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, _parameters.getPowerEvaluationReplications());
        bValid = false;
    }
    if (_parameters.getCriticalValuesType() == Parameters::CV_POWER_VALUES) {
        if (_parameters.getCriticalValue05() < 0.0) {
            bValid = false;
            PrintDirection.Printf("%s:\nThe power evaluation critical value at .05 '%lf' is invalid. Please use a value greater than zero.\n",
                                  BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, _parameters.getCriticalValue05());
        }
        if (_parameters.getCriticalValue01() < 0.0) {
            bValid = false;
            PrintDirection.Printf("%s:\nThe power evaluation critical value at .01 '%lf' is invalid. Please use a value greater than zero.\n",
                                  BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, _parameters.getCriticalValue01());
        }
        if (_parameters.getCriticalValue001() < 0.0) {
            bValid = false;
            PrintDirection.Printf("%s:\nThe power evaluation critical value at .001 '%lf' is invalid. Please use a value greater than zero.\n",
                                  BasePrint::P_PARAMERROR, MSG_INVALID_PARAM, _parameters.getCriticalValue001());
        }
    }
    if (_parameters.getPowerEvaluationType() == Parameters::PE_ONLY_SPECIFIED_CASES && _parameters.getPowerEvaluationTotalCases() < 2) {
        PrintDirection.Printf("%s:\nThe number of specified power evaluation cases must be two or more.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
        bValid = false;
    }
    if (_parameters.getPowerEvaluationType() == Parameters::PE_ONLY_SPECIFIED_CASES &&
        !((_parameters.getModelType() == Parameters::POISSON || _parameters.getModelType() == Parameters::BERNOULLI_TREE || _parameters.getScanType() == Parameters::TIMEONLY) && _parameters.getConditionalType() == Parameters::TOTALCASES)) {
        PrintDirection.Printf("%s:\nThe power evaluation option to define total cases is only permitted with the conditional Poisson model or conditional Bernoulli model or time only scan.\n", BasePrint::P_PARAMERROR, MSG_INVALID_PARAM);
        bValid = false;
    }
    if (_parameters.getModelType() == Parameters::BERNOULLI_TREE && _parameters.getConditionalType() == Parameters::TOTALCASES) {
        double top, bottom;
        string_to_type<double>(_parameters.getPowerBaselineProbabilityRatio().first.c_str(), top);
        string_to_type<double>(_parameters.getPowerBaselineProbabilityRatio().second.c_str(), bottom);
        if (top == 0 || bottom == 0 || top >= bottom) {
            bValid = false;
            PrintDirection.Printf("Invalid Parameter Setting:\nThe power evaluation baseline probabilty must be between zero and one.\n", BasePrint::P_PARAMERROR);
        }
        if (_parameters.getPowerZ() <= 0 || 0.01 < _parameters.getPowerZ()) {
            bValid = false;
            PrintDirection.Printf("Invalid Parameter Setting:\nThe power evaluation Z constant must be greater than zero and no greater than 0.01.\n", BasePrint::P_PARAMERROR);
        }
    }
    return bValid;
}

/** Validates temporal window settings. */
bool ParametersValidate::ValidateTemporalWindowParameters(BasePrint & PrintDirection) const {
    bool bValid=true;

    if (!Parameters::isTemporalScanType(_parameters.getScanType()) && _parameters.getIsProspectiveAnalysis()) {
        bValid = false;
        PrintDirection.Printf("Invalid Parameter Setting:\nThe prospective evaluation option is only implemtend for temporal analyses.\n", BasePrint::P_PARAMERROR);
    }
    if (Parameters::isTemporalScanType(_parameters.getScanType()) && _parameters.getDataTimeRangeSet().getDataTimeRangeSets().size() > 0) {
        switch (_parameters.getMaximumWindowType()) {
            case Parameters::PERCENTAGE_WINDOW: {
                if (_parameters.getMaximumWindowPercentage() <= 0) {
                    bValid = false;
                    PrintDirection.Printf("Invalid Parameter Setting:\nThe maximum temporal size must be greater than zero.\n", BasePrint::P_PARAMERROR);
                } else if (_parameters.getMaximumWindowPercentage() > 50.0) {
                    PrintDirection.Printf("Parameter Setting Warning:\nThe maximum temporal size is greater than 50 percent of data time range.\n", BasePrint::P_WARNING);
                }
                unsigned int max_length = static_cast<unsigned int>(std::floor(static_cast<double>(_parameters.getDataTimeRangeSet().getMinMax().numDaysInRange()) * _parameters.getMaximumWindowPercentage()/100.0));
                if (_parameters.getMinimumWindowLength() <= 0 || _parameters.getMinimumWindowLength() > max_length) {
                    bValid = false;
                    PrintDirection.Printf("Invalid Parameter Setting:\nThe minimum temporal window length must be greater than zero and no greater than the specified maximum temporal window length.\nWith a specifed maximum as %g%%, the maximum window length is %u.\n", BasePrint::P_PARAMERROR, _parameters.getMaximumWindowPercentage(), max_length);
                }
            } break;
            case Parameters::FIXED_LENGTH: {
                unsigned int max_length = static_cast<unsigned int>(std::floor(static_cast<double>(_parameters.getDataTimeRangeSet().getMinMax().numDaysInRange()) * 0.5));
                if (_parameters.getMaximumWindowLength() <= 0) {
                    bValid = false;
                    PrintDirection.Printf("Invalid Parameter Setting:\nThe maximum temporal window length must be greater than zero.\n", BasePrint::P_PARAMERROR);
                } else if (_parameters.getMaximumWindowLength() > max_length) {
                    PrintDirection.Printf("Parameter Setting Warning:\nThe maximum temporal window length is greater than 50 percent of data time range (%u time units).\n", BasePrint::P_WARNING, max_length);
                }
                if (_parameters.getMinimumWindowLength() <= 0 || _parameters.getMinimumWindowLength() > _parameters.getMaximumWindowLength()) {
                    bValid = false;
                    PrintDirection.Printf("Invalid Parameter Setting:\nThe minimum window length must be greater than zero and no greater than the specified maximum window length.\n", BasePrint::P_PARAMERROR);
                }
            } break;
            default: throw prg_error("Unknown maximum window type (%d).", "getTemporalWindowParameters()", _parameters.getMaximumWindowType());
        }

        const DataTimeRange * pStart, * pEnd;
        if (_parameters.getRestrictTemporalWindows()) {
            pStart = &_parameters.getTemporalStartRange();
            pEnd = &_parameters.getTemporalEndRange();
        } else {
            if (_parameters.getDataTimeRangeSet().getDataTimeRangeSets().size() == 0) return bValid;
            pStart = &_parameters.getDataTimeRangeSet().getDataTimeRangeSets().front();
            pEnd = &_parameters.getDataTimeRangeSet().getDataTimeRangeSets().front();
        }

        // check whether any cuts will be evaluated given the specified maximum temporal window size and temporal window ranges
        int unitsInShortestWindow;
        if (pStart->getEnd() >= pEnd->getStart()) {
            unitsInShortestWindow = 1;
        } else {
            unitsInShortestWindow = pEnd->getEnd() - pStart->getEnd() + 1;
        }
        if (static_cast<DataTimeRange::index_t>(_parameters.getMaximumWindowInTimeUnits()) < unitsInShortestWindow) {
            bValid = false;
            PrintDirection.Printf("Invalid Parameter Setting:\nNo cuts will be evaluated since the maximum window size is %u yet the minimum number of time units in temporal window is %d.\n", 
                                  BasePrint::P_NOTICE, _parameters.getMaximumWindowInTimeUnits(), unitsInShortestWindow);
        }

        // check whether any cuts will be evaluated given the specified minimum temporal window size and temporal window ranges
        DataTimeRange::index_t unitsInTemporalWindow = pEnd->getEnd() - pStart->getStart() + 1;
        if (static_cast<DataTimeRange::index_t>(_parameters.getMinimumWindowLength()) > unitsInTemporalWindow) {
            bValid = false;
            PrintDirection.Printf("Invalid Parameter Setting:\nNo cuts will be evaluated since the minimum window size is %u yet the maximum number of time units in temporal window is %d.\n", 
                                  BasePrint::P_NOTICE, _parameters.getMinimumWindowLength(), unitsInTemporalWindow);
        }
    }
    return bValid;
}

/** Validates parameters parameters related to randomization seed.
    Prints errors to print direction and returns whether values are valid. */
bool ParametersValidate::ValidateRandomizationSeed(BasePrint& PrintDirection) const {
  if (!_parameters.getNumReplicationsRequested()) return true;

  if (_parameters.isRandomlyGeneratingSeed()) {
      double dMaxSeed = (double)RandomNumberGenerator::glM - (double)_parameters.getNumReplicationsRequested();
      boost::minstd_rand generator(static_cast<int>(time(0)));
      const_cast<Parameters&>(_parameters).setRandomizationSeed(boost::uniform_int<>(1,static_cast<int>(dMaxSeed))(generator));
      return true;
  }
  // validate hidden parameter which specifies randomization seed
  if (!(0 < _parameters.getRandomizationSeed() && _parameters.getRandomizationSeed() < RandomNumberGenerator::glM)) {
      PrintDirection.Printf("Invalid Parameter Setting:\nRandomization seed out of range [1 - %ld].\n",
                            BasePrint::P_PARAMERROR, RandomNumberGenerator::glM - 1);
      return false;
  }
  // validate that generated seeds during randomization will not exceed defined range
  double dMaxRandomizationSeed = (double)_parameters.getRandomizationSeed() + (double)_parameters.getNumReplicationsRequested();
  if (dMaxRandomizationSeed >= static_cast<double>(RandomNumberGenerator::glM)) {
      /** case #1 - glRandomizationSeed == RandomNumberGenerator::glDefaultSeed
            In this case, it is assumed that user accepted default - at this time
            changing the initial seed through parameter settings is a 'hidden' parameter;
            so no direction should be given regarding the alteration of seed value. */
      if (_parameters.getRandomizationSeed() == RandomNumberGenerator::glDefaultSeed) {
        double dMaxReplications = (double)RandomNumberGenerator::glM - (double)_parameters.getRandomizationSeed();
        dMaxReplications = (floor((dMaxReplications)/1000) - 1)  * 1000 + 999;
        PrintDirection.Printf("Invalid Parameter Setting:\nRequested number of replications causes randomization seed to exceed defined limit. "
                              "Maximum number of replications is %.0lf.\n", BasePrint::P_PARAMERROR, dMaxReplications);
        return false;
      }
      /** case #2 - user specified alternate randomization seed
            This alternate seed or the number of requested replications could be the problem.
            User has two options, either pick a lesser seed or request fewer replications. */
      // calculate maximum seed for requested number of replications
      double dMaxSeed = (double)RandomNumberGenerator::glM - (double)_parameters.getNumReplicationsRequested();
      // calculate maximum number of replications for requested seed
      double dMaxReplications = (double)RandomNumberGenerator::glM - (double)_parameters.getRandomizationSeed();
      dMaxReplications = (floor((dMaxReplications)/1000) - 1)  * 1000 + 999;
      // check whether specified combination of seed and requested number of replications fights each other
      if (dMaxReplications < 9 && (dMaxSeed <= 0 || dMaxSeed > RandomNumberGenerator::glM)) {
        PrintDirection.Printf("Invalid Parameter Setting:\nRandomization seed will exceed defined limit. "
                              "The specified initial seed, in conjunction with the number of replications, "
                              "contend for numerical range in defined limits. Please modify the specified "
                              "initial seed and/or lessen the number of replications and try again.\n", BasePrint::P_PARAMERROR);
      }
      // check that randomization seed is not so large that we can't run any replications
      else if (dMaxReplications < 9) {
        PrintDirection.Printf("Invalid Parameter Setting:\nRandomization seed will exceed defined limit. "
                              "The intial seed specified prevents any replications from being performed. "
                              "With %ld replications, the initial seed can be [0 - %.0lf].\n",
                              BasePrint::P_PARAMERROR, _parameters.getNumReplicationsRequested(), dMaxSeed);
      }
      // check that number of replications isn't too large
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

bool ParametersValidate::ValidateSequentialScanParameters(BasePrint & PrintDirection) const {
    bool bValid=true;
    if (_parameters.getSequentialScan()) {
        if (!((_parameters.getScanType() == Parameters::TIMEONLY && _parameters.getConditionalType() == Parameters::TOTALCASES) ||
              (_parameters.getModelType() == Parameters::BERNOULLI_TREE && _parameters.getConditionalType() == Parameters::UNCONDITIONAL) ||
			  _parameters.getModelType() == Parameters::POISSON
			)) {
            bValid = false;
            PrintDirection.Printf("Invalid Parameter Setting:\nSequential scan is only implemented for the time-only scan conditioned on total cases, unconditional Benoulli, and Poisson.\n", BasePrint::P_PARAMERROR);
        }
        if (_parameters.getModelType() == Parameters::BERNOULLI_TREE && 
            _parameters.getConditionalType() == Parameters::UNCONDITIONAL && 
            _parameters.getVariableCaseProbability()) {
            bValid = false;
            PrintDirection.Printf("Invalid Parameter Setting:\nSequential scan is not implemented for the unconditional Benoulli using variable case probability.\n", BasePrint::P_PARAMERROR);
        }
        if (_parameters.getPerformPowerEvaluations()) {
            bValid = false;
            PrintDirection.Printf("Invalid Parameter Setting:\nThe sequential scan is not implemented for the power estimation.\n", BasePrint::P_PARAMERROR);
        }
        if (_parameters.isReadingSimulationData()) {
            bValid = false;
            PrintDirection.Printf("Invalid Parameter Setting:\nThe sequential scan is not implemented with the options to read simulation data.\n", BasePrint::P_PARAMERROR);
        }
        if (_parameters.getPerformDayOfWeekAdjustment()) {
            bValid = false;
            PrintDirection.Printf("Invalid Parameter Setting:\nThe sequential scan is not implemented for the day of week adjustment.\n", BasePrint::P_PARAMERROR);
        }
        if (_parameters.getScanType() == Parameters::TIMEONLY) {
            //if (_parameters.getSequentialMaximumSignal() < 2) {
            //    bValid = false;
            //    PrintDirection.Printf("Invalid Parameter Setting:\nThe total cases for sequential scan must be 2 or more.\n", BasePrint::P_PARAMERROR);
            //}
            if (!(_parameters.getSequentialMinimumSignal() >= 2 && _parameters.getSequentialMinimumSignal() < _parameters.getSequentialMaximumSignal())) {
                bValid = false;
                PrintDirection.Printf("Invalid Parameter Setting:\nFor sequential scan, the minimum number of cases to signal must be greater than 1 and less than total sequential cases.\n", BasePrint::P_PARAMERROR);
            }
        }
        if (_parameters.getNumReplicationsRequested() < 999) {
            bValid = false;
            PrintDirection.Printf("Invalid Parameter Setting:\nFor sequential scan, the minimum number of replications is 999.\n", BasePrint::P_PARAMERROR);
        } else if (_parameters.getModelType() == Parameters::BERNOULLI_TREE || _parameters.getModelType() == Parameters::POISSON) {
            if (_parameters.getSequentialAlphaOverall() < 1.0 / static_cast<double>(_parameters.getNumReplicationsRequested() + 1) || 0.05 < _parameters.getSequentialAlphaOverall()) {
                bValid = false;
                std::string buffer;
                PrintDirection.Printf(
                    "Invalid Parameter Setting:\nFor sequential scan, the overal alpha cannot be less than %s with %u replications or greater than 0.05.\n",
                    BasePrint::P_PARAMERROR,
                    getRoundAsString(1.0 / static_cast<double>(_parameters.getNumReplicationsRequested() + 1),
                        buffer, static_cast<unsigned int>(ceil(fabs(log10(_parameters.getNumReplicationsRequested()))))
                    ).c_str(),
                    _parameters.getNumReplicationsRequested()
                );
            }
            if (_parameters.getSequentialAlphaSpending() < 1.0 / static_cast<double>(_parameters.getNumReplicationsRequested() + 1)) {
                bValid = false;
                std::string buffer;
                PrintDirection.Printf("Invalid Parameter Setting:\nFor sequential scan, alpha spending cannot be less than %s with %u replications.\n", 
                    BasePrint::P_PARAMERROR, 
                    getRoundAsString(1.0 / static_cast<double>(_parameters.getNumReplicationsRequested() + 1), 
                        buffer, static_cast<unsigned int>(ceil(fabs(log10(_parameters.getNumReplicationsRequested()))))
                    ).c_str(),
                    _parameters.getNumReplicationsRequested()
                );
            }
            if (_parameters.getSequentialAlphaSpending() > _parameters.getSequentialAlphaOverall()) {
                bValid = false;
                PrintDirection.Printf("Invalid Parameter Setting:\nFor sequential scan, alpha spending cannot be greater than alpha overall.\n", BasePrint::P_PARAMERROR);
            }
        }
        if (_parameters.getReportCriticalValues()) {
            // We don't report p-value or critical values at the 0.001, 0.01, and 0.05 for sequential scan.
            const_cast<Parameters&>(_parameters).setReportCriticalValues(false);
            PrintDirection.Printf("Parameter Setting Warning:\nThe option to report critical values is not available for sequential scans.\nThe option was disabled.\n", BasePrint::P_WARNING);
        }
    }
    return bValid;
}
