//***************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//***************************************************************************
#include <iostream>
#include "ScanRunner.h"
#include "PrintScreen.h"
#include "FileName.h"
#include "Parameters.h"
#include "Toolkit.h"
#include "ParameterFileAccess.h"
#include "ParametersValidate.h"
#include "ParameterProgramOptions.h"
#include "ParametersPrint.h"

#include <boost/program_options.hpp>
namespace po = boost::program_options;

void __TreeScanInit(const char * sApplicationFullPath) {
  AppToolkit::ToolKitCreate(sApplicationFullPath);
}

void __TreeScanExit() {
  AppToolkit::ToolKitDestroy();
}

void usage_message(std::string program, po::options_description& desc, const ParameterProgramOptions& prgOptions, ParameterProgramOptions::ParamOptContainer_t& opt_descriptions, bool displayParams, PrintScreen& console) {
    FileName            exe(program.c_str());
    std::stringstream   message;
    message << std::endl << "Usage: " << exe.getFileName().c_str() << exe.getExtension().c_str() << " <parameter file>(optional) [options]";
    message << std::endl << std::endl << desc;

    message << std::endl << "The parameter file is an optional argument. You can define settings in 3 ways:" << std::endl << std::endl;
    message << " All parameter settings specified with a parameter file." << std::endl;
    message << "   example: " << exe.getFileName().c_str() << exe.getExtension().c_str() << " <parameter filename>" << std::endl << std::endl;
    message << " All parameter settings specified with command-line arguments -- see 'display-parameters'." << std::endl;
    message << "   example: " << exe.getFileName().c_str() << exe.getExtension().c_str() << " --" << prgOptions.getOption(Parameters::TREE_FILE) 
        << " <tree filename> --" << prgOptions.getOption(Parameters::COUNT_FILE) <<" <count filename> ..."<< std::endl << std::endl;
    message << " Default parameter settings defined in a parameter file then override with command-line arguments." << std::endl;
    message << "   example: " << exe.getFileName().c_str() << exe.getExtension().c_str() << " <parameter filename> --"
            << prgOptions.getOption(Parameters::TREE_FILE) << " <tree filename> --" << prgOptions.getOption(Parameters::COUNT_FILE) << " <count filename> --"
            << prgOptions.getOption(Parameters::RESULTS_FILE) << " <results filename>" << std::endl;

    if (displayParams) {
        for (size_t t=0; t < opt_descriptions.size(); ++t) {
            if (opt_descriptions[t]->get<1>())
                message << std::endl << std::endl << opt_descriptions[t]->get<0>();
            if (opt_descriptions[t]->get<2>().size())
                message << std::endl << "  " << opt_descriptions[t]->get<2>();
        }
    }
    console.Print(message.str().c_str(), BasePrint::P_STDOUT);
}

int main(int argc, char* argv[]) {
    Parameters parameters;
    std::string sMessage;
    po::variables_map vm;
    PrintScreen console(false);
    bool verifyParameters=false, printParameters=false;

    try {
        __TreeScanInit(argv[0]);

        // general options
        po::options_description application("", 200);
        application.add_options()
            ("parameter-file,f", po::value<std::string>(), "parameter file")
            ("display-parameters,s", "display parameter options list")
            ("version,v", "program version")
            ("verify-parameters,c", po::bool_switch(&verifyParameters), "verify parameters only")
            ("print-parameters,p", po::bool_switch(&printParameters), "print parameters only")
            ("write-parameters,w", po::value<std::string>(), "write parameters to file")
            ("help,h", "Help");

        // try to determine if user has specified parameter options version
        Parameters::CreationVersion opts_version;
        try {
            // positional options
            po::positional_options_description pd;
            pd.add("parameter-file", -1);
            const po::parsed_options& options = po::command_line_parser(argc, argv).options(application).allow_unregistered().style(po::command_line_style::default_style|po::command_line_style::case_insensitive).positional(pd).run();
            for (size_t opt=0; opt < options.options.size(); ++opt) {
                if (options.options.at(opt).string_key == "override-version" || options.options.at(opt).string_key == "n") {
                    if (sscanf(options.options.at(opt).value.front().c_str(), "%u.%u.%u", &opts_version.iMajor, &opts_version.iMinor, &opts_version.iRelease) < 3)
                        throw resolvable_error("Invalid 'options-version' specified '%s', format of #.#.# expected.", options.options.at(opt).string_key.c_str());
                }
            }
        } catch (std::exception& x) {
            console.Printf("Program options error: %s\n", BasePrint::P_ERROR, x.what());
            __TreeScanExit();
            return 1;
        }

        // hidden options
        bool force_censored_algorithm = false;
        po::options_description hidden("Hidden options", 200);
        hidden.add_options()("force-censor-algorithm,k", po::bool_switch(&force_censored_algorithm), "Force Censor Algorithm");

        // positional options
        po::positional_options_description pd; 
        pd.add("parameter-file", 1);
        // parse program options
        po::options_description cmdline_options;
        // define parameter options based upon determined version
        ParameterProgramOptions parameterOptions(parameters, opts_version, console);
        ParameterProgramOptions::ParamOptContainer_t opt_descriptions;
        parameterOptions.getOptions(opt_descriptions);
        for (size_t t=0; t < opt_descriptions.size(); ++t)
            cmdline_options.add(opt_descriptions[t]->get<0>());
        cmdline_options.add(application).add(hidden);
        // display help if no additional arguments specified 
        if (argc < 2) {
            usage_message(argv[0], application, parameterOptions, opt_descriptions, false, console);
            return 1;
        }
        try {
            po::store(po::command_line_parser(argc, argv).options(cmdline_options).style(po::command_line_style::default_style|po::command_line_style::case_insensitive).positional(pd).run(), vm);
            po::notify(vm);
        } catch (std::exception& x) {
            console.Printf("Program options error: %s\n", BasePrint::P_ERROR, x.what());
            __TreeScanExit();
            return 1;
        }

        // program options processing
        if (vm.count("help")) {usage_message(argv[0], application, parameterOptions, opt_descriptions, false, console); return 0;}
        if (vm.count("version")) {console.Printf("TreeScan %s.%s.%s %s.\n", BasePrint::P_STDOUT, VERSION_MAJOR, VERSION_MINOR, VERSION_RELEASE, VERSION_PHASE); return 0;}
        if (vm.count("display-parameters")) {usage_message(argv[0], application, parameterOptions, opt_descriptions, true, console); return 0;}
        // read parameter file
        if (vm.count("parameter-file")) {
            if (!ParameterAccessCoordinator(parameters).read(vm["parameter-file"].as<std::string>().c_str(), console))
                throw resolvable_error("\nThe parameter settings that prevent TreeScan from continuing.\n"
                                       "Please review above message(s) and modify parameter settings accordingly.");
        }

        // apply parameter overrides
        if (!parameterOptions.setParameterOverrides(vm)) {
            throw resolvable_error("\nThe parameter settings that prevent TreeScan from continuing.\n"
                                   "Please review above message(s) and modify parameter settings accordingly.");
        }
        // write parameters to file, if requested
        if (vm.count("write-parameters")) {
            ParameterAccessCoordinator(parameters).write(vm["write-parameters"].as<std::string>().c_str(), console);
        }
        if (force_censored_algorithm) parameters.setForcedCensoredAlgorithm(true);
        // validate parameters - print errors to console
        if (!ParametersValidate(parameters).Validate(console))
            throw resolvable_error("\nThe parameter file contains incorrect settings that prevent TreeScan from continuing. "
                                   "Please review above message(s) and modify parameter settings accordingly.");
        // additional program options processing
        if (printParameters) {ParametersPrint(parameters).Print(std::cout); return 0;}
        if (verifyParameters) {console.Printf("Parameters verified, no setting errors detected.\n", BasePrint::P_STDOUT); return 0;}

        std::string buffer;
        console.Printf(AppToolkit::getToolkit().GetAcknowledgment(buffer), BasePrint::P_STDOUT);
        ScanRunner(parameters, console).run();
        __TreeScanExit();
    } catch (resolvable_error & x) {
        console.Printf("%s\n\nUse '--help' to get help with program options.\n", BasePrint::P_ERROR, x.what());
        __TreeScanExit();
        return 1;
    } catch (prg_exception& x) {
        console.Printf("\n\nJob cancelled due to an unexpected program error.\n\n"
                      "Please contact technical support with the following information:\n"
                       "%s\n%s\n", BasePrint::P_ERROR, x.what(), x.trace());
        __TreeScanExit();
        return 1;
    } catch (std::bad_alloc &x) {
        console.Printf("\nTreeScan is unable to perform analysis due to insufficient memory.\n", BasePrint::P_ERROR);
        __TreeScanExit();
        return 1;
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
