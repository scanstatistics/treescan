//***************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//***************************************************************************
#include "Parameters.h"
#include "PrjException.h"
#include "RandomNumberGenerator.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/assign.hpp>

const int Parameters::giNumParameters = 39;

Parameters::cut_maps_t Parameters::getCutTypeMap() {
   cut_map_t cut_type_map_abbr = boost::assign::map_list_of("S",Parameters::SIMPLE) ("P",Parameters::PAIRS) ("T",Parameters::TRIPLETS) ("O",Parameters::ORDINAL);
   cut_map_t cut_type_map = boost::assign::map_list_of("simple",Parameters::SIMPLE) ("pairs",Parameters::PAIRS) ("triplets",Parameters::TRIPLETS) ("ordinal",Parameters::ORDINAL);
   return std::make_pair(cut_type_map_abbr, cut_type_map);
}

bool  Parameters::operator==(const Parameters& rhs) const {
  if (_replications != rhs._replications) return false;
  if (_treeFileName != rhs._treeFileName) return false;
  if (_cutsFileName != rhs._cutsFileName) return false;  
  if (_countFileName != rhs._countFileName) return false;
  if (_dataTimeRangeSet.getDataTimeRangeSets() != rhs._dataTimeRangeSet.getDataTimeRangeSets()) return false;
  if (_temporalStartRange != rhs._temporalStartRange) return false;
  if (_temporalEndRange != rhs._temporalEndRange) return false;
  if (_outputFileName != rhs._outputFileName) return false;
  if (_resultsFormat != rhs._resultsFormat) return false;
  if (_parametersSourceFileName != rhs._parametersSourceFileName) return false;
  //if (_creationVersion != rhs._creationVersion) return false;
  //if (_randomizationSeed != rhs._randomizationSeed) return false;
  if (_numRequestedParallelProcesses != rhs._numRequestedParallelProcesses) return false;
  if (_randomlyGenerateSeed != rhs._randomlyGenerateSeed) return false;
  if (_duplicates != rhs._duplicates) return false;  
  if (_generateHtmlResults != rhs._generateHtmlResults) return false;
  if (_generateTableResults != rhs._generateTableResults) return false;
  if (_printColumnHeaders != rhs._printColumnHeaders) return false; 
  if (_modelType != rhs._modelType) return false;
  if (_probablility_ratio != rhs._probablility_ratio) return false;
  if (_cut_type != rhs._cut_type) return false;
  if (_conditional_type != rhs._conditional_type) return false;
  if (_scan_type != rhs._scan_type) return false;
  if (_maximum_window_percentage != rhs._maximum_window_percentage) return false;
  if (_minimum_window_length != rhs._minimum_window_length) return false;
  if (_generate_llr_results != rhs._generate_llr_results) return false;
  if (_read_simulations != rhs._read_simulations) return false;
  if (_input_sim_file != rhs._input_sim_file) return false;
  if (_write_simulations != rhs._write_simulations) return false;
  if (_output_sim_file != rhs._output_sim_file) return false;
  if (_maximum_window_percentage != rhs._maximum_window_percentage) return false;
  if (_maximum_window_length != rhs._maximum_window_length) return false;
  if (_maximum_window_type != rhs._maximum_window_type) return false;
  if (_minimum_window_length != rhs._minimum_window_length) return false;
  if (_report_critical_values != rhs._report_critical_values) return false;
  if (_perform_power_evaluations != rhs._perform_power_evaluations) return false;
  if (_critical_values_type != rhs._critical_values_type) return false;
  if (_critical_value_05 != rhs._critical_value_05) return false;
  if (_critical_value_01 != rhs._critical_value_01) return false;
  if (_critical_value_001 != rhs._critical_value_001) return false;
  if (_power_evaluation_type != rhs._power_evaluation_type) return false;
  if (_power_evaluation_totalcases != rhs._power_evaluation_totalcases) return false;
  if (_power_replica != rhs._power_replica) return false;
  if (_power_alt_hypothesis_filename != rhs._power_alt_hypothesis_filename) return false;

  return true;
}

/** If passed filename contains a slash, then assumes that path is complete and
    sInputFilename is not modified. If filename does not contain a slash, it is
    assumed that filename is located in same directory of parameter file.
    sInputFilename is reset to this location. Note that the primary reason for
    implementing this feature was to permit the program to be installed in any
    location and sample parameter files run immediately without having to edit
    input file paths. */
void Parameters::assignMissingPath(std::string & sInputFilename, bool bCheckWritable) {
  FileName      fParameterFilename, fFilename;
  std::string   buffer;

  if (! sInputFilename.empty()) {
    //Assume that if slashes exist, then this is a complete file path, so
    //we'll make no attempts to determine what path might be otherwise.
    if (sInputFilename.find(FileName::SLASH) == sInputFilename.npos) {
      //If no slashes, then this file is assumed to be in same directory as parameters file.
      fParameterFilename.setFullPath(getSourceFileName().c_str());

      fFilename.setFullPath(sInputFilename.c_str());
      fFilename.setLocation(fParameterFilename.getLocation(buffer).c_str());

      if (bCheckWritable && !ValidateFileAccess(fFilename.getFullPath(buffer), true)) {
        // if writability fails, then try setting to user documents directory
        std::string temp;
        fFilename.setLocation(GetUserDocumentsDirectory(buffer, fParameterFilename.getLocation(temp)).c_str());
      }

      fFilename.getFullPath(sInputFilename);
    }
  }
}

/** Copies all class variables from the given Parameters object (rhs) into this one */
void Parameters::copy(const Parameters &rhs) {
    _treeFileName = rhs._treeFileName;
    _countFileName = rhs._countFileName;
    _dataTimeRangeSet = rhs._dataTimeRangeSet;
    _cutsFileName = rhs._cutsFileName;
    _duplicates = rhs._duplicates;

    _scan_type = rhs._scan_type;
    _conditional_type = rhs._conditional_type;
    _modelType = rhs._modelType;
    _probablility_ratio = rhs._probablility_ratio;
    _temporalStartRange = rhs._temporalStartRange;
    _temporalEndRange = rhs._temporalEndRange;
    _replications = rhs._replications;
    _cut_type = rhs._cut_type;
    _maximum_window_percentage = rhs._maximum_window_percentage;
    _maximum_window_length = rhs._maximum_window_length;
    _maximum_window_type = rhs._maximum_window_type;
    _minimum_window_length = rhs._minimum_window_length;

    _outputFileName = rhs._outputFileName;
    _resultsFormat = rhs._resultsFormat;
    _generateHtmlResults = rhs._generateHtmlResults;
    _generateTableResults = rhs._generateTableResults;
    _printColumnHeaders = rhs._printColumnHeaders;
    _generate_llr_results = rhs._generate_llr_results;

    _perform_power_evaluations = rhs._perform_power_evaluations;
    _critical_values_type = rhs._critical_values_type;
    _critical_value_05 = rhs._critical_value_05;
    _critical_value_01 = rhs._critical_value_01;
    _critical_value_001 = rhs._critical_value_001;
    _power_evaluation_type = rhs._power_evaluation_type;
    _power_evaluation_totalcases = rhs._power_evaluation_totalcases;
    _power_replica = rhs._power_replica;
    _power_alt_hypothesis_filename = rhs._power_alt_hypothesis_filename;

    _read_simulations = rhs._read_simulations;
    _input_sim_file = rhs._input_sim_file;
    _write_simulations = rhs._write_simulations;
    _output_sim_file = rhs._output_sim_file;
    _report_critical_values = rhs._report_critical_values;

    _randomizationSeed = rhs._randomizationSeed;
    _numRequestedParallelProcesses = rhs._numRequestedParallelProcesses;
    _randomlyGenerateSeed = rhs._randomlyGenerateSeed;

    _parametersSourceFileName = rhs._parametersSourceFileName;
    _creationVersion = rhs._creationVersion;
}

/* Returns the maximum temporal window in data time units. */
unsigned int Parameters::getMaximumWindowInTimeUnits() const {
    switch (_maximum_window_type) {
        case Parameters::PERCENTAGE_WINDOW :
            return static_cast<unsigned int>(std::floor(static_cast<double>(_dataTimeRangeSet.getTotalDaysAcrossRangeSets()) * _maximum_window_percentage / 100.0));
        case Parameters::FIXED_LENGTH : return _maximum_window_length;
        default: throw prg_error("Unknown maximum window type (%d).", "getMaximumWindowInTimeUnits()", _maximum_window_type);
    }
}

/** Returns number of parallel processes to run. */
unsigned int Parameters::getNumParallelProcessesToExecute() const {
#ifdef RPRTCMPT_RUNTIMES
  // reporting of run-time components is not thread safe at this time,
  // and has no useful purpose to be such - at this time
  return 1;
#else
  unsigned int  iNumProcessors;

  if (_numRequestedParallelProcesses <= 0)
    //parameter of zero or less indicates that we want all available processors
    iNumProcessors = GetNumSystemProcessors();
  else
    //else parameter indicates the maximum number of processors to use
    iNumProcessors = std::min(_numRequestedParallelProcesses, GetNumSystemProcessors());
    //iNumProcessors = giNumRequestedParallelProcesses;

  return iNumProcessors;
#endif  
}

/** If passed filename has same path as passed parameter filename, returns 'name.extension' else returns filename. */
const char * Parameters::getRelativeToParameterName(const FileName& fParameterName,
                                                     const std::string& sFilename,
                                                     std::string& sValue) const {
  FileName      fInputFilename(sFilename.c_str());
  std::string   buffer, buffer2;

  #if defined(_WINDOWS_)
  if (!_stricmp(fInputFilename.getLocation(buffer).c_str(), fParameterName.getLocation(buffer2).c_str()))
  #else
  if (!stricmp(fInputFilename.getLocation(buffer).c_str(), fParameterName.getLocation(buffer2).c_str()))
  #endif
    sValue = fInputFilename.getFileName() + fInputFilename.getExtension();
  else
    sValue = sFilename.c_str();
  return sValue.c_str();
}


/** Sets counts data file name.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file does not exist. */
void Parameters::setCountFileName(const char * sCountFileName, bool bCorrectForRelativePath) {
  _countFileName = sCountFileName;
  if (bCorrectForRelativePath) assignMissingPath(_countFileName);
}

/** Sets cuts data file name.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file does not exist. */
void Parameters::setCutsFileName(const char * sCutsFileName, bool bCorrectForRelativePath) {
  _cutsFileName = sCutsFileName;
  if (bCorrectForRelativePath) assignMissingPath(_cutsFileName);
}

/** Sets counts data file name.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file does not exist. */
void Parameters::setTreeFileName(const char * sTreeFileName, bool bCorrectForRelativePath) {
  _treeFileName = sTreeFileName;
  if (bCorrectForRelativePath) assignMissingPath(_treeFileName);
}


/** initializes global variables to default values */
void Parameters::setAsDefaulted() {
    _treeFileName = "";
    _countFileName = "";
    _dataTimeRangeSet = DataTimeRangeSet();
    _cutsFileName = "";
    _duplicates = false;

    _scan_type = TREEONLY;
    _conditional_type = UNCONDITIONAL;
    _modelType = POISSON;
    _probablility_ratio = ratio_t(1,2);
    _temporalStartRange = DataTimeRange();
    _temporalEndRange = DataTimeRange();
    _replications = 999;
    _cut_type = SIMPLE;
    _maximum_window_percentage = 50.0;
    _maximum_window_length = 1;
    _maximum_window_type = PERCENTAGE_WINDOW;
    _minimum_window_length = 2;

    _outputFileName = "";
    _generateHtmlResults = false;
    _generateTableResults = false;
    _printColumnHeaders = true;
    _resultsFormat = TEXT;
    _generate_llr_results = false;

    _perform_power_evaluations = false;
    _critical_values_type = CV_MONTECARLO;
    _critical_value_05                       = 0.0;
    _critical_value_01                       = 0.0;
    _critical_value_001                      = 0.0;

    _read_simulations = false;
    _input_sim_file = "";
    _write_simulations = false;
    _output_sim_file = "";
    _report_critical_values = false;
    _power_evaluation_type = PE_WITH_ANALYSIS;
    _power_evaluation_totalcases = 600;
    _power_replica = _replications + 1;
    _power_alt_hypothesis_filename = "";

    _creationVersion.iMajor = atoi(VERSION_MAJOR);
    _creationVersion.iMinor = atoi(VERSION_MINOR);
    _creationVersion.iRelease = atoi(VERSION_RELEASE);
    _randomizationSeed = RandomNumberGenerator::glDefaultSeed;
    _randomlyGenerateSeed = false;
    _numRequestedParallelProcesses = 0;
}

/** Sets output data file name.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file
    does not exist. */
void Parameters::setOutputFileName(const char * sOutPutFileName, bool bCorrectForRelativePath) {
  _outputFileName = sOutPutFileName;
  if (bCorrectForRelativePath) assignMissingPath(_outputFileName, true);
}

/** Sets power evaluation alternative hypothesis data file name.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file
    does not exist. */
void Parameters::setPowerEvaluationAltHypothesisFilename(const char * s, bool bCorrectForRelativePath) {
  _power_alt_hypothesis_filename = s;
  if (bCorrectForRelativePath) assignMissingPath(_power_alt_hypothesis_filename, false);
}

/** Sets filename of file used to load parameters. */
void Parameters::setSourceFileName(const char * sParametersSourceFileName) {
  //Use FileName class to ensure that a relative path is expanded to absolute path.
  std::string buffer;
  _parametersSourceFileName = FileName(sParametersSourceFileName).getFullPath(buffer);
}

void Parameters::setInputSimulationsFilename(const char * s, bool bCorrectForRelativePath) {
  _input_sim_file = s;
  if (bCorrectForRelativePath) assignMissingPath(_input_sim_file);
}

void Parameters::setOutputSimulationsFilename(const char * s, bool bCorrectForRelativePath) {
  _output_sim_file = s;
  if (bCorrectForRelativePath) assignMissingPath(_output_sim_file);
}

void Parameters::read(const std::string &filename, ParametersFormat type) {
    using boost::property_tree::ptree;
    ptree pt;

    switch (type) {
        case JSON: read_json(filename, pt); break;
        case XML: 
        default: read_xml(filename, pt);
    }
    setSourceFileName(filename.c_str());

    // Input
    setTreeFileName(pt.get<std::string>("parameters.input.tree-filename", "").c_str(), true);
    setCountFileName(pt.get<std::string>("parameters.input.count-filename", "").c_str(), true);
    _dataTimeRangeSet.assign(pt.get<std::string>("parameters.input.data-time-range", "0,0"));
    // Advanced Input
    setCutsFileName(pt.get<std::string>("parameters.input.advanced.cuts-filename", "").c_str(), true);
    _cut_type = static_cast<CutType>(pt.get<unsigned int>("parameters.input-advanced.cuts-type", SIMPLE));
    _duplicates = pt.get<bool>("parameters.input.advanced.duplicates", false);
    // Analysis
    _scan_type = static_cast<ScanType>(pt.get<unsigned int>("parameters.analysis.scan", TREEONLY));
    _conditional_type = static_cast<ConditionalType>(pt.get<unsigned int>("parameters.analysis.conditional", UNCONDITIONAL));
    _modelType = static_cast<ModelType>(pt.get<unsigned int>("parameters.analysis.probability-model", POISSON));
    _probablility_ratio.first = pt.get<unsigned int>("parameters.analysis.event-probability.numerator", 1);
    _probablility_ratio.second = pt.get<unsigned int>("parameters.analysis.event-probability.denominator", 2);
    _temporalStartRange.assign(pt.get<std::string>("parameters.analysis.temporal-window.start-range", "0,0"));
    _temporalEndRange.assign(pt.get<std::string>("parameters.analysis.temporal-window.end-range", "0,0"));
    // Advanced Analysis - Temporal Window
    _maximum_window_percentage = pt.get<double>("parameters.analysis.advanced.maximum-window-percentage", 50);
    _maximum_window_length = pt.get<unsigned int>("parameters.analysis.advanced.maximum-window-length", 1);
    _maximum_window_type = static_cast<MaximumWindowType>(pt.get<unsigned int>("parameters.analysis.advanced.maximum-window-type", PERCENTAGE_WINDOW));
    _minimum_window_length = pt.get<unsigned int>("parameters.analysis.advanced.minimum-window-length", 2);
    // Advanced Analysis - Inference
    _replications = pt.get<unsigned int>("parameters.analysis.advanced.replications", 999);
    _randomizationSeed = pt.get<unsigned int>("parameters.analysis-advanced.seed", static_cast<unsigned int>(RandomNumberGenerator::glDefaultSeed));
    _randomlyGenerateSeed = pt.get<bool>("parameters.analysis-advanced.generate-seed", false);
    // Output
    setOutputFileName(pt.get<std::string>("parameters.output.results-file", "").c_str(), true);
    _generateHtmlResults = pt.get<bool>("parameters.output.generate-html-results", true);
    _generateTableResults = pt.get<bool>("parameters.output.generate-table-results", true);
    _generate_llr_results = pt.get<bool>("parameters.output.generate-llr-results", true);
    _report_critical_values = pt.get<bool>("parameters.output.report-critical-values", false);
    // Power Evaluations
    _perform_power_evaluations = pt.get<bool>("parameters.power-evaluations.perform-power-evaluations", false);
    _power_evaluation_type = static_cast<PowerEvaluationType>(pt.get<unsigned int>("parameters.power-evaluations.power-evaluation-type", PE_WITH_ANALYSIS));
    _critical_values_type = static_cast<CriticalValuesType>(pt.get<unsigned int>("parameters.power-evaluations.critical-values-type", CV_MONTECARLO));
    _critical_value_05 = pt.get<double>("parameters.power-evaluations.critical-value-05", 0);
    _critical_value_01 = pt.get<double>("parameters.power-evaluations.critical-value-01", 0);
    _critical_value_001 = pt.get<double>("parameters.power-evaluations.critical-value-001", 0);
    _power_evaluation_totalcases = pt.get<int>("parameters.power-evaluations.totalcases", 0);
    _power_replica = pt.get<int>("parameters.power-evaluations.replications", _replications + 1);
    setPowerEvaluationAltHypothesisFilename(pt.get<std::string>("parameters.power-evaluations.alternative-hypothesis-file", "").c_str(), true);
    // Power Simulations
    _read_simulations = pt.get<bool>("parameters.power-simulations.input-simulations", true);
    setInputSimulationsFilename(pt.get<std::string>("parameters.power-simulations.input-simulations-file", "").c_str(), true);
    _write_simulations = pt.get<bool>("parameters.power-simulations.output-simulations", true);
    setOutputSimulationsFilename(pt.get<std::string>("parameters.power-simulations.output-simulations-file", "").c_str(), true);
    // Run Options
    _numRequestedParallelProcesses = pt.get<unsigned int>("parameters.run-options.processors", 0);
}

void Parameters::write(const std::string &filename, ParametersFormat type) const {
    using boost::property_tree::ptree;
    using boost::property_tree::xml_writer_settings;
    ptree pt;
    std::string buffer;

    // Input
    pt.put("parameters.input.tree-file", _treeFileName);
    pt.put("parameters.input.count-file", _countFileName);
    pt.put("parameters.input.data-time-range", _dataTimeRangeSet.toString(buffer));
    // Advanced Input
    pt.put("parameters.input.advanced.cuts-file", _cutsFileName);
    pt.put("parameters.input-advanced.cuts-type", static_cast<unsigned int>(_cut_type));
    pt.put("parameters.input.advanced.duplicates", _duplicates);
    // Analysis
    pt.put("parameters.analysis.scan", static_cast<unsigned int>(_scan_type));
    pt.put("parameters.analysis.conditional", static_cast<unsigned int>(_conditional_type));
    pt.put("parameters.analysis.probability-model", static_cast<unsigned int>(_modelType));
    pt.put("parameters.analysis.event-probability.numerator", _probablility_ratio.first);
    pt.put("parameters.analysis.event-probability.denominator", _probablility_ratio.second);
    pt.put("parameters.analysis.temporal-window.start-range", _temporalStartRange.toString(buffer));
    pt.put("parameters.analysis.temporal-window.end-range", _temporalEndRange.toString(buffer));
    // Advanced Analysis - Temporal Window
    pt.put("parameters.analysis.advanced.maximum-window-percentage", _maximum_window_percentage);
    pt.put("parameters.analysis.advanced.maximum-window-length", _maximum_window_length);
    pt.put("parameters.analysis.advanced.maximum-window-type", static_cast<unsigned int>(_maximum_window_type));
    pt.put("parameters.analysis.advanced.minimum-window-length", _minimum_window_length);
    // Advanced Analysis - Inference
    pt.put("parameters.analysis.advanced.replications", _replications);
    pt.put("parameters.analysis.advanced.seed", _randomizationSeed);
    pt.put("parameters.analysis.advanced.generate-seed", _randomlyGenerateSeed);
    // Output
    pt.put("parameters.output.results-file", _outputFileName);
    pt.put("parameters.output.generate-html-results", _generateHtmlResults);
    pt.put("parameters.output.generate-table-results", _generateTableResults);
    pt.put("parameters.output.generate-llr-results", _generate_llr_results);
    pt.put("parameters.output.report-critical-values", _report_critical_values);
    // Power Evaluations
    pt.put("parameters.power-evaluations.perform-power-evaluations", _perform_power_evaluations);
    pt.put("parameters.power-evaluations.power-evaluation-type", static_cast<unsigned int>(_power_evaluation_type));
    pt.put("parameters.power-evaluations.critical-values-type", static_cast<unsigned int>(_critical_values_type));
    pt.put("parameters.power-evaluations.critical-value-05", _critical_value_05);
    pt.put("parameters.power-evaluations.critical-value-01", _critical_value_01);
    pt.put("parameters.power-evaluations.critical-value-001", _critical_value_001);
    pt.put("parameters.power-evaluations.totalcases", _power_evaluation_totalcases);
    pt.put("parameters.power-evaluations.replications", _power_replica);
    pt.put("parameters.power-evaluations.alternative-hypothesis-file", _power_alt_hypothesis_filename);
    // Power Simulations
    pt.put("parameters.power-simulations.input-simulations", _read_simulations);
    pt.put("parameters.power-simulations.input-simulations-file", _input_sim_file);
    pt.put("parameters.power-simulations.output-simulations", _write_simulations);
    pt.put("parameters.power-simulations.output-simulations-file", _output_sim_file);
    // Run Options
    pt.put("parameters.run-options.processors", _numRequestedParallelProcesses);

    switch (type) {
        case JSON: write_json(filename, pt); break;
        case XML:  
        default: {
            //const xml_writer_settings<char> w(' ', 4);
            write_xml(filename, pt, std::locale());
        }
    }
}
