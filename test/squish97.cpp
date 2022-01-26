
// project files
#include "fixture_examples.h"
#include "ScanRunner.h"
#include "Toolkit.h"
#include "DataFileWriter.h"

/* Regression unit test for issue https://www.squishlist.com/ims/treescan/97/.
   Temporal scan statistic with censoring.
*/

struct squish97_TreeTemporal_censordata_fixture : prm_testset_fixture {
    squish97_TreeTemporal_censordata_fixture() : prm_testset_fixture("/squish97/TreeTemporal-censordata.prm") { }
    virtual ~squish97_TreeTemporal_censordata_fixture() { /*BOOST_TEST_MESSAGE( "teardown fixture" );*/ }

    std::string _results_user_directory;
};

struct squish97_TreeTemporal_timeonly_censordata_fixture : prm_testset_fixture {
    squish97_TreeTemporal_timeonly_censordata_fixture() : prm_testset_fixture("/squish97/TreeTemporal-timeonly-censordata.prm") { }
    virtual ~squish97_TreeTemporal_timeonly_censordata_fixture() { /*BOOST_TEST_MESSAGE( "teardown fixture" );*/ }

    std::string _results_user_directory;
};

/* Test Suite for the squish 97 regression. */
BOOST_AUTO_TEST_SUITE( squish97_suite )

/* Tests time tree analysis w/ censored data. */
BOOST_AUTO_TEST_CASE( test_treetime_censored ) {
    squish97_TreeTemporal_censordata_fixture censore_fixture;
    censore_fixture._parameters.setGeneratingTableResults(true);
    censore_fixture._parameters.setPrintColumnHeaders(true);
    censore_fixture._parameters.setReportAttributableRisk(true);
    censore_fixture._parameters.setAttributableRiskExposed(200);
    censore_fixture._parameters.setApplyingRiskWindowRestrictionCensored(false);

    run_analysis("test", censore_fixture._results_user_directory, censore_fixture._parameters, censore_fixture._print);

    // open the tabular results file and confirm expected columns and values match expected for this data set
    std::string buffer;
    std::ifstream stream;
    stream.open(CutsRecordWriter::getFilename(censore_fixture._parameters, buffer).c_str());
    if (!stream) throw std::exception("could not open file");

    CSV_Row_t headers;
    getCSVRow(stream, headers);
    if (headers.size() != static_cast<size_t>(15)) BOOST_FAIL("expecting 15 columns, got " << headers.size());
    std::vector<std::string>::iterator itrCutNum = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::CUT_NUM_FIELD);
    std::vector<std::string>::iterator itrNodeId = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::NODE_ID_FIELD);
    std::vector<std::string>::iterator itrTreeLevel = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::P_LEVEL_FLD);
    std::vector<std::string>::iterator itrNodeCases = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::NODE_CASES_FIELD);
    std::vector<std::string>::iterator itrWndStart = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::START_WINDOW_FIELD);
    std::vector<std::string>::iterator itrWndEnd = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::END_WINDOW_FIELD);
    std::vector<std::string>::iterator itrWndCases = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::WNDW_CASES_FIELD);
    std::vector<std::string>::iterator itrExpected = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::EXPECTED_CASES_FIELD);
    std::vector<std::string>::iterator itrRR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::RELATIVE_RISK_FIELD);
    std::vector<std::string>::iterator itrExcess = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::EXCESS_CASES_FIELD);
    std::vector<std::string>::iterator itrAR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::ATTRIBUTABLE_RISK_FIELD);
    std::vector<std::string>::iterator itrLLR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::LOG_LIKL_RATIO_FIELD);
    std::vector<std::string>::iterator itrPValue = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::P_VALUE_FLD);

    // check the expected values
    unsigned int dataRows = 0, childRows = 0;
    CSV_Row_t data;
    getCSVRow(stream, data);
    while (data.size()) {
        if (data.at(std::distance(headers.begin(), itrCutNum)).find("_") != std::string::npos) {
            ++childRows;
            getCSVRow(stream, data);
            continue;
        }
        ++dataRows;
        unsigned int cut_num; BOOST_CHECK(string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrCutNum)).c_str(), cut_num));
        unsigned int treelevel; BOOST_CHECK(string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrTreeLevel)).c_str(), treelevel));
        unsigned int nodecases; BOOST_CHECK(string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrNodeCases)).c_str(), nodecases));
        unsigned int wndstart; BOOST_CHECK(string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrWndStart)).c_str(), wndstart));
        unsigned int wndend; BOOST_CHECK(string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrWndEnd)).c_str(), wndend));
        unsigned int wndcases; BOOST_CHECK(string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrWndCases)).c_str(), wndcases));
        double expected; BOOST_CHECK(string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrExpected)).c_str(), expected));
        double rr; BOOST_CHECK(string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrRR)).c_str(), rr));
        double excess; BOOST_CHECK(string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrExcess)).c_str(), excess));
        double ar; BOOST_CHECK(string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrAR)).c_str(), ar));
        double llr; BOOST_CHECK(string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrLLR)).c_str(), llr));
        double p_value; BOOST_CHECK(string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrPValue)).c_str(), p_value));

        BOOST_CHECK_EQUAL(cut_num, dataRows);
        switch (dataRows) {
        case 1: BOOST_CHECK_EQUAL(data.at(std::distance(headers.begin(), itrNodeId)), "Node3");
            BOOST_CHECK_EQUAL(treelevel, static_cast<unsigned int>(3));
            BOOST_CHECK_EQUAL(nodecases, static_cast<unsigned int>(46));
            BOOST_CHECK_EQUAL(wndstart, static_cast<unsigned int>(4));
            BOOST_CHECK_EQUAL(wndend, static_cast<unsigned int>(5));
            BOOST_CHECK_EQUAL(wndcases, static_cast<unsigned int>(21));
            BOOST_CHECK_CLOSE(expected, 3.29, 0.001);
            BOOST_CHECK_CLOSE(rr, 10.92, 0.001);
            BOOST_CHECK_CLOSE(excess, 19.08, 0.001);
            BOOST_CHECK_CLOSE(ar, 0.095, 0.00001);
            BOOST_CHECK_CLOSE(llr, 25.562266, 0.00001);
            BOOST_CHECK_CLOSE(p_value, 0.001, 0.0001); break;
        case 2: BOOST_CHECK_EQUAL(data.at(std::distance(headers.begin(), itrNodeId)), "Node6");
            BOOST_CHECK_EQUAL(treelevel, static_cast<unsigned int>(2));
            BOOST_CHECK_EQUAL(nodecases, static_cast<unsigned int>(55));
            BOOST_CHECK_EQUAL(wndstart, static_cast<unsigned int>(4));
            BOOST_CHECK_EQUAL(wndend, static_cast<unsigned int>(7));
            BOOST_CHECK_EQUAL(wndcases, static_cast<unsigned int>(29));
            BOOST_CHECK_CLOSE(expected, 7.86, 0.001);
            BOOST_CHECK_CLOSE(rr, 6.69, 0.001);
            BOOST_CHECK_CLOSE(excess, 24.67, 0.001);
            BOOST_CHECK_CLOSE(ar, 0.12, 0.00001);
            BOOST_CHECK_CLOSE(llr, 22.398076, 0.00001);
            BOOST_CHECK_CLOSE(p_value, 0.001, 0.0001); break;
        case 3: BOOST_CHECK_EQUAL(data.at(std::distance(headers.begin(), itrNodeId)), "Root");
            BOOST_CHECK_EQUAL(treelevel, static_cast<unsigned int>(1));
            BOOST_CHECK_EQUAL(nodecases, static_cast<unsigned int>(98));
            BOOST_CHECK_EQUAL(wndstart, static_cast<unsigned int>(2));
            BOOST_CHECK_EQUAL(wndend, static_cast<unsigned int>(7));
            BOOST_CHECK_EQUAL(wndcases, static_cast<unsigned int>(52));
            BOOST_CHECK_CLOSE(expected, 21.59, 0.001);
            BOOST_CHECK_CLOSE(rr, 4.00, 0.001);
            BOOST_CHECK_CLOSE(excess, 39.00, 0.001);
            BOOST_CHECK_CLOSE(ar, 0.20, 0.00001);
            BOOST_CHECK_CLOSE(llr, 22.363116, 0.00001);
            BOOST_CHECK_CLOSE(p_value, 0.001, 0.0001); break;
        default: break;
        }
        getCSVRow(stream, data);
    }
    if (dataRows != 15) BOOST_FAIL("expecting 15 data rows, got " << dataRows);
    if (childRows != 15) BOOST_FAIL("expecting 15 child data rows, got " << childRows);
    stream.close();
}

/* Tests time only analysis w/ censored data. */
BOOST_AUTO_TEST_CASE(test_timeonly_censored) {
    squish97_TreeTemporal_timeonly_censordata_fixture censore_fixture;
    censore_fixture._parameters.setGeneratingTableResults(true);
    censore_fixture._parameters.setPrintColumnHeaders(true);
    censore_fixture._parameters.setReportAttributableRisk(true);
    censore_fixture._parameters.setAttributableRiskExposed(200);

    run_analysis("test", censore_fixture._results_user_directory, censore_fixture._parameters, censore_fixture._print);

    // open the tabular results file and confirm expected columns and values match expected for this data set
    std::string buffer;
    std::ifstream stream;
    stream.open(CutsRecordWriter::getFilename(censore_fixture._parameters, buffer).c_str());
    if (!stream) throw std::exception("could not open file");

    CSV_Row_t headers;
    getCSVRow(stream, headers);
    if (headers.size() != static_cast<size_t>(12)) BOOST_FAIL("expecting 12 columns, got " << headers.size());
    std::vector<std::string>::iterator itrCutNum = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::CUT_NUM_FIELD);
    std::vector<std::string>::iterator itrNodeId = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::NODE_ID_FIELD);
    std::vector<std::string>::iterator itrNodeCases = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::NODE_CASES_FIELD);
    std::vector<std::string>::iterator itrWndStart = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::START_WINDOW_FIELD);
    std::vector<std::string>::iterator itrWndEnd = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::END_WINDOW_FIELD);
    std::vector<std::string>::iterator itrWndCases = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::WNDW_CASES_FIELD);
    std::vector<std::string>::iterator itrExpected = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::EXPECTED_CASES_FIELD);
    std::vector<std::string>::iterator itrRR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::RELATIVE_RISK_FIELD);
    std::vector<std::string>::iterator itrExcess = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::EXCESS_CASES_FIELD);
    std::vector<std::string>::iterator itrAR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::ATTRIBUTABLE_RISK_FIELD);
    std::vector<std::string>::iterator itrLLR = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::LOG_LIKL_RATIO_FIELD);
    std::vector<std::string>::iterator itrPValue = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::P_VALUE_FLD);

    // check the expected values
    unsigned int dataRows = 0, childRows = 0;
    CSV_Row_t data;
    getCSVRow(stream, data);
    while (data.size()) {
        if (data.at(std::distance(headers.begin(), itrCutNum)).find("_") != std::string::npos) {
            ++childRows;
            getCSVRow(stream, data);
            continue;
        }
        ++dataRows;
        unsigned int cut_num; BOOST_CHECK(string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrCutNum)).c_str(), cut_num));
        unsigned int nodecases; BOOST_CHECK(string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrNodeCases)).c_str(), nodecases));
        unsigned int wndstart; BOOST_CHECK(string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrWndStart)).c_str(), wndstart));
        unsigned int wndend; BOOST_CHECK(string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrWndEnd)).c_str(), wndend));
        unsigned int wndcases; BOOST_CHECK(string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrWndCases)).c_str(), wndcases));
        double expected; BOOST_CHECK(string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrExpected)).c_str(), expected));
        double rr; BOOST_CHECK(string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrRR)).c_str(), rr));
        double excess; BOOST_CHECK(string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrExcess)).c_str(), excess));
        double ar; BOOST_CHECK(string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrAR)).c_str(), ar));
        double llr; BOOST_CHECK(string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrLLR)).c_str(), llr));
        double p_value; BOOST_CHECK(string_to_numeric_type<double>(data.at(std::distance(headers.begin(), itrPValue)).c_str(), p_value));

        BOOST_CHECK_EQUAL(cut_num, dataRows);
        switch (dataRows) {
        case 1: BOOST_CHECK_EQUAL(data.at(std::distance(headers.begin(), itrNodeId)), "All");
            BOOST_CHECK_EQUAL(nodecases, static_cast<unsigned int>(98));
            BOOST_CHECK_EQUAL(wndstart, static_cast<unsigned int>(2));
            BOOST_CHECK_EQUAL(wndend, static_cast<unsigned int>(7));
            BOOST_CHECK_EQUAL(wndcases, static_cast<unsigned int>(52));
            BOOST_CHECK_CLOSE(expected, 21.59, 0.001);
            BOOST_CHECK_CLOSE(rr, 4.00, 0.001);
            BOOST_CHECK_CLOSE(excess, 39.00, 0.001);
            BOOST_CHECK_CLOSE(ar, 0.20, 0.00001);
            BOOST_CHECK_CLOSE(llr, 22.363116, 0.00001);
            BOOST_CHECK_CLOSE(p_value, 0.001, 0.0001); break;
        case 2: BOOST_CHECK_EQUAL(data.at(std::distance(headers.begin(), itrNodeId)), "All");
            BOOST_CHECK_EQUAL(nodecases, static_cast<unsigned int>(98));
            BOOST_CHECK_EQUAL(wndstart, static_cast<unsigned int>(2));
            BOOST_CHECK_EQUAL(wndend, static_cast<unsigned int>(8));
            BOOST_CHECK_EQUAL(wndcases, static_cast<unsigned int>(54));
            BOOST_CHECK_CLOSE(expected, 25.19, 0.001);
            BOOST_CHECK_CLOSE(rr, 3.55, 0.001);
            BOOST_CHECK_CLOSE(excess, 38.78, 0.001);
            BOOST_CHECK_CLOSE(ar, 0.19, 0.00001);
            BOOST_CHECK_CLOSE(llr, 19.016506, 0.00001);
            BOOST_CHECK_CLOSE(p_value, 0.001, 0.0001); break;
        case 3: BOOST_CHECK_EQUAL(data.at(std::distance(headers.begin(), itrNodeId)), "All");
            BOOST_CHECK_EQUAL(nodecases, static_cast<unsigned int>(98));
            BOOST_CHECK_EQUAL(wndstart, static_cast<unsigned int>(3));
            BOOST_CHECK_EQUAL(wndend, static_cast<unsigned int>(6));
            BOOST_CHECK_EQUAL(wndcases, static_cast<unsigned int>(38));
            BOOST_CHECK_CLOSE(expected, 14.39, 0.001);
            BOOST_CHECK_CLOSE(rr, 3.68, 0.001);
            BOOST_CHECK_CLOSE(excess, 27.67, 0.001);
            BOOST_CHECK_CLOSE(ar, 0.14, 0.00001);
            BOOST_CHECK_CLOSE(llr, 16.983154, 0.00001);
            BOOST_CHECK_CLOSE(p_value, 0.001, 0.0001); break;
        default: break;
        }
        getCSVRow(stream, data);
    }
    if (dataRows != 16) BOOST_FAIL("expecting 16 data rows, got " << dataRows);
    if (childRows != 0) BOOST_FAIL("expecting 0 child data rows, got " << childRows);
    stream.close();
}

BOOST_AUTO_TEST_SUITE_END()
