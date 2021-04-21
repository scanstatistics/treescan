
// project files
#include "fixture_examples.h"
#include "ScanRunner.h"
#include "Toolkit.h"
#include "DataFileWriter.h"
#include "DataSource.h"

/* Test Suite for the sequential scan. */
BOOST_AUTO_TEST_SUITE( sequential_scan_suite )

/* Tests the contents of the sequential scan file are consistent given the same settings. */
BOOST_FIXTURE_TEST_CASE( test_sequential_file_output, time_only_fixture ) {
    BOOST_CHECK(_parameters.getScanType() ==  Parameters::TIMEONLY);
    BOOST_CHECK(_parameters.getConditionalType() ==  Parameters::TOTALCASES);
    _parameters.setSequentialScan(true);
    _parameters.setSequentialMinimumSignal(3);
    _parameters.setSequentialMaximumSignal(200);
    _parameters.setNumProcesses(1);

    std::string results_user_directory;
    std::stringstream filename1, filename2;
    filename1 << GetUserTemporaryDirectory(results_user_directory).c_str() << "\\test_sequential.csv";
    filename2 << results_user_directory.c_str() << "\\test-scan-out-2.csv";

    // Run analysis, generating sequential scan file.
    //_parameters.setSequentialFilename(filename1.str().c_str());
	remove(filename1.str().c_str());
    BOOST_REQUIRE_EQUAL( ParametersValidate(_parameters).Validate(_print), true ); 
    run_analysis("test", results_user_directory, _parameters, _print);

    // Run analysis again, generating a separate sequential scan file.
    _parameters.setSequentialFilename(filename2.str().c_str());
    BOOST_REQUIRE_EQUAL( ParametersValidate(_parameters).Validate(_print), true );
    run_analysis("test", results_user_directory, _parameters, _print);

    // Open sequential source files and compare LLR values.
    SequentialFileDataSource source1(filename1.str(), _parameters);
    source1.gotoFirstRecord();
    SequentialFileDataSource source2(filename2.str(), _parameters);
    source2.gotoFirstRecord();
    for (unsigned int r=0; r < _parameters.getNumReplicationsRequested(); ++r) {
        boost::optional<double> loglikelihood1 = source1.nextLLR();
        boost::optional<double> loglikelihood2 = source2.nextLLR();
        if (!loglikelihood1 || !loglikelihood2) {
            BOOST_FAIL("Expecting loglikelihoods for index " << (r + 1) << "to have value in data record.");
        }
        if (loglikelihood1.get() != loglikelihood2.get()) {
            BOOST_FAIL("Expecting loglikelihoods for index " << (r + 1) << "be equal.");
        }
    }
}

/* Tests that sequential scans must user the same parameter settings for subsequent scans. */
BOOST_FIXTURE_TEST_CASE( test_select_parameters_cannot_change, time_only_fixture ) {
    BOOST_CHECK(_parameters.getScanType() ==  Parameters::TIMEONLY);
    BOOST_CHECK(_parameters.getConditionalType() ==  Parameters::TOTALCASES);
    _parameters.setSequentialScan(true);
    _parameters.setSequentialMinimumSignal(3);
    _parameters.setSequentialMaximumSignal(200);
    _parameters.setNumProcesses(1);

    std::string results_user_directory;
    std::stringstream filename;
    filename << GetUserTemporaryDirectory(results_user_directory).c_str() << "\\test_sequential.csv";

    // Run analysis, generating sequential scan file.
    //_parameters.setSequentialFilename(filename.str().c_str());
	remove(filename.str().c_str());
    BOOST_REQUIRE_EQUAL( ParametersValidate(_parameters).Validate(_print), true ); 
    run_analysis("test", results_user_directory, _parameters, _print);

    // Confirm that the written and checked parameter settings are as expected.
    SequentialScanLoglikelihoodRatioWriter::ParametersList_t parameter_types;
    SequentialScanLoglikelihoodRatioWriter::getSequentialParametersList(parameter_types);
    BOOST_CHECK(parameter_types.size() ==  13);

    // First 3 type are not relevant at this time - included for future use.
    BOOST_CHECK(parameter_types[0] ==  Parameters::SCAN_TYPE);
    BOOST_CHECK(parameter_types[1] ==  Parameters::CONDITIONAL_TYPE);
    BOOST_CHECK(parameter_types[2] ==  Parameters::MODEL_TYPE);

    // For each parameter type, change the setting and confirm that analysis throws resolvable_error.

    // data time range
    std::string buffer;
    Parameters test(_parameters);
    BOOST_CHECK(parameter_types[3] ==  Parameters::DATA_TIME_RANGES);
    printString(buffer, "[%d,%d]", test.getDataTimeRangeSet().getDataTimeRangeSets().front().getStart(),
                                   test.getDataTimeRangeSet().getDataTimeRangeSets().front().getEnd() + 1);
    test.setDataTimeRangeSet(DataTimeRangeSet(buffer, _parameters.getDatePrecisionType(), boost::optional<boost::gregorian::date>()));
    BOOST_REQUIRE_EQUAL( ParametersValidate(test).Validate(_print), true );
    BOOST_CHECK_THROW( run_analysis("test", results_user_directory, test, _print), resolvable_error );

    // window start range
    test = _parameters;
    BOOST_CHECK(parameter_types[4] ==  Parameters::START_DATA_TIME_RANGE);
    printString(buffer, "[%d,%d]", test.getTemporalStartRange().getStart(), test.getTemporalStartRange().getEnd() + 1);
    test.setTemporalStartRange(DataTimeRange(buffer, _parameters.getDatePrecisionType(), test.getDataTimeRangeSet().getDataTimeRangeSets().front().getDateStart()));
    BOOST_REQUIRE_EQUAL( ParametersValidate(test).Validate(_print), true );
    BOOST_CHECK_THROW( run_analysis("test", results_user_directory, test, _print), resolvable_error );

    // window end range
    test = _parameters;
    BOOST_CHECK(parameter_types[5] ==  Parameters::END_DATA_TIME_RANGE);
    printString(buffer, "[%d,%d]", test.getTemporalEndRange().getStart(), test.getTemporalEndRange().getEnd() + 1);
    test.setTemporalEndRange(DataTimeRange(buffer, _parameters.getDatePrecisionType(), test.getDataTimeRangeSet().getDataTimeRangeSets().front().getDateStart()));
    BOOST_REQUIRE_EQUAL( ParametersValidate(test).Validate(_print), true );
    BOOST_CHECK_THROW( run_analysis("test", results_user_directory, test, _print), resolvable_error );

    // maximum window as percentage
    test = _parameters;
    BOOST_CHECK(parameter_types[6] ==  Parameters::MAXIMUM_WINDOW_PERCENTAGE);
    BOOST_CHECK(test.getMaximumWindowType() ==  Parameters::PERCENTAGE_WINDOW);
    test.setMaximumWindowPercentage(test.getMaximumWindowPercentage() - 1.0);
    BOOST_REQUIRE_EQUAL( ParametersValidate(test).Validate(_print), true );
    BOOST_CHECK_THROW( run_analysis("test", results_user_directory, test, _print), resolvable_error );

    // maximum window as specified length
    test = _parameters;
    BOOST_CHECK(parameter_types[7] ==  Parameters::MAXIMUM_WINDOW_FIXED);
    BOOST_CHECK(parameter_types[8] ==  Parameters::MAXIMUM_WINDOW_TYPE);
    test.setMaximumWindowType(Parameters::FIXED_LENGTH);
    test.setMaximumWindowLength(test.getMaximumWindowLength() + 10);
    BOOST_REQUIRE_EQUAL( ParametersValidate(test).Validate(_print), true );
    BOOST_CHECK_THROW( run_analysis("test", results_user_directory, test, _print), resolvable_error );

    // number of replications
    test = _parameters;
    BOOST_CHECK(parameter_types[9] ==  Parameters::MINIMUM_WINDOW_FIXED);
    test.setMinimumWindowLength(test.getMinimumWindowLength() + 1);
    BOOST_REQUIRE_EQUAL( ParametersValidate(test).Validate(_print), true );
    BOOST_CHECK_THROW( run_analysis("test", results_user_directory, test, _print), resolvable_error );

    // number of replications
    test = _parameters;
    BOOST_CHECK(parameter_types[10] ==  Parameters::REPLICATIONS);
    test.setNumReplications(9);
    BOOST_REQUIRE_EQUAL( ParametersValidate(test).Validate(_print), false );
    //BOOST_CHECK_THROW( run_analysis("test", results_user_directory, test, _print), resolvable_error );

    // maximum cases to signal
    test = _parameters;
    BOOST_CHECK(parameter_types[11] ==  Parameters::SEQUENTIAL_MAX_SIGNAL);
    test.setSequentialMaximumSignal(test.getSequentialMaximumSignal() + 1);
    BOOST_REQUIRE_EQUAL( ParametersValidate(test).Validate(_print), true );
    BOOST_CHECK_THROW( run_analysis("test", results_user_directory, test, _print), resolvable_error );

    // minumum cases to signal
    test = _parameters;
    BOOST_CHECK(parameter_types[12] ==  Parameters::SEQUENTIAL_MIN_SIGNAL);
    test.setSequentialMinimumSignal(test.getSequentialMinimumSignal() + 1);
    BOOST_REQUIRE_EQUAL( ParametersValidate(test).Validate(_print), true );
    BOOST_CHECK_THROW( run_analysis("test", results_user_directory, test, _print), resolvable_error );
}

BOOST_AUTO_TEST_SUITE_END()
