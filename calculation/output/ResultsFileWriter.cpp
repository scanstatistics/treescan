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

std::string& ResultsFileWriter::getFilenameHTML(const Parameters& parameters, std::string& buffer) {
  return getDerivedFilename(parameters.getOutputFileName(), "", ".html", buffer);
}

std::ofstream & ResultsFileWriter::openStream(const std::string& outputfile, std::ofstream & outfile, bool overwrite) {
    std::string buffer;
    BasePrint & print(const_cast<ScanRunner&>(_scanRunner).getPrint());

    outfile.open(outputfile.c_str());
    if (!outfile) {
        print.Printf("Unable to create specified output file: %s\n", BasePrint::P_READERROR, outputfile.c_str());
        FileName currFilename(outputfile.c_str()), documentsfile(outputfile.c_str());
        std::string buffer, temp;
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

bool ResultsFileWriter::writeASCII(time_t start, time_t end) {
    std::ofstream outfile;
    openStream(_scanRunner.getParameters().getOutputFileName(), outfile, true);
    if (!outfile) return false;

    AsciiPrintFormat PrintFormat;
    Parameters& parameters(const_cast<Parameters&>(_scanRunner.getParameters()));
    ScanRunner::Loglikelihood_t calcLogLikelihood(AbstractLoglikelihood::getNewLoglikelihood(parameters, _scanRunner.getTotalC(), _scanRunner.getTotalN(), _scanRunner.isCensoredData()));

    PrintFormat.PrintVersionHeader(outfile);
    std::string buffer = ctime(&start);
    outfile << std::endl << "Program run on: " << buffer << std::endl;
    PrintFormat.PrintNonRightMarginedDataString(outfile, getAnalysisSuccinctStatement(buffer), false);

    PrintFormat.SetMarginsAsSummarySection();
    PrintFormat.PrintSectionSeparatorString(outfile);
    outfile << std::endl << "SUMMARY OF DATA" << std::endl << std::endl;
    if (!parameters.getPerformPowerEvaluations() || 
        !(parameters.getPerformPowerEvaluations() && parameters.getPowerEvaluationType() == Parameters::PE_ONLY_CASEFILE && parameters.getConditionalType() == Parameters::UNCONDITIONAL)) {
        PrintFormat.PrintSectionLabel(outfile, "Total Cases", false);
        PrintFormat.PrintAlignedMarginsDataString(outfile, printString(buffer, "%ld", _scanRunner.getTotalC()));
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
    }
    if (parameters.getModelType() == Parameters::POISSON) {
        PrintFormat.PrintSectionLabel(outfile, "Total Expected", false);
        PrintFormat.PrintAlignedMarginsDataString(outfile, getValueAsString(_scanRunner.getTotalN(), buffer, 1));
    }
    if (parameters.getModelType() == Parameters::BERNOULLI) {
        PrintFormat.PrintSectionLabel(outfile, "Total Observations", false);
        PrintFormat.PrintAlignedMarginsDataString(outfile, printString(buffer, "%ld", static_cast<int>(_scanRunner.getTotalN())));
    }
    if (Parameters::isTemporalScanType(parameters.getScanType())) {
        PrintFormat.PrintSectionLabel(outfile, "Data Time Range", false);
        PrintFormat.PrintAlignedMarginsDataString(outfile, parameters.getDataTimeRangeSet().toString(buffer));
    }
    if (parameters.isApplyingExclusionTimeRanges()) {
        PrintFormat.PrintSectionLabel(outfile, "Excluded Time Ranges", false);
        PrintFormat.PrintAlignedMarginsDataString(outfile, parameters.getExclusionTimeRangeSet().toString(buffer));
    }
    if (parameters.getScanType() != Parameters::TIMEONLY) {
        const TreeStatistics& treestats =  _scanRunner.getTreeStatistics();
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
        for (TreeStatistics::NodesLevel_t::const_iterator itr=treestats._nodes_per_level.begin(); itr != treestats._nodes_per_level.end(); ++itr) {
            stringbuffer << itr->second << (itr->first == treestats._nodes_per_level.size() ? "" : ", ");
        }
        PrintFormat.PrintAlignedMarginsDataString(outfile, stringbuffer.str().c_str());
    }

    PrintFormat.PrintSectionSeparatorString(outfile, 0, 2);

    if (Parameters::isTemporalScanType(parameters.getScanType())) {
        std::string buffer;
        _scanRunner.getCaselessWindowsAsString(buffer);
        if (buffer.size()) {
            outfile << "Warning: The following days in the data time range do not have cases:" << std::endl;
            PrintFormat.PrintNonRightMarginedDataString(outfile, buffer, false);
            PrintFormat.PrintSectionSeparatorString(outfile, 0, 2);
        }
    }

    if (parameters.getSequentialScan() && static_cast<unsigned int>(_scanRunner.getTotalC()) > parameters.getSequentialMaximumSignal()) {
        outfile << "Note: The sequential scan reached or exceeded the specified maximum cases." << std::endl << "      The sequential analysis is over." << std::endl;
    } else if (!parameters.getPerformPowerEvaluations() || (parameters.getPerformPowerEvaluations() && parameters.getPowerEvaluationType() == Parameters::PE_WITH_ANALYSIS)) {
        if (_scanRunner.getCuts().size() == 0 || !_scanRunner.reportableCut(*_scanRunner.getCuts()[0])) {
            outfile << "No cuts were found." << std::endl;
        } else {
            outfile << "MOST LIKELY CUTS"<< std::endl << std::endl;
            std::string format, replicas;
            printString(replicas, "%u", parameters.getNumReplicationsRequested());
            printString(format, "%%.%dlf", replicas.size());

            unsigned int k=0;
            outfile.setf(std::ios::fixed);
            outfile.precision(5);
            while(k < _scanRunner.getCuts().size() && _scanRunner.reportableCut(*_scanRunner.getCuts()[k])) {
                PrintFormat.SetMarginsAsCutSection( k + 1);
                outfile << k + 1 << ")";
                // skip reporting node identifier for time-only scans
                if (parameters.getScanType() != Parameters::TIMEONLY) {
                    PrintFormat.PrintSectionLabel(outfile, "Node Identifier", false);
                    buffer = _scanRunner.getNodes()[_scanRunner.getCuts()[k]->getID()]->getIdentifier();
                    if (_scanRunner.getCuts()[k]->getCutChildren().size()) {
                        buffer += " children: ";
                        const CutStructure::CutChildContainer_t& childNodeIds = _scanRunner.getCuts()[k]->getCutChildren();
                        for (size_t t=0; t < childNodeIds.size(); ++t) {
                            buffer +=  _scanRunner.getNodes()[childNodeIds[t]]->getIdentifier();
                            if (t < childNodeIds.size() - 1)
                                buffer += ", ";
                        }
                    }
                    PrintFormat.PrintAlignedMarginsDataString(outfile, buffer.c_str());

					PrintFormat.PrintSectionLabel(outfile, "Tree Level", true);
					printString(buffer, "%ld", _scanRunner.getNodes()[_scanRunner.getCuts()[k]->getID()]->getLevel());
					PrintFormat.PrintAlignedMarginsDataString(outfile, buffer);
                }

                if (parameters.getConditionalType() == Parameters::NODEANDTIME) {
                    PrintFormat.PrintSectionLabel(outfile, "Node Cases", true);
                    printString(buffer, "%ld", static_cast<int>(_scanRunner.getNodes()[_scanRunner.getCuts()[k]->getID()]->getBrC()));
                    PrintFormat.PrintAlignedMarginsDataString(outfile, buffer);
                    PrintFormat.PrintSectionLabel(outfile, "Time Window", true);
                    printString(buffer, "%ld to %ld", _scanRunner.getCuts()[k]->getStartIdx() - _scanRunner.getZeroTranslationAdditive(), _scanRunner.getCuts()[k]->getEndIdx() - _scanRunner.getZeroTranslationAdditive());
                    PrintFormat.PrintAlignedMarginsDataString(outfile, buffer);
                    PrintFormat.PrintSectionLabel(outfile, "Cases in Window", true);
                } else if (parameters.getModelType() == Parameters::UNIFORM) {
                    // skip reporting node cases for time-only scans
                    if (parameters.getScanType() != Parameters::TIMEONLY) {
                        PrintFormat.PrintSectionLabel(outfile, "Node Cases", true);
                        if (parameters.isPerformingDayOfWeekAdjustment() || _scanRunner.isCensoredData()) {
                            printString(buffer, "%ld", static_cast<int>(_scanRunner.getNodes()[_scanRunner.getCuts()[k]->getID()]->getBrC()));
                        } else {
                            printString(buffer, "%ld", static_cast<int>(_scanRunner.getCuts()[k]->getN()));
                        }
                        PrintFormat.PrintAlignedMarginsDataString(outfile, buffer);
                    }
                    PrintFormat.PrintSectionLabel(outfile, "Time Window", parameters.getScanType() != Parameters::TIMEONLY);
                    printString(buffer, "%ld to %ld", _scanRunner.getCuts()[k]->getStartIdx() - _scanRunner.getZeroTranslationAdditive(), _scanRunner.getCuts()[k]->getEndIdx() - _scanRunner.getZeroTranslationAdditive());
                    PrintFormat.PrintAlignedMarginsDataString(outfile, buffer);
                    PrintFormat.PrintSectionLabel(outfile, "Cases in Window", true);
                } else if (parameters.getModelType() == Parameters::BERNOULLI) {
                    PrintFormat.PrintSectionLabel(outfile, "Observations", true);
                    printString(buffer, "%ld", static_cast<int>(_scanRunner.getCuts()[k]->getN()));
                    PrintFormat.PrintAlignedMarginsDataString(outfile, buffer);
                    PrintFormat.PrintSectionLabel(outfile, "Cases", true);
                } else if (parameters.getModelType() == Parameters::POISSON) {
                    PrintFormat.PrintSectionLabel(outfile, "Observed Cases", true);
                }
                printString(buffer, "%ld", _scanRunner.getCuts()[k]->getC());
                PrintFormat.PrintAlignedMarginsDataString(outfile, buffer);
                PrintFormat.PrintSectionLabel(outfile, parameters.getModelType() == Parameters::UNIFORM ? "Expected Cases" : "Expected", true);
                PrintFormat.PrintAlignedMarginsDataString(outfile, getValueAsString(_scanRunner.getCuts()[k]->getExpected(_scanRunner), buffer));
                PrintFormat.PrintSectionLabel(outfile, "Relative Risk", true);
                PrintFormat.PrintAlignedMarginsDataString(outfile, getValueAsString(_scanRunner.getCuts()[k]->getRelativeRisk(_scanRunner), buffer));
                PrintFormat.PrintSectionLabel(outfile, "Excess Cases", true);
                PrintFormat.PrintAlignedMarginsDataString(outfile, getValueAsString(_scanRunner.getCuts()[k]->getExcessCases(_scanRunner), buffer));
                if (parameters.getReportAttributableRisk()) {
                    PrintFormat.PrintSectionLabel(outfile, "Attributable Risk", true);
                    buffer = _scanRunner.getCuts()[k]->getAttributableRiskAsString(_scanRunner, buffer);
                    PrintFormat.PrintAlignedMarginsDataString(outfile, buffer);
                }
                if ((parameters.getScanType() == Parameters::TREETIME && parameters.getConditionalType() == Parameters::NODEANDTIME) ||
                    (parameters.getScanType() == Parameters::TIMEONLY && parameters.getConditionalType() == Parameters::TOTALCASES && parameters.isPerformingDayOfWeekAdjustment()) ||
                    (parameters.getScanType() == Parameters::TREETIME && parameters.getConditionalType() == Parameters::NODE && parameters.isPerformingDayOfWeekAdjustment())) {
                    // If we stick with Poisson log-likelihood calculation, then label is 'Test Statistic' in place of 'Log Likelihood Ratio', hyper-geometric is 'Log Likelihood Ratio'.
                    PrintFormat.PrintSectionLabel(outfile, "Test Statistic", true);
                } else {
                    PrintFormat.PrintSectionLabel(outfile, "Log Likelihood Ratio", true);
                }
                printString(buffer, "%.6lf", calcLogLikelihood->LogLikelihoodRatio(_scanRunner.getCuts()[k]->getLogLikelihood()));
                PrintFormat.PrintAlignedMarginsDataString(outfile, buffer);
                if (parameters.getNumReplicationsRequested() > 9/*require more than 9 replications to report p-values*/) {
                    PrintFormat.PrintSectionLabel(outfile, "P-value", true);
                    printString(buffer, format.c_str(), (double)_scanRunner.getCuts()[k]->getRank() /(parameters.getNumReplicationsRequested() + 1));
                    PrintFormat.PrintAlignedMarginsDataString(outfile, buffer);
                }

				outfile << std::endl;
                k++;
            }
        }
    }

    // Print critical values if requested.
    if ((parameters.getReportCriticalValues() && parameters.getNumReplicationsRequested() >= 19) || 
        (parameters.getPerformPowerEvaluations() && parameters.getCriticalValuesType() == Parameters::CV_MONTECARLO)) {
        outfile << "A cut is statistically significant when its log likelihood ratio is greater than the critical value, which is, for significance level:" << std::endl;
        if (parameters.getNumReplicationsRequested() >= 99999) {
            CriticalValues::alpha_t alpha00001(_scanRunner.getCriticalValues().getAlpha00001());
            outfile << "... 0.00001: " <<  alpha00001.second << std::endl;
        }
        if (parameters.getNumReplicationsRequested() >= 9999) {
            CriticalValues::alpha_t alpha0001(_scanRunner.getCriticalValues().getAlpha0001());
            outfile << "... 0.0001: " <<  alpha0001.second << std::endl;
        }
        if (parameters.getNumReplicationsRequested() >= 999) {
            CriticalValues::alpha_t alpha001(_scanRunner.getCriticalValues().getAlpha001());
            outfile << "... 0.001: " <<  alpha001.second << std::endl;
        }
        if (parameters.getNumReplicationsRequested() >= 99) {
            CriticalValues::alpha_t alpha01(_scanRunner.getCriticalValues().getAlpha01());
            outfile << "... 0.01: " <<  alpha01.second << std::endl;
        }
        if (parameters.getNumReplicationsRequested() >= 19) {
            CriticalValues::alpha_t alpha05(_scanRunner.getCriticalValues().getAlpha05());
            outfile << "... 0.05: " <<  alpha05.second << std::endl;
        }
        outfile << std::endl;
    }

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

/** Returns a string which is */
std::string & ResultsFileWriter::getAnalysisSuccinctStatement(std::string & buffer) const {
    const Parameters& parameters = _scanRunner.getParameters();
    switch (parameters.getScanType()) {
        case Parameters::TREEONLY : {
            buffer = "Tree Only Scan";
            switch (parameters.getConditionalType()) {
                case Parameters::UNCONDITIONAL : buffer += " with Unconditional"; break;
                case Parameters::TOTALCASES : buffer += " with Conditional"; break;
                case Parameters::NODE : break;
                default: throw prg_error("Unknown conditional type (%d).", "getAnalysisSuccinctStatement()", parameters.getConditionalType());
            }
            switch (parameters.getModelType()) {
                case Parameters::POISSON : buffer += " Poisson Model"; break;
                case Parameters::BERNOULLI : buffer += " Bernoulli Model";
                    if (_scanRunner.getParameters().getSelfControlDesign())
                        buffer += " (self-control design)";
                    break;
                case Parameters::UNIFORM : break;
                default: throw prg_error("Unknown nodel type (%d).", "getAnalysisSuccinctStatement()", parameters.getModelType());
            }
        } break;
        case Parameters::TREETIME : 
            buffer = "Tree Temporal Scan";
            switch (parameters.getConditionalType()) {
                case Parameters::NODE : buffer += " with Conditional Uniform Model"; break;
                case Parameters::NODEANDTIME : buffer += " Conditioned on Node and Time"; break;
                default: throw prg_error("Unknown conditional type (%d).", "getAnalysisSuccinctStatement()", parameters.getConditionalType());
            } break;
        case Parameters::TIMEONLY : 
            buffer = parameters.getSequentialScan() ? "Time Only Sequential Scan" : "Time Only Scan";
            switch (parameters.getConditionalType()) {
                case Parameters::TOTALCASES : buffer += " with Conditional Uniform Model"; break;
                default: throw prg_error("Unknown conditional type (%d).", "getAnalysisSuccinctStatement()", parameters.getConditionalType());
            } break;
            if (parameters.getSequentialScan()) {
                std::string temp;
                buffer += printString(temp, ", %d out of %u cases observed.", _scanRunner.getTotalC(), parameters.getSequentialMaximumSignal());
            }
        default: throw prg_error("Unknown scan type (%d).", "getAnalysisSuccinctStatement()", parameters.getScanType());
    }
    return buffer;
}

std::string & ResultsFileWriter::getTotalRunningTime(time_t start, time_t end, std::string & buffer) const {
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

std::string & ResultsFileWriter::stripNodeIdForHtml(std::string & s) {
    std::string buffer;
    buffer.reserve(s.size());
    for (size_t pos = 0; pos != s.size(); ++pos) {
        switch (s[pos]) {
        case '&':  buffer.append("-amp-"); break;
        case '\"': buffer.append("-quot-"); break;
        case '\'': buffer.append("-apos-"); break;
        case '<':  buffer.append("-lt-"); break;
        case '>':  buffer.append("-gt-"); break;
        case '.':  buffer.append("-period-"); break;
        case '#':  buffer.append("-num-"); break;
        default:   buffer.append(&s[pos], 1); break;
        }
    }
    s.swap(buffer);
    return s;
}

bool ResultsFileWriter::writeHTML(time_t start, time_t end) {
    std::string buffer;
    Parameters& parameters(const_cast<Parameters&>(_scanRunner.getParameters()));
    std::ofstream outfile;
    openStream(getFilenameHTML(parameters, buffer), outfile);
    if (!outfile) return false;
    ScanRunner::Loglikelihood_t calcLogLikelihood(AbstractLoglikelihood::getNewLoglikelihood(parameters, _scanRunner.getTotalC(), _scanRunner.getTotalN(), _scanRunner.isCensoredData()));

    outfile << "<!DOCTYPE html>" << std::endl; 
    outfile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?> " << std::endl;
    outfile << "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\">" << std::endl;
    outfile << "<head>" << std::endl;
    outfile << "<link rel=\"stylesheet\" href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.1.1/css/bootstrap.min.css\">" << std::endl;
    outfile << "<link rel=\"stylesheet\" href=\"https://cdn.datatables.net/1.10.16/css/jquery.dataTables.min.css\">" << std::endl;
    outfile << "<link rel=\"stylesheet\" href=\"http://tools.imsweb.com/~hostovic/treescan/tree-visualization/treescan-results.css\">" << std::endl;
    outfile << "<link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css\">" << std::endl;
    outfile << "</head>" << std::endl;
    if (parameters.getScanType() != Parameters::TIMEONLY) {
        outfile << "<script src=\"http://tools.imsweb.com/~hostovic/treescan/tree-visualization/raphael.js\" type=\"text/javascript\"></script>" << std::endl;
        outfile << "<script src=\"http://tools.imsweb.com/~hostovic/treescan/tree-visualization/Treant.js\" type=\"text/javascript\"></script>" << std::endl;
    }
    outfile << "<script src=\"https://code.jquery.com/jquery-3.3.1.js\" type=\"text/javascript\"></script>" << std::endl;
    outfile << "<script src=\"https://cdn.datatables.net/1.10.16/js/jquery.dataTables.min.js\" type=\"text/javascript\"></script>" << std::endl;
    outfile << "<script src=\"https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.12.9/umd/popper.min.js\" type=\"text/javascript\"></script>" << std::endl;
    outfile << "<script src=\"https://stackpath.bootstrapcdn.com/bootstrap/4.1.1/js/bootstrap.min.js\" type=\"text/javascript\"></script>" << std::endl;

    if (parameters.getScanType() != Parameters::TIMEONLY) {
        outfile << "<script type=\"text/javascript\" charset=\"utf-8\">" << std::endl;
        outfile << "var chart_config = { chart: { container: \"#treescan-tree-visualization\", levelSeparation: 20, siblingSeparation: 15, subTeeSeparation: 15, rootOrientation: \"WEST\", ";
        outfile << "hideRootNode: " << (_scanRunner.getRootNodes().size() > 1 ? "true" : "false") << ", " << std::endl;
        outfile << "node: { HTMLclass: \"treescan-node-tree\", drawLineThrough: false, collapsable: true }, connectors: { type: \"bCurve\"} }, ";
        outfile << "nodeStructure: ";

        // create a map of NodeID to Cut objects
        std::map<int, const CutStructure*> node_cut_map;
        ScanRunner::CutStructureContainer_t::const_iterator itr = _scanRunner.getCuts().begin(), enditr = _scanRunner.getCuts().end();
        for (; itr != enditr; ++itr) {
            node_cut_map[(*itr)->getID()] = (*itr);
        }

        if (_scanRunner.getRootNodes().size() > 1)
            outfile << " { text:{name:\"Root\"}, children: [" << std::endl;

        for (NodeStructure::RelationContainer_t::const_iterator itr = _scanRunner.getRootNodes().begin(); itr != _scanRunner.getRootNodes().end(); ++itr) {
            writeJsTreeNode(outfile, *(*itr), node_cut_map, 2);
            if (itr + 1 != _scanRunner.getRootNodes().end()) outfile << ",";
        }

        if (_scanRunner.getRootNodes().size() > 1)
            outfile << "] }" << std::endl;

        outfile << "};</script>" << std::endl;
    }
    outfile << "<script src=\"http://tools.imsweb.com/~hostovic/treescan/tree-visualization/treescan-results.js\" type=\"text/javascript\"></script>" << std::endl;
    outfile << "<body>" << std::endl;
    buffer = AppToolkit::getToolkit().GetWebSite();
    outfile << "<table width=\"100%\" border=\"0\" cellpadding=\"2\" cellspacing=\"0\" bgcolor=\"#F8FAFA\" style=\"border-collapse: collapse;\">";
    outfile << "<tbody><tr style=\"background-image:url('" << buffer << "images/bannerbg.jpg'); background-repeat:repeat-x;\">";
    outfile << "<td width=\"130\" height=\"125\" align=\"center\" style=\"background: url(" << buffer << "images/TreeScan_logo.png) no-repeat center;\"></td>";
    outfile << "<td align=\"center\"><img height=\"120\" style=\"margin-left:-20px;\" src=\"" << buffer << "images/banner.jpg\" alt=\"TreeScan&trade; - Software for the spatial, temporal, and space-time scan statistics\" title=\"TreeScan&trade; - Software for the spatial, temporal, and space-time scan statistics\"></td></tr></tbody></table>";

    outfile << "<div class=\"hr\"></div><div class=\"program-info\">" << std::endl;
    outfile << getAnalysisSuccinctStatement(buffer);
    outfile << "<table style=\"text-align: left;\"><tbody>" << std::endl;
    if (!parameters.getPerformPowerEvaluations() || 
        !(parameters.getPerformPowerEvaluations() && parameters.getPowerEvaluationType() == Parameters::PE_ONLY_CASEFILE && parameters.getConditionalType() == Parameters::UNCONDITIONAL)) {
        outfile << "<tr><th>Total Cases:</th><td>" << _scanRunner.getTotalC() << "</td></tr>" << std::endl;
        if (_scanRunner.isCensoredData()) {
            outfile << "<tr><th>Total Censored Cases:</th><td>" << _scanRunner.getNumCensoredCases() << "</td></tr>" << std::endl;
            outfile << "<tr><th>Average Censoring Time:</th><td>" << _scanRunner.getAvgCensorTime() << "</td></tr>" << std::endl;
        }
    }
    if (parameters.isApplyingExclusionTimeRanges()) {
        outfile << "<tr><th>Total Cases Excluded:</th><td>" << _scanRunner.getNumExcludedCases() << "</td></tr>" << std::endl;
    }
    if (parameters.getModelType() == Parameters::POISSON) {
        outfile << "<tr><th>Total Expected:</th><td>" << getValueAsString(_scanRunner.getTotalN(), buffer, 1).c_str() << "</td></tr>" << std::endl;
    }
    if (parameters.getModelType() == Parameters::BERNOULLI) {
        outfile << "<tr><th>Total Observations:</th><td>" << static_cast<int>(_scanRunner.getTotalN()) << "</td></tr>" << std::endl;
    }
    if (Parameters::isTemporalScanType(parameters.getScanType())) {
        outfile << "<tr><th>Data Time Range:</th><td>" << parameters.getDataTimeRangeSet().toString(buffer).c_str() << "</td></tr>" << std::endl;
    }
    if (Parameters::isTemporalScanType(parameters.getScanType())) {
        outfile << "<tr><th>Excluded Time Ranges:</th><td>" << parameters.getExclusionTimeRangeSet().toString(buffer).c_str() << "</td></tr>" << std::endl;
    }
    if (parameters.getScanType() != Parameters::TIMEONLY) {
        const TreeStatistics& treestats =  _scanRunner.getTreeStatistics();
        outfile << "<tr><th>Number of Nodes:</th><td>" << treestats._num_nodes << "</td></tr>" << std::endl;
        outfile << "<tr><th>Number of Root Nodes:</th><td>" << treestats._num_root << "</td></tr>" << std::endl;
        outfile << "<tr><th>Number of Nodes with Children:</th><td>" << treestats._num_parent << "</td></tr>" << std::endl;
        outfile << "<tr><th>Number of Leaf Nodes:</th><td>" << treestats._num_leaf << "</td></tr>" << std::endl;
        outfile << "<tr><th>Number of Levels in Tree:</th><td>" << treestats._nodes_per_level.size() << "</td></tr>" << std::endl;
        std::stringstream stringbuffer;
        for (TreeStatistics::NodesLevel_t::const_iterator itr=treestats._nodes_per_level.begin(); itr != treestats._nodes_per_level.end(); ++itr) {
            stringbuffer << itr->second << (itr->first == treestats._nodes_per_level.size() ? "" : ", ");
        }
        outfile << "<tr><th>Nodes per Levels:</th><td>" << stringbuffer.str().c_str() << "</td></tr>" << std::endl;
    }
    outfile << "</tbody></table></div>" << std::endl;

    outfile << "<div class=\"hr\"></div>" << std::endl;
    if (parameters.getSequentialScan() && static_cast<unsigned int>(_scanRunner.getTotalC()) > parameters.getSequentialMaximumSignal()) {
        outfile << "<div class=\"warning\">Note: The sequential scan reached or exceeded the specified maximum cases. The sequential analysis is over.</div><div class=\"hr\"></div>";
    } else if (!parameters.getPerformPowerEvaluations() || (parameters.getPerformPowerEvaluations() && parameters.getPowerEvaluationType() == Parameters::PE_WITH_ANALYSIS)) {
        outfile << "<div id=\"cuts\">" << std::endl;
        if (_scanRunner.getCuts().size() == 0 || !_scanRunner.reportableCut(*_scanRunner.getCuts()[0])) {
            outfile << "<h3>No cuts were found.</h3>" << std::endl;
        } else {
            outfile << "<h3>MOST LIKELY CUTS</h3><div><table id=\"id_cuts\" class=\"display\" style=\"width:100%\">" << std::endl;
            outfile << "<thead><tr><th>No.</th>";
            // skip reporting node identifier for time-only scans
            if (parameters.getScanType() != Parameters::TIMEONLY) {
                outfile << "<th>Node Identifier</th><th>Tree Level</th>";
            }
            if (Parameters::isTemporalScanType(parameters.getScanType())) {
               // skip reporting node cases for time-only scans
                if (parameters.getScanType() != Parameters::TIMEONLY) {
                    outfile << "<th>Node Cases</th>";
                }
                outfile << "<th>Time Window</th>";
                outfile << "<th>Cases in Window</th>";
            } else if (parameters.getModelType() == Parameters::BERNOULLI) {
                outfile << "<th>Observations</th>";
                outfile << "<th>Cases</th>";
            } else if (parameters.getModelType() == Parameters::POISSON) {
                outfile << "<th>Observed Cases</th>";
            }
            outfile << "<th>" << (parameters.getModelType() == Parameters::UNIFORM ? "Expected Cases" : "Expected") << "</th>";
            outfile << "<th>Relative Risk</th><th>Excess Cases</th>" << std::endl;
            if (parameters.getReportAttributableRisk()) {
                outfile << "<th>Attributable Risk</th>" << std::endl;
            }
            if ((parameters.getScanType() == Parameters::TREETIME && parameters.getConditionalType() == Parameters::NODEANDTIME) ||
                (parameters.getScanType() == Parameters::TIMEONLY && parameters.getConditionalType() == Parameters::TOTALCASES && parameters.isPerformingDayOfWeekAdjustment()) ||
                (parameters.getScanType() == Parameters::TREETIME && parameters.getConditionalType() == Parameters::NODE && parameters.isPerformingDayOfWeekAdjustment())) {
                // If we stick with Poisson log-likelihood calculation, then label is 'Test Statistic' in place of 'Log Likelihood Ratio', hyper-geometric is 'Log Likelihood Ratio'.
                outfile << "<th>Test Statistic</th>" << std::endl;
            } else {
                outfile << "<th>Log Likelihood Ratio</th>" << std::endl;
            }
            if (parameters.getNumReplicationsRequested() > 9/*require more than 9 replications to report p-values*/) {
                outfile << "<th>P-value</th>";
            }
            outfile << "</tr></thead><tbody>" << std::endl;
            std::string format, replicas;
            printString(replicas, "%u", parameters.getNumReplicationsRequested());
            printString(format, "%%.%dlf", replicas.size());

            unsigned int k=0;
            outfile.setf(std::ios::fixed);
            outfile.precision(5);
            while (k < _scanRunner.getCuts().size() && _scanRunner.reportableCut(*_scanRunner.getCuts()[k])) {
                buffer = _scanRunner.getNodes()[_scanRunner.getCuts()[k]->getID()]->getIdentifier();
                outfile << "<tr id=\"tr-" << stripNodeIdForHtml(buffer) << "\"><td>" << k + 1 << "</td>";
                // skip reporting node identifier for time-only scans
                if (parameters.getScanType() != Parameters::TIMEONLY) {
                    buffer = _scanRunner.getNodes()[_scanRunner.getCuts()[k]->getID()]->getIdentifier();
                    outfile  << "<td>" << htmlencode(buffer);
                    if (_scanRunner.getCuts()[k]->getCutChildren().size()) {
                        outfile  << " children: ";
                        const CutStructure::CutChildContainer_t& childNodeIds = _scanRunner.getCuts()[k]->getCutChildren();
                        for (size_t t=0; t < childNodeIds.size(); ++t) {
                            buffer = _scanRunner.getNodes()[childNodeIds[t]]->getIdentifier();
                            outfile << htmlencode(buffer) << ((t < childNodeIds.size() - 1) ? ", " : "");
                        }
                    }
                    outfile << "</td>";
                    outfile << "<td>" << printString(buffer, "%ld", static_cast<int>(_scanRunner.getNodes()[_scanRunner.getCuts()[k]->getID()]->getLevel())).c_str() << "</td>";
                }
                if (parameters.getScanType() == Parameters::TREETIME && parameters.getConditionalType() == Parameters::NODEANDTIME) {
                    // write node cases and time window
                    printString(buffer, "%ld", static_cast<int>(_scanRunner.getNodes()[_scanRunner.getCuts()[k]->getID()]->getBrC()));
                    outfile << "<td>" << buffer.c_str() << "</td>";
                    outfile << "<td>" << (_scanRunner.getCuts()[k]->getStartIdx() - _scanRunner.getZeroTranslationAdditive()) << " to " << (_scanRunner.getCuts()[k]->getEndIdx() - _scanRunner.getZeroTranslationAdditive()) << "</td>";
                } else if (parameters.getModelType() == Parameters::UNIFORM) {
                    // write node cases
                    // skip reporting node cases for time-only scans
                    if (parameters.getScanType() != Parameters::TIMEONLY) {
                        if (parameters.isPerformingDayOfWeekAdjustment() && _scanRunner.isCensoredData()) {
                            printString(buffer, "%ld", static_cast<int>(_scanRunner.getNodes()[_scanRunner.getCuts()[k]->getID()]->getBrC()));
                        } else {
                            printString(buffer, "%ld", static_cast<int>(_scanRunner.getCuts()[k]->getN()));
                        }
                        outfile << "<td>" << buffer.c_str() << "</td>";
                    }
                    // write time window
                    outfile << "<td>" << (_scanRunner.getCuts()[k]->getStartIdx() - _scanRunner.getZeroTranslationAdditive()) << " to " << (_scanRunner.getCuts()[k]->getEndIdx() - _scanRunner.getZeroTranslationAdditive()) << "</td>";
                } else if (parameters.getModelType() == Parameters::BERNOULLI) {
                    // write number of observations
                    outfile << "<td>" << static_cast<int>(_scanRunner.getCuts()[k]->getN()) << "</td>";
                }
                // write cases in window or cases or observed cases, depending on settings
                outfile << "<td>" << _scanRunner.getCuts()[k]->getC() << "</td>";
                outfile << "<td>" << getValueAsString(_scanRunner.getCuts()[k]->getExpected(_scanRunner), buffer) << "</td>";
                outfile << "<td>" << getValueAsString(_scanRunner.getCuts()[k]->getRelativeRisk(_scanRunner), buffer) << "</td>";
                outfile << "<td>" << getValueAsString(_scanRunner.getCuts()[k]->getExcessCases(_scanRunner), buffer) << "</td>";
                if (parameters.getReportAttributableRisk()) {
                     buffer = _scanRunner.getCuts()[k]->getAttributableRiskAsString(_scanRunner, buffer);
                    outfile << "<td>" << buffer << "</td>";
                }
                outfile << "<td>" << printString(buffer, "%.6lf", calcLogLikelihood->LogLikelihoodRatio(_scanRunner.getCuts()[k]->getLogLikelihood())).c_str() << "</td>";
                // write p-value
                if (parameters.getNumReplicationsRequested() > 9/*require more than 9 replications to report p-values*/) {
                    outfile << "<td>" << printString(buffer, format.c_str(), (double)_scanRunner.getCuts()[k]->getRank() /(parameters.getNumReplicationsRequested() + 1)) << "</td>";
                }
                outfile << "</tr>" << std::endl;
                k++;
            }
            outfile << "</tbody></table></div></div>" << std::endl;
        }
    }

    if (parameters.getScanType() != Parameters::TIMEONLY) {
        outfile << "<a class=\"btn btn-primary btn-sm\" id=\"show_tree\" data-toggle=\"collapse\" href=\"#collapseExample\" role=\"button\" aria-expanded=\"false\" aria-controls=\"collapseExample\">Tree Visualization</a>" << std::endl;
        outfile << "<div class=\"collapse\" id=\"collapseExample\"><h3>Visualization of Analysis Tree  <span id=\"loading_graph\">... loading <i class=\"fa fa-circle-o-notch fa-spin\"></i></span><span id=\"fail_message\"></h3>";
        outfile << "<div class=\"row\">" << std::endl;

        outfile << "<div class=\"col-2\"><div class=\"custom-control custom-radio\">" << std::endl;
        outfile << "<input type=\"radio\" class=\"custom-control-input\" id=\"customControlValidation2\" name=\"radio-stacked\" legend=\"legend-p-value\" required checked=checked>" << std::endl;
        outfile << "<label class=\"custom-control-label\" for=\"customControlValidation2\">Color Nodes by P-Value</label></div>" << std::endl;
        outfile << "<div class=\"custom-control custom-radio mb-3\">" << std::endl;
        outfile << "<input type=\"radio\" class=\"custom-control-input\" id=\"customControlValidation3\" name=\"radio-stacked\" legend=\"legend-relative-risk\" required>" << std::endl;
        outfile << "<label class=\"custom-control-label\" for=\"customControlValidation3\">Color Nodes by Relative Risk</label></div></div>" << std::endl;

        outfile << "<div class=\"col-10\"><div class='chart-legend legend-p-value'><div class='legend-title'>P-Value Legend</div><div class='legend-scale'>" << std::endl;
        outfile << "<ul class='legend-labels'><li><span style='background:#566573;'></span>&gt; 0.05</li><li><span style='background:#DBD51B;'></span>0.05</li>";
        outfile << "<li><span style='background:#FFC300;'></span>0.01</li><li><span style='background:#FF5733;'></span>0.001</li></ul></div></div>" << std::endl;
        outfile << "<div class='chart-legend legend-relative-risk'><div class='legend-title'>Relative Risk Legend</div><div class='legend-scale'>";
        outfile << "<ul class='legend-labels'><li><span style='background:#566573;'></span>&le; 2</li><li><span style='background:#DBD51B;'></span>2</li>";
        outfile << "<li><span style='background:#FFC300;'></span>4</li><li><span style='background:#FF5733;'></span>&ge; 8</li></ul></div></div></div>" << std::endl;
        outfile << "<p style=\"font-style:italic; padding-left:15px; \">Selecting the circle in the upper right corner of each node will expand/collapse children under node. The color of circle indicates maximum p-value / relative risk found in children nodes.</p></div>" << std::endl;
        outfile << "<div class=\"chart\" id=\"treescan-tree-visualization\" style=\"background-color: #EAEDED; border: 2px solid #566573; border-radius: 5px; padding:2px;\"> </div></div>" << std::endl;
    }

    outfile << "<div class=\"hr\"></div>" << std::endl;
    if (Parameters::isTemporalScanType(parameters.getScanType())) {
        std::string buffer;
        _scanRunner.getCaselessWindowsAsString(buffer);
        if (buffer.size()) {
            outfile << "<div class=\"warning\">Warning: The following days in the data time range do not have cases: " << buffer.c_str() << "</div><div class=\"hr\"></div>";
        }
    }

    // Print critical values if requested.
    if ((parameters.getReportCriticalValues() && parameters.getNumReplicationsRequested() >= 19) || (parameters.getPerformPowerEvaluations() && parameters.getCriticalValuesType() == Parameters::CV_MONTECARLO)) {
        outfile << "<div class=\"program-info\">" << std::endl;
        outfile << std::endl << "<div>A cut is statistically significant when its log likelihood ratio is greater than the critical value, which is, for significance level:</div>" << std::endl;
        if (parameters.getNumReplicationsRequested() >= 99999) {
            CriticalValues::alpha_t alpha00001(_scanRunner.getCriticalValues().getAlpha00001());
            outfile << "<div>... 0.00001: " <<  alpha00001.second << "</div>" << std::endl;
        }
        if (parameters.getNumReplicationsRequested() >= 9999) {
            CriticalValues::alpha_t alpha0001(_scanRunner.getCriticalValues().getAlpha0001());
            outfile << "<div>... 0.0001: " <<  alpha0001.second << "</div>"<< std::endl;
        }
        if (parameters.getNumReplicationsRequested() >= 999) {
            CriticalValues::alpha_t alpha001(_scanRunner.getCriticalValues().getAlpha001());
            outfile << "<div>... 0.001: " <<  alpha001.second << "</div>" << std::endl;
        }
        if (parameters.getNumReplicationsRequested() >= 99) {
            CriticalValues::alpha_t alpha01(_scanRunner.getCriticalValues().getAlpha01());
            outfile << "<div>... 0.01: " <<  alpha01.second << "</div>" << std::endl;
        }
        if (parameters.getNumReplicationsRequested() >= 19) {
            CriticalValues::alpha_t alpha05(_scanRunner.getCriticalValues().getAlpha05());
            outfile << "<div>... 0.05: " <<  alpha05.second << "</div>" << std::endl;
        }
        outfile << "</div>" << std::endl;
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
    outfile << "</tbody></table></div>" << std::endl;

    outfile << "</body></html>" << std::endl;
    outfile.close();

    return true;
}

/* Convert the p-value to class name to be used in html/javascript. */
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

/* Convert the relative risk to class name to be used in html/javascript. */
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

ResultsFileWriter::BestCutSet_t ResultsFileWriter::writeJsTreeNode(std::ofstream & outfile, const NodeStructure& node, const std::map<int, const CutStructure*>& cutMap, int collapseAtLevel) {
    std::string buffer;

    buffer = node.getIdentifier();
    outfile << "{ HTMLid: '" << stripNodeIdForHtml(buffer) << "', innerHTML: \"<ul><li><a data-toggle='tooltip' title='<ul><li class=" << buffer << ">";
    buffer = node.getIdentifier();
    outfile << htmlencode(buffer) << "</li></ul>' data-html='true'>";
    // Truncate identifier if longer than 25 characters -- so it can fit in node box outline.
    buffer = node.getIdentifier();
    if (buffer.size() > 25) {
        buffer.resize(25);
        buffer.resize(27, '.');
    }
    outfile << htmlencode(buffer) << "</a></li>";

    BestCutSet_t best_pval_rr(std::numeric_limits<double>::max(), -std::numeric_limits<double>::max());
    std::map<int, const CutStructure*>::const_iterator itr = cutMap.find(node.getID());
    if (itr != cutMap.end()) {
        best_pval_rr.get<0>() = (double)itr->second->getRank() / (_scanRunner.getParameters().getNumReplicationsRequested() + 1);
        best_pval_rr.get<1>() = itr->second->getRelativeRisk(_scanRunner);
        outfile << "<li>Relative Risk: " << getValueAsString(best_pval_rr.get<1>(), buffer) << "</li><li>P-Value: ";
        std::string format, replicas;
        printString(replicas, "%u", _scanRunner.getParameters().getNumReplicationsRequested());
        printString(format, "%%.%dlf", replicas.size());
        printString(buffer, format.c_str(), best_pval_rr.get<0>());
        outfile << buffer << "</li>";
    }
    outfile << "</ul><div class='nbottom'></div>\"";

    BestCutSet_t best_pval_rr_children(std::numeric_limits<double>::max(), -std::numeric_limits<double>::max());
    if (node.getChildren().size()) {
        if (node.getLevel() >= collapseAtLevel) outfile << ", collapsed : true ";
        outfile << ", children: [" << std::endl;
        for (NodeStructure::RelationContainer_t::const_iterator itr = node.getChildren().begin(); itr != node.getChildren().end(); ++itr) {
            BestCutSet_t test = writeJsTreeNode(outfile, *(*itr), cutMap, collapseAtLevel);
            best_pval_rr_children.get<0>() = std::min(best_pval_rr_children.get<0>(), test.get<0>());
            best_pval_rr_children.get<1>() = std::max(best_pval_rr_children.get<1>(), test.get<1>());
            if (itr + 1 != node.getChildren().end()) outfile << ",";
        }
        outfile << "]" << std::endl;
    }

    // Get the class names for node relative risk and p-value. Do this for best child node as well.
    buffer = getRelativeRiskClass(best_pval_rr.get<1>(), false);
    buffer += getPvalueClass(best_pval_rr.get<0>(), false);
    buffer += getRelativeRiskClass(best_pval_rr_children.get<1>(), true);
    buffer += getPvalueClass(best_pval_rr_children.get<0>(), true);
    outfile << ", HTMLclass: \"" << buffer << "\"" << "}" << std::endl;

    // Now take the best relative risk and p-value to pass back to calling parent.
    best_pval_rr.get<0>() = std::min(best_pval_rr_children.get<0>(), best_pval_rr.get<0>());
    best_pval_rr.get<1>() = std::max(best_pval_rr_children.get<1>(), best_pval_rr.get<1>());
    return best_pval_rr;
}