// project files
#include "fixture_examples.h"
#include "ScanRunner.h"
#include "Toolkit.h"
#include "DataFileWriter.h"
#include <boost/any.hpp>

/* Tests the expected results of the example data sets */

std::vector<std::string>::iterator getHeaderColumnIteratorOrFail(CSV_Row_t& headerRow, const std::string& columnName) {
    std::vector<std::string>::iterator itr = std::find(headerRow.begin(), headerRow.end(), columnName);
    if (itr == headerRow.end()) BOOST_FAIL( "Column '" << columnName.c_str() << "' not found" );
    return itr;
}

/* Test Suite for the Poisson example data set. */
BOOST_FIXTURE_TEST_SUITE( poisson_example_suite, poisson_fixture )

/* Tests the expected values of the data file writer with the unconditional Poisson model. */
BOOST_AUTO_TEST_CASE( test_poisson_unconditional ) {
    // set parameters to report csv data file
    _parameters.setConditionalType(Parameters::UNCONDITIONAL);
    _parameters.setGeneratingTableResults(true);
    _parameters.setPrintColumnHeaders(true);

    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

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
    if (headers.size() != static_cast<size_t>(9)) BOOST_FAIL( "expecting 9 columns, got " << headers.size() );
    std::vector<std::string>::iterator itrCutNum = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::CUT_NUM_FIELD);
    std::vector<std::string>::iterator itrNodeId = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::NODE_ID_FIELD);
    std::vector<std::string>::iterator itrObserved = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::OBSERVED_CASES_FIELD);
    std::vector<std::string>::iterator itrExpected = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::EXPECTED_FIELD);
    std::vector<std::string>::iterator itrODE = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::OBSERVED_DIV_EXPECTED_FIELD);
    std::vector<std::string>::iterator itrRR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::RELATIVE_RISK_FIELD);
    std::vector<std::string>::iterator itrExcess = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::EXCESS_CASES_FIELD);
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
        double ode; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrODE)).c_str(), ode) );
        double rr; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrRR)).c_str(), rr) );
        double excess; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrExcess)).c_str(), excess) );
        double llr; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrLLR)).c_str(), llr) );
        double p_value; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrPValue)).c_str(), p_value) );

        BOOST_CHECK_EQUAL( cut_num, dataRows );
        switch (dataRows) {
            case 1 : BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "node 237" );
                     BOOST_CHECK_EQUAL( observed, static_cast<unsigned int>(43) );
                     BOOST_CHECK_CLOSE( expected, 0.66, 0.001 );
                     BOOST_CHECK_CLOSE( ode, 65.29, 0.001 );
                     BOOST_CHECK_CLOSE( rr, 65.29, 0.001 );
                     BOOST_CHECK_CLOSE( excess, 42.34, 0.001 );
                     BOOST_CHECK_CLOSE( llr, 137.351531, 0.00001 );
                     BOOST_CHECK_CLOSE( p_value, 0.001, 0.0001 ); break;
            case 2 : BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "node 291" );
                     BOOST_CHECK_EQUAL( observed, static_cast<unsigned int>(39) );
                     BOOST_CHECK_CLOSE( expected,0.56, 0.001 );
                     BOOST_CHECK_CLOSE( ode, 70.25, 0.001 );
                     BOOST_CHECK_CLOSE( rr, 70.25, 0.001 );
                     BOOST_CHECK_CLOSE( excess, 38.44, 0.001 );
                     BOOST_CHECK_CLOSE( llr, 127.383648, 0.00001 );
                     BOOST_CHECK_CLOSE( p_value, 0.001, 0.0001 ); break;
            case 70 :BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "node 1056" );
                     BOOST_CHECK_EQUAL( observed, static_cast<unsigned int>(4) );
                     BOOST_CHECK_CLOSE( expected, 2.29, 0.001 );
                     BOOST_CHECK_CLOSE( ode, 1.74, 0.001 );
                     BOOST_CHECK_CLOSE( rr, 1.74, 0.001 );
                     BOOST_CHECK_CLOSE( excess, 1.71, 0.001 );
                     BOOST_CHECK_CLOSE( llr, 0.518698, 0.00001 );
                     BOOST_CHECK_CLOSE( p_value, 0.999, 0.0001 ); break;
            case 71 :BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "node 1055" );
                     BOOST_CHECK_EQUAL( observed, static_cast<unsigned int>(4) );
                     BOOST_CHECK_CLOSE( expected, 2.29, 0.001 );
                     BOOST_CHECK_CLOSE( ode, 1.74, 0.001 );
                     BOOST_CHECK_CLOSE( rr, 1.74, 0.001 );
                     BOOST_CHECK_CLOSE( excess, 1.71, 0.001 );
                     BOOST_CHECK_CLOSE( llr, 0.518698, 0.00001 );
                     BOOST_CHECK_CLOSE( p_value, 0.999, 0.0001 ); break;
            default : break;
        }
        getCSVRow(stream, data);
    }
    if (dataRows != 71) BOOST_FAIL( "expecting 71 data rows, got" << dataRows );
    stream.close();
}

/* Tests the expected values of the data file writer with the conditional Poisson model. */
BOOST_AUTO_TEST_CASE( test_poisson_conditional ) {
    // set parameters to report csv data file
    _parameters.setConditionalType(Parameters::TOTALCASES);
    _parameters.setGeneratingTableResults(true);
    _parameters.setPrintColumnHeaders(true);

    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

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
    if (headers.size() != static_cast<size_t>(9)) BOOST_FAIL( "expecting 9 columns, got " << headers.size() );
    std::vector<std::string>::iterator itrCutNum = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::CUT_NUM_FIELD);
    std::vector<std::string>::iterator itrNodeId = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::NODE_ID_FIELD);
    std::vector<std::string>::iterator itrObserved = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::OBSERVED_CASES_FIELD);
    std::vector<std::string>::iterator itrExpected = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::EXPECTED_FIELD);
    std::vector<std::string>::iterator itrODE = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::OBSERVED_DIV_EXPECTED_FIELD);
    std::vector<std::string>::iterator itrRR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::RELATIVE_RISK_FIELD);
    std::vector<std::string>::iterator itrExcess = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::EXCESS_CASES_FIELD);
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
        double ode; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrODE)).c_str(), ode) );
        double rr; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrRR)).c_str(), rr) );
        double excess; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrExcess)).c_str(), excess) );
        double llr; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrLLR)).c_str(), llr) );
        double p_value; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrPValue)).c_str(), p_value) );

        BOOST_CHECK_EQUAL( cut_num, dataRows );
        switch (dataRows) {
            case 1 : BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "node 237" );
                     BOOST_CHECK_EQUAL( observed, static_cast<unsigned int>(43) );
                     BOOST_CHECK_CLOSE( expected, 1.11, 0.001 );
                     BOOST_CHECK_CLOSE( ode, 38.69, 0.001 );
                     BOOST_CHECK_CLOSE( rr, 44.07, 0.001 );
                     BOOST_CHECK_CLOSE( excess, 42.02, 0.001 );
                     BOOST_CHECK_CLOSE( llr, 117.965788, 0.00001 );
                     BOOST_CHECK_CLOSE( p_value, 0.001, 0.0001 ); break;
            case 2 : BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "node 291" );
                     BOOST_CHECK_EQUAL( observed, static_cast<unsigned int>(39) );
                     BOOST_CHECK_CLOSE( expected, 0.94, 0.001 );
                     BOOST_CHECK_CLOSE( ode, 41.62, 0.001 );
                     BOOST_CHECK_CLOSE( rr, 46.81, 0.001 );
                     BOOST_CHECK_CLOSE( excess, 38.17, 0.001 );
                     BOOST_CHECK_CLOSE( llr, 109.545209, 0.00001 );
                     BOOST_CHECK_CLOSE( p_value, 0.001, 0.0001 ); break;
            case 26 :BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "node 428" );
                     BOOST_CHECK_EQUAL( observed, static_cast<unsigned int>(3) );
                     BOOST_CHECK_CLOSE( expected, 0.78, 0.001 );
                     BOOST_CHECK_CLOSE( ode, 3.84, 0.001 );
                     BOOST_CHECK_CLOSE( rr, 3.87, 0.001 );
                     BOOST_CHECK_CLOSE( excess, 2.22, 0.001 );
                     BOOST_CHECK_CLOSE( llr, 1.827640, 0.00001 );
                     BOOST_CHECK_CLOSE( p_value, 0.948, 0.0001 ); break;
            case 27 :BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "node 647" );
                     BOOST_CHECK_EQUAL( observed, static_cast<unsigned int>(28) );
                     BOOST_CHECK_CLOSE( expected, 19.99, 0.001 );
                     BOOST_CHECK_CLOSE( ode, 1.40, 0.001 );
                     BOOST_CHECK_CLOSE( rr, 1.44, 0.001 );
                     BOOST_CHECK_CLOSE( excess, 8.51, 0.001 );
                     BOOST_CHECK_CLOSE( llr, 1.526511, 0.00001 );
                     BOOST_CHECK_CLOSE( p_value, 0.981, 0.0001 ); break;
            default : break;
        }
        getCSVRow(stream, data);
    }
    if (dataRows != 27) BOOST_FAIL( "expecting 27 data rows, got" << dataRows );
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

    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

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
    std::vector<std::string>::iterator itrObserved = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::OBSERVATIONS_FIELD);
    std::vector<std::string>::iterator itrCases = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::CASES_FIELD);
    std::vector<std::string>::iterator itrExpected = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::EXPECTED_FIELD);
    std::vector<std::string>::iterator itrODE = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::OBSERVED_DIV_EXPECTED_FIELD);
    std::vector<std::string>::iterator itrRR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::RELATIVE_RISK_FIELD);
    std::vector<std::string>::iterator itrExcess = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::EXCESS_CASES_FIELD);
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
        double ode; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrODE)).c_str(), ode) );
        double rr; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrRR)).c_str(), rr) );
        double excess; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrExcess)).c_str(), excess) );
        double llr; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrLLR)).c_str(), llr) );
        double p_value; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrPValue)).c_str(), p_value) );

        BOOST_CHECK_EQUAL( cut_num, dataRows );
        switch (dataRows) {
            case 1 : BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "node 28" );
                     BOOST_CHECK_EQUAL( observed, static_cast<unsigned int>(102) );
                     BOOST_CHECK_EQUAL( cases, static_cast<unsigned int>(100) );
                     BOOST_CHECK_CLOSE( expected, 51.00, 0.001 );
                     BOOST_CHECK_CLOSE( ode, 1.96, 0.001 );
                     BOOST_CHECK_CLOSE( rr, 1.96, 0.001 );
                     BOOST_CHECK_CLOSE( excess, 49.00, 0.001 );
                     BOOST_CHECK_CLOSE( llr, 60.857098, 0.00001 );
                     BOOST_CHECK_CLOSE( p_value, 0.001, 0.0001 ); break;
            case 2 : BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "node 27" );
                     BOOST_CHECK_EQUAL( observed, static_cast<unsigned int>(112) );
                     BOOST_CHECK_EQUAL( cases, static_cast<unsigned int>(100) );
                     BOOST_CHECK_CLOSE( expected, 56.00, 0.001 );
                     BOOST_CHECK_CLOSE( ode, 1.79, 0.001 );
                     BOOST_CHECK_CLOSE( rr, 1.79, 0.001 );
                     BOOST_CHECK_CLOSE( excess, 44.00, 0.001 );
                     BOOST_CHECK_CLOSE( llr, 39.496509, 0.00001 );
                     BOOST_CHECK_CLOSE( p_value, 0.001, 0.0001 ); break;
            case 18 :BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "node 1072" );
                     BOOST_CHECK_EQUAL( observed, static_cast<unsigned int>(9) );
                     BOOST_CHECK_EQUAL( cases, static_cast<unsigned int>(8) );
                     BOOST_CHECK_CLOSE( expected, 4.50, 0.001 );
                     BOOST_CHECK_CLOSE( ode, 1.78, 0.001 );
                     BOOST_CHECK_CLOSE( rr, 1.78, 0.001 );
                     BOOST_CHECK_CLOSE( excess, 3.50, 0.001 );
                     BOOST_CHECK_CLOSE( llr, 3.098836, 0.00001 );
                     BOOST_CHECK_CLOSE( p_value, 0.999, 0.0001 ); break;
            case 19 :BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "node 1066" );
                     BOOST_CHECK_EQUAL( observed, static_cast<unsigned int>(9) );
                     BOOST_CHECK_EQUAL( cases, static_cast<unsigned int>(8) );
                     BOOST_CHECK_CLOSE( expected, 4.50, 0.001 );
                     BOOST_CHECK_CLOSE( ode, 1.78, 0.001 );
                     BOOST_CHECK_CLOSE( rr, 1.78, 0.001 );
                     BOOST_CHECK_CLOSE( excess, 3.50, 0.001 );
                     BOOST_CHECK_CLOSE( llr, 3.098836, 0.00001 );
                     BOOST_CHECK_CLOSE( p_value, 0.999, 0.0001 ); break;
            default : break;
        }
        getCSVRow(stream, data);
    }
    if (dataRows != 19) BOOST_FAIL( "expecting 19 data rows, got" << dataRows );
    stream.close();
}

/* Tests the expected values of the data file writer with the conditional Bernoulli model. */
BOOST_AUTO_TEST_CASE( test_bernoulli_conditional ) {
    // set parameters to report csv data file
    _parameters.setConditionalType(Parameters::TOTALCASES);
    _parameters.setGeneratingTableResults(true);
    _parameters.setPrintColumnHeaders(true);

    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

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
    std::vector<std::string>::iterator itrObserved = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::OBSERVATIONS_FIELD);
    std::vector<std::string>::iterator itrCases = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::CASES_FIELD);
    std::vector<std::string>::iterator itrExpected = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::EXPECTED_FIELD);
    std::vector<std::string>::iterator itrODE = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::OBSERVED_DIV_EXPECTED_FIELD);
    std::vector<std::string>::iterator itrRR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::RELATIVE_RISK_FIELD);
    std::vector<std::string>::iterator itrExcess = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::EXCESS_CASES_FIELD);
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
        double ode; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrODE)).c_str(), ode) );
        double rr; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrRR)).c_str(), rr) );
        double excess; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrExcess)).c_str(), excess) );
        double llr; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrLLR)).c_str(), llr) );
        double p_value; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrPValue)).c_str(), p_value) );

        BOOST_CHECK_EQUAL( cut_num, dataRows );
        switch (dataRows) {
            case 1 : BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "node 28" );
                     BOOST_CHECK_EQUAL( observed, static_cast<unsigned int>(102) );
                     BOOST_CHECK_EQUAL( cases, static_cast<unsigned int>(100) );
                     BOOST_CHECK_CLOSE( expected, 9.43, 0.001 );
                     BOOST_CHECK_CLOSE( ode, 10.60, 0.001 );
                     BOOST_CHECK_CLOSE( rr, 12.90, 0.001 );
                     BOOST_CHECK_CLOSE( excess, 92.25, 0.001 );
                     BOOST_CHECK_CLOSE( llr, 237.830221, 0.00001 );
                     BOOST_CHECK_CLOSE( p_value, 0.001, 0.0001 ); break;
            case 2 : BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "node 22" );
                     BOOST_CHECK_EQUAL( observed, static_cast<unsigned int>(137) );
                     BOOST_CHECK_EQUAL( cases, static_cast<unsigned int>(115) );
                     BOOST_CHECK_CLOSE( expected, 12.67, 0.001 );
                     BOOST_CHECK_CLOSE( ode, 9.08, 0.001 );
                     BOOST_CHECK_CLOSE( rr, 11.38, 0.001 );
                     BOOST_CHECK_CLOSE( excess, 104.90, 0.001 );
                     BOOST_CHECK_CLOSE( llr, 227.740852, 0.00001 );
                     BOOST_CHECK_CLOSE( p_value, 0.001, 0.0001 ); break;
            case 71 :BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "node 357" );
                     BOOST_CHECK_EQUAL( observed, static_cast<unsigned int>(6) );
                     BOOST_CHECK_EQUAL( cases, static_cast<unsigned int>(3) );
                     BOOST_CHECK_CLOSE( expected, 0.55, 0.001 );
                     BOOST_CHECK_CLOSE( ode, 5.41, 0.001 );
                     BOOST_CHECK_CLOSE( rr, 5.43, 0.001 );
                     BOOST_CHECK_CLOSE( excess, 2.45, 0.001 );
                     BOOST_CHECK_CLOSE( llr, 3.280798, 0.00001 );
                     BOOST_CHECK_CLOSE( p_value, 0.992, 0.0001 ); break;
            case 72 :BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "node 1070" );
                     BOOST_CHECK_EQUAL( observed, static_cast<unsigned int>(3) );
                     BOOST_CHECK_EQUAL( cases, static_cast<unsigned int>(2) );
                     BOOST_CHECK_CLOSE( expected, 0.28, 0.001 );
                     BOOST_CHECK_CLOSE( ode, 7.21, 0.001 );
                     BOOST_CHECK_CLOSE( rr, 7.23, 0.001 );
                     BOOST_CHECK_CLOSE( excess, 1.72, 0.001 );
                     BOOST_CHECK_CLOSE( llr, 2.952112, 0.00001 );
                     BOOST_CHECK_CLOSE( p_value, 0.999, 0.0001 ); break;
            default : break;
        }
        getCSVRow(stream, data);
    }
    if (dataRows != 72) BOOST_FAIL( "expecting 72 data rows, got" << dataRows );
    stream.close();
}

BOOST_AUTO_TEST_SUITE_END()

/* Test Suite for the tree-time scan example data set. */
BOOST_FIXTURE_TEST_SUITE( tree_time_example_suite, tree_temporal_fixture )

/* Tests the expected values of the data file writer with the tree-time scan conditioned on node. */
BOOST_AUTO_TEST_CASE( test_tree_time_condition_node ) {
    // set parameters to report csv data file
    _parameters.setConditionalType(Parameters::NODE);
    _parameters.setGeneratingTableResults(true);
    _parameters.setPrintColumnHeaders(true);

    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

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
    std::vector<std::string>::iterator itrODE = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::OBSERVED_DIV_EXPECTED_FIELD);
    std::vector<std::string>::iterator itrRR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::RELATIVE_RISK_FIELD);
    std::vector<std::string>::iterator itrExcess = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::EXCESS_CASES_FIELD);
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
        double ode; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrODE)).c_str(), ode) );
        double rr; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrRR)).c_str(), rr, false) );
        double excess; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrExcess)).c_str(), excess) );
        double llr; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrLLR)).c_str(), llr) );
        double p_value; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrPValue)).c_str(), p_value) );

        BOOST_CHECK_EQUAL( cut_num, dataRows );
        switch (dataRows) {
            case 1 :  BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "node 22" );
                      BOOST_CHECK_EQUAL( nodecases, static_cast<unsigned int>(115) );
                      BOOST_CHECK_EQUAL( wndstart, static_cast<unsigned int>(1) );
                      BOOST_CHECK_EQUAL( wndend, static_cast<unsigned int>(2) );
                      BOOST_CHECK_EQUAL( wndcases, static_cast<unsigned int>(115) );
                      BOOST_CHECK_CLOSE( expected, 8.52, 0.001 );
                      BOOST_CHECK_CLOSE( ode, 13.50, 0.001 );
                      BOOST_CHECK( boost::math::isinf<double>(rr) );
                      BOOST_CHECK_CLOSE( excess, 115.00, 0.001 );
                      BOOST_CHECK_CLOSE( llr, 299.309314, 0.00001 );
                      BOOST_CHECK_CLOSE( p_value, 0.001, 0.0001 ); break;
            case 2 :  BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "node 0" );
                      BOOST_CHECK_EQUAL( nodecases, static_cast<unsigned int>(154) );
                      BOOST_CHECK_EQUAL( wndstart, static_cast<unsigned int>(1) );
                      BOOST_CHECK_EQUAL( wndend, static_cast<unsigned int>(2) );
                      BOOST_CHECK_EQUAL( wndcases, static_cast<unsigned int>(129) );
                      BOOST_CHECK_CLOSE( expected, 11.41, 0.001 );
                      BOOST_CHECK_CLOSE( ode, 11.31, 0.001 );
                      BOOST_CHECK_CLOSE( rr, 64.50, 0.001 );
                      BOOST_CHECK_CLOSE( excess, 127.00, 0.001 );
                      BOOST_CHECK_CLOSE( llr, 269.367990, 0.00001 );
                      BOOST_CHECK_CLOSE( p_value, 0.001, 0.0001 ); break;
            case 116 :BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "node 410" );
                      BOOST_CHECK_EQUAL( nodecases, static_cast<unsigned int>(2) );
                      BOOST_CHECK_EQUAL( wndstart, static_cast<unsigned int>(3) );
                      BOOST_CHECK_EQUAL( wndend, static_cast<unsigned int>(4) );
                      BOOST_CHECK_EQUAL( wndcases, static_cast<unsigned int>(2) );
                      BOOST_CHECK_CLOSE( expected, 0.15, 0.001 );
                      BOOST_CHECK_CLOSE( ode, 13.50, 0.001 );
                      BOOST_CHECK( boost::math::isinf<double>(rr) );
                      BOOST_CHECK_CLOSE( excess, 2.00, 0.001 );
                      BOOST_CHECK_CLOSE( llr, 5.205379, 0.00001 );
                      BOOST_CHECK_CLOSE( p_value, 0.637, 0.0001 ); break;
            case 117 :BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "node 1070" );
                      BOOST_CHECK_EQUAL( nodecases, static_cast<unsigned int>(2) );
                      BOOST_CHECK_EQUAL( wndstart, static_cast<unsigned int>(9) );
                      BOOST_CHECK_EQUAL( wndend, static_cast<unsigned int>(10) );
                      BOOST_CHECK_EQUAL( wndcases, static_cast<unsigned int>(2) );
                      BOOST_CHECK_CLOSE( expected, 0.15, 0.001 );
                      BOOST_CHECK_CLOSE( ode, 13.50, 0.001 );
                      BOOST_CHECK( boost::math::isinf<double>(rr) );
                      BOOST_CHECK_CLOSE( excess, 2.00, 0.001 );
                      BOOST_CHECK_CLOSE( llr, 5.205379, 0.00001 );
                      BOOST_CHECK_CLOSE( p_value, 0.637, 0.0001 ); break;
            default : break;
        }
        getCSVRow(stream, data);
    }
    if (dataRows != 117) BOOST_FAIL( "expecting 117 data rows, got" << dataRows );
    stream.close();
}

/* Tests the expected values of the data file writer with the tree-time scan conditioned on node, with day of week adjustment. */
BOOST_AUTO_TEST_CASE( test_tree_time_condition_node_day_of_week_adjustment ) {
    // set parameters to report csv data file
    _parameters.setConditionalType(Parameters::NODE);
    _parameters.setPerformDayOfWeekAdjustment(true);
    _parameters.setGeneratingTableResults(true);
    _parameters.setPrintColumnHeaders(true);

    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

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
    std::vector<std::string>::iterator itrODE = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::OBSERVED_DIV_EXPECTED_FIELD);
    std::vector<std::string>::iterator itrRR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::RELATIVE_RISK_FIELD);
    std::vector<std::string>::iterator itrExcess = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::EXCESS_CASES_FIELD);
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
        double ode; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrODE)).c_str(), ode) );
        double rr; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrRR)).c_str(), rr, false) );
        double excess; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrExcess)).c_str(), excess) );
        double teststat; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrTestStat)).c_str(), teststat) );
        double p_value; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrPValue)).c_str(), p_value) );

        BOOST_CHECK_EQUAL( cut_num, dataRows );
        switch (dataRows) {
            case 1 :  BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "node 0" );
                      BOOST_CHECK_EQUAL( nodecases, static_cast<unsigned int>(154) );
                      BOOST_CHECK_EQUAL( wndstart, static_cast<unsigned int>(2) );
                      BOOST_CHECK_EQUAL( wndend, static_cast<unsigned int>(3) );
                      BOOST_CHECK_EQUAL( wndcases, static_cast<unsigned int>(129) );
                      BOOST_CHECK_CLOSE( expected, 14.75, 0.001 );
                      BOOST_CHECK_CLOSE( ode, 8.75, 0.001 );
                      BOOST_CHECK_CLOSE( rr, 48.72, 0.001 );
                      BOOST_CHECK_CLOSE( excess, 126.35, 0.001 );
                      BOOST_CHECK_CLOSE( teststat, 178.870448, 0.00001 );
                      BOOST_CHECK_CLOSE( p_value, 0.001, 0.0001 ); break;
            case 2 :  BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "node 22" );
                      BOOST_CHECK_EQUAL( nodecases, static_cast<unsigned int>(115) );
                      BOOST_CHECK_EQUAL( wndstart, static_cast<unsigned int>(2) );
                      BOOST_CHECK_EQUAL( wndend, static_cast<unsigned int>(3) );
                      BOOST_CHECK_EQUAL( wndcases, static_cast<unsigned int>(115) );
                      BOOST_CHECK_CLOSE( expected, 11.01, 0.001 );
                      BOOST_CHECK_CLOSE( ode, 10.44, 0.001 );
                      BOOST_CHECK( boost::math::isinf<double>(rr) );
                      BOOST_CHECK_CLOSE( excess, 115.00, 0.001 );
                      BOOST_CHECK_CLOSE( teststat, 176.686230, 0.00001 );
                      BOOST_CHECK_CLOSE( p_value, 0.001, 0.0001 ); break;
            case 79  :BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "node 342" );
                      BOOST_CHECK_EQUAL( nodecases, static_cast<unsigned int>(4) );
                      BOOST_CHECK_EQUAL( wndstart, static_cast<unsigned int>(3) );
                      BOOST_CHECK_EQUAL( wndend, static_cast<unsigned int>(4) );
                      BOOST_CHECK_EQUAL( wndcases, static_cast<unsigned int>(4) );
                      BOOST_CHECK_CLOSE( expected, 0.13, 0.001 );
                      BOOST_CHECK_CLOSE( ode, 30.59, 0.001 );
                      BOOST_CHECK( boost::math::isinf<double>(rr) );
                      BOOST_CHECK_CLOSE( excess, 4.00, 0.001 );
                      BOOST_CHECK_CLOSE( teststat, 9.827472, 0.00001 );
                      BOOST_CHECK_CLOSE( p_value, 0.997, 0.0001 ); break;
            case 80  :BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "node 341" );
                      BOOST_CHECK_EQUAL( nodecases, static_cast<unsigned int>(4) );
                      BOOST_CHECK_EQUAL( wndstart, static_cast<unsigned int>(3) );
                      BOOST_CHECK_EQUAL( wndend, static_cast<unsigned int>(4) );
                      BOOST_CHECK_EQUAL( wndcases, static_cast<unsigned int>(4) );
                      BOOST_CHECK_CLOSE( expected, 0.13, 0.001 );
                      BOOST_CHECK_CLOSE( ode, 30.59, 0.001 );
                      BOOST_CHECK( boost::math::isinf<double>(rr) );
                      BOOST_CHECK_CLOSE( excess, 4.00, 0.001 );
                      BOOST_CHECK_CLOSE( teststat, 9.827472, 0.00001 );
                      BOOST_CHECK_CLOSE( p_value, 0.997, 0.0001 ); break;
            default : break;
        }
        getCSVRow(stream, data);
    }
    if (dataRows != 80) BOOST_FAIL( "expecting 80 data rows, got" << dataRows );
    stream.close();
}

/* Tests the expected values of the data file writer with the tree-time scan conditioned on node. */
BOOST_AUTO_TEST_CASE( test_tree_time_condition_node_and_time ) {
    // set parameters to report csv data file
    _parameters.setConditionalType(Parameters::NODEANDTIME);
    _parameters.setGeneratingTableResults(true);
    _parameters.setPrintColumnHeaders(true);

    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

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
    std::vector<std::string>::iterator itrNodeCases = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::NODE_CASES_FIELD);
    std::vector<std::string>::iterator itrWndStart = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::START_WINDOW_FIELD);
    std::vector<std::string>::iterator itrWndEnd = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::END_WINDOW_FIELD);
    std::vector<std::string>::iterator itrWndCase = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::WNDW_CASES_FIELD);
    std::vector<std::string>::iterator itrExpected = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::EXPECTED_CASES_FIELD);
    std::vector<std::string>::iterator itrODE = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::OBSERVED_DIV_EXPECTED_FIELD);
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
        double ode; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrODE)).c_str(), ode) );
        double teststat; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrTestStat)).c_str(), teststat) );
        double p_value; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrPValue)).c_str(), p_value) );

        BOOST_CHECK_EQUAL( cut_num, dataRows );
        switch (dataRows) {
            case 1 :  BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "node 812" );
                      BOOST_CHECK_EQUAL( nodecases, static_cast<unsigned int>(68) );
                      BOOST_CHECK_EQUAL( wndstart, static_cast<unsigned int>(7) );
                      BOOST_CHECK_EQUAL( wndend, static_cast<unsigned int>(8) );
                      BOOST_CHECK_EQUAL( wndcases, static_cast<unsigned int>(68) );
                      BOOST_CHECK_CLOSE( expected, 11.40, 0.001 );
                      BOOST_CHECK_CLOSE( ode, 5.97, 0.001 );
                      BOOST_CHECK_CLOSE( teststat, 67.987519, 0.00001 );
                      BOOST_CHECK_CLOSE( p_value, 0.001, 0.0001 ); break;
            case 2 :  BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "node 1053" );
                      BOOST_CHECK_EQUAL( nodecases, static_cast<unsigned int>(43) );
                      BOOST_CHECK_EQUAL( wndstart, static_cast<unsigned int>(0) );
                      BOOST_CHECK_EQUAL( wndend, static_cast<unsigned int>(1) );
                      BOOST_CHECK_EQUAL( wndcases, static_cast<unsigned int>(41) );
                      BOOST_CHECK_CLOSE( expected, 3.64, 0.001 );
                      BOOST_CHECK_CLOSE( ode, 11.26, 0.001 );
                      BOOST_CHECK_CLOSE( teststat, 63.221840, 0.00001 );
                      BOOST_CHECK_CLOSE( p_value, 0.001, 0.0001 ); break;
            case 109 :BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "node 814" );
                      BOOST_CHECK_EQUAL( nodecases, static_cast<unsigned int>(2) );
                      BOOST_CHECK_EQUAL( wndstart, static_cast<unsigned int>(7) );
                      BOOST_CHECK_EQUAL( wndend, static_cast<unsigned int>(8) );
                      BOOST_CHECK_EQUAL( wndcases, static_cast<unsigned int>(2) );
                      BOOST_CHECK_CLOSE( expected, 0.34, 0.001 );
                      BOOST_CHECK_CLOSE( ode, 5.97, 0.001 );
                      BOOST_CHECK_CLOSE( teststat, 1.910231, 0.00001 );
                      BOOST_CHECK_CLOSE( p_value, 0.989, 0.0001 ); break;
            case 110 :BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "node 815" );
                      BOOST_CHECK_EQUAL( nodecases, static_cast<unsigned int>(2) );
                      BOOST_CHECK_EQUAL( wndstart, static_cast<unsigned int>(7) );
                      BOOST_CHECK_EQUAL( wndend, static_cast<unsigned int>(8) );
                      BOOST_CHECK_EQUAL( wndcases, static_cast<unsigned int>(2) );
                      BOOST_CHECK_CLOSE( expected, 0.34, 0.001 );
                      BOOST_CHECK_CLOSE( ode, 5.97, 0.001 );
                      BOOST_CHECK_CLOSE( teststat, 1.910231, 0.00001 );
                      BOOST_CHECK_CLOSE( p_value, 0.989, 0.0001 ); break;
            default : break;
        }
        getCSVRow(stream, data);
    }
    if (dataRows != 110) BOOST_FAIL( "expecting 110 data rows, got" << dataRows );
    stream.close();
}

/* Tests the expected values of the data file writer with the tree-time scan conditioned on node, with day of week adjustment. */
BOOST_AUTO_TEST_CASE( test_tree_time_condition_node_and_time_day_of_week_adjustment ) {
    // set parameters to report csv data file
    _parameters.setConditionalType(Parameters::NODEANDTIME);
    _parameters.setPerformDayOfWeekAdjustment(true);
    _parameters.setGeneratingTableResults(true);
    _parameters.setPrintColumnHeaders(true);

    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

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
    std::vector<std::string>::iterator itrNodeCases = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::NODE_CASES_FIELD);
    std::vector<std::string>::iterator itrWndStart = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::START_WINDOW_FIELD);
    std::vector<std::string>::iterator itrWndEnd = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::END_WINDOW_FIELD);
    std::vector<std::string>::iterator itrWndCase = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::WNDW_CASES_FIELD);
    std::vector<std::string>::iterator itrExpected = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::EXPECTED_CASES_FIELD);
    std::vector<std::string>::iterator itrODE = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::OBSERVED_DIV_EXPECTED_FIELD);
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
        double ode; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrODE)).c_str(), ode) );
        double teststat; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrTestStat)).c_str(), teststat) );
        double p_value; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrPValue)).c_str(), p_value) );

        BOOST_CHECK_EQUAL( cut_num, dataRows );
        switch (dataRows) {
            case 1 :  BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "node 1053" );
                      BOOST_CHECK_EQUAL( nodecases, static_cast<unsigned int>(43) );
                      BOOST_CHECK_EQUAL( wndstart, static_cast<unsigned int>(0) );
                      BOOST_CHECK_EQUAL( wndend, static_cast<unsigned int>(1) );
                      BOOST_CHECK_EQUAL( wndcases, static_cast<unsigned int>(41) );
                      BOOST_CHECK_CLOSE( expected, 13.77, 0.001 );
                      BOOST_CHECK_CLOSE( ode, 2.98, 0.001 );
                      BOOST_CHECK_CLOSE( teststat, 18.224382, 0.00001 );
                      BOOST_CHECK_CLOSE( p_value, 0.001, 0.0001 ); break;
            case 2 :  BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "node 1054" );
                      BOOST_CHECK_EQUAL( nodecases, static_cast<unsigned int>(43) );
                      BOOST_CHECK_EQUAL( wndstart, static_cast<unsigned int>(0) );
                      BOOST_CHECK_EQUAL( wndend, static_cast<unsigned int>(1) );
                      BOOST_CHECK_EQUAL( wndcases, static_cast<unsigned int>(41) );
                      BOOST_CHECK_CLOSE( expected, 13.77, 0.001 );
                      BOOST_CHECK_CLOSE( ode, 2.98, 0.001 );
                      BOOST_CHECK_CLOSE( teststat, 18.224382, 0.00001 );
                      BOOST_CHECK_CLOSE( p_value, 0.001, 0.0001 ); break;
            case 35  :BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "node 66" );
                      BOOST_CHECK_EQUAL( nodecases, static_cast<unsigned int>(25) );
                      BOOST_CHECK_EQUAL( wndstart, static_cast<unsigned int>(5) );
                      BOOST_CHECK_EQUAL( wndend, static_cast<unsigned int>(6) );
                      BOOST_CHECK_EQUAL( wndcases, static_cast<unsigned int>(25) );
                      BOOST_CHECK_CLOSE( expected, 20.16, 0.001 );
                      BOOST_CHECK_CLOSE( ode, 1.24, 0.001 );
                      BOOST_CHECK_CLOSE( teststat, 0.563094, 0.00001 );
                      BOOST_CHECK_CLOSE( p_value, 0.961, 0.0001 ); break;
            case 36  :BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "node 38" );
                      BOOST_CHECK_EQUAL( nodecases, static_cast<unsigned int>(45) );
                      BOOST_CHECK_EQUAL( wndstart, static_cast<unsigned int>(6) );
                      BOOST_CHECK_EQUAL( wndend, static_cast<unsigned int>(7) );
                      BOOST_CHECK_EQUAL( wndcases, static_cast<unsigned int>(25) );
                      BOOST_CHECK_CLOSE( expected, 20.16, 0.001 );
                      BOOST_CHECK_CLOSE( ode, 1.24, 0.001 );
                      BOOST_CHECK_CLOSE( teststat, 0.563094, 0.00001 );
                      BOOST_CHECK_CLOSE( p_value, 0.961, 0.0001 ); break;
            default : break;
        }
        getCSVRow(stream, data);
    }
    if (dataRows != 36) BOOST_FAIL( "expecting 36 data rows, got" << dataRows );
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

    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

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
    std::vector<std::string>::iterator itrODE = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::OBSERVED_DIV_EXPECTED_FIELD);
    std::vector<std::string>::iterator itrRR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::RELATIVE_RISK_FIELD);
    std::vector<std::string>::iterator itrExcess = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::EXCESS_CASES_FIELD);
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
        double ode; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrODE)).c_str(), ode) );
        double rr; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrRR)).c_str(), rr, false) );
        double excess; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrExcess)).c_str(), excess) );
        double llr; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrLLR)).c_str(), llr) );
        double p_value; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrPValue)).c_str(), p_value) );

        BOOST_CHECK_EQUAL( cut_num, dataRows );
        switch (dataRows) {
            case 1 :  BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "All" );
                      BOOST_CHECK_EQUAL( nodecases, static_cast<unsigned int>(543) );
                      BOOST_CHECK_EQUAL( wndstart, static_cast<unsigned int>(1) );
                      BOOST_CHECK_EQUAL( wndend, static_cast<unsigned int>(8) );
                      BOOST_CHECK_EQUAL( wndcases, static_cast<unsigned int>(516) );
                      BOOST_CHECK_CLOSE( expected, 160.89, 0.001 );
                      BOOST_CHECK_CLOSE( ode, 3.21, 0.001 );
                      BOOST_CHECK_CLOSE( rr, 45.39, 0.001 );
                      BOOST_CHECK_CLOSE( excess, 504.63, 0.001 );
                      BOOST_CHECK_CLOSE( llr, 529.796056, 0.00001 );
                      BOOST_CHECK_CLOSE( p_value, 0.001, 0.0001 ); break;
            default : break;
        }
        getCSVRow(stream, data);
    }
    if (dataRows != 1) BOOST_FAIL( "expecting 1 data rows, got" << dataRows );
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

    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

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
    std::vector<std::string>::iterator itrODE = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::OBSERVED_DIV_EXPECTED_FIELD);
    std::vector<std::string>::iterator itrRR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::RELATIVE_RISK_FIELD);
    std::vector<std::string>::iterator itrExcess = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::EXCESS_CASES_FIELD);
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
        double ode; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrODE)).c_str(), ode) );
        double rr; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrRR)).c_str(), rr, false) );
        double excess; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrExcess)).c_str(), excess) );
        double teststat; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrteststat)).c_str(), teststat) );
        double p_value; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrPValue)).c_str(), p_value) );

        BOOST_CHECK_EQUAL( cut_num, dataRows );
        switch (dataRows) {
            case 1 :  BOOST_CHECK_EQUAL( data.at(std::distance(headers.begin(), itrNodeId)), "All" );
                      BOOST_CHECK_EQUAL( nodecases, static_cast<unsigned int>(543) );
                      BOOST_CHECK_EQUAL( wndstart, static_cast<unsigned int>(1) );
                      BOOST_CHECK_EQUAL( wndend, static_cast<unsigned int>(8) );
                      BOOST_CHECK_EQUAL( wndcases, static_cast<unsigned int>(516) );
                      BOOST_CHECK_CLOSE( expected, 170.00, 0.001 );
                      BOOST_CHECK_CLOSE( ode, 3.04, 0.001 );
                      BOOST_CHECK_CLOSE( rr, 41.93, 0.001 );
                      BOOST_CHECK_CLOSE( excess, 503.69, 0.001 );
                      BOOST_CHECK_CLOSE( teststat, 502.024076, 0.00001 );
                      BOOST_CHECK_CLOSE( p_value, 0.001, 0.0001 ); break;
            default : break;
        }
        getCSVRow(stream, data);
    }
    if (dataRows != 1) BOOST_FAIL( "expecting 1 data rows, got" << dataRows );
    stream.close();
}

BOOST_AUTO_TEST_SUITE_END()
