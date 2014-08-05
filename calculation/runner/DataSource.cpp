//******************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//******************************************************************************
#include "DataSource.h"
#include "PrjException.h"
#include "UtilityFunctions.h"
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

/** Static method which returns newly allocated DataSource object. */
DataSource * DataSource::getNewDataSourceObject(const std::string& sSourceFilename) {
    return new CSVFileDataSource(sSourceFilename);
}

/** constructor */
CSVFileDataSource::CSVFileDataSource(const std::string& sSourceFilename) : DataSource(), _readCount(0), _delimiter(","), _group("\"") {
    try {
        _sourceFile.open(sSourceFilename.c_str());
        if (!_sourceFile)
            throw resolvable_error("Error: Could not open file:\n'%s'.\n", sSourceFilename.c_str());
    } catch (prg_exception& x) {
        x.addTrace("constructor()","CSVFileDataSource");
        throw;
    }
}

/** Re-positions file cursor to beginning of file. */
void CSVFileDataSource::gotoFirstRecord() {
    _blank_record_flag = false;
    _readCount = 0;
    _sourceFile.clear();
    _sourceFile.seekg(0L, std::ios::beg);
}

/** sets current parsing string -- returns indication of whether string contains any words. */
bool CSVFileDataSource::parse(const std::string& s, const std::string& delimiter, const std::string& grouper) {
    _read_values.clear();
    std::string e("\\");
    boost::escaped_list_separator<char> separator(e, delimiter, grouper);
    boost::tokenizer<boost::escaped_list_separator<char> > values(s, separator);
    for (boost::tokenizer<boost::escaped_list_separator<char> >::const_iterator itr=values.begin(); itr != values.end(); ++itr) {
        _read_values.push_back(*itr);
        //trim any whitespace around value
        boost::trim(_read_values.back());
    }
    return _read_values.size() > 0;
}

/** Attempts to read line from source and parse into 'words'. If read count is zero, first
    checks file is not UNICODE by looking for byte-order mark -- throwing an exception if found.
    Lines that contain no words are skipped and scanning continues to next line in file. Returns
    indication of whether record buffer contains 'words'.*/
bool CSVFileDataSource::readRecord() {
    bool isBlank = true;
    std::string readbuffer;

    if (!_readCount) {
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
    }

    _read_values.clear();
    while (isBlank && getlinePortable(_sourceFile, readbuffer)) {
        isBlank = !parse(readbuffer, _delimiter, _group);
        if (isBlank) {
            tripBlankRecordFlag();
            ++_readCount;
        }
    }
    ++_readCount;
    return _read_values.size() > 0;
}

void CSVFileDataSource::throwUnicodeException() {
    throw resolvable_error("Error: The file contains data that is Unicode formatted. Expecting ASCII file.\n");
}
