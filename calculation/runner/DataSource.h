//******************************************************************************
#ifndef __DataSource_H
#define __DataSource_H
//******************************************************************************
#include <string>
#include <iostream>
#include <fstream>

/** Input data source abstraction. */
class DataSource {
   public:
     DataSource() {}
     virtual ~DataSource() {}

     virtual size_t                     getCurrentRecordIndex() const = 0;
     static DataSource                * getNewDataSourceObject(const std::string& sSourceFilename);
     virtual size_t                     getNumValues() = 0;
     virtual std::string              & getValueAt(long iFieldIndex) = 0;
     virtual void                       gotoFirstRecord() = 0;
     virtual bool                       readRecord() = 0;
};

/** CSV file data source. */
class CSVFileDataSource : public DataSource {
   private:
     size_t                             _readCount;
     std::ifstream                      _sourceFile;
     std::vector<std::string>           _read_values;

    void                                throwUnicodeException();

   public:
     CSVFileDataSource(const std::string& sSourceFilename);
     virtual ~CSVFileDataSource() {}

     virtual size_t                     getCurrentRecordIndex() const {return _readCount;}
     virtual size_t                     getNumValues() {return _read_values.size();}
     virtual std::string              & getValueAt(long iFieldIndex) {return _read_values.at(iFieldIndex);}
     virtual void                       gotoFirstRecord();
     virtual bool                       readRecord();
};
//******************************************************************************
#endif
