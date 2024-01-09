//******************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//******************************************************************************
#include "DataFileWriter.h"
#include "PrjException.h"
#include "ScanRunner.h"
#include "IniParameterFileAccess.h"

/** Class constructor */
RecordBuffer::RecordBuffer(const ptr_vector<FieldDef>& vFields) : vFieldDefinitions(vFields) {
  for (size_t t=0; t < vFieldDefinitions.size(); ++t) {
     gvFieldValues.push_back(FieldValue(vFieldDefinitions[t]->GetType()));
     gvBlankFields.push_back(true);
  }
}

/** Class destructor */
RecordBuffer::~RecordBuffer() {}

/** FieldDef definition for field with name */
const FieldDef & RecordBuffer::GetFieldDefinition(const std::string& sFieldName) const {
  return *vFieldDefinitions[GetFieldIndex(sFieldName)];
}

/** FieldDef definition for field at index */
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

/** Returns whether or not the field at iFieldNumber should be blank.
pre : none
post : returns true if field should be blank */
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
   if (!bFound) throw prg_error("Field name %s not found in the field vector.", "GetFieldIndex()", sFieldName.c_str());
  }
  catch (prg_exception& x) {
    x.addTrace("GetFieldIndex()","RecordBuffer");
    throw;
  }
  return iPosition;
}

unsigned int RecordBuffer::GetFieldIndex(std::initializer_list<std::string> fieldNames) const {
    try {
        for (auto& filename : fieldNames) {
            for (size_t i=0; i < vFieldDefinitions.size(); ++i) {
                if (!strcmp(vFieldDefinitions[i]->GetName(), filename.c_str())) return static_cast<unsigned int>(i);
            }
        }
        throw prg_error("Field name not found in the field vector.", "GetFieldIndex()");
    } catch (prg_exception& x) {
        x.addTrace("GetFieldIndex()", "RecordBuffer");
        throw;
    }
    return 0;
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

        // write column headers when requested
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

/** Creates a formatted string for field value suitable for use in csv string. */
std::string& CSVDataFileWriter::encodeForCSV(std::string& sValue, const FieldDef& FieldDef, const FieldValue& fv) {
    std::string temp;
    switch(fv.GetType()) {
        case FieldValue::ALPHA_FLD : {
                for (size_t pos=0; pos != fv.AsString().size(); ++pos) {
                    switch (fv.AsString()[pos]) {
                        case '\"': temp.append("\"\""); break;
                        default: temp.append(&fv.AsString()[pos], 1); break;
                    }
                }
                if (temp.find(",") != std::string::npos) {
                    printString(sValue, "\"%s\"", temp.c_str());
                } else sValue = temp;
            } break;
        case FieldValue::NUMBER_FLD:
            sValue = getValueAsString(fv.AsDouble(), temp, FieldDef.GetAsciiDecimals());
            break;
        default : throw prg_error("Unsupported field type %c", "Error!", fv.GetType());
    }
    return sValue;
}

/** Writes record buffer to file stream. */
void CSVDataFileWriter::writeRecord(const RecordBuffer& Record) {
    std::string value;
    for (unsigned int j=0; j < Record.GetNumFields(); ++j) {
        value.clear();
        if (!Record.GetFieldIsBlank(j))
            encodeForCSV(value, Record.GetFieldDefinition(j), Record.GetFieldValue(j));
        _outfile << value;
        if (j < Record.GetNumFields() - 1) _outfile << ",";
    }
    _outfile << std::endl;
}


////////////////////////////////////////////////////////////

const char * DataRecordWriter::CUT_NUM_FIELD                             = "Cut No.";
const char * DataRecordWriter::NODE_ID_FIELD                             = "Node Identifier";
const char * DataRecordWriter::NODE_NAME_FIELD                           = "Node Name";
const char * DataRecordWriter::NODE_OBSERVATIONS_FIELD                   = "Node Observations";
const char * DataRecordWriter::NODE_CASES_FIELD                          = "Node Cases";
const char * DataRecordWriter::START_WINDOW_FIELD                        = "Time Window Start";
const char * DataRecordWriter::END_WINDOW_FIELD                          = "Time Window End";
const char * DataRecordWriter::OBSERVATIONS_FIELD                        = "Observations";

const char * DataRecordWriter::WNDW_OBSERVED_FIELD                       = "Observations in Window";
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
const char * DataRecordWriter::RECURR_FLD                                = "Recurrence Interval";
const char * DataRecordWriter::P_LEVEL_FLD                               = "Tree Level";
const char * DataRecordWriter::BRANCH_ORDER_FLD                          = "Branch Order";
const char * DataRecordWriter::PARENT_NODE_FLD                           = "Parent Node";
const char * DataRecordWriter::PARENT_NODE_NAME_FLD                      = "Parent Node Name";
const char * DataRecordWriter::SIGNALLED_FLD                             = "Signalled";
const size_t DataRecordWriter::DEFAULT_LOC_FIELD_SIZE                    = 30;
const size_t DataRecordWriter::MAX_LOC_FIELD_SIZE                        = 254;

const char * DataRecordWriter::HYPOTHESIS_ALTERNATIVE_NUM_FIELD          = "Alternative Hypothesis";
const char * DataRecordWriter::HA_ALPHA05_FIELD                          = "Alpha 0.05";
const char * DataRecordWriter::HA_ALPHA01_FIELD                          = "Alpha 0.01";
const char * DataRecordWriter::HA_ALPHA001_FIELD                         = "Alpha 0.001";

///////////// CutsRecordWriter ///////////////////////////////////

const char * CutsRecordWriter::CUT_FILE_SUFFIX                            = "";

CutsRecordWriter::CutsRecordWriter(const ScanRunner& scanRunner) : _scanner(scanRunner) {
  std::string    buffer;
  const Parameters& params = _scanner.getParameters();

  try {
     getFieldDefs(_dataFieldDefinitions, params, _scanner.hasNodeDescriptions());
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

ptr_vector<FieldDef>& CutsRecordWriter::getFieldDefs(ptr_vector<FieldDef>& fields, const Parameters& params, bool hasNodeNames) {
    unsigned short uwOffset = 0;
    std::string    buffer;

    CreateField(fields, CUT_NUM_FIELD, FieldValue::ALPHA_FLD, 10, 0, uwOffset, 0);
    CreateField(fields, NODE_ID_FIELD, FieldValue::ALPHA_FLD, static_cast<short>(getLocationIdentiferFieldLength()), 0, uwOffset, 0);
    if (hasNodeNames) CreateField(fields, NODE_NAME_FIELD, FieldValue::ALPHA_FLD, static_cast<short>(getLocationIdentiferFieldLength()), 0, uwOffset, 0);
    if (params.getScanType() != Parameters::TIMEONLY) {
        CreateField(fields, P_LEVEL_FLD, FieldValue::NUMBER_FLD, 19, 0, uwOffset, 0);
    }
    switch (params.getModelType()) {
        case Parameters::BERNOULLI_TIME:
            if (params.getScanType() != Parameters::TIMEONLY) {
                CreateField(fields, NODE_OBSERVATIONS_FIELD, FieldValue::NUMBER_FLD, 19, 0, uwOffset, 0);
                CreateField(fields, NODE_CASES_FIELD, FieldValue::NUMBER_FLD, 19, 0, uwOffset, 0);
            }
            if (params.getDatePrecisionType() == DataTimeRange::GENERIC) {
                CreateField(fields, START_WINDOW_FIELD, FieldValue::NUMBER_FLD, 19, 0, uwOffset, 0);
                CreateField(fields, END_WINDOW_FIELD, FieldValue::NUMBER_FLD, 19, 0, uwOffset, 0);
            } else {
                CreateField(fields, START_WINDOW_FIELD, FieldValue::ALPHA_FLD, 10, 0, uwOffset, 0);
                CreateField(fields, END_WINDOW_FIELD, FieldValue::ALPHA_FLD, 10, 0, uwOffset, 0);
            }
        case Parameters::BERNOULLI_TREE:
            CreateField(fields, OBSERVATIONS_FIELD, FieldValue::NUMBER_FLD, 19, 0, uwOffset, 0);
            CreateField(fields, CASES_FIELD, FieldValue::NUMBER_FLD, 19, 0, uwOffset, 0);
            CreateField(fields, EXPECTED_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
            break;
        case Parameters::POISSON:
            CreateField(fields, OBSERVED_CASES_FIELD, FieldValue::NUMBER_FLD, 19, 0, uwOffset, 0);
            CreateField(fields, EXPECTED_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
            break;
        case Parameters::UNIFORM:
        case Parameters::MODEL_NOT_APPLICABLE:
        default:
            if (Parameters::isTemporalScanType(params.getScanType())) {
                CreateField(fields, NODE_CASES_FIELD, FieldValue::NUMBER_FLD, 19, 0, uwOffset, 0);
                if (params.getDatePrecisionType() == DataTimeRange::GENERIC) {
                    CreateField(fields, START_WINDOW_FIELD, FieldValue::NUMBER_FLD, 19, 0, uwOffset, 0);
                    CreateField(fields, END_WINDOW_FIELD, FieldValue::NUMBER_FLD, 19, 0, uwOffset, 0);
                } else {
                    CreateField(fields, START_WINDOW_FIELD, FieldValue::ALPHA_FLD, 10, 0, uwOffset, 0);
                    CreateField(fields, END_WINDOW_FIELD, FieldValue::ALPHA_FLD, 10, 0, uwOffset, 0);
                }
                CreateField(fields, WNDW_CASES_FIELD, FieldValue::NUMBER_FLD, 19, 0, uwOffset, 0);
                CreateField(fields, EXPECTED_CASES_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
            } else
                throw prg_error("Unknown model type (%d).", "CutsRecordWriter()", params.getModelType());
        }
        CreateField(fields, RELATIVE_RISK_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
        CreateField(fields, EXCESS_CASES_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
        if (params.getReportAttributableRisk()) {
            CreateField(fields, ATTRIBUTABLE_RISK_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 2);
        }
        if ((params.getScanType() == Parameters::TREETIME && params.getConditionalType() == Parameters::NODEANDTIME) ||
            (params.getScanType() == Parameters::TIMEONLY && params.getConditionalType() == Parameters::TOTALCASES && params.isPerformingDayOfWeekAdjustment()) ||
            (params.getScanType() == Parameters::TREETIME && params.getConditionalType() == Parameters::NODE && params.isPerformingDayOfWeekAdjustment())) {
            // If we stick with Poisson log-likelihood calculation, then label is 'Test Statistic' in place of 'Log Likelihood Ratio', hyper-geometric is 'Log Likelihood Ratio'.
            CreateField(fields, TEST_STATISTIC_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 6);
            // CreateField(_dataFieldDefinitions, LOG_LIKL_RATIO_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 6);
        } else {
            CreateField(fields, LOG_LIKL_RATIO_FIELD, FieldValue::NUMBER_FLD, 19, 10, uwOffset, 6);
        }
        if (!params.isSequentialScanTreeOnly()) {
            printString(buffer, "%u", params.getNumReplicationsRequested());
            CreateField(fields, P_VALUE_FLD, FieldValue::NUMBER_FLD, 19, 17/*std::min(17,(int)buffer.size())*/, uwOffset, static_cast<unsigned short>(buffer.size()));
            if (params.getIsProspectiveAnalysis())
                CreateField(fields, RECURR_FLD, FieldValue::NUMBER_FLD, 19, 0, uwOffset, 0);
        }
        if (params.getScanType() != Parameters::TIMEONLY) {
            CreateField(fields, PARENT_NODE_FLD, FieldValue::ALPHA_FLD, static_cast<short>(getLocationIdentiferFieldLength()) * 10/*multiple? guessing*/, 0, uwOffset, 0);
            if (hasNodeNames) CreateField(fields, PARENT_NODE_NAME_FLD, FieldValue::ALPHA_FLD, static_cast<short>(getLocationIdentiferFieldLength()) * 10/*multiple? guessing*/, 0, uwOffset, 0);
            CreateField(fields, BRANCH_ORDER_FLD, FieldValue::NUMBER_FLD, 19, 0, uwOffset, 0);
        }
        if (params.isSequentialScanTreeOnly())
            CreateField(fields, SIGNALLED_FLD, FieldValue::NUMBER_FLD, 19, 0, uwOffset, 0);

        return fields;
}

std::string& CutsRecordWriter::getFilename(const Parameters& parameters, std::string& buffer) {
    std::stringstream suffix;
    suffix << CutsRecordWriter::CUT_FILE_SUFFIX;
    return getDerivedFilename(parameters.getOutputFileName(), suffix.str().c_str(), CSVDataFileWriter::CSV_FILE_EXT, buffer);
}

void CutsRecordWriter::write(const CutStructure& thisCut) const {
    std::string buffer;

    try {
        RecordBuffer Record(_dataFieldDefinitions);
        _csvWriter->writeRecord(getRecordForCut(Record, thisCut, _scanner));
        // Now write records for each direct child of this cut/node.
        std::vector<boost::shared_ptr<RecordBuffer> > childRecords;
        unsigned int countFieldIdx = Record.GetFieldIndex({ std::string(CASES_FIELD), std::string(OBSERVED_CASES_FIELD), std::string(WNDW_CASES_FIELD) });
        for (auto pnode : _scanner.getCutChildNodes(thisCut)) {
            boost::shared_ptr<RecordBuffer> record(new RecordBuffer(_dataFieldDefinitions));
            getRecordForCutChild(*(record), thisCut, *pnode, thisCut.getReportOrder(), _scanner);
            if (record->GetFieldValue(countFieldIdx).AsDouble() > 0.0 && !std::isnan(record->GetFieldValue(EXCESS_CASES_FIELD).AsDouble()))
                childRecords.push_back(record);
        }
        std::sort(std::begin(childRecords), std::end(childRecords), [](boost::shared_ptr<RecordBuffer> recordA, boost::shared_ptr<RecordBuffer> recordB) {
            return recordA->GetFieldValue(EXCESS_CASES_FIELD).AsDouble() > recordB->GetFieldValue(EXCESS_CASES_FIELD).AsDouble();
        });
        unsigned int writeIdx = 0;
        for (auto& record : childRecords) {
            record->GetFieldValue(CUT_NUM_FIELD).AsString() = printString(buffer, "%u_%u", thisCut.getReportOrder(), ++writeIdx);
            _csvWriter->writeRecord(*record);
        }
    } catch (prg_exception& x) {
        x.addTrace("write()","CutsRecordWriter");
        throw;
    }
}

/** Populates the RecordBuffer for NodeStructure of this CutStructure per analysis settings. */
RecordBuffer& CutsRecordWriter::getRecordForCut(RecordBuffer& Record, const CutStructure& thisCut, const ScanRunner& scanner) {
    const Parameters& params = scanner.getParameters();
    std::string buffer;
    Loglikelihood_t calcLogLikelihood(AbstractLoglikelihood::getNewLoglikelihood(scanner.getParameters(), scanner.getTotalC(), scanner.getTotalN(), scanner.isCensoredData()));
    const NodeStructure& cutNode = *(scanner.getNodes()[thisCut.getID()]);

    try {
        Record.GetFieldValue(CUT_NUM_FIELD).AsString() = printString(buffer, "%u", thisCut.getReportOrder());
        Record.GetFieldValue(NODE_ID_FIELD).AsString() = cutNode.getIdentifier();
        if (scanner.hasNodeDescriptions()) Record.GetFieldValue(NODE_NAME_FIELD).AsString() = cutNode.getName();
        if (params.getScanType() != Parameters::TIMEONLY)
            Record.GetFieldValue(P_LEVEL_FLD).AsDouble() = static_cast<int>(cutNode.getLevel());
        switch (params.getModelType()) {
            case Parameters::BERNOULLI_TIME:
                if (params.getScanType() != Parameters::TIMEONLY) {
                    Record.GetFieldValue(NODE_OBSERVATIONS_FIELD).AsDouble() = static_cast<int>(cutNode.getBrN());
                    Record.GetFieldValue(NODE_CASES_FIELD).AsDouble() = static_cast<int>(cutNode.getBrC());
                }
                if (params.getDatePrecisionType() == DataTimeRange::GENERIC) {
                    Record.GetFieldValue(START_WINDOW_FIELD).AsDouble() = thisCut.getStartIdx() - scanner.getZeroTranslationAdditive();
                    Record.GetFieldValue(END_WINDOW_FIELD).AsDouble() = thisCut.getEndIdx() - scanner.getZeroTranslationAdditive();
                }
                else {
                    const DataTimeRange& range = params.getDataTimeRangeSet().getDataTimeRangeSets().front();
                    std::pair<std::string, std::string> rangeDates = range.rangeToGregorianStrings(
                        thisCut.getStartIdx() - scanner.getZeroTranslationAdditive(),
                        thisCut.getEndIdx() - scanner.getZeroTranslationAdditive(),
                        params.getDatePrecisionType()
                    );
                    Record.GetFieldValue(START_WINDOW_FIELD).AsString() = rangeDates.first;
                    Record.GetFieldValue(END_WINDOW_FIELD).AsString() = rangeDates.second;
                }
            case Parameters::BERNOULLI_TREE:
                Record.GetFieldValue(OBSERVATIONS_FIELD).AsDouble() = static_cast<int>(thisCut.getN());
                Record.GetFieldValue(CASES_FIELD).AsDouble() = static_cast<int>(thisCut.getC());
                Record.GetFieldValue(EXPECTED_FIELD).AsDouble() = thisCut.getExpected(scanner);
                break;
            case Parameters::POISSON:
                Record.GetFieldValue(OBSERVED_CASES_FIELD).AsDouble() = thisCut.getC();
                Record.GetFieldValue(EXPECTED_FIELD).AsDouble() = thisCut.getExpected(scanner);
                break;
            case Parameters::UNIFORM:
            case Parameters::MODEL_NOT_APPLICABLE:
            default:
                if (Parameters::isTemporalScanType(params.getScanType())) {
                    if (params.isPerformingDayOfWeekAdjustment() || params.getConditionalType() == Parameters::NODEANDTIME || scanner.isCensoredData())
                        Record.GetFieldValue(NODE_CASES_FIELD).AsDouble() = static_cast<int>(cutNode.getBrC());
                    else
                        Record.GetFieldValue(NODE_CASES_FIELD).AsDouble() = static_cast<int>(thisCut.getN());
                    if (params.getDatePrecisionType() == DataTimeRange::GENERIC) {
                        Record.GetFieldValue(START_WINDOW_FIELD).AsDouble() = thisCut.getStartIdx() - scanner.getZeroTranslationAdditive();
                        Record.GetFieldValue(END_WINDOW_FIELD).AsDouble() = thisCut.getEndIdx() - scanner.getZeroTranslationAdditive();
                    }
                    else {
                        const DataTimeRange& range = params.getDataTimeRangeSet().getDataTimeRangeSets().front();
                        std::pair<std::string, std::string> rangeDates = range.rangeToGregorianStrings(
                            thisCut.getStartIdx() - scanner.getZeroTranslationAdditive(),
                            thisCut.getEndIdx() - scanner.getZeroTranslationAdditive(),
                            params.getDatePrecisionType()
                        );
                        Record.GetFieldValue(START_WINDOW_FIELD).AsString() = rangeDates.first;
                        Record.GetFieldValue(END_WINDOW_FIELD).AsString() = rangeDates.second;
                    }
                    Record.GetFieldValue(WNDW_CASES_FIELD).AsDouble() = static_cast<int>(thisCut.getC());
                    Record.GetFieldValue(EXPECTED_CASES_FIELD).AsDouble() = thisCut.getExpected(scanner);
                }
                else
                    throw prg_error("Unknown model type (%d).", "CutsRecordWriter()", params.getModelType());
        }
        Record.GetFieldValue(RELATIVE_RISK_FIELD).AsDouble() = thisCut.getRelativeRisk(scanner);
        Record.GetFieldValue(EXCESS_CASES_FIELD).AsDouble() = thisCut.getExcessCases(scanner);
        if (params.getReportAttributableRisk())
            Record.GetFieldValue(ATTRIBUTABLE_RISK_FIELD).AsDouble() = thisCut.getAttributableRisk(scanner);
        if ((params.getScanType() == Parameters::TREETIME && params.getConditionalType() == Parameters::NODEANDTIME) ||
            (params.getScanType() == Parameters::TIMEONLY && params.getConditionalType() == Parameters::TOTALCASES && params.isPerformingDayOfWeekAdjustment()) ||
            (params.getScanType() == Parameters::TREETIME && params.getConditionalType() == Parameters::NODE && params.isPerformingDayOfWeekAdjustment())) {
            // If we stick with Poisson log-likelihood calculation, then label is 'Test Statistic' in place of 'Log Likelihood Ratio', hyper-geometric is 'Log Likelihood Ratio'.
            Record.GetFieldValue(TEST_STATISTIC_FIELD).AsDouble() = calcLogLikelihood->LogLikelihoodRatio(thisCut.getLogLikelihood());
        } else
            Record.GetFieldValue(LOG_LIKL_RATIO_FIELD).AsDouble() = calcLogLikelihood->LogLikelihoodRatio(thisCut.getLogLikelihood());
        if (scanner.reportablePValue(thisCut)) {
            Record.GetFieldValue(P_VALUE_FLD).AsDouble() = thisCut.getPValue(scanner);
            if (params.getIsProspectiveAnalysis())
                Record.GetFieldValue(RECURR_FLD).AsDouble() = scanner.getRecurrenceInterval(thisCut).second;
        }
        if (params.getScanType() != Parameters::TIMEONLY) {
            cutNode.getParentIndentifiers(Record.GetFieldValue(PARENT_NODE_FLD).AsString(), true);
            if (scanner.hasNodeDescriptions()) {
                // If defining node names and parent node names are different than parent node IDs.
                cutNode.getParentIndentifiers(buffer, false);
                if (buffer != Record.GetFieldValue(PARENT_NODE_FLD).AsString())
                    Record.GetFieldValue(PARENT_NODE_NAME_FLD).AsString() = buffer;
            }
            Record.GetFieldValue(BRANCH_ORDER_FLD).AsDouble() = thisCut.getBranchOrder();
        }
        if (params.isSequentialScanTreeOnly()) {
            unsigned int signalInLook = scanner.getSequentialStatistic().testCutSignaled(static_cast<size_t>(thisCut.getID()));
            if (signalInLook != 0)
                Record.GetFieldValue(SIGNALLED_FLD).AsDouble() = signalInLook;
        }
    } catch (prg_exception& x) {
        x.addTrace("getRecordForCut()", "CutsRecordWriter");
        throw;
    }
    return Record;
}

/** Populates the RecordBuffer for child NodeStructure of this CutStructure per analysis settings. */
RecordBuffer& CutsRecordWriter::getRecordForCutChild(RecordBuffer& Record, const CutStructure& thisCut, const NodeStructure& childNode, size_t subIndex, const ScanRunner& scanner) {
    std::string buffer;
    const Parameters& params = scanner.getParameters();

    Record.GetFieldValue(CUT_NUM_FIELD).AsString() = printString(buffer, "%u_%u", thisCut.getReportOrder(), subIndex);
    Record.GetFieldValue(NODE_ID_FIELD).AsString() = childNode.getIdentifier();
    if (scanner.hasNodeDescriptions()) Record.GetFieldValue(NODE_NAME_FIELD).AsString() = childNode.getName();
    if (params.getScanType() != Parameters::TIMEONLY) {
        Record.GetFieldValue(P_LEVEL_FLD).AsDouble() = static_cast<int>(childNode.getLevel());
        childNode.getParentIndentifiers(Record.GetFieldValue(PARENT_NODE_FLD).AsString(), true);
        if (scanner.hasNodeDescriptions()) {
            // If defining node names and parent node names are different than parent node IDs.
            childNode.getParentIndentifiers(buffer, false);
            if (buffer != Record.GetFieldValue(PARENT_NODE_FLD).AsString())
                childNode.getParentIndentifiers(Record.GetFieldValue(PARENT_NODE_NAME_FLD).AsString(), false);
        }
    }
    // Calculate case and expected based on analysis type - this mirrors the scan process.
    int _C;
    double _N;
    if ((params.getScanType() == Parameters::TREETIME && params.getConditionalType() == Parameters::NODEANDTIME) ||
        (params.getScanType() == Parameters::TIMEONLY && params.isPerformingDayOfWeekAdjustment()) ||
        (params.getScanType() == Parameters::TREETIME && params.getConditionalType() == Parameters::NODE && params.isPerformingDayOfWeekAdjustment())) {
        _C = childNode.getBrC_C()[thisCut.getStartIdx()] - childNode.getBrC_C()[thisCut.getEndIdx() + 1];
        _N = childNode.getBrN_C()[thisCut.getStartIdx()] - childNode.getBrN_C()[thisCut.getEndIdx() + 1];
    } else if (params.getModelType() == Parameters::UNIFORM) {
        if (scanner.isCensoredData()) {
            _C = childNode.getBrC_C()[thisCut.getStartIdx()] - childNode.getBrC_C()[thisCut.getEndIdx() + 1];
            _N = childNode.getBrN_C()[thisCut.getStartIdx()] - childNode.getBrN_C()[thisCut.getEndIdx() + 1];
        } else {
            _C = childNode.getBrC_C()[thisCut.getStartIdx()] - childNode.getBrC_C()[thisCut.getEndIdx() + 1],
            _N = static_cast<NodeStructure::expected_t>(childNode.getBrC());
        }
    } else if (params.getModelType() == Parameters::BERNOULLI_TIME) {
        _C = childNode.getBrC_C()[thisCut.getStartIdx()] - childNode.getBrC_C()[thisCut.getEndIdx() + 1];
        _N = childNode.getBrN_C()[thisCut.getStartIdx()] - childNode.getBrN_C()[thisCut.getEndIdx() + 1];
    } else {
        _C = childNode.getBrC();
        _N = childNode.getBrN();
    }
    switch (params.getModelType()) {
        case Parameters::BERNOULLI_TIME:
            if (params.getScanType() != Parameters::TIMEONLY) {
                Record.GetFieldValue(NODE_OBSERVATIONS_FIELD).AsDouble() = static_cast<int>(childNode.getBrN());
                Record.GetFieldValue(NODE_CASES_FIELD).AsDouble() = static_cast<int>(childNode.getBrC());
            }
            if (params.getDatePrecisionType() == DataTimeRange::GENERIC) {
                Record.GetFieldValue(START_WINDOW_FIELD).AsDouble() = thisCut.getStartIdx() - scanner.getZeroTranslationAdditive();
                Record.GetFieldValue(END_WINDOW_FIELD).AsDouble() = thisCut.getEndIdx() - scanner.getZeroTranslationAdditive();
            } else {
                const DataTimeRange& range = params.getDataTimeRangeSet().getDataTimeRangeSets().front();
                std::pair<std::string, std::string> rangeDates = range.rangeToGregorianStrings(
                    thisCut.getStartIdx() - scanner.getZeroTranslationAdditive(),
                    thisCut.getEndIdx() - scanner.getZeroTranslationAdditive(),
                    params.getDatePrecisionType()
                );
                Record.GetFieldValue(START_WINDOW_FIELD).AsString() = rangeDates.first;
                Record.GetFieldValue(END_WINDOW_FIELD).AsString() = rangeDates.second;
            }
        case Parameters::BERNOULLI_TREE:
            Record.GetFieldValue(OBSERVATIONS_FIELD).AsDouble() = static_cast<int>(_N);
            Record.GetFieldValue(CASES_FIELD).AsDouble() = static_cast<int>(_C);
            Record.GetFieldValue(EXPECTED_FIELD).AsDouble() = getExpectedFor(scanner, childNode.getID(), _C, _N, thisCut.getStartIdx(), thisCut.getEndIdx());
            break;
        case Parameters::POISSON:
            Record.GetFieldValue(OBSERVED_CASES_FIELD).AsDouble() = _C;
            Record.GetFieldValue(EXPECTED_FIELD).AsDouble() = getExpectedFor(scanner, childNode.getID(), _C, _N, thisCut.getStartIdx(), thisCut.getEndIdx());
            break;
        case Parameters::UNIFORM:
        case Parameters::MODEL_NOT_APPLICABLE:
        default:
            if (Parameters::isTemporalScanType(params.getScanType())) {
                if (params.isPerformingDayOfWeekAdjustment() || params.getConditionalType() == Parameters::NODEANDTIME || scanner.isCensoredData()) {
                    Record.GetFieldValue(NODE_CASES_FIELD).AsDouble() = static_cast<int>(childNode.getBrC());
                } else {
                    Record.GetFieldValue(NODE_CASES_FIELD).AsDouble() = static_cast<int>(_N);
                }
                if (params.getDatePrecisionType() == DataTimeRange::GENERIC) {
                    Record.GetFieldValue(START_WINDOW_FIELD).AsDouble() = thisCut.getStartIdx() - scanner.getZeroTranslationAdditive();
                    Record.GetFieldValue(END_WINDOW_FIELD).AsDouble() = thisCut.getEndIdx() - scanner.getZeroTranslationAdditive();
                } else {
                    const DataTimeRange& range = params.getDataTimeRangeSet().getDataTimeRangeSets().front();
                    std::pair<std::string, std::string> rangeDates = range.rangeToGregorianStrings(
                        thisCut.getStartIdx() - scanner.getZeroTranslationAdditive(),
                        thisCut.getEndIdx() - scanner.getZeroTranslationAdditive(),
                        params.getDatePrecisionType()
                    );
                    Record.GetFieldValue(START_WINDOW_FIELD).AsString() = rangeDates.first;
                    Record.GetFieldValue(END_WINDOW_FIELD).AsString() = rangeDates.second;
                }
                Record.GetFieldValue(WNDW_CASES_FIELD).AsDouble() = static_cast<int>(_C);
                Record.GetFieldValue(EXPECTED_CASES_FIELD).AsDouble() = getExpectedFor(scanner, childNode.getID(), _C, _N, thisCut.getStartIdx(), thisCut.getEndIdx());
            }
            else
                throw prg_error("Unknown model type (%d).", "CutsRecordWriter()", params.getModelType());
    }

    if (_C > 0) { // In terms of child nodes, we're only interested if there are cases in them, so shortcut these calculations (which don't always play nice here).
        Record.GetFieldValue(RELATIVE_RISK_FIELD).AsDouble() = getRelativeRiskFor(scanner, childNode.getID(), _C, _N, thisCut.getStartIdx(), thisCut.getEndIdx());
        Record.GetFieldValue(EXCESS_CASES_FIELD).AsDouble() = getExcessCasesFor(scanner, childNode.getID(), _C, _N, thisCut.getStartIdx(), thisCut.getEndIdx());
        if (params.getReportAttributableRisk()) {
            Record.GetFieldValue(ATTRIBUTABLE_RISK_FIELD).AsDouble() = getAttributableRiskFor(scanner, childNode.getID(), _C, _N, thisCut.getStartIdx(), thisCut.getEndIdx());
        }
    }
    return Record;
}

///////////// PowerEstimationRecordWriter ///////////////////////////////////

const char * PowerEstimationRecordWriter::POWER_FILE_SUFFIX                            = "_power";

/** Constructor: defines data filefields and allocates csv data file writer. */
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

/** Returns the filename to which alternative hypotheses are written. The filename is a derivative of the main results filename. */
std::string& PowerEstimationRecordWriter::getFilename(const Parameters& parameters, std::string& buffer) {
    return getDerivedFilename(parameters.getOutputFileName(), PowerEstimationRecordWriter::POWER_FILE_SUFFIX, CSVDataFileWriter::CSV_FILE_EXT, buffer);
}

/** Writes power estimation values for all alternative hypothesis iterations to csv file. */
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
const char * LoglikelihoodRatioWriter::SIMULATION_IDX_FIELD  = "SIMIDX";
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

LoglikelihoodRatioWriter::LoglikelihoodRatioWriter(const ScanRunner& scanRunner, bool ispower, bool append, bool includeIdx) : 
    _scanner(scanRunner), _include_sim_idx(includeIdx){
  unsigned short uwOffset=0;
  std::string buffer;
  try {
      if (_include_sim_idx)
          CreateField(_dataFieldDefinitions, SIMULATION_IDX_FIELD, FieldValue::NUMBER_FLD, 19, 0, uwOffset, 0);
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

void LoglikelihoodRatioWriter::write(double llr, unsigned int simulation) const {
    std::string  buffer;
    RecordBuffer Record(_dataFieldDefinitions);

    try {
        if (_include_sim_idx)
            Record.GetFieldValue(SIMULATION_IDX_FIELD).AsUnsignedLong() = simulation;
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
		_include_sim_idx = false;
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
    LoglikelihoodRatioWriter::write(llr, 0);
}
