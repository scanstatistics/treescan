// Boost unit test header
#include <boost/test/unit_test.hpp>

#include "MinimalGrowthArray.h"

#include <functional>
#include <string>
#include <vector>

BOOST_AUTO_TEST_SUITE( test_minimal_growth_array )

BOOST_AUTO_TEST_CASE( default_constructed_array_is_empty ) {
    MinimalGrowthArray<double> values;

    BOOST_CHECK( values.empty() );
    BOOST_CHECK_EQUAL( values.size(), 0 );
    BOOST_CHECK( values.getArray() == 0 );
    BOOST_REQUIRE_THROW( values.at(0), std::out_of_range );
}

BOOST_AUTO_TEST_CASE( size_constructor_fills_values ) {
    MinimalGrowthArray<double> values(3, 7.25);

    BOOST_CHECK( !values.empty() );
    BOOST_CHECK_EQUAL( values.size(), 3 );
    BOOST_CHECK_EQUAL( values[0], 7.25 );
    BOOST_CHECK_EQUAL( values[1], 7.25 );
    BOOST_CHECK_EQUAL( values.at(2), 7.25 );
    BOOST_REQUIRE_THROW( values.at(3), std::out_of_range );
}

BOOST_AUTO_TEST_CASE( vector_constructor_and_get_round_trip_values ) {
    std::vector<double> source;
    source.push_back( 3.5 );
    source.push_back( 1.25 );
    source.push_back( 4.75 );

    MinimalGrowthArray<double> values(source);

    std::vector<double> target;
    BOOST_CHECK( &values.get(target) == &target );
    BOOST_REQUIRE_EQUAL( target.size(), source.size() );
    BOOST_CHECK_EQUAL( target[0], 3.5 );
    BOOST_CHECK_EQUAL( target[1], 1.25 );
    BOOST_CHECK_EQUAL( target[2], 4.75 );
}

BOOST_AUTO_TEST_CASE( copy_constructor_and_assignment_make_independent_copies ) {
    std::vector<double> source;
    source.push_back( 1.25 );
    source.push_back( 2.5 );
    source.push_back( 3.5 );

    MinimalGrowthArray<double> original(source);
    MinimalGrowthArray<double> copy(original);
    MinimalGrowthArray<double> assigned;
    assigned = original;

    original[0] = 99.5;

    BOOST_CHECK_EQUAL( copy[0], 1.25 );
    BOOST_CHECK_EQUAL( copy[1], 2.5 );
    BOOST_CHECK_EQUAL( assigned[0], 1.25 );
    BOOST_CHECK_EQUAL( assigned[2], 3.5 );
    BOOST_CHECK( copy != original );
    BOOST_CHECK( !(copy != assigned) );
}

BOOST_AUTO_TEST_CASE( set_clear_and_exists_update_storage ) {
    MinimalGrowthArray<double> values;
    std::vector<double> source;
    source.push_back( 5.5 );
    source.push_back( 8.25 );

    values.set(source);

    BOOST_CHECK_EQUAL( values.size(), 2 );
    BOOST_CHECK( values.exists(5.5) );
    BOOST_CHECK( values.exists(8.25) );
    BOOST_CHECK( !values.exists(13.75) );

    values.clear();

    BOOST_CHECK( values.empty() );
    BOOST_CHECK_EQUAL( values.size(), 0 );
    BOOST_CHECK( values.getArray() == 0 );
}

BOOST_AUTO_TEST_CASE( add_can_append_or_insert_in_sorted_order ) {
    MinimalGrowthArray<double> values;

    values.add(3.5);
    values.add(1.25);
    values.add(2.75, true);

    BOOST_REQUIRE_EQUAL( values.size(), 3 );
    BOOST_CHECK_EQUAL( values[0], 1.25 );
    BOOST_CHECK_EQUAL( values[1], 2.75 );
    BOOST_CHECK_EQUAL( values[2], 3.5 );
}

BOOST_AUTO_TEST_CASE( sort_uses_default_or_custom_order ) {
    std::vector<double> source;
    source.push_back( 4.75 );
    source.push_back( 2.5 );
    source.push_back( 7.75 );

    MinimalGrowthArray<double> values(source);
    values.sort();

    BOOST_CHECK_EQUAL( values[0], 2.5 );
    BOOST_CHECK_EQUAL( values[1], 4.75 );
    BOOST_CHECK_EQUAL( values[2], 7.75 );

    values.sort(std::greater<double>());

    BOOST_CHECK_EQUAL( values[0], 7.75 );
    BOOST_CHECK_EQUAL( values[1], 4.75 );
    BOOST_CHECK_EQUAL( values[2], 2.5 );
}

BOOST_AUTO_TEST_CASE( template_supports_string_values ) {
    std::vector<std::string> source;
    source.push_back( "oak" );
    source.push_back( "ash" );

    MinimalGrowthArray<std::string> values(source);
    values.add("elm", true);

    BOOST_REQUIRE_EQUAL( values.size(), 3 );
    BOOST_CHECK_EQUAL( values[0], "ash" );
    BOOST_CHECK_EQUAL( values[1], "elm" );
    BOOST_CHECK_EQUAL( values[2], "oak" );
    BOOST_CHECK( values.exists("elm") );
}

BOOST_AUTO_TEST_SUITE_END()
