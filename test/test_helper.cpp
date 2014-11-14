
#include "test_helper.h"

#include <boost/test/unit_test.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

//#include "AnalysisRun.h"
#include "Toolkit.h"
#include "UtilityFunctions.h"

po::options_description& addCustomOptions(po::options_description& prg_options) {
    prg_options.add_options()
        ("fixture-examples-path,s", po::value<std::string>(), "path to fixture examples data files")
        ("fixture-testdata-path,t", po::value<std::string>(), "path to fixture test data files");
    return prg_options;
}

std::string getExamplesFilesPath() {
    namespace utf = boost::unit_test::framework;
    char ** argv = utf::master_test_suite().argv;
    int argc = utf::master_test_suite().argc;

    po::variables_map vm;
    po::options_description prg_options("", 200);
    addCustomOptions(prg_options);

    po::store(po::command_line_parser(argc, argv).options(prg_options).style(po::command_line_style::default_style|po::command_line_style::case_insensitive).run(), vm);
    po::notify(vm);
    if (!vm.count("fixture-examples-path"))
        BOOST_FAIL("Examples path not specified (fixture-examples-path)");
    return vm["fixture-examples-path"].as<std::string>();
}

std::string getTestSetFilesPath() {
    namespace utf = boost::unit_test::framework;
    char ** argv = utf::master_test_suite().argv;
    int argc = utf::master_test_suite().argc;

    po::variables_map vm;
    po::options_description prg_options("", 200);
    addCustomOptions(prg_options);

    po::store(po::command_line_parser(argc, argv).options(prg_options).style(po::command_line_style::default_style|po::command_line_style::case_insensitive).run(), vm);
    po::notify(vm);
    if (!vm.count("fixture-testdata-path"))
        BOOST_FAIL("Test Data path not specified (fixture-testdata-path)");
    return vm["fixture-testdata-path"].as<std::string>();
}

std::ifstream & getFileStream(std::ifstream& stream, const std::string& filename, std::string& results_user_directory) {
    std::stringstream rr_filename;
    rr_filename << GetUserTemporaryDirectory(results_user_directory).c_str() << "\\" << filename.c_str();
    stream.open(rr_filename.str().c_str());
    if (!stream) throw std::exception("could not open file");
    return stream;
}

void run_analysis(const std::string& analysis_name, std::string& results_user_directory, CParameters& parameters, BasePrint& print) {
    // set results file to the user document directory
    std::stringstream filename;
    /*filename << GetUserTemporaryDirectory(results_user_directory).c_str() << "\\" << analysis_name.c_str() << ".txt";
    parameters.SetOutputFileName(filename.str().c_str());

    time_t startTime;
    time(&startTime);
    AppToolkit::ToolKitCreate(boost::unit_test::framework::master_test_suite().argv[0]);
    AnalysisRunner(parameters, startTime, print).Execute();
    AppToolkit::ToolKitDestroy();
    */
}

CSV_Row_t& getCSVRow(std::ifstream& stream, CSV_Row_t& row) {
    row.clear();
    std::string line;
    std::getline(stream, line);
    boost::escaped_list_separator<char> separator("\\", "\t\v\f\r\n ", "\"");
    boost::tokenizer<boost::escaped_list_separator<char> > headers(line, separator);
    for (boost::tokenizer<boost::escaped_list_separator<char> >::const_iterator itr=headers.begin(); itr != headers.end(); ++itr) {
        row.push_back(*itr);
        boost::trim(row.back());
        if (!row.back().size()) row.pop_back();
    }
    return row;
}
