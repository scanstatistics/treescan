// Boost unit test header
#include <boost/test/unit_test.hpp>
#include <boost/assign/std/vector.hpp>
using namespace boost::assign;
#include "UtilityFunctions.h"

/* Test Suite for the TreeScan::cumulative templated function. */
BOOST_AUTO_TEST_SUITE( test_cumulative_function_suite )

BOOST_AUTO_TEST_CASE( test1_cumulative_function ) {
    std::vector<int> int_collection;
    int_collection += 1;
    TreeScan::cumulative(int_collection);
    BOOST_CHECK( int_collection.front() == 1 );
}

BOOST_AUTO_TEST_CASE( test2_cumulative_function ) {
    std::vector<int> int_collection;
    int_collection += 1,1;
    TreeScan::cumulative(int_collection);
    BOOST_CHECK( int_collection.front() == 2 );
}

BOOST_AUTO_TEST_CASE( test3_cumulative_function ) {
    std::vector<int> int_collection;
    int_collection += 1,1,1;
    TreeScan::cumulative(int_collection);
    BOOST_CHECK( int_collection.front() == 3 );
}

BOOST_AUTO_TEST_CASE( test4_cumulative_function ) {
    std::vector<int> int_collection;
    int_collection += 1,2,3,4,5,6,7,8,9,10;
    TreeScan::cumulative(int_collection);
    BOOST_CHECK( int_collection.front() == 55 );
}

BOOST_AUTO_TEST_CASE( test5_cumulative_function ) {
    std::vector<int> int_collection;
    int_collection += 0,0,3,0,5,0,7,0,9,0;
    TreeScan::cumulative(int_collection);
    BOOST_CHECK( int_collection.front() == 24 );
}

BOOST_AUTO_TEST_SUITE_END()
