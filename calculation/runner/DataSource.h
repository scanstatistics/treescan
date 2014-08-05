//******************************************************************************
#ifndef __DataSource_H
#define __DataSource_H
//******************************************************************************
#include <string>
#include <iostream>
#include <fstream>

/** Input data source abstraction. */
class DataSource {
    protected:
        bool _blank_record_flag;

    public:
        DataSource() : _blank_record_flag(false) {}
        virtual ~DataSource() {}

        void                               clearBlankRecordFlag() {_blank_record_flag=false;}
        bool                               detectBlankRecordFlag() const {return _blank_record_flag;}
        virtual size_t                     getCurrentRecordIndex() const = 0;
        static DataSource                * getNewDataSourceObject(const std::string& sSourceFilename);
        virtual size_t                     getNumValues() = 0;
        virtual std::string              & getValueAt(long iFieldIndex) = 0;
        virtual void                       gotoFirstRecord() = 0;
        virtual bool                       readRecord() = 0;
        void                               tripBlankRecordFlag() {_blank_record_flag=true;}
};

/** CSV file data source. */
class CSVFileDataSource : public DataSource {
    private:
        std::string _delimiter;
        std::string _group;
        size_t                    _readCount;
        std::ifstream             _sourceFile;
        std::vector<std::string>  _read_values;

        bool parse(const std::string& s, const std::string& delimiter=",", const std::string& grouper="\"");
        void throwUnicodeException();

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
