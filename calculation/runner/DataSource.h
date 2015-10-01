//******************************************************************************
#ifndef __DataSource_H
#define __DataSource_H
//******************************************************************************
#include <string>
#include <iostream>
#include <fstream>
#include "Parameters.h"

/** Input data source abstraction. */
class DataSource {
    protected:
        bool _blank_record_flag;
        FieldMapContainer_t _fields_map;

        virtual long tranlateFieldIndex(long idx) const {
            if (idx < static_cast<long>(_fields_map.size())) {
                return boost::any_cast<long>(_fields_map.at(static_cast<size_t>(idx))) - 1;
            }
            return idx;
        }

    public:
        DataSource() : _blank_record_flag(false) {}
        virtual ~DataSource() {}

        void                               clearBlankRecordFlag() {_blank_record_flag=false;}
        bool                               detectBlankRecordFlag() const {return _blank_record_flag;}
        virtual size_t                     getCurrentRecordIndex() const = 0;
        static DataSource                * getNewDataSourceObject(const std::string& sSourceFilename, const Parameters::InputSource * source);
        virtual size_t                     getNumValues() = 0;
        virtual std::string              & getValueAt(long iFieldIndex) = 0;
        virtual void                       gotoFirstRecord() = 0;
        virtual bool                       readRecord() = 0;
        void                               tripBlankRecordFlag() {_blank_record_flag=true;}
        void                               setFieldsMap(const std::vector<boost::any>& map) {_fields_map = map;}
};

/** CSV file data source. */
class CSVFileDataSource : public DataSource {
    private:
        std::string _delimiter;
        std::string _grouper;
        unsigned long _skip;
        bool _firstRowHeaders;
        size_t _readCount;
        std::ifstream _sourceFile;
        std::vector<std::string> _read_values;
        bool _ignore_empty_fields;
        std::string _read_buffer;

        bool parse(const std::string& s, const std::string& delimiter=",", const std::string& grouper="\"");
        void throwUnicodeException();

    public:
        CSVFileDataSource(const std::string& sSourceFilename, const std::string& delimiter=",", const std::string& grouper="\"", unsigned long skip=0, bool firstRowHeaders=false);
        virtual ~CSVFileDataSource() {}

        virtual size_t                     getCurrentRecordIndex() const {return _readCount;}
        virtual size_t                     getNumValues();
        virtual std::string              & getValueAt(long iFieldIndex);
        virtual void                       gotoFirstRecord();
        virtual bool                       readRecord();
};
//******************************************************************************
#endif
