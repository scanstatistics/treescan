#ifndef __FIXTURE_PRM_SOURCE_H
#define __FIXTURE_PRM_SOURCE_H

// Boost unit test header
#include <boost/test/unit_test.hpp>

// project files
#include "test_helper.h"
#include "Parameters.h"
#include "ParametersValidate.h"
#include "ParameterFileAccess.h"

struct parameter_fixture {
    parameter_fixture() {}
    virtual ~parameter_fixture() {}

    Parameters _parameters;
    PrintNull _print;
};

struct prm_fixture : parameter_fixture {
    prm_fixture(const std::string& filename, const std::string& path) {
        std::stringstream filepath;
        filepath << path.c_str() << "\\" << filename.c_str();
        // test reading parameters settings from file
        BOOST_CHECK_EQUAL( ParameterAccessCoordinator(_parameters).read(filepath.str().c_str(), _print), true );
        // make sure these parameters validated
        BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );
    }
    virtual ~prm_fixture() { }
};

struct prm_examples_fixture : prm_fixture {
    prm_examples_fixture(const std::string& filename) : prm_fixture(filename, getExamplesFilesPath()) {}
    virtual ~prm_examples_fixture() { }
};

struct prm_testset_fixture : prm_fixture {
    prm_testset_fixture(const std::string& filename) : prm_fixture(filename, getTestSetFilesPath()) {}
    virtual ~prm_testset_fixture() { }
};
#endif
