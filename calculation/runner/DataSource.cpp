//******************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//******************************************************************************
#include "DataSource.h"
#include "PrjException.h"
#include "UtilityFunctions.h"
#include "DataFileWriter.h"
#include <regex>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/replace.hpp>

using namespace boost;

/** Static method which returns newly allocated DataSource object. */
DataSource * DataSource::getNewDataSourceObject(const std::string& sSourceFilename, const Parameters::InputSource * source) {
    // if a InputSource is not defined, default to space delimited ascii source
    if (!source)
        return new CSVFileDataSource(sSourceFilename, ",", "\"", 0, false);
    // return data source object by input source type
    DataSource * dataSource=0;
    switch (source->getSourceType()) {
        case CSV : dataSource = new CSVFileDataSource(sSourceFilename, source->getDelimiter(), source->getGroup(), source->getSkip(), source->getFirstRowHeader()); break;
        default  : dataSource = new CSVFileDataSource(sSourceFilename, ",");
    }
    dataSource->setFieldsMap(source->getFieldsMap());
    return dataSource;
}


/** constructor */
CSVFileDataSource::CSVFileDataSource(const std::string& sSourceFilename, const std::string& delimiter, const std::string& grouper, unsigned long skip, bool firstRowHeaders)
                  :DataSource(), _readCount(0), _delimiter(delimiter), _grouper(grouper), _skip(skip), _ignore_empty_fields(false), _firstRowHeaders(firstRowHeaders), _grouper_escape("þæ"){
    // special processing for 'whitespace' delimiter string
    if (_delimiter == "" || _delimiter == " ") {
        _delimiter = "\t\v\f\r\n ";
        _ignore_empty_fields = true;
    }
    _sourceFile.open(sSourceFilename.c_str());
    if (!_sourceFile)
        throw resolvable_error("Error: Could not open file:\n'%s'.\n", sSourceFilename.c_str());
    // Get the byte-order mark, if there is one
    unsigned char bom[4];
    _sourceFile.read(reinterpret_cast<char*>(bom), 4);
    //Since we don't know what the endian was on the machine that created the file we
    //are reading, we'll need to check both ways.
    if ((bom[0] == 0xef && bom[1] == 0xbb && bom[2] == 0xbf) ||             // utf-8
        (bom[0] == 0 && bom[1] == 0 && bom[2] == 0xfe && bom[3] == 0xff) || // UTF-32, big-endian
        (bom[0] == 0xff && bom[1] == 0xfe && bom[2] == 0 && bom[3] == 0) || // UTF-32, little-endian
        (bom[0] == 0xfe && bom[1] == 0xff) ||                               // UTF-16, big-endian
        (bom[0] == 0xff && bom[1] == 0xfe))                                 // UTF-16, little-endian
      throwUnicodeException();
    _sourceFile.seekg(0L, std::ios::beg);
    // if first row is header, increment number of rows to skip
    if (_firstRowHeaders) ++_skip;
}

/** Re-positions file cursor to beginning of file. */
void CSVFileDataSource::gotoFirstRecord() {
    _blank_record_flag = false;
    _readCount = 0;
    _sourceFile.clear();
    _sourceFile.seekg(0L, std::ios::beg);
    std::string readbuffer;
    for (unsigned long i=0; i < _skip; ++i) {
        getlinePortable(_sourceFile, readbuffer);
        ++_readCount;
    }
}

/** sets current parsing string -- returns indication of whether string contains any words. */
bool CSVFileDataSource::parse(std::string& s, std::vector<std::string>& read_values, bool ignore_empty_fields, const std::string& delimiter, const std::string& grouper) {
    /* The default escape character with boost is the backslash - to mimic C style escaping. But that style doesn't align with common software like Excel,
       where doubling the group character the escape sequence. So we're going force mimcing the common software here - which requires a hack. */
    std::stringstream escaped_group_seq;
    escaped_group_seq << grouper << grouper; // Typical way to escape the grouping character is to escape with itself.
    boost::replace_all(s, escaped_group_seq.str(), _grouper_escape); // Replace escaped group sequence in parsing string.
    // Define tokenizer without escape character and specified delimiter and group characters.
    boost::tokenizer<boost::escaped_list_separator<char> > values(s, boost::escaped_list_separator<char>(std::string(""), delimiter, grouper));
    read_values.clear();
    for (boost::tokenizer<boost::escaped_list_separator<char> >::const_iterator itr=values.begin(); itr != values.end(); ++itr) {
        read_values.push_back(*itr);
        // Replace the escaped grouping sequence - now that token has been parsed.
        boost::replace_all(read_values.back(), _grouper_escape, grouper);
        //trim any whitespace around value
        boost::trim(read_values.back());
        // ignore empty values if delimiter is whitespace -- boost::escaped_list_separator does not consume adjacent whitespace delimiters
        if (!read_values.back().size() && ignore_empty_fields)
            read_values.pop_back();
    }
    // if all fields are empty string, then treat this as empty record
    size_t blanks=0;
    for (std::vector<std::string>::const_iterator itr=read_values.begin(); itr != read_values.end(); ++itr)
        if (itr->empty()) ++blanks;
    if (blanks == read_values.size()) read_values.clear();

    return read_values.size() > 0;
}

/** Attempts to read line from source and parse into 'words'. If read count is zero, first
    checks file is not UNICODE by looking for byte-order mark -- throwing an exception if found.
    Lines that contain no words are skipped and scanning continues to next line in file. Returns
    indication of whether record buffer contains 'words'.*/
bool CSVFileDataSource::readRecord() {
    bool isBlank = true;
    std::string readbuffer;

    // skip records as necessary
    for (long i=_readCount; i < _skip; ++i) {
        getlinePortable(_sourceFile, readbuffer);
        ++_readCount;
    }
    _read_values.clear();
    while (isBlank && getlinePortable(_sourceFile, readbuffer)) {
        try {
            isBlank = !parse(readbuffer, _read_values, _ignore_empty_fields, _delimiter, _grouper);
        } catch (boost::escaped_list_error& e) {
            throw resolvable_error("Unable to parse CSV line:\n%s", readbuffer.c_str());
        }
        if (isBlank) {
            tripBlankRecordFlag();
            ++_readCount;
        }
    }
    ++_readCount;
    return _read_values.size() > 0;
}

/** Returns number of values */
size_t CSVFileDataSource::getNumValues() {
    // Field maps are all or nothing. This means that all fields are defined in mapping or straight from record parse.
    return _fields_map.size() ? _fields_map.size() : _read_values.size();
}

std::string& CSVFileDataSource::getValueAt(long iFieldIndex) {
    // see if value at field index is mapped FieldType
    if (_fields_map.size()) {
        if (iFieldIndex < static_cast<long>(_fields_map.size())) {
            iFieldIndex = tranlateFieldIndex(iFieldIndex);
        } else {
            // index beyond defined mappings
            _read_buffer.clear();
            return _read_buffer;
        }
    }
    if (iFieldIndex > static_cast<long>(_read_values.size()) - 1) {
        _read_buffer.clear();
        return _read_buffer;
    }
    return _read_values.at(static_cast<size_t>(iFieldIndex));
}

void CSVFileDataSource::throwUnicodeException() {
    throw resolvable_error("Error: The file contains data that is Unicode formatted. Expecting ASCII file.\n");
}


//////////////////////////// SequentialFileDataSource //////////////////////////////////////////////

/** constructor */
SequentialFileDataSource::SequentialFileDataSource(const std::string& sSourceFilename, const Parameters& parameters) :
    CSVFileDataSource(sSourceFilename, ",", "\"", 1, true) {
    std::string currentparameters, sourceparameters;

    // get the settings string for the current parameter settings
    boost::trim(SequentialScanLoglikelihoodRatioWriter::getSequentialParametersString(parameters, currentparameters));

    // read first line from source data file, it should contain same parameter settings string
    while (sourceparameters.empty() && getlinePortable(_sourceFile, sourceparameters)) {
        boost::trim(sourceparameters);
    }
    if (sourceparameters.empty())
        throw resolvable_error("Error: The sequential analysis source file does not contain data.\n");

    // compare to ensure that this source file was created with current parameter settings
    if (sourceparameters != currentparameters)
        throw resolvable_error("Error: The analysis was stopped. The sequential analysis source file was created with parameter settings that are\n"
                               "       different than current analysis. You must either revert settings to those that created the initial sequential\n"
                               "       analysis file or specify a different sequential analysis filename to be created.\n");
    //_readCount = 1;
}

/** Returns the next LLR value in data source. */
boost::optional<double> SequentialFileDataSource::nextLLR() {
    boost::optional<double> llr;

    if (readRecord()) {
        if (getNumValues() != 1)
            throw resolvable_error("Error: The sequential scan source file contains a record with extra data, record %u.\n", getCurrentRecordIndex());
        double value;
        if (!string_to_numeric_type<double>(getValueAt(0).c_str(), value)) {
            throw resolvable_error("Error: The sequential scan source file contains record with value that is not numeric, record %u, value %s.\n", getCurrentRecordIndex(), getValueAt(0).c_str());
        }
        llr.reset(value);
    }
    return llr;
}
