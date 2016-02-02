
// project files
#include "fixture_examples.h"
#include "ScanRunner.h"
#include "Toolkit.h"
#include "DataFileWriter.h"

struct poisson_power_estimation_fixture : prm_testset_fixture {
    poisson_power_estimation_fixture() : prm_testset_fixture("/power-estimate/power-estimate.prm") { }
    virtual ~poisson_power_estimation_fixture() {}

    std::string _results_user_directory;
};

struct bernoulli_power_estimation_fixture : prm_testset_fixture {
    bernoulli_power_estimation_fixture() : prm_testset_fixture("/power-estimate/power-estimate-bernoulli.prm") { }
    virtual ~bernoulli_power_estimation_fixture() {}

    std::string _results_user_directory;
};

struct treetemporal_power_estimation_fixture : prm_testset_fixture {
    treetemporal_power_estimation_fixture() : prm_testset_fixture("/power-estimate/power-estimate-tree-temporal.prm") { }
    virtual ~treetemporal_power_estimation_fixture() {}

    std::string _results_user_directory;
};

struct timeonly_power_estimation_fixture : prm_testset_fixture {
    timeonly_power_estimation_fixture() : prm_testset_fixture("/power-estimate/power-estimate-timeonly.prm") { }
    virtual ~timeonly_power_estimation_fixture() {}

    std::string _results_user_directory;
};

/* Test Suite for the power estimation data file write. */
BOOST_AUTO_TEST_SUITE( power_estimation_suite )

/* Tests the expected values of the power estimation data file writer for time-only conditioned on total cases. */
BOOST_FIXTURE_TEST_CASE( test_timeonly_power_estimation, timeonly_power_estimation_fixture ) {
    BOOST_CHECK(_parameters.getScanType() ==  Parameters::TIMEONLY);
    BOOST_CHECK(_parameters.getConditionalType() ==  Parameters::TOTALCASES);
    run_analysis("test", _results_user_directory, _parameters, _print);

    // open the power estimation file and confirm expected columns and values match expected for this data set
    std::string buffer;
    printString(buffer, "test%s%s", PowerEstimationRecordWriter::POWER_FILE_SUFFIX, CSVDataFileWriter::CSV_FILE_EXT);
    std::ifstream stream;
    getFileStream(stream, buffer, _results_user_directory);

    CSV_Row_t headers;
    getCSVRow(stream, headers);
    std::vector<std::string>::iterator itrCol1 = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::HYPOTHESIS_ALTERNATIVE_NUM_FIELD);
    std::vector<std::string>::iterator itrCol2 = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::HA_ALPHA05_FIELD);
    std::vector<std::string>::iterator itrCol3 = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::HA_ALPHA01_FIELD);
    std::vector<std::string>::iterator itrCol4 = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::HA_ALPHA001_FIELD);

    // check the expected values this analysis -- there should be 3 sets of power estimations
    unsigned int dataRows=0;
    CSV_Row_t data;
    getCSVRow(stream, data);
    while (data.size()) {
        ++dataRows;
        unsigned int ha_num; BOOST_CHECK( string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrCol1)).c_str(), ha_num) );
        double alpha05; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrCol2)).c_str(), alpha05) );
        double alpha01; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrCol3)).c_str(), alpha01) );
        double alpha001; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrCol4)).c_str(), alpha001) );

        BOOST_CHECK_EQUAL( ha_num, dataRows );
        switch (dataRows) {
            case 1 : BOOST_REQUIRE_CLOSE( alpha05, 0.083, 0.001 );
                     BOOST_REQUIRE_CLOSE( alpha01, 0.026, 0.001 );
                     BOOST_REQUIRE_CLOSE( alpha001, 0.007, 0.001 ); break;
            case 2 : BOOST_REQUIRE_CLOSE( alpha05, 0.965, 0.001 );
                     BOOST_REQUIRE_CLOSE( alpha01, 0.883, 0.001 );
                     BOOST_REQUIRE_CLOSE( alpha001, 0.790, 0.001 ); break;
            default : break;
        }
        getCSVRow(stream, data);
    }
    if (dataRows != 2) BOOST_FAIL( "expecting 2 data rows, got" << dataRows );
    stream.close();
}

/* Tests the expected values of the power estimation data file writer for tree-time conditioned on node. */
BOOST_FIXTURE_TEST_CASE( test_conditional_treetime_power_estimation, treetemporal_power_estimation_fixture ) {
    BOOST_CHECK(_parameters.getScanType() ==  Parameters::TREETIME);
    BOOST_CHECK(_parameters.getConditionalType() ==  Parameters::NODE);
    run_analysis("test", _results_user_directory, _parameters, _print);

    // open the power estimation file and confirm expected columns and values match expected for this data set
    std::string buffer;
    printString(buffer, "test%s%s", PowerEstimationRecordWriter::POWER_FILE_SUFFIX, CSVDataFileWriter::CSV_FILE_EXT);
    std::ifstream stream;
    getFileStream(stream, buffer, _results_user_directory);

    CSV_Row_t headers;
    getCSVRow(stream, headers);
    std::vector<std::string>::iterator itrCol1 = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::HYPOTHESIS_ALTERNATIVE_NUM_FIELD);
    std::vector<std::string>::iterator itrCol2 = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::HA_ALPHA05_FIELD);
    std::vector<std::string>::iterator itrCol3 = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::HA_ALPHA01_FIELD);
    std::vector<std::string>::iterator itrCol4 = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::HA_ALPHA001_FIELD);

    // check the expected values this analysis -- there should be 3 sets of power estimations
    unsigned int dataRows=0;
    CSV_Row_t data;
    getCSVRow(stream, data);
    while (data.size()) {
        ++dataRows;
        unsigned int ha_num; BOOST_CHECK( string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrCol1)).c_str(), ha_num) );
        double alpha05; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrCol2)).c_str(), alpha05) );
        double alpha01; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrCol3)).c_str(), alpha01) );
        double alpha001; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrCol4)).c_str(), alpha001) );

        BOOST_CHECK_EQUAL( ha_num, dataRows );
        switch (dataRows) {
            case 1 : BOOST_REQUIRE_CLOSE( alpha05, 0.038, 0.001 );
                     BOOST_REQUIRE_CLOSE( alpha01, 0.008, 0.001 );
                     BOOST_REQUIRE_CLOSE( alpha001, 0.001, 0.001 ); break;
            case 2 : BOOST_REQUIRE_CLOSE( alpha05, 0.051, 0.001 );
                     BOOST_REQUIRE_CLOSE( alpha01, 0.017, 0.001 );
                     BOOST_REQUIRE_CLOSE( alpha001, 0.007, 0.001 ); break;
            default : break;
        }
        getCSVRow(stream, data);
    }
    if (dataRows != 2) BOOST_FAIL( "expecting 2 data rows, got" << dataRows );
    stream.close();
}

/* Tests the expected values of the power estimation data file writer for condtional poisson. */
BOOST_FIXTURE_TEST_CASE( test_conditional_poisson_power_estimation, poisson_power_estimation_fixture ) {
    _parameters.setConditionalType(Parameters::TOTALCASES);
    run_analysis("test", _results_user_directory, _parameters, _print);

    // open the power estimation file and confirm expected columns and values match expected for this data set
    std::string buffer;
    printString(buffer, "test%s%s", PowerEstimationRecordWriter::POWER_FILE_SUFFIX, CSVDataFileWriter::CSV_FILE_EXT);
    std::ifstream stream;
    getFileStream(stream, buffer, _results_user_directory);

    CSV_Row_t headers;
    getCSVRow(stream, headers);
    std::vector<std::string>::iterator itrCol1 = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::HYPOTHESIS_ALTERNATIVE_NUM_FIELD);
    std::vector<std::string>::iterator itrCol2 = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::HA_ALPHA05_FIELD);
    std::vector<std::string>::iterator itrCol3 = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::HA_ALPHA01_FIELD);
    std::vector<std::string>::iterator itrCol4 = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::HA_ALPHA001_FIELD);

    // check the expected values this analysis -- there should be 3 sets of power estimations
    unsigned int dataRows=0;
    CSV_Row_t data;
    getCSVRow(stream, data);
    while (data.size()) {
        ++dataRows;
        unsigned int ha_num; BOOST_CHECK( string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrCol1)).c_str(), ha_num) );
        double alpha05; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrCol2)).c_str(), alpha05) );
        double alpha01; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrCol3)).c_str(), alpha01) );
        double alpha001; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrCol4)).c_str(), alpha001) );

        BOOST_CHECK_EQUAL( ha_num, dataRows );
        switch (dataRows) {
            case 1 : BOOST_REQUIRE_CLOSE( alpha05, 0.045, 0.001 );
                     BOOST_REQUIRE_CLOSE( alpha01, 0.005, 0.001 );
                     BOOST_REQUIRE_CLOSE( alpha001, 0.001, 0.001 ); break;
            case 2 : BOOST_REQUIRE_CLOSE( alpha05, 0.097, 0.001 );
                     BOOST_REQUIRE_CLOSE( alpha01, 0.011, 0.001 );
                     BOOST_REQUIRE_CLOSE( alpha001, 0.002, 0.001 ); break;
            case 3 : BOOST_REQUIRE_CLOSE( alpha05, 0.966, 0.001 );
                     BOOST_REQUIRE_CLOSE( alpha01, 0.855, 0.001 );
                     BOOST_REQUIRE_CLOSE( alpha001, 0.749, 0.001 ); break;
            default : break;
        }
        getCSVRow(stream, data);
    }
    if (dataRows != 3) BOOST_FAIL( "expecting 3 data rows, got" << dataRows );
    stream.close();
}

/* Tests the expected values of the power estimation data file writer for uncondtional poisson. */
BOOST_FIXTURE_TEST_CASE( test_unconditional_poisson_power_estimation, poisson_power_estimation_fixture ) {
    _parameters.setConditionalType(Parameters::UNCONDITIONAL);
    run_analysis("test", _results_user_directory, _parameters, _print);

    // open the power estimation file and confirm expected columns and values match expected for this data set
    std::string buffer;
    printString(buffer, "test%s%s", PowerEstimationRecordWriter::POWER_FILE_SUFFIX, CSVDataFileWriter::CSV_FILE_EXT);
    std::ifstream stream;
    getFileStream(stream, buffer, _results_user_directory);

    CSV_Row_t headers;
    getCSVRow(stream, headers);
    std::vector<std::string>::iterator itrCol1 = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::HYPOTHESIS_ALTERNATIVE_NUM_FIELD);
    std::vector<std::string>::iterator itrCol2 = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::HA_ALPHA05_FIELD);
    std::vector<std::string>::iterator itrCol3 = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::HA_ALPHA01_FIELD);
    std::vector<std::string>::iterator itrCol4 = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::HA_ALPHA001_FIELD);

    // check the expected values this analysis -- there should be 3 sets of power estimations
    unsigned int dataRows=0;
    CSV_Row_t data;
    getCSVRow(stream, data);
    while (data.size()) {
        ++dataRows;
        unsigned int ha_num; BOOST_CHECK( string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrCol1)).c_str(), ha_num) );
        double alpha05; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrCol2)).c_str(), alpha05) );
        double alpha01; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrCol3)).c_str(), alpha01) );
        double alpha001; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrCol4)).c_str(), alpha001) );

        BOOST_CHECK_EQUAL( ha_num, dataRows );
        switch (dataRows) {
            case 1 : BOOST_REQUIRE_CLOSE( alpha05, 1.000, 0.001 );
                     BOOST_REQUIRE_CLOSE( alpha01, 0.988, 0.001 );
                     BOOST_REQUIRE_CLOSE( alpha001, 0.929, 0.001 ); break;
            case 2 : BOOST_REQUIRE_CLOSE( alpha05, 0.032, 0.001 );
                     BOOST_REQUIRE_CLOSE( alpha01, 0.004, 0.001 );
                     BOOST_REQUIRE_CLOSE( alpha001, 0.001, 0.001 ); break;
            case 3 : BOOST_REQUIRE_CLOSE( alpha05, 1.000, 0.001 );
                     BOOST_REQUIRE_CLOSE( alpha01, 1.000, 0.001 );
                     BOOST_REQUIRE_CLOSE( alpha001, 0.718, 0.001 ); break;
            default : break;
        }
        getCSVRow(stream, data);
    }
    if (dataRows != 3) BOOST_FAIL( "expecting 3 data rows, got" << dataRows );
    stream.close();
}

/* Tests the expected values of the power estimation data file writer for condtional bernoulli. */
BOOST_FIXTURE_TEST_CASE( test_conditional_bernoulli_power_estimation, bernoulli_power_estimation_fixture ) {
    _parameters.setConditionalType(Parameters::TOTALCASES);
    run_analysis("test", _results_user_directory, _parameters, _print);

    // open the power estimation file and confirm expected columns and values match expected for this data set
    std::string buffer;
    printString(buffer, "test%s%s", PowerEstimationRecordWriter::POWER_FILE_SUFFIX, CSVDataFileWriter::CSV_FILE_EXT);
    std::ifstream stream;
    getFileStream(stream, buffer, _results_user_directory);

    CSV_Row_t headers;
    getCSVRow(stream, headers);
    std::vector<std::string>::iterator itrCol1 = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::HYPOTHESIS_ALTERNATIVE_NUM_FIELD);
    std::vector<std::string>::iterator itrCol2 = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::HA_ALPHA05_FIELD);
    std::vector<std::string>::iterator itrCol3 = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::HA_ALPHA01_FIELD);
    std::vector<std::string>::iterator itrCol4 = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::HA_ALPHA001_FIELD);

    // check the expected values this analysis -- there should be 3 sets of power estimations
    unsigned int dataRows=0;
    CSV_Row_t data;
    getCSVRow(stream, data);
    while (data.size()) {
        ++dataRows;
        unsigned int ha_num; BOOST_CHECK( string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrCol1)).c_str(), ha_num) );
        double alpha05; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrCol2)).c_str(), alpha05) );
        double alpha01; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrCol3)).c_str(), alpha01) );
        double alpha001; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrCol4)).c_str(), alpha001) );

        BOOST_CHECK_EQUAL( ha_num, dataRows );
        switch (dataRows) {
            case 1 : BOOST_REQUIRE_CLOSE( alpha05, 0.019, 0.001 );
                     BOOST_REQUIRE_CLOSE( alpha01, 0.019, 0.001 );
                     BOOST_REQUIRE_CLOSE( alpha001, 0.001, 0.001 ); break;
            case 2 : BOOST_REQUIRE_CLOSE( alpha05, 0.044, 0.001 );
                     BOOST_REQUIRE_CLOSE( alpha01, 0.044, 0.001 );
                     BOOST_REQUIRE_CLOSE( alpha001, 0.003, 0.001 ); break;
            case 3 : BOOST_REQUIRE_CLOSE( alpha05, 0.049, 0.001 );
                     BOOST_REQUIRE_CLOSE( alpha01, 0.049, 0.001 );
                     BOOST_REQUIRE_CLOSE( alpha001, 0.003, 0.001 ); break;
            default : break;
        }
        getCSVRow(stream, data);
    }
    if (dataRows != 3) BOOST_FAIL( "expecting 3 data rows, got" << dataRows );
    stream.close();
}

/* Tests the expected values of the power estimation data file writer for uncondtional bernoulli. */
BOOST_FIXTURE_TEST_CASE( test_unconditional_bernoulli_power_estimation, bernoulli_power_estimation_fixture ) {
    _parameters.setConditionalType(Parameters::UNCONDITIONAL);
    run_analysis("test", _results_user_directory, _parameters, _print);

    // open the power estimation file and confirm expected columns and values match expected for this data set
    std::string buffer;
    printString(buffer, "test%s%s", PowerEstimationRecordWriter::POWER_FILE_SUFFIX, CSVDataFileWriter::CSV_FILE_EXT);
    std::ifstream stream;
    getFileStream(stream, buffer, _results_user_directory);

    CSV_Row_t headers;
    getCSVRow(stream, headers);
    std::vector<std::string>::iterator itrCol1 = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::HYPOTHESIS_ALTERNATIVE_NUM_FIELD);
    std::vector<std::string>::iterator itrCol2 = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::HA_ALPHA05_FIELD);
    std::vector<std::string>::iterator itrCol3 = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::HA_ALPHA01_FIELD);
    std::vector<std::string>::iterator itrCol4 = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::HA_ALPHA001_FIELD);

    // check the expected values this analysis -- there should be 3 sets of power estimations
    unsigned int dataRows=0;
    CSV_Row_t data;
    getCSVRow(stream, data);
    while (data.size()) {
        ++dataRows;
        unsigned int ha_num; BOOST_CHECK( string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrCol1)).c_str(), ha_num) );
        double alpha05; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrCol2)).c_str(), alpha05) );
        double alpha01; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrCol3)).c_str(), alpha01) );
        double alpha001; BOOST_CHECK( string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrCol4)).c_str(), alpha001) );

        BOOST_CHECK_EQUAL( ha_num, dataRows );
        switch (dataRows) {
            case 1 : BOOST_REQUIRE_CLOSE( alpha05, 0.245, 0.001 );
                     BOOST_REQUIRE_CLOSE( alpha01, 0.019, 0.001 );
                     BOOST_REQUIRE_CLOSE( alpha001, 0.001, 0.001 ); break;
            case 2 : BOOST_REQUIRE_CLOSE( alpha05, 0.548, 0.001 );
                     BOOST_REQUIRE_CLOSE( alpha01, 0.199, 0.001 );
                     BOOST_REQUIRE_CLOSE( alpha001, 0.008, 0.001 ); break;
            case 3 : BOOST_REQUIRE_CLOSE( alpha05, 0.659, 0.001 );
                     BOOST_REQUIRE_CLOSE( alpha01, 0.282, 0.001 );
                     BOOST_REQUIRE_CLOSE( alpha001, 0.012, 0.001 ); break;
            default : break;
        }
        getCSVRow(stream, data);
    }
    if (dataRows != 3) BOOST_FAIL( "expecting 3 data rows, got" << dataRows );
    stream.close();
}

BOOST_AUTO_TEST_SUITE_END()
