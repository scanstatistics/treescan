
// project files
#include "fixture_examples.h"
#include "ScanRunner.h"
#include "Toolkit.h"
#include "DataFileWriter.h"

/** Regression unit test for issue https://www.squishlist.com/ims/treescan/85/.
    Unconditional Poisson analysis with zero cases in count file -- performing power estimation.
*/

struct squish85_fixture : prm_testset_fixture {
    squish85_fixture() : prm_testset_fixture("/squish85/squish85.prm") { }
    virtual ~squish85_fixture() { /*BOOST_TEST_MESSAGE( "teardown fixture -- power_estimation_fixture" );*/ }

    std::string _results_user_directory;
};

/** Test Suite for the squish 85 regression. */
BOOST_FIXTURE_TEST_SUITE( squish85_suite, squish85_fixture )

/** Tests the expected power estimation values for unconditional Poisson with zero counts defined. */
BOOST_AUTO_TEST_CASE( test_unconditional_poison_power_estimation ) {
    _parameters.setDataOnlyOnLeaves(false);
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
            case 1 : BOOST_REQUIRE_CLOSE( alpha05, 0.797, 0.001 );
                     BOOST_REQUIRE_CLOSE( alpha01, 0.483, 0.001 );
                     BOOST_REQUIRE_CLOSE( alpha001, 0.111, 0.001 ); break;
            default : break;
        }
        getCSVRow(stream, data);
    }
    if (dataRows != 1) BOOST_FAIL( "expecting 1 data row, got" << dataRows );
    stream.close();
}

/** Tests that conditional Poisson, with zero counts defined, fails to run. */
BOOST_AUTO_TEST_CASE( test_conditional_poison_power_estimation ) {
    _parameters.setConditionalType(Parameters::TOTALCASES);
    BOOST_REQUIRE_THROW(run_analysis("test", _results_user_directory, _parameters, _print), resolvable_error);
}

BOOST_AUTO_TEST_SUITE_END()
