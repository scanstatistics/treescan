//******************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//******************************************************************************
#include "DataSource.h"
#include "PrjException.h"
#include "UtilityFunctions.h"
#include<boost/tokenizer.hpp>

/** Static method which returns newly allocated DataSource object. */
DataSource * DataSource::getNewDataSourceObject(const std::string& sSourceFilename) {
    return new CSVFileDataSource(sSourceFilename);
}

/** constructor */
CSVFileDataSource::CSVFileDataSource(const std::string& sSourceFilename) : DataSource(), _readCount(0) {
  try {
    _sourceFile.open(sSourceFilename.c_str());
    if (!_sourceFile)
      throw resolvable_error("Error: Could not open file:\n'%s'.\n", sSourceFilename.c_str());
  }
  catch (prg_exception& x) {
    x.addTrace("constructor()","CSVFileDataSource");
    throw;
  }
}

/** Re-positions file cursor to beginning of file. */
void CSVFileDataSource::gotoFirstRecord() {
  _readCount = 0;
  _sourceFile.clear();
  _sourceFile.seekg(0L, std::ios::beg);
}

/** Attempts to read line from source and parse into 'words'. If read count is zero, first
    checks file is not UNICODE by looking for byte-order mark -- throwing an exception if found.
    Lines that contain no words are skipped and scanning continues to next line in file. Returns
    indication of whether record buffer contains 'words'.*/
bool CSVFileDataSource::readRecord() {
  std::string  buffer;

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
  while (getlinePortable(_sourceFile, buffer)) {
        boost::tokenizer<boost::escaped_list_separator<char> > csv(buffer);
        boost::tokenizer<boost::escaped_list_separator<char> >::const_iterator itr=csv.begin();
        _read_values.clear();
        for (;itr != csv.end(); ++itr) {
            std::string val = (*itr); 
            _read_values.push_back(trimString(val));
        }
        ++_readCount;
        if (_read_values.size()) break;
  }
  return _read_values.size() > 0;
}

void CSVFileDataSource::throwUnicodeException() {
  throw resolvable_error("Error: The file contains data that is Unicode formatted. Expecting ASCII file.\n");
}
