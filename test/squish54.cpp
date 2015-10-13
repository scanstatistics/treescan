
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
    BOOST_REQUIRE_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

    std::string results_user_directory;
    run_analysis("test", results_user_directory, _parameters, _print);

    // open the power estimation file and confirm expected columns and values match expected for this data set
    std::string buffer;
    printString(buffer, "test%s", CSVDataFileWriter::CSV_FILE_EXT);
    std::ifstream stream;
    getFileStream(stream, buffer, results_user_directory);

    CSV_Row_t headers;
    getCSVRow(stream, headers);
    std::vector<std::string>::iterator itrAR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::ATTRIBUTABLE_RISK_FIELD);

    // check the expected values for this analysis
    unsigned int dataRows=0, expectedRows=0;
    if (dataRows != expectedRows) BOOST_FAIL( "expecting " << expectedRows << " data rows, got " << dataRows );
    stream.close();
}

/* Tests the expected values of the attributable risk data  with conditional Poisson model. */
BOOST_FIXTURE_TEST_CASE( test_attributable_risk_conditional_poisson, poisson_fixture ) {
    _parameters.setGeneratingTableResults(true);
    _parameters.setConditionalType(Parameters::TOTALCASES);
    _parameters.setReportAttributableRisk(true);
    _parameters.setAttributableRiskExposed(200);
    BOOST_REQUIRE_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

    std::string results_user_directory;
    run_analysis("test", results_user_directory, _parameters, _print);

    // open the power estimation file and confirm expected columns and values match expected for this data set
    std::string buffer;
    printString(buffer, "test%s", CSVDataFileWriter::CSV_FILE_EXT);
    std::ifstream stream;
    getFileStream(stream, buffer, results_user_directory);

    CSV_Row_t headers;
    getCSVRow(stream, headers);
    std::vector<std::string>::iterator itrAR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::ATTRIBUTABLE_RISK_FIELD);

    // check the expected values for this analysis
    double expected [6] = {0.059, 0.12, 0.089, 0.023, 0.017, 0.02};
    unsigned int dataRows=0, expectedRows=6;
    CSV_Row_t data;
    getCSVRow(stream, data);
    while (data.size()) {
        if (dataRows < expectedRows) {
            double ar; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrAR)).c_str(), ar) );
            BOOST_REQUIRE_CLOSE( ar, expected[dataRows], 0.001 );
        }
        ++dataRows;
        getCSVRow(stream, data);
    }
    if (dataRows != expectedRows) BOOST_FAIL( "expecting " << expectedRows << " data rows, got " << dataRows );
    stream.close();
}

/* Tests the expected values of the attributable risk data  with unconditional Bernoulli model. */
BOOST_FIXTURE_TEST_CASE( test_attributable_risk_unconditional_bernoulli, bernoulli_fixture ) {
    _parameters.setGeneratingTableResults(true);
    _parameters.setConditionalType(Parameters::UNCONDITIONAL);
    _parameters.setReportAttributableRisk(true);
    _parameters.setAttributableRiskExposed(200);
    BOOST_REQUIRE_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

    std::string results_user_directory;
    run_analysis("test", results_user_directory, _parameters, _print);

    // open the power estimation file and confirm expected columns and values match expected for this data set
    std::string buffer;
    printString(buffer, "test%s", CSVDataFileWriter::CSV_FILE_EXT);
    std::ifstream stream;
    getFileStream(stream, buffer, results_user_directory);

    CSV_Row_t headers;
    getCSVRow(stream, headers);
    std::vector<std::string>::iterator itrAR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::ATTRIBUTABLE_RISK_FIELD);

    // check the expected values for this analysis
    double expected [9] = {0.11, 0.063, 0.083, 0.026, 0.021, 0.073, 0.026, 0.021, 0.0052};
    unsigned int dataRows=0, expectedRows=9;
    CSV_Row_t data;
    getCSVRow(stream, data);
    while (data.size()) {
        if (dataRows < expectedRows) {
            double ar; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrAR)).c_str(), ar) );
            BOOST_REQUIRE_CLOSE( ar, expected[dataRows], 0.001 );
        }
        ++dataRows;
        getCSVRow(stream, data);
    }
    if (dataRows != expectedRows) BOOST_FAIL( "expecting " << expectedRows << " data rows, got " << dataRows );
    stream.close();
}

/* Tests the expected values of the attributable risk data with unconditional Bernoulli model and self control design. */
BOOST_FIXTURE_TEST_CASE( test_attributable_risk_unconditional_bernoulli_self_control_design, bernoulli_fixture ) {
    _parameters.setGeneratingTableResults(true);
    _parameters.setConditionalType(Parameters::UNCONDITIONAL);
    _parameters.setReportAttributableRisk(true);
    _parameters.setAttributableRiskExposed(200);
    _parameters.setSelfControlDesign(true);
    BOOST_REQUIRE_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

    std::string results_user_directory;
    run_analysis("test", results_user_directory, _parameters, _print);

    // open the power estimation file and confirm expected columns and values match expected for this data set
    std::string buffer;
    printString(buffer, "test%s", CSVDataFileWriter::CSV_FILE_EXT);
    std::ifstream stream;
    getFileStream(stream, buffer, results_user_directory);

    CSV_Row_t headers;
    getCSVRow(stream, headers);
    std::vector<std::string>::iterator itrRR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::RELATIVE_RISK_FIELD);
    std::vector<std::string>::iterator itrExcess = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::EXCESS_CASES_FIELD);
    std::vector<std::string>::iterator itrAR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::ATTRIBUTABLE_RISK_FIELD);

    // check the expected values for this analysis
    double expected_rr [9] = {2.75, 7.0, 1.84, 3.5, 3.0, 1.34, 2.25, 1.31, 1.33};
    double expected_ex [9] = {21.0, 12.0, 16.0, 5.0, 4.0, 14.0, 5.0, 4.0, 1.0};
    double expected_ar [9] = {0.11, 0.06, 0.08, 0.025, 0.02, 0.07, 0.025, 0.02, 0.005};
    unsigned int dataRows=0, expectedRows=9;
    CSV_Row_t data;
    getCSVRow(stream, data);
    while (data.size()) {
        if (dataRows < expectedRows) {
            double rr; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrRR)).c_str(), rr) );
            double excess; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrExcess)).c_str(), excess) );
            double ar; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrAR)).c_str(), ar) );
            BOOST_CHECK_CLOSE( rr, expected_rr[dataRows], 0.001 );
            BOOST_CHECK_CLOSE( excess, expected_ex[dataRows], 0.001 );
            BOOST_REQUIRE_CLOSE( ar, expected_ar[dataRows], 0.00001 );
        }
        ++dataRows;
        getCSVRow(stream, data);
    }
    if (dataRows != expectedRows) BOOST_FAIL( "expecting " << expectedRows << " data rows, got " << dataRows );
    stream.close();
}

/* Tests the expected values of the attributable risk data  with conditional Bernoulli model. */
BOOST_FIXTURE_TEST_CASE( test_attributable_risk_conditional_bernoulli, bernoulli_fixture ) {
    _parameters.setGeneratingTableResults(true);
    _parameters.setConditionalType(Parameters::TOTALCASES);
    _parameters.setReportAttributableRisk(true);
    _parameters.setAttributableRiskExposed(200);
    BOOST_REQUIRE_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

    std::string results_user_directory;
    run_analysis("test", results_user_directory, _parameters, _print);

    // open the power estimation file and confirm expected columns and values match expected for this data set
    std::string buffer;
    printString(buffer, "test%s", CSVDataFileWriter::CSV_FILE_EXT);
    std::ifstream stream;
    getFileStream(stream, buffer, results_user_directory);

    CSV_Row_t headers;
    getCSVRow(stream, headers);
    std::vector<std::string>::iterator itrAR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::ATTRIBUTABLE_RISK_FIELD);

    // check the expected values for this analysis
    double expected [6] = {0.14, 0.06, 0.097, 0.021, 0.016, 0.019};
    unsigned int dataRows=0, expectedRows=6;
    CSV_Row_t data;
    getCSVRow(stream, data);
    while (data.size()) {
        if (dataRows < expectedRows) {
            double ar; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrAR)).c_str(), ar) );
            BOOST_REQUIRE_CLOSE( ar, expected[dataRows], 0.001 );
        }
        ++dataRows;
        getCSVRow(stream, data);
    }
    if (dataRows != expectedRows) BOOST_FAIL( "expecting " << expectedRows << " data rows, got " << dataRows );
    stream.close();
}

/* Tests the expected values of the attributable risk data  with time-only scan. */
BOOST_FIXTURE_TEST_CASE( test_attributable_risk_time_only_scan, time_only_fixture ) {
    _parameters.setGeneratingTableResults(true);
    _parameters.setReportAttributableRisk(true);
    _parameters.setAttributableRiskExposed(200);
    BOOST_REQUIRE_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

    std::string results_user_directory;
    run_analysis("test", results_user_directory, _parameters, _print);

    // open the power estimation file and confirm expected columns and values match expected for this data set
    std::string buffer;
    printString(buffer, "test%s", CSVDataFileWriter::CSV_FILE_EXT);
    std::ifstream stream;
    getFileStream(stream, buffer, results_user_directory);

    CSV_Row_t headers;
    getCSVRow(stream, headers);
    std::vector<std::string>::iterator itrAR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::ATTRIBUTABLE_RISK_FIELD);

    // check the expected values for this analysis
    double expected [16] = {0.2, 0.2, 0.14, 0.2, 0.1, 0.19, 0.081, 0.19, 0.19, 0.18, 0.17, 0.14, 0.12, 0.027, 0.075, 0.011};
    unsigned int dataRows=0, expectedRows=16;
    CSV_Row_t data;
    getCSVRow(stream, data);
    while (data.size()) {
        if (dataRows < expectedRows) {
            double ar; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrAR)).c_str(), ar) );
            BOOST_REQUIRE_CLOSE( ar, expected[dataRows], 0.001 );
        }
        ++dataRows;
        getCSVRow(stream, data);
    }
    if (dataRows != expectedRows) BOOST_FAIL( "expecting " << expectedRows << " data rows, got " << dataRows );
    stream.close();
}

/* Tests the expected values of the attributable risk data  with time-only scan w/ day of week adjustment. */
BOOST_FIXTURE_TEST_CASE( test_attributable_risk_time_only_scan_w_dow, time_only_fixture ) {
    _parameters.setGeneratingTableResults(true);
    _parameters.setPerformDayOfWeekAdjustment(true);
    _parameters.setReportAttributableRisk(true);
    _parameters.setAttributableRiskExposed(200);
    BOOST_REQUIRE_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

    std::string results_user_directory;
    run_analysis("test", results_user_directory, _parameters, _print);

    // open the power estimation file and confirm expected columns and values match expected for this data set
    std::string buffer;
    printString(buffer, "test%s", CSVDataFileWriter::CSV_FILE_EXT);
    std::ifstream stream;
    getFileStream(stream, buffer, results_user_directory);

    CSV_Row_t headers;
    getCSVRow(stream, headers);
    std::vector<std::string>::iterator itrAR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::ATTRIBUTABLE_RISK_FIELD);

    // check the expected values for this analysis
    double expected [16] = {0.21, 0.22, 0.22, 0.17, 0.2, 0.13, 0.19, 0.2, 0.1, 0.19, 0.17, 0.15, 0.12, 0.043, 0.085, 0.022};
    unsigned int dataRows=0, expectedRows=16;
    CSV_Row_t data;
    getCSVRow(stream, data);
    while (data.size()) {
        if (dataRows < expectedRows) {
            double ar; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrAR)).c_str(), ar) );
            BOOST_REQUIRE_CLOSE( ar, expected[dataRows], 0.001 );
        }
        ++dataRows;
        getCSVRow(stream, data);
    }
    if (dataRows != expectedRows) BOOST_FAIL( "expecting " << expectedRows << " data rows, got " << dataRows );
    stream.close();
}

/* Tests the expected values of the attributable risk data  with tree-time scan. */
BOOST_FIXTURE_TEST_CASE( test_attributable_risk_tree_time_scan_condition_node, tree_temporal_fixture ) {
    _parameters.setGeneratingTableResults(true);
    _parameters.setReportAttributableRisk(true);
    _parameters.setAttributableRiskExposed(200);
    BOOST_REQUIRE_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

    std::string results_user_directory;
    run_analysis("test", results_user_directory, _parameters, _print);

    // open the power estimation file and confirm expected columns and values match expected for this data set
    std::string buffer;
    printString(buffer, "test%s", CSVDataFileWriter::CSV_FILE_EXT);
    std::ifstream stream;
    getFileStream(stream, buffer, results_user_directory);

    CSV_Row_t headers;
    getCSVRow(stream, headers);
    std::vector<std::string>::iterator itrAR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::ATTRIBUTABLE_RISK_FIELD);

    // check the expected values for this analysis
    double expected [10] = {0.068, 0.018, 0.055, 0.033, 0.019, 0.019, 0.019, 0.0087, 0.0087, 0.024};
    unsigned int dataRows=0, expectedRows=10;
    CSV_Row_t data;
    getCSVRow(stream, data);
    while (data.size()) {
        if (dataRows < expectedRows) {
            double ar; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrAR)).c_str(), ar) );
            BOOST_REQUIRE_CLOSE( ar, expected[dataRows], 0.001 );
        }
        ++dataRows;
        getCSVRow(stream, data);
    }
    if (dataRows != expectedRows) BOOST_FAIL( "expecting " << expectedRows << " data rows, got " << dataRows );
    stream.close();
}

/* Tests the expected values of the attributable risk data  with tree-time scan. */
BOOST_FIXTURE_TEST_CASE( test_attributable_risk_tree_time_scan_condition_node_w_dow, tree_temporal_fixture ) {
    _parameters.setPerformDayOfWeekAdjustment(true);
    _parameters.setGeneratingTableResults(true);
    _parameters.setReportAttributableRisk(true);
    _parameters.setAttributableRiskExposed(200);
    BOOST_REQUIRE_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

    std::string results_user_directory;
    run_analysis("test", results_user_directory, _parameters, _print);

    // open the power estimation file and confirm expected columns and values match expected for this data set
    std::string buffer;
    printString(buffer, "test%s", CSVDataFileWriter::CSV_FILE_EXT);
    std::ifstream stream;
    getFileStream(stream, buffer, results_user_directory);

    CSV_Row_t headers;
    getCSVRow(stream, headers);
    std::vector<std::string>::iterator itrAR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::ATTRIBUTABLE_RISK_FIELD);

    // check the expected values for this analysis
    double expected [9] = {0.009, 0.0089, 0.018, 0.018, 0.046, 0.0085, 0.062, 0.0044, 0.0071};
    unsigned int dataRows=0, expectedRows=9;
    CSV_Row_t data;
    getCSVRow(stream, data);
    while (data.size()) {
        if (dataRows < expectedRows) {
            double ar; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrAR)).c_str(), ar) );
            BOOST_REQUIRE_CLOSE( ar, expected[dataRows], 0.001 );
        }
        ++dataRows;
        getCSVRow(stream, data);
    }
    if (dataRows != expectedRows) BOOST_FAIL( "expecting " << expectedRows << " data rows, got " << dataRows );
    stream.close();
}

/* Tests the expected values of the attributable risk data  with tree-time scan. */
BOOST_FIXTURE_TEST_CASE( test_attributable_risk_tree_time_scan_condition_nodetime, tree_temporal_fixture ) {
    _parameters.setConditionalType(Parameters::NODEANDTIME);
    _parameters.setGeneratingTableResults(true);
    _parameters.setReportAttributableRisk(true);
    _parameters.setAttributableRiskExposed(200);
    BOOST_REQUIRE_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

    std::string results_user_directory;
    run_analysis("test", results_user_directory, _parameters, _print);

    // open the power estimation file and confirm expected columns and values match expected for this data set
    std::string buffer;
    printString(buffer, "test%s", CSVDataFileWriter::CSV_FILE_EXT);
    std::ifstream stream;
    getFileStream(stream, buffer, results_user_directory);

    CSV_Row_t headers;
    getCSVRow(stream, headers);
    std::vector<std::string>::iterator itrAR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::ATTRIBUTABLE_RISK_FIELD);

    // check the expected values for this analysis
    double expected [10] = {0.068, 0.018, 0.055, 0.033, 0.019, 0.019, 0.019, 0.0087, 0.0087, 0.024};
    unsigned int dataRows=0, expectedRows=10;
    CSV_Row_t data;
    getCSVRow(stream, data);
    while (data.size()) {
        if (dataRows < expectedRows) {
            double ar; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrAR)).c_str(), ar) );
            BOOST_REQUIRE_CLOSE( ar, expected[dataRows], 0.001 );
        }
        ++dataRows;
        getCSVRow(stream, data);
    }
    if (dataRows != expectedRows) BOOST_FAIL( "expecting " << expectedRows << " data rows, got " << dataRows );
    stream.close();
}

/* Tests the expected values of the attributable risk data  with tree-time scan. */
BOOST_FIXTURE_TEST_CASE( test_attributable_risk_tree_time_scan_condition_nodetime_w_dow, tree_temporal_fixture ) {
    _parameters.setConditionalType(Parameters::NODEANDTIME);
    _parameters.setPerformDayOfWeekAdjustment(true);
    _parameters.setGeneratingTableResults(true);
    _parameters.setReportAttributableRisk(true);
    _parameters.setAttributableRiskExposed(200);
    BOOST_REQUIRE_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

    std::string results_user_directory;
    run_analysis("test", results_user_directory, _parameters, _print);

    // open the power estimation file and confirm expected columns and values match expected for this data set
    std::string buffer;
    printString(buffer, "test%s", CSVDataFileWriter::CSV_FILE_EXT);
    std::ifstream stream;
    getFileStream(stream, buffer, results_user_directory);

    CSV_Row_t headers;
    getCSVRow(stream, headers);
    std::vector<std::string>::iterator itrAR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::ATTRIBUTABLE_RISK_FIELD);

    // check the expected values for this analysis
    double expected [9] = {0.009, 0.0089, 0.018, 0.018, 0.046, 0.0085, 0.062, 0.0044, 0.0071};
    unsigned int dataRows=0, expectedRows=9;
    CSV_Row_t data;
    getCSVRow(stream, data);
    while (data.size()) {
        if (dataRows < expectedRows) {
            double ar; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrAR)).c_str(), ar) );
            BOOST_REQUIRE_CLOSE( ar, expected[dataRows], 0.001 );
        }
        ++dataRows;
        getCSVRow(stream, data);
    }
    if (dataRows != expectedRows) BOOST_FAIL( "expecting " << expectedRows << " data rows, got " << dataRows );
    stream.close();
}

BOOST_AUTO_TEST_SUITE_END()
