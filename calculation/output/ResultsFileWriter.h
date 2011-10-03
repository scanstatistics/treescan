//******************************************************************************
#ifndef __ResultsFileWriter_H
#define __ResultsFileWriter_H
//******************************************************************************
#include <iostream>
#include <fstream>

class ScanRunner;

class ResultsFileWriter {
  protected:
    const ScanRunner & _scanRunner;

    std::ofstream & openStream(const std::string& outputfile, std::ofstream & outfile);
    std::string & getTotalRunningTime(time_t start, time_t end, std::string & buffer);

  public:
    ResultsFileWriter(const ScanRunner& scanRunner) : _scanRunner(scanRunner) {}

    bool writeASCII(const std::string& outputfile, time_t start, time_t end);
    bool writeHTML(const std::string& outputfile, time_t start, time_t end);
};
//******************************************************************************
#endif
