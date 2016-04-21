//******************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//******************************************************************************
#include "DataFileWriter.h"
#include "PrjException.h"
#include "ScanRunner.h"
#include "IniParameterFileAccess.h"

/** class constructor */
RecordBuffer::RecordBuffer(const ptr_vector<FieldDef>& vFields) : vFieldDefinitions(vFields) {
  for (size_t t=0; t < vFieldDefinitions.size(); ++t) {
     gvFieldValues.push_back(FieldValue(vFieldDefinitions[t]->GetType()));
     gvBlankFields.push_back(true);
  }
}

/** class destructor */
RecordBuffer::~RecordBuffer() {}

/** FieldDef definition for field with name. */
const FieldDef & RecordBuffer::GetFieldDefinition(const std::string& sFieldName) const {
  return *vFieldDefinitions[GetFieldIndex(sFieldName)];
}

/** FieldDef definition for field at index. */
const FieldDef & RecordBuffer::GetFieldDefinition(unsigned int iFieldIndex) const {
  try {
    if (iFieldIndex >= vFieldDefinitions.size())
      throw prg_error("Index %u out of range [size=%u].", "GetFieldDefinition()", iFieldIndex, vFieldDefinitions.size());
  }
  catch (prg_exception& x) {
    x.addTrace("GetFieldDefinition()","RecordBuffer");
    throw;
  }
  return *vFieldDefinitions[iFieldIndex];
}

// returns whether or not the field at iFieldNumber should be blank
// pre : none
// post : returns true is field should be blank
bool RecordBuffer::GetFieldIsBlank(unsigned int iFieldNumber) const {
  try {
    if (iFieldNumber >= gvBlankFields.size())
      throw prg_error("Index %u out of range [size=%u].", "GetFieldIsBlank()", iFieldNumber, gvBlankFields.size());
  }
  catch (prg_exception& x) {
    x.addTrace("GetFieldIsBlank()","RecordBuffer");
    throw;
  }
  return gvBlankFields[iFieldNumber];
}

/** Returns field index for named field. */
unsigned int RecordBuffer::GetFieldIndex(const std::string& sFieldName) const {
  bool                 bFound(false);
  unsigned int         i, iPosition;

  try {
    for (i=0; i < vFieldDefinitions.size() && !bFound; ++i) {
       bFound = (!strcmp(vFieldDefinitions[i]->GetName(), sFieldName.c_str()));
       iPosition = i;
   }
   if (!bFound)
     throw prg_error("Field name %s not found in the field vector.", "GetFieldIndex()", sFieldName.c_str());
  }
  catch (prg_exception& x) {
    x.addTrace("GetFieldIndex()","RecordBuffer");
    throw;
  }
  return iPosition;
}

/** Returns reference to field value for named field, setting field 'blank' indicator to false. */
FieldValue& RecordBuffer::GetFieldValue(const std::string& sFieldName) {
  try {
    unsigned int iFieldIndex = GetFieldIndex(sFieldName);
    gvBlankFields[iFieldIndex] = false;
    return gvFieldValues[iFieldIndex];
  }
  catch (prg_exception& x) {
    x.addTrace("GetFieldValue()","RecordBuffer");
    throw;
  }
}

/** Returns FieldValue reference for field index. Throws prg_error if
    iFieldIndex is greater than number of FieldValues. */
FieldValue& RecordBuffer::GetFieldValue(unsigned int iFieldIndex) {
  try {
    if (iFieldIndex >= gvFieldValues.size())
      throw prg_error("Index %u out of range [size=%u].", "GetFieldValue()", iFieldIndex, gvFieldValues.size());
    gvBlankFields[iFieldIndex] = false;
  }
  catch (prg_exception& x) {
    x.addTrace("GetFieldValue()","RecordBuffer");
    throw;
  }
  return gvFieldValues[iFieldIndex];
}

/** Returns FieldValue reference for field index. Throws prj_error if
    iFieldIndex is greater than number of FieldValues. */
const FieldValue& RecordBuffer::GetFieldValue(unsigned int iFieldIndex) const {
  try {
    if (iFieldIndex >= gvFieldValues.size())
      throw prg_error("Index %u out of range [size=%u].", "GetFieldValue()", iFieldIndex, gvFieldValues.size());
  }
  catch (prg_exception& x) {
    x.addTrace("GetFieldValue()","RecordBuffer");
    throw;
  }
  return gvFieldValues[iFieldIndex];
}

/** Sets all blank indicators as not blank. */
void RecordBuffer::SetAllFieldsBlank(bool bBlank) {
  std::fill(gvBlankFields.begin(), gvBlankFields.end(), bBlank);
}

/** Sets the field at fieldnumber to either be blank or non-blank. */
void RecordBuffer::SetFieldIsBlank(const std::string& sFieldName, bool bBlank) {
  try {
    gvBlankFields[GetFieldIndex(sFieldName)] = bBlank;
  }
  catch (prg_exception& x) {
    x.addTrace("SetFieldIsBlank()","RecordBuffer");
    throw;
  }
}

/** Sets the field at fieldnumber to either be blank or non-blank. */
void RecordBuffer::SetFieldIsBlank(unsigned int iFieldNumber, bool bBlank) {
  try {
    if (iFieldNumber >= gvBlankFields.size())
      throw prg_error("Index %u out of range [size=%u].", "SetFieldIsBlank()", iFieldNumber, gvBlankFields.size());
    gvBlankFields[iFieldNumber] = bBlank;
  }
  catch (prg_exception& x) {
    x.addTrace("SetFieldIsBlank()","RecordBuffer");
    throw;
  }
}

//////////////////////// CSVDataFileWriter /////////////////////////////////

const char * CSVDataFileWriter::CSV_FILE_EXT = ".csv";

/** constructor */
CSVDataFileWriter::CSVDataFileWriter(std::ofstream& outfile, const ptr_vector<FieldDef>& vFieldDefs, bool printHeaders, bool append) : _outfile(outfile) {
    try {
        if (!_outfile.is_open())
            throw resolvable_error("std::ofstream is not open.");

        //write column headers when requested
        if (printHeaders) {
            ptr_vector<FieldDef>::const_iterator itr=vFieldDefs.begin(), itr_end=vFieldDefs.end();
            for  (; itr != itr_end; ++itr) {
                _outfile << (*itr)->GetName();
                if (itr + 1 != itr_end) _outfile << ",";
            }
            _outfile << std::endl;
        }
    } catch (prg_exception& x) {
        x.addTrace("constructor()","CSVDataFileWriter");
        throw;
    }
}

// creates the formatted string from the precision and type of the field value and stores the formatted
//  output value in sValue
// pre : none
// post : formats the string sValue based upon the settings of fieldValue
void CSVDataFileWriter::createFormatString(std::string& sValue, const FieldDef& FieldDef, const FieldValue& fv) {
  std::string   temp;
  unsigned long ulStringLength = 0;

  switch(fv.GetType()) {
    case FieldValue::ALPHA_FLD  :
      {if (fv.AsString().find(",") != std::string::npos) {
          printString(sValue, "\"%s\"", fv.AsString().c_str());
       } else sValue = fv.AsString();
      } break;
    case FieldValue::NUMBER_FLD :
        sValue = getValueAsString(fv.AsDouble(), temp ,FieldDef.GetAsciiDecimals());
      break;
    default : throw prg_error("Unsupported field type %c", "Error!", fv.GetType());
  }
}

/** Writes record buffer to file stream. */
void CSVDataFileWriter::writeRecord(const RecordBuffer& Record) {
  std::string sFormatString;

  for (unsigned int j=0; j < Record.GetNumFields(); ++j) {
    sFormatString.clear();
    if (!Record.GetFieldIsBlank(j))
      createFormatString(sFormatString, Record.GetFieldDefinition(j), Record.GetFieldValue(j));
    _outfile << sFormatString;
    if (j < Record.GetNumFields() - 1) _outfile << ",";
  }
  _outfile << std::endl;
}


////////////////////////////////////////////////////////////

const char * DataRecordWriter::CUT_NUM_FIELD                             = "Cut";
const char * DataRecordWriter::NODE_ID_FIELD                             = "Node Identifier";
const char * DataRecordWriter::NODE_CASES_FIELD                          = "Node Cases";
const char * DataRecordWriter::START_WINDOW_FIELD                        = "Time Window Start";
const char * DataRecordWriter::END_WINDOW_FIELD                          = "Time Window End";
const char * DataRecordWriter::OBSERVATIONS_FIELD                        = "Observations";

const char * DataRecordWriter::WNDW_CASES_FIELD                          = "Cases in Window";
const char * DataRecordWriter::CASES_FIELD                               = "Cases";
const char * DataRecordWriter::OBSERVED_CASES_FIELD                      = "Observed Cases";

const char * DataRecordWriter::EXPECTED_FIELD                            = "Expected";
const char * DataRecordWriter::EXPECTED_CASES_FIELD                      = "Expected Cases";

const char * DataRecordWriter::RELATIVE_RISK_FIELD                       = "Relative Risk";
const char * DataRecordWriter::EXCESS_CASES_FIELD                        = "Excess Cases";
const char * DataRecordWriter::ATTRIBUTABLE_RISK_FIELD                   = "Attributable Risk";
const char * DataRecordWriter::LOG_LIKL_RATIO_FIELD                      = "Log Likelihood Ratio";
const char * DataRecordWriter::TEST_STATISTIC_FIELD                      = "Test Statistic";
const char * DataRecordWriter::P_VALUE_FLD                               = "P-value";
const size_t DataRecordWriter::DEFAULT_LOC_FIELD_SIZE                    = 30;
const size_t DataRecordWriter::MAX_LOC_FIELD_SIZE                        = 254;

const char * DataRecordWriter::HYPOTHESIS_ALTERNATIVE_NUM_FIELD          = "Alternative Hypothesis";
const char * DataRecordWriter::HA_ALPHA05_FIELD                          = "Alpha 0.05";
const char * DataRecordWriter::HA_ALPHA01_FIELD                          = "Alpha 0.01";
const char * DataRecordWriter::HA_ALPHA001_FIELD                         = "Alpha 0.001";

///////////// CutsRecordWriter ///////////////////////////////////

const char * CutsRecordWriter::CUT_FILE_SUFFIX                            = "";

CutsRecordWriter::CutsRecordWriter(const ScanRunner& scanRunner) : _scanner(scanRunner) {
  unsigned short uwOffset=0;
  std::string    buffer;
  const Parameters& params = _scanner.getParameters();

  try {
    CreateField(_dataFieldDefinitions, CUT_NUM_FIELD, FieldValue::NUMBER_FLD, 19, 0, uwOffset, 0);
    CreateField(_dataFieldDefinitions, NODE_ID_FIELD, FieldValue::ALPHA_FLD, static_cast<short>(getLocationIdentiferFieldLength()), 0, uwOffset, 0);
    switch (params.getModelType()) {
        case Parameters::BERNOULLI :
            CreateField(_dataFieldDefinitions, OBSERVATIONS_FIELD, FieldValue::NUMBER_FLD, 19, 0, uwOffset, 0);
            CreateField(_dataFieldDefinitions, CASES_FIELD, FieldValue::NUMBER_FLD, 19, 0, uwOffset, 0);
            CreateField(_dataFieldDefinitions, EXPECTED_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
            break;
        case Parameters::POISSON :
            CreateField(_dataFieldDefinitions, OBSERVED_CASES_FIELD, FieldValue::NUMBER_FLD, 19, 0, uwOffset, 0);
            CreateField(_dataFieldDefinitions, EXPECTED_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
            break;
        case Parameters::UNIFORM :
        case Parameters::MODEL_NOT_APPLICABLE :
        default :
            if (Parameters::isTemporalScanType(params.getScanType())) {
                CreateField(_dataFieldDefinitions, NODE_CASES_FIELD, FieldValue::NUMBER_FLD, 19, 0, uwOffset, 0);
                CreateField(_dataFieldDefinitions, START_WINDOW_FIELD, FieldValue::NUMBER_FLD, 19, 0, uwOffset, 0);
                CreateField(_dataFieldDefinitions, END_WINDOW_FIELD, FieldValue::NUMBER_FLD, 19, 0, uwOffset, 0);
                CreateField(_dataFieldDefinitions, WNDW_CASES_FIELD, FieldValue::NUMBER_FLD, 19, 0, uwOffset, 0);
                CreateField(_dataFieldDefinitions, EXPECTED_CASES_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
            } else
                throw prg_error("Unknown model type (%d).", "CutsRecordWriter()", params.getModelType());
    }
    CreateField(_dataFieldDefinitions, RELATIVE_RISK_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
    CreateField(_dataFieldDefinitions, EXCESS_CASES_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);

    if (params.getReportAttributableRisk()) {
        CreateField(_dataFieldDefinitions, ATTRIBUTABLE_RISK_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
    }

    if ((params.getScanType() == Parameters::TREETIME && params.getConditionalType() == Parameters::NODEANDTIME) ||
        (params.getScanType() == Parameters::TIMEONLY && params.getConditionalType() == Parameters::TOTALCASES && params.isPerformingDayOfWeekAdjustment()) ||
        (params.getScanType() == Parameters::TREETIME && params.getConditionalType() == Parameters::NODE && params.isPerformingDayOfWeekAdjustment())) {
        // If we stick with Poisson log-likelihood calculation, then label is 'Test Statistic' in place of 'Log Likelihood Ratio', hyper-geometric is 'Log Likelihood Ratio'.
        CreateField(_dataFieldDefinitions, TEST_STATISTIC_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 6);
        //CreateField(_dataFieldDefinitions, LOG_LIKL_RATIO_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 6);
    } else {
        CreateField(_dataFieldDefinitions, LOG_LIKL_RATIO_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 6);
    }

    printString(buffer, "%u", params.getNumReplicationsRequested());
    CreateField(_dataFieldDefinitions, P_VALUE_FLD, FieldValue::NUMBER_FLD, 19, 17/*std::min(17,(int)buffer.size())*/, uwOffset, static_cast<unsigned short>(buffer.size()));

    _outfile.open(getFilename(params, buffer).c_str(), std::ofstream::trunc);
    if (!_outfile.is_open())
      throw resolvable_error("Unable to open/create file %s.", buffer.c_str());
    _csvWriter.reset(new CSVDataFileWriter(_outfile, _dataFieldDefinitions, params.isPrintColumnHeaders()));
  } catch (prg_exception& x) {
    x.addTrace("constructor()","CutsRecordWriter");
    throw;
  }
}

CutsRecordWriter::~CutsRecordWriter() {
    try {
        _outfile.close();
  } catch (...) { }
}

std::string& CutsRecordWriter::getFilename(const Parameters& parameters, std::string& buffer) {
    return getDerivedFilename(parameters.getOutputFileName(), CutsRecordWriter::CUT_FILE_SUFFIX, CSVDataFileWriter::CSV_FILE_EXT, buffer);
}

void CutsRecordWriter::write(unsigned int cutIndex) const {
    const Parameters& params = _scanner.getParameters();
    std::string buffer;
    RecordBuffer Record(_dataFieldDefinitions);
    ScanRunner::Loglikelihood_t calcLogLikelihood(AbstractLoglikelihood::getNewLoglikelihood(_scanner.getParameters(), _scanner.getTotalC(), _scanner.getTotalN()));

    try {
        Record.GetFieldValue(CUT_NUM_FIELD).AsDouble() = cutIndex + 1;
        Record.GetFieldValue(NODE_ID_FIELD).AsString() = _scanner.getNodes()[_scanner.getCuts()[cutIndex]->getID()]->getIdentifier();
        switch (params.getModelType()) {
            case Parameters::BERNOULLI :
                Record.GetFieldValue(OBSERVATIONS_FIELD).AsDouble() = static_cast<int>(_scanner.getCuts()[cutIndex]->getN());
                Record.GetFieldValue(CASES_FIELD).AsDouble() = static_cast<int>(_scanner.getCuts()[cutIndex]->getC());
                Record.GetFieldValue(EXPECTED_FIELD).AsDouble() = _scanner.getCuts()[cutIndex]->getExpected(_scanner);
                break;
            case Parameters::POISSON :
                Record.GetFieldValue(OBSERVED_CASES_FIELD).AsDouble() = _scanner.getCuts()[cutIndex]->getC();
                Record.GetFieldValue(EXPECTED_FIELD).AsDouble() = _scanner.getCuts()[cutIndex]->getExpected(_scanner);
                break;
            case Parameters::UNIFORM :
            case Parameters::MODEL_NOT_APPLICABLE :
            default :
                if (Parameters::isTemporalScanType(params.getScanType())) {
                    if (params.isPerformingDayOfWeekAdjustment() || params.getConditionalType() == Parameters::NODEANDTIME) {
                        Record.GetFieldValue(NODE_CASES_FIELD).AsDouble() = static_cast<int>(_scanner.getNodes()[_scanner.getCuts()[cutIndex]->getID()]->getBrC());
                    } else {
                        Record.GetFieldValue(NODE_CASES_FIELD).AsDouble() = static_cast<int>(_scanner.getCuts()[cutIndex]->getN());
                    }
                    Record.GetFieldValue(START_WINDOW_FIELD).AsDouble() = _scanner.getCuts()[cutIndex]->getStartIdx()  - _scanner.getZeroTranslationAdditive();
                    Record.GetFieldValue(END_WINDOW_FIELD).AsDouble() = _scanner.getCuts()[cutIndex]->getEndIdx() - _scanner.getZeroTranslationAdditive();
                    Record.GetFieldValue(WNDW_CASES_FIELD).AsDouble() = static_cast<int>(_scanner.getCuts()[cutIndex]->getC());
                    Record.GetFieldValue(EXPECTED_CASES_FIELD).AsDouble() = _scanner.getCuts()[cutIndex]->getExpected(_scanner);
                } else
                    throw prg_error("Unknown model type (%d).", "CutsRecordWriter()", params.getModelType());
        }
        Record.GetFieldValue(RELATIVE_RISK_FIELD).AsDouble() = _scanner.getCuts()[cutIndex]->getRelativeRisk(_scanner);
        Record.GetFieldValue(EXCESS_CASES_FIELD).AsDouble() = _scanner.getCuts()[cutIndex]->getExcessCases(_scanner);
        if (params.getReportAttributableRisk()) {
            Record.GetFieldValue(ATTRIBUTABLE_RISK_FIELD).AsDouble() = _scanner.getCuts()[cutIndex]->getAttributableRisk(_scanner);
        }

        if ((params.getScanType() == Parameters::TREETIME && params.getConditionalType() == Parameters::NODEANDTIME) ||
            (params.getScanType() == Parameters::TIMEONLY && params.getConditionalType() == Parameters::TOTALCASES && params.isPerformingDayOfWeekAdjustment()) ||
            (params.getScanType() == Parameters::TREETIME && params.getConditionalType() == Parameters::NODE && params.isPerformingDayOfWeekAdjustment())) {
            // If we stick with Poisson log-likelihood calculation, then label is 'Test Statistic' in place of 'Log Likelihood Ratio', hyper-geometric is 'Log Likelihood Ratio'.
            Record.GetFieldValue(TEST_STATISTIC_FIELD).AsDouble() = calcLogLikelihood->LogLikelihoodRatio(_scanner.getCuts()[cutIndex]->getLogLikelihood());
        } else {
            Record.GetFieldValue(LOG_LIKL_RATIO_FIELD).AsDouble() = calcLogLikelihood->LogLikelihoodRatio(_scanner.getCuts()[cutIndex]->getLogLikelihood());
        }
        if (_scanner.getParameters().getNumReplicationsRequested() > 9/*require more than 9 replications to report p-values*/) {
            Record.GetFieldValue(P_VALUE_FLD).AsDouble() =  static_cast<double>(_scanner.getCuts()[cutIndex]->getRank()) /(_scanner.getParameters().getNumReplicationsRequested() + 1);
        }
        _csvWriter->writeRecord(Record);
    } catch (prg_exception& x) {
        x.addTrace("write()","CutsRecordWriter");
        throw;
    }
}

///////////// PowerEstimationRecordWriter ///////////////////////////////////

const char * PowerEstimationRecordWriter::POWER_FILE_SUFFIX                            = "_power";

/* constructor: defines data filefields and allocates csv data file writer. */
PowerEstimationRecordWriter::PowerEstimationRecordWriter(const ScanRunner& scanRunner) : _scanner(scanRunner) {
    unsigned short uwOffset=0;
    std::string    buffer;

    try {
        CreateField(_dataFieldDefinitions, HYPOTHESIS_ALTERNATIVE_NUM_FIELD, FieldValue::NUMBER_FLD, 19, 0, uwOffset, 0);
        CreateField(_dataFieldDefinitions, HA_ALPHA05_FIELD, FieldValue::ALPHA_FLD, 19, 0, uwOffset, 0);
        CreateField(_dataFieldDefinitions, HA_ALPHA01_FIELD, FieldValue::ALPHA_FLD, 19, 0, uwOffset, 0);
        CreateField(_dataFieldDefinitions, HA_ALPHA001_FIELD, FieldValue::ALPHA_FLD, 19, 0, uwOffset, 0);

        _outfile.open(getFilename(_scanner.getParameters(), buffer).c_str(), std::ofstream::trunc);
        if (!_outfile.is_open())
            throw resolvable_error("Unable to open/create file %s.", buffer.c_str());
        _csvWriter.reset(new CSVDataFileWriter(_outfile, _dataFieldDefinitions, _scanner.getParameters().isPrintColumnHeaders()));
    } catch (prg_exception& x) {
        x.addTrace("constructor()","PowerEstimationRecordWriter");
        throw;
    }
}

PowerEstimationRecordWriter::~PowerEstimationRecordWriter() {
    try {
        _outfile.close();
    } catch (...) { }
}

/* Returns the filename which alternative hypothesis are written. The filename is a derivative of the main results filename. */
std::string& PowerEstimationRecordWriter::getFilename(const Parameters& parameters, std::string& buffer) {
    return getDerivedFilename(parameters.getOutputFileName(), PowerEstimationRecordWriter::POWER_FILE_SUFFIX, CSVDataFileWriter::CSV_FILE_EXT, buffer);
}

/* Writes power estimation values for all alternative hypothesis iterations to csv file. */
void PowerEstimationRecordWriter::write() const {
    RecordBuffer  Record(_dataFieldDefinitions);

    try {
        std::string buffer;
        ScanRunner::PowerEstimationContainer_t::const_iterator itr=_scanner.getPowerEstimations().begin(), itr_end=_scanner.getPowerEstimations().end();
        for (;itr != itr_end; ++itr) {
                Record.GetFieldValue(HYPOTHESIS_ALTERNATIVE_NUM_FIELD).AsDouble() = static_cast<double>(std::distance(_scanner.getPowerEstimations().begin(), itr) + 1);
                Record.GetFieldValue(HA_ALPHA05_FIELD).AsString() = getRoundAsString(itr->get<0>(),buffer, 3);
                Record.GetFieldValue(HA_ALPHA01_FIELD).AsString() = getRoundAsString(itr->get<1>(),buffer, 3);
                Record.GetFieldValue(HA_ALPHA001_FIELD).AsString() = getRoundAsString(itr->get<2>(),buffer, 3);
                _csvWriter->writeRecord(Record);
        }
    } catch (prg_exception& x) {
        x.addTrace("write()","PowerEstimationRecordWriter");
        throw;
    }
}

///////////// LoglikelihoodRatioWriter ///////////////////////////////////

const char * LoglikelihoodRatioWriter::LLR_FILE_SUFFIX       = "_llr";
const char * LoglikelihoodRatioWriter::LLR_HA_FILE_SUFFIX    = "_llr_ha";
const char * LoglikelihoodRatioWriter::LOG_LIKL_RATIO_FIELD  = "LLR";

LoglikelihoodRatioWriter::LoglikelihoodRatioWriter(const ScanRunner& scanRunner) : _scanner(scanRunner) {
  unsigned short uwOffset=0;
  std::string buffer;
  try {
    CreateField(_dataFieldDefinitions, LOG_LIKL_RATIO_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 6);
  } catch (prg_exception& x) {
    x.addTrace("constructor()","LoglikelihoodRatioWriter");
    throw;
  }
}

LoglikelihoodRatioWriter::LoglikelihoodRatioWriter(const ScanRunner& scanRunner, bool ispower, bool append) : _scanner(scanRunner) {
  unsigned short uwOffset=0;
  std::string buffer;
  try {
    CreateField(_dataFieldDefinitions, LOG_LIKL_RATIO_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 6);

    _outfile.open(getFilename(_scanner.getParameters(), buffer, ispower).c_str(), append ? std::ofstream::app : std::ofstream::trunc);
    if (!_outfile.is_open())
        throw resolvable_error("Unable to open/create file %s.", buffer.c_str());
    _csvWriter.reset(new CSVDataFileWriter(_outfile, _dataFieldDefinitions, _scanner.getParameters().isPrintColumnHeaders() && !append, append));
  } catch (prg_exception& x) {
    x.addTrace("constructor()","LoglikelihoodRatioWriter");
    throw;
  }
}

LoglikelihoodRatioWriter::~LoglikelihoodRatioWriter() {
    try {
        _outfile.close();
    } catch (...) { }
}

std::string& LoglikelihoodRatioWriter::getFilename(const Parameters& parameters, std::string& buffer, bool ispower) {
  return getDerivedFilename(parameters.getOutputFileName(), ispower ? LoglikelihoodRatioWriter::LLR_HA_FILE_SUFFIX : LoglikelihoodRatioWriter::LLR_FILE_SUFFIX, CSVDataFileWriter::CSV_FILE_EXT, buffer);
}

void LoglikelihoodRatioWriter::write(double llr) const {
    std::string  buffer;
    RecordBuffer Record(_dataFieldDefinitions);

    try {
        Record.GetFieldValue(LOG_LIKL_RATIO_FIELD).AsDouble() = llr;
        _csvWriter->writeRecord(Record);
    } catch (prg_exception& x) {
        x.addTrace("write()","LoglikelihoodRatioWriter");
        throw;
    }
}

///////////////////////////////// SequentialScanLoglikelihoodRatioWriter ////////////////////////////////////

const char * SequentialScanLoglikelihoodRatioWriter::SEQUENTIAL_FILE_SUFFIX       = "_sequential";

SequentialScanLoglikelihoodRatioWriter::SequentialScanLoglikelihoodRatioWriter(const ScanRunner& scanRunner) : LoglikelihoodRatioWriter(scanRunner) {
    std::string buffer;
    try {
        _outfile.open(getFilename(_scanner.getParameters(), buffer).c_str(), std::ofstream::trunc);
        if (!_outfile.is_open())
            throw resolvable_error("Unable to open/create file %s.", buffer.c_str());
        // Write the parameter settings that generated this collection of log likelihood values.
        _outfile << getSequentialParametersString(scanRunner.getParameters(), buffer).c_str() << std::endl;

        _csvWriter.reset(new CSVDataFileWriter(_outfile, _dataFieldDefinitions, true, false));
    } catch (prg_exception& x) {
        x.addTrace("constructor()","SequentialScanLoglikelihoodRatioWriter");
        throw;
    }
}

/** Gets the expected filename or user specified name -- if provided. */
std::string& SequentialScanLoglikelihoodRatioWriter::getFilename(const Parameters& parameters, std::string& buffer) {
    if (parameters.getSequentialFilename().size())
        buffer = parameters.getSequentialFilename(); 
    else 
        getDerivedFilename(parameters.getOutputFileName(), SequentialScanLoglikelihoodRatioWriter::SEQUENTIAL_FILE_SUFFIX, CSVDataFileWriter::CSV_FILE_EXT, buffer);
    return buffer;
}

SequentialScanLoglikelihoodRatioWriter::ParametersList_t & SequentialScanLoglikelihoodRatioWriter::getSequentialParametersList(SequentialScanLoglikelihoodRatioWriter::ParametersList_t& parameterslist) {
    parameterslist.clear();

    parameterslist.push_back(Parameters::SCAN_TYPE);
    parameterslist.push_back(Parameters::CONDITIONAL_TYPE);
    parameterslist.push_back(Parameters::MODEL_TYPE);

    parameterslist.push_back(Parameters::DATA_TIME_RANGES);
    parameterslist.push_back(Parameters::START_DATA_TIME_RANGE);
    parameterslist.push_back(Parameters::END_DATA_TIME_RANGE);

    parameterslist.push_back(Parameters::MAXIMUM_WINDOW_PERCENTAGE);
    parameterslist.push_back(Parameters::MAXIMUM_WINDOW_FIXED);
    parameterslist.push_back(Parameters::MAXIMUM_WINDOW_TYPE);
    parameterslist.push_back(Parameters::MINIMUM_WINDOW_FIXED);

    parameterslist.push_back(Parameters::REPLICATIONS);

    parameterslist.push_back(Parameters::SEQUENTIAL_MAX_SIGNAL);
    parameterslist.push_back(Parameters::SEQUENTIAL_MIN_SIGNAL);

    return parameterslist;
}

std::string & SequentialScanLoglikelihoodRatioWriter::getSequentialParametersString(const Parameters& parameters, std::string& buffer) {
    std::stringstream s;
    ParametersList_t parameterlist;
    PrintNull printNull;

    try {
        IniParameterFileAccess access(const_cast<Parameters&>(parameters), printNull);
        getSequentialParametersList(parameterlist);

        s << "settings=";
        for (ParametersList_t::const_iterator itr=parameterlist.begin(); itr != parameterlist.end(); ++itr) {
            if (itr != parameterlist.begin()) s << ";";
            s << access.GetParameterString(*itr, buffer).c_str();
        }
        buffer = s.str();
    } catch (prg_exception& x) {
        x.addTrace("getSequentialParametersString()","SequentialScanLoglikelihoodRatioWriter");
        throw;
    }
    return buffer;
}

void SequentialScanLoglikelihoodRatioWriter::write(double llr, boost::mutex& mutex) const {
    boost::mutex::scoped_lock lock(mutex);
    LoglikelihoodRatioWriter::write(llr);
}
