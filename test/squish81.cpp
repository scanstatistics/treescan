
// project files
#include "fixture_examples.h"
#include "ScanRunner.h"
#include "Toolkit.h"
#include "DataFileWriter.h"

struct power_estimation_fixture : prm_testset_fixture {
    power_estimation_fixture() : prm_testset_fixture("/power-estimate/power-estimate.prm") { }
    virtual ~power_estimation_fixture() { /*BOOST_TEST_MESSAGE( "teardown fixture -- power_estimation_fixture" );*/ }

    std::string _results_user_directory;
};

/* Test Suite for the power estimation data file write. */
BOOST_FIXTURE_TEST_SUITE( power_estimation_suite, power_estimation_fixture )

/* Tests the expected values of the power estimation data file writer. */
BOOST_AUTO_TEST_CASE( test_power_estimation ) {
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

BOOST_AUTO_TEST_SUITE_END()
