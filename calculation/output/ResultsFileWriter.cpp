//******************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//******************************************************************************
#include "ResultsFileWriter.h"
#include "PrjException.h"
#include "ScanRunner.h"
#include "AsciiPrintFormat.h"
#include "ParametersPrint.h"

std::ofstream & ResultsFileWriter::openStream(const std::string& outputfile, std::ofstream & outfile) {
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
        const_cast<Parameters&>(_scanRunner.getParameters()).setOutputFileName(buffer.c_str());
        print.Printf("Creating the output file in documents directory: %s\n", BasePrint::P_STDOUT, buffer.c_str());
    } else
        print.Printf("Creating the output file: %s\n", BasePrint::P_STDOUT, outputfile.c_str());
    return outfile;
}

bool ResultsFileWriter::writeASCII(const std::string& outputfile, time_t start, time_t end) {
    std::ofstream outfile;
    openStream(outputfile, outfile);
    if (!outfile) return false;

    AsciiPrintFormat PrintFormat;
    Parameters& parameters(const_cast<Parameters&>(_scanRunner.getParameters()));

    PrintFormat.PrintVersionHeader(outfile);
    std::string buffer = ctime(&start);
    outfile << std::endl << "Program run on: " << buffer;

    PrintFormat.SetMarginsAsSummarySection();
    PrintFormat.PrintSectionSeparatorString(outfile);
    outfile << std::endl << "SUMMARY OF DATA" << std::endl << std::endl;
    PrintFormat.PrintSectionLabel(outfile, "Analysis", false);
    buffer = (parameters.isConditional() ? "Conditional" : "Unconditional");
    PrintFormat.PrintAlignedMarginsDataString(outfile, buffer);
    PrintFormat.PrintSectionLabel(outfile, "Total Cases", false);
    PrintFormat.PrintAlignedMarginsDataString(outfile, printString(buffer, "%ld", _scanRunner.getTotalC()));
    PrintFormat.PrintSectionLabel(outfile, "Total Measure", false);
    PrintFormat.PrintAlignedMarginsDataString(outfile, printString(buffer, "%lf", _scanRunner.getTotalN()));
    PrintFormat.PrintSectionSeparatorString(outfile, 0, 2);

    // write detected cuts
    //if (_parameters.isDuplicates()) outfile << "#CasesWithoutDuplicates ";
    //outfile << "#Exp O/E ";
    //if (_parameters.isDuplicates()) outfile << "O/EWithoutDuplicates ";
    //outfile << "LLR pvalue" << std::endl;

    if (_scanRunner.getCuts().at(0)->getC() == 0) {
        outfile << "No clusters were found." << std::endl;
        outfile.close();
        return true;
    }

    outfile << "Most Likely Cuts"<< std::endl << std::endl;

    std::string format, replicas;
    printString(replicas, "%u", parameters.getNumReplicationsRequested());
    printString(format, "%%.%dlf", replicas.size());

    unsigned int k=0;
    outfile.setf(std::ios::fixed);
    outfile.precision(5);
    while( k < _scanRunner.getCuts().size() && _scanRunner.getCuts().at(k)->getC() > 0 && _scanRunner.getRanks().at(k) < parameters.getNumReplicationsRequested() + 1) {
        PrintFormat.SetMarginsAsClusterSection( k + 1);
        outfile << k + 1 << ")";
        PrintFormat.PrintSectionLabel(outfile, "Node Identifier", false);
        PrintFormat.PrintAlignedMarginsDataString(outfile, _scanRunner.getNodes().at(_scanRunner.getCuts().at(k)->getID())->getIdentifier());

        PrintFormat.PrintSectionLabel(outfile, "Number of Cases", true);
        printString(buffer, "%ld", _scanRunner.getCuts().at(k)->getC());
        PrintFormat.PrintAlignedMarginsDataString(outfile, buffer);
        if (parameters.isDuplicates()) {
            PrintFormat.PrintSectionLabel(outfile, "Cases (Duplicates Removed)", true);
            printString(buffer, "%ld", _scanRunner.getCuts().at(k)->getC() - _scanRunner.getNodes().at(_scanRunner.getCuts().at(k)->getID())->getDuplicates());
            PrintFormat.PrintAlignedMarginsDataString(outfile, buffer);
        }
        PrintFormat.PrintSectionLabel(outfile, "Expected", true);
        PrintFormat.PrintAlignedMarginsDataString(outfile, getValueAsString(_scanRunner.getCuts().at(k)->getN(), buffer));
        PrintFormat.PrintSectionLabel(outfile, "Observed/Expected", true);
        PrintFormat.PrintAlignedMarginsDataString(outfile, getValueAsString(_scanRunner.getCuts().at(k)->getC()/_scanRunner.getCuts().at(k)->getN(), buffer));
        if (parameters.isDuplicates()) {
            PrintFormat.PrintSectionLabel(outfile, "O/E (Duplicates Removed)", true);
            PrintFormat.PrintAlignedMarginsDataString(outfile, getValueAsString((_scanRunner.getCuts().at(k)->getC() - _scanRunner.getNodes().at(_scanRunner.getCuts().at(k)->getID())->getDuplicates())/_scanRunner.getCuts().at(k)->getN(), buffer));
        }
        PrintFormat.PrintSectionLabel(outfile, "Log Likelihood Ratio", true);
        if (parameters.isConditional())
            printString(buffer, "%lf", _scanRunner.getCuts().at(k)->getLogLikelihood() - _scanRunner.getTotalC() * log(_scanRunner.getTotalC()/_scanRunner.getTotalN()));
        else
            printString(buffer, "%lf", _scanRunner.getCuts().at(k)->getLogLikelihood());
        PrintFormat.PrintAlignedMarginsDataString(outfile, buffer);
        PrintFormat.PrintSectionLabel(outfile, "P-value", true);
        printString(buffer, format.c_str(), (double)_scanRunner.getRanks().at(k) /(parameters.getNumReplicationsRequested() + 1));
        PrintFormat.PrintAlignedMarginsDataString(outfile, buffer, 2);
        k++;
    }

    //outfile << std::endl << std::endl;
    //outfile << "Information About Each Node" << std::endl;
    //outfile << "ID Obs Exp O/E" << std::endl;
    // outfile.width(10);
    //for (size_t i=0; i < _Nodes.size(); i++)
    //    if (_Nodes.at(i)->_BrN > 0)
    //        outfile << "0 " << _Nodes.at(i)->_identifier.c_str() << " " << _Nodes.at(i)->_BrC << " " << _Nodes.at(i)->_BrN << " " << _Nodes.at(i)->_BrC/_Nodes.at(i)->_BrN << " 0 0 " << std::endl;

    ParametersPrint(parameters).Print(outfile);
    outfile << "Program run on     : " << ctime(&start);
    outfile << "Program completed  : " << ctime(&end);
    outfile << "Total Running Time : " << getTotalRunningTime(start, end, buffer) << std::endl;
    if (parameters.getNumParallelProcessesToExecute() > 1) outfile << "Processor Usage    : " << parameters.getNumParallelProcessesToExecute() << " processors" << std::endl;

    outfile.close();
    return true;
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

bool ResultsFileWriter::writeHTML(const std::string& outputfile, time_t start, time_t end) {
    std::ofstream outfile;
    openStream(outputfile, outfile);
    if (!outfile) return false;
    Parameters& parameters(const_cast<Parameters&>(_scanRunner.getParameters()));
    std::string buffer;

    outfile << "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">" << std::endl; 
    outfile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?> <!-- this has to be after the doctype so that IE6 doesn't use quirks mode -->" << std::endl;
    outfile << "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\">" << std::endl;
    outfile << "<head><style type=\"text/css\">" << std::endl;
    outfile << "body {color: #353535;font: 0.75em Arial,Garuda,sans-serif;padding: 20px;}" << std::endl;
    outfile << "#banner {background: none repeat scroll 0 0 #7A991A;height: 40px;color: white;font-size: 2em;text-align: center;vertical-align: middle;padding: 10px 0 10px 0;}" << std::endl;
    outfile << ".program-info {background-color: #E5EECC;padding: 5px 0 5px 5px;font-size: 1.1em;}" << std::endl;
    outfile << "#clusters table {border: 1px solid #BDBDBD;border-collapse: collapse;width: 100%;}" << std::endl;
    outfile << "#clusters table th {background-color: #DADAD5;text-align: center;text-align: left;/*min-width:200px;*/}" << std::endl;
    outfile << "#clusters table th, #clusters table td {border: 1px solid #C3C3C3;padding: 4px 6px;}" << std::endl;
    outfile << ".hr {background-color: #E5EECC;border: 1px solid #000000;height: 1px;/*margin: 10px 0;*//*width: 760px;*/}" << std::endl;
    outfile << "#clusters {padding: 10px;}" << std::endl;
    outfile << "#clusters h3 {margin-top: 0px;margin-bottom: 4px;}" << std::endl;
    outfile << ".reported-cluster {margin: 20px 0px 0px 20px;}" << std::endl;
    outfile << "#parameter-settings {padding: 10px;}" << std::endl;
    outfile << "#parameter-settings h4 {margin-top: 2px;margin-bottom: 2px;}" << std::endl;
    outfile << ".parameter-section {margin-left: 20px;}" << std::endl;
    outfile << ".parameter-section h4 {font-weight: bold;margin-top: 2px;margin-bottom: 2px;}" << std::endl;
    outfile << ".parameter-section table {margin-left: 20px;}" << std::endl;
    outfile << ".parameter-section table th {text-align: left;}" << std::endl;
    //outfile << ".additional-clusters {display: none;}" << std::endl;
    outfile << "#id_more_clusters {text-decoration: underline;}" << std::endl;
    outfile << "#id_more_clusters:hover {cursor: pointer;}" << std::endl;
    outfile << "#id_cuts th:hover {cursor: pointer;}" << std::endl;
    outfile << "</style>" << std::endl;

    // TODO: host these from treescan website or link to local copy?
    outfile << "<script src=\"http://ajax.googleapis.com/ajax/libs/jquery/1.4/jquery.min.js\" type=\"text/javascript\"></script>" << std::endl;
    // TODO: css and image resources for table sorter
    outfile << "<script src=\"http://artknow.googlecode.com/svn-history/r14/trunk/Site/librarys/jquery.tablesorter.js\" type=\"text/javascript\"></script>" << std::endl;

    outfile << "<script type=\"text/javascript\" charset=\"utf-8\">$(document).ready(function() {" << std::endl;
    outfile << "    //$('.additional-clusters').hide();" << std::endl;
    outfile << "    $('#id_more_clusters').click(function(){$('.additional-clusters').toggle();});" << std::endl;
    outfile << "    $('#id_cuts').tablesorter(); " << std::endl;
    outfile << "});</script>" << std::endl;
    printString(buffer, "TreeScan v%s.%s%s%s%s%s", VERSION_MAJOR, VERSION_MINOR, (!strcmp(VERSION_RELEASE, "0") ? "" : "."), (!strcmp(VERSION_RELEASE, "0") ? "" : VERSION_RELEASE), (strlen(VERSION_PHASE) ? " " : ""), VERSION_PHASE);
    outfile << "</head>" << std::endl << "<body><div id=\"banner\"><div id=\"title\">" << buffer << "</div></div>" << std::endl;
    outfile << "<div class=\"program-info\"><table style=\"text-align: left;\"><tbody>" << std::endl;
    outfile << "<tr><th>Analysis</th><td>" << (parameters.isConditional() ? "Conditional" : "Unconditional") << "</td></tr>" << std::endl;
    outfile << "<tr><th>Total Cases</th><td>" << _scanRunner.getTotalC() << "</td></tr>" << std::endl;
    outfile << "<tr><th>Total Measure</th><td>" << _scanRunner.getTotalN() << "</td></tr>" << std::endl;
    outfile << "</tbody></table></div>" << std::endl;
    outfile << "<div class=\"hr\"></div>" << std::endl;

    outfile << "<div id=\"clusters\">" << std::endl;
    if (_scanRunner.getCuts().at(0)->getC() == 0) {
        outfile << "<h3>No clusters were found.</h3>" << std::endl;
    } else {
        outfile << "<h3>Most Likely Cuts</h3><div style=\"overflow:auto;height:350px;\"><table id=\"id_cuts\">" << std::endl;
        outfile << "<thead><tr><th>No.</th><th>Node Identifier</th><th>Number of Cases</th>";
        if (parameters.isDuplicates()) {outfile << "<th>Cases (Duplicates Removed)</th>";}
        outfile << "<th>Expected</th><th>Observed/Expected</th>";
        if (parameters.isDuplicates()) {outfile << "<th>O/E (Duplicates Removed)</th>";}
        outfile << "<th>Log Likelihood Ratio</th><th>P-value</th></tr></thead><tbody>" << std::endl;
        std::string format, replicas;
        printString(replicas, "%u", parameters.getNumReplicationsRequested());
        printString(format, "%%.%dlf", replicas.size());

        unsigned int k=0;
        outfile.setf(std::ios::fixed);
        outfile.precision(5);
        while( k < _scanRunner.getCuts().size() && _scanRunner.getCuts().at(k)->getC() > 0 && _scanRunner.getRanks().at(k) < parameters.getNumReplicationsRequested() + 1) {
            outfile << "<tr" << (k > 9 ? " class=\"additional-clusters\"" : "" ) << "><td>" << k + 1 << "</td>"
                    << "<td>" << _scanRunner.getNodes().at(_scanRunner.getCuts().at(k)->getID())->getIdentifier() << "</td>"
                    << "<td>" << _scanRunner.getCuts().at(k)->getC() << "</td>";
            if (parameters.isDuplicates())
                outfile << "<td>" << _scanRunner.getCuts().at(k)->getC() - _scanRunner.getNodes().at(_scanRunner.getCuts().at(k)->getID())->getDuplicates() << "</td>";
            outfile << "<td>" << getValueAsString(_scanRunner.getCuts().at(k)->getN(), buffer) << "</td>";
            outfile << "<td>" << getValueAsString(_scanRunner.getCuts().at(k)->getC()/_scanRunner.getCuts().at(k)->getN(), buffer) << "</td>";
            if (parameters.isDuplicates())
                outfile << "<td>" << getValueAsString((_scanRunner.getCuts().at(k)->getC() - _scanRunner.getNodes().at(_scanRunner.getCuts().at(k)->getID())->getDuplicates())/_scanRunner.getCuts().at(k)->getN(), buffer) << "</td>";
            if (parameters.isConditional())
                outfile << "<td>" << _scanRunner.getCuts().at(k)->getLogLikelihood() - _scanRunner.getTotalC() * log(_scanRunner.getTotalC()/_scanRunner.getTotalN())<< "/<td>";
            else
                outfile << "<td>" << _scanRunner.getCuts().at(k)->getLogLikelihood() << "</td>";
            outfile << "<td>" << printString(buffer, format.c_str(), (double)_scanRunner.getRanks().at(k) /(parameters.getNumReplicationsRequested() + 1)) << "</td><tr>" << std::endl;
            k++;
        }
        outfile << "</tbody></table></div>" << std::endl;
        if (k > 10) {
            outfile << "<!-- <span id=\"id_more_clusters\">Toggle Additional Clusters</span> -->" << std::endl;
        }
        outfile << "</div>" << std::endl;
    }
    outfile << "<div class=\"hr\"></div>" << std::endl;

    ParametersPrint(parameters).PrintHTML(outfile);
    outfile << "<div class=\"hr\"></div>" << std::endl;

    outfile << "<div class=\"program-info\"><table style=\"text-align: left;\"><tbody>" << std::endl;
    outfile << "<tr><th>Program run on</th><td>" << ctime(&start) << "</td></tr>" << std::endl;
    outfile << "<tr><th>Program completed</th><td>" << ctime(&end) << "</td></tr>" << std::endl;
    outfile << "<tr><th>Total Running Time</th><td>" << getTotalRunningTime(start, end, buffer) << "</td></tr>" << std::endl;
    if (parameters.getNumParallelProcessesToExecute() > 1) outfile << "<tr><th>Processor Usage</th><td>" << parameters.getNumParallelProcessesToExecute() << " processors" << "</td></tr>" << std::endl;
    outfile << "</tbody></table></div>" << std::endl;

    outfile << "</body></html>" << std::endl;

    outfile.close();
    return true;
}
