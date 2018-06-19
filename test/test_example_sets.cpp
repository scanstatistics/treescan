// project files
#include "fixture_examples.h"
#include "ScanRunner.h"
#include "Toolkit.h"
#include "DataFileWriter.h"
#include <boost/any.hpp>

/* Tests the expected results of the example data sets */

/* Test Suite for the Poisson example data set. */
BOOST_FIXTURE_TEST_SUITE( poisson_example_suite, poisson_fixture )

/* Tests the expected values of the data file writer with the unconditional Poisson model. */
BOOST_AUTO_TEST_CASE( test_poisson_unconditional ) {
    // set parameters to report csv data file
    _parameters.setConditionalType(Parameters::UNCONDITIONAL);
    _parameters.setGeneratingTableResults(true);
    _parameters.setPrintColumnHeaders(true);
    _parameters.setReportAttributableRisk(true);
    _parameters.setAttributableRiskExposed(200);

    BOOST_REQUIRE_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

    // run analysis
    std::string results_user_directory;
    run_analysis("test", results_user_directory, _parameters, _print);

    // open the tabular results file and confirm expected columns and values match expected for this data set
    std::string buffer;
    std::ifstream stream;
    stream.open(CutsRecordWriter::getFilename(_parameters, buffer).c_str());
    if (!stream) throw std::exception("could not open file");

    CSV_Row_t headers;
    getCSVRow(stream, headers);
    if (headers.size() != static_cast<size_t>(10)) BOOST_FAIL( "expecting 10 columns, got " << headers.size() );

    // check the expected values
    unsigned int dataRows=0;
    if (dataRows != 0) BOOST_FAIL( "expecting 0 data rows, got " << dataRows );
    stream.close();
}

/* Tests the expected values of the data file writer with the conditional Poisson model. */
BOOST_AUTO_TEST_CASE( test_poisson_conditional ) {
    // set parameters to report csv data file
    _parameters.setConditionalType(Parameters::TOTALCASES);
    _parameters.setGeneratingTableResults(true);
    _parameters.setPrintColumnHeaders(true);
    _parameters.setReportAttributableRisk(true);
    _parameters.setAttributableRiskExposed(200);

    BOOST_REQUIRE_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

    // run analysis
    std::string results_user_directory;
    run_analysis("test", results_user_directory, _parameters, _print);

    // open the tabular results file and confirm expected columns and values match expected for this data set
    std::string buffer;
    std::ifstream stream;
    stream.open(CutsRecordWriter::getFilename(_parameters, buffer).c_str());
    if (!stream) throw std::exception("could not open file");

    CSV_Row_t headers;
    getCSVRow(stream, headers);
    if (headers.size() != static_cast<size_t>(10)) BOOST_FAIL( "expecting 10 columns, got " << headers.size() );
    std::vector<std::string>::iterator itrCutNum = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::CUT_NUM_FIELD);
    std::vector<std::string>::iterator itrNodeId = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::NODE_ID_FIELD);
    std::vector<std::string>::iterator itrObserved = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::OBSERVED_CASES_FIELD);
    std::vector<std::string>::iterator itrExpected = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::EXPECTED_FIELD);
    std::vector<std::string>::iterator itrRR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::RELATIVE_RISK_FIELD);
    std::vector<std::string>::iterator itrExcess = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::EXCESS_CASES_FIELD);
    std::vector<std::string>::iterator itrAR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::ATTRIBUTABLE_RISK_FIELD);
    std::vector<std::string>::iterator itrLLR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::LOG_LIKL_RATIO_FIELD);
    std::vector<std::string>::iterator itrPValue = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::P_VALUE_FLD);

    // check the expected values
    unsigned int dataRows=0;
    CSV_Row_t data;
    getCSVRow(stream, data);
    while (data.size()) {
        ++dataRows;
        unsigned int cut_num; BOOST_CHECK( string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrCutNum)).c_str(), cut_num) );
        unsigned int observed; BOOST_CHECK( string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrObserved)).c_str(), observed) );
        double expected; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrExpected)).c_str(), expected) );
        double rr; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrRR)).c_str(), rr) );
        double excess; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrExcess)).c_str(), excess) );
        double ar; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrAR)).c_str(), ar) );
        double llr; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrLLR)).c_str(), llr) );
        double p_value; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrPValue)).c_str(), p_value) );

        BOOST_CHECK_EQUAL( cut_num, dataRows );
        switch (dataRows) {
            case 6 : BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "Leaf9" );
                     BOOST_CHECK_EQUAL( observed, static_cast<unsigned int>(14) );
                     BOOST_CHECK_CLOSE( expected, 2.68, 0.001 );
                     BOOST_CHECK_CLOSE( rr, 6.66, 0.001 );
                     BOOST_CHECK_CLOSE( excess, 11.9, 0.001 );
                     BOOST_CHECK_CLOSE( ar, 0.059, 0.00001 );
                     BOOST_CHECK_CLOSE( llr, 13.136308, 0.00001 );
                     BOOST_CHECK_CLOSE( p_value, 0.0001, 0.0001 ); break;
            case 3 : BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "Node3" );
                     BOOST_CHECK_EQUAL( observed, static_cast<unsigned int>(33) );
                     BOOST_CHECK_CLOSE( expected, 16.1, 0.001 );
                     BOOST_CHECK_CLOSE( rr, 3.63, 0.001 );
                     BOOST_CHECK_CLOSE( excess, 23.9, 0.001 );
                     BOOST_CHECK_CLOSE( ar, 0.12, 0.00001 );
                     BOOST_CHECK_CLOSE( llr, 11.148394, 0.00001 );
                     BOOST_CHECK_CLOSE( p_value, 0.0001, 0.0001 ); break;
            case 1 :BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "Leaf4" );
                     BOOST_CHECK_EQUAL( observed, static_cast<unsigned int>(6) );
                     BOOST_CHECK_CLOSE( expected, 2.68, 0.001 );
                     BOOST_CHECK_CLOSE( rr, 2.39, 0.001 );
                     BOOST_CHECK_CLOSE( excess, 3.49, 0.001 );
                     BOOST_CHECK_CLOSE( ar, 0.017, 0.00001 );
                     BOOST_CHECK_CLOSE( llr, 1.619486, 0.00001 );
                     BOOST_CHECK_CLOSE( p_value, 0.4879, 0.0001 ); break;
            case 5 :BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "Leaf8" );
                     BOOST_CHECK_EQUAL( observed, static_cast<unsigned int>(9) );
                     BOOST_CHECK_CLOSE( expected, 5.37, 0.001 );
                     BOOST_CHECK_CLOSE( rr, 1.81, 0.001 );
                     BOOST_CHECK_CLOSE( excess, 4.03, 0.001 );
                     BOOST_CHECK_CLOSE( ar, 0.02, 0.00001 );
                     BOOST_CHECK_CLOSE( llr, 1.156791, 0.00001 );
                     BOOST_CHECK_CLOSE( p_value, 0.6400, 0.64 ); break;
            default : break;
        }
        getCSVRow(stream, data);
    }
    if (dataRows != 6) BOOST_FAIL( "expecting 6 data rows, got " << dataRows );
    stream.close();
}

BOOST_AUTO_TEST_SUITE_END()

/* Test Suite for the Bernoulli example data set. */
BOOST_FIXTURE_TEST_SUITE( bernoulli_example_suite, bernoulli_fixture )

/* Tests the expected values of the data file writer with the unconditional Poisson model. */
BOOST_AUTO_TEST_CASE( test_bernoulli_unconditional ) {
    // set parameters to report csv data file
    _parameters.setConditionalType(Parameters::UNCONDITIONAL);
    _parameters.setGeneratingTableResults(true);
    _parameters.setPrintColumnHeaders(true);
    _parameters.setReportAttributableRisk(true);
    _parameters.setAttributableRiskExposed(200);

    BOOST_REQUIRE_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

    // run analysis
    std::string results_user_directory;
    run_analysis("test", results_user_directory, _parameters, _print);

    // open the tabular results file and confirm expected columns and values match expected for this data set
    std::string buffer;
    std::ifstream stream;
    stream.open(CutsRecordWriter::getFilename(_parameters, buffer).c_str());
    if (!stream) throw std::exception("could not open file");

    CSV_Row_t headers;
    getCSVRow(stream, headers);
    if (headers.size() != static_cast<size_t>(11)) BOOST_FAIL( "expecting 11 columns, got " << headers.size() );
    std::vector<std::string>::iterator itrCutNum = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::CUT_NUM_FIELD);
    std::vector<std::string>::iterator itrNodeId = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::NODE_ID_FIELD);
    std::vector<std::string>::iterator itrObserved = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::OBSERVATIONS_FIELD);
    std::vector<std::string>::iterator itrCases = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::CASES_FIELD);
    std::vector<std::string>::iterator itrExpected = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::EXPECTED_FIELD);
    std::vector<std::string>::iterator itrRR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::RELATIVE_RISK_FIELD);
    std::vector<std::string>::iterator itrExcess = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::EXCESS_CASES_FIELD);
    std::vector<std::string>::iterator itrAR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::ATTRIBUTABLE_RISK_FIELD);
    std::vector<std::string>::iterator itrLLR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::LOG_LIKL_RATIO_FIELD);
    std::vector<std::string>::iterator itrPValue = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::P_VALUE_FLD);

    // check the expected values
    unsigned int dataRows=0;
    CSV_Row_t data;
    getCSVRow(stream, data);
    while (data.size()) {
        ++dataRows;
        unsigned int cut_num; BOOST_CHECK( string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrCutNum)).c_str(), cut_num) );
        unsigned int observed; BOOST_CHECK( string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrObserved)).c_str(), observed) );
        unsigned int cases; BOOST_CHECK( string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrCases)).c_str(), cases) );
        double expected; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrExpected)).c_str(), expected) );
        double rr; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrRR)).c_str(), rr) );
        double excess; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrExcess)).c_str(), excess) );
        double ar; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrAR)).c_str(), ar) );
        double llr; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrLLR)).c_str(), llr) );
        double p_value; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrPValue)).c_str(), p_value) );

        BOOST_CHECK_EQUAL( cut_num, dataRows );
        switch (dataRows) {
            case 6 : BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "Node3" );
                     BOOST_CHECK_EQUAL( observed, static_cast<unsigned int>(45) );
                     BOOST_CHECK_EQUAL( cases, static_cast<unsigned int>(33) );
                     BOOST_CHECK_CLOSE( expected, 22.5, 0.001 );
                     BOOST_CHECK_CLOSE( rr, 1.47, 0.001 );
                     BOOST_CHECK_CLOSE( excess, 10.5, 0.001 );
                     BOOST_CHECK_CLOSE( ar, 0.052, 0.00001 );
                     BOOST_CHECK_CLOSE( llr, 5.09544, 0.00001 );
                     BOOST_CHECK_CLOSE( p_value, 0.008, 0.0001 ); break;
            case 9 : BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "Leaf9" );
                     BOOST_CHECK_EQUAL( observed, static_cast<unsigned int>(16) );
                     BOOST_CHECK_EQUAL( cases, static_cast<unsigned int>(14) );
                     BOOST_CHECK_CLOSE( expected, 8.00, 0.001 );
                     BOOST_CHECK_CLOSE( rr, 1.75, 0.001 );
                     BOOST_CHECK_CLOSE( excess, 6.0, 0.001 );
                     BOOST_CHECK_CLOSE( ar, 0.03, 0.00001 );
                     BOOST_CHECK_CLOSE( llr, 5.062032, 0.00001 );
                     BOOST_CHECK_CLOSE( p_value, 0.01, 0.0001 ); break;
            case 2  :BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "Node2" );
                     BOOST_CHECK_EQUAL( observed, static_cast<unsigned int>(30) );
                     BOOST_CHECK_EQUAL( cases, static_cast<unsigned int>(17) );
                     BOOST_CHECK_CLOSE( expected, 15.0, 0.001 );
                     BOOST_CHECK_CLOSE( rr, 1.13, 0.001 );
                     BOOST_CHECK_CLOSE( excess, 2.0, 0.001 );
                     BOOST_CHECK_CLOSE( ar, 0.01, 0.00001 );
                     BOOST_CHECK_CLOSE( llr, 0.267462, 0.00001 );
                     BOOST_CHECK_CLOSE( p_value, 0.956, 0.0001 ); break;
            case 4  :BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "Leaf5" );
                     BOOST_CHECK_EQUAL( observed, static_cast<unsigned int>(7) );
                     BOOST_CHECK_EQUAL( cases, static_cast<unsigned int>(4) );
                     BOOST_CHECK_CLOSE( expected, 3.5, 0.001 );
                     BOOST_CHECK_CLOSE( rr, 1.14, 0.001 );
                     BOOST_CHECK_CLOSE( excess, 0.5, 0.001 );
                     BOOST_CHECK_CLOSE( ar, 0.0025, 0.00001 );
                     BOOST_CHECK_CLOSE( llr, 0.0716735, 0.00001 );
                     BOOST_CHECK_CLOSE( p_value, 0.994, 0.0001 ); break;
            default : break;
        }
        getCSVRow(stream, data);
    }
    if (dataRows != 9) BOOST_FAIL( "expecting 9 data rows, got " << dataRows );
    stream.close();
}

/* Tests the expected values of the data file writer with the conditional Bernoulli model. */
BOOST_AUTO_TEST_CASE( test_bernoulli_conditional ) {
    // set parameters to report csv data file
    _parameters.setConditionalType(Parameters::TOTALCASES);
    _parameters.setGeneratingTableResults(true);
    _parameters.setPrintColumnHeaders(true);
    _parameters.setReportAttributableRisk(true);
    _parameters.setAttributableRiskExposed(200);

    BOOST_REQUIRE_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

    // run analysis
    std::string results_user_directory;
    run_analysis("test", results_user_directory, _parameters, _print);

    // open the tabular results file and confirm expected columns and values match expected for this data set
    std::string buffer;
    std::ifstream stream;
    stream.open(CutsRecordWriter::getFilename(_parameters, buffer).c_str());
    if (!stream) throw std::exception("could not open file");

    CSV_Row_t headers;
    getCSVRow(stream, headers);
    if (headers.size() != static_cast<size_t>(11)) BOOST_FAIL( "expecting 11 columns, got " << headers.size() );
    std::vector<std::string>::iterator itrCutNum = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::CUT_NUM_FIELD);
    std::vector<std::string>::iterator itrNodeId = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::NODE_ID_FIELD);
    std::vector<std::string>::iterator itrObserved = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::OBSERVATIONS_FIELD);
    std::vector<std::string>::iterator itrCases = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::CASES_FIELD);
    std::vector<std::string>::iterator itrExpected = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::EXPECTED_FIELD);
    std::vector<std::string>::iterator itrRR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::RELATIVE_RISK_FIELD);
    std::vector<std::string>::iterator itrExcess = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::EXCESS_CASES_FIELD);
    std::vector<std::string>::iterator itrAR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::ATTRIBUTABLE_RISK_FIELD);
    std::vector<std::string>::iterator itrLLR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::LOG_LIKL_RATIO_FIELD);
    std::vector<std::string>::iterator itrPValue = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::P_VALUE_FLD);

    // check the expected values
    unsigned int dataRows=0;
    CSV_Row_t data;
    getCSVRow(stream, data);
    while (data.size()) {
        ++dataRows;
        unsigned int cut_num; BOOST_CHECK( string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrCutNum)).c_str(), cut_num) );
        unsigned int observed; BOOST_CHECK( string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrObserved)).c_str(), observed) );
        unsigned int cases; BOOST_CHECK( string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrCases)).c_str(), cases) );
        double expected; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrExpected)).c_str(), expected) );
        double rr; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrRR)).c_str(), rr) );
        double excess; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrExcess)).c_str(), excess) );
        double ar; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrAR)).c_str(), ar) );
        double llr; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrLLR)).c_str(), llr) );
        double p_value; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrPValue)).c_str(), p_value) );

        BOOST_CHECK_EQUAL( cut_num, dataRows );
        switch (dataRows) {
            case 3 : BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "Node3" );
                     BOOST_CHECK_EQUAL( observed, static_cast<unsigned int>(45) );
                     BOOST_CHECK_EQUAL( cases, static_cast<unsigned int>(33) );
                     BOOST_CHECK_CLOSE( expected, 25.78, 0.001 );
                     BOOST_CHECK_CLOSE( rr, 1.7, 0.001 );
                     BOOST_CHECK_CLOSE( excess, 13.59, 0.001 );
                     BOOST_CHECK_CLOSE( ar, 0.068, 0.00001 );
                     BOOST_CHECK_CLOSE( llr, 4.55287, 0.00001 );
                     BOOST_CHECK_CLOSE( p_value, 0.031, 0.0001 ); break;
            case 6 : BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "Leaf9" );
                     BOOST_CHECK_EQUAL( observed, static_cast<unsigned int>(16) );
                     BOOST_CHECK_EQUAL( cases, static_cast<unsigned int>(14) );
                     BOOST_CHECK_CLOSE( expected, 9.17, 0.001 );
                     BOOST_CHECK_CLOSE( rr, 1.71, 0.001 );
                     BOOST_CHECK_CLOSE( excess, 5.8, 0.001 );
                     BOOST_CHECK_CLOSE( ar, 0.029, 0.00001 );
                     BOOST_CHECK_CLOSE( llr, 4.062552, 0.00001 );
                     BOOST_CHECK_CLOSE( p_value, 0.076, 0.0001 ); break;
            case 1 :BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "Leaf4" );
                     BOOST_CHECK_EQUAL( observed, static_cast<unsigned int>(8) );
                     BOOST_CHECK_EQUAL( cases, static_cast<unsigned int>(6) );
                     BOOST_CHECK_CLOSE( expected, 4.58, 0.001 );
                     BOOST_CHECK_CLOSE( rr, 1.35, 0.001 );
                     BOOST_CHECK_CLOSE( excess, 1.55, 0.001 );
                     BOOST_CHECK_CLOSE( ar, 0.0077, 0.00001 );
                     BOOST_CHECK_CLOSE( llr, 0.591424, 0.00001 );
                     BOOST_CHECK_CLOSE( p_value, 0.937, 0.0001 ); break;
            case 5 :BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "Leaf8" );
                     BOOST_CHECK_EQUAL( observed, static_cast<unsigned int>(13) );
                     BOOST_CHECK_EQUAL( cases, static_cast<unsigned int>(9) );
                     BOOST_CHECK_CLOSE( expected, 7.45, 0.001 );
                     BOOST_CHECK_CLOSE( rr, 1.25, 0.001 );
                     BOOST_CHECK_CLOSE( excess, 1.8, 0.001 );
                     BOOST_CHECK_CLOSE( ar, 0.009, 0.00001 );
                     BOOST_CHECK_CLOSE( llr, 0.4512, 0.00001 );
                     BOOST_CHECK_CLOSE( p_value, 0.966, 0.0001 ); break;
            default : break;
        }
        getCSVRow(stream, data);
    }
    if (dataRows != 6) BOOST_FAIL( "expecting 6 data rows, got " << dataRows );
    stream.close();
}

BOOST_AUTO_TEST_SUITE_END()

/* Test Suite for the tree-time scan example data set. */
BOOST_FIXTURE_TEST_SUITE( tree_time_example_suite, tree_temporal_fixture )

/* Tests the expected values of the data file writer with the tree-time scan conditioned on node. */
BOOST_AUTO_TEST_CASE( test_tree_time_condition_node ) {
    // set parameters to report csv data file
    _parameters.setConditionalType(Parameters::NODE);
    _parameters.setModelType(Parameters::UNIFORM);
    _parameters.setGeneratingTableResults(true);
    _parameters.setPrintColumnHeaders(true);
    _parameters.setReportAttributableRisk(true);
    _parameters.setAttributableRiskExposed(200);

    BOOST_REQUIRE_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

    // run analysis
    std::string results_user_directory;
    run_analysis("test", results_user_directory, _parameters, _print);

    // open the tabular results file and confirm expected columns and values match expected for this data set
    std::string buffer;
    std::ifstream stream;
    stream.open(CutsRecordWriter::getFilename(_parameters, buffer).c_str());
    if (!stream) throw std::exception("could not open file");

    CSV_Row_t headers;
    getCSVRow(stream, headers);
    if (headers.size() != static_cast<size_t>(13)) BOOST_FAIL( "expecting 13 columns, got " << headers.size() );
    std::vector<std::string>::iterator itrCutNum = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::CUT_NUM_FIELD);
    std::vector<std::string>::iterator itrNodeId = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::NODE_ID_FIELD);
    std::vector<std::string>::iterator itrNodeCases = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::NODE_CASES_FIELD);
    std::vector<std::string>::iterator itrWndStart = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::START_WINDOW_FIELD);
    std::vector<std::string>::iterator itrWndEnd = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::END_WINDOW_FIELD);
    std::vector<std::string>::iterator itrWndCase = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::WNDW_CASES_FIELD);
    std::vector<std::string>::iterator itrExpected = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::EXPECTED_CASES_FIELD);
    std::vector<std::string>::iterator itrRR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::RELATIVE_RISK_FIELD);
    std::vector<std::string>::iterator itrExcess = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::EXCESS_CASES_FIELD);
    std::vector<std::string>::iterator itrAR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::ATTRIBUTABLE_RISK_FIELD);
    std::vector<std::string>::iterator itrLLR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::LOG_LIKL_RATIO_FIELD);
    std::vector<std::string>::iterator itrPValue = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::P_VALUE_FLD);

    // check the expected values
    unsigned int dataRows=0;
    CSV_Row_t data;
    getCSVRow(stream, data);
    while (data.size()) {
        ++dataRows;
        unsigned int cut_num; BOOST_CHECK( string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrCutNum)).c_str(), cut_num) );
        unsigned int nodecases; BOOST_CHECK( string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrNodeCases)).c_str(), nodecases) );
        unsigned int wndstart; BOOST_CHECK( string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrWndStart)).c_str(), wndstart) );
        unsigned int wndend; BOOST_CHECK( string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrWndEnd)).c_str(), wndend) );
        unsigned int wndcases; BOOST_CHECK( string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrWndCase)).c_str(), wndcases) );
        double expected; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrExpected)).c_str(), expected) );
        double rr; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrRR)).c_str(), rr, false) );
        double excess; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrExcess)).c_str(), excess) );
        double ar; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrAR)).c_str(), ar) );
        double llr; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrLLR)).c_str(), llr) );
        double p_value; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrPValue)).c_str(), p_value) );

        BOOST_CHECK_EQUAL( cut_num, dataRows );
        switch (dataRows) {
            case 9 :  BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "Node3" );
                      BOOST_CHECK_EQUAL( nodecases, static_cast<unsigned int>(46) );
                      BOOST_CHECK_EQUAL( wndstart, static_cast<unsigned int>(4) );
                      BOOST_CHECK_EQUAL( wndend, static_cast<unsigned int>(5) );
                      BOOST_CHECK_EQUAL( wndcases, static_cast<unsigned int>(21) );
                      BOOST_CHECK_CLOSE( expected, 3.29, 0.001 );
                      BOOST_CHECK_CLOSE( rr, 10.92, 0.001 );
                      BOOST_CHECK_CLOSE( excess, 19.08, 0.001 );
                      BOOST_CHECK_CLOSE( ar, 0.095, 0.00001 );
                      BOOST_CHECK_CLOSE( llr, 25.562266, 0.00001 );
                      BOOST_CHECK_CLOSE( p_value, 0.001, 0.0001 ); break;
            case 1 :  BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "Root" );
                      BOOST_CHECK_EQUAL( nodecases, static_cast<unsigned int>(97) );
                      BOOST_CHECK_EQUAL( wndstart, static_cast<unsigned int>(2) );
                      BOOST_CHECK_EQUAL( wndend, static_cast<unsigned int>(7) );
                      BOOST_CHECK_EQUAL( wndcases, static_cast<unsigned int>(52) );
                      BOOST_CHECK_CLOSE( expected, 20.79, 0.001 );
                      BOOST_CHECK_CLOSE( rr, 4.24, 0.001 );
                      BOOST_CHECK_CLOSE( excess, 39.73, 0.001 );
                      BOOST_CHECK_CLOSE( ar, 0.2, 0.00001 );
                      BOOST_CHECK_CLOSE( llr, 23.972955, 0.00001 );
                      BOOST_CHECK_CLOSE( p_value, 0.001, 0.0001 ); break;
            case 3  : BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "Leaf2" );
                      BOOST_CHECK_EQUAL( nodecases, static_cast<unsigned int>(5) );
                      BOOST_CHECK_EQUAL( wndstart, static_cast<unsigned int>(6) );
                      BOOST_CHECK_EQUAL( wndend, static_cast<unsigned int>(7) );
                      BOOST_CHECK_EQUAL( wndcases, static_cast<unsigned int>(2) );
                      BOOST_CHECK_CLOSE( expected, 0.36, 0.001 );
                      BOOST_CHECK_CLOSE( rr, 8.67, 0.001 );
                      BOOST_CHECK_CLOSE( excess, 1.77, 0.001 );
                      BOOST_CHECK_CLOSE( ar, 0.0088, 0.00001 );
                      BOOST_CHECK_CLOSE( llr, 2.13538, 0.00001 );
                      BOOST_CHECK_CLOSE( p_value, 0.998, 0.0001 ); break;
            case 7  : BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "Leaf7" );
                      BOOST_CHECK_EQUAL( nodecases, static_cast<unsigned int>(11) );
                      BOOST_CHECK_EQUAL( wndstart, static_cast<unsigned int>(6) );
                      BOOST_CHECK_EQUAL( wndend, static_cast<unsigned int>(7) );
                      BOOST_CHECK_EQUAL( wndcases, static_cast<unsigned int>(3) );
                      BOOST_CHECK_CLOSE( expected, 0.79, 0.001 );
                      BOOST_CHECK_CLOSE( rr, 4.88, 0.001 );
                      BOOST_CHECK_CLOSE( excess, 2.38, 0.001 );
                      BOOST_CHECK_CLOSE( ar, 0.012, 0.00001 );
                      BOOST_CHECK_CLOSE( llr, 2.064557, 0.00001 );
                      BOOST_CHECK_CLOSE( p_value, 0.999, 0.0001 ); break;
            default : break;
        }
        getCSVRow(stream, data);
    }
    if (dataRows != 13) BOOST_FAIL( "expecting 13 data rows, got " << dataRows );
    stream.close();
}

/* Tests the expected values of the data file writer with the tree-time scan conditioned on node, with day of week adjustment. */
BOOST_AUTO_TEST_CASE( test_tree_time_condition_node_day_of_week_adjustment ) {
    // set parameters to report csv data file
    _parameters.setConditionalType(Parameters::NODE);
    _parameters.setModelType(Parameters::UNIFORM);
    _parameters.setPerformDayOfWeekAdjustment(true);
    _parameters.setGeneratingTableResults(true);
    _parameters.setPrintColumnHeaders(true);
    _parameters.setReportAttributableRisk(true);
    _parameters.setAttributableRiskExposed(200);

    BOOST_REQUIRE_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

    // run analysis
    std::string results_user_directory;
    run_analysis("test", results_user_directory, _parameters, _print);

    // open the tabular results file and confirm expected columns and values match expected for this data set
    std::string buffer;
    std::ifstream stream;
    stream.open(CutsRecordWriter::getFilename(_parameters, buffer).c_str());
    if (!stream) throw std::exception("could not open file");

    CSV_Row_t headers;
    getCSVRow(stream, headers);
    if (headers.size() != static_cast<size_t>(13)) BOOST_FAIL( "expecting 13 columns, got " << headers.size() );
    std::vector<std::string>::iterator itrCutNum = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::CUT_NUM_FIELD);
    std::vector<std::string>::iterator itrNodeId = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::NODE_ID_FIELD);
    std::vector<std::string>::iterator itrNodeCases = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::NODE_CASES_FIELD);
    std::vector<std::string>::iterator itrWndStart = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::START_WINDOW_FIELD);
    std::vector<std::string>::iterator itrWndEnd = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::END_WINDOW_FIELD);
    std::vector<std::string>::iterator itrWndCase = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::WNDW_CASES_FIELD);
    std::vector<std::string>::iterator itrExpected = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::EXPECTED_CASES_FIELD);
    std::vector<std::string>::iterator itrRR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::RELATIVE_RISK_FIELD);
    std::vector<std::string>::iterator itrExcess = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::EXCESS_CASES_FIELD);
    std::vector<std::string>::iterator itrAR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::ATTRIBUTABLE_RISK_FIELD);
    std::vector<std::string>::iterator itrTestStat = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::TEST_STATISTIC_FIELD);
    std::vector<std::string>::iterator itrPValue = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::P_VALUE_FLD);

    // check the expected values
    unsigned int dataRows=0;
    CSV_Row_t data;
    getCSVRow(stream, data);
    while (data.size()) {
        ++dataRows;
        unsigned int cut_num; BOOST_CHECK( string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrCutNum)).c_str(), cut_num) );
        unsigned int nodecases; BOOST_CHECK( string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrNodeCases)).c_str(), nodecases) );
        unsigned int wndstart; BOOST_CHECK( string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrWndStart)).c_str(), wndstart) );
        unsigned int wndend; BOOST_CHECK( string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrWndEnd)).c_str(), wndend) );
        unsigned int wndcases; BOOST_CHECK( string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrWndCase)).c_str(), wndcases) );
        double expected; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrExpected)).c_str(), expected) );
        double rr; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrRR)).c_str(), rr, false) );
        double excess; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrExcess)).c_str(), excess) );
        double ar; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrAR)).c_str(), ar) );
        double teststat; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrTestStat)).c_str(), teststat) );
        double p_value; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrPValue)).c_str(), p_value) );

        BOOST_CHECK_EQUAL( cut_num, dataRows );
        switch (dataRows) {
            case 1 :  BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "Root" );
                      BOOST_CHECK_EQUAL( nodecases, static_cast<unsigned int>(97) );
                      BOOST_CHECK_EQUAL( wndstart, static_cast<unsigned int>(1) );
                      BOOST_CHECK_EQUAL( wndend, static_cast<unsigned int>(7) );
                      BOOST_CHECK_EQUAL( wndcases, static_cast<unsigned int>(56) );
                      BOOST_CHECK_CLOSE( expected, 23.65, 0.001 );
                      BOOST_CHECK_CLOSE( rr, 4.24, 0.001 );
                      BOOST_CHECK_CLOSE( excess, 42.78, 0.001 );
                      BOOST_CHECK_CLOSE( ar, 0.21, 0.001 );
                      BOOST_CHECK_CLOSE( teststat, 24.422872, 0.00001 );
                      BOOST_CHECK_CLOSE( p_value, 0.001, 0.0001 ); break;
            case 9 :  BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "Node3" );
                      BOOST_CHECK_EQUAL( nodecases, static_cast<unsigned int>(46) );
                      BOOST_CHECK_EQUAL( wndstart, static_cast<unsigned int>(4) );
                      BOOST_CHECK_EQUAL( wndend, static_cast<unsigned int>(5) );
                      BOOST_CHECK_EQUAL( wndcases, static_cast<unsigned int>(21) );
                      BOOST_CHECK_CLOSE( expected, 3.91, 0.001 );
                      BOOST_CHECK_CLOSE( rr, 9.04, 0.001 );
                      BOOST_CHECK_CLOSE( excess, 18.68, 0.001 );
                      BOOST_CHECK_CLOSE( ar, 0.093, 0.001 );
                      BOOST_CHECK_CLOSE( teststat, 19.874551, 0.00001 );
                      BOOST_CHECK_CLOSE( p_value, 0.001, 0.0001 ); break;
            case 3   :BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "Leaf2" );
                      BOOST_CHECK_EQUAL( nodecases, static_cast<unsigned int>(5) );
                      BOOST_CHECK_EQUAL( wndstart, static_cast<unsigned int>(7) );
                      BOOST_CHECK_EQUAL( wndend, static_cast<unsigned int>(8) );
                      BOOST_CHECK_EQUAL( wndcases, static_cast<unsigned int>(2) );
                      BOOST_CHECK_CLOSE( expected, 0.32, 0.001 );
                      BOOST_CHECK_CLOSE( rr, 9.85, 0.001 );
                      BOOST_CHECK_CLOSE( excess, 1.8, 0.001 );
                      BOOST_CHECK_CLOSE( ar, 0.009, 0.001 );
                      BOOST_CHECK_CLOSE( teststat, 2.015681, 0.00001 );
                      BOOST_CHECK_CLOSE( p_value, 0.97, 0.0001 ); break;
            case 7   :BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "Leaf7" );
                      BOOST_CHECK_EQUAL( nodecases, static_cast<unsigned int>(11) );
                      BOOST_CHECK_EQUAL( wndstart, static_cast<unsigned int>(6) );
                      BOOST_CHECK_EQUAL( wndend, static_cast<unsigned int>(7) );
                      BOOST_CHECK_EQUAL( wndcases, static_cast<unsigned int>(3) );
                      BOOST_CHECK_CLOSE( expected, 0.82, 0.001 );
                      BOOST_CHECK_CLOSE( rr, 4.64, 0.001 );
                      BOOST_CHECK_CLOSE( excess, 2.35, 0.001 );
                      BOOST_CHECK_CLOSE( ar, 0.012, 0.001 );
                      BOOST_CHECK_CLOSE( teststat, 1.73029, 0.00001 );
                      BOOST_CHECK_CLOSE( p_value, 0.995, 0.0001 ); break;
            default : break;
        }
        getCSVRow(stream, data);
    }
    if (dataRows != 13) BOOST_FAIL( "expecting 13 data rows, got " << dataRows );
    stream.close();
}

/* Tests the expected values of the data file writer with the tree-time scan conditioned on node. */
BOOST_AUTO_TEST_CASE( test_tree_time_condition_node_and_time ) {
    // set parameters to report csv data file
    _parameters.setConditionalType(Parameters::NODEANDTIME);
    _parameters.setGeneratingTableResults(true);
    _parameters.setPrintColumnHeaders(true);
    _parameters.setReportAttributableRisk(true);
    _parameters.setAttributableRiskExposed(200);

    BOOST_REQUIRE_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

    // run analysis
    std::string results_user_directory;
    run_analysis("test", results_user_directory, _parameters, _print);

    // open the tabular results file and confirm expected columns and values match expected for this data set
    std::string buffer;
    std::ifstream stream;
    stream.open(CutsRecordWriter::getFilename(_parameters, buffer).c_str());
    if (!stream) throw std::exception("could not open file");

    CSV_Row_t headers;
    getCSVRow(stream, headers);
    if (headers.size() != static_cast<size_t>(13)) BOOST_FAIL( "expecting 13 columns, got " << headers.size() );
    std::vector<std::string>::iterator itrCutNum = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::CUT_NUM_FIELD);
    std::vector<std::string>::iterator itrNodeId = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::NODE_ID_FIELD);
    std::vector<std::string>::iterator itrNodeCases = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::NODE_CASES_FIELD);
    std::vector<std::string>::iterator itrWndStart = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::START_WINDOW_FIELD);
    std::vector<std::string>::iterator itrWndEnd = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::END_WINDOW_FIELD);
    std::vector<std::string>::iterator itrWndCase = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::WNDW_CASES_FIELD);
    std::vector<std::string>::iterator itrExpected = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::EXPECTED_CASES_FIELD);
    std::vector<std::string>::iterator itrRR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::RELATIVE_RISK_FIELD);
    std::vector<std::string>::iterator itrExcess = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::EXCESS_CASES_FIELD);
    std::vector<std::string>::iterator itrAR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::ATTRIBUTABLE_RISK_FIELD);
    std::vector<std::string>::iterator itrTestStat = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::TEST_STATISTIC_FIELD);
    std::vector<std::string>::iterator itrPValue = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::P_VALUE_FLD);

    // check the expected values
    unsigned int dataRows=0;
    CSV_Row_t data;
    getCSVRow(stream, data);
    while (data.size()) {
        ++dataRows;
        unsigned int cut_num; BOOST_CHECK( string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrCutNum)).c_str(), cut_num) );
        unsigned int nodecases; BOOST_CHECK( string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrNodeCases)).c_str(), nodecases) );
        unsigned int wndstart; BOOST_CHECK( string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrWndStart)).c_str(), wndstart) );
        unsigned int wndend; BOOST_CHECK( string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrWndEnd)).c_str(), wndend) );
        unsigned int wndcases; BOOST_CHECK( string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrWndCase)).c_str(), wndcases) );
        double expected; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrExpected)).c_str(), expected) );
        double ar; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrAR)).c_str(), ar) );
        double rr; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrRR)).c_str(), rr, false) );
        double excess; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrExcess)).c_str(), excess) );
        double teststat; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrTestStat)).c_str(), teststat) );
        double p_value; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrPValue)).c_str(), p_value) );

        BOOST_CHECK_EQUAL( cut_num, dataRows );
        switch (dataRows) {
            case 6 :  BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "Node3" );
                      BOOST_CHECK_EQUAL( nodecases, static_cast<unsigned int>(46) );
                      BOOST_CHECK_EQUAL( wndstart, static_cast<unsigned int>(4) );
                      BOOST_CHECK_EQUAL( wndend, static_cast<unsigned int>(5) );
                      BOOST_CHECK_EQUAL( wndcases, static_cast<unsigned int>(21) );
                      BOOST_CHECK_CLOSE( expected, 10.43, 0.001 );
                      BOOST_CHECK_CLOSE( ar, 0.068, 0.00001 );
                      BOOST_CHECK_CLOSE( rr, 42.0, 0.001 );
                      BOOST_CHECK_CLOSE( excess, 20.5, 0.001 );
                      BOOST_CHECK_CLOSE( teststat, 4.796446, 0.00001 );
                      BOOST_CHECK_CLOSE( p_value, 0.021, 0.0001 ); break;
            case 3 :  BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "Leaf5" );
                      BOOST_CHECK_EQUAL( nodecases, static_cast<unsigned int>(7) );
                      BOOST_CHECK_EQUAL( wndstart, static_cast<unsigned int>(2) );
                      BOOST_CHECK_EQUAL( wndend, static_cast<unsigned int>(3) );
                      BOOST_CHECK_EQUAL( wndcases, static_cast<unsigned int>(4) );
                      BOOST_CHECK_CLOSE( expected, 0.87, 0.001 );
                      BOOST_CHECK_CLOSE( rr, 13.67, 0.001 );
                      BOOST_CHECK_CLOSE( excess, 3.71, 0.001 );
                      BOOST_CHECK_CLOSE( ar, 0.018, 0.00001 );
                      BOOST_CHECK_CLOSE( teststat, 3.038383, 0.00001 );
                      BOOST_CHECK_CLOSE( p_value, 0.363, 0.0001 ); break;
            case 1 :  BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "Leaf3" );
                      BOOST_CHECK_EQUAL( nodecases, static_cast<unsigned int>(4) );
                      BOOST_CHECK_EQUAL( wndstart, static_cast<unsigned int>(12) );
                      BOOST_CHECK_EQUAL( wndend, static_cast<unsigned int>(17) );
                      BOOST_CHECK_EQUAL( wndcases, static_cast<unsigned int>(2) );
                      BOOST_CHECK_CLOSE( expected, 0.45, 0.001 );
                      BOOST_CHECK_CLOSE( rr, 9.33, 0.001 );
                      BOOST_CHECK_CLOSE( excess, 1.79, 0.001 );
                      BOOST_CHECK_CLOSE( ar, 0.0087, 0.00001 );
                      BOOST_CHECK_CLOSE( teststat, 1.433396, 0.00001 );
                      BOOST_CHECK_CLOSE( p_value, 0.997, 0.0001 ); break;
            case 2  : BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "Node2" );
                      BOOST_CHECK_EQUAL( nodecases, static_cast<unsigned int>(30) );
                      BOOST_CHECK_EQUAL( wndstart, static_cast<unsigned int>(1) );
                      BOOST_CHECK_EQUAL( wndend, static_cast<unsigned int>(3) );
                      BOOST_CHECK_EQUAL( wndcases, static_cast<unsigned int>(9) );
                      BOOST_CHECK_CLOSE( expected, 4.95, 0.001 );
                      BOOST_CHECK_CLOSE( ar, 0.024, 0.00001 );
                      BOOST_CHECK_CLOSE( rr, 3.67, 0.001 );
                      BOOST_CHECK_CLOSE( excess, 6.55, 0.001 );
                      BOOST_CHECK_CLOSE( teststat, 1.422299, 0.00001 );
                      BOOST_CHECK_CLOSE( p_value, 0.998, 0.0001 ); break;
            default : break;
        }
        getCSVRow(stream, data);
    }
    if (dataRows != 10) BOOST_FAIL( "expecting 10 data rows, got " << dataRows );
    stream.close();
}

/* Tests the expected values of the data file writer with the tree-time scan conditioned on node, with day of week adjustment. */
BOOST_AUTO_TEST_CASE( test_tree_time_condition_node_and_time_day_of_week_adjustment ) {
    // set parameters to report csv data file
    _parameters.setConditionalType(Parameters::NODEANDTIME);
    _parameters.setPerformDayOfWeekAdjustment(true);
    _parameters.setGeneratingTableResults(true);
    _parameters.setPrintColumnHeaders(true);
    _parameters.setReportAttributableRisk(true);
    _parameters.setAttributableRiskExposed(200);

    BOOST_REQUIRE_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

    // run analysis
    std::string results_user_directory;
    run_analysis("test", results_user_directory, _parameters, _print);

    // open the tabular results file and confirm expected columns and values match expected for this data set
    std::string buffer;
    std::ifstream stream;
    stream.open(CutsRecordWriter::getFilename(_parameters, buffer).c_str());
    if (!stream) throw std::exception("could not open file");

    CSV_Row_t headers;
    getCSVRow(stream, headers);
    if (headers.size() != static_cast<size_t>(13)) BOOST_FAIL( "expecting 13 columns, got " << headers.size() );
    std::vector<std::string>::iterator itrCutNum = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::CUT_NUM_FIELD);
    std::vector<std::string>::iterator itrNodeId = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::NODE_ID_FIELD);
    std::vector<std::string>::iterator itrNodeCases = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::NODE_CASES_FIELD);
    std::vector<std::string>::iterator itrWndStart = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::START_WINDOW_FIELD);
    std::vector<std::string>::iterator itrWndEnd = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::END_WINDOW_FIELD);
    std::vector<std::string>::iterator itrWndCase = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::WNDW_CASES_FIELD);
    std::vector<std::string>::iterator itrExpected = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::EXPECTED_CASES_FIELD);
    std::vector<std::string>::iterator itrRR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::RELATIVE_RISK_FIELD);
    std::vector<std::string>::iterator itrExcess = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::EXCESS_CASES_FIELD);
    std::vector<std::string>::iterator itrAR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::ATTRIBUTABLE_RISK_FIELD);
    std::vector<std::string>::iterator itrTestStat = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::TEST_STATISTIC_FIELD);
    std::vector<std::string>::iterator itrPValue = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::P_VALUE_FLD);

    // check the expected values
    unsigned int dataRows=0;
    CSV_Row_t data;
    getCSVRow(stream, data);
    while (data.size()) {
        ++dataRows;
        unsigned int cut_num; BOOST_CHECK( string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrCutNum)).c_str(), cut_num) );
        unsigned int nodecases; BOOST_CHECK( string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrNodeCases)).c_str(), nodecases) );
        unsigned int wndstart; BOOST_CHECK( string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrWndStart)).c_str(), wndstart) );
        unsigned int wndend; BOOST_CHECK( string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrWndEnd)).c_str(), wndend) );
        unsigned int wndcases; BOOST_CHECK( string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrWndCase)).c_str(), wndcases) );
        double expected; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrExpected)).c_str(), expected) );
        double ar; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrAR)).c_str(), ar) );
        double rr; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrRR)).c_str(), rr, false) );
        double excess; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrExcess)).c_str(), excess) );
        double teststat; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrTestStat)).c_str(), teststat) );
        double p_value; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrPValue)).c_str(), p_value) );

        BOOST_CHECK_EQUAL( cut_num, dataRows );
        switch (dataRows) {
            case 2 :  BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "Leaf5" );
                      BOOST_CHECK_EQUAL( nodecases, static_cast<unsigned int>(7) );
                      BOOST_CHECK_EQUAL( wndstart, static_cast<unsigned int>(8) );
                      BOOST_CHECK_EQUAL( wndend, static_cast<unsigned int>(11) );
                      BOOST_CHECK_EQUAL( wndcases, static_cast<unsigned int>(2) );
                      BOOST_CHECK_CLOSE( expected, 0.27, 0.001 );
                      BOOST_CHECK_CLOSE( rr, 11.6, 0.001 );
                      BOOST_CHECK_CLOSE( excess, 1.83, 0.001 );
                      BOOST_CHECK_CLOSE( ar, 0.009, 0.00001 );
                      BOOST_CHECK_CLOSE( teststat, 2.312096, 0.00001 );
                      BOOST_CHECK_CLOSE( p_value, 0.384, 0.0001 ); break;
            case 6 :  BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "Leaf11" );
                      BOOST_CHECK_EQUAL( nodecases, static_cast<unsigned int>(7) );
                      BOOST_CHECK_EQUAL( wndstart, static_cast<unsigned int>(14) );
                      BOOST_CHECK_EQUAL( wndend, static_cast<unsigned int>(17) );
                      BOOST_CHECK_EQUAL( wndcases, static_cast<unsigned int>(2) );
                      BOOST_CHECK_CLOSE( expected, 0.3, 0.001 );
                      BOOST_CHECK_CLOSE( rr, 5.6, 0.001 );
                      BOOST_CHECK_CLOSE( excess, 1.64, 0.001 );
                      BOOST_CHECK_CLOSE( ar, 0.0089, 0.00001 );
                      BOOST_CHECK_CLOSE( teststat, 2.12392, 0.00001 );
                      BOOST_CHECK_CLOSE( p_value, 0.504, 0.0001 ); break;
            case 1  : BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "Leaf3" );
                      BOOST_CHECK_EQUAL( nodecases, static_cast<unsigned int>(4) );
                      BOOST_CHECK_EQUAL( wndstart, static_cast<unsigned int>(11) );
                      BOOST_CHECK_EQUAL( wndend, static_cast<unsigned int>(12) );
                      BOOST_CHECK_EQUAL( wndcases, static_cast<unsigned int>(1) );
                      BOOST_CHECK_CLOSE( expected, 0.15, 0.001 );
                      BOOST_CHECK_CLOSE( rr, 10.0, 0.001 );
                      BOOST_CHECK_CLOSE( excess, 0.9, 0.001 );
                      BOOST_CHECK_CLOSE( ar, 0.0044, 0.00001 );
                      BOOST_CHECK_CLOSE( teststat, 1.029356, 0.00001 );
                      BOOST_CHECK_CLOSE( p_value, 0.981, 0.0001 ); break;
            case 4  : BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "Leaf7" );
                      BOOST_CHECK_EQUAL( nodecases, static_cast<unsigned int>(11) );
                      BOOST_CHECK_EQUAL( wndstart, static_cast<unsigned int>(9) );
                      BOOST_CHECK_EQUAL( wndend, static_cast<unsigned int>(13) );
                      BOOST_CHECK_EQUAL( wndcases, static_cast<unsigned int>(2) );
                      BOOST_CHECK_CLOSE( expected, 0.67, 0.001 );
                      BOOST_CHECK_CLOSE( rr, 4.56, 0.001 );
                      BOOST_CHECK_CLOSE( excess, 1.56, 0.001 );
                      BOOST_CHECK_CLOSE( ar, 0.0071, 0.00001 );
                      BOOST_CHECK_CLOSE( teststat, 0.870215, 0.00001 );
                      BOOST_CHECK_CLOSE( p_value, 0.999, 0.0001 ); break;
            default : break;
        }
        getCSVRow(stream, data);
    }
    if (dataRows != 9) BOOST_FAIL( "expecting 9 data rows, got " << dataRows );
    stream.close();
}

BOOST_AUTO_TEST_SUITE_END()

/* Test Suite for the time only scan example data set. */
BOOST_FIXTURE_TEST_SUITE( time_only_example_suite, time_only_fixture )

/* Tests the expected values of the data file writer with the time only scan conditioned on total cases. */
BOOST_AUTO_TEST_CASE( test_time_only_condition_totalcases ) {
    // set parameters to report csv data file
    _parameters.setScanType(Parameters::TIMEONLY);
    _parameters.setConditionalType(Parameters::TOTALCASES);
    _parameters.setGeneratingTableResults(true);
    _parameters.setPrintColumnHeaders(true);
    _parameters.setPerformDayOfWeekAdjustment(false);
    _parameters.setReportAttributableRisk(true);
    _parameters.setAttributableRiskExposed(200);

    BOOST_REQUIRE_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

    // run analysis
    std::string results_user_directory;
    run_analysis("test", results_user_directory, _parameters, _print);

    // open the tabular results file and confirm expected columns and values match expected for this data set
    std::string buffer;
    std::ifstream stream;
    stream.open(CutsRecordWriter::getFilename(_parameters, buffer).c_str());
    if (!stream) throw std::exception("could not open file");

    CSV_Row_t headers;
    getCSVRow(stream, headers);
    if (headers.size() != static_cast<size_t>(12)) BOOST_FAIL( "expecting 12 columns, got " << headers.size() );
    std::vector<std::string>::iterator itrCutNum = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::CUT_NUM_FIELD);
    std::vector<std::string>::iterator itrNodeId = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::NODE_ID_FIELD);
    std::vector<std::string>::iterator itrNodeCases = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::NODE_CASES_FIELD);
    std::vector<std::string>::iterator itrWndStart = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::START_WINDOW_FIELD);
    std::vector<std::string>::iterator itrWndEnd = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::END_WINDOW_FIELD);
    std::vector<std::string>::iterator itrWndCase = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::WNDW_CASES_FIELD);
    std::vector<std::string>::iterator itrExpected = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::EXPECTED_CASES_FIELD);
    std::vector<std::string>::iterator itrRR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::RELATIVE_RISK_FIELD);
    std::vector<std::string>::iterator itrExcess = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::EXCESS_CASES_FIELD);
    std::vector<std::string>::iterator itrAR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::ATTRIBUTABLE_RISK_FIELD);
    std::vector<std::string>::iterator itrLLR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::LOG_LIKL_RATIO_FIELD);
    std::vector<std::string>::iterator itrPValue = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::P_VALUE_FLD);

    // check the expected values
    unsigned int dataRows=0;
    CSV_Row_t data;
    getCSVRow(stream, data);
    while (data.size()) {
        ++dataRows;
        unsigned int cut_num; BOOST_CHECK( string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrCutNum)).c_str(), cut_num) );
        unsigned int nodecases; BOOST_CHECK( string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrNodeCases)).c_str(), nodecases) );
        unsigned int wndstart; BOOST_CHECK( string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrWndStart)).c_str(), wndstart) );
        unsigned int wndend; BOOST_CHECK( string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrWndEnd)).c_str(), wndend) );
        unsigned int wndcases; BOOST_CHECK( string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrWndCase)).c_str(), wndcases) );
        double expected; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrExpected)).c_str(), expected) );
        double rr; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrRR)).c_str(), rr, false) );
        double excess; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrExcess)).c_str(), excess) );
        double ar; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrAR)).c_str(), ar) );
        double llr; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrLLR)).c_str(), llr) );
        double p_value; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrPValue)).c_str(), p_value) );

        BOOST_CHECK_EQUAL( cut_num, dataRows );
        switch (dataRows) {
            case 1 :  BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "All" );
                      BOOST_CHECK_EQUAL( nodecases, static_cast<unsigned int>(97) );
                      BOOST_CHECK_EQUAL( wndstart, static_cast<unsigned int>(2) );
                      BOOST_CHECK_EQUAL( wndend, static_cast<unsigned int>(7) );
                      BOOST_CHECK_EQUAL( wndcases, static_cast<unsigned int>(52) );
                      BOOST_CHECK_CLOSE( expected, 20.79, 0.001 );
                      BOOST_CHECK_CLOSE( rr, 4.24, 0.001 );
                      BOOST_CHECK_CLOSE( excess, 39.73, 0.001 );
                      BOOST_CHECK_CLOSE( ar, 0.2, 0.00001 );
                      BOOST_CHECK_CLOSE( llr, 23.972955, 0.00001 );
                      BOOST_CHECK_CLOSE( p_value, 0.001, 0.0001 ); break;
            case 16:  BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "All" );
                      BOOST_CHECK_EQUAL( nodecases, static_cast<unsigned int>(97) );
                      BOOST_CHECK_EQUAL( wndstart, static_cast<unsigned int>(1) );
                      BOOST_CHECK_EQUAL( wndend, static_cast<unsigned int>(2) );
                      BOOST_CHECK_EQUAL( wndcases, static_cast<unsigned int>(9) );
                      BOOST_CHECK_CLOSE( expected, 6.93, 0.001 );
                      BOOST_CHECK_CLOSE( rr, 1.33, 0.001 );
                      BOOST_CHECK_CLOSE( excess, 2.23, 0.001 );
                      BOOST_CHECK_CLOSE( ar, 0.011, 0.00001 );
                      BOOST_CHECK_CLOSE( llr, 0.306713, 0.00001 );
                      BOOST_CHECK_CLOSE( p_value, 0.994, 0.0001 ); break;
            default : break;
        }
        getCSVRow(stream, data);
    }
    if (dataRows != 16) BOOST_FAIL( "expecting 16 data rows, got " << dataRows );
    stream.close();
}

/* Tests the expected values of the data file writer with the time only scan conditioned on total cases, w/ day of week adjustment. */
BOOST_AUTO_TEST_CASE( test_time_only_condition_totalcases_day_of_week_adjustment ) {
    // set parameters to report csv data file
    _parameters.setScanType(Parameters::TIMEONLY);
    _parameters.setConditionalType(Parameters::TOTALCASES);
    _parameters.setGeneratingTableResults(true);
    _parameters.setPrintColumnHeaders(true);
    _parameters.setPerformDayOfWeekAdjustment(true);
    _parameters.setReportAttributableRisk(true);
    _parameters.setAttributableRiskExposed(200);

    BOOST_REQUIRE_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

    // run analysis
    std::string results_user_directory;
    run_analysis("test", results_user_directory, _parameters, _print);

    // open the tabular results file and confirm expected columns and values match expected for this data set
    std::string buffer;
    std::ifstream stream;
    stream.open(CutsRecordWriter::getFilename(_parameters, buffer).c_str());
    if (!stream) throw std::exception("could not open file");

    CSV_Row_t headers;
    getCSVRow(stream, headers);
    if (headers.size() != static_cast<size_t>(12)) BOOST_FAIL( "expecting 12 columns, got " << headers.size() );
    std::vector<std::string>::iterator itrCutNum = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::CUT_NUM_FIELD);
    std::vector<std::string>::iterator itrNodeId = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::NODE_ID_FIELD);
    std::vector<std::string>::iterator itrNodeCases = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::NODE_CASES_FIELD);
    std::vector<std::string>::iterator itrWndStart = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::START_WINDOW_FIELD);
    std::vector<std::string>::iterator itrWndEnd = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::END_WINDOW_FIELD);
    std::vector<std::string>::iterator itrWndCase = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::WNDW_CASES_FIELD);
    std::vector<std::string>::iterator itrExpected = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::EXPECTED_CASES_FIELD);
    std::vector<std::string>::iterator itrRR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::RELATIVE_RISK_FIELD);
    std::vector<std::string>::iterator itrExcess = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::EXCESS_CASES_FIELD);
    std::vector<std::string>::iterator itrAR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::ATTRIBUTABLE_RISK_FIELD);
    std::vector<std::string>::iterator itrteststat = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::TEST_STATISTIC_FIELD);
    std::vector<std::string>::iterator itrPValue = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::P_VALUE_FLD);

    // check the expected values
    unsigned int dataRows=0;
    CSV_Row_t data;
    getCSVRow(stream, data);
    while (data.size()) {
        ++dataRows;
        unsigned int cut_num; BOOST_CHECK( string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrCutNum)).c_str(), cut_num) );
        unsigned int nodecases; BOOST_CHECK( string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrNodeCases)).c_str(), nodecases) );
        unsigned int wndstart; BOOST_CHECK( string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrWndStart)).c_str(), wndstart) );
        unsigned int wndend; BOOST_CHECK( string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrWndEnd)).c_str(), wndend) );
        unsigned int wndcases; BOOST_CHECK( string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrWndCase)).c_str(), wndcases) );
        double expected; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrExpected)).c_str(), expected) );
        double rr; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrRR)).c_str(), rr, false) );
        double excess; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrExcess)).c_str(), excess) );
        double ar; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrAR)).c_str(), ar) );
        double teststat; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrteststat)).c_str(), teststat) );
        double p_value; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrPValue)).c_str(), p_value) );

        BOOST_CHECK_EQUAL( cut_num, dataRows );
        switch (dataRows) {
            case 1 :  BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "All" );
                      BOOST_CHECK_EQUAL( nodecases, static_cast<unsigned int>(97) );
                      BOOST_CHECK_EQUAL( wndstart, static_cast<unsigned int>(1) );
                      BOOST_CHECK_EQUAL( wndend, static_cast<unsigned int>(7) );
                      BOOST_CHECK_EQUAL( wndcases, static_cast<unsigned int>(56) );
                      BOOST_CHECK_CLOSE( expected, 23.65, 0.001 );
                      BOOST_CHECK_CLOSE( rr, 4.24, 0.001 );
                      BOOST_CHECK_CLOSE( excess, 42.78, 0.001 );
                      BOOST_CHECK_CLOSE( ar, 0.21, 0.001 );
                      BOOST_CHECK_CLOSE( teststat, 24.422872, 0.00001 );
                      BOOST_CHECK_CLOSE( p_value, 0.001, 0.0001 ); break;
            case 16:  BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "All" );
                      BOOST_CHECK_EQUAL( nodecases, static_cast<unsigned int>(97) );
                      BOOST_CHECK_EQUAL( wndstart, static_cast<unsigned int>(1) );
                      BOOST_CHECK_EQUAL( wndend, static_cast<unsigned int>(2) );
                      BOOST_CHECK_EQUAL( wndcases, static_cast<unsigned int>(9) );
                      BOOST_CHECK_CLOSE( expected, 4.9, 0.001 );
                      BOOST_CHECK_CLOSE( rr, 1.92, 0.001 );
                      BOOST_CHECK_CLOSE( excess, 4.32, 0.001 );
                      BOOST_CHECK_CLOSE( ar, 0.022, 0.001 );
                      BOOST_CHECK_CLOSE( teststat, 1.464549, 0.00001 );
                      BOOST_CHECK_CLOSE( p_value, 0.495, 0.0001 ); break;
            default : break;
        }
        getCSVRow(stream, data);
    }
    if (dataRows != 16) BOOST_FAIL( "expecting 16 data rows, got " << dataRows );
    stream.close();
}

BOOST_AUTO_TEST_SUITE_END()
