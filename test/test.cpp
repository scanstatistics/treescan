
#include <boost/test/included/unit_test.hpp>
//#include <boost/test/test_case_template.hpp>
//#include <boost/mpl/list.hpp>


using namespace boost::unit_test;


test_suite * init_unit_test_suite( int argc, char* argv[] )
{
    //boost::unit_test::unit_test_log.set_threshold_level( boost::unit_test::log_all_errors );

    //framework::master_test_suite().add( BOOST_TEST_CASE( &free_test_function ) );
    //framework::master_test_suite().add( BOOST_TEST_CASE( &free_test_function_fail ) );
    //framework::master_test_suite().add( BOOST_TEST_CASE( &free_test_function_fail_another ) );

    //boost::shared_ptr<test_class> tester( new test_class );

    //framework::master_test_suite().add( BOOST_TEST_CASE( boost::bind( &test_class::test_method1, tester )));
    //framework::master_test_suite().add( BOOST_TEST_CASE( boost::bind( &test_class::test_method2, tester )));

    //typedef boost::mpl::list<int,long,unsigned char> test_types;
    //framework::master_test_suite().add( BOOST_TEST_CASE_TEMPLATE( my_test, test_types ) );
    return 0;
}

