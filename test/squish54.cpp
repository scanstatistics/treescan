
// project files
#include "fixture_examples.h"
#include "ScanRunner.h"
#include "Toolkit.h"
#include "DataFileWriter.h"
#include "ParametersValidate.h"

/* Test Suite for the attributable risk data write -- https://www.squishlist.com/ims/treescan/54/. */
BOOST_AUTO_TEST_SUITE( attributable_risk_suite )

/* Tests the expected values of the attributable risk data  with unconditional Poisson model. */
BOOST_FIXTURE_TEST_CASE( test_attributable_risk_unconditional_poisson, poisson_fixture ) {
    _parameters.setGeneratingTableResults(true);
    _parameters.setConditionalType(Parameters::UNCONDITIONAL);
    _parameters.setReportAttributableRisk(true);
    _parameters.setAttributableRiskExposed(200);
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

    std::string results_user_directory;
    run_analysis("test", results_user_directory, _parameters, _print);

    // open the power estimation file and confirm expected columns and values match expected for this data set
    std::string buffer;
    printString(buffer, "test%s", CSVDataFileWriter::CSV_FILE_EXT);
    std::ifstream stream;
    getFileStream(stream, buffer, results_user_directory);

    CSV_Row_t headers;
    getCSVRow(stream, headers);
    std::vector<std::string>::iterator itrCol = std::find(headers.begin(), headers.end(), std::string(DataRecordWriter::ATTRIBUTABLE_RISK_FIELD));
    if (itrCol == headers.end()) BOOST_FAIL( "Attributable Risk column not found" );

    // check the expected values for this analysis
    double expected [71] = {0.21, 0.19, 0.19, 0.19, 0.064, 0.056, 0.056, 0.073, 0.073, 0.019, 0.019, 0.019, 0.14, 0.023, 0.023, 0.067, 0.075, 0.081, 0.023, 0.023, 0.023, 0.023, 0.019, 0.088, 0.13, 0.079, 0.077, 0.021, 0.062, 0.038, 0.08, 0.08, 0.08, 0.039, 0.013, 0.0092, 0.0092, 0.013, 0.067, 0.031, 0.031, 0.031, 0.0081, 0.0081, 0.024, 0.017, 0.008, 0.0079, 0.0079, 0.018, 0.018, 0.016, 0.018, 0.018, 0.015, 0.015, 0.0071, 0.0071, 0.011, 0.011, 0.01, 0.01, 0.01, 0.0063, 0.0063, 0.0099, 0.0099, 0.0099, 0.0098, 0.0085, 0.0085};
    unsigned int dataRows=0;
    CSV_Row_t data;
    getCSVRow(stream, data);
    while (data.size()) {
        double ar;
        BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrCol)).c_str(), ar) );
        BOOST_REQUIRE_CLOSE( ar, expected[dataRows], 0.001 );
        ++dataRows;
        getCSVRow(stream, data);
    }
    if (dataRows != 71) BOOST_FAIL( "expecting 71 data rows, got" << dataRows );
    stream.close();
}

/* Tests the expected values of the attributable risk data  with conditional Poisson model. */
BOOST_FIXTURE_TEST_CASE( test_attributable_risk_conditional_poisson, poisson_fixture ) {
    _parameters.setGeneratingTableResults(true);
    _parameters.setConditionalType(Parameters::TOTALCASES);
    _parameters.setReportAttributableRisk(true);
    _parameters.setAttributableRiskExposed(200);
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

    std::string results_user_directory;
    run_analysis("test", results_user_directory, _parameters, _print);

    // open the power estimation file and confirm expected columns and values match expected for this data set
    std::string buffer;
    printString(buffer, "test%s", CSVDataFileWriter::CSV_FILE_EXT);
    std::ifstream stream;
    getFileStream(stream, buffer, results_user_directory);

    CSV_Row_t headers;
    getCSVRow(stream, headers);
    std::vector<std::string>::iterator itrCol = std::find(headers.begin(), headers.end(), std::string(DataRecordWriter::ATTRIBUTABLE_RISK_FIELD));
    if (itrCol == headers.end()) BOOST_FAIL( "Attributable Risk column not found" );

    // check the expected values for this analysis
    double expected [27] = {0.21, 0.19, 0.19, 0.19, 0.053, 0.053, 0.06, 0.065, 0.065, 0.019, 0.019, 0.019, 0.022, 0.022, 0.021, 0.021, 0.021, 0.018, 0.021, 0.019, 0.043, 0.0086, 0.0086, 0.012, 0.043, 0.011, 0.043};
    unsigned int dataRows=0;
    CSV_Row_t data;
    getCSVRow(stream, data);
    while (data.size()) {
        double ar;
        BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrCol)).c_str(), ar) );
        BOOST_REQUIRE_CLOSE( ar, expected[dataRows], 0.001 );
        ++dataRows;
        getCSVRow(stream, data);
    }
    if (dataRows != 27) BOOST_FAIL( "expecting 27 data rows, got" << dataRows );
    stream.close();
}

/* Tests the expected values of the attributable risk data  with time-only scan. */
BOOST_FIXTURE_TEST_CASE( test_attributable_risk_time_only_scan, time_only_fixture ) {
    _parameters.setGeneratingTableResults(true);
    _parameters.setReportAttributableRisk(true);
    _parameters.setAttributableRiskExposed(200);
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

    std::string results_user_directory;
    run_analysis("test", results_user_directory, _parameters, _print);

    // open the power estimation file and confirm expected columns and values match expected for this data set
    std::string buffer;
    printString(buffer, "test%s", CSVDataFileWriter::CSV_FILE_EXT);
    std::ifstream stream;
    getFileStream(stream, buffer, results_user_directory);

    CSV_Row_t headers;
    getCSVRow(stream, headers);
    std::vector<std::string>::iterator itrCol = std::find(headers.begin(), headers.end(), std::string(DataRecordWriter::ATTRIBUTABLE_RISK_FIELD));
    if (itrCol == headers.end()) BOOST_FAIL( "Attributable Risk column not found" );

    // check the expected values for this analysis
    double expected [10] = {2.52, 2.51, 2.52, 1.96, 1.92, 0.96, 1.35, 1.21, 1.08, 0.031};
    unsigned int dataRows=0;
    CSV_Row_t data;
    getCSVRow(stream, data);
    while (data.size()) {
        double ar;
        BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrCol)).c_str(), ar) );
        BOOST_REQUIRE_CLOSE( ar, expected[dataRows], 0.001 );
        ++dataRows;
        getCSVRow(stream, data);
    }
    if (dataRows != 10) BOOST_FAIL( "expecting 10 data rows, got" << dataRows );
    stream.close();
}

/* Tests the expected values of the attributable risk data  with time-only scan w/ day of week adjustment. */
BOOST_FIXTURE_TEST_CASE( test_attributable_risk_time_only_scan_w_dow, time_only_fixture ) {
    _parameters.setGeneratingTableResults(true);
    _parameters.setPerformDayOfWeekAdjustment(true);
    _parameters.setReportAttributableRisk(true);
    _parameters.setAttributableRiskExposed(200);
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

    std::string results_user_directory;
    run_analysis("test", results_user_directory, _parameters, _print);

    // open the power estimation file and confirm expected columns and values match expected for this data set
    std::string buffer;
    printString(buffer, "test%s", CSVDataFileWriter::CSV_FILE_EXT);
    std::ifstream stream;
    getFileStream(stream, buffer, results_user_directory);

    CSV_Row_t headers;
    getCSVRow(stream, headers);
    std::vector<std::string>::iterator itrCol = std::find(headers.begin(), headers.end(), std::string(DataRecordWriter::ATTRIBUTABLE_RISK_FIELD));
    if (itrCol == headers.end()) BOOST_FAIL( "Attributable Risk column not found" );

    // check the expected values for this analysis
    double expected [10] = {2.52, 2.49, 2.5, 1.93, 1.93, 1.18, 1.01, 0.85, 0.82, 0.063};
    unsigned int dataRows=0;
    CSV_Row_t data;
    getCSVRow(stream, data);
    while (data.size()) {
        double ar;
        BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrCol)).c_str(), ar) );
        // spot check data rows
        BOOST_REQUIRE_CLOSE( ar, expected[dataRows], 0.001 );
        ++dataRows;
        getCSVRow(stream, data);
    }
    if (dataRows != 10) BOOST_FAIL( "expecting 10 data rows, got" << dataRows );
    stream.close();
}

/* Tests the expected values of the attributable risk data  with tree-time scan. */
BOOST_FIXTURE_TEST_CASE( test_attributable_risk_tree_time_scan_condition_node, tree_temporal_fixture ) {
    _parameters.setGeneratingTableResults(true);
    _parameters.setReportAttributableRisk(true);
    _parameters.setAttributableRiskExposed(200);
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

    std::string results_user_directory;
    run_analysis("test", results_user_directory, _parameters, _print);

    // open the power estimation file and confirm expected columns and values match expected for this data set
    std::string buffer;
    printString(buffer, "test%s", CSVDataFileWriter::CSV_FILE_EXT);
    std::ifstream stream;
    getFileStream(stream, buffer, results_user_directory);

    CSV_Row_t headers;
    getCSVRow(stream, headers);
    std::vector<std::string>::iterator itrCol = std::find(headers.begin(), headers.end(), std::string(DataRecordWriter::ATTRIBUTABLE_RISK_FIELD));
    if (itrCol == headers.end()) BOOST_FAIL( "Attributable Risk column not found" );

    // check the expected values for this analysis
    double expected [117] = {0.57, 0.64, 0.5, 0.5, 0.34, 0.29, 0.23, 0.23, 0.23, 0.23, 0.22, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.17, 0.15, 0.15, 0.14, 0.13, 0.13, 0.13, 0.13, 0.12, 0.12, 0.12, 0.12, 0.12, 0.11, 0.11, 0.11, 0.1, 0.085, 0.085, 0.075, 0.075, 0.075, 0.075, 0.07, 0.07, 0.07, 0.07, 0.07, 0.065, 0.06, 0.06, 0.05, 0.05, 0.05, 0.045, 0.045, 0.04, 0.04, 0.04, 0.04, 0.04, 0.035, 0.035, 0.035, 0.03, 0.03, 0.03, 0.03, 0.03, 0.025, 0.025, 0.025, 0.025, 0.025, 0.025, 0.025, 0.025, 0.025, 0.025, 0.025, 0.025, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.015, 0.015, 0.015, 0.015, 0.015, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01};
    unsigned int dataRows=0;
    CSV_Row_t data;
    getCSVRow(stream, data);
    while (data.size()) {
        double ar;
        BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrCol)).c_str(), ar) );
        BOOST_REQUIRE_CLOSE( ar, expected[dataRows], 0.001 );
        ++dataRows;
        getCSVRow(stream, data);
    }
    if (dataRows != 117) BOOST_FAIL( "expecting 117 data rows, got" << dataRows );
    stream.close();
}

/* Tests the expected values of the attributable risk data  with tree-time scan. */
BOOST_FIXTURE_TEST_CASE( test_attributable_risk_tree_time_scan_condition_node_w_dow, tree_temporal_fixture ) {
    _parameters.setPerformDayOfWeekAdjustment(true);
    _parameters.setGeneratingTableResults(true);
    _parameters.setReportAttributableRisk(true);
    _parameters.setAttributableRiskExposed(200);
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

    std::string results_user_directory;
    run_analysis("test", results_user_directory, _parameters, _print);

    // open the power estimation file and confirm expected columns and values match expected for this data set
    std::string buffer;
    printString(buffer, "test%s", CSVDataFileWriter::CSV_FILE_EXT);
    std::ifstream stream;
    getFileStream(stream, buffer, results_user_directory);

    CSV_Row_t headers;
    getCSVRow(stream, headers);
    std::vector<std::string>::iterator itrCol = std::find(headers.begin(), headers.end(), std::string(DataRecordWriter::ATTRIBUTABLE_RISK_FIELD));
    if (itrCol == headers.end()) BOOST_FAIL( "Attributable Risk column not found" );

    // check the expected values for this analysis
    double expected [80] = {0.63, 0.57, 0.5, 0.5, 0.34, 0.29, 0.2, 0.23, 0.23, 0.23, 0.23, 0.2, 0.2, 0.15, 0.17, 0.22, 0.12, 0.2, 0.2, 0.2, 0.15, 0.14, 0.1, 0.13, 0.13, 0.13, 0.13, 0.12, 0.12, 0.12, 0.12, 0.11, 0.11, 0.11, 0.07, 0.07, 0.065, 0.085, 0.085, 0.06, 0.06, 0.075, 0.075, 0.075, 0.05, 0.05, 0.075, 0.07, 0.07, 0.07, 0.04, 0.05, 0.035, 0.045, 0.045, 0.03, 0.04, 0.04, 0.04, 0.04, 0.035, 0.035, 0.025, 0.025, 0.025, 0.025, 0.025, 0.025, 0.025, 0.025, 0.03, 0.03, 0.03, 0.03, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02};
    unsigned int dataRows=0;
    CSV_Row_t data;
    getCSVRow(stream, data);
    while (data.size()) {
        double ar;
        BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrCol)).c_str(), ar) );
        BOOST_REQUIRE_CLOSE( ar, expected[dataRows], 0.001 );
        ++dataRows;
        getCSVRow(stream, data);
    }
    if (dataRows != 80) BOOST_FAIL( "expecting 80 data rows, got" << dataRows );
    stream.close();
}

/* Tests the expected values of the attributable risk data  with tree-time scan. */
BOOST_FIXTURE_TEST_CASE( test_attributable_risk_tree_time_scan_condition_nodetime, tree_temporal_fixture ) {
    _parameters.setConditionalType(Parameters::NODEANDTIME);
    _parameters.setGeneratingTableResults(true);
    _parameters.setReportAttributableRisk(true);
    _parameters.setAttributableRiskExposed(200);
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

    std::string results_user_directory;
    run_analysis("test", results_user_directory, _parameters, _print);

    // open the power estimation file and confirm expected columns and values match expected for this data set
    std::string buffer;
    printString(buffer, "test%s", CSVDataFileWriter::CSV_FILE_EXT);
    std::ifstream stream;
    getFileStream(stream, buffer, results_user_directory);

    CSV_Row_t headers;
    getCSVRow(stream, headers);
    std::vector<std::string>::iterator itrCol = std::find(headers.begin(), headers.end(), std::string(DataRecordWriter::ATTRIBUTABLE_RISK_FIELD));
    if (itrCol == headers.end()) BOOST_FAIL( "Attributable Risk column not found" );

    // check the expected values for this analysis
    double expected [110] = {0.34, 0.2, 0.2, 0.2, 0.57, 0.23, 0.23, 0.23, 0.5, 0.5, 0.29, 0.57, 0.15, 0.11, 0.11, 0.17, 0.23, 0.12, 0.15, 0.14, 0.1, 0.12, 0.12, 0.12, 0.12, 0.13, 0.13, 0.13, 0.13, 0.11, 0.07, 0.07, 0.22, 0.065, 0.085, 0.085, 0.075, 0.075, 0.075, 0.06, 0.06, 0.2, 0.2, 0.2, 0.04, 0.04, 0.04, 0.04, 0.05, 0.05, 0.04, 0.01, 0.01, 0.05, 0.035, 0.025, 0.025, 0.025, 0.025, 0.045, 0.045, 0.03, 0.02, 0.02, 0.025, 0.025, 0.025, 0.025, 0.025, 0.025, 0.025, 0.025, 0.035, 0.035, 0.03, 0.075, 0.03, 0.03, 0.03, 0.07, 0.07, 0.07, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.015, 0.015, 0.015, 0.015, 0.02, 0.02, 0.02, 0.015, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01};
    unsigned int dataRows=0;
    CSV_Row_t data;
    getCSVRow(stream, data);
    while (data.size()) {
        double ar;
        BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrCol)).c_str(), ar) );
        BOOST_REQUIRE_CLOSE( ar, expected[dataRows], 0.001 );
        ++dataRows;
        getCSVRow(stream, data);
    }
    if (dataRows != 110) BOOST_FAIL( "expecting 110 data rows, got" << dataRows );
    stream.close();
}

/* Tests the expected values of the attributable risk data  with tree-time scan. */
BOOST_FIXTURE_TEST_CASE( test_attributable_risk_tree_time_scan_condition_nodetime_w_dow, tree_temporal_fixture ) {
    _parameters.setConditionalType(Parameters::NODEANDTIME);
    _parameters.setPerformDayOfWeekAdjustment(true);
    _parameters.setGeneratingTableResults(true);
    _parameters.setReportAttributableRisk(true);
    _parameters.setAttributableRiskExposed(200);
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

    std::string results_user_directory;
    run_analysis("test", results_user_directory, _parameters, _print);

    // open the power estimation file and confirm expected columns and values match expected for this data set
    std::string buffer;
    printString(buffer, "test%s", CSVDataFileWriter::CSV_FILE_EXT);
    std::ifstream stream;
    getFileStream(stream, buffer, results_user_directory);

    CSV_Row_t headers;
    getCSVRow(stream, headers);
    std::vector<std::string>::iterator itrCol = std::find(headers.begin(), headers.end(), std::string(DataRecordWriter::ATTRIBUTABLE_RISK_FIELD));
    if (itrCol == headers.end()) BOOST_FAIL( "Attributable Risk column not found" );

    // check the expected values for this analysis
    double expected [36] = {0.2, 0.2, 0.11, 0.11, 0.34, 0.01, 0.01, 0.23, 0.23, 0.23, 0.04, 0.04, 0.04, 0.04, 0.025, 0.025, 0.025, 0.025, 0.12, 0.12, 0.12, 0.12, 0.02, 0.02, 0.075, 0.075, 0.075, 0.11, 0.17, 0.15, 0.14, 0.13, 0.13, 0.13, 0.13, 0.044};
    unsigned int dataRows=0;
    CSV_Row_t data;
    getCSVRow(stream, data);
    while (data.size()) {
        double ar;
        BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrCol)).c_str(), ar) );
        BOOST_REQUIRE_CLOSE( ar, expected[dataRows], 0.001 );
        ++dataRows;
        getCSVRow(stream, data);
    }
    if (dataRows != 36) BOOST_FAIL( "expecting 36 data rows, got" << dataRows );
    stream.close();
}

BOOST_AUTO_TEST_SUITE_END()
