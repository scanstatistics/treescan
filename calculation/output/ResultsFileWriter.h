//******************************************************************************
#ifndef __ResultsFileWriter_H
#define __ResultsFileWriter_H
//******************************************************************************
#include <iostream>
#include <fstream>
#include "Loglikelihood.h"

class ScanRunner;
class Parameters;
class NodeStructure;
class CutStructure;

class ResultsFileWriter {
  public:
      typedef boost::tuple<double, double>  BestCutSet_t; // p-value, relative risk
      typedef boost::tuple<BestCutSet_t, BestCutSet_t>  NodeSet_t; // node, descendents

  protected:
    const ScanRunner & _scanRunner;

    std::ofstream & addTableRowForCut(CutStructure& thisCut, int k, Loglikelihood_t & calcLogLikelihood, const std::string& format, std::ofstream& outfile);
    std::string & truncateNodeId(std::string& val);
    const char * getPvalueClass(double pval, bool childClass);
    const char * getRelativeRiskClass(double rr, bool childClass);
    std::string & stripNodeIdForHtml(std::string & s);
    NodeSet_t writeJsTreeNode(std::stringstream & outfile, const NodeStructure& node, const std::map<int, const CutStructure*>& cutMap, int collapseAtLevel);

    std::ofstream & openStream(const std::string& outputfile, std::ofstream & outfile, bool overwrite=false);
    std::string & getTotalRunningTime(time_t start, time_t end, std::string & buffer) const;
    std::string & getAnalysisSuccinctStatement(std::string & buffer) const;

  public:
    ResultsFileWriter(const ScanRunner& scanRunner) : _scanRunner(scanRunner) {}

    static std::string & getFilenameHTML(const Parameters& parameters, std::string& buffer);

    bool writeASCII(time_t start, time_t end);
    bool writeHTML(time_t start, time_t end);
};
//******************************************************************************
#endif
