//******************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//******************************************************************************
#include "ParametersPrint.h"
#include "PrjException.h"
#include "RandomNumberGenerator.h"
#include "ResultsFileWriter.h"
#include "DataFileWriter.h"
#include "ChartGenerator.h"
#include "ParameterFileAccess.h"

/** Prints parameters, in a particular format, to passed ascii file. */
void ParametersPrint::Print(std::ostream& out) const {
    SettingContainer_t files;
    getAdditionalOutputFiles(files);
    if (files.size()) {
        AsciiPrintFormat::PrintSectionSeparatorString(out, 0, 2);
        out << "ADDITIONAL RESULTS FILES" << std::endl << std::endl;
        WriteSettingsContainer(files, "", out);
    }

    AsciiPrintFormat::PrintSectionSeparatorString(out, 0, 2);
    out << "PARAMETER SETTINGS" << std::endl;

    SettingContainer_t settings;
    // print 'Input' tab settings
    WriteSettingsContainer(getInputParameters(settings), "Input", out);
    // print 'Analysis' tab settings
    WriteSettingsContainer(getAnalysisParameters(settings), "Analysis", out);
    // print 'Output' tab settings
    WriteSettingsContainer(getOutputParameters(settings), "Output", out);
    // print 'Advanced Input' tab settings
    WriteSettingsContainer(getAdvancedInputParameters(settings), "Advanced Input", out);
    // print 'Temporal Window' tab settings
    WriteSettingsContainer(getTemporalWindowParameters(settings), "Temporal Window", out);
    // print 'Adjustments' tab settings
    WriteSettingsContainer(getAdjustmentsParameters(settings), "Adjustments", out);
    // print 'Inference' tab settings
    WriteSettingsContainer(getInferenceParameters(settings), "Inference", out);
    // print 'Sequential Analysis' tab settings
    WriteSettingsContainer(getSequentialScanParameters(settings), "Sequential Analysis", out);
    // print 'Power Evaluations' tab settings
    WriteSettingsContainer(getPowerEvaluationsParameters(settings), "Power Evaluations", out);
    // print 'Miscellaneous' tab settings
    WriteSettingsContainer(getMiscellaneousAnalysisParameters(settings), "Miscellaneous", out);
    // print 'Additional Output' tab settings
    WriteSettingsContainer(getAdditionalOutputParameters(settings), "Additional Output", out);
    // print 'Power Simulations' tab settings
    WriteSettingsContainer(getPowerSimulationsParameters(settings), "Power Simulations", out);
    // print 'RunOptions' settings
    WriteSettingsContainer(getRunOptionsParameters(settings), "Run Options", out);
    // print 'System' parameters
    WriteSettingsContainer(getSystemParameters(settings), "System", out);
    AsciiPrintFormat::PrintSectionSeparatorString(out, 0, 1);
}

/** Prints parameters, in a particular format, to passed ascii file. */
void ParametersPrint::PrintHTML(std::ostream& out) const {
    std::string section_class = "additional-results-section";
    SettingContainer_t files;
    getAdditionalOutputFiles(files);
    if (files.size()) {
        AsciiPrintFormat::PrintSectionSeparatorString(out, 0, 2);
        out << "<div id=\"parameter-settings\"><h4>Additional Results Files</h4>" << std::endl;
        WriteSettingsContainerHTML(files, "", section_class, out);
        out << std::endl << "</div>" << std::endl;
    }

    section_class = "parameter-section";
    SettingContainer_t settings;
    out << "<div id=\"parameter-settings\"><h4>Parameter Settings</h4>" << std::endl;
    // print 'Input' tab settings
    WriteSettingsContainerHTML(getInputParameters(settings), "Input", section_class, out);
    // print 'Analysis' tab settings
    WriteSettingsContainerHTML(getAnalysisParameters(settings), "Analysis", section_class, out);
    // print 'Output' tab settings
    WriteSettingsContainerHTML(getOutputParameters(settings), "Output", section_class, out);
    // print 'Advanced Input' tab settings
    WriteSettingsContainerHTML(getAdvancedInputParameters(settings), "Advanced Input", section_class, out);
    // print 'Temporal Window' tab settings
    WriteSettingsContainerHTML(getTemporalWindowParameters(settings), "Temporal Window", section_class, out);
    // print 'Adjustments' tab settings
    WriteSettingsContainerHTML(getAdjustmentsParameters(settings), "Adjustments", section_class, out);
    // print 'Inference' tab settings
    WriteSettingsContainerHTML(getInferenceParameters(settings), "Inference", section_class, out);
    // print 'Sequential Analysis' tab settings
    WriteSettingsContainerHTML(getSequentialScanParameters(settings), "Sequential Analysis", section_class, out);
    // print 'Power Evaluations' tab settings
    WriteSettingsContainerHTML(getPowerEvaluationsParameters(settings), "Power Evaluations", section_class, out);
    // print 'Miscellaneous' tab settings
    WriteSettingsContainerHTML(getMiscellaneousAnalysisParameters(settings), "Miscellaneous", section_class, out);
    // print 'Additional Output' tab settings
    WriteSettingsContainerHTML(getAdditionalOutputParameters(settings), "Additional Output", section_class, out);
    // print 'Power Simulations' tab settings
    WriteSettingsContainerHTML(getPowerSimulationsParameters(settings), "Power Simulations", section_class, out);
    // print 'RunOptions' settings
    WriteSettingsContainerHTML(getRunOptionsParameters(settings), "Run Options", section_class, out);
    // print 'System' parameters
    WriteSettingsContainerHTML(getSystemParameters(settings), "System", section_class, out);
    out << std::endl << "</div>" << std::endl;
}

/** Get 'Analysis - Miscellaneous Analysis' tab/section. */
ParametersPrint::SettingContainer_t & ParametersPrint::getMiscellaneousAnalysisParameters(SettingContainer_t & settings) const {
    std::string buffer;
    settings.clear();
    if (_parameters.getIsProspectiveAnalysis()) {
        switch (_parameters.getProspectiveFrequencyType()) {
        case Parameters::DAILY:
            if (_parameters.getProspectiveFrequency() > 1) printString(buffer, "Daily (every %u days)", _parameters.getProspectiveFrequency());
            else buffer = "Daily";
            break;
        case Parameters::WEEKLY:
            if (_parameters.getProspectiveFrequency() > 1) printString(buffer, "Weekly (every %u weeks)", _parameters.getProspectiveFrequency());
            else buffer = "Weekly";
            break;
        case Parameters::MONTHLY:
            if (_parameters.getProspectiveFrequency() > 1) printString(buffer, "Monthly (every %u months)", _parameters.getProspectiveFrequency());
            else buffer = "Monthly";
            break;
        case Parameters::QUARTERLY:
            if (_parameters.getProspectiveFrequency() > 1) printString(buffer, "Quarterly (every %u quarters)", _parameters.getProspectiveFrequency());
            else buffer = "Quarterly";
            break;
        case Parameters::YEARLY:
            if (_parameters.getProspectiveFrequency() > 1) printString(buffer, "Yearly (every %u years)", _parameters.getProspectiveFrequency());
            else buffer = "Yearly";
            break;
        default: throw prg_error("Unknown prospective frequency type '%d'.\n", "getMiscellaneousAnalysisParameters()", _parameters.getProspectiveFrequencyType());
        }
        settings.emplace_back("Prospective Analysis Frequency", buffer);
    }
    return settings;
}

/** Prints 'Input' tab parameters to file stream. */
ParametersPrint::SettingContainer_t & ParametersPrint::getInputParameters(SettingContainer_t & settings) const {
    std::string buffer;
    settings.clear();
    if (_parameters.getScanType() != Parameters::TIMEONLY)
        settings.emplace_back("Tree File",_parameters.getTreeFileNames().front());
    settings.emplace_back("Count File",_parameters.getCountFileName());
    if ((_parameters.getModelType() == Parameters::BERNOULLI_TREE || _parameters.getModelType() == Parameters::BERNOULLI_TIME) && !_parameters.getControlFileName().empty())
        settings.emplace_back("Control File", _parameters.getControlFileName());
    if (Parameters::isTemporalScanType(_parameters.getScanType()))
        settings.emplace_back("Data Time Range", _parameters.getDataTimeRangeStr());
    switch (_parameters.getDatePrecisionType()) {
        case DataTimeRange::GENERIC:
            settings.emplace_back("Time Precision", "Generic"); break;
        case DataTimeRange::DAY :
            settings.emplace_back("Time Precision", "Day"); break;
        case DataTimeRange::MONTH:
            settings.emplace_back("Time Precision", "Month"); break;
        case DataTimeRange::YEAR:
            settings.emplace_back("Time Precision", "Year"); break;
        case DataTimeRange::NONE:
            settings.emplace_back("Time Precision", "None"); break;
        default: throw prg_error("Unknown date precision type '%d'.\n", "getInputParameters()", _parameters.getDatePrecisionType());
    }
    return settings;
}

/** Prints 'ADDITIONAL RESULTS FILES' section to file stream. */
ParametersPrint::SettingContainer_t& ParametersPrint::getAdditionalOutputFiles(SettingContainer_t& files) const {
    FileName filename(_parameters.getOutputFileName().c_str());
    std::string buffer;

    try {
        auto addByExtension = [&](const std::string& file_label, const std::string& extension) {
            files.emplace_back(file_label, filename.setExtension(extension.c_str()).getFullPath(buffer));
        };
        auto addByFullpath = [&](const std::string& file_label, const std::string& full_path) {
            files.emplace_back(file_label, full_path);
            filename.setFullPath(_parameters.getOutputFileName().c_str()); // reset
        };
        if (_parameters.isGeneratingHtmlResults())
            addByFullpath("Results HTML", ResultsFileWriter::getHtmlFilename(_parameters, buffer));
        if (_parameters.isGeneratingTableResults())
            addByFullpath("Results CSV Table", CutsRecordWriter::getFilename(_parameters, buffer));
        if (_parameters.getScanType() != Parameters::TIMEONLY) {
            if (_parameters.isGeneratingNCBIAsnResults())
                addByFullpath("NCBI Genome Workbench ASN1 File", ResultsFileWriter::getAsnFilename(_parameters, buffer));
            if (_parameters.isGeneratingNewickFile())
                addByFullpath("Newick Tree Format File", ResultsFileWriter::getNewickFilename(_parameters, buffer));
        }
        if (_parameters.isGeneratingLLRResults())
            addByFullpath("Simulated Log Likelihood Ratios", LoglikelihoodRatioWriter::getFilename(_parameters, buffer, false));
        if (_parameters.getOutputTemporalGraphFile())
            addByFullpath("Temporal Graph File", TemporalChartGenerator::getFilename(filename).getFullPath(buffer));
    } catch (prg_exception& x) {
        x.addTrace("getAdditionalOutputFiles()", "ParametersPrint");
        throw;
    }
    return files;
}

/** Prints 'Additional Output' tab parameters to file stream. */
ParametersPrint::SettingContainer_t & ParametersPrint::getAdditionalOutputParameters(SettingContainer_t & settings) const {
    std::string buffer;
    settings.clear();

    settings.emplace_back("Attributable Risk ",(_parameters.getReportAttributableRisk() ? "Yes" : "No"));
    if (_parameters.getReportAttributableRisk()) {
        printString(buffer, "%u", _parameters.getAttributableRiskExposed());
        settings.emplace_back("Report Attributable Risk Based on # Exposed",buffer);
    }
    settings.emplace_back("Report Simulated Log Likelihood Ratios",(_parameters.isGeneratingLLRResults() ? "Yes" : "No"));
    settings.emplace_back("Report Critical Values",(_parameters.getReportCriticalValues() ? "Yes" : "No"));
    if (_parameters.isGeneratingTableResults())
        settings.emplace_back("Print Column Headers", (_parameters.isPrintColumnHeaders() ? "Yes" : "No"));
    if (_parameters.isTemporalScanType(_parameters.getScanType()))
        settings.emplace_back("Produce Temporal Graphs", (_parameters.getOutputTemporalGraphFile() ? "Yes" : "No"));
    if (_parameters.getOutputTemporalGraphFile()) {
        settings.push_back(std::make_pair("Cluster Graphing", ""));
        switch (_parameters.getTemporalGraphReportType()) {
            case Parameters::MLC_ONLY: settings.back().second = "Most likely cluster only"; break;
            case Parameters::X_MCL_ONLY:
                printString(settings.back().second,
                    "%d most likely clusters, one graph for each", _parameters.getTemporalGraphMostLikelyCount()
                ); break;
            case Parameters::SIGNIFICANT_ONLY:
                printString(settings.back().second,
                    "All clusters, one graph for each, meeting cutoff %g", _parameters.getTemporalGraphSignificantCutoff()
                ); break;
            default: throw prg_error("Unknown temporal graph type %d.\n", "PrintTemporalOutputParameters()", _parameters.getOutputTemporalGraphFile());
        }
    }
    return settings;
}

/** Prints 'Adjustments' tab parameters to file stream. */
ParametersPrint::SettingContainer_t & ParametersPrint::getAdjustmentsParameters(SettingContainer_t & settings) const {
    std::string buffer;
    settings.clear();
    if (Parameters::isTemporalScanType(_parameters.getScanType()) && _parameters.getModelType() != Parameters::BERNOULLI_TIME) {
        switch (_parameters.getConditionalType()) {
            case Parameters::TOTALCASES: // this is the time-only scan
            case Parameters::NODE: buffer = "Perform Day-of-Week Adjustment"; break;
            case Parameters::NODEANDTIME: buffer = "Perform Node by Day-of-Week Adjustment"; break;
            default: throw prg_error("Unknown conditional type (%d).", "getAdjustmentsParameters()", _parameters.getConditionalType());
        }
        settings.emplace_back("Perform Day of Week Adjustment",(_parameters.getPerformDayOfWeekAdjustment() ? "Yes" : "No"));
    }
    if (_parameters.isTemporalScanType(_parameters.getScanType())) {
        settings.emplace_back("Apply Data Time Range Exclusions", (_parameters.isApplyingExclusionTimeRanges() ? "Yes" : "No"));
        if (_parameters.isApplyingExclusionTimeRanges())
            settings.emplace_back("Data Time Range Exclusions", _parameters.getExclusionTimeRangeStr());
    }
    return settings;
}

/** Prints 'Advanced Input' tab parameters to file stream. */
ParametersPrint::SettingContainer_t & ParametersPrint::getAdvancedInputParameters(SettingContainer_t & settings) const {
    std::string buffer;
    settings.clear();
    if (_parameters.getScanType() != Parameters::TIMEONLY)
        settings.emplace_back("Cut File",_parameters.getCutsFileName());
    settings.emplace_back("Only Allow Data on Leaves of Tree", (_parameters.getDataOnlyOnLeaves() ? "Yes" : "No"));
    settings.emplace_back("Relaxed Study Data Period Checking", (_parameters.getRelaxedStudyDataPeriodChecking() ? "Yes" : "No"));
    settings.emplace_back("Allow Multiple Parents for the Same Node", (_parameters.getAllowMultiParentNodes() ? "Yes" : "No"));
    settings.emplace_back("Allow Multiple Root Nodes", (_parameters.getAllowMultipleRoots() ? "Yes" : "No"));
    if (_parameters.getScanType() != Parameters::TIMEONLY && _parameters.getTreeFileNames().size() > 1) {
        for (auto& filename: _parameters.getTreeFileNames())
            settings.emplace_back("Tree File", filename);
    }
    if (_parameters.isApplyingRiskWindowRestrictionCensored())
        settings.emplace_back("Applying Risk Window Restriction Due to Censoring", "Yes");
    return settings;
}

/** Prints 'Analysis' tab parameters to file stream. */
ParametersPrint::SettingContainer_t & ParametersPrint::getAnalysisParameters(SettingContainer_t & settings) const {
    std::string buffer;
    settings.clear();

    buffer = "Type of Scan";
    switch (_parameters.getScanType()) {
        case Parameters::TREEONLY : settings.emplace_back(buffer,"Tree Only"); break;
        case Parameters::TREETIME : settings.emplace_back(buffer,"Tree and Time"); break;
        case Parameters::TIMEONLY : settings.emplace_back(buffer,"Time Only"); break;
        default: throw prg_error("Unknown scan type (%d).", "getAnalysisParameters()", _parameters.getScanType());
    }
    buffer = "Conditional Analysis";
    switch (_parameters.getConditionalType()) {
        case Parameters::UNCONDITIONAL : settings.emplace_back(buffer,"No (unconditional)"); break;
        case Parameters::TOTALCASES : settings.emplace_back(buffer,"Total Cases"); break;
        case Parameters::NODE : settings.emplace_back(buffer,"Node"); break;
        case Parameters::NODEANDTIME : settings.emplace_back(buffer,"Node and Time"); break;
        default: throw prg_error("Unknown conditional type (%d).", "getAnalysisParameters()", _parameters.getConditionalType());
    }
    switch (_parameters.getModelType()) {
        case Parameters::POISSON : settings.emplace_back("Probability Model - Tree","Poisson"); break;
        case Parameters::BERNOULLI_TREE: settings.emplace_back("Probability Model - Tree","Bernoulli"); break;
        case Parameters::UNIFORM :
            if (_parameters.getConditionalType() == Parameters::NODE)
                settings.emplace_back("Probability Model - Time","Uniform");
            break;
        case Parameters::BERNOULLI_TIME: settings.emplace_back("Probability Model - Time", "Beroulli");
        case Parameters::MODEL_NOT_APPLICABLE: break;
        default: throw prg_error("Unknown model type (%d).", "getAnalysisParameters()", _parameters.getModelType());
    }
    if (_parameters.getModelType() == Parameters::BERNOULLI_TREE && _parameters.getConditionalType() == Parameters::UNCONDITIONAL) {
        settings.emplace_back("Self-Control Design",_parameters.getSelfControlDesign() ? "Yes" : "No");
        settings.emplace_back("Variable Case Probability", _parameters.getVariableCaseProbability() ? "Yes" : "No");
        if (!_parameters.getVariableCaseProbability())
            settings.emplace_back("Case Probability", AbtractParameterFileAccess::AsString(buffer, _parameters.getProbabilityRatio()));
    }
    buffer = "Scan Rate";
    switch (_parameters.getScanRateType()) {
        case Parameters::HIGHRATE: settings.emplace_back(buffer, "High Rates"); break;
        case Parameters::LOWRATE: settings.emplace_back(buffer, "Low Rates"); break;
        case Parameters::HIGHORLOWRATE: settings.emplace_back(buffer, "High or Low Rates"); break;
        default: throw prg_error("Unknown scan rate type (%d).", "getAnalysisParameters()", _parameters.getScanRateType());
    }
    return settings;
}

/** Prints 'Sequential Analysis' tab parameters to file stream. */
ParametersPrint::SettingContainer_t & ParametersPrint::getSequentialScanParameters(SettingContainer_t & settings) const {
    std::string buffer;
    settings.clear();

    if ((_parameters.getScanType() == Parameters::TIMEONLY && _parameters.getConditionalType() == Parameters::TOTALCASES) ||
        (_parameters.getModelType() == Parameters::BERNOULLI_TREE && _parameters.getConditionalType() == Parameters::UNCONDITIONAL) ||
		(_parameters.getModelType() == Parameters::POISSON && _parameters.getConditionalType() == Parameters::UNCONDITIONAL)) {
        settings.emplace_back("Perform Sequential Analysis", _parameters.getSequentialScan() ? "Yes" : "No");
        if (_parameters.isSequentialScanPurelyTemporal() && _parameters.getScanType() == Parameters::TIMEONLY) {
            printString(buffer, "%u", _parameters.getSequentialMinimumSignal());
            settings.emplace_back("Sequential Minimum Cases to Signal", buffer);
            printString(buffer, "%u", _parameters.getSequentialMaximumSignal());
            settings.emplace_back("Sequential Maximum Cases to Signal", buffer);
            settings.emplace_back("Sequential File", SequentialScanLoglikelihoodRatioWriter::getFilename(_parameters, buffer));
        }
        if (_parameters.isSequentialScanTreeOnly()) {
            printString(buffer, "%g", _parameters.getSequentialAlphaOverall());
            settings.emplace_back("Alpha Overall", buffer);
            printString(buffer, "%g", _parameters.getSequentialAlphaSpending());
            settings.emplace_back("Alpha Spend Current Look", buffer);
        }
    }
    return settings;
}

/** Prints 'Inference' tab parameters to file stream. */
ParametersPrint::SettingContainer_t & ParametersPrint::getInferenceParameters(SettingContainer_t & settings) const {
    std::string buffer;
    settings.clear();

    if (!_parameters.isSequentialScanTreeOnly()) { // tree sequential scan does not report p-values
        buffer = "P-Value Reporting";
        switch (_parameters.getPValueReportingType()) {
        case Parameters::STANDARD_PVALUE:
            settings.emplace_back(buffer, "Standard Monte Carlo");
            break;
        case Parameters::TERMINATION_PVALUE:
            settings.emplace_back(buffer, "Sequential Monte Carlo Early Termination");
            printString(buffer, "%u", _parameters.getEarlyTermThreshold());
            settings.emplace_back("Termination Cutoff", buffer);
            break;
        }
    }
    settings.emplace_back("Number of Replications", printString(buffer, "%u", _parameters.getNumReplicationsRequested()));

    if (_parameters.getScanType() != Parameters::TIMEONLY) {
        settings.emplace_back("Restrict Tree Levels", _parameters.getRestrictTreeLevels() ? "Yes" : "No");
        if (_parameters.getRestrictTreeLevels()) {
            typelist_csv_string<unsigned int>(_parameters.getRestrictedTreeLevels(), buffer);
            settings.emplace_back("Tree Levels Excluded From Evaluation", buffer);
        }
        settings.emplace_back("Restrict Evaluated Tree Nodes", _parameters.getRestrictEvaluatedTreeNodes() ? "Yes" : "No");
        if (_parameters.getRestrictEvaluatedTreeNodes())
            settings.emplace_back("Not Evaluated Nodes File", _parameters.getNotEvaluatedNodesFileName());
    }

    buffer = "Cut Type";
    switch (_parameters.getCutType()) {
        case Parameters::PAIRS : settings.emplace_back(buffer,"Pairs"); break;
        case Parameters::TRIPLETS : settings.emplace_back(buffer,"Triplets"); break;
        case Parameters::ORDINAL : settings.emplace_back(buffer,"Ordinal"); break;
        case Parameters::COMBINATORIAL : settings.emplace_back(buffer,"Combinatorial"); break;
        case Parameters::SIMPLE : //settings.emplace_back(buffer,"Simple"); break;
        default: break;
    }
    if (_parameters.getScanRateType() != Parameters::LOWRATE)
        settings.emplace_back("Minimum Number of High Rate Node Cases", printString(buffer, "%u", _parameters.getMinimumHighRateNodeCases()));
    return settings;
}

/** Prints 'Output' tab parameters to file stream. */
ParametersPrint::SettingContainer_t & ParametersPrint::getOutputParameters(SettingContainer_t & settings) const {
   std::string buffer;
    settings.clear();
    settings.emplace_back("Results File",_parameters.getOutputFileName());
    settings.emplace_back("Report Results as HTML",(_parameters.isGeneratingHtmlResults() ? "Yes" : "No"));
    settings.emplace_back("Report Results as CSV Table",(_parameters.isGeneratingTableResults() ? "Yes" : "No"));
    if (_parameters.getScanType() != Parameters::TIMEONLY) {
        settings.emplace_back("Generate NCBI Genome Workbench ASN1 File", (_parameters.isGeneratingNCBIAsnResults() ? "Yes" : "No"));
        settings.emplace_back("Generate Newick Tree Format File", (_parameters.isGeneratingNewickFile() ? "Yes" : "No"));
    }
    return settings;
}

/** Prints 'Power Evaluations' parameters to file stream. */
ParametersPrint::SettingContainer_t & ParametersPrint::getPowerEvaluationsParameters(SettingContainer_t & settings) const {
    std::string buffer, buffer2;
    settings.clear();
    if (_parameters.getModelType() == Parameters::POISSON || _parameters.getModelType() == Parameters::BERNOULLI_TREE ||
        (_parameters.getScanType() == Parameters::TIMEONLY && _parameters.getConditionalType() == Parameters::TOTALCASES) ||
        (_parameters.getScanType() == Parameters::TREETIME && _parameters.getConditionalType() == Parameters::NODE)) {
        settings.emplace_back("Perform Power Evaluations", (_parameters.getPerformPowerEvaluations() ? "Yes" : "No"));
        if (!_parameters.getPerformPowerEvaluations()) return settings;
        buffer = "Power Evaluation Type";
        switch (_parameters.getPowerEvaluationType()) {
            case Parameters::PE_WITH_ANALYSIS: 
                settings.emplace_back(buffer,"Standard Analysis and Power Evaluation Together"); break;
            case Parameters::PE_ONLY_CASEFILE:
                printString(buffer2, "Only Power Evaluation%s", (_parameters.getConditionalType() == Parameters::UNCONDITIONAL ? "" : ", Using Total Cases from Count File"));
                settings.emplace_back(buffer,buffer2); break;
            case Parameters::PE_ONLY_SPECIFIED_CASES: 
                settings.emplace_back(buffer,"Only Power Evaluation, Using Defined Total Cases"); 
                printString(buffer, "%i", _parameters.getPowerEvaluationTotalCases());
                settings.emplace_back("Power Evaluation Total Cases",buffer); 
                break;
            default: throw prg_error("Unknown power evaluation type '%d'.\n", "PrintPowerEvaluationsParameters()", _parameters.getPowerEvaluationType());
        }
        buffer = "Critical Values";
        switch (_parameters.getCriticalValuesType()) {
            case Parameters::CV_MONTECARLO: 
                settings.emplace_back(buffer,"Monte Carlo"); break;
            case Parameters::CV_POWER_VALUES: 
                settings.emplace_back(buffer,"User Defined");
                printString(buffer, "%lf", _parameters.getCriticalValue05());
                settings.emplace_back("Critical Value .05",buffer);
                printString(buffer, "%lf", _parameters.getCriticalValue01());
                settings.emplace_back("Critical Value .01",buffer);
                printString(buffer, "%lf", _parameters.getCriticalValue001());
                settings.emplace_back("Critical Value .001",buffer);
                break;
            default: throw prg_error("Unknown critical values type '%d'.\n", "PrintPowerEvaluationsParameters()", _parameters.getCriticalValuesType());
        }
        printString(buffer, "%u", _parameters.getPowerEvaluationReplications());
        settings.emplace_back("Number of Replications",buffer);
        settings.emplace_back("Alternative Hypothesis File",_parameters.getPowerEvaluationAltHypothesisFilename());
        settings.emplace_back("Alternative Hypothesis Results", PowerEstimationRecordWriter::getFilename(_parameters, buffer));
        if (_parameters.isGeneratingLLRResults()) {
            settings.emplace_back("Simulated Log Likelihood Ratios (HA)", LoglikelihoodRatioWriter::getFilename(_parameters, buffer, true));
        }
        if (_parameters.getModelType() == Parameters::BERNOULLI_TREE && _parameters.getConditionalType() == Parameters::TOTALCASES) {
            settings.emplace_back("Baseline Probability", AbtractParameterFileAccess::AsString(buffer, _parameters.getPowerBaselineProbabilityRatio()));
            if (_parameters.getPowerZ() != 0.001) {
                printString(buffer, "%lf", _parameters.getPowerZ());
                settings.emplace_back("Power Z", buffer);
            }
        }
    }
    return settings;
}

/** Prints 'Power Simulations' parameters to file stream. */
ParametersPrint::SettingContainer_t & ParametersPrint::getPowerSimulationsParameters(SettingContainer_t & settings) const {
    settings.clear();
    if (_parameters.isReadingSimulationData()) {
        settings.emplace_back("Read Simulation Data", "Yes");
        settings.emplace_back("Simulation Input Data File",_parameters.getInputSimulationsFilename());
    }
    if (_parameters.isWritingSimulationData()) {
        settings.emplace_back("Write Simulation Data", "Yes");
        settings.emplace_back("Simulation Output Data File",_parameters.getOutputSimulationsFilename());
    }
    return settings;
}

/** Prints 'Run Options' parameters to file stream. */
ParametersPrint::SettingContainer_t & ParametersPrint::getRunOptionsParameters(SettingContainer_t & settings) const {
    std::string buffer;
    settings.clear();
    if (_parameters.getNumRequestedParallelProcesses() == 0)
        settings.emplace_back("Processer Usage","All Available Processors");
    else {
        printString(buffer, "At Most %u Processors", _parameters.getNumRequestedParallelProcesses());
        settings.emplace_back("Processer Usage",buffer);
    }
    if (_parameters.isRandomlyGeneratingSeed())
        settings.emplace_back("Use Random Seed",(_parameters.isRandomlyGeneratingSeed() ? "Yes" : "No"));
    if (_parameters.getRandomizationSeed() != RandomNumberGenerator::glDefaultSeed) {
        printString(buffer, "%ld\n", _parameters.getRandomizationSeed());
        settings.emplace_back("Randomization Seed",buffer);
    }
    return settings;
}

/** Prints 'System' parameters to file stream. */
ParametersPrint::SettingContainer_t & ParametersPrint::getSystemParameters(SettingContainer_t & settings) const {
    const Parameters::CreationVersion & IniVersion = _parameters.getCreationVersion();
    Parameters::CreationVersion Current = {static_cast<unsigned int>(atoi(VERSION_MAJOR)), static_cast<unsigned int>(atoi(VERSION_MINOR)), static_cast<unsigned int>(atoi(VERSION_RELEASE))};
    std::string buffer;
    settings.clear();
    if (IniVersion.iMajor != Current.iMajor ||
        IniVersion.iMinor != Current.iMinor ||
        IniVersion.iRelease != Current.iRelease) {
        printString(buffer, "%u.%u.%u", IniVersion.iMajor, IniVersion.iMinor, IniVersion.iRelease);
        settings.emplace_back("Parameters Version",buffer);
    }
    return settings;
}

/** Prints 'Temporal Window' tab parameters to file stream. */
ParametersPrint::SettingContainer_t & ParametersPrint::getTemporalWindowParameters(SettingContainer_t & settings) const {
    std::string buffer;
    settings.clear();
    if (_parameters.isTemporalScanType(_parameters.getScanType())) {
        switch (_parameters.getMaximumWindowType()) {
            case Parameters::PERCENTAGE_WINDOW :
                printString(buffer, "%g%% of Data Time Range", _parameters.getMaximumWindowPercentage());
                settings.emplace_back("Maximum Temporal Window", buffer); 
                break;
            case Parameters::FIXED_LENGTH :
                printString(buffer, "%u Time Units", _parameters.getMaximumWindowLength());
                settings.emplace_back("Maximum Temporal Window",buffer); 
                break;
            default: throw prg_error("Unknown maximum window type (%d).", "getTemporalWindowParameters()", _parameters.getMaximumWindowType());
        }
        printString(buffer, "%u Time Units", _parameters.getMinimumWindowLength());
        settings.emplace_back("Minimum Temporal Window",buffer);
    }
    if (_parameters.isTemporalScanType(_parameters.getScanType())) {
        settings.emplace_back("Apply Risk Window Restriction", (_parameters.isApplyingRiskWindowRestriction() ? "Yes" : "No"));
        if (_parameters.isApplyingRiskWindowRestriction()) {
            printString(buffer, "Restrict Risk Window to %g%% of Evaluated Windows", _parameters.getRiskWindowPercentage());
            settings.emplace_back("Maximum Temporal Window", buffer);
        }
        settings.emplace_back("Prospective Analysis", (_parameters.getIsProspectiveAnalysis() ? "Yes" : "No"));
        if (!_parameters.getIsProspectiveAnalysis()) {
            settings.emplace_back("Restrict Temporal Windows", (_parameters.getRestrictTemporalWindows() ? "Yes" : "No"));
            if (_parameters.getRestrictTemporalWindows()) {
                settings.emplace_back("Temporal Time Window Start", _parameters.getTemporalStartRangeStr());
                settings.emplace_back("Temporal Time Window End", _parameters.getTemporalEndRangeStr());
            }
        }
    }
    return settings;
}

/** Writes settings container to file stream. */
void ParametersPrint::WriteSettingsContainer(const SettingContainer_t& settings, const std::string& section, std::ostream& out) const {
    try {
        if (!settings.size()) return;
        if (section.size()) { //print section label
            out << std::endl << section << std::endl;
            std::fill_n(std::ostream_iterator<char>(out), section.size(), '-');
            out << std::endl;
        }
        // first calculate maximum label length
        size_t tMaxLabel=0;
        for (auto& setting : settings)
            tMaxLabel = std::max(tMaxLabel, setting.first.size());
        // print settings
        for (auto& setting : settings) {
            out << "  ";
            out << setting.first;
            for (size_t t= setting.first.size(); t < tMaxLabel; ++t)
                out << " ";
            out << " : ";
            out << setting.second;
            out << std::endl;
        }
    } catch (prg_exception& x) {
        x.addTrace("WriteSettingsContainer()","ParametersPrint");
        throw;
    }
}

/** Writes settings container to file stream. */
void ParametersPrint::WriteSettingsContainerHTML(const SettingContainer_t& settings, const std::string& section, const std::string& sectionClass, std::ostream& out) const {
    try {
        if (!settings.size()) return;
        out << "<div class='" << sectionClass << "'>" << std::endl;
        if (section.size()) {// print section label
            out << "<h4>" << section << "</h4>" << std::endl;
        }
        out << "<table><tbody>" << std::endl;
        // print settings
        for (auto& setting: settings) {
            if (setting.first == "Temporal Graph File")
                out << "<tr><th>" << setting.first << " :</th><td><a target=\"_blank\" href=\"file:///" << setting.second << "\">" << setting.second << "</a></td></tr>" << std::endl;
            else
                out << "<tr><th>" << setting.first << " :</th><td>" << setting.second << "</td></tr>" << std::endl;
        }
        out << std::endl << "</tbody></table></div>";
    } catch (prg_exception& x) {
        x.addTrace("WriteSettingsContainerHTML()","ParametersPrint");
        throw;
    }
}
