//******************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//******************************************************************************
#include "ParametersPrint.h"
#include "PrjException.h"
#include "RandomNumberGenerator.h"
#include "ResultsFileWriter.h"
#include "DataFileWriter.h"

/** Prints parameters, in a particular format, to passed ascii file. */
void ParametersPrint::Print(std::ostream& out) const {
    AsciiPrintFormat::PrintSectionSeparatorString(out, 0, 2);
    out << "PARAMETER SETTINGS" << std::endl;

    SettingContainer_t settings;
    // print 'Analysis' tab settings
    WriteSettingsContainer(getAnalysisParameters(settings), "Analysis", out);
    //print 'Input' tab settings
    WriteSettingsContainer(getInputParameters(settings), "Input", out);
    //print 'Output' tab settings
    WriteSettingsContainer(getOutputParameters(settings), "Output", out);
    //print 'Temporal Window' tab settings
    WriteSettingsContainer(getTemporalWindowParameters(settings), "Temporal Window", out);
    //print 'Inference' tab settings
    WriteSettingsContainer(getInferenceParameters(settings), "Inference", out);
    //print 'Power Evaluations' tab settings
    WriteSettingsContainer(getPowerEvaluationsParameters(settings), "Power Evaluations", out);
    //print 'Advanced Input' tab settings
    WriteSettingsContainer(getAdvancedInputParameters(settings), "Advanced Input", out);
    //print 'Additional Output' tab settings
    WriteSettingsContainer(getAdditionalOutputParameters(settings), "Additional Output", out);
    //print 'Power Simulations' tab settings
    WriteSettingsContainer(getPowerSimulationsParameters(settings), "Power Simulations", out);
    //print 'RunOptions' settings
    WriteSettingsContainer(getRunOptionsParameters(settings), "Run Options", out);
    //print 'System' parameters
    WriteSettingsContainer(getSystemParameters(settings), "System", out);
    AsciiPrintFormat::PrintSectionSeparatorString(out, 0, 1);
}

/** Prints parameters, in a particular format, to passed ascii file. */
void ParametersPrint::PrintHTML(std::ostream& out) const {
    SettingContainer_t settings;
    out << "<div id=\"parameter-settings\"><h4>Parameter Settings</h4>" << std::endl;
    // print 'Analysis' tab settings
    WriteSettingsContainerHTML(getAnalysisParameters(settings), "Analysis", out);
    //print 'Input' tab settings
    WriteSettingsContainerHTML(getInputParameters(settings), "Input", out);
    //print 'Output' tab settings
    WriteSettingsContainerHTML(getOutputParameters(settings), "Output", out);
    //print 'Temporal Window' tab settings
    WriteSettingsContainerHTML(getTemporalWindowParameters(settings), "Temporal Window", out);
    //print 'Inference' tab settings
    WriteSettingsContainerHTML(getInferenceParameters(settings), "Inference", out);
    //print 'Power Evaluations' tab settings
    WriteSettingsContainerHTML(getPowerEvaluationsParameters(settings), "Power Evaluations", out);
    //print 'Advanced Input' tab settings
    WriteSettingsContainerHTML(getAdvancedInputParameters(settings), "Advanced Input", out);
    //print 'Additional Output' tab settings
    WriteSettingsContainerHTML(getAdditionalOutputParameters(settings), "Additional Output", out);
    //print 'Power Simulations' tab settings
    WriteSettingsContainerHTML(getPowerSimulationsParameters(settings), "Power Simulations", out);
    //print 'RunOptions' settings
    WriteSettingsContainerHTML(getRunOptionsParameters(settings), "Run Options", out);
    //print 'System' parameters
    WriteSettingsContainerHTML(getSystemParameters(settings), "System", out);
    out << std::endl << "</div>" << std::endl;
}

/** Prints 'Input' tab parameters to file stream. */
ParametersPrint::SettingContainer_t & ParametersPrint::getInputParameters(SettingContainer_t & settings) const {
    std::string buffer;
    settings.clear();
    settings.push_back(std::make_pair("Tree File",_parameters.getTreeFileName()));
    settings.push_back(std::make_pair("Count File",_parameters.getCountFileName()));
    if (_parameters.getModelType() == Parameters::TEMPORALSCAN)
        settings.push_back(std::make_pair("Data Time Range",_parameters.getDataTimeRangeSet().toString(buffer)));
    //buffer = (_parameters.isDuplicates() ? "Yes" : "No");
    //settings.push_back(std::make_pair("Duplicates",buffer));
    return settings;
}

/** Prints 'Additional Output' tab parameters to file stream. */
ParametersPrint::SettingContainer_t & ParametersPrint::getAdditionalOutputParameters(SettingContainer_t & settings) const {
    std::string buffer;
    settings.clear();
    settings.push_back(std::make_pair("Report Simulated Log Likelihood Ratios",(_parameters.isGeneratingLLRResults() ? "Yes" : "No")));
    if (_parameters.isGeneratingLLRResults()) {
        settings.push_back(std::make_pair("Simulated Log Likelihood Ratios", LoglikelihoodRatioWriter::getFilename(_parameters, buffer, false)));
    }
    settings.push_back(std::make_pair("Report Critical Values",(_parameters.getReportCriticalValues() ? "Yes" : "No")));
    if (_parameters.isGeneratingTableResults() && _parameters.isPrintColumnHeaders()) {
        settings.push_back(std::make_pair("Print Column Headers","Yes"));
    }
    return settings;
}

/** Prints 'Advanced Input' tab parameters to file stream. */
ParametersPrint::SettingContainer_t & ParametersPrint::getAdvancedInputParameters(SettingContainer_t & settings) const {
    settings.clear();
    settings.push_back(std::make_pair("Cut File",_parameters.getCutsFileName()));
    return settings;
}

/** Prints 'Analysis' tab parameters to file stream. */
ParametersPrint::SettingContainer_t & ParametersPrint::getAnalysisParameters(SettingContainer_t & settings) const {
    std::string buffer;
    settings.clear();

    buffer = "Type of Scan";
    switch (_parameters.getScanType()) {
        case Parameters::TREEONLY : settings.push_back(std::make_pair(buffer,"Tree Only")); break;
        case Parameters::TREETIME : settings.push_back(std::make_pair(buffer,"Tree and Time")); break;
        default: throw prg_error("Unknown scan type (%d).", "getAnalysisParameters()", _parameters.getScanType());
    }
    buffer = "Conditional";
    switch (_parameters.getConditionalType()) {
        case Parameters::UNCONDITIONAL : settings.push_back(std::make_pair(buffer,"No (unconditional)")); break;
        case Parameters::TOTALCASES : settings.push_back(std::make_pair(buffer,"Total Cases")); break;
        case Parameters::CASESEACHBRANCH : settings.push_back(std::make_pair(buffer,"Cases on each Branch")); break;
        default: throw prg_error("Unknown conditional type (%d).", "getAnalysisParameters()", _parameters.getConditionalType());
    }
    switch (_parameters.getModelType()) {
        case Parameters::POISSON : settings.push_back(std::make_pair("Probability Model - Tree","Poisson")); break;
        case Parameters::BERNOULLI : settings.push_back(std::make_pair("Probability Model - Tree","Bernoulli")); break;
        case Parameters::TEMPORALSCAN : settings.push_back(std::make_pair("Probability Model - Time","Temporal Scan")); break;
        default: throw prg_error("Unknown model type (%d).", "getAnalysisParameters()", _parameters.getModelType());
    }
    if (_parameters.getModelType() == Parameters::BERNOULLI) {
        printString(buffer, "%u/%u", _parameters.getProbabilityRatio().first, _parameters.getProbabilityRatio().second);
        settings.push_back(std::make_pair("Case Probability",buffer));
    }
    if (_parameters.getModelType() == Parameters::TEMPORALSCAN) {
        settings.push_back(std::make_pair("Temporal Time Window Start", _parameters.getTemporalStartRange().toString(buffer)));
        settings.push_back(std::make_pair("Temporal Time Window End", _parameters.getTemporalEndRange().toString(buffer)));
    }
    return settings;
}

/** Prints 'Inference' tab parameters to file stream. */
ParametersPrint::SettingContainer_t & ParametersPrint::getInferenceParameters(SettingContainer_t & settings) const {
    std::string buffer;
    settings.clear();
    printString(buffer, "%u", _parameters.getNumReplicationsRequested());  
    settings.push_back(std::make_pair("Number of Replications",buffer));
    buffer = "Cut Type";
    switch (_parameters.getCutType()) {
        case Parameters::PAIRS : settings.push_back(std::make_pair(buffer,"Pairs")); break;
        case Parameters::TRIPLETS : settings.push_back(std::make_pair(buffer,"Triplets")); break;
        case Parameters::ORDINAL : settings.push_back(std::make_pair(buffer,"Ordinal")); break;
        case Parameters::COMBINATORIAL : settings.push_back(std::make_pair(buffer,"Combinatorial")); break;
        case Parameters::SIMPLE : //settings.push_back(std::make_pair(buffer,"Simple")); break;
        default: break;
    }
    return settings;
}

/** Prints 'Output' tab parameters to file stream. */
ParametersPrint::SettingContainer_t & ParametersPrint::getOutputParameters(SettingContainer_t & settings) const {
   std::string buffer;
    settings.clear();
    settings.push_back(std::make_pair("Results File",_parameters.getOutputFileName()));
    settings.push_back(std::make_pair("Report Results as HTML",(_parameters.isGeneratingHtmlResults() ? "Yes" : "No")));
    if (_parameters.isGeneratingHtmlResults()) {
        settings.push_back(std::make_pair("Results HTML", ResultsFileWriter::getFilenameHTML(_parameters, buffer)));
    }
    settings.push_back(std::make_pair("Report Results as CSV Table",(_parameters.isGeneratingTableResults() ? "Yes" : "No")));
    if (_parameters.isGeneratingTableResults()) {
        settings.push_back(std::make_pair("Results CSV Table", CutsRecordWriter::getFilename(_parameters, buffer)));
    }
    return settings;
}

/** Prints 'Power Evaluations' parameters to file stream. */
ParametersPrint::SettingContainer_t & ParametersPrint::getPowerEvaluationsParameters(SettingContainer_t & settings) const {
    std::string buffer, buffer2;
    settings.clear();
    if (_parameters.getModelType() == Parameters::POISSON || _parameters.getModelType() == Parameters::BERNOULLI) {
        settings.push_back(std::make_pair("Perform Power Evaluations", (_parameters.getPerformPowerEvaluations() ? "Yes" : "No")));
        if (!_parameters.getPerformPowerEvaluations()) return settings;
        buffer = "Power Evaluation Type";
        switch (_parameters.getPowerEvaluationType()) {
            case Parameters::PE_WITH_ANALYSIS: 
                settings.push_back(std::make_pair(buffer,"Standard Analysis and Power Evaluation Together")); break;
            case Parameters::PE_ONLY_CASEFILE:
                printString(buffer2, "Only Power Evaluation%s", (_parameters.getConditionalType() == Parameters::UNCONDITIONAL ? "" : ", Using Total Cases from Case File"));
                settings.push_back(std::make_pair(buffer,buffer2)); break;
            case Parameters::PE_ONLY_SPECIFIED_CASES: 
                settings.push_back(std::make_pair(buffer,"Only Power Evaluation, Using Defined Total Cases")); 
                printString(buffer, "%i", _parameters.getPowerEvaluationTotalCases());
                settings.push_back(std::make_pair("Power Evaluation Total Cases",buffer)); 
                break;
            default: throw prg_error("Unknown power evaluation type '%d'.\n", "PrintPowerEvaluationsParameters()", _parameters.getPowerEvaluationType());
        }
        buffer = "Critical Values";
        switch (_parameters.getCriticalValuesType()) {
            case Parameters::CV_MONTECARLO: 
                settings.push_back(std::make_pair(buffer,"Monte Carlo")); break;
            case Parameters::CV_POWER_VALUES: 
                settings.push_back(std::make_pair(buffer,"User Defined"));
                printString(buffer, "%lf", _parameters.getCriticalValue05());
                settings.push_back(std::make_pair("Critical Value .05",buffer));
                printString(buffer, "%lf", _parameters.getCriticalValue01());
                settings.push_back(std::make_pair("Critical Value .01",buffer));
                printString(buffer, "%lf", _parameters.getCriticalValue001());
                settings.push_back(std::make_pair("Critical Value .001",buffer));
                break;
            default: throw prg_error("Unknown critical values type '%d'.\n", "PrintPowerEvaluationsParameters()", _parameters.getCriticalValuesType());
        }
        printString(buffer, "%u", _parameters.getPowerEvaluationReplications());
        settings.push_back(std::make_pair("Number of Replications",buffer));
        settings.push_back(std::make_pair("Alternative Hypothesis File",_parameters.getPowerEvaluationAltHypothesisFilename()));
        if (_parameters.isGeneratingLLRResults()) {
            settings.push_back(std::make_pair("Simulated Log Likelihood Ratios (HA)", LoglikelihoodRatioWriter::getFilename(_parameters, buffer, true)));
        }
    }
    return settings;
}

/** Prints 'Power Simulations' parameters to file stream. */
ParametersPrint::SettingContainer_t & ParametersPrint::getPowerSimulationsParameters(SettingContainer_t & settings) const {
    std::string buffer;
    settings.clear();
    if (_parameters.isReadingSimulationData()) {
        settings.push_back(std::make_pair("Read Simulation Data", "Yes"));
        settings.push_back(std::make_pair("Simulation Input Data File",_parameters.getInputSimulationsFilename()));
    }
    if (_parameters.isWritingSimulationData()) {
        settings.push_back(std::make_pair("Write Simulation Data", "Yes"));
        settings.push_back(std::make_pair("Simulation Output Data File",_parameters.getOutputSimulationsFilename()));
    }
    return settings;
}

/** Prints 'Run Options' parameters to file stream. */
ParametersPrint::SettingContainer_t & ParametersPrint::getRunOptionsParameters(SettingContainer_t & settings) const {
    std::string        buffer;
    settings.clear();
    if (_parameters.getNumRequestedParallelProcesses() == 0)
        settings.push_back(std::make_pair("Processer Usage","All Available Proccessors"));
    else {
        printString(buffer, "At Most %u Proccessors", _parameters.getNumRequestedParallelProcesses());
        settings.push_back(std::make_pair("Processer Usage",buffer));
    }
    if (_parameters.isRandomlyGeneratingSeed())
        settings.push_back(std::make_pair("Use Random Seed",(_parameters.isRandomlyGeneratingSeed() ? "Yes" : "No")));
    if (_parameters.getRandomizationSeed() != RandomNumberGenerator::glDefaultSeed) {
        printString(buffer, "%ld\n", _parameters.getRandomizationSeed());
        settings.push_back(std::make_pair("Randomization Seed",buffer));
    }
    return settings;
}

/** Prints 'System' parameters to file stream. */
ParametersPrint::SettingContainer_t & ParametersPrint::getSystemParameters(SettingContainer_t & settings) const {
    const Parameters::CreationVersion  & IniVersion = _parameters.getCreationVersion();
    Parameters::CreationVersion          Current = {atoi(VERSION_MAJOR), atoi(VERSION_MINOR), atoi(VERSION_RELEASE)};  
    std::string                           buffer;
    settings.clear();
    if (IniVersion.iMajor != Current.iMajor ||
        IniVersion.iMinor != Current.iMinor ||
        IniVersion.iRelease != Current.iRelease) {
        printString(buffer, "%u.%u.%u", IniVersion.iMajor, IniVersion.iMinor, IniVersion.iRelease);
        settings.push_back(std::make_pair("Parameters Version",buffer));
    }
    return settings;
}

/** Prints 'Temporal Window' tab parameters to file stream. */
ParametersPrint::SettingContainer_t & ParametersPrint::getTemporalWindowParameters(SettingContainer_t & settings) const {
    std::string buffer;
    settings.clear();    
    switch (_parameters.getMaximumWindowType()) {
        case Parameters::PERCENTAGE_WINDOW :
            printString(buffer, "%g%% of Data Time Range", _parameters.getMaximumWindowPercentage());
            settings.push_back(std::make_pair("Maximum Temporal Window", buffer)); 
            break;
        case Parameters::FIXED_LENGTH :
            printString(buffer, "%u Time Units", _parameters.getMaximumWindowLength());
            settings.push_back(std::make_pair("Maximum Temporal Window",buffer)); 
            break;
        default: throw prg_error("Unknown maximum window type (%d).", "getTemporalWindowParameters()", _parameters.getMaximumWindowType());
    }
    printString(buffer, "%u Time Units", _parameters.getMinimumWindowLength());
    settings.push_back(std::make_pair("Minimum Temporal Window",buffer));
    return settings;
}

/** Writes settings container to file stream. */
void ParametersPrint::WriteSettingsContainer(const SettingContainer_t& settings, const std::string& section, std::ostream& out) const {
    try {
        if (!settings.size()) return;
        //print section label
        out << std::endl;
        out << section;
        out << std::endl;
        for (size_t t=0; t < section.size(); ++t)
            out << "-";
        out << std::endl;
        SettingContainer_t::const_iterator itr=settings.begin();
        //first calculate maximum label length
        size_t tMaxLabel=0;
        for (; itr != settings.end(); ++itr)
            tMaxLabel = std::max(tMaxLabel, itr->first.size());
        //print settings
        for (itr=settings.begin(); itr != settings.end(); ++itr) {
            out << "  ";
            out << itr->first;
            for (size_t t=itr->first.size(); t < tMaxLabel; ++t)
                out << " ";
            out << " : ";
            out << itr->second;
            out << std::endl;
        }
    } catch (prg_exception& x) {
        x.addTrace("WriteSettingsContainer()","ParametersPrint");
        throw;
    }
}

/** Writes settings container to file stream. */
void ParametersPrint::WriteSettingsContainerHTML(const SettingContainer_t& settings, const std::string& section, std::ostream& out) const {
    try {
        if (!settings.size()) return;
        //print section label
        out << "<div class=\"parameter-section\">" << std::endl;
        out << "<h4>" << section << "</h4>" << std::endl << "<table><tbody>" << std::endl;
        SettingContainer_t::const_iterator itr=settings.begin();
        //print settings
        for (itr=settings.begin(); itr != settings.end(); ++itr)
            out << "<tr><th>" << itr->first << " :</th><td>" << itr->second << "</td></tr>" << std::endl;
        out << std::endl << "</tbody></table></div>";
    } catch (prg_exception& x) {
        x.addTrace("WriteSettingsContainerHTML()","ParametersPrint");
        throw;
    }
}
