//******************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//******************************************************************************
#include "ResultsFileWriter.h"
#include "PrjException.h"
#include "ScanRunner.h"
#include "AsciiPrintFormat.h"
#include "ParametersPrint.h"
#include "UtilityFunctions.h"
#include "Toolkit.h"
#include "DataFileWriter.h"
#include <string>

/** Returns filename of html output. */
std::string& ResultsFileWriter::getHtmlFilename(const Parameters& parameters, std::string& buffer) {
  return getDerivedFilename(parameters.getOutputFileName(), "", ".html", buffer);
}

/** Returns filename of NCBI Asn1 output. */
std::string & ResultsFileWriter::getAsnFilename(const Parameters& parameters, std::string& buffer) {
    return getDerivedFilename(parameters.getOutputFileName(), "_ncbi", ".asn", buffer);
}

/** Returns filename of Newick output. */
std::string & ResultsFileWriter::getNewickFilename(const Parameters& parameters, std::string& buffer) {
    return getDerivedFilename(parameters.getOutputFileName(), "", ".nwk", buffer);
}

std::ofstream & ResultsFileWriter::openStream(const std::string& outputfile, std::ofstream & outfile, bool overwrite) const {
    std::string buffer;
    BasePrint & print(const_cast<ScanRunner&>(_scanRunner).getPrint());

    outfile.open(outputfile.c_str());
    if (!outfile) {
        print.Printf("Unable to create specified output file: %s\n", BasePrint::P_READERROR, outputfile.c_str());
        FileName currFilename(outputfile.c_str()), documentsfile(outputfile.c_str());
        std::string temp;
        documentsfile.setLocation(GetUserDocumentsDirectory(buffer, currFilename.getLocation(temp)).c_str());
        documentsfile.getFullPath(buffer);
        print.Printf("Trying to create file in documents directory ...\n", BasePrint::P_STDOUT);
        outfile.open(documentsfile.getFullPath(buffer).c_str());
        if (!outfile) {
            print.Printf("Unable to create output file: %s\n", BasePrint::P_READERROR, buffer.c_str());
            return outfile;
        }
        if (overwrite) const_cast<Parameters&>(_scanRunner.getParameters()).setOutputFileName(buffer.c_str());
        print.Printf("Creating the output file in documents directory: %s\n", BasePrint::P_STDOUT, buffer.c_str());
    } else
        print.Printf("Creating the output file: %s\n", BasePrint::P_STDOUT, outputfile.c_str());
    return outfile;
}

/** Returns whether alpha spending is complete and the sequential analysis is over. */
bool ResultsFileWriter::treeSequentialAnalysisComplete() const {
    return _scanRunner.getParameters().isSequentialScanTreeOnly() && macro_less_than_or_equal(
        _scanRunner.getParameters().getSequentialAlphaOverall(), _scanRunner.getSequentialStatistic().getAlphaSpending(), DBL_CMP_TOLERANCE
    );
}

/** Writes results of analysis to primary text file. */
bool ResultsFileWriter::writeASCII(time_t start, time_t end) {
    std::ofstream outfile;
    openStream(_scanRunner.getParameters().getOutputFileName(), outfile, true);
    if (!outfile) return false;

    AsciiPrintFormat PrintFormat;
    Parameters& parameters(const_cast<Parameters&>(_scanRunner.getParameters()));
    Loglikelihood_t calcLogLikelihood(AbstractLoglikelihood::getNewLoglikelihood(_scanRunner));

    PrintFormat.PrintVersionHeader(outfile);
    std::string buffer = ctime(&start), buffer2;
    outfile << std::endl << "Program run on: " << buffer << std::endl;
    PrintFormat.PrintNonRightMarginedDataString(outfile, getAnalysisSuccinctStatement(buffer, std::string("\n")), false);
    PrintFormat.SetMarginsAsSummarySection();
    PrintFormat.PrintSectionSeparatorString(outfile);
    outfile << std::endl << "SUMMARY STATISTICS" << std::endl << std::endl;
    if (parameters.getScanType() != Parameters::TIMEONLY) {
        outfile << "Tree:" << std::endl;
        const TreeStatistics& treestats = _scanRunner.getTreeStatistics();
        PrintFormat.PrintSectionLabel(outfile, "Number of Nodes", false);
        PrintFormat.PrintAlignedMarginsDataString(outfile, printString(buffer, "%u", treestats._num_nodes));
        PrintFormat.PrintSectionLabel(outfile, "Number of Root Nodes", false);
        PrintFormat.PrintAlignedMarginsDataString(outfile, printString(buffer, "%u", treestats._num_root));
        PrintFormat.PrintSectionLabel(outfile, "Number of Nodes with Children", false);
        PrintFormat.PrintAlignedMarginsDataString(outfile, printString(buffer, "%u", treestats._num_parent));
        PrintFormat.PrintSectionLabel(outfile, "Number of Leaf Nodes", false);
        PrintFormat.PrintAlignedMarginsDataString(outfile, printString(buffer, "%u", treestats._num_leaf));
        PrintFormat.PrintSectionLabel(outfile, "Number of Levels in Tree", false);
        PrintFormat.PrintAlignedMarginsDataString(outfile, printString(buffer, "%u", treestats._nodes_per_level.size()));
        PrintFormat.PrintSectionLabel(outfile, "Nodes per Levels", false);
        std::stringstream stringbuffer;
        for (TreeStatistics::NodesLevel_t::const_iterator itr = treestats._nodes_per_level.begin(); itr != treestats._nodes_per_level.end(); ++itr) {
            stringbuffer << itr->second << (itr->first == treestats._nodes_per_level.size() ? "" : ", ");
        }
        PrintFormat.PrintAlignedMarginsDataString(outfile, stringbuffer.str().c_str());
        if (parameters.getRestrictTreeLevels() || parameters.getRestrictEvaluatedTreeNodes()) {
            PrintFormat.PrintSectionLabel(outfile, "Number of Nodes Evaluated", false);
            PrintFormat.PrintAlignedMarginsDataString(outfile, printString(buffer, "%u", treestats._num_nodes_evaluated));
            if (parameters.getRestrictTreeLevels()) {
                PrintFormat.PrintSectionLabel(outfile, "Tree Levels Included", false);
                PrintFormat.PrintAlignedMarginsDataString(outfile, treestats.toCsvString(treestats._levels_included, buffer));
                PrintFormat.PrintSectionLabel(outfile, "Tree Levels Excluded", false);
                PrintFormat.PrintAlignedMarginsDataString(outfile, treestats.toCsvString(treestats._levels_excluded, buffer));
            }
        }
    }
    if (parameters.isSequentialScanTreeOnly()) {
        const SequentialStatistic & sequentialStatistic = _scanRunner.getSequentialStatistic();
        outfile << std::endl << "Present Look:" << std::endl;
        PrintFormat.PrintSectionLabel(outfile, "Look", false);
        std::stringstream stringbuffer;
        stringbuffer << _scanRunner.getSequentialStatistic().getLook();
        PrintFormat.PrintAlignedMarginsDataString(outfile, stringbuffer.str().c_str());
        PrintFormat.PrintSectionLabel(outfile, "Alpha This Look", false);
        stringbuffer.str("");
        stringbuffer << parameters.getSequentialAlphaSpending();
        PrintFormat.PrintAlignedMarginsDataString(outfile, stringbuffer.str().c_str());
        PrintFormat.PrintSectionLabel(outfile, "Total Cases", false);
        PrintFormat.PrintAlignedMarginsDataString(outfile, printString(buffer, "%ld", _scanRunner.getTotalsFromLook().first));
        if (parameters.getModelType() == Parameters::POISSON) {
            PrintFormat.PrintSectionLabel(outfile, "Total Expected", false);
            PrintFormat.PrintAlignedMarginsDataString(outfile, getValueAsString(_scanRunner.getTotalsFromLook().second, buffer, 1));
        }
        if (parameters.getModelType() == Parameters::BERNOULLI_TREE) {
            PrintFormat.PrintSectionLabel(outfile, "Total Observations", false);
            PrintFormat.PrintAlignedMarginsDataString(outfile, printString(buffer, "%ld", static_cast<int>(_scanRunner.getTotalsFromLook().second)));
        }
        PrintFormat.PrintSectionLabel(outfile, "Critical Value To Reject", false);
        outfile << _scanRunner.getSequentialStatistic().getCriticalValue() << std::endl;
        if (_scanRunner.getSequentialStatistic().getLook() > 1) {
            outfile << std::endl << "Cumulative Look:" << std::endl;
            PrintFormat.PrintSectionLabel(outfile, "Alpha Spent To Date", false);
            stringbuffer.str("");
            stringbuffer << sequentialStatistic.getAlphaSpending();
            PrintFormat.PrintAlignedMarginsDataString(outfile, stringbuffer.str().c_str());
            PrintFormat.PrintSectionLabel(outfile, "Total Cases", false);
            PrintFormat.PrintAlignedMarginsDataString(outfile, printString(buffer, "%ld", _scanRunner.getTotalC()));
            if (parameters.getModelType() == Parameters::POISSON) {
                PrintFormat.PrintSectionLabel(outfile, "Total Expected", false);
                PrintFormat.PrintAlignedMarginsDataString(outfile, getValueAsString(_scanRunner.getTotalN(), buffer, 1));
            }
            if (parameters.getModelType() == Parameters::BERNOULLI_TREE) {
                PrintFormat.PrintSectionLabel(outfile, "Total Observations", false);
                if (parameters.getConditionalType() == Parameters::UNCONDITIONAL && parameters.getVariableCaseProbability())
                    PrintFormat.PrintAlignedMarginsDataString(outfile, printString(buffer, "%ld", _scanRunner.getTotalC() + _scanRunner.getTotalControls()));
                else
                    PrintFormat.PrintAlignedMarginsDataString(outfile, printString(buffer, "%ld", static_cast<int>(_scanRunner.getTotalN())));
            }
            if (parameters.getReportAttributableRisk()) {
                PrintFormat.PrintSectionLabel(outfile, "Total Exposed", false);
                PrintFormat.PrintAlignedMarginsDataString(outfile, printString(buffer, "%u", parameters.getAttributableRiskExposed()));
            }
        }
    } else {
        outfile << std::endl << "Data Summary:" << std::endl;
        if (!parameters.getPerformPowerEvaluations() ||
            !(parameters.getPerformPowerEvaluations() && parameters.getPowerEvaluationType() == Parameters::PE_ONLY_CASEFILE && parameters.getConditionalType() == Parameters::UNCONDITIONAL)) {
            if (parameters.getModelType() == Parameters::SIGNED_RANK) {
                size_t numSites = _scanRunner.getSampleSiteIdentifiers().size();
                PrintFormat.PrintSectionLabel(outfile, "Total Sample Sites", false);
                PrintFormat.PrintAlignedMarginsDataString(outfile, printString(buffer, "%u", numSites));
                PrintFormat.PrintSectionLabel(outfile, "Largest Possible Test Statistic", false);
                PrintFormat.PrintAlignedMarginsDataString(outfile, printString(buffer, "%u", (numSites * (numSites + 1))/2));
                PrintFormat.PrintSectionLabel(outfile, "Total Comparisons", false);
                PrintFormat.PrintAlignedMarginsDataString(outfile, printString(buffer, "%ld", _scanRunner.getTotalC()));
            } else {
                PrintFormat.PrintSectionLabel(outfile, "Total Cases", false);
                PrintFormat.PrintAlignedMarginsDataString(outfile, printString(buffer, "%ld", _scanRunner.getTotalC()));
            }
            if (_scanRunner.isCensoredData()) {
                PrintFormat.PrintSectionLabel(outfile, "Total Censored Cases", false);
                PrintFormat.PrintAlignedMarginsDataString(outfile, printString(buffer, "%ld", _scanRunner.getNumCensoredCases()));
                PrintFormat.PrintSectionLabel(outfile, "Average Censoring Time", false);
                PrintFormat.PrintAlignedMarginsDataString(outfile, printString(buffer, "%ld", _scanRunner.getAvgCensorTime()));
            }
        }
        if (parameters.isApplyingExclusionTimeRanges()) {
            PrintFormat.PrintSectionLabel(outfile, "Total Cases Excluded", false);
            PrintFormat.PrintAlignedMarginsDataString(outfile, printString(buffer, "%ld", _scanRunner.getNumExcludedCases()));
            if (parameters.getModelType() == Parameters::BERNOULLI_TIME) {
                PrintFormat.PrintSectionLabel(outfile, "Total Controls Excluded", false);
                PrintFormat.PrintAlignedMarginsDataString(outfile, printString(buffer, "%ld", _scanRunner.getNumExcludedControls()));
            }
        }
        if (parameters.getModelType() == Parameters::POISSON) {
            PrintFormat.PrintSectionLabel(outfile, "Total Expected", false);
            PrintFormat.PrintAlignedMarginsDataString(outfile, getValueAsString(_scanRunner.getTotalN(), buffer, 1));
        }
        if (parameters.getModelType() == Parameters::BERNOULLI_TREE || parameters.getModelType() == Parameters::BERNOULLI_TIME) {
            PrintFormat.PrintSectionLabel(outfile, "Total Observations", false);
            if (parameters.getVariableCaseProbability()) {
                PrintFormat.PrintAlignedMarginsDataString(outfile,
                    printString(buffer, "%ld", _scanRunner.getTotalC() + _scanRunner.getTotalControls())
                );
            } else {
                PrintFormat.PrintAlignedMarginsDataString(outfile, printString(buffer, "%ld", static_cast<int>(_scanRunner.getTotalN())));
            }
        }
        if (parameters.getReportAttributableRisk()) {
            PrintFormat.PrintSectionLabel(outfile, "Total Exposed", false);
            PrintFormat.PrintAlignedMarginsDataString(outfile, printString(buffer, "%u", parameters.getAttributableRiskExposed()));
        }
        if (Parameters::isTemporalScanType(parameters.getScanType())) {
            PrintFormat.PrintSectionLabel(outfile, "Data Time Range", false);
            PrintFormat.PrintAlignedMarginsDataString(outfile, parameters.getDataTimeRangeSet().toString(buffer, parameters.getDatePrecisionType()));
        }
        if (parameters.isApplyingExclusionTimeRanges()) {
            PrintFormat.PrintSectionLabel(outfile, "Excluded Time Ranges", false);
            PrintFormat.PrintAlignedMarginsDataString(outfile, parameters.getExclusionTimeRangeSet().toString(buffer, parameters.getDatePrecisionType()));
        }
    }
    PrintFormat.PrintSectionSeparatorString(outfile, 0, 2);

    if (Parameters::isTemporalScanType(parameters.getScanType())) {
        _scanRunner.getCaselessWindowsAsString(buffer);
        if (buffer.size()) {
            outfile << "Warning: The following "
                << (parameters.getDatePrecisionType() == DataTimeRange::GENERIC ? "days" : "dates")
                << " in the data time range do not have cases:" << std::endl;
            PrintFormat.PrintSectionStatement(outfile, buffer.c_str());
            PrintFormat.PrintSectionSeparatorString(outfile, 0, 2);
        }
    }

    if (parameters.isSequentialScanPurelyTemporal() && static_cast<unsigned int>(_scanRunner.getTotalC()) > parameters.getSequentialMaximumSignal()) {
        outfile << "Note: The sequential scan reached or exceeded the specified maximum cases." << std::endl << "      The sequential analysis is over." << std::endl;
    } else if (!parameters.getPerformPowerEvaluations() || (parameters.getPerformPowerEvaluations() && parameters.getPowerEvaluationType() == Parameters::PE_WITH_ANALYSIS)) {
        if (treeSequentialAnalysisComplete())
            outfile << "Note: The alpha spending for sequential scan reached the specified alpha overall." << std::endl << "      The sequential analysis is over." << std::endl << std::endl;
        if (_scanRunner.getCuts().size() == 0 || !_scanRunner.reportableCut(*_scanRunner.getCuts()[0])) {
            outfile << "No cuts were found." << std::endl;
        } else {
            outfile << "MOST LIKELY CUTS"<< std::endl << std::endl;
            std::string format, replicas;
            printString(replicas, "%u", parameters.getNumReplicationsRequested());
            printString(format, "%%.%dlf", replicas.size());

            outfile.setf(std::ios::fixed);
            outfile.precision(5);

            ScanRunner::CutStructureContainer_t::const_iterator itrCuts = _scanRunner.getCuts().begin(), itrCutsEnd = _scanRunner.getCuts().end();
            for (; itrCuts != itrCutsEnd; ++itrCuts) {
                CutStructure& thisCut = *(*itrCuts);
                if (!_scanRunner.reportableCut(thisCut))
                    break;
                const NodeStructure& thisNode = *(_scanRunner.getNodes()[thisCut.getID()]);
                PrintFormat.SetMarginsAsCutSection(thisCut.getReportOrder());
                outfile << thisCut.getReportOrder() << ")";
                // skip reporting node identifier for time-only scans
                if (parameters.getScanType() != Parameters::TIMEONLY) {
                    PrintFormat.PrintSectionLabel(outfile, "Node ID", false);
                    buffer = thisNode.getIdentifier();
                    if (thisCut.getCutChildren().size()) {
                        buffer += " children: ";
                        const CutStructure::CutChildContainer_t& childNodeIds = thisCut.getCutChildren();
                        for (size_t t=0; t < childNodeIds.size(); ++t) {
                            buffer +=  _scanRunner.getNodes()[childNodeIds[t]]->getIdentifier();
                            if (t < childNodeIds.size() - 1)
                                buffer += ", ";
                        }
                    }
                    PrintFormat.PrintAlignedMarginsDataString(outfile, buffer.c_str());
                    if (thisNode.getName().size()) {
                        PrintFormat.PrintSectionLabel(outfile, "Node Name", true);
                        PrintFormat.PrintAlignedMarginsDataString(outfile, thisNode.getName().c_str());
                    }
                    if (thisNode.getLevel() > 1) {
                        PrintFormat.PrintSectionLabel(outfile, "Parent Node", true);
                        PrintFormat.PrintAlignedMarginsDataString(outfile, thisCut.getParentIndentifiers(_scanRunner, buffer, true));
                        if (_scanRunner.hasNodeDescriptions()) {
                            if (thisCut.getParentIndentifiers(_scanRunner, buffer2, false) != buffer) {
                                PrintFormat.PrintSectionLabel(outfile, "Parent Node Name", true);
                                PrintFormat.PrintAlignedMarginsDataString(outfile, thisCut.getParentIndentifiers(_scanRunner, buffer, false));
                            }
                        }
                    }
                    PrintFormat.PrintSectionLabel(outfile, "Tree Level", true);
                    printString(buffer, "%ld", thisNode.getLevel());
                    PrintFormat.PrintAlignedMarginsDataString(outfile, buffer);
                }

                if (parameters.getConditionalType() == Parameters::NODEANDTIME) {
                    PrintFormat.PrintSectionLabel(outfile, "Node Cases", true);
                    printString(buffer, "%ld", static_cast<int>(thisNode.getBrC()));
                    PrintFormat.PrintAlignedMarginsDataString(outfile, buffer);
                    PrintFormat.PrintSectionLabel(outfile, "Time Window", true);
                    if (parameters.getDatePrecisionType() == DataTimeRange::GENERIC)
                        printString(buffer, "%ld to %ld", 
                            thisCut.getStartIdx() - _scanRunner.getZeroTranslationAdditive(), thisCut.getEndIdx() - _scanRunner.getZeroTranslationAdditive()
                        );
                    else {
                        std::pair<std::string, std::string> rangeDates = parameters.getDataTimeRangeSet().getDataTimeRangeSets().front().rangeToGregorianStrings(
                            thisCut.getStartIdx() - _scanRunner.getZeroTranslationAdditive(),
                            thisCut.getEndIdx() - _scanRunner.getZeroTranslationAdditive(),
                            parameters.getDatePrecisionType()
                        );
                        printString(buffer, "%s to %s", rangeDates.first.c_str(), rangeDates.second.c_str());
                    }
                    PrintFormat.PrintAlignedMarginsDataString(outfile, buffer);
                    PrintFormat.PrintSectionLabel(outfile, "Cases in Window", true);
                } else if (parameters.getModelType() == Parameters::UNIFORM) {
                    // skip reporting node cases for time-only scans
                    if (parameters.getScanType() != Parameters::TIMEONLY) {
                        PrintFormat.PrintSectionLabel(outfile, "Node Cases", true);
                        if (parameters.isPerformingDayOfWeekAdjustment() || _scanRunner.isCensoredData()) {
                            printString(buffer, "%ld", static_cast<int>(thisNode.getBrC()));
                        } else {
                            printString(buffer, "%ld", static_cast<int>(thisCut.getN()));
                        }
                        PrintFormat.PrintAlignedMarginsDataString(outfile, buffer);
                    }
                    PrintFormat.PrintSectionLabel(outfile, "Time Window", parameters.getScanType() != Parameters::TIMEONLY);
                    if (parameters.getDatePrecisionType() == DataTimeRange::GENERIC)
                        printString(buffer, "%ld to %ld",
                            thisCut.getStartIdx() - _scanRunner.getZeroTranslationAdditive(), thisCut.getEndIdx() - _scanRunner.getZeroTranslationAdditive()
                        );
                    else {
                        std::pair<std::string, std::string> rangeDates = parameters.getDataTimeRangeSet().getDataTimeRangeSets().front().rangeToGregorianStrings(
                            thisCut.getStartIdx() - _scanRunner.getZeroTranslationAdditive(),
                            thisCut.getEndIdx() - _scanRunner.getZeroTranslationAdditive(),
                            parameters.getDatePrecisionType()
                        );
                        printString(buffer, "%s to %s", rangeDates.first.c_str(), rangeDates.second.c_str());
                    }
                    PrintFormat.PrintAlignedMarginsDataString(outfile, buffer);
                    PrintFormat.PrintSectionLabel(outfile, "Cases in Window", true);
                } else if (parameters.getModelType() == Parameters::BERNOULLI_TIME) {
                    // skip reporting node cases/observations for time-only scans
                    if (parameters.getScanType() != Parameters::TIMEONLY) {
                        PrintFormat.PrintSectionLabel(outfile, "Node Observations", true);
                        printString(buffer, "%ld", static_cast<int>(thisNode.getBrN()));
                        PrintFormat.PrintAlignedMarginsDataString(outfile, buffer);
                        PrintFormat.PrintSectionLabel(outfile, "Node Cases", true);
                        printString(buffer, "%ld", static_cast<int>(thisNode.getBrC()));
                        PrintFormat.PrintAlignedMarginsDataString(outfile, buffer);
                    }
                    PrintFormat.PrintSectionLabel(outfile, "Time Window", parameters.getScanType() != Parameters::TIMEONLY);
                    if (parameters.getDatePrecisionType() == DataTimeRange::GENERIC)
                        printString(buffer, "%ld to %ld",
                            thisCut.getStartIdx() - _scanRunner.getZeroTranslationAdditive(), thisCut.getEndIdx() - _scanRunner.getZeroTranslationAdditive()
                        );
                    else {
                        std::pair<std::string, std::string> rangeDates = parameters.getDataTimeRangeSet().getDataTimeRangeSets().front().rangeToGregorianStrings(
                            thisCut.getStartIdx() - _scanRunner.getZeroTranslationAdditive(),
                            thisCut.getEndIdx() - _scanRunner.getZeroTranslationAdditive(),
                            parameters.getDatePrecisionType()
                        );
                        printString(buffer, "%s to %s", rangeDates.first.c_str(), rangeDates.second.c_str());
                    }
                    PrintFormat.PrintAlignedMarginsDataString(outfile, buffer);
                    PrintFormat.PrintSectionLabel(outfile, "Observations in Window", true);
                    printString(buffer, "%ld", static_cast<int>(thisCut.getN()));
                    PrintFormat.PrintAlignedMarginsDataString(outfile, buffer);
                    PrintFormat.PrintSectionLabel(outfile, "Cases in Window", true);
                } else if (parameters.getModelType() == Parameters::BERNOULLI_TREE) {
                    PrintFormat.PrintSectionLabel(outfile, "Observations", true);
                    if (parameters.getConditionalType() == Parameters::UNCONDITIONAL && parameters.getVariableCaseProbability())
                        printString(buffer, "%ld", thisCut.getMatchedSets().get().size());
                    else
                        printString(buffer, "%ld", static_cast<int>(thisCut.getN()));
                    PrintFormat.PrintAlignedMarginsDataString(outfile, buffer);
                    PrintFormat.PrintSectionLabel(outfile, "Cases", true);
                } else if (parameters.getModelType() == Parameters::POISSON) {
                    PrintFormat.PrintSectionLabel(outfile, "Observed Cases", true);
                } else if (parameters.getModelType() == Parameters::SIGNED_RANK) {
                    auto proportions = getAverage(thisCut.getSampleSiteData());
                    PrintFormat.PrintSectionLabel(outfile, "Baseline Average", true);
                    PrintFormat.PrintAlignedMarginsDataString(outfile, printString(buffer, "%g", proportions.first));
                    PrintFormat.PrintSectionLabel(outfile, "Current Average", true);
                    PrintFormat.PrintAlignedMarginsDataString(outfile, printString(buffer, "%g", proportions.second));
                    PrintFormat.PrintSectionLabel(outfile, "Percent Change", true);
                    if (proportions.first)
                        PrintFormat.PrintAlignedMarginsDataString(outfile, printString(buffer, "%g", proportions.second/proportions.first - 1.0));
                    else
                        PrintFormat.PrintAlignedMarginsDataString(outfile, "infinity");
                    PrintFormat.PrintSectionLabel(outfile, "Absolute Change", true);
                    PrintFormat.PrintAlignedMarginsDataString(outfile, printString(buffer, "%g", std::abs(proportions.second - proportions.first)));
                }
                if (parameters.getModelType() != Parameters::SIGNED_RANK) {
                    PrintFormat.PrintAlignedMarginsDataString(outfile, printString(buffer, "%ld", thisCut.getC()));
                    PrintFormat.PrintSectionLabel(outfile, parameters.getModelType() == Parameters::UNIFORM ? "Expected Cases" : "Expected", true);
                    PrintFormat.PrintAlignedMarginsDataString(outfile, getValueAsString(thisCut.getExpected(_scanRunner), buffer));
                    PrintFormat.PrintSectionLabel(outfile, "Relative Risk", true);
                }
                if (parameters.getIsSelfControlVariableBerounlli())
                    PrintFormat.PrintAlignedMarginsDataString(outfile, getValueAsString(thisCut.getRelativeRisk(_scanRunner), buffer));
                else if(parameters.getModelType() != Parameters::SIGNED_RANK)
                    PrintFormat.PrintAlignedMarginsDataString(outfile, getValueAsString(thisCut.getRelativeRisk(_scanRunner), buffer));
                if (parameters.getModelType() != Parameters::SIGNED_RANK) {
                    PrintFormat.PrintSectionLabel(outfile, "Excess Cases", true);
                    PrintFormat.PrintAlignedMarginsDataString(outfile, getValueAsString(thisCut.getExcessCases(_scanRunner), buffer));
                }
                if (parameters.getReportAttributableRisk()) {
                    PrintFormat.PrintSectionLabel(outfile, "Attributable Risk", true);
                    PrintFormat.PrintAlignedMarginsDataString(outfile, AttributableRiskAsString(thisCut.getAttributableRisk(_scanRunner), buffer));
                }
                if (parameters.getIsTestStatistic()) {
                    // If we stick with Poisson log-likelihood calculation, then label is 'Test Statistic' in place of 'Log Likelihood Ratio', hyper-geometric is 'Log Likelihood Ratio'.
                    PrintFormat.PrintSectionLabel(outfile, "Test Statistic", true);
                } else {
                    PrintFormat.PrintSectionLabel(outfile, "Log Likelihood Ratio", true);
                }
                double llr = calcLogLikelihood->LogLikelihoodRatio(thisCut.getLogLikelihood());
                if (parameters.isSequentialScanTreeOnly() && !macro_less_than(MIN_CUT_LLR, llr, DBL_CMP_TOLERANCE))
                    buffer = "Not Applicable"; // It's possible that this cut signalled in prior look but now its llr is not significant.
                else if (parameters.getModelType() == Parameters::SIGNED_RANK)
                    printString(buffer, "%.0lf", llr);
                else
                    printString(buffer, "%.6lf", llr);
                PrintFormat.PrintAlignedMarginsDataString(outfile, buffer);
                if (_scanRunner.reportablePValue(thisCut)) {
                    PrintFormat.PrintSectionLabel(outfile, "P-value", true);
                    printString(buffer, format.c_str(), thisCut.getPValue(_scanRunner));
                    PrintFormat.PrintAlignedMarginsDataString(outfile, buffer);
                    if (parameters.getIsProspectiveAnalysis()) {
                        PrintFormat.PrintSectionLabel(outfile, "Recurrence Interval", true);
                        PrintFormat.PrintAlignedMarginsDataString(outfile, getRecurranceIntervalAsString(_scanRunner.getRecurrenceInterval(thisCut), buffer));
                    }
                }
                if (parameters.isSequentialScanTreeOnly()) {
                    PrintFormat.PrintSectionLabel(outfile, "Signalled", true);
                    unsigned int signalInLook = _scanRunner.getSequentialStatistic().testCutSignaled(static_cast<size_t>(thisCut.getID()));
                    if (signalInLook != 0)
                        PrintFormat.PrintAlignedMarginsDataString(outfile, printString(buffer, "Look %ld", signalInLook));
                    else
                        PrintFormat.PrintAlignedMarginsDataString(outfile, printString(buffer, "No"));
                }
                outfile << std::endl;
            }
        }
    }

    // Print critical values if requested.
    if ((parameters.getReportCriticalValues() && parameters.getNumReplicationsRequested() >= 19) || 
        (parameters.getPerformPowerEvaluations() && parameters.getCriticalValuesType() == Parameters::CV_MONTECARLO)) {
        PrintFormat.SetMarginsAsOverviewSection();
        PrintFormat.PrintSectionStatement(outfile, 
            "A cut is statistically significant when its log likelihood ratio is greater than the critical value, which is, for significance level:"
        );
        if (parameters.getNumReplicationsRequested() >= 99999) {
            CriticalValues::alpha_t alpha00001(_scanRunner.getCriticalValues().getAlpha00001());
            outfile << "... 0.00001: ";
            if (parameters.getModelType() == Parameters::SIGNED_RANK)
                outfile <<  printString(buffer, "%.0lf", alpha00001.second) << std::endl;
            else
                outfile <<  alpha00001.second << std::endl;
        }
        if (parameters.getNumReplicationsRequested() >= 9999) {
            CriticalValues::alpha_t alpha0001(_scanRunner.getCriticalValues().getAlpha0001());
            outfile << "... 0.0001: ";
            if (parameters.getModelType() == Parameters::SIGNED_RANK)
                outfile << printString(buffer, "%.0lf", alpha0001.second) << std::endl;
            else
                outfile << alpha0001.second << std::endl;
        }
        if (parameters.getNumReplicationsRequested() >= 999) {
            CriticalValues::alpha_t alpha001(_scanRunner.getCriticalValues().getAlpha001());
            outfile << "... 0.001: ";
            if (parameters.getModelType() == Parameters::SIGNED_RANK)
                outfile << printString(buffer, "%.0lf", alpha001.second) << std::endl;
            else
                outfile << alpha001.second << std::endl;
        }
        if (parameters.getNumReplicationsRequested() >= 99) {
            CriticalValues::alpha_t alpha01(_scanRunner.getCriticalValues().getAlpha01());
            outfile << "... 0.01: ";
            if (parameters.getModelType() == Parameters::SIGNED_RANK)
                outfile << printString(buffer, "%.0lf", alpha01.second) << std::endl;
            else
                outfile << alpha01.second << std::endl;
        }
        if (parameters.getNumReplicationsRequested() >= 19) {
            CriticalValues::alpha_t alpha05(_scanRunner.getCriticalValues().getAlpha05());
            outfile << "... 0.05: ";
            if (parameters.getModelType() == Parameters::SIGNED_RANK)
                outfile << printString(buffer, "%.0lf", alpha05.second) << std::endl;
            else
                outfile << alpha05.second << std::endl;
        }
        outfile << std::endl;
    }
    if (parameters.getTerminateSimulationsEarly() && _scanRunner.getCuts().size() && _scanRunner.getSimulationVariables().get_sim_count() < parameters.getNumReplicationsRequested())
        outfile << "NOTE: The sequential Monte Carlo procedure was used to terminate the calculations after " << _scanRunner.getSimulationVariables().get_sim_count() << " replications." << std::endl;
    // print power estimation values
    if (parameters.getPerformPowerEvaluations()) {
        // sanity check
        if (_scanRunner.getPowerEstimations().size() == 0)
            outfile << "No estimates power values to report." << std::endl; //throw prg_error("Number of power estimations is zero.", "ResultsFileWriter::writeASCII(...)");
        else {
            outfile << std::endl << "Estimated Power" << std::endl;
            writePadRight("Alpha", outfile, 25, ' ');
            writePadRight("0.05", outfile, 20, ' ');
            writePadRight("0.01", outfile, 20, ' ');
            outfile << "0.001" << std::endl;
            writePadRight("-----", outfile, 25, ' ');
            writePadRight("----", outfile, 20, ' ');
            writePadRight("----", outfile, 20, ' ');
            outfile << "-----" << std::endl;
            std::string buffer;
            ScanRunner::PowerEstimationContainer_t::const_iterator itr=_scanRunner.getPowerEstimations().begin(), itr_end=_scanRunner.getPowerEstimations().end();
            for (;itr != itr_end; ++itr) {
                writePadRight(printString(buffer, "Alternative #%u", std::distance(_scanRunner.getPowerEstimations().begin(), itr) + 1), outfile, 25, ' ');
                writePadRight(getRoundAsString(itr->get<0>(),buffer, 3), outfile, 20, ' ');
                writePadRight(getRoundAsString(itr->get<1>(),buffer, 3), outfile, 20, ' ');
                outfile << getRoundAsString(itr->get<2>(),buffer, 3) << std::endl;
            }
        }
    }

    ParametersPrint(parameters).Print(outfile);
    outfile << std::endl << "COMPUTATIONAL INFORMATION" << std::endl << std::endl;
    outfile << "Program run on     : " << ctime(&start);
    outfile << "Program completed  : " << ctime(&end);
    outfile << "Total Running Time : " << getTotalRunningTime(start, end, buffer) << std::endl;
    if (parameters.getNumParallelProcessesToExecute() > 1) outfile << "Processor Usage    : " << parameters.getNumParallelProcessesToExecute() << " processors" << std::endl;

    outfile.close();
    return true;
}

/** Returns a string which details primary analysis settings. */
std::string & ResultsFileWriter::getAnalysisSuccinctStatement(std::string & statement, const std::string& newline) const {
    const Parameters& parameters = _scanRunner.getParameters();
	std::stringstream scanrate, buffer;
	scanrate << newline << "looking for ";
    switch (parameters.getScanRateType()) {
        case Parameters::LOWRATE: 
            if (parameters.getModelType() == Parameters::SIGNED_RANK)
                scanrate << "descreasing branches" << newline;
            else
                scanrate << "low rate branches" << newline;
            break;
        case Parameters::HIGHORLOWRATE: 
            if (parameters.getModelType() == Parameters::SIGNED_RANK)
                scanrate << "increasing or descreasing branches" << newline;
            else
                scanrate << "high or low rate branches" << newline;
            break;
        case Parameters::HIGHRATE: 
            if (parameters.getModelType() == Parameters::SIGNED_RANK)
                scanrate << "increasing branches" << newline;
            else
                scanrate << "high rate branches" << newline;
            break;
        default: throw prg_error("Unknown scan rate type'%d'.", "getAnalysisSuccinctStatement()", parameters.getScanRateType());
    }
    switch (parameters.getScanType()) {
        case Parameters::TREEONLY : {
            buffer << (parameters.isSequentialScanTreeOnly() ? "Tree Only Sequential scan" : "Tree Only scan") << scanrate.str();
            switch (parameters.getConditionalType()) {
                case Parameters::UNCONDITIONAL : buffer << "with unconditional"; break;
                case Parameters::TOTALCASES : buffer << "with conditional"; break;
                case Parameters::NODE : break;
                default: throw prg_error("Unknown conditional type (%d).", "getAnalysisSuccinctStatement()", parameters.getConditionalType());
            }
            switch (parameters.getModelType()) {
                case Parameters::POISSON : buffer << " Poisson model"; break;
                case Parameters::BERNOULLI_TREE: buffer << " Bernoulli model";
                    if (_scanRunner.getParameters().getSelfControlDesign())
                        buffer << " (self-control design)";
                    break;
                case Parameters::SIGNED_RANK: buffer << " Trend model"; break;
                case Parameters::UNIFORM : break;
                default: throw prg_error("Unknown nodel type (%d).", "getAnalysisSuccinctStatement()", parameters.getModelType());
            }
        } break;
        case Parameters::TREETIME :
            buffer << (parameters.getIsProspectiveAnalysis() ? "Prospective " : "") << "Tree Temporal scan" << scanrate.str();
            switch (parameters.getConditionalType()) {
                case Parameters::NODE : 
                    if (parameters.getModelType() == Parameters::BERNOULLI_TIME)
                        buffer << "with conditional Bernoulli model";
                    else
                        buffer << "with conditional Uniform model"; 
                    break;
                case Parameters::NODEANDTIME : buffer << "conditioned on node and time"; break;
                default: throw prg_error("Unknown conditional type (%d).", "getAnalysisSuccinctStatement()", parameters.getConditionalType());
            } break;
        case Parameters::TIMEONLY :
            buffer << (parameters.isSequentialScanPurelyTemporal() ? "Time Only Sequential scan" : "Time Only scan") << scanrate.str();
            switch (parameters.getConditionalType()) {
                case Parameters::TOTALCASES : 
                    if (parameters.getModelType() == Parameters::BERNOULLI_TIME)
                        buffer << "with conditional Bernoulli model";
                    else
                        buffer << "with conditional Uniform model"; 
                    break;
                default: throw prg_error("Unknown conditional type (%d).", "getAnalysisSuccinctStatement()", parameters.getConditionalType());
            } break;
            if (parameters.isSequentialScanPurelyTemporal()) {
                std::string temp;
                buffer << printString(temp, ", %d out of %u cases observed.", _scanRunner.getTotalC(), parameters.getSequentialMaximumSignal());
            }
        default: throw prg_error("Unknown scan type (%d).", "getAnalysisSuccinctStatement()", parameters.getScanType());
    }
    if (parameters.getPerformDayOfWeekAdjustment())
        buffer << newline << "adjusting for weekly trends nonparametrically";
	statement = buffer.str();
    return statement;
}

/** Writes tree structure and results of analysis to NCBI ASN1 file. */
bool ResultsFileWriter::writeNCBIAsn() const {
    // Create output filename.
    std::string buffer;
    std::ofstream outfile;
    openStream(getAsnFilename(_scanRunner.getParameters(), buffer), outfile, true);
    if (!outfile) return false;
    // Write opening block and fdict structure.
    ptr_vector<FieldDef> fieldDefinitions;
    CutsRecordWriter::getFieldDefs(fieldDefinitions, _scanRunner.getParameters(), _scanRunner.hasNodeDescriptions());
    outfile << "BioTreeContainer ::= {" << std::endl << "fdict {" << std::endl;
    outfile << "{ id 0, name \"label\" }," << std::endl << "{ id 1, name \"dist\" }," << std::endl << "{ id 2, name \"" << DataRecordWriter::NODE_ID_FIELD << "\" }";
    if (_scanRunner.hasNodeDescriptions()) outfile << "," << std::endl << "{ id 3, name \"" << DataRecordWriter::NODE_NAME_FIELD << "\" }";
    size_t fid = (_scanRunner.hasNodeDescriptions() ? 4 : 3);
    std::string cutField(DataRecordWriter::CUT_NUM_FIELD), idField(DataRecordWriter::NODE_ID_FIELD), nameField(DataRecordWriter::NODE_NAME_FIELD);
    for(auto const field: fieldDefinitions) {
        if (cutField == field->GetName() || idField == field->GetName() || nameField == field->GetName()) continue;
        outfile << "," << std::endl << "{ id " << fid << ", name \"" << field->GetName() <<  "\" }";
        ++fid;
    }
    outfile << std::endl << "}," << std::endl;
    // Write the nodes of tree and possibly the cut record as well.
    outfile << "nodes {" << std::endl;
    std::stringstream destination("");
    // create a map of NodeID to Cut objects
    std::map<int, const CutStructure*> nodeCuts;
    for (const auto& cut : _scanRunner.getCuts()) nodeCuts[cut->getID()] = cut;
    size_t nroots(_scanRunner.getRootNodes().size());
    if (nroots > 1) outfile << "{ id 0 }," << std::endl;
    for (auto const& root: _scanRunner.getRootNodes())
        getNCBIAsnDefinition(*root, fieldDefinitions, nroots > 1, nodeCuts, destination);
    // Remove trailing comma ...
    destination.seekp(-2, std::ios_base::end);
    destination << ' ';
    outfile << destination.rdbuf();
    outfile <<  "}," << std::endl;
    // Write user options.
    outfile << "user { type str \"Tree Metadata\", data { { label str \"layout\", data int 0 }, { label str \"use-distances\", data bool TRUE }, { label str \"rotate-labels\", data bool FALSE } } }" << std::endl;
    outfile << "}" << std::endl; // end block
    outfile.close();
    return true;
}

/** Collects asn node definition for this node and children. */
std::stringstream & ResultsFileWriter::getNCBIAsnDefinition(const NodeStructure& node, const ptr_vector<FieldDef>& fieldDefinitions, bool idoffset, const std::map<int, const CutStructure*>& nodeCuts, std::stringstream& destination) const {
    std::string buffer, buffer2;
    // First write the current node, including additional information if there is a cut for the node.
    auto const& nodeCut = nodeCuts.find(node.getID());
    destination << "{ id " << (node.getID() + (idoffset ? 1 : 0)) << ", ";
    std::string distance = "1";
    if (node.getLevel() == 1 && idoffset) {
        destination << "parent 0,";
        distance = "0";
    } else if (node.getLevel() > 1) {
        destination << "parent " << (node.getParents().front().first->getID() + (idoffset ? 1 : 0)) << ",";
        distance = node.getParents().front().second;
    }
    if (nodeCut != nodeCuts.end()) printString(buffer, " (Cut #%u)", nodeCut->second->getReportOrder());
    buffer = node.getIdentifier();
    if (node.getName().size()) {
        buffer2 = node.getName();
        if (buffer2.size() > 25) {
            buffer2.resize(25);
            buffer2.resize(27, '.');
        }
        buffer += " ("; buffer += buffer2; buffer += ")";
    }
    buffer2 = node.getIdentifier();
    destination << " features { { featureid 0, value \"" << encodeForASN(buffer) << "\" },{ featureid 1, value \"" << distance << "\" },{ featureid 2, value \""<< buffer2 << "\" }";
    if (_scanRunner.hasNodeDescriptions()) {
        buffer = node.getName();
        destination << ",{ featureid 3, value \"" << encodeForASN(buffer) << "\" }";
    }
    if (nodeCut != nodeCuts.end()) {
        RecordBuffer Record(fieldDefinitions);
        CutsRecordWriter::getRecordForCut(Record, *(nodeCut->second), _scanRunner);
        size_t fid = (_scanRunner.hasNodeDescriptions() ? 4 : 3);
        std::string cutField(DataRecordWriter::CUT_NUM_FIELD), idField(DataRecordWriter::NODE_ID_FIELD), 
            nameField(DataRecordWriter::NODE_NAME_FIELD), parentField(DataRecordWriter::PARENT_NODE_FLD);
        for (auto const field : fieldDefinitions) {
            if (cutField == field->GetName() || idField == field->GetName() || nameField == field->GetName()) continue;
            if (field->GetType() == FieldValue::ALPHA_FLD) // Special case character fields.
                buffer = encodeForASN(Record.GetFieldValue(field->GetName()).AsString());
            else // Otherwise format in same way as CSV output.
                CSVDataFileWriter::encodeForCSV(buffer, *field, Record.GetFieldValue(field->GetName()));
            destination << ", { featureid " << fid << ", value \"" << buffer << "\"}";
            ++fid;
        }
    }
    destination << " } }," << std::endl;
    // Recursively write child nodes.
    for (const auto child : node.getChildren())
        getNCBIAsnDefinition(*child, fieldDefinitions, idoffset, nodeCuts, destination);
    return destination;
}

/** Writes tree structure to Newick formatted file. */
bool ResultsFileWriter::writeNewick() const {
    Parameters& parameters(const_cast<Parameters&>(_scanRunner.getParameters()));
    std::string buffer;
    std::ofstream outfile;
    openStream(getNewickFilename(parameters, buffer), outfile, true);
    if (!outfile) return false;
    std::stringstream destination("");
    size_t nroots(_scanRunner.getRootNodes().size());
    if (nroots > 1) outfile << "(";
    for (size_t r=0; r < nroots; ++r) {
        outfile << getNewickDefinition(*(_scanRunner.getRootNodes()[r]), destination).rdbuf();
        if (r + 1 < nroots) outfile << ",";
    }
    if (nroots > 1) outfile << ")";
    outfile << ";";
    outfile.close();
    return true;
}

/** Collects Newick node definition for this node and children. */
std::stringstream & ResultsFileWriter::getNewickDefinition(const NodeStructure& node, std::stringstream& destination) const {
    size_t nchild(node.getChildren().size());
    if (nchild) destination << "(";
    for (size_t c=0; c < nchild; ++c) {
        getNewickDefinition(*(node.getChildren()[c]), destination);
        if (c + 1 < nchild) destination << ",";
    }
    if (nchild) destination << ")";
    std::string buffer(node.getIdentifier());
    destination << encodeForNewick(buffer) << ":" << (node.getParents().size() ? node.getParents().front().second : "1");
    return destination;
}

std::string & ResultsFileWriter::getTotalRunningTime(time_t start, time_t end, std::string & buffer) {
    double nTotalTime = difftime(end, start);
    double nHours     = floor(nTotalTime/(60*60));
    double nMinutes   = floor((nTotalTime - nHours*60*60)/60);
    double nSeconds   = nTotalTime - (nHours*60*60) - (nMinutes*60);
    const char * szHours = (0 < nHours && nHours < 1.5 ? "hour" : "hours");
    const char * szMinutes = (0 < nMinutes && nMinutes < 1.5 ? "minute" : "minutes");
    const char * szSeconds = (0.5 <= nSeconds && nSeconds < 1.5 ? "second" : "seconds");
    if (nHours > 0) printString(buffer, "%.0f %s %.0f %s %.0f %s", nHours, szHours, nMinutes, szMinutes, nSeconds, szSeconds);
    else if (nMinutes > 0) printString(buffer, "%.0f %s %.0f %s", nMinutes, szMinutes, nSeconds, szSeconds);
    else printString(buffer, "%.0f %s",nSeconds, szSeconds);
    return buffer;
}

/** Encodes text for javascript id reference by replacing symbols/punctuation with underscore. */
std::string & ResultsFileWriter::encodeForJavascript(std::string & text) const {
    // Now replace HTML number characters that would interfere in javascript.
    std::transform(text.begin(), text.end(), text.begin(), [](char& ch) {
        if (std::ispunct(ch) || ch == ' ')
            ch = '_';
        return ch;
    });
    return text;
}

/** Encodes string for Newick format (https://en.wikipedia.org/wiki/Newick_format). */
std::string & ResultsFileWriter::encodeForNewick(std::string & text) const {
    std::stringstream buffer;
    buffer << '\''; // Always enclosing in single quotes - just in case there are characters which conflict with reserved characters.
    for (auto ch : text) {
        switch (ch) {
            case '\'': buffer << "\'\'"; break; // escape single quotes since they are the enclosing character for Newick
            default: buffer << ch; break;
        }
    }
    buffer << '\'';
    text = buffer.str();
    return text;
}

/** Encodes string characters for ASN.1 format used in Genome Workbench and NCBI's Tree Viewer. */
std::string & ResultsFileWriter::encodeForASN(std::string & text) const {
    std::stringstream buffer;
    for (auto ch : text) {
        switch (ch) {
            case '\"': buffer << "\"\""; break; // escape double quotes since they are the enclosing character for ASN values
            default: buffer << ch; break;
        }
    }
    text = buffer.str();
    return text;
}

bool ResultsFileWriter::writeHTML(time_t start, time_t end) {
    std::string buffer;
    Parameters& parameters(const_cast<Parameters&>(_scanRunner.getParameters()));
    std::ofstream outfile;
    openStream(getHtmlFilename(parameters, buffer), outfile);
    if (!outfile) return false;
    Loglikelihood_t calcLogLikelihood(AbstractLoglikelihood::getNewLoglikelihood(_scanRunner));

    outfile << "<!DOCTYPE html>" << std::endl; 
    outfile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?> " << std::endl;
    outfile << "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\">" << std::endl;
    outfile << "<head>" << std::endl;
    outfile << "<link rel=\"stylesheet\" href=\"https://www.treescan.org/libs/bootstrap.4.1.1/bootstrap.4.1.1.css\">" << std::endl;
    outfile << "<link rel=\"stylesheet\" href=\"https://www.treescan.org/libs/datatables.1.10.16/css/jquery.dataTables.min.css\">" << std::endl;
    outfile << "<link rel=\"stylesheet\" href=\"https://www.treescan.org/html-results/treescan-results.1.2.css\">" << std::endl;
    outfile << "<link rel=\"stylesheet\" href=\"http://maxcdn.bootstrapcdn.com/font-awesome/4.7.0/css/font-awesome.min.css\">" << std::endl;
    outfile << "</head>" << std::endl;
    if (parameters.getScanType() != Parameters::TIMEONLY) {
        outfile << "<script src=\"https://www.treescan.org/libs/raphael.2.1.4.js\" type=\"text/javascript\"></script>" << std::endl;
        outfile << "<script src=\"https://www.treescan.org/libs/Treant-20180418.js\" type=\"text/javascript\"></script>" << std::endl;
    }
    outfile << "<script src=\"https://www.treescan.org/libs/jquery.3.3.1/jquery-3.3.1.js\" type=\"text/javascript\"></script>" << std::endl;
    outfile << "<script src=\"https://www.treescan.org/libs/datatables.1.10.16/js/jquery.dataTables.min.js\" type=\"text/javascript\"></script>" << std::endl;
    outfile << "<script src=\"https://www.treescan.org/libs/bootstrap.4.1.1/popper.4.1.1.js\" type=\"text/javascript\"></script>" << std::endl;
    outfile << "<script src=\"https://www.treescan.org/libs/bootstrap.4.1.1/bootstrap.4.1.1.js\" type=\"text/javascript\"></script>" << std::endl;

    /** Determine if we should show the tree visualization. 
       - Since we're pruning the tree by minimum p-value of 0.05, we can't possibly have any significant nodes if the number of replications
         is less than 19 -- which implies the necessary rank out of 20 is 1 to meet at most 0.05. 
       - If this is a sequential scan and we haven't reached the maximum cases, we will report cuts.
       - If this is a power evaluation with an analysis, we will report cuts.
       - There needs to be at least one reportable cut.
       */
    bool showingTreeGraph = parameters.getNumReplicationsRequested() >= 19 && parameters.getScanType() != Parameters::TIMEONLY;
    if (parameters.isSequentialScanPurelyTemporal())
        showingTreeGraph &= !(static_cast<unsigned int>(_scanRunner.getTotalC()) > parameters.getSequentialMaximumSignal());
    if (parameters.getPerformPowerEvaluations())
        showingTreeGraph &= parameters.getPowerEvaluationType() == Parameters::PE_WITH_ANALYSIS;
    showingTreeGraph &= (_scanRunner.getCuts().size() > 0 && _scanRunner.reportableCut(*_scanRunner.getCuts()[0]));

    bool sequentialTreeOnly = _scanRunner.getParameters().isSequentialScanTreeOnly();
    if (showingTreeGraph) {
        outfile << "<script type=\"text/javascript\" charset=\"utf-8\">" << std::endl;
        outfile << "var chart_config = { chart: { container: \"#treescan-tree-visualization\", levelSeparation: 20, siblingSeparation: 15, subTeeSeparation: 15, rootOrientation: \"WEST\", ";
        outfile << "hideRootNode: true, " << std::endl;
        outfile << "node: { HTMLclass: \"treescan-node-tree\", drawLineThrough: false, collapsable: true }, connectors: { type: \"bCurve\"} }, ";
        outfile << "nodeStructure: ";
        // create a map of NodeID to Cut objects
        std::map<int, const CutStructure*> node_cut_map;
        ScanRunner::CutStructureContainer_t::const_iterator itr = _scanRunner.getCuts().begin(), enditr = _scanRunner.getCuts().end();
        for (; itr != enditr; ++itr)
            node_cut_map[(*itr)->getID()] = (*itr);
        // add the trimmed cuts as well
        enditr = _scanRunner.getTrimmedCuts().end();
        for (itr = _scanRunner.getTrimmedCuts().begin(); itr != enditr; ++itr)
            node_cut_map[(*itr)->getID()] = (*itr);
        outfile << " { text:{name:\"Root\"}";
        std::stringstream  rootstream;
        unsigned int root_counter = 0;
        for (const auto& rootNode: _scanRunner.getRootNodes()) {
            // For each root node, walk down tree looking for significant nodes.
            std::stringstream  nodestream;
            NodeSet_t test = writeJsTreeNode(nodestream, *rootNode, node_cut_map, 2);
            // If the best p-value for this node or down along descendent's branch meets threshold, then include branch in nodes. Otherwise exclude entire branch.
            BestCutSet_t best_branch(sequentialTreeOnly ? std::max(test.get<0>().get<0>(), test.get<1>().get<0>()) : std::min(test.get<0>().get<0>(), test.get<1>().get<0>()),
                                     std::max(test.get<0>().get<1>(), test.get<1>().get<1>()));
            if ((sequentialTreeOnly ? best_branch.get<0>() > 0.0 : best_branch.get<0>() <= 0.05)) {
                if (root_counter > 0) rootstream << ",";
                ++root_counter;
                rootstream << nodestream.str();
            }
        }
        if (root_counter > 0)
            outfile << ", children: [" << std::endl << rootstream.str() << "]";
        outfile << " }" << std::endl << "};" << std::endl;
        outfile << "$(document).ready(function(){if (Object.keys(chart_config.nodeStructure).length < 2){$('#show_tree').addClass('disabled').html('Cuts Visualization (No nodes for display)');}});</script>" << std::endl;
    }
    outfile << "<script src=\"https://www.treescan.org/html-results/treescan-results.1.3.1.js\" type=\"text/javascript\"></script>" << std::endl;
    outfile << "<body>" << std::endl;
    buffer = AppToolkit::getToolkit().GetWebSite();
    outfile << "<div class='hr' style='margin-top: 5px;'></div><div class='program-info'>" << std::endl;
    outfile << "<p style='font-size:15px;font-weight:bold;'>" << getAnalysisSuccinctStatement(buffer, std::string("<br/>")) << "</p>";
    outfile << "<h2 style='font-size:15px;margin: 8px 0 5px 0;font-weight:bold;'>SUMMARY STATISTICS</h2>" << std::endl;    

    outfile << "<table class='analysis-summary'><tbody>" << std::endl;
    if (parameters.getScanType() != Parameters::TIMEONLY) {
        const TreeStatistics& treestats = _scanRunner.getTreeStatistics();
        outfile << "<tr><th class='section-caption'><span>Tree</span></th><td></td></tr>" << std::endl;
        outfile << "<tr><th>Number of Nodes:</th><td>" << treestats._num_nodes << "</td></tr>" << std::endl;
        outfile << "<tr><th>Number of Root Nodes:</th><td>" << treestats._num_root << "</td></tr>" << std::endl;
        outfile << "<tr><th>Number of Nodes with Children:</th><td>" << treestats._num_parent << "</td></tr>" << std::endl;
        outfile << "<tr><th>Number of Leaf Nodes:</th><td>" << treestats._num_leaf << "</td></tr>" << std::endl;
        outfile << "<tr><th>Number of Levels in Tree:</th><td>" << treestats._nodes_per_level.size() << "</td></tr>" << std::endl;
        std::stringstream stringbuffer;
        for (TreeStatistics::NodesLevel_t::const_iterator itr = treestats._nodes_per_level.begin(); itr != treestats._nodes_per_level.end(); ++itr)
            stringbuffer << itr->second << (itr->first == treestats._nodes_per_level.size() ? "" : ", ");
        outfile << "<tr><th>Nodes per Levels:</th><td>" << stringbuffer.str().c_str() << "</td></tr>" << std::endl;

        if (parameters.getRestrictTreeLevels() || parameters.getRestrictEvaluatedTreeNodes()) {
            outfile << "<tr><th>Number of Nodes Evaluated:</th><td>" << treestats._num_nodes_evaluated << "</td></tr>" << std::endl;
            if (parameters.getRestrictTreeLevels()) {
                outfile << "<tr><th>Tree Levels Included:</th><td>" << treestats.toCsvString(treestats._levels_included, buffer) << "</td></tr>" << std::endl;
                outfile << "<tr><th>Tree Levels Excluded:</th><td>" << treestats.toCsvString(treestats._levels_excluded, buffer) << "</td></tr>" << std::endl;
            }
        }
    }
    if (parameters.isSequentialScanTreeOnly()) {
        const SequentialStatistic & sequentialStatistic = _scanRunner.getSequentialStatistic();
        outfile << "<tr><th class='section-caption'><span>Present Look</span></th><td></td></tr>" << std::endl;
        outfile << "<tr><th>Look:</th><td>" << _scanRunner.getSequentialStatistic().getLook() << "</td></tr>" << std::endl;
        outfile << "<tr><th>Alpha This Look:</th><td>" << parameters.getSequentialAlphaSpending() << "</td></tr>" << std::endl;
        outfile << "<tr><th>Total Cases:</th><td>" << _scanRunner.getTotalsFromLook().first << "</td></tr>" << std::endl;
        if (parameters.getModelType() == Parameters::POISSON)
            outfile << "<tr><th>Total Expected:</th><td>" << getValueAsString(_scanRunner.getTotalsFromLook().second, buffer, 1) << "</td></tr>" << std::endl;
        if (parameters.getModelType() == Parameters::BERNOULLI_TREE)
            outfile << "<tr><th>Total Observations:</th><td>" << static_cast<int>(_scanRunner.getTotalsFromLook().second) << "</td></tr>" << std::endl;
        outfile << "<tr><th>Critical Value To Reject:</th><td>" << _scanRunner.getSequentialStatistic().getCriticalValue() << "</td></tr>" << std::endl;
        if (_scanRunner.getSequentialStatistic().getLook() > 1) {
            outfile << "<tr><th class='section-caption'><span>Cumulative Look</span></th><td></td></tr>" << std::endl;
            outfile << "<tr><th>Alpha Spent To Date:</th><td>" << sequentialStatistic.getAlphaSpending() << "</td></tr>" << std::endl;
            outfile << "<tr><th>Total Cases:</th><td>" << _scanRunner.getTotalC() << "</td></tr>" << std::endl;
            if (parameters.getModelType() == Parameters::POISSON)
                outfile << "<tr><th>Total Expected:</th><td>" << getValueAsString(_scanRunner.getTotalN(), buffer, 1) << "</td></tr>" << std::endl;
            if (parameters.getModelType() == Parameters::BERNOULLI_TREE)
                outfile << "<tr><th>Total Observations:</th><td>" << static_cast<int>(_scanRunner.getTotalN()) << "</td></tr>" << std::endl;
        }
    } else {
        outfile << "<tr><th class='section-caption'><span>Data Summary</span></th><td></td></tr>" << std::endl;
        if (!parameters.getPerformPowerEvaluations() ||
            !(parameters.getPerformPowerEvaluations() && parameters.getPowerEvaluationType() == Parameters::PE_ONLY_CASEFILE && parameters.getConditionalType() == Parameters::UNCONDITIONAL)) {
            if (parameters.getModelType() == Parameters::SIGNED_RANK) {
                unsigned int numSites = _scanRunner.getSampleSiteIdentifiers().size();  
                outfile << "<tr><th>Total Sample Sites:</th><td>" << numSites << "</td></tr>" << std::endl;
                outfile << "<tr><th>Largest Possible Test Statistic:</th><td>" << (numSites * (numSites + 1))/2 << "</td></tr>" << std::endl;
                outfile << "<tr><th>Total Comparisons:</th><td>" << _scanRunner.getTotalC() << "</td></tr>" << std::endl;
            } else
                outfile << "<tr><th>Total Cases:</th><td>" << _scanRunner.getTotalC() << "</td></tr>" << std::endl;
            if (_scanRunner.isCensoredData()) {
                outfile << "<tr><th>Total Censored Cases:</th><td>" << _scanRunner.getNumCensoredCases() << "</td></tr>" << std::endl;
                outfile << "<tr><th>Average Censoring Time:</th><td>" << _scanRunner.getAvgCensorTime() << "</td></tr>" << std::endl;
            }
        }
        if (parameters.isApplyingExclusionTimeRanges()) {
            outfile << "<tr><th>Total Cases Excluded:</th><td>" << _scanRunner.getNumExcludedCases() << "</td></tr>" << std::endl;
            if (parameters.getModelType() == Parameters::BERNOULLI_TIME)
                outfile << "<tr><th>Total Controls Excluded:</th><td>" << _scanRunner.getNumExcludedControls() << "</td></tr>" << std::endl;
        }
        if (parameters.getModelType() == Parameters::POISSON)
            outfile << "<tr><th>Total Expected:</th><td>" << getValueAsString(_scanRunner.getTotalN(), buffer, 1).c_str() << "</td></tr>" << std::endl;
        if (parameters.getModelType() == Parameters::BERNOULLI_TREE || parameters.getModelType() == Parameters::BERNOULLI_TIME) {
            outfile << "<tr><th>Total Observations:</th><td>";
            if (parameters.getConditionalType() == Parameters::UNCONDITIONAL && parameters.getVariableCaseProbability())
                outfile << _scanRunner.getTotalC() + _scanRunner.getTotalControls();
            else
                outfile << static_cast<int>(_scanRunner.getTotalN());
            outfile << "</td></tr>" << std::endl;
        }
        if (Parameters::isTemporalScanType(parameters.getScanType()))
            outfile << "<tr><th>Data Time Range:</th><td>" << parameters.getDataTimeRangeSet().toString(buffer, parameters.getDatePrecisionType()).c_str() << "</td></tr>" << std::endl;
        if (parameters.isApplyingExclusionTimeRanges())
            outfile << "<tr><th>Excluded Time Ranges:</th><td>" << parameters.getExclusionTimeRangeSet().toString(buffer, parameters.getDatePrecisionType()).c_str() << "</td></tr>" << std::endl;
        if (parameters.isSequentialScanTreeOnly()) {
            const SequentialStatistic & sequentialStatistic = _scanRunner.getSequentialStatistic();
            outfile << "<tr><th>Alpha Spent To Date:</th>";
            outfile << "<td>" << sequentialStatistic.getAlphaSpending() << " (" << parameters.getSequentialAlphaOverall() << " alpha overall)</td></tr>" << std::endl;
        }
    }
    outfile << "</tbody></table></div>" << std::endl;

    outfile << "<div class='hr'></div>" << std::endl;
    if (parameters.isSequentialScanPurelyTemporal() && static_cast<unsigned int>(_scanRunner.getTotalC()) > parameters.getSequentialMaximumSignal()) {
        outfile << "<div class=\"warning\">Note: The sequential scan reached or exceeded the specified maximum cases. The sequential analysis is over.</div><div class=\"hr\"></div>";
    } else if (!parameters.getPerformPowerEvaluations() || (parameters.getPerformPowerEvaluations() && parameters.getPowerEvaluationType() == Parameters::PE_WITH_ANALYSIS)) {
        if (treeSequentialAnalysisComplete())
            outfile << "<div class='warning'>Note: The alpha spending for sequential scan reached the specified alpha overall. The sequential analysis is over.</div><div class='hr'></div>";
        outfile << "<div id='cuts'>" << std::endl;
        if (_scanRunner.getCuts().size() == 0 || !_scanRunner.reportableCut(*_scanRunner.getCuts()[0])) {
            outfile << "<h3>No cuts were found.</h3>" << std::endl;
        } else {
            outfile << "<h3>MOST LIKELY CUTS</h3><div><table id='id_cuts' class='display' style='width:100%'>" << std::endl;
            outfile << "<thead><tr><th>No.</th>";
            // skip reporting node identifier for time-only scans
            if (parameters.getScanType() != Parameters::TIMEONLY) {
                outfile << "<th>Node ID</th>" << (_scanRunner.hasNodeDescriptions() ? "<th>Node Name</th>" : "") << "<th>Tree Level</th>";
            }
            if (Parameters::isTemporalScanType(parameters.getScanType())) {
               // skip reporting node cases for time-only scans
                if (parameters.getScanType() != Parameters::TIMEONLY) {
                    if (parameters.getModelType() == Parameters::BERNOULLI_TIME)
                        outfile << "<th>Node Observations</th>";
                    outfile << "<th>Node Cases</th>";
                }
                outfile << "<th>Time Window</th>";
                if (parameters.getModelType() == Parameters::BERNOULLI_TIME)
                    outfile << "<th>Observations in Window</th>";
                outfile << "<th>Cases in Window</th>";
            } else if (parameters.getModelType() == Parameters::BERNOULLI_TREE) {
                outfile << "<th>Observations</th><th>Cases</th>";
            } else if (parameters.getModelType() == Parameters::POISSON) {
                outfile << "<th>Observed Cases</th>";
            } else if (parameters.getModelType() == Parameters::SIGNED_RANK) {
                outfile << "<th>Baseline Average</th><th>Current Average</th><th>Percent Change</th><th>Absolute Change</th>";
            }
            if (parameters.getModelType() != Parameters::SIGNED_RANK) {
                outfile << "<th>" << (parameters.getModelType() == Parameters::UNIFORM ? "Expected Cases" : "Expected") << "</th>";
                outfile << "<th>Relative Risk</th><th>Excess Cases</th>" << std::endl;
                if (parameters.getReportAttributableRisk())
                    outfile << "<th>Attributable Risk</th>" << std::endl;
            }
            if (parameters.getIsTestStatistic()) {
                // If we stick with Poisson log-likelihood calculation, then label is 'Test Statistic' in place of 'Log Likelihood Ratio', hyper-geometric is 'Log Likelihood Ratio'.
                outfile << "<th>Test Statistic</th>" << std::endl;
            } else {
                outfile << "<th>LLR</th>" << std::endl;
            }
            if (parameters.getNumReplicationsRequested() > MIN_REPLICA_RPT_PVALUE && !parameters.isSequentialScanTreeOnly()) {
                outfile << "<th>P-value</th>";
                if (parameters.getIsProspectiveAnalysis())
                    outfile << "<th>Recurrence Interval</th>";
            }
            if (parameters.getScanType() != Parameters::TIMEONLY) {
                outfile << "<th>Parent Node</th>" << (_scanRunner.hasNodeDescriptions() ? "<th>Parent Node Name</th>" : "") << "<th>Branch Order</th>";
            }
            if (parameters.isSequentialScanTreeOnly()) {
                outfile << "<th>Signalled</th>";
            }

            outfile << "</tr></thead><tbody>" << std::endl;
            std::string format, replicas;
            printString(replicas, "%u", parameters.getNumReplicationsRequested());
            printString(format, "%%.%dlf", replicas.size());

            std::stringstream subrows;
            outfile.setf(std::ios::fixed);
            outfile.precision(5);
            ScanRunner::CutStructureContainer_t::const_iterator itrCuts = _scanRunner.getCuts().begin(), itrCutsEnd = _scanRunner.getCuts().end();
            for (unsigned int k=0; itrCuts != itrCutsEnd; ++itrCuts, ++k)
                addTableRowForCut(*(*itrCuts), calcLogLikelihood, format, outfile, &subrows);
            outfile << "</tbody></table></div>" << std::endl;
            outfile << "</div></div>" << std::endl;
            outfile << "<script>" << std::endl << "var sub_rows = {" << subrows.str() << "};" << std::endl << "</script>" << std::endl;

            outfile << "<div id=\"id_trimmed_cuts\" style=\"display:none;\"><table>";
            itrCuts = _scanRunner.getTrimmedCuts().begin();
            itrCutsEnd = _scanRunner.getTrimmedCuts().end();
            for (; itrCuts != itrCutsEnd; ++itrCuts)
                addTableRowForCut(*(*itrCuts), calcLogLikelihood, format, outfile);
            outfile << "</table></div>" << std::endl;
        }
    }

    if (showingTreeGraph) {
        outfile << "<a class=\"btn btn-primary btn-sm\" id=\"show_tree\" data-toggle=\"collapse\" href=\"#collapseExample\" role=\"button\" aria-expanded=\"false\" aria-controls=\"collapseExample\">Cuts Visualization</a>" << std::endl;
        outfile << "<div class=\"collapse\" id=\"collapseExample\"><h3>Visualization of Cuts in Tree  <span id=\"loading_graph\">... loading <i class=\"fa fa-circle-o-notch fa-spin\"></i></span><span id=\"fail_message\"></span></h3>";
        outfile << "<div class=\"row\">" << std::endl;

        outfile << "<div class=\"col-3\"><div class=\"custom-control custom-radio\">" << std::endl;
        if (sequentialTreeOnly) {
            outfile << "<input type=\"radio\" class=\"custom-control-input\" id=\"customControlValidation1\" name=\"radio-stacked\" legend=\"legend-signalled\" required checked=checked>" << std::endl;
            outfile << "<label class=\"custom-control-label\" for=\"customControlValidation1\">Color Nodes by Signal</label></div>" << std::endl;
        } else {
            outfile << "<input type=\"radio\" class=\"custom-control-input\" id=\"customControlValidation2\" name=\"radio-stacked\" legend=\"legend-p-value\" required checked=checked>" << std::endl;
            outfile << "<label class=\"custom-control-label\" for=\"customControlValidation2\">Color Nodes by P-Value</label></div>" << std::endl;
        }
        if (parameters.getIsProspectiveAnalysis()) {
            outfile << "<div class=\"custom-control custom-radio\">" << std::endl;
            outfile << "<input type=\"radio\" class=\"custom-control-input\" id=\"customControlValidation4\" name=\"radio-stacked\" legend=\"legend-recurrence-interval\" required>" << std::endl;
            outfile << "<label class=\"custom-control-label\" for=\"customControlValidation4\">Color Nodes by Recurrence Interval</label></div>" << std::endl;
        }
        outfile << "<div class=\"custom-control custom-radio mb-3\">" << std::endl;
        outfile << "<input type=\"radio\" class=\"custom-control-input\" id=\"customControlValidation3\" name=\"radio-stacked\" legend=\"legend-relative-risk\" required>" << std::endl;
        outfile << "<label class=\"custom-control-label\" for=\"customControlValidation3\">Color Nodes by Relative Risk</label></div></div>" << std::endl;
        outfile << "<div class=\"col-9\">";
        if (sequentialTreeOnly) {
            outfile << "<div class='chart-legend legend-signalled'><div class='legend-title'>Signal Legend</div><div class='legend-scale'>" << std::endl;
            outfile << "<ul class='legend-labels'><li><span style='background:#566573;'></span><span class=\"legend-val\">Not Signalled</span></li>";
            outfile << "<li><span style='background:#FF5733;'></span><span class=\"legend-val\">Signalled</span></li></ul></div></div>" << std::endl;
        } else {
            outfile << "<div class='chart-legend legend-p-value'><div class='legend-title'>P-Value Legend</div><div class='legend-scale'>" << std::endl;
            outfile << "<ul class='legend-labels'><li><span style='background:#566573;'></span><span class=\"legend-val\">&gt; 0.05</span></li><li><span style='background:#DBD51B;'></span><span class=\"legend-val\">0.05</span></li>";
            outfile << "<li><span style='background:#FFC300;'></span><span class=\"legend-val\">0.01</span></li><li><span style='background:#FF5733;'></span><span class=\"legend-val\">0.001</span></li></ul></div></div>" << std::endl;
        }
        if (parameters.getIsProspectiveAnalysis()) {
            outfile << "<div class='chart-legend legend-recurrence-interval'><div class='legend-title'>Recurrence Interval Legend</div><div class='legend-scale'>";
            outfile << "<ul class='legend-labels'><li><span style='background:#566573;'></span><span class=\"legend-val\">&lt; 100 days</span></li><li><span style='background:#DBD51B;'></span><span class=\"legend-val\">100 days</span></li>";
            outfile << "<li><span style='background:#FFC300;'></span><span class=\"legend-val\">1 year</span></li><li><span style='background:#FF5733;'></span><span class=\"legend-val\">5 years</span></li>" << std::endl;
            outfile << "<li><span style='background:#8A1901;'></span><span class=\"legend-val\">100 years</span></li></ul></div></div>" << std::endl;
        }
        outfile << "<div class='chart-legend legend-relative-risk'><div class='legend-title'>Relative Risk Legend</div><div class='legend-scale'>";
        outfile << "<ul class='legend-labels'><li><span style='background:#566573;'></span><span class=\"legend-val\">&lt; 2</span></li><li><span style='background:#DBD51B;'></span><span class=\"legend-val\">2</span></li>";
        outfile << "<li><span style='background:#FFC300;'></span><span class=\"legend-val\">4</span></li><li><span style='background:#FF5733;'></span><span class=\"legend-val\">8</span></li></ul></div></div></div>" << std::endl;
        outfile << "</div><div class=\"chart\" id=\"treescan-tree-visualization\" style=\"background-color: #EAEDED; border: 2px solid #566573; border-radius: 5px; padding:2px;\"> </div>" << std::endl;
        outfile << "<div class=\"row\" style=\"font-style:italic; margin:5px 20px 10px 30px;font-size: 1.1em;\"><ol><li>Selecting the circle in the upper right corner of each node will expand/collapse children under node. The color of circle indicates best p-value &#47; relative risk found in descendent nodes.</li>";
        if (sequentialTreeOnly)
            outfile << "<li>The visualization tree displays significant nodes which have signalled. Siblings and ancestry to root are also displayed for significant nodes, regardless of whether they have signalled.</li>";
        else if (parameters.getIsProspectiveAnalysis())
            outfile << "<li>The visualization tree displays nodes with recurrence interval &ge; 100 days. Siblings and ancestry to root are also displayed for significant nodes, regardless of their recurrence interval.</li>";
        else
            outfile << "<li>The visualization tree displays nodes with significant p-values (&le; 0.05). Siblings and ancestry to root are also displayed for significant nodes, regardless of their p-value.</li>";
        outfile << "</ol></div></div>" << std::endl;
    }

    outfile << "<div class=\"hr\"></div>" << std::endl;
    if (Parameters::isTemporalScanType(parameters.getScanType())) {
        std::string buffer;
        _scanRunner.getCaselessWindowsAsString(buffer);
        if (buffer.size()) {
            outfile << "<div class=\"warning\">Warning: The following "
                    << (parameters.getDatePrecisionType() == DataTimeRange::GENERIC ? "days" : "dates")
                    <<" in the data time range do not have cases: " << buffer.c_str() << "</div><div class=\"hr\"></div>";
        }
    }

    // Print critical values if requested.
    if ((parameters.getReportCriticalValues() && parameters.getNumReplicationsRequested() >= 19) || (parameters.getPerformPowerEvaluations() && parameters.getCriticalValuesType() == Parameters::CV_MONTECARLO)) {
        outfile << "<div class=\"program-info\">" << std::endl;
        outfile << std::endl << "<div>A cut is statistically significant when its log likelihood ratio is greater than the critical value, which is, for significance level:</div>" << std::endl;
        if (parameters.getNumReplicationsRequested() >= 99999) {
            CriticalValues::alpha_t alpha00001(_scanRunner.getCriticalValues().getAlpha00001());
            outfile << "<div>... 0.00001: ";
            if (parameters.getModelType() == Parameters::SIGNED_RANK)
                outfile << printString(buffer, "%.0lf", alpha00001.second) << "</div>" << std::endl;
            else
                outfile << alpha00001.second << "</div>" << std::endl;
        }
        if (parameters.getNumReplicationsRequested() >= 9999) {
            CriticalValues::alpha_t alpha0001(_scanRunner.getCriticalValues().getAlpha0001());
            outfile << "<div>... 0.0001: ";
            if (parameters.getModelType() == Parameters::SIGNED_RANK)
                outfile << printString(buffer, "%.0lf", alpha0001.second) << "</div>" << std::endl;
            else
                outfile << alpha0001.second << "</div>" << std::endl;
        }
        if (parameters.getNumReplicationsRequested() >= 999) {
            CriticalValues::alpha_t alpha001(_scanRunner.getCriticalValues().getAlpha001());
            outfile << "<div>... 0.001: ";
            if (parameters.getModelType() == Parameters::SIGNED_RANK)
                outfile << printString(buffer, "%.0lf", alpha001.second) << "</div>" << std::endl;
            else
                outfile << alpha001.second << "</div>" << std::endl;
        }
        if (parameters.getNumReplicationsRequested() >= 99) {
            CriticalValues::alpha_t alpha01(_scanRunner.getCriticalValues().getAlpha01());
            outfile << "<div>... 0.01: ";
            if (parameters.getModelType() == Parameters::SIGNED_RANK)
                outfile << printString(buffer, "%.0lf", alpha01.second) << "</div>" << std::endl;
            else
                outfile << alpha01.second << "</div>" << std::endl;
        }
        if (parameters.getNumReplicationsRequested() >= 19) {
            CriticalValues::alpha_t alpha05(_scanRunner.getCriticalValues().getAlpha05());
            outfile << "<div>... 0.05: ";
            if (parameters.getModelType() == Parameters::SIGNED_RANK)
                outfile << printString(buffer, "%.0lf", alpha05.second) << "</div>" << std::endl;
            else
                outfile << alpha05.second << "</div>" << std::endl;
        }
        outfile << "</div>" << std::endl;
    }
    // Report critical value for tree-only sequentual scans.
    if (parameters.isSequentialScanTreeOnly()) {
        outfile << "<div class=\"program-info\">" << std::endl;
        outfile << "<div>Critical value for look: " << _scanRunner.getSequentialStatistic().getCriticalValue() << std::endl;
        outfile << "</div></div>" << std::endl;
    }
    // Report early termination
    if (parameters.getTerminateSimulationsEarly() && _scanRunner.getCuts().size() && _scanRunner.getSimulationVariables().get_sim_count() < parameters.getNumReplicationsRequested()) {
        outfile << "<div class=\"program-info\">" << std::endl;
        outfile << "<div>The sequential Monte Carlo procedure was used to terminate the calculations after " << _scanRunner.getSimulationVariables().get_sim_count() << " replications." << std::endl;
        outfile << "</div></div>" << std::endl;
    }
    // print power estimation values
    if (parameters.getPerformPowerEvaluations()) {
        outfile << "<div class=\"program-info\">" << std::endl;
        if (_scanRunner.getPowerEstimations().size() == 0)
            throw prg_error("Number of power estimations is zero.", "ResultsFileWriter::writeHTML(...)");
        outfile << std::endl << "<h4>Estimated Power</h4><table style=\"text-align: left;width:50%;\"><tbody>" << std::endl;
        outfile << "<tr><th>Alpha</th><th>0.05</th><th>0.01</th><th>0.001</th></tr>" << std::endl;
        std::string buffer;
        ScanRunner::PowerEstimationContainer_t::const_iterator itr=_scanRunner.getPowerEstimations().begin(), itr_end=_scanRunner.getPowerEstimations().end();
        for (;itr != itr_end; ++itr) {
            outfile << "<tr><th>" << printString(buffer, "Alternative #%u", std::distance(_scanRunner.getPowerEstimations().begin(), itr) + 1).c_str() << "</th>";
            outfile << "<td>" << getRoundAsString(itr->get<0>(),buffer, 3).c_str() << "</td>";
            outfile << "<td>" << getRoundAsString(itr->get<1>(),buffer, 3).c_str() << "</td>";
            outfile << "<td>" << getRoundAsString(itr->get<2>(),buffer, 3).c_str() << "</td></tr>" << std::endl;
        }
        outfile << "</tbody></table></div>" << std::endl;
    }

    ParametersPrint(parameters).PrintHTML(outfile);
    outfile << "<div class=\"hr\"></div>" << std::endl;
    outfile << "<div class=\"program-info\"><h4>COMPUTATIONAL INFORMATION</h4><table style=\"text-align: left;\"><tbody>" << std::endl;
    outfile << "<tr><th>Program run on</th><td>" << ctime(&start) << "</td></tr>" << std::endl;
    outfile << "<tr><th>Program completed</th><td>" << ctime(&end) << "</td></tr>" << std::endl;
    outfile << "<tr><th>Total Running Time</th><td>" << getTotalRunningTime(start, end, buffer) << "</td></tr>" << std::endl;
    if (parameters.getNumParallelProcessesToExecute() > 1) outfile << "<tr><th>Processor Usage</th><td>" << parameters.getNumParallelProcessesToExecute() << " processors" << "</td></tr>" << std::endl;
    printString(buffer,
        "TreeScan v%s.%s%s%s%s%s",
        VERSION_MAJOR,
        VERSION_MINOR,
        (!strcmp(VERSION_RELEASE, "0") ? "" : "."),
        (!strcmp(VERSION_RELEASE, "0") ? "" : VERSION_RELEASE),
        (strlen(VERSION_PHASE) ? " " : ""),
        VERSION_PHASE
    );
    outfile << "<tr><th>Version</th><td>" << buffer << "</td></tr>" << std::endl;
    outfile << "</tbody></table></div>" << std::endl;

    outfile << "</body></html>" << std::endl;
    outfile.close();

    return true;
}

/** Write table row for cut. */
std::ofstream & ResultsFileWriter::addTableRowForCut(CutStructure& thisCut, Loglikelihood_t & calcLogLikelihood, const std::string& format, std::ofstream& outfile, std::stringstream * subrows) {
    const Parameters& parameters = _scanRunner.getParameters();
    const NodeStructure& thisNode = *(_scanRunner.getNodes()[thisCut.getID()]);
    std::string node_tr, buffer, buffer2;
    std::vector<boost::shared_ptr<RecordBuffer>> childRecords;
    ptr_vector<FieldDef> fieldDefinitions;

    printString(node_tr, "ID_%d", thisNode.getID());
    outfile << "<tr id=\"tr-" << encodeForJavascript(node_tr) << "\"><td>" << thisCut.getReportOrder() << "</td>";
    // skip reporting node identifier for time-only scans
    if (parameters.getScanType() != Parameters::TIMEONLY) {
        // Obtain the child notes for this cut and remove any children which are not interesting.
        CutsRecordWriter::getFieldDefs(fieldDefinitions, parameters, _scanRunner.hasNodeDescriptions());
        for (auto pnode : _scanRunner.getCutChildNodes(thisCut)) {
            boost::shared_ptr<RecordBuffer> record(new RecordBuffer(fieldDefinitions));
            CutsRecordWriter::getRecordForCutChild(*(record), thisCut, *pnode, thisCut.getReportOrder(), _scanRunner);
            if (CutsRecordWriter::includeChild(_scanRunner, thisCut, *(record))) // Add this record if it is interesting.
                childRecords.push_back(record);
        }
        CutsRecordWriter::sortChildRecords(childRecords, _scanRunner.getParameters(), thisCut.getRate(_scanRunner));
        buffer = thisNode.getIdentifier();
        outfile << "<td>" << (childRecords.size() > 0 ? "<a href=\"#\" class=\"cut-node-w-children\">" : "") << htmlencode(buffer);
        outfile << (childRecords.size() > 0 ? "</a>" : "") << "</td>";
        if (_scanRunner.hasNodeDescriptions()) {
            buffer = thisNode.getName();
            outfile << "<td>" << htmlencode(buffer) << "</td>";
        }
        outfile << "<td>" << printString(buffer, "%ld", static_cast<int>(thisNode.getLevel())).c_str() << "</td>";
    }
    if (parameters.getScanType() == Parameters::TREETIME && parameters.getConditionalType() == Parameters::NODEANDTIME) {
        // write node cases and time window
        printString(buffer, "%ld", static_cast<int>(thisNode.getBrC()));
        outfile << "<td>" << buffer.c_str() << "</td>";
        int startIdx = (thisCut.getStartIdx() - _scanRunner.getZeroTranslationAdditive());
        if (parameters.getDatePrecisionType() == DataTimeRange::GENERIC)
            outfile << "<td data-order=" << startIdx << ">" << startIdx << " to " << (thisCut.getEndIdx() - _scanRunner.getZeroTranslationAdditive()) << "</td>";
        else {
            std::pair<std::string, std::string> rangeDates = parameters.getDataTimeRangeSet().getDataTimeRangeSets().front().rangeToGregorianStrings(
                thisCut.getStartIdx() - _scanRunner.getZeroTranslationAdditive(),
                thisCut.getEndIdx() - _scanRunner.getZeroTranslationAdditive(),
                parameters.getDatePrecisionType()
            );
            outfile << "<td data-order=" << startIdx << ">" << rangeDates.first << " to " << rangeDates.second << "</td>";
        }
    } else if (parameters.getModelType() == Parameters::UNIFORM) {
        // write node cases
        // skip reporting node cases for time-only scans
        if (parameters.getScanType() != Parameters::TIMEONLY) {
            if (parameters.isPerformingDayOfWeekAdjustment() || _scanRunner.isCensoredData()) {
                printString(buffer, "%ld", static_cast<int>(thisNode.getBrC()));
            } else {
                printString(buffer, "%ld", static_cast<int>(thisCut.getN()));
            }
            outfile << "<td>" << buffer.c_str() << "</td>";
        }
        // write time window
        int startIdx = (thisCut.getStartIdx() - _scanRunner.getZeroTranslationAdditive());
        if (parameters.getDatePrecisionType() == DataTimeRange::GENERIC)
            outfile << "<td data-order=" << startIdx << ">" << startIdx << " to " << (thisCut.getEndIdx() - _scanRunner.getZeroTranslationAdditive()) << "</td>";
        else {
            std::pair<std::string, std::string> rangeDates = parameters.getDataTimeRangeSet().getDataTimeRangeSets().front().rangeToGregorianStrings(
                thisCut.getStartIdx() - _scanRunner.getZeroTranslationAdditive(),
                thisCut.getEndIdx() - _scanRunner.getZeroTranslationAdditive(),
                parameters.getDatePrecisionType()
            );
            outfile << "<td data-order=" << startIdx << ">" << rangeDates.first << " to " << rangeDates.second << "</td>";
        }
    } else if (parameters.getModelType() == Parameters::BERNOULLI_TIME) {
        // skip reporting node cases/observations for time-only scans
        if (parameters.getScanType() != Parameters::TIMEONLY) {
            printString(buffer, "%ld", static_cast<int>(thisNode.getBrN()));
            outfile << "<td>" << buffer.c_str() << "</td>";
            printString(buffer, "%ld", static_cast<int>(thisNode.getBrC()));
            outfile << "<td>" << buffer.c_str() << "</td>";
        }
        // write time window
        int startIdx = (thisCut.getStartIdx() - _scanRunner.getZeroTranslationAdditive());
        if (parameters.getDatePrecisionType() == DataTimeRange::GENERIC)
            outfile << "<td data-order=" << startIdx << ">" << startIdx << " to " << (thisCut.getEndIdx() - _scanRunner.getZeroTranslationAdditive()) << "</td>";
        else {
            std::pair<std::string, std::string> rangeDates = parameters.getDataTimeRangeSet().getDataTimeRangeSets().front().rangeToGregorianStrings(
                thisCut.getStartIdx() - _scanRunner.getZeroTranslationAdditive(),
                thisCut.getEndIdx() - _scanRunner.getZeroTranslationAdditive(),
                parameters.getDatePrecisionType()
            );
            outfile << "<td data-order=" << startIdx << ">" << rangeDates.first << " to " << rangeDates.second << "</td>";
        }
        outfile << "<td>" << static_cast<int>(thisCut.getN()) << "</td>";
    } else if (parameters.getModelType() == Parameters::BERNOULLI_TREE) {
        // write number of observations
        if (parameters.getConditionalType() == Parameters::UNCONDITIONAL && parameters.getVariableCaseProbability())
            outfile << "<td>" << thisCut.getMatchedSets().get().size() << "</td>";
        else
            outfile << "<td>" << static_cast<int>(thisCut.getN()) << "</td>";
    }
    if (parameters.getModelType() == Parameters::SIGNED_RANK) {
        auto proportions = getAverage(thisCut.getSampleSiteData());
        outfile << "<td>" << printString(buffer, "%g", proportions.first) << "</td><td>" << printString(buffer2, "%g", proportions.second) << "</td>";
        if (proportions.first)
            outfile << "<td>" << printString(buffer, "%g", proportions.second/proportions.first - 1.0);
        else
            outfile << "<td data-order=99999>infinity";
        outfile << "</td><td>" << printString(buffer2, "%g", std::abs(proportions.second - proportions.first)) << "</td>";
    } else {
        // write cases in window or cases or observed cases, depending on settings
        outfile << "<td>" << thisCut.getC() << "</td>";
        outfile << "<td>" << getValueAsString(thisCut.getExpected(_scanRunner), buffer) << "</td>";
        double rr = thisCut.getRelativeRisk(_scanRunner);
        outfile << "<td data-order=" << std::scientific << (rr == std::numeric_limits<double>::infinity() ? std::numeric_limits<double>::max() : rr);
        if (parameters.getIsSelfControlVariableBerounlli())
            outfile << std::fixed << ">" << getValueAsString(rr, buffer) << "</td>";
        else
            outfile << std::fixed << ">" << getValueAsString(rr, buffer) << "</td>";
        outfile << "<td>" << getValueAsString(thisCut.getExcessCases(_scanRunner), buffer) << "</td>";
        if (parameters.getReportAttributableRisk()) {
            double ar = thisCut.getAttributableRisk(_scanRunner);
            outfile << "<td data-order="<< ar << ">" << AttributableRiskAsString(ar, buffer) << "</td>";
        }
    }
    double llr = calcLogLikelihood->LogLikelihoodRatio(thisCut.getLogLikelihood());
    if (parameters.isSequentialScanTreeOnly() && !macro_less_than(MIN_CUT_LLR, llr, DBL_CMP_TOLERANCE))
        outfile << "<td data-order=0>Not Applicable</td>"; 
    else if (parameters.getModelType() == Parameters::SIGNED_RANK) {
        outfile << "<td>" << printString(buffer, "%.0lf", llr) << "</td>";
    } else {
        printString(buffer, "%.6lf", llr);
        outfile << "<td data-order=" << buffer << ">" << buffer << "</td>";
    }
    // write p-value
    if (_scanRunner.reportablePValue(thisCut)) {
        outfile << "<td>" << printString(buffer, format.c_str(), thisCut.getPValue(_scanRunner)) << "</td>";
        if (parameters.getIsProspectiveAnalysis()) {
            RecurrenceInterval_t ri = _scanRunner.getRecurrenceInterval(thisCut);
            outfile << "<td data-order="
                << getRoundAsString(ri.second, buffer, std::min(std::to_string(_scanRunner.getSimulationVariables().get_sim_count()).size(), (size_t)10))
                << ">" << getRecurranceIntervalAsString(ri, buffer2) << "</td>";
        }
    }
    if (parameters.getScanType() != Parameters::TIMEONLY) {
        outfile << "<td>" << htmlencode(thisCut.getParentIndentifiers(_scanRunner, buffer, true)) << "</td>";
        if (_scanRunner.hasNodeDescriptions())
            outfile << "<td>" << (htmlencode(thisCut.getParentIndentifiers(_scanRunner, buffer2, false)) != buffer ? buffer2 : "") << "</td>";
        outfile << "<td class='exclude-tree'>" << thisCut.getBranchOrder() << "</td>";
    }
    if (parameters.isSequentialScanTreeOnly()) {
        /* Hack - this is dependent on the ResultsFileWriter::writeASCII being called first. */
        unsigned int signalInLook = _scanRunner.getSequentialStatistic().testCutSignaled(static_cast<size_t>(thisCut.getID()));
        outfile << "<td data-order=" << signalInLook << ">";
        if (signalInLook != 0)
            outfile << "Look " << signalInLook;
        else
            outfile << "No";
        outfile << "</td>";
    }
    outfile << "</tr>" << std::endl;

    if (parameters.getScanType() != Parameters::TIMEONLY && subrows && childRecords.size() > 0) {
        // Now write records for each direct child of this cut/node. This process is slightly brittle and relies on the columns in csv matching html table.
        std::string not_applicable("'-'");
        std::vector<std::string> children_arrays;
        for (auto& Record : childRecords) {
            std::vector<std::string> string_values;
            string_values.push_back(printString(buffer, "'%u_%u'", thisCut.getReportOrder(), children_arrays.size() + 1));
            buffer = Record->GetFieldValue(CutsRecordWriter::NODE_ID_FIELD).AsString();
            string_values.push_back(printString(buffer, "'%s'", htmlencode(buffer).c_str()));
            if (_scanRunner.hasNodeDescriptions()) {
                buffer = Record->GetFieldValue(CutsRecordWriter::NODE_NAME_FIELD).AsString();
                string_values.push_back(printString(buffer, "'%s'", htmlencode(buffer).c_str()));
            }
            string_values.push_back(printString(buffer, "'%ld'", static_cast<int>(Record->GetFieldValue(CutsRecordWriter::P_LEVEL_FLD).AsDouble())));
            switch (parameters.getModelType()) {
                case Parameters::BERNOULLI_TIME:
                    if (parameters.getScanType() != Parameters::TIMEONLY) {
                        string_values.push_back(printString(buffer, "'%ld'", static_cast<int>(Record->GetFieldValue(CutsRecordWriter::NODE_OBSERVATIONS_FIELD).AsDouble())));
                        string_values.push_back(printString(buffer, "'%ld'", static_cast<int>(Record->GetFieldValue(CutsRecordWriter::NODE_CASES_FIELD).AsDouble())));
                        if (parameters.getDatePrecisionType() == DataTimeRange::GENERIC)
                            string_values.push_back(
                                printString(buffer, "'%ld to %ld'", static_cast<int>(Record->GetFieldValue(CutsRecordWriter::START_WINDOW_FIELD).AsDouble()), static_cast<int>(Record->GetFieldValue(CutsRecordWriter::END_WINDOW_FIELD).AsDouble()))
                            );
                        else
                            string_values.push_back(
                                printString(buffer, "'%s to %s'", Record->GetFieldValue(CutsRecordWriter::START_WINDOW_FIELD).AsString().c_str(), Record->GetFieldValue(CutsRecordWriter::END_WINDOW_FIELD).AsString().c_str())
                            );
                    }
                case Parameters::BERNOULLI_TREE:
                    string_values.push_back(printString(buffer, "'%ld'", static_cast<int>(Record->GetFieldValue(CutsRecordWriter::OBSERVATIONS_FIELD).AsDouble())));
                    string_values.push_back(printString(buffer, "'%ld'", static_cast<int>(Record->GetFieldValue(CutsRecordWriter::CASES_FIELD).AsDouble())));
                    string_values.push_back(printString(buffer, "'%s'", getValueAsString(Record->GetFieldValue(CutsRecordWriter::EXPECTED_FIELD).AsDouble(), buffer2).c_str()));
                    break;
                case Parameters::POISSON:
                    string_values.push_back(printString(buffer, "'%ld'", static_cast<int>(Record->GetFieldValue(CutsRecordWriter::OBSERVED_CASES_FIELD).AsDouble())));
                    string_values.push_back(printString(buffer, "'%s'", getValueAsString(Record->GetFieldValue(CutsRecordWriter::EXPECTED_FIELD).AsDouble(), buffer2).c_str()));
                    break;
                case Parameters::SIGNED_RANK:
                    string_values.push_back(printString(buffer, "'%g'", Record->GetFieldValue(CutsRecordWriter::SS_BASELINE_FIELD).AsDouble()));
                    string_values.push_back(printString(buffer, "'%g'", Record->GetFieldValue(CutsRecordWriter::SS_CURRENT_FIELD).AsDouble()));
                    if (Record->GetFieldValue(CutsRecordWriter::SS_PERC_CHANGE_FIELD).AsDouble() == std::numeric_limits<double>::infinity())
                        string_values.emplace_back("'infinity'");
                    else
                        string_values.push_back(printString(buffer, "'%g'", Record->GetFieldValue(CutsRecordWriter::SS_PERC_CHANGE_FIELD).AsDouble()));
                    string_values.push_back(printString(buffer, "'%g'", Record->GetFieldValue(CutsRecordWriter::SS_ABS_CHANGE_FIELD).AsDouble()));
                    break;
                case Parameters::UNIFORM:
                case Parameters::MODEL_NOT_APPLICABLE:
                default:
                    if (Parameters::isTemporalScanType(parameters.getScanType())) {
                        string_values.push_back(printString(buffer, "'%ld'", static_cast<int>(Record->GetFieldValue(CutsRecordWriter::NODE_CASES_FIELD).AsDouble())));
                        if (parameters.getDatePrecisionType() == DataTimeRange::GENERIC)
                            string_values.push_back(
                                printString(buffer, "'%ld to %ld'", static_cast<int>(Record->GetFieldValue(CutsRecordWriter::START_WINDOW_FIELD).AsDouble()), static_cast<int>(Record->GetFieldValue(CutsRecordWriter::END_WINDOW_FIELD).AsDouble()))
                            );
                        else 
                            string_values.push_back(
                                printString(buffer, "'%s to %s'", Record->GetFieldValue(CutsRecordWriter::START_WINDOW_FIELD).AsString().c_str(), Record->GetFieldValue(CutsRecordWriter::END_WINDOW_FIELD).AsString().c_str())
                            );
                        string_values.push_back(printString(buffer, "'%ld'", static_cast<int>(Record->GetFieldValue(CutsRecordWriter::WNDW_CASES_FIELD).AsDouble())));
                        string_values.push_back(printString(buffer, "'%s'", getValueAsString(Record->GetFieldValue(CutsRecordWriter::EXPECTED_CASES_FIELD).AsDouble(), buffer2).c_str()));
                    } else
                        throw prg_error("Unknown model type (%d).", "CutsRecordWriter()", parameters.getModelType());
            }
            if (parameters.getModelType() != Parameters::SIGNED_RANK) {
                string_values.push_back(printString(buffer, "'%s'", getValueAsString(Record->GetFieldValue(CutsRecordWriter::RELATIVE_RISK_FIELD).AsDouble(), buffer2).c_str()));
                string_values.push_back(printString(buffer, "'%s'", getValueAsString(Record->GetFieldValue(CutsRecordWriter::EXCESS_CASES_FIELD).AsDouble(), buffer2).c_str()));
                if (parameters.getReportAttributableRisk())
                    string_values.push_back(printString(buffer, "'%s'", AttributableRiskAsString(Record->GetFieldValue(CutsRecordWriter::ATTRIBUTABLE_RISK_FIELD).AsDouble(), buffer2).c_str()));
            }
            string_values.push_back(not_applicable);
            if (_scanRunner.reportablePValue(thisCut)) {
                string_values.push_back(not_applicable);
                if (parameters.getIsProspectiveAnalysis()) {
                    string_values.push_back(not_applicable);
                }
            }
            if (parameters.getScanType() != Parameters::TIMEONLY) {
                buffer2 = thisNode.getIdentifier();
                string_values.push_back(printString(buffer, "'%s'", htmlencode(buffer2).c_str()));
                if (_scanRunner.hasNodeDescriptions()) {
                    buffer2 = thisNode.getName();
                    string_values.push_back(printString(buffer, "'%s'", htmlencode(buffer2).c_str()));
                }
                string_values.push_back(not_applicable);
            }
            if (parameters.isSequentialScanTreeOnly()) {
                string_values.push_back(not_applicable);
            }
            typelist_csv_string<std::string>(string_values, buffer2);
            buffer = Record->GetFieldValue(CutsRecordWriter::NODE_ID_FIELD).AsString();
            children_arrays.push_back(printString(buffer, "'tr-%s': [%s]", encodeForJavascript(buffer).c_str(), buffer2.c_str()));
        }
        typelist_csv_string<std::string>(children_arrays, buffer);
        *subrows << "'tr-" << node_tr << "' : {" << buffer << "},";
    }
    return outfile;
}

/** Convert the p-value to class name to be used in html/javascript. */
const char * ResultsFileWriter::getPvalueClass(double pval, bool childClass) {
    if (pval <= 0.001)
        return (childClass ? "pvalue-001-children " : "pvalue-001 ");
    else if (pval <= 0.01)
        return (childClass ? "pvalue-01-children " : "pvalue-01 ");
    else if (pval <= 0.05)
        return (childClass ? "pvalue-05-children " : "pvalue-05 ");
    else
        return (childClass ? "pvalue-less-children " : "pvalue-less ");
}

/** Convert the relative risk to class name to be used in html/javascript. */
const char * ResultsFileWriter::getRelativeRiskClass(double rr, bool childClass) {
    if (rr >= 8)
        return (childClass ? "rr-8-children " : "rr-8 ");
    else if (rr >= 4)
        return (childClass ? "rr-4-children " : "rr-4 ");
    else if (rr >= 2)
        return (childClass ? "rr-2-children " : "rr-2 ");
    else
        return (childClass ? "rr-less-children " : "rr-less ");
}

const char * ResultsFileWriter::getRecurranceIntervalClass(const RecurrenceInterval_t& ri, bool childClass) const {
    if (ri.first >= 100.0)
        return (childClass ? "ri-range4-children " : "ri-range4 ");
    else if (ri.first >= 5.0)
        return (childClass ? "ri-range3-children " : "ri-range3 ");
    else if (ri.first >= 1.0)
        return (childClass ? "ri-range2-children " : "ri-range2 ");
    else if (ri.second >= 100.0)
        return (childClass ? "ri-range1-children " : "ri-range1 ");
    else
        return (childClass ? "ri-less-children " : "ri-less ");
}

/** Returns the RecurrenceInterval_t as a formatted string. */
std::string& ResultsFileWriter::getRecurranceIntervalAsString(const RecurrenceInterval_t& ri, std::string& buffer) const {
    if (ri.first < 1.0)
        return printString(buffer, "%.0lf %s%s", ri.second, "day", (ri.second < 1.5 ? "" : "s"));
    else if (ri.first <= 10.0)
        return printString(buffer, "%.1lf %s%s", ri.first, "year", (ri.first < 1.05 ? "" : "s"));
    else {
        std::string buffer2;
        return printString(buffer, "%s %s", humanize(ri.first, buffer2, 1).c_str(), "years");
    }
}

/** Convert the signal to class name to be used in html/javascript. */
const char * ResultsFileWriter::getSignalClass(double pval, bool childClass) {
    if (pval > 0.0)
        return (childClass ? "signalled-children " : "signalled ");
    else
        return (childClass ? "not-signalled-children " : "not-signalled ");
}

ResultsFileWriter::NodeSet_t ResultsFileWriter::writeJsTreeNode(std::stringstream & outfile, const NodeStructure& node, const std::map<int, const CutStructure*>& cutMap, int collapseAtLevel) {
    const Parameters& parameters = _scanRunner.getParameters();
    bool sequentialTreeOnly = parameters.isSequentialScanTreeOnly();
    bool prospectiveScan = parameters.getIsProspectiveAnalysis();
    bool signedrank = parameters.getModelType() == Parameters::SIGNED_RANK;
    std::stringstream nodestream;
    // Write header section to this nodestream.
    std::string buffer(node.getOutputLabel()), parent;
    printString(buffer, "ID_%d", node.getID());
    nodestream << "{ HTMLid: '" << encodeForJavascript(buffer) << "', innerHTML: \"<ul><li><a data-toggle='tooltip' title='<ul><li class=" << buffer << ">";
    buffer = node.getOutputLabel();
    nodestream << htmlencode(buffer) << "</li></ul>' data-html='true'";
    if (node.getParents().size())
        nodestream << " parent_id=" << encodeForJavascript(printString(parent, "ID_%d", node.getParents().front().first->getID())) << " ";
    nodestream << "> ";
    // Create truncated identifier if longer than 25 characters -- so it can fit in node box outline.
    buffer = node.getOutputLabel();
    if (buffer.size() > 25) {
        buffer.resize(25);
        buffer.resize(27, '.');
    }
    nodestream << htmlencode(buffer) << "</a></li>";
    // Add quick info in node that details the relative risk and p-value if there is a cut.
    BestCutSet_t node_attr_rr(sequentialTreeOnly ? 0.0 : std::numeric_limits<double>::max(), -std::numeric_limits<double>::max(), RecurrenceInterval_t(0.0, 0.0));
    std::map<int, const CutStructure*>::const_iterator itr = cutMap.find(node.getID());
    if (itr != cutMap.end()) {
        if (sequentialTreeOnly) 
            node_attr_rr.get<0>() = static_cast<double>(_scanRunner.getSequentialStatistic().testCutSignaled(static_cast<size_t>(itr->second->getID())));
        else
            node_attr_rr.get<0>() = itr->second->getPValue(_scanRunner);
        // Get relative risk for this node.
        // For signed-rank model, relative risk is not defined, so set to 0.0 for now.
        // There might be some future request to revise this feature for signed-rank model, so
        // I'm not guesss to what might be nor am I going to remove it currently.
        node_attr_rr.get<1>() = signedrank ? 0.0 : itr->second->getRelativeRisk(_scanRunner);
        nodestream << "<li>RR = " << getValueAsString(node_attr_rr.get<1>(), buffer);
        if (sequentialTreeOnly) {
            if (node_attr_rr.get<0>() != 0.0)
                nodestream << ", signalled look " << node_attr_rr.get<0>();
            else
                nodestream << ", not signalled";
        } else {
            nodestream << ", p = ";
            std::string format, replicas;
            printString(replicas, "%u", parameters.getNumReplicationsRequested());
            printString(format, "%%.%dlf", replicas.size());
            printString(buffer, format.c_str(), node_attr_rr.get<0>());
            nodestream << buffer;
        }
        if (prospectiveScan) {
            node_attr_rr.get<2>() = _scanRunner.getRecurrenceInterval(*(itr->second));
            nodestream << ", RI = " << getRecurranceIntervalAsString(node_attr_rr.get<2>(), buffer);
        }
        nodestream << "</li>";
    }
    nodestream << "</ul><div class='nbottom'></div>\"";

    // Is this node significant?
    bool nodesignificant = true;
    if (sequentialTreeOnly)
        nodesignificant = node_attr_rr.get<0>() > 0.0;
    else if (prospectiveScan)
        nodesignificant = node_attr_rr.get<2>().second >= 100.0;
    else
        nodesignificant = node_attr_rr.get<0>() <= 0.05;

    // Create a stream for children nodes, if any.
    BestCutSet_t best_attr_rr_children(sequentialTreeOnly ? 0.0 : std::numeric_limits<double>::max(), -std::numeric_limits<double>::max(), RecurrenceInterval_t(0.0, 0.0));
    std::stringstream childrenstream;
    unsigned int children_count = 0;
    if (node.getChildren().size()) {
        unsigned int significantChildNodes = 0, significantBranches = 0;
        std::vector<boost::shared_ptr<std::stringstream> > childNodestreams;
        for (size_t t = 0; t < node.getChildren().size(); ++t)
            childNodestreams.push_back(boost::shared_ptr<std::stringstream>(new std::stringstream()));
        std::vector<NodeSet_t> childrenNodesets;
        // Iterate over children recursively obtain branch stream and p-value/relative risk by node and best child.
        std::vector<boost::shared_ptr<std::stringstream> >::iterator itrStream = childNodestreams.begin();
        // Copy the children and sort by identifier
        NodeStructure::ChildContainer_t childrenCopy = node.getChildren();
        std::sort(childrenCopy.begin(), childrenCopy.end(), CompareNodeStructureByIdentifier());
        NodeStructure::ChildContainer_t::const_iterator itrChild = childrenCopy.begin();

        for (; itrChild != childrenCopy.end(); ++itrChild, ++itrStream) {
            childrenNodesets.push_back(writeJsTreeNode(*(*itrStream), *(*itrChild), cutMap, collapseAtLevel));
            if (sequentialTreeOnly ? childrenNodesets.back().get<0>().get<0>() > 0.0 : childrenNodesets.back().get<0>().get<0>() <= 0.05) {
                ++significantChildNodes;
                ++significantBranches;
            } 
            if (sequentialTreeOnly ? childrenNodesets.back().get<1>().get<0>() > 0.0 : childrenNodesets.back().get<1>().get<0>() <= 0.05)
                ++significantBranches;
        }
        if (nodesignificant || significantChildNodes > 0 || significantBranches > 0) {
            // There exist significant child nodes and/or significant descendents, so we're including at least some children of this node
            if (static_cast<int>(node.getLevel()) >= collapseAtLevel) childrenstream << ", collapsed : true ";
            childrenstream << ", children: [" << std::endl;
            if (nodesignificant || significantChildNodes > 0) {
                // Rule 1a - Include child nodes if this node is significant - so at least one level below significant nodes is displayed.
                // Rule 1b - Include child node if it or one of its siblings are significant. This means we're including all children if we're including one.
                std::vector<NodeSet_t>::iterator itrNodeSets = childrenNodesets.begin();
                for (itrStream = childNodestreams.begin(); itrStream != childNodestreams.end(); ++itrStream, ++itrNodeSets) {
                    childrenstream << (*itrStream)->str();
                    if ((itrStream + 1) != childNodestreams.end()) childrenstream << ", ";
                    if (sequentialTreeOnly) {
                        best_attr_rr_children.get<0>() = std::max(best_attr_rr_children.get<0>(), itrNodeSets->get<0>().get<0>());
                        best_attr_rr_children.get<0>() = std::max(best_attr_rr_children.get<0>(), itrNodeSets->get<1>().get<0>());
                    } else {
                        best_attr_rr_children.get<0>() = std::min(best_attr_rr_children.get<0>(), itrNodeSets->get<0>().get<0>());
                        best_attr_rr_children.get<0>() = std::min(best_attr_rr_children.get<0>(), itrNodeSets->get<1>().get<0>());
                    }
                    best_attr_rr_children.get<1>() = std::max(best_attr_rr_children.get<1>(), itrNodeSets->get<0>().get<1>());
                    best_attr_rr_children.get<1>() = std::max(best_attr_rr_children.get<1>(), itrNodeSets->get<1>().get<1>());
                    if (prospectiveScan) {
                        best_attr_rr_children.get<2>() = best_attr_rr_children.get<2>().first >= itrNodeSets->get<0>().get<2>().first ? best_attr_rr_children.get<2>() : itrNodeSets->get<0>().get<2>();
                        best_attr_rr_children.get<2>() = best_attr_rr_children.get<2>().first >= itrNodeSets->get<1>().get<2>().first ? best_attr_rr_children.get<2>() : itrNodeSets->get<1>().get<2>();
                    }
                    ++children_count;
                }
            } else {
                // Rule 2 - include child node if it or at least one descendent is significant.
                std::vector<NodeSet_t>::iterator itrNodeSets = childrenNodesets.begin();
                for (itrStream = childNodestreams.begin(); itrNodeSets != childrenNodesets.end(); ++itrNodeSets, ++itrStream) {
                    if (itrNodeSets->get<0>().get<0>() <= 0.05 || itrNodeSets->get<1>().get<0>() <= 0.05) {
                        if (children_count > 0) childrenstream << ",";
                        childrenstream << (*itrStream)->str();
                        if (sequentialTreeOnly) {
                            best_attr_rr_children.get<0>() = std::max(best_attr_rr_children.get<0>(), itrNodeSets->get<0>().get<0>());
                            best_attr_rr_children.get<0>() = std::max(best_attr_rr_children.get<0>(), itrNodeSets->get<1>().get<0>());
                        } else {
                            best_attr_rr_children.get<0>() = std::min(best_attr_rr_children.get<0>(), itrNodeSets->get<0>().get<0>());
                            best_attr_rr_children.get<0>() = std::min(best_attr_rr_children.get<0>(), itrNodeSets->get<1>().get<0>());
                        }
                        best_attr_rr_children.get<1>() = std::max(best_attr_rr_children.get<1>(), itrNodeSets->get<0>().get<1>());
                        best_attr_rr_children.get<1>() = std::max(best_attr_rr_children.get<1>(), itrNodeSets->get<1>().get<1>());
                        if (prospectiveScan) {
                            best_attr_rr_children.get<2>() = best_attr_rr_children.get<2>().first >= itrNodeSets->get<0>().get<2>().first ? best_attr_rr_children.get<2>() : itrNodeSets->get<0>().get<2>();
                            best_attr_rr_children.get<2>() = best_attr_rr_children.get<2>().first >= itrNodeSets->get<1>().get<2>().first ? best_attr_rr_children.get<2>() : itrNodeSets->get<1>().get<2>();
                        }
                        ++children_count;
                    }
                }
            }
            childrenstream << "]" << std::endl;
        }

    }

    // Write the node and children streams to caller's stream.
    outfile << nodestream.str();
    if (children_count > 0) outfile << childrenstream.str();

    // Get the class names for node relative risk and p-value. Do this for best child node as well.
    buffer = getRelativeRiskClass(node_attr_rr.get<1>(), false);
    if (sequentialTreeOnly)
        buffer += getSignalClass(node_attr_rr.get<0>(), false);
    else
        buffer += getPvalueClass(node_attr_rr.get<0>(), false);
    if (_scanRunner.getParameters().getIsProspectiveAnalysis())
        buffer += getRecurranceIntervalClass(node_attr_rr.get<2>(), false);

    buffer += getRelativeRiskClass(best_attr_rr_children.get<1>(), true);
    if (sequentialTreeOnly)
        buffer += getSignalClass(best_attr_rr_children.get<0>(), true);
    else
        buffer += getPvalueClass(best_attr_rr_children.get<0>(), true);
    if (_scanRunner.getParameters().getIsProspectiveAnalysis())
        buffer += getRecurranceIntervalClass(best_attr_rr_children.get<2>(), true);

    outfile << ", HTMLclass: \"" << buffer << "\"" << "}" << std::endl;

    // Return the pairs for node and best child node separately.
    return NodeSet_t(node_attr_rr, best_attr_rr_children);
}