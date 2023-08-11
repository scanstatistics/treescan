//******************************************************************************
#ifndef __AbstractDataFileWriter_H
#define __AbstractDataFileWriter_H
//******************************************************************************
#include "Parameters.h"
#include "FieldDef.h"
#include "ptr_vector.h"
#include <iostream>
#include <fstream>
#include "boost/thread/mutex.hpp"

/** Collection of field values for buffering records of additional output files.*/
class RecordBuffer {
   private:
      const ptr_vector<FieldDef>      & vFieldDefinitions;  /** field definitions */
      std::vector<FieldValue>           gvFieldValues;      /** record buffer of field values */
      std::vector<bool>                 gvBlankFields;      /** indicators whether relative field is blank */

   public:
      RecordBuffer(const ptr_vector<FieldDef>& vFields);
      virtual ~RecordBuffer();

      const FieldDef                  & GetFieldDefinition(unsigned int iFieldIndex) const;
      const FieldDef                  & GetFieldDefinition(const std::string& sFieldName) const;
      unsigned int                      GetFieldIndex(const std::string& sFieldName) const;
      unsigned int                      GetFieldIndex(std::initializer_list<std::string> fieldNames) const;
      bool                              GetFieldIsBlank(unsigned int iFieldNumber) const;
      FieldValue                      & GetFieldValue(const std::string& sFieldName);
      FieldValue                      & GetFieldValue(unsigned int iFieldIndex);
      const FieldValue                & GetFieldValue(unsigned int iFieldIndex) const;
      size_t                            GetNumFields() const { return gvFieldValues.size();}
      void                              SetAllFieldsBlank(bool bBlank);
      void                              SetFieldIsBlank(const std::string& sFieldName, bool bBlank);
      void                              SetFieldIsBlank(unsigned int iFieldNumber, bool bBlank);
};

/** Base class for writing record based data to files. */
class DataRecordWriter {
  public:
    static const size_t         DEFAULT_LOC_FIELD_SIZE;
    static const size_t         MAX_LOC_FIELD_SIZE;
    static const char         * CUT_NUM_FIELD;
    static const char         * NODE_ID_FIELD;
    static const char         * NODE_NAME_FIELD;
    static const char         * NODE_OBSERVATIONS_FIELD;
    static const char         * NODE_CASES_FIELD;
    static const char         * START_WINDOW_FIELD;
    static const char         * END_WINDOW_FIELD;
    static const char         * OBSERVATIONS_FIELD;
    static const char         * WNDW_OBSERVED_FIELD;
    static const char         * WNDW_CASES_FIELD;
    static const char         * CASES_FIELD;
    static const char         * OBSERVED_CASES_FIELD;
    static const char         * EXPECTED_FIELD;
    static const char         * EXPECTED_CASES_FIELD;
    static const char         * RELATIVE_RISK_FIELD;
    static const char         * EXCESS_CASES_FIELD;
    static const char         * ATTRIBUTABLE_RISK_FIELD;
    static const char         * LOG_LIKL_RATIO_FIELD;
    static const char         * TEST_STATISTIC_FIELD;
    static const char         * P_VALUE_FLD;
    static const char         * RECURR_FLD;
    static const char         * P_LEVEL_FLD;
    static const char         * BRANCH_ORDER_FLD;
    static const char         * PARENT_NODE_FLD;
    static const char         * PARENT_NODE_NAME_FLD;
    static const char         * SIGNALLED_FLD;
    static const char         * HYPOTHESIS_ALTERNATIVE_NUM_FIELD;
    static const char         * HA_ALPHA05_FIELD;
    static const char         * HA_ALPHA01_FIELD;
    static const char         * HA_ALPHA001_FIELD;

  protected:
    ptr_vector<FieldDef>        _dataFieldDefinitions;       /** field definitions              */


  public:
    DataRecordWriter() {}
    virtual ~DataRecordWriter() {}

    static size_t               getLocationIdentiferFieldLength() { return DEFAULT_LOC_FIELD_SIZE; }

    static void                 CreateField(ptr_vector<FieldDef>& vFields, const std::string& sFieldName, char cType,
                                            short wLength, short wPrecision, unsigned short& uwOffset, 
                                            unsigned short uwAsciiDecimals, bool bCreateIndex=false) {
        vFields.push_back(new FieldDef(sFieldName.c_str(), cType, wLength, wPrecision, uwOffset, uwAsciiDecimals));
        uwOffset += wLength;
    }
};

/** CSV data writer. */
class CSVDataFileWriter {
   public:
     static const char        * CSV_FILE_EXT;

   protected:
     std::ofstream  & _outfile;

   public:
      CSVDataFileWriter(std::ofstream& outfile, const ptr_vector<FieldDef>& vFieldDefs, bool printHeaders, bool append=false);
      virtual ~CSVDataFileWriter() {}

      static std::string& encodeForCSV(std::string& sValue, const FieldDef& FieldDef, const FieldValue& fv);
      virtual void         writeRecord(const RecordBuffer& Record);
};

class ScanRunner; /* forward class declaration */
class CutStructure; /* forward class declaration */
class NodeStructure; /* forward class declaration */
class CutsRecordWriter : public DataRecordWriter {
  public:
    static const char         * CUT_FILE_SUFFIX;

  private:
       const ScanRunner & _scanner;
       std::ofstream _outfile;
       std::auto_ptr<CSVDataFileWriter> _csvWriter;

   public:
       CutsRecordWriter(const ScanRunner& scanRunner);
       virtual ~CutsRecordWriter();

       static std::string & getFilename(const Parameters& parameters, std::string& buffer);
       static ptr_vector<FieldDef>& getFieldDefs(ptr_vector<FieldDef>& fields, const Parameters& params, bool hasNodeNames);
       static RecordBuffer& getRecordForCut(RecordBuffer& Record, const CutStructure& thisCut, const ScanRunner& scanner);
       static RecordBuffer& getRecordForCutChild(RecordBuffer& Record, const CutStructure& thisCut, const NodeStructure& childNode, size_t subIndex, const ScanRunner& scanner);

       void                  write(const CutStructure& thisCut) const;
};

/* Data file writer for the power estimation. */
class PowerEstimationRecordWriter : public DataRecordWriter {
  public:
    static const char         * POWER_FILE_SUFFIX;

  private:
       const ScanRunner & _scanner;
       std::ofstream _outfile;
       std::auto_ptr<CSVDataFileWriter> _csvWriter;

   public:
       PowerEstimationRecordWriter(const ScanRunner& scanRunner);
       virtual ~PowerEstimationRecordWriter();

       static std::string & getFilename(const Parameters& parameters, std::string& buffer);

       void                  write() const;
};

/** Loglikelihood ratio data file writer. */
class LoglikelihoodRatioWriter : public DataRecordWriter {
    protected:
        static const char * LLR_FILE_SUFFIX;
        static const char * LLR_HA_FILE_SUFFIX;
        static const char * SIMULATION_IDX_FIELD;
        static const char * LOG_LIKL_RATIO_FIELD;

        const ScanRunner & _scanner;
        std::ofstream _outfile;
        std::auto_ptr<CSVDataFileWriter> _csvWriter;
        bool _include_sim_idx;

        LoglikelihoodRatioWriter(const ScanRunner& scanRunner);

    public:
        LoglikelihoodRatioWriter(const ScanRunner& scanRunner, bool ispower, bool append, bool includeIdx);
        virtual ~LoglikelihoodRatioWriter();

        static std::string & getFilename(const Parameters& parameters, std::string& buffer, bool ispower);

        void write(double llr, unsigned int simulation) const;
};

/** Sequential Scan - Loglikelihood ratio data file writer. */
class SequentialScanLoglikelihoodRatioWriter : public LoglikelihoodRatioWriter {
    public:
        typedef std::vector<Parameters::ParameterType> ParametersList_t;
        static const char * SEQUENTIAL_FILE_SUFFIX;

    protected:
        ParametersList_t _sequential_parameters;

    public:
        SequentialScanLoglikelihoodRatioWriter(const ScanRunner& scanRunner);
        virtual ~SequentialScanLoglikelihoodRatioWriter() {}

        static std::string & getFilename(const Parameters& parameters, std::string& buffer);
        static ParametersList_t & getSequentialParametersList(ParametersList_t& parameterslist);
        static std::string & getSequentialParametersString(const Parameters& parameters, std::string& buffer);

        void write(double llr, boost::mutex& mutex) const;
};
//******************************************************************************
#endif

