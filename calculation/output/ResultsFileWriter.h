//******************************************************************************
#ifndef __ResultsFileWriter_H
#define __ResultsFileWriter_H
//******************************************************************************
#include <iostream>
#include <fstream>
#include <map>
#include "Loglikelihood.h"
#include "ptr_vector.h"

class ScanRunner;
class Parameters;
class NodeStructure;
class CutStructure;
class FieldDef;

class ResultsFileWriter {
  public:
      typedef boost::tuple<double, double, RecurrenceInterval_t>  BestCutSet_t; // p-value, relative risk
      typedef boost::tuple<BestCutSet_t, BestCutSet_t>  NodeSet_t; // node, descendents

  protected:
    const ScanRunner & _scanRunner;

    std::ofstream & addTableRowForCut(CutStructure& thisCut, Loglikelihood_t & calcLogLikelihood, const std::string& format, std::ofstream& outfile, std::stringstream * subrows=0);
    const char    * getPvalueClass(double pval, bool childClass);
    const char    * getRelativeRiskClass(double rr, bool childClass);
    const char    * getRecurranceIntervalClass(const RecurrenceInterval_t& ri, bool childClass) const;
    std::string   & getRecurranceIntervalAsString(const RecurrenceInterval_t& ri, std::string& buffer) const;
    const char    * getSignalClass(double pval, bool childClass);
    std::string   & encodeForJavascript(std::string & text) const;
    bool            treeSequentialAnalysisComplete() const;
    NodeSet_t       writeJsTreeNode(std::stringstream & outfile, const NodeStructure& node, const std::map<int, const CutStructure*>& cutMap, int collapseAtLevel);

    std::ofstream & openStream(const std::string& outputfile, std::ofstream & outfile, bool overwrite=false) const;
    std::string   & getTotalRunningTime(time_t start, time_t end, std::string & buffer) const;
    std::string   & getAnalysisSuccinctStatement(std::string & statement, const std::string& newline) const;

    std::stringstream & getNCBIAsnDefinition(const NodeStructure& node, const ptr_vector<FieldDef>& fieldDefinitions, bool idoffset, const std::map<int, const CutStructure*>& nodeCuts, std::stringstream& destination) const;
    std::string & encodeForASN(std::string & text) const;
    std::stringstream & getNewickDefinition(const NodeStructure& node, std::stringstream& destination) const;
    std::string & encodeForNewick(std::string & text) const;

  public:
    ResultsFileWriter(const ScanRunner& scanRunner) : _scanRunner(scanRunner) {}

    static std::string & getHtmlFilename(const Parameters& parameters, std::string& buffer);
    static std::string & getAsnFilename(const Parameters& parameters, std::string& buffer);
    static std::string & getNewickFilename(const Parameters& parameters, std::string& buffer);

    bool writeASCII(time_t start, time_t end);
    bool writeHTML(time_t start, time_t end);
    bool writeNCBIAsn() const;
    bool writeNewick() const;
};
//******************************************************************************
#endif
