//******************************************************************************
#ifndef __AbstractDataFileWriter_H
#define __AbstractDataFileWriter_H
//******************************************************************************
#include "Parameters.h"
#include "FieldDef.h"
#include "ptr_vector.h"
#include <iostream>
#include <fstream>

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
    static const char         * OBSERVED_FIELD;
    static const char         * OBSERVED_NO_DUPLICATES_FIELD;
    static const char         * TOTAL_CASES_FIELD;
    static const char         * EXPECTED_FIELD;
    static const char         * OBSERVED_DIV_EXPECTED_FIELD;
    static const char         * RELATIVE_RISK_FIELD;
    static const char         * OBSERVED_DIV_EXPECTED_NO_DUPLICATES_FIELD;
    static const char         * START_WINDOW_FIELD;
    static const char         * END_WINDOW_FIELD;
    static const char         * LOG_LIKL_RATIO_FIELD;
    static const char         * P_VALUE_FLD;

  protected:
    ptr_vector<FieldDef>        _dataFieldDefinitions;       /** field definitions              */

    size_t                      getLocationIdentiferFieldLength() const {return DEFAULT_LOC_FIELD_SIZE;}

  public:
    DataRecordWriter() {}
    virtual ~DataRecordWriter() {}

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

   protected :
     std::ofstream              _outfile;

     void                       createFormatString(std::string& sValue, const FieldDef& FieldDef, const FieldValue& fv);

   public :
      CSVDataFileWriter(const std::string& filename, const ptr_vector<FieldDef>& vFieldDefs, bool printHeaders);
      virtual ~CSVDataFileWriter();

     virtual void	            writeRecord(const RecordBuffer& Record);
};

class ScanRunner; /* forward class declaration */
class CutsRecordWriter : public DataRecordWriter {
  public:
    static const char         * CUT_FILE_SUFFIX;

  private:
       const ScanRunner                  &  _scanner;
       std::auto_ptr<CSVDataFileWriter>     _csvWriter;

   public:
       CutsRecordWriter(const ScanRunner& scanRunner);
       virtual ~CutsRecordWriter() {}

       static std::string & getFilename(const Parameters& parameters, std::string& buffer);

       void                  write(unsigned int cutIndex) const;
};

//******************************************************************************
#endif

