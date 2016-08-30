
// project files
#include "fixture_examples.h"
#include "ScanRunner.h"
#include "Toolkit.h"
#include "DataFileWriter.h"
#include "ParametersValidate.h"
#include <boost/assign/std/vector.hpp>
using namespace boost::assign;

/* Test Suite for the ability to restrict evaluated levels in tree -- https://www.squishlist.com/ims/treescan/122/. */
BOOST_AUTO_TEST_SUITE( restrict_tree_levels_suite )

/* Tests the expected values of the attributable risk data  with conditional Poisson model. */
BOOST_FIXTURE_TEST_CASE( test_restrict_tree_levels, poisson_fixture ) {
    _parameters.setGeneratingTableResults(true);
    _parameters.setPrintColumnHeaders(true);
    BOOST_REQUIRE_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

    std::string results_user_directory;
    run_analysis("test", results_user_directory, _parameters, _print);

    // open the csv results file and get the reported levels
    std::string buffer;
    std::ifstream stream;
	stream.open(CutsRecordWriter::getFilename(_parameters, buffer).c_str());
    if (!stream) throw std::exception("could not open file");

    CSV_Row_t headers;
    getCSVRow(stream, headers);
    std::vector<std::string>::iterator itrLevel = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::P_LEVEL_FLD);

	std::set<unsigned int> levels;
    unsigned int dataRows=0, expectedRows=6;
    CSV_Row_t data;
    getCSVRow(stream, data);
    while (data.size()) {
        if (dataRows < expectedRows) {
            unsigned int level; BOOST_CHECK( string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrLevel)).c_str(), level) );
			levels.insert(level);
        }
        ++dataRows;
        getCSVRow(stream, data);
    }
    if (dataRows != expectedRows) BOOST_FAIL( "expecting " << expectedRows << " data rows, got " << dataRows );
    if (levels.size() != 3) BOOST_FAIL( "expecting " << 3 << " levels reported, got " << levels.size() );
    stream.close();

	// Now re-run the analysis - but restrict evaluation of the first level encountered in baseline run.
	_parameters.setRestrictTreeLevels(true);
	Parameters::RestrictTreeLevels_t restricted_levels;
	for (std::set<unsigned int>::const_iterator itr=levels.begin(); itr != levels.end(); ++itr)
		restricted_levels.push_back(*itr);
	unsigned int only_level = restricted_levels.back();
	restricted_levels.pop_back();
	_parameters.setRestrictedTreeLevels(restricted_levels);
    run_analysis("test", results_user_directory, _parameters, _print);

	stream.open(CutsRecordWriter::getFilename(_parameters, buffer).c_str());
    if (!stream) throw std::exception("could not open file");

    getCSVRow(stream, headers);
    itrLevel = getHeaderColumnIteratorOrFail(headers, DataRecordWriter::P_LEVEL_FLD);

    dataRows=0;
    getCSVRow(stream, data);
    while (data.size()) {
        unsigned int level; BOOST_CHECK( string_to_numeric_type<unsigned int>(data.at(std::distance(headers.begin(), itrLevel)).c_str(), level) );
		if (level != only_level) BOOST_FAIL( "restricted tree level " << *levels.begin() << " still exits in results file" );
        ++dataRows;
        getCSVRow(stream, data);
    }
    stream.close();
    if (dataRows == 0) BOOST_FAIL( "no rows tested");
}

BOOST_AUTO_TEST_SUITE_END()
