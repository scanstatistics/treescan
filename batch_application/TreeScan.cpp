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
#include "ParameterFileAccess.h"
#include "ParametersValidate.h"
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

void usage_message(std::string program, po::options_description& primary, po::options_description& optional, po::options_description& help, PrintScreen& console) {
    FileName            exe(program.c_str());
    std::stringstream   message;
    message << "Usage: " << std::endl;
#ifdef _WINDOWS_
    message  << exe.getFileName().c_str() << exe.getExtension().c_str() << " -s <settings file> [options]" << std::endl;
    message  << exe.getFileName().c_str() << exe.getExtension().c_str() << " -t <tree file> -c <count file> -p <output file> [options]" << std::endl;
#else
    message  << exe.getFileName().c_str() << exe.getExtension().c_str() << " --settings-file <settings file> [options]" << std::endl;
    message  << exe.getFileName().c_str() << exe.getExtension().c_str() << " --tree-file <tree file> --count-file <counts file> --output-file <output file> [options]" << std::endl;
#endif
    message << std::endl << primary << std::endl << optional << std::endl << help << std::endl;
    console.Printf(message.str().c_str(), BasePrint::P_STDOUT);
}

int main(int argc, char* argv[]) {
    PrintScreen console(false);
    Parameters parameters;
    po::variables_map vm;

    try {
        __TreeScanInit(argv[0]);
        /* required */
        po::options_description primary("primary options");
        primary.add_options()("settings-file,s", po::value<std::string>(), "Settings file with saved settings.")
                             ("tree-file,t", po::value<std::string>(), "Input file defining tree structure.")
                             ("cuts-file,u", po::value<std::string>(), "Input file defining node cuts.")
                             ("count-file,c", po::value<std::string>(), "Input file identifer counts and population.")
                             ("output-file,p", po::value<std::string>(), "Output filename to print results.");

        /* options */
        po::options_description optional("secondary options");
        optional.add_options()("replications,r", po::value<int>(), "Number of Monte Carlo replicatons (default=99999).")
                              //("duplicates,d", po::value<bool>(), "Expect duplicates in count file (default=false).")
                              ("limit-threads,l", po::value<int>(), "Limit threads in simulation (default=0).")
                              ("conditional,n", po::value<bool>(), "Perform conditional analysis (default=false).");

        /* help */
        po::options_description help("help");
        help.add_options()("help,h", "Help")
                          ("version,v", "Program version");

        /* parse program options */
        po::options_description cmdline_options;
        cmdline_options.add(primary).add(help).add(optional);
        try {
            po::store(po::command_line_parser(argc, argv).options(cmdline_options).run(), vm);
            po::notify(vm);
        } catch (std::exception& x) {
            console.Printf("Program options error: %s\n\n", BasePrint::P_ERROR, x.what());
            usage_message(argv[0], primary, optional, help, console);
            __TreeScanExit();
            return 1;
        }

        if (vm.count("help")) {usage_message(argv[0], primary, optional, help, console); return 0;}
        if (vm.count("version")) {console.Printf("TreeScan %s.%s.%s %s.\n", BasePrint::P_STDOUT, VERSION_MAJOR, VERSION_MINOR, VERSION_RELEASE, VERSION_PHASE); return 0;}
        if (!vm.count("settings-file") && !vm.count("tree-file"))  {console.Printf("Missing tree-file parameter.\n\n", BasePrint::P_STDOUT); usage_message(argv[0], primary, optional, help, console); return 1;}
        if (!vm.count("settings-file") && !vm.count("count-file"))  {console.Printf("Missing count-file parameter.\n\n", BasePrint::P_STDOUT); usage_message(argv[0], primary, optional, help, console); return 1;}
        if (!vm.count("settings-file") && !vm.count("output-file"))  {console.Printf("Missing output-file parameter.\n\n", BasePrint::P_STDOUT); usage_message(argv[0], primary, optional, help, console); return 1;}

        std::string buffer;
        console.Printf(AppToolkit::getToolkit().GetAcknowledgment(buffer), BasePrint::P_STDOUT);        
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
        if (vm.count("settings-file")) ParameterAccessCoordinator(parameters).read(vm["settings-file"].as<std::string>());
        if (vm.count("tree-file")) parameters.setTreeFileName(vm["tree-file"].as<std::string>().c_str());
        if (vm.count("cuts-file")) parameters.setCutsFileName(vm["cuts-file"].as<std::string>().c_str());
        if (vm.count("count-file")) parameters.setCountFileName(vm["count-file"].as<std::string>().c_str());
        if (vm.count("output-file")) parameters.setOutputFileName(vm["output-file"].as<std::string>().c_str());
        if (vm.count("replications")) parameters.setNumReplications(vm["replications"].as<int>());
        if (vm.count("duplicates")) parameters.setDuplicates(vm["duplicates"].as<bool>());
        if (vm.count("limit-threads")) parameters.setNumProcesses(vm["limit-threads"].as<int>());

        if (!ParametersValidate(parameters).Validate(console)) 
            throw resolvable_error("\nThe parameter file contains incorrect settings that prevent TreeScan from continuing. "
                                   "Please review above message(s) and modify parameter settings accordingly.");

        ScanRunner(parameters, console).run();
        __TreeScanExit();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        __TreeScanExit();
        return 1;
    } catch (...) {
        std::cerr << "Error: Unknown program expection." << std::endl;
        __TreeScanExit();
        return 1;
    }

  return 0;
}
