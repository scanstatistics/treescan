//------------------------------------------------------------------------------
//
//                           TreeScan C++ code
//
//                      Written by: Martin Kulldorff
//                  January, 2004; revised September 2007; revised September 2011
//
//------------------------------------------------------------------------------
#include <iostream>
#include "ScanRunner.h"
#include "PrintScreen.h"
#include "FileName.h"
#include "Parameters.h"
#include "Toolkit.h"

#include <boost/program_options.hpp>
namespace po = boost::program_options;

void __TreeScanInit(const char * sApplicationFullPath) {
  AppToolkit::ToolKitCreate(sApplicationFullPath);
}

void __TreeScanExit() {
  AppToolkit::ToolKitDestroy();
}

// A helper function to simplify the main part.
template<class T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v) {
    std::copy(v.begin(), v.end(), std::ostream_iterator<T>(std::cout, " "));
    return os;
}

void usage_message(std::string program, po::options_description& required, po::options_description& optional, PrintScreen& console) {
    FileName            exe(program.c_str());
    std::stringstream   message;
    message << "Usage: " << exe.getFileName().c_str() << exe.getExtension().c_str();
#ifdef _WINDOWS_
    message << " -t <tree file> -c <count file> -p <output file>";
#else
    message << " --tree-file <file> --count-file <file> --output-file <file>";
#endif
    message << " [options]" << std::endl << std::endl << required << std::endl << optional;
    console.Printf(message.str().c_str(), BasePrint::P_STDOUT);
}

int main(int argc, char* argv[]) {
    PrintScreen console(false);
    Parameters parameters;
    int replicas, limit_threads;
    bool duplicates = false, conditional=false, output_html=false;
    po::variables_map vm;

    try {
        __TreeScanInit(argv[0]);
        /* required */
        po::options_description required("required");
        required.add_options()("tree-file,t", po::value<std::string>(), "Input file defining tree structure.")
                              ("count-file,c", po::value<std::string>(), "Input file identifer counts and population.")
                              ("output-file,p", po::value<std::string>(), "Output filename to print results.");

        /* options */
        po::options_description optional("options");
        optional.add_options()("help,h", "Help")
                              ("version,v", "Program version")
                              ("replications,r", po::value<int>(&replicas)->default_value(99999), "Number of Monte Carlo replicatons.")
                              ("duplicates,d", po::bool_switch(&duplicates), "Expect duplicates in count file.")
                              ("limit-threads,l", po::value<int>(&limit_threads)->default_value(0), "Limit threads in simulation.")
                              ("conditional,n", po::bool_switch(&conditional), "Perform conditional analysis.")
                              ("output-html,m", po::bool_switch(&output_html), "Print results as html.");

        /* parse program options */
        po::options_description cmdline_options;
        cmdline_options.add(required).add(optional);
        try {
            po::store(po::command_line_parser(argc, argv).options(cmdline_options).run(), vm);
            po::notify(vm);
        } catch (std::exception& x) {
            console.Printf("Program options error: %s\n\n", BasePrint::P_ERROR, x.what());
            usage_message(argv[0], required, optional, console);
            __TreeScanExit();
            return 1;
        }

        if (vm.count("help")) {usage_message(argv[0], required, optional, console); return 0;}
        if (vm.count("version")) {console.Printf("TreeScan %s.%s.%s %s.\n", BasePrint::P_STDOUT, VERSION_MAJOR, VERSION_MINOR, VERSION_RELEASE, VERSION_PHASE); return 0;}
        if (!vm.count("tree-file"))  {console.Printf("Missing tree-file parameter.\n\n", BasePrint::P_STDOUT); usage_message(argv[0], required, optional, console); return 1;}
        if (!vm.count("count-file"))  {console.Printf("Missing count-file parameter.\n\n", BasePrint::P_STDOUT); usage_message(argv[0], required, optional, console); return 1;}
        if (!vm.count("output-file"))  {console.Printf("Missing output-file parameter.\n\n", BasePrint::P_STDOUT); usage_message(argv[0], required, optional, console); return 1;}

        std::string buffer;
        console.Printf(AppToolkit::getToolkit().GetAcknowledgment(buffer), BasePrint::P_STDOUT);
        parameters.setNumProcesses(static_cast<unsigned int>(limit_threads));
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        __TreeScanExit();
        return 1;
    } catch (...) {
        std::cerr << "Error: Unknown program expection." << std::endl;
        __TreeScanExit();
        return 1;
    }

    try {
        parameters.setTreeFileName(vm["tree-file"].as<std::string>().c_str());
        parameters.setCountFileName(vm["count-file"].as<std::string>().c_str());
        parameters.setOutputFileName(vm["output-file"].as<std::string>().c_str());
        parameters.setResultsFormat(output_html ? Parameters::HTML : Parameters::TEXT);
        parameters.setNumReplications(static_cast<unsigned int>(replicas));
        parameters.setConditional(conditional);
        parameters.setDuplicates(duplicates);

        ScanRunner runner(parameters, console);
        runner.run();
        __TreeScanExit();
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        __TreeScanExit();
        return 1;
    } catch (...) {
        std::cerr << "Error: Unknown program expection." << std::endl;
        __TreeScanExit();
        return 1;
    }

  return 0;
}
