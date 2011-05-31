//------------------------------------------------------------------------------
//
//                           TreeScan C++ code
//
//                      Written by: Martin Kulldorff
//                  January, 2004; revised September 2007
//
//------------------------------------------------------------------------------
#include <iostream>
#include "ScanRunner.h"
#include "PrintScreen.h"
#include "FileName.h"

#include <boost/program_options.hpp>
namespace po = boost::program_options;

// A helper function to simplify the main part.
template<class T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v) {
    std::copy(v.begin(), v.end(), std::ostream_iterator<T>(cout, " ")); 
    return os;
}

void usage_message(std::string program, po::options_description& desc, PrintScreen& console) {
    FileName            exe(program.c_str());
    std::stringstream   message;
    message << "Usage: " << exe.getFileName().c_str() << exe.getExtension().c_str() << " [options]" << std::endl << desc;
    console.Printf(message.str().c_str(), BasePrint::P_STDOUT);
}

int main(int argc, char* argv[]) {
    PrintScreen console(false);
    int replicas, cuts;
    bool duplicates = false, conditional=false;
    po::variables_map vm;

    try {
        /* general options */
        po::options_description generic("options");
        generic.add_options()("help,h", "Help")
                             ("version,v", "Program version")
                             ("replications,r", po::value<int>(&replicas)->default_value(99999), "Number of Monte Carlo replicatons.")
                             ("cuts,s", po::value<int>(&cuts)->default_value(2000), "Number of most likely cuts that are saved.")
                             ("tree-file,t", po::value<std::string>(), "Input file defining tree structure.")
                             ("count-file,f", po::value<std::string>(), "Input file identifer counts and population.")
                             ("duplicates,d", po::bool_switch(&duplicates), "Expect duplicates in count file.")
                             ("output-file,p", po::value<std::string>(), "Output filename to print results.")
                             ("conditional,n", po::bool_switch(&conditional), "Perform conditional analysis.");

        /* parse program options */
        po::options_description cmdline_options;
        cmdline_options.add(generic);
        try {
            po::store(po::command_line_parser(argc, argv).options(cmdline_options).run(), vm);
            po::notify(vm);
        } catch (std::exception& x) {
            console.Printf("Program options error: %s\n", BasePrint::P_ERROR, x.what());
            return 1;
        }

        if (vm.count("help")) {usage_message(argv[0], generic, console); return 0;}
        if (vm.count("version")) {console.Printf("TreeScan %s.%s.%s %s.\n", BasePrint::P_STDOUT, VERSION_MAJOR, VERSION_MINOR, VERSION_RELEASE, VERSION_PHASE); return 0;}
        if (!vm.count("tree-file"))  {console.Printf("Missing tree-file parameter.\n", BasePrint::P_STDOUT); return 1;}
        if (!vm.count("count-file"))  {console.Printf("Missing count-file parameter.\n", BasePrint::P_STDOUT); return 1;}
        if (!vm.count("output-file"))  {console.Printf("Missing output-file parameter.\n", BasePrint::P_STDOUT); return 1;}
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Error: Unknown program expection." << std::endl;
        return 1;
    }

    try {
        ScanRunner runner(conditional, duplicates, cuts, replicas, console);
        runner.run(vm["tree-file"].as<std::string>(), vm["count-file"].as<std::string>(), vm["output-file"].as<std::string>());
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Error: Unknown program expection." << std::endl;
        return 1;
    }

  return 0;
}
