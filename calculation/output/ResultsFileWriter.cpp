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
    ScanRunner::Loglikelihood_t calcLogLikelihood(AbstractLoglikelihood::getNewLoglikelihood(parameters, _scanRunner.getTotalC(), _scanRunner.getTotalN()));

    PrintFormat.PrintVersionHeader(outfile);
    std::string buffer = ctime(&start);
    outfile << std::endl << "Program run on: " << buffer << std::endl;
    PrintFormat.PrintNonRightMarginedDataString(outfile, getAnalysisSuccinctStatement(buffer), false);

    PrintFormat.SetMarginsAsSummarySection();
    PrintFormat.PrintSectionSeparatorString(outfile);
    outfile << std::endl << "SUMMARY OF DATA" << std::endl << std::endl;
    PrintFormat.PrintSectionLabel(outfile, "Total Cases:", false);
    PrintFormat.PrintAlignedMarginsDataString(outfile, printString(buffer, "%ld", _scanRunner.getTotalC()));
    if (parameters.getModelType() == Parameters::POISSON) {
        PrintFormat.PrintSectionLabel(outfile, "Total Expected:", false);
        PrintFormat.PrintAlignedMarginsDataString(outfile, getValueAsString(_scanRunner.getTotalN(), buffer, 1));
    }
    if (parameters.getModelType() == Parameters::BERNOULLI) {
        PrintFormat.PrintSectionLabel(outfile, "Total Observations:", false);
        PrintFormat.PrintAlignedMarginsDataString(outfile, printString(buffer, "%ld", static_cast<int>(_scanRunner.getTotalN())));
    }
    if (parameters.getModelType() == Parameters::TEMPORALSCAN) {
        PrintFormat.PrintSectionLabel(outfile, "Data Time Range:", false);
        PrintFormat.PrintAlignedMarginsDataString(outfile, parameters.getDataTimeRangeSet().toString(buffer));
    }
    PrintFormat.PrintSectionSeparatorString(outfile, 0, 2);

    if (parameters.getModelType() == Parameters::TEMPORALSCAN) {
        std::string buffer;
        _scanRunner.getCaselessWindowsAsString(buffer);
        if (buffer.size()) {
            outfile << "Warning: The following days in the data time range do not have cases:" << std::endl;
            PrintFormat.PrintNonRightMarginedDataString(outfile, buffer, false);
            PrintFormat.PrintSectionSeparatorString(outfile, 0, 2);
        }
    }

    // write detected cuts
    //if (_parameters.isDuplicates()) outfile << "#CasesWithoutDuplicates ";
    //outfile << "#Exp O/E ";
    //if (_parameters.isDuplicates()) outfile << "O/EWithoutDuplicates ";
    //outfile << "LLR pvalue" << std::endl;

    if (!_scanRunner.getParameters().getPerformPowerEvaluations() || (_scanRunner.getParameters().getPerformPowerEvaluations() && _scanRunner.getParameters().getPowerEvaluationType() == Parameters::PE_WITH_ANALYSIS)) {
        if (_scanRunner.getCuts().size() == 0 || _scanRunner.getCuts().at(0)->getC() == 0 || 
            (parameters.getNumReplicationsRequested() > 0 && _scanRunner.getCuts().at(0)->getRank() > parameters.getNumReplicationsRequested())) {
            outfile << "No cuts were found." << std::endl;
        } else {
            outfile << "MOST LIKELY CUTS"<< std::endl << std::endl;
            std::string format, replicas;
            printString(replicas, "%u", parameters.getNumReplicationsRequested());
            printString(format, "%%.%dlf", replicas.size());

            unsigned int k=0;
            outfile.setf(std::ios::fixed);
            outfile.precision(5);
            while(k < _scanRunner.getCuts().size() && _scanRunner.getCuts().at(k)->getC() > 0 && 
                  (parameters.getNumReplicationsRequested() == 0 || _scanRunner.getCuts().at(k)->getRank() < parameters.getNumReplicationsRequested() + 1)) {
                PrintFormat.SetMarginsAsCutSection( k + 1);
                outfile << k + 1 << ")";
                PrintFormat.PrintSectionLabel(outfile, "Node Identifier", false);
                PrintFormat.PrintAlignedMarginsDataString(outfile, _scanRunner.getNodes().at(_scanRunner.getCuts().at(k)->getID())->getIdentifier());
                if (parameters.getModelType() == Parameters::TEMPORALSCAN) {
                    PrintFormat.PrintSectionLabel(outfile, "Node Cases", true);
                    printString(buffer, "%ld", static_cast<int>(_scanRunner.getCuts().at(k)->getN()));
                    PrintFormat.PrintAlignedMarginsDataString(outfile, buffer);
                    PrintFormat.PrintSectionLabel(outfile, "Time Window", true);
                    printString(buffer, "%ld to %ld", _scanRunner.getCuts().at(k)->getStartIdx() - _scanRunner.getZeroTranslationAdditive(), _scanRunner.getCuts().at(k)->getEndIdx() - _scanRunner.getZeroTranslationAdditive());
                    PrintFormat.PrintAlignedMarginsDataString(outfile, buffer);
                    PrintFormat.PrintSectionLabel(outfile, "Cases in Window", true);
                } else if (parameters.getModelType() == Parameters::BERNOULLI) {
                    PrintFormat.PrintSectionLabel(outfile, "Observations", true);
                    printString(buffer, "%ld", static_cast<int>(_scanRunner.getCuts().at(k)->getN()));
                    PrintFormat.PrintAlignedMarginsDataString(outfile, buffer);
                    PrintFormat.PrintSectionLabel(outfile, "Cases", true);
                } else if (parameters.getModelType() == Parameters::POISSON) {
                    PrintFormat.PrintSectionLabel(outfile, "Observed Cases", true);
                }
                printString(buffer, "%ld", _scanRunner.getCuts().at(k)->getC());
                PrintFormat.PrintAlignedMarginsDataString(outfile, buffer);
                if (parameters.isDuplicates()) {
                    PrintFormat.PrintSectionLabel(outfile, "Cases (Duplicates Removed)", true);
                    printString(buffer, "%ld", _scanRunner.getCuts().at(k)->getC() - _scanRunner.getNodes().at(_scanRunner.getCuts().at(k)->getID())->getDuplicates());
                    PrintFormat.PrintAlignedMarginsDataString(outfile, buffer);
                }
                PrintFormat.PrintSectionLabel(outfile, parameters.getModelType() == Parameters::TEMPORALSCAN ? "Expected Cases" : "Expected", true);
                PrintFormat.PrintAlignedMarginsDataString(outfile, getValueAsString(_scanRunner.getCuts().at(k)->getExpected(_scanRunner), buffer));
                PrintFormat.PrintSectionLabel(outfile, "Observed/Expected", true);
                PrintFormat.PrintAlignedMarginsDataString(outfile, getValueAsString(_scanRunner.getCuts().at(k)->getODE(_scanRunner), buffer));
                if (parameters.isDuplicates()) {
                    PrintFormat.PrintSectionLabel(outfile, "O/E (Duplicates Removed)", true);
                    PrintFormat.PrintAlignedMarginsDataString(outfile, getValueAsString((_scanRunner.getCuts().at(k)->getC() - _scanRunner.getNodes().at(_scanRunner.getCuts().at(k)->getID())->getDuplicates())/_scanRunner.getCuts().at(k)->getExpected(_scanRunner), buffer));
                }
                PrintFormat.PrintSectionLabel(outfile, "Relative Risk", true);
                PrintFormat.PrintAlignedMarginsDataString(outfile, getValueAsString(_scanRunner.getCuts().at(k)->getRelativeRisk(_scanRunner), buffer));
                PrintFormat.PrintSectionLabel(outfile, "Excess Cases", true);
                PrintFormat.PrintAlignedMarginsDataString(outfile, getValueAsString(_scanRunner.getCuts().at(k)->getExcessCases(_scanRunner), buffer));
                PrintFormat.PrintSectionLabel(outfile, "Log Likelihood Ratio", true);
                printString(buffer, "%.1lf", calcLogLikelihood->LogLikelihoodRatio(_scanRunner.getCuts().at(k)->getLogLikelihood()));
                PrintFormat.PrintAlignedMarginsDataString(outfile, buffer);
                PrintFormat.PrintSectionLabel(outfile, "P-value", true);
                printString(buffer, format.c_str(), (double)_scanRunner.getCuts().at(k)->getRank() /(parameters.getNumReplicationsRequested() + 1));
                PrintFormat.PrintAlignedMarginsDataString(outfile, buffer, 2);
                k++;
            }
        }
    }

    // Print critical values if requested.
    if ((_scanRunner.getParameters().getReportCriticalValues() && _scanRunner.getParameters().getNumReplicationsRequested() >= 19) || 
        (_scanRunner.getParameters().getPerformPowerEvaluations() && _scanRunner.getParameters().getCriticalValuesType() == Parameters::CV_MONTECARLO)) {
        outfile << "A cut is statistically significant when its log likelihood ratio is greater than the critical value, which is, for significance level:" << std::endl;
        if (_scanRunner.getParameters().getNumReplicationsRequested() >= 99999) {
            CriticalValues::alpha_t alpha00001(_scanRunner.getCriticalValues().getAlpha00001());
            outfile << "... 0.00001: " <<  alpha00001.second << std::endl;
        }
        if (_scanRunner.getParameters().getNumReplicationsRequested() >= 9999) {
            CriticalValues::alpha_t alpha0001(_scanRunner.getCriticalValues().getAlpha0001());
            outfile << "... 0.0001: " <<  alpha0001.second << std::endl;
        }
        if (_scanRunner.getParameters().getNumReplicationsRequested() >= 999) {
            CriticalValues::alpha_t alpha001(_scanRunner.getCriticalValues().getAlpha001());
            outfile << "... 0.001: " <<  alpha001.second << std::endl;
        }
        if (_scanRunner.getParameters().getNumReplicationsRequested() >= 99) {
            CriticalValues::alpha_t alpha01(_scanRunner.getCriticalValues().getAlpha01());
            outfile << "... 0.01: " <<  alpha01.second << std::endl;
        }
        if (_scanRunner.getParameters().getNumReplicationsRequested() >= 19) {
            CriticalValues::alpha_t alpha05(_scanRunner.getCriticalValues().getAlpha05());
            outfile << "... 0.05: " <<  alpha05.second << std::endl;
        }
        outfile << std::endl;
    }

    // print power estimation values
    if (_scanRunner.getParameters().getPerformPowerEvaluations()) {
        // sanity check
        if (_scanRunner.getPowerEstimations().size() == 0)
            throw prg_error("Number of power estimations is zero.", "ResultsFileWriter::writeASCII(...)");
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
    switch (_scanRunner.getParameters().getScanType()) {
        case Parameters::TREEONLY : {
            buffer = "Tree Scan";
            switch (_scanRunner.getParameters().getConditionalType()) {
                case Parameters::UNCONDITIONAL : buffer += " with Unconditional"; break;
                case Parameters::TOTALCASES : buffer += " with Conditional"; break;
                case Parameters::CASESEACHBRANCH : break;
                default: throw prg_error("Unknown conditional type (%d).", "writeASCII()", _scanRunner.getParameters().getConditionalType());
            }
            switch (_scanRunner.getParameters().getModelType()) {
                case Parameters::POISSON : buffer += " Poisson Model"; break;
                case Parameters::BERNOULLI : buffer += " Bernoulli Model"; break;
                case Parameters::TEMPORALSCAN : break;
                default: throw prg_error("Unknown nodel type (%d).", "writeASCII()", _scanRunner.getParameters().getModelType());
            }
        } break;
        case Parameters::TREETIME : buffer = "Tree Temporal Scan"; break;
        default: throw prg_error("Unknown scan type (%d).", "writeASCII()", _scanRunner.getParameters().getScanType());
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

bool ResultsFileWriter::writeHTML(time_t start, time_t end) {
    std::string buffer;
    Parameters& parameters(const_cast<Parameters&>(_scanRunner.getParameters()));
    std::ofstream outfile;
    openStream(getFilenameHTML(parameters, buffer), outfile);
    if (!outfile) return false;
    ScanRunner::Loglikelihood_t calcLogLikelihood(AbstractLoglikelihood::getNewLoglikelihood(parameters, _scanRunner.getTotalC(), _scanRunner.getTotalN()));

    outfile << "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">" << std::endl; 
    outfile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?> <!-- this has to be after the doctype so that IE6 doesn't use quirks mode -->" << std::endl;
    outfile << "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\">" << std::endl;
    outfile << "<head><style type=\"text/css\">" << std::endl;
    outfile << "body {color: #353535;font: 0.75em Arial,Garuda,sans-serif;padding: 20px;}" << std::endl;
    outfile << "#banner {background: none repeat scroll 0 0 #7A991A;height: 40px;color: white;font-size: 2em;text-align: center;vertical-align: middle;padding: 10px 0 10px 0;}" << std::endl;
    outfile << ".program-info {background-color: #E5EECC;padding: 5px 0 5px 5px;font-size: 1.1em;}" << std::endl;
    outfile << ".program-info h4 {margin: 2px 0 10px 0;}" << std::endl;
    outfile << "#cuts table {border: 1px solid #BDBDBD;border-collapse: collapse;width: 100%;}" << std::endl;
    outfile << "#cuts table th {background-color: #DADAD5;text-align: center;text-align: left;/*min-width:200px;*/}" << std::endl;
    outfile << "#cuts table th, #cuts table td {border: 1px solid #C3C3C3;padding: 4px 6px;}" << std::endl;
    outfile << ".hr {background-color: #E5EECC;border: 1px solid #000000;height: 1px;/*margin: 10px 0;*//*width: 760px;*/}" << std::endl;
    outfile << "#cuts {padding: 10px;}" << std::endl;
    outfile << "#cuts h3 {margin-top: 0px;margin-bottom: 4px;}" << std::endl;
    outfile << ".reported-cut {margin: 20px 0px 0px 20px;}" << std::endl;
    outfile << "#parameter-settings {padding: 10px;}" << std::endl;
    outfile << "#parameter-settings h4 {margin-top: 2px;margin-bottom: 2px;}" << std::endl;
    outfile << ".parameter-section {margin-left: 20px;}" << std::endl;
    outfile << ".parameter-section h4 {font-weight: bold;margin-top: 2px;margin-bottom: 2px;}" << std::endl;
    outfile << ".parameter-section table {margin-left: 20px;}" << std::endl;
    outfile << ".parameter-section table th {text-align: left;}" << std::endl;
    //outfile << ".additional-cuts {display: none;}" << std::endl;
    outfile << "#id_more_cuts {text-decoration: underline;}" << std::endl;
    outfile << "#id_more_cuts:hover {cursor: pointer;}" << std::endl;
    outfile << "#id_cuts th:hover {cursor: pointer;}" << std::endl;
    outfile << ".warning {background-color:#816834;color:white;padding:10px;}" << std::endl;
    outfile << "</style>" << std::endl;

    // TODO: host these from treescan website or link to local copy?
    outfile << "<script src=\"http://www.satscan.org/javascript/jquery/jquery-1.10.2/jquery-1.10.2.min.js\" type=\"text/javascript\"></script>" << std::endl;
    // TODO: css and image resources for table sorter
    outfile << "<script src=\"http://www.satscan.org/javascript/jquery/jquery-tablesorter-2.0/jquery.tablesorter.min.js\" type=\"text/javascript\"></script>" << std::endl;

    outfile << "<script type=\"text/javascript\" charset=\"utf-8\">$(document).ready(function() {" << std::endl;
    outfile << "    //$('.additional-cuts').hide();" << std::endl;
    outfile << "    $('#id_more_cuts').click(function(){$('.additional-cuts').toggle();});" << std::endl;
    outfile << "    $('#id_cuts').tablesorter(); " << std::endl;
    outfile << "});</script>" << std::endl;
    printString(buffer, "TreeScan v%s.%s%s%s%s%s", VERSION_MAJOR, VERSION_MINOR, (!strcmp(VERSION_RELEASE, "0") ? "" : "."), (!strcmp(VERSION_RELEASE, "0") ? "" : VERSION_RELEASE), (strlen(VERSION_PHASE) ? " " : ""), VERSION_PHASE);
    outfile << "</head>" << std::endl << "<body><div id=\"banner\"><div id=\"title\">" << buffer << "</div></div>" << std::endl;
    outfile << "<div class=\"program-info\">" << std::endl;

    outfile << getAnalysisSuccinctStatement(buffer);

    outfile << "<table style=\"text-align: left;\"><tbody>" << std::endl;
    outfile << "<tr><th>Total Cases:</th><td>" << _scanRunner.getTotalC() << "</td></tr>" << std::endl;
    if (parameters.getModelType() == Parameters::POISSON) {
        outfile << "<tr><th>Total Expected:</th><td>" << getValueAsString(_scanRunner.getTotalN(), buffer, 1).c_str() << "</td></tr>" << std::endl;
    }
    if (parameters.getModelType() == Parameters::BERNOULLI) {
        outfile << "<tr><th>Total Observations:</th><td>" << static_cast<int>(_scanRunner.getTotalN()) << "</td></tr>" << std::endl;
    }
    if (parameters.getModelType() == Parameters::TEMPORALSCAN) {
        outfile << "<tr><th>Data Time Range:</th><td>" << parameters.getDataTimeRangeSet().toString(buffer).c_str() << "</td></tr>" << std::endl;
    }
    outfile << "</tbody></table></div>" << std::endl;
    outfile << "<div class=\"hr\"></div>" << std::endl;

    if (!_scanRunner.getParameters().getPerformPowerEvaluations() || (_scanRunner.getParameters().getPerformPowerEvaluations() && _scanRunner.getParameters().getPowerEvaluationType() == Parameters::PE_WITH_ANALYSIS)) {
        outfile << "<div id=\"cuts\">" << std::endl;
        if (_scanRunner.getCuts().size() == 0 || _scanRunner.getCuts().at(0)->getC() == 0 || 
            (parameters.getNumReplicationsRequested() > 0 && _scanRunner.getCuts().at(0)->getRank() > parameters.getNumReplicationsRequested())) {
            outfile << "<h3>No cuts were found.</h3>" << std::endl;
        } else {
            outfile << "<h3>MOST LIKELY CUTS</h3><div style=\"overflow:auto;max-height:350px;\"><table id=\"id_cuts\">" << std::endl;
            outfile << "<thead><tr><th>No.</th><th>Node Identifier</th>";
            if (parameters.getModelType() == Parameters::TEMPORALSCAN) {
                outfile << "<th>Node Cases</th>";
                outfile << "<th>Time Window</th>";
                outfile << "<th>Cases in Window</th>";
            } else if (parameters.getModelType() == Parameters::BERNOULLI) {
                outfile << "<th>Observations</th>";
                outfile << "<th>Cases</th>";
            } else if (parameters.getModelType() == Parameters::POISSON) {
                outfile << "<th>Observed Cases</th>";
            }
            if (parameters.isDuplicates()) {outfile << "<th>Cases (Duplicates Removed)</th>";}
            outfile << "<th>" << (parameters.getModelType() == Parameters::TEMPORALSCAN ? "Expected Cases" : "Expected") << "</th><th>Observed/Expected</th>";
            if (parameters.isDuplicates()) {outfile << "<th>O/E (Duplicates Removed)</th>";}
            outfile << "<th>Relative Risk</th><th>Excess Cases</th><th>Log Likelihood Ratio</th><th>P-value</th></tr></thead><tbody>" << std::endl;
            std::string format, replicas;
            printString(replicas, "%u", parameters.getNumReplicationsRequested());
            printString(format, "%%.%dlf", replicas.size());

            unsigned int k=0;
            outfile.setf(std::ios::fixed);
            outfile.precision(5);
            while(k < _scanRunner.getCuts().size() && _scanRunner.getCuts().at(k)->getC() > 0 && 
                  (parameters.getNumReplicationsRequested() == 0 || _scanRunner.getCuts().at(k)->getRank() < parameters.getNumReplicationsRequested() + 1)) {
                outfile << "<tr" << (k > 9 ? " class=\"additional-cuts\"" : "" ) << "><td>" << k + 1 << "</td>"
                        << "<td>" << _scanRunner.getNodes().at(_scanRunner.getCuts().at(k)->getID())->getIdentifier() << "</td>";
                if (parameters.getModelType() == Parameters::TEMPORALSCAN) {   
                    outfile << "<td>" << static_cast<int>(_scanRunner.getCuts().at(k)->getN()) << "</td>";
                    outfile << "<td>" << (_scanRunner.getCuts().at(k)->getStartIdx() - _scanRunner.getZeroTranslationAdditive()) << " to " << (_scanRunner.getCuts().at(k)->getEndIdx() - _scanRunner.getZeroTranslationAdditive()) << "</td>";
                }
                if (parameters.getModelType() == Parameters::BERNOULLI) {
                    outfile << "<td>" << static_cast<int>(_scanRunner.getCuts().at(k)->getN()) << "</td>";
                }
                outfile << "<td>" << _scanRunner.getCuts().at(k)->getC() << "</td>";
                if (parameters.isDuplicates())
                    outfile << "<td>" << _scanRunner.getCuts().at(k)->getC() - _scanRunner.getNodes().at(_scanRunner.getCuts().at(k)->getID())->getDuplicates() << "</td>";
                outfile << "<td>" << getValueAsString(_scanRunner.getCuts().at(k)->getExpected(_scanRunner), buffer) << "</td>";
                outfile << "<td>" << getValueAsString(_scanRunner.getCuts().at(k)->getODE(_scanRunner), buffer) << "</td>";
                if (parameters.isDuplicates())
                    outfile << "<td>" << getValueAsString((_scanRunner.getCuts().at(k)->getC() - _scanRunner.getNodes().at(_scanRunner.getCuts().at(k)->getID())->getDuplicates())/_scanRunner.getCuts().at(k)->getExpected(_scanRunner), buffer) << "</td>";
                outfile << "<td>" << getValueAsString(_scanRunner.getCuts().at(k)->getRelativeRisk(_scanRunner), buffer) << "</td>";
                outfile << "<td>" << getValueAsString(_scanRunner.getCuts().at(k)->getExcessCases(_scanRunner), buffer) << "</td>";
                outfile << "<td>" << printString(buffer, "%.1lf", calcLogLikelihood->LogLikelihoodRatio(_scanRunner.getCuts().at(k)->getLogLikelihood())).c_str() << "</td>";
                outfile << "<td>" << printString(buffer, format.c_str(), (double)_scanRunner.getCuts().at(k)->getRank() /(parameters.getNumReplicationsRequested() + 1)) << "</td><tr>" << std::endl;
                k++;
            }
            outfile << "</tbody></table></div>" << std::endl;
            if (k > 10) {
                outfile << "<!-- <span id=\"id_more_cuts\">Toggle Additional Cuts</span> -->" << std::endl;
            }
            outfile << "</div>" << std::endl;
        }
    }

    outfile << "<div class=\"hr\"></div>" << std::endl;
    if (parameters.getModelType() == Parameters::TEMPORALSCAN) {
        std::string buffer;
        _scanRunner.getCaselessWindowsAsString(buffer);
        if (buffer.size()) {
            outfile << "<div class=\"warning\">Warning: The following days in the data time range do not have cases: " << buffer.c_str() << "</div><div class=\"hr\"></div>";
        }
    }

    // Print critical values if requested.
    if ((_scanRunner.getParameters().getReportCriticalValues() && _scanRunner.getParameters().getNumReplicationsRequested() >= 19) || 
        (_scanRunner.getParameters().getPerformPowerEvaluations() && _scanRunner.getParameters().getCriticalValuesType() == Parameters::CV_MONTECARLO)) {
        outfile << "<div class=\"program-info\">" << std::endl;
        outfile << std::endl << "<div>A cut is statistically significant when its log likelihood ratio is greater than the critical value, which is, for significance level:</div>" << std::endl;
        if (_scanRunner.getParameters().getNumReplicationsRequested() >= 99999) {
            CriticalValues::alpha_t alpha00001(_scanRunner.getCriticalValues().getAlpha00001());
            outfile << "<div>... 0.00001: " <<  alpha00001.second << "</div>" << std::endl;
        }
        if (_scanRunner.getParameters().getNumReplicationsRequested() >= 9999) {
            CriticalValues::alpha_t alpha0001(_scanRunner.getCriticalValues().getAlpha0001());
            outfile << "<div>... 0.0001: " <<  alpha0001.second << "</div>"<< std::endl;
        }
        if (_scanRunner.getParameters().getNumReplicationsRequested() >= 999) {
            CriticalValues::alpha_t alpha001(_scanRunner.getCriticalValues().getAlpha001());
            outfile << "<div>... 0.001: " <<  alpha001.second << "</div>" << std::endl;
        }
        if (_scanRunner.getParameters().getNumReplicationsRequested() >= 99) {
            CriticalValues::alpha_t alpha01(_scanRunner.getCriticalValues().getAlpha01());
            outfile << "<div>... 0.01: " <<  alpha01.second << "</div>" << std::endl;
        }
        if (_scanRunner.getParameters().getNumReplicationsRequested() >= 19) {
            CriticalValues::alpha_t alpha05(_scanRunner.getCriticalValues().getAlpha05());
            outfile << "<div>... 0.05: " <<  alpha05.second << "</div>" << std::endl;
        }
        outfile << "</div>" << std::endl;
    }

    // print power estimation values
    if (_scanRunner.getParameters().getPerformPowerEvaluations()) {
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
