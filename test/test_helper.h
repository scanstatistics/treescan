#ifndef __TEST_HELPER_H
#define __TEST_HELPER_H

#include <string>
#include <vector>
#include <fstream>
#include <boost/program_options.hpp>

namespace po = boost::program_options;
class Parameters;
class BasePrint;

po::options_description& addCustomOptions(po::options_description& prg_options);
std::string getExamplesFilesPath();
std::string getTestSetFilesPath();
std::ifstream & getFileStream(std::ifstream& stream, const std::string& filename, std::string& results_user_directory);
void run_analysis(const std::string& analysis_name, std::string& results_user_directory, Parameters& parameters, BasePrint& print);

typedef std::vector<std::string> CSV_Row_t;
CSV_Row_t& getCSVRow(std::ifstream& stream, CSV_Row_t& row);

#endif
