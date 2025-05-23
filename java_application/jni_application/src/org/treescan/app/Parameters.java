package org.treescan.app;

import java.util.ArrayList;
import org.treescan.importer.InputSourceSettings;

public class Parameters implements Cloneable {
    public native boolean Read(String filename);
    public native void Write(String filename);
    static public native String getAlphaSpentToDateString(String filename);
    static public double getAlphaSpentToDate(String filename) {
        String spent = Parameters.getAlphaSpentToDateString(filename);
        double alpha = 0.0;
        for (String val : spent.split(","))
            if (!val.isEmpty()) alpha += Double.parseDouble(val);
        return alpha;
    }    
    static public int getLookNumber(String filename) {
        String spent = Parameters.getAlphaSpentToDateString(filename);
        return spent.split(",").length;
    } 
    public enum ResultsFormat {TEXT};
    public enum ModelType {POISSON, BERNOULLI_TREE, UNIFORM, MODEL_NOT_APPLICABLE, BERNOULLI_TIME};
    public enum ScanType {TREEONLY, TREETIME, TIMEONLY};
    public enum ConditionalType {UNCONDITIONAL, TOTALCASES, NODE, NODEANDTIME};
    public enum ScanRateType { HIGHRATE, LOWRATE, HIGHORLOWRATE };
    public enum MaximumWindowType {PERCENTAGE_WINDOW, FIXED_LENGTH};
    public enum PowerEvaluationType {PE_WITH_ANALYSIS, PE_ONLY_CASEFILE,PE_ONLY_SPECIFIED_CASES};
    public enum DatePrecisionType { NONE, GENERIC, YEAR, MONTH, DAY };
    public enum ProspectiveFrequency { DAILY, WEEKLY, MONTHLY, QUARTERLY, YEARLY };
    /** temporal graph reporting type */
    public enum TemporalGraphReportType {MLC_ONLY, X_MCL_ONLY, SIGNIFICANT_ONLY}    
    /** p-values reporting type */
    public enum PValueReportingType { STANDARD_PVALUE, TERMINATION_PVALUE };
    public class CreationVersion {
    	public int _major;
    	public int _minor;
    	public int _release;
    	public CreationVersion(int major, int minor, int release) {
            super();
            _major = major;
            _minor = minor;
            _release = release;
        }
    }
    private CreationVersion _creationversion = new CreationVersion(1,1,0);
    private String _sourcefilename="";
    private ArrayList<String> _treefilenames;
    private String _cutsfilename="";
    private String _countfilename="";
    private String _controlfilename="";
    private String _outputfilename="";
    private ResultsFormat _resultsFormat=ResultsFormat.TEXT;
    private int _numprocesses=0;
    private int _replications=999;
    private int _randomizationSeed=12345678;
    //private boolean _duplicates=false;
    private boolean _randomlyGenerateSeed=false;
    private boolean _generateHtmlResults=false;
    private boolean _generateTableResults=false;
    private boolean _generateNCBIAsnResults=false;
    private boolean _generateNewickFile=false;
    private boolean _printColumnHeaders=true;
    private ModelType _modelType=ModelType.POISSON;
    private ScanType _scanType=ScanType.TREEONLY;
    private ConditionalType _conditionalType=ConditionalType.UNCONDITIONAL;
    private String _probability_ratio_numerator="1";
    private String _probability_ratio_denominator="2";
    private ScanRateType _scan_rate_type=ScanRateType.HIGHRATE;

    private boolean _restrict_temporal_windows = false;
    private String _data_time_range_sstart = "0";
    private String _data_time_range_send = "0";
    
    private String _temporal_start_range_sbegin = "0";
    private String _temporal_start_range_sclose = "0";
    private String _temporal_end_range_sbegin = "0";
    private String _temporal_end_range_sclose = "0";
    
    private boolean _generate_llr_results=false;
    private double _maximum_window_percentage=50.0;
    private int _maximum_window_length=1;
    private MaximumWindowType _maximum_window_type=MaximumWindowType.PERCENTAGE_WINDOW;
    private int _minimum_window_length=2;
    private boolean _perform_power_evaluations=false;
    private PowerEvaluationType _power_evaluation_type=PowerEvaluationType.PE_WITH_ANALYSIS;
    private int _power_evaluation_totalcases=600;
    private int _power_replica=_replications+1;
    private String _power_alt_hypothesis_filename="";
    private boolean _dayofweek_adjustment=false;
    private boolean _report_attributable_risk=false;
    private int _attributable_risk_exposed=0;
    private boolean _self_control_design=false;
    private boolean _variable_case_probablility=false;
    private String _power_baseline_probability_ratio_numerator="1";
    private String _power_baseline_probability_ratio_denominator="2";
    private boolean _restrict_tree_levels=false;
    private String _restricted_tree_levels = "";
    private boolean _restrict_evaluated_tree_nodes=false;
    private String _not_evaluated_nodes_filename = "";
    private boolean _sequential_scan=false;
    private int _sequential_min_signal=3;
    private int _sequential_max_signal=200;
    private String _sequential_file="";
    private boolean _apply_risk_window_restriction=true;
    private double _risk_window_percentage=20.0;
    private boolean _apply_exclusion_ranges=false;
    private String _exclusion_time_ranges="";
    private double _sequential_alpha_overall = 0.05;
    private double _sequential_alpha_spending = 0.01;
    private DatePrecisionType _date_precision_type=DatePrecisionType.NONE;
    private boolean _prospective_analysis = false;
    private ProspectiveFrequency _prospective_frequency_type = ProspectiveFrequency.DAILY;
    private boolean _output_temporal_graph=false; // report temporal graph file
    private TemporalGraphReportType _temporal_graph_report_type=TemporalGraphReportType.MLC_ONLY; // which clusters to report in temporal graph
    private int _temporal_graph_report_count=1; // number of MLC clusters to graph with TemporalGraphReportType.X_MCL_ONLY
    private double _temporal_graph_report_cutoff=0.05; // P-Value used limit graphed clusters with TemporalGraphReportType.SIGNIFICANT_ONLY
    private int _minimum_highrate_nodes_cases=2; // minimum number of high rate nodes cases
    private boolean _data_only_on_leaves=true;
    private boolean _relaxed_study_data_period_checking=false;
    private boolean _allow_multi_parent_nodes=false;
    private boolean _allow_multiple_roots=true;
    private boolean _include_identical_parent_cuts=false;
    /* PValue Reporting variables */
    private PValueReportingType _pvalue_reporting_type=PValueReportingType.STANDARD_PVALUE; /** PValue reporting type */
    private int _early_term_threshold=50; /** early termination threshold */
    
    private ArrayList<InputSourceSettings>     _input_sources;

    public Parameters() {
    	super();
        _input_sources = new ArrayList<>();
        _treefilenames = new ArrayList<>();
        _treefilenames.add("");
    }
    @Override
    public Object clone() {
      try {
    	  Parameters newObject = (Parameters)super.clone();
    	  newObject._sourcefilename = new String(_sourcefilename);
          newObject._treefilenames = new ArrayList<String>(_treefilenames);
    	  newObject._cutsfilename = new String(_cutsfilename);
    	  newObject._countfilename = new String(_countfilename);
    	  newObject._controlfilename = new String(_controlfilename);
    	  newObject._outputfilename = new String(_outputfilename);
    	  newObject._power_alt_hypothesis_filename = new String(_power_alt_hypothesis_filename);
          newObject._sequential_file = new String(_sequential_file);
          newObject._exclusion_time_ranges = new String(_exclusion_time_ranges);
          newObject._input_sources = new ArrayList<InputSourceSettings>();
          for (InputSourceSettings iss : _input_sources) {
            newObject._input_sources.add(iss.clone());
          }
          newObject._restricted_tree_levels = new String(_restricted_tree_levels);
   	      newObject._data_time_range_sstart = new String(_data_time_range_sstart);
          newObject._data_time_range_send = new String(_data_time_range_send);
          newObject._temporal_start_range_sbegin = new String(_temporal_start_range_sbegin);
          newObject._temporal_start_range_sclose = new String(_temporal_start_range_sclose);
          newObject._temporal_end_range_sbegin = new String(_temporal_end_range_sbegin);
          newObject._temporal_end_range_sclose = new String(_temporal_end_range_sclose);
          newObject._not_evaluated_nodes_filename = new String(_not_evaluated_nodes_filename);
          newObject._probability_ratio_numerator = new String(_probability_ratio_numerator);
          newObject._probability_ratio_denominator = new String(_probability_ratio_denominator);
          newObject._power_baseline_probability_ratio_numerator = new String(_power_baseline_probability_ratio_numerator);
          newObject._power_baseline_probability_ratio_denominator = new String(_power_baseline_probability_ratio_denominator);
    	  return newObject;
      } catch (CloneNotSupportedException e) {
        throw new InternalError("clone() failed!");
      }
    }
    @Override
    public boolean equals(Object _rhs) {
    	  Parameters rhs = (Parameters)_rhs;
   	  if (_scanType != rhs._scanType) return false;
          if (_conditionalType != rhs._conditionalType) return false;
          if (_modelType != rhs._modelType) return false;
          if (!_probability_ratio_numerator.equals(rhs._probability_ratio_numerator)) return false;
          if (!_probability_ratio_denominator.equals(rhs._probability_ratio_denominator)) return false;
          if (_self_control_design != rhs._self_control_design) return false;
          if (_variable_case_probablility != rhs._variable_case_probablility) return false;
          if (_scan_rate_type != rhs._scan_rate_type) return false;
          if (_restrict_temporal_windows != rhs._restrict_temporal_windows) return false;
          if (!_temporal_start_range_sbegin.equals(rhs._temporal_start_range_sbegin)) return false;
          if (!_temporal_start_range_sclose.equals(rhs._temporal_start_range_sclose)) return false;
          if (!_temporal_end_range_sbegin.equals(rhs._temporal_end_range_sbegin)) return false;
          if (!_temporal_end_range_sclose.equals(rhs._temporal_end_range_sclose)) return false;
          if (_maximum_window_percentage != rhs._maximum_window_percentage) return false;
          if (_maximum_window_length != rhs._maximum_window_length) return false;
          if (_maximum_window_type != rhs._maximum_window_type) return false;
          if (_minimum_window_length != rhs._minimum_window_length) return false;
          if (_dayofweek_adjustment != rhs._dayofweek_adjustment) return false;
    	  if (_replications != rhs._replications) return false;
          if (_perform_power_evaluations != rhs._perform_power_evaluations) return false;
          if (_power_evaluation_type != rhs._power_evaluation_type) return false;
          if (_power_evaluation_totalcases != rhs._power_evaluation_totalcases) return false;
          if (_power_replica != rhs._power_replica) return false;
          if (!_power_alt_hypothesis_filename.equals(rhs._power_alt_hypothesis_filename)) return false;
          if (_sequential_scan != rhs._sequential_scan) return false;
          if (_sequential_min_signal != rhs._sequential_min_signal) return false;
          if (_sequential_max_signal != rhs._sequential_max_signal) return false;
          if (!_sequential_file.equals(rhs._sequential_file)) return false;
          if (_sequential_alpha_spending != rhs._sequential_alpha_spending) return false;
          if (_sequential_alpha_overall != rhs._sequential_alpha_overall) return false;

    	  if (!_treefilenames.equals(rhs._treefilenames)) return false;
    	  if (!_countfilename.equals(rhs._countfilename)) return false;
    	  if (!_controlfilename.equals(rhs._controlfilename)) return false;
          if (!_data_time_range_sstart.equals(rhs._data_time_range_sstart)) return false;
          if (!_data_time_range_send.equals(rhs._data_time_range_send)) return false;
          if (!_cutsfilename.equals(rhs._cutsfilename)) return false;
    	  if (_apply_risk_window_restriction != rhs._apply_risk_window_restriction) return false;
    	  if (_risk_window_percentage != rhs._risk_window_percentage) return false;
          if (_restrict_evaluated_tree_nodes != rhs._restrict_evaluated_tree_nodes) return false;
          if (!_not_evaluated_nodes_filename.equals(rhs._not_evaluated_nodes_filename)) return false;
    
    	  if (!_outputfilename.equals(rhs._outputfilename)) return false;
    	  if (_generateHtmlResults != rhs._generateHtmlResults) return false;
    	  if (_generateTableResults != rhs._generateTableResults) return false;
    	  if (_generateNCBIAsnResults != rhs._generateNCBIAsnResults) return false;
    	  if (_generateNewickFile != rhs._generateNewickFile) return false;
          if (_report_attributable_risk != rhs._report_attributable_risk) return false;
          if (_attributable_risk_exposed != rhs._attributable_risk_exposed) return false;
          if (_generate_llr_results != rhs._generate_llr_results) return false;

          if (_resultsFormat != rhs._resultsFormat) return false;
    	  if (!_sourcefilename.equals(rhs._sourcefilename)) return false;
          if (!_input_sources.equals(rhs._input_sources)) return false;
    	  if (_randomizationSeed != rhs._randomizationSeed) return false;
    	  if (_numprocesses != rhs._numprocesses) return false;
    	  if (_randomlyGenerateSeed != rhs._randomlyGenerateSeed) return false;
    	  if (_printColumnHeaders != rhs._printColumnHeaders) return false;

          if (!_power_baseline_probability_ratio_numerator.equals(rhs._power_baseline_probability_ratio_numerator)) return false;
          if (!_power_baseline_probability_ratio_denominator.equals(rhs._power_baseline_probability_ratio_denominator)) return false;
          if (_restrict_tree_levels != rhs._restrict_tree_levels) return false;
          if (!_restricted_tree_levels.equals(rhs._restricted_tree_levels)) return false;
          if (_apply_exclusion_ranges != rhs._apply_exclusion_ranges) return false;
          if (!_exclusion_time_ranges.equals(rhs._exclusion_time_ranges)) return false;
          
          if (_date_precision_type != rhs._date_precision_type) return false;
          if (_prospective_analysis != rhs._prospective_analysis) return false;
          if (_prospective_frequency_type != rhs._prospective_frequency_type) return false;

          if (_output_temporal_graph != rhs._output_temporal_graph) return false;
          if (_temporal_graph_report_count != rhs._temporal_graph_report_count) return false;
          if (_temporal_graph_report_cutoff != rhs._temporal_graph_report_cutoff) return false;
          if (_temporal_graph_report_type != rhs._temporal_graph_report_type) return false;          
          if (_minimum_highrate_nodes_cases != rhs._minimum_highrate_nodes_cases) return false;
          if (_data_only_on_leaves != rhs._data_only_on_leaves) return false;
          if (_relaxed_study_data_period_checking != rhs._relaxed_study_data_period_checking) return false;
          if (_allow_multi_parent_nodes != rhs._allow_multi_parent_nodes) return false;
          if (_allow_multiple_roots != rhs._allow_multiple_roots) return false;
          if (_include_identical_parent_cuts != rhs._include_identical_parent_cuts) return false;
          if (_pvalue_reporting_type != rhs._pvalue_reporting_type) return false;
          if (_early_term_threshold != rhs._early_term_threshold) return false;
          
          return true;
    }
    
    public boolean getVariableCaseProbability() { return _variable_case_probablility; }
    public void setVariableCaseProbability(boolean b) { _variable_case_probablility = b; }
    public final String getNotEvaluatedNodesFileName() {return _not_evaluated_nodes_filename;}
    public void setNotEvaluatedNodesFileName(final String s) {_not_evaluated_nodes_filename = s;}
    public boolean getRestrictEvaluatedTreeNodes() { return _restrict_evaluated_tree_nodes; }
    public void setRestrictEvaluatedTreeNodes(boolean b) { _restrict_evaluated_tree_nodes = b; }
    public boolean getIncludeIdenticalParentCuts() { return _include_identical_parent_cuts; }
    public void setIncludeIdenticalParentCuts(boolean b) { _include_identical_parent_cuts = b; }
    public boolean getAllowMultipleRoots() { return _allow_multiple_roots; }
    public void setAllowMultipleRoots(boolean b) { _allow_multiple_roots = b; }
    public boolean getAllowMultiParentNodes() { return _allow_multi_parent_nodes; }
    public void setAllowMultiParentNodes(boolean b) { _allow_multi_parent_nodes = b; }
    public PValueReportingType getPValueReportingType() { return _pvalue_reporting_type; }
    public void setPValueReportingType(int ord){try {_pvalue_reporting_type = PValueReportingType.values()[ord];} catch (ArrayIndexOutOfBoundsException e) {ThrowEnumException(ord, PValueReportingType.values());}}
    public int getEarlyTermThreshold() { return _early_term_threshold; }
    public void setEarlyTermThreshold(int i) { _early_term_threshold = i; }
    public boolean getRelaxedStudyDataPeriodChecking() { return _relaxed_study_data_period_checking; }
    public void setRelaxedStudyDataPeriodChecking(boolean b) { _relaxed_study_data_period_checking = b; }
    public boolean getDataOnlyOnLeaves() { return _data_only_on_leaves; }
    public void setDataOnlyOnLeaves(boolean b) { _data_only_on_leaves = b; }
    public ScanRateType getScanRateType() { return _scan_rate_type; }
    public void setScanRateType(int ord){try {_scan_rate_type = ScanRateType.values()[ord];} catch (ArrayIndexOutOfBoundsException e) {ThrowEnumException(ord, ScanRateType.values());}}
    public boolean isGeneratingNCBIAsnResults() { return _generateNCBIAsnResults; }
    public boolean isGeneratingNewickFile() { return _generateNewickFile; }    
    public void setGeneratingNCBIAsnResults(boolean b) { _generateNCBIAsnResults = b; }
    public void setGeneratingNewickFile(boolean b) { _generateNewickFile = b; }
    public int getMinimumHighRateNodeCases() {return _minimum_highrate_nodes_cases;}
    public void setMinimumHighRateNodeCases(int i) {_minimum_highrate_nodes_cases = i;}
    public boolean getOutputTemporalGraphFile() {return _output_temporal_graph;}
    public void setOutputTemporalGraphFile(boolean b) {_output_temporal_graph = b;}    
    public double getTemporalGraphSignificantCutoff() {return _temporal_graph_report_cutoff;}
    public void setTemporalGraphSignificantCutoff(double d) {_temporal_graph_report_cutoff = d;}
    public int getTemporalGraphMostLikelyCount() {return _temporal_graph_report_count;}
    public void setTemporalGraphMostLikelyCount(int i) {_temporal_graph_report_count = i;}
    public TemporalGraphReportType getTemporalGraphReportType() {return _temporal_graph_report_type;}
    public void setTemporalGraphReportType(int ord){try {_temporal_graph_report_type = TemporalGraphReportType.values()[ord];} catch (ArrayIndexOutOfBoundsException e) {ThrowEnumException(ord, TemporalGraphReportType.values());}}
    
    public ProspectiveFrequency getProspectiveFrequencyType() { return _prospective_frequency_type; }
    public void setProspectiveFrequencyType(ProspectiveFrequency e) { _prospective_frequency_type = e; }
    public void setProspectiveFrequencyType(int ord) {try {_prospective_frequency_type = ProspectiveFrequency.values()[ord];} catch (ArrayIndexOutOfBoundsException e) {ThrowEnumException(ord, ProspectiveFrequency.values());}}
    
    public boolean getRestrictTemporalWindows() { return _restrict_temporal_windows; }
    public void setRestrictTemporalWindows(boolean b) { _restrict_temporal_windows = b; }
    public boolean getIsProspectiveAnalysis() { return _prospective_analysis; }
    public void setIsProspectiveAnalysis(boolean b) { _prospective_analysis = b; }    
    
    public boolean isSequentialScanBernoulli() {
        return _sequential_scan &&
               _modelType == Parameters.ModelType.BERNOULLI_TREE &&
               _conditionalType == Parameters.ConditionalType.UNCONDITIONAL;
    }

    public boolean isSequentialScanPoisson() {
        return _sequential_scan && _modelType == Parameters.ModelType.POISSON;
    }
    
    public boolean isSequentialScanTreeOnly() {
        return isSequentialScanBernoulli() || isSequentialScanPoisson();
    }    
    
    public DatePrecisionType getPrecisionOfTimesType() {return _date_precision_type;}
    public void setPrecisionOfTimesType(DatePrecisionType e) { _date_precision_type = e;}
    public void setPrecisionOfTimesType(int ord) {try {_date_precision_type = DatePrecisionType.values()[ord];} catch (ArrayIndexOutOfBoundsException e) {ThrowEnumException(ord, DatePrecisionType.values());}}    
    
    public boolean isApplyingExclusionTimeRanges() {return _apply_exclusion_ranges;}
    public void setApplyingExclusionTimeRanges(boolean b) {_apply_exclusion_ranges = b;}
    public final String getExclusionTimeRangeSet() {return _exclusion_time_ranges;}
    public void setExclusionTimeRangeSet(final String s) {_exclusion_time_ranges = s;}

    public boolean isApplyingRiskWindowRestriction() { return _apply_risk_window_restriction; }
    public void setApplyingRiskWindowRestriction(boolean b) { _apply_risk_window_restriction = b; }
    public double getRiskWindowPercentage() { return _risk_window_percentage; }
    public void setRiskWindowPercentage(double d) { _risk_window_percentage = d; }
    public boolean getRestrictTreeLevels() {return _restrict_tree_levels;}
    public void setRestrictTreeLevels(boolean b) {_restrict_tree_levels = b;}
    public final String getRestrictedTreeLevels() {return _restricted_tree_levels;}
    public void setRestrictedTreeLevels(final String s) {_restricted_tree_levels = s;}

    public boolean getSequentialScan() {return _sequential_scan;}
    public void setSequentialScan(boolean b) {_sequential_scan = b;}

    public double getSequentialAlphaOverall() {return _sequential_alpha_overall;}
    public void setSequentialAlphaOverall(double d) {_sequential_alpha_overall = d;}
    public double getSequentialAlphaSpending() {return _sequential_alpha_spending;}
    public void setSequentialAlphaSpending(double d) {_sequential_alpha_spending = d;}

    public int getSequentialMinimumSignal() {return _sequential_min_signal;}
    public void setSequentialMinimumSignal(int i) {_sequential_min_signal = i;}
    public int getSequentialMaximumSignal() {return _sequential_max_signal;}
    public void setSequentialMaximumSignal(int i) {_sequential_max_signal = i;}
    public String getSequentialFilename() {return _sequential_file;}
    public void setSequentialFilename(String s) {_sequential_file = s;}

    public void addInputSourceSettings(InputSourceSettings iss) {_input_sources.add(iss);}
    public void clearInputSourceSettings() {_input_sources.clear();}
    public ArrayList<InputSourceSettings> getInputSourceSettings() {return _input_sources;}

    public boolean getSelfControlDesign() {return _self_control_design;}
    public void setSelfControlDesign(boolean b) {_self_control_design = b;}
    public boolean getReportAttributableRisk() {return _report_attributable_risk;}
    public int getAttributableRiskExposed() {return _attributable_risk_exposed;}
    public void setReportAttributableRisk(boolean b) {_report_attributable_risk = b;}
    public void setAttributableRiskExposed(int i) {_attributable_risk_exposed = i;}
    public boolean getPerformDayOfWeekAdjustment() {return _dayofweek_adjustment;}
    public void setPerformDayOfWeekAdjustment(boolean b) {_dayofweek_adjustment = b;}
    public boolean getPerformPowerEvaluations() {return _perform_power_evaluations;}
    public void setPerformPowerEvaluations(boolean b) {_perform_power_evaluations = b;}
    public PowerEvaluationType getPowerEvaluationType() {return _power_evaluation_type;}
    public void setPowerEvaluationType(int ord) {try {_power_evaluation_type = PowerEvaluationType.values()[ord];} catch (ArrayIndexOutOfBoundsException e) {ThrowEnumException(ord, PowerEvaluationType.values());}}
    public int getPowerEvaluationTotalCases() {return _power_evaluation_totalcases;}
    public void setPowerEvaluationTotalCases(int i) {_power_evaluation_totalcases = i;}
    public int getPowerEvaluationReplications() {return _power_replica;}
    public void setPowerEvaluationReplications(int i) {_power_replica = i;}
    public String getPowerEvaluationAltHypothesisFilename() {return _power_alt_hypothesis_filename;}
    public void setPowerEvaluationAltHypothesisFilename(String s) {_power_alt_hypothesis_filename = s;}

    public double getMaximumWindowPercentage() {return _maximum_window_percentage;}
    public void setMaximumWindowPercentage(double d) {_maximum_window_percentage = d;}
    public int getMaximumWindowLength() {return _maximum_window_length;}
    public void setMaximumWindowLength(int u) {_maximum_window_length = u;}
    public MaximumWindowType getMaximumWindowType() {return _maximum_window_type;}
    public void setMaximumWindowType(MaximumWindowType e) {_maximum_window_type = e;}
    public void setMaximumWindowType(int ord) {try {_maximum_window_type = MaximumWindowType.values()[ord];} catch (ArrayIndexOutOfBoundsException e) {ThrowEnumException(ord, MaximumWindowType.values());}}
    public int getMinimumWindowLength() {return _minimum_window_length;}
    public void setMinimumWindowLength(int u) {_minimum_window_length = u;}

    public String getDataTimeRangeBegin() { return _data_time_range_sstart; }
    public void setDataTimeRangeBegin(final String text) {_data_time_range_sstart = text;}
    public String getDataTimeRangeClose() { return _data_time_range_send; }
    public void setDataTimeRangeClose(final String text) {_data_time_range_send = text;}
    
    public final String getStartRangeEndDate() {return _temporal_start_range_sclose;}
    public void setStartRangeEndDate(final String text) {_temporal_start_range_sclose = text;}
    public final String getStartRangeStartDate() {return _temporal_start_range_sbegin;}
    public void setStartRangeStartDate(final String text) {_temporal_start_range_sbegin = text;}    
    public final String getEndRangeEndDate() {return _temporal_end_range_sclose;}
    public void setEndRangeEndDate(final String text) {_temporal_end_range_sclose = text;}
    public final String getEndRangeStartDate() {return _temporal_end_range_sbegin;}   
    public void setEndRangeStartDate(final String text) {_temporal_end_range_sbegin = text;}       

    public final CreationVersion getCreationVersion() {return _creationversion;}
    public void setCreationVersion(final CreationVersion v) {_creationversion = v;}
    public int getNumRequestedParallelProcesses() {return _numprocesses;}
    public void setNumProcesses(int i) {_numprocesses = i;}
    public int getNumReplicationsRequested() {return _replications;}
    public void setNumReplications(int i) {_replications = i;}
    public final String getSourceFileName() {return _sourcefilename;}
    public void setSourceFileName(final String s) {_sourcefilename = s;}

    public final String getTreeFileName(int idx/*=1*/) {
        return _treefilenames.get(idx - 1);
    }
    public final ArrayList<String> getTreeFileNames() {return _treefilenames;}
    public void setTreeFileName(final String filename, int idx/*=1*/) {
        while (idx > _treefilenames.size())
            _treefilenames.add("");
        _treefilenames.set(idx - 1, filename);
    }
    public final String getCutsFileName() {return _cutsfilename;}
    public void setCutsFileName(final String s) {_cutsfilename = s;}
    public final String getCountFileName() {return _countfilename;}
    public void setCountFileName(final String s) {_countfilename = s;}
    public final String getControlFileName() {return _controlfilename;}
    public void setControlFileName(final String s) {_controlfilename = s;}
    public final String getOutputFileName() {return _outputfilename; }
    public void setOutputFileName(final String s) {_outputfilename = s;}
    public int getRandomizationSeed() {return _randomizationSeed;}
    public void setRandomizationSeed(int i) {_randomizationSeed = i;}
    public final boolean isRandomlyGeneratingSeed() {return _randomlyGenerateSeed;}
    public void setRandomlyGeneratingSeed(boolean b) {_randomlyGenerateSeed = b;}
    //public final boolean isDuplicates() {return _duplicates;}
    //public void setDuplicates(boolean b) {_duplicates = b;}
    public final boolean isGeneratingHtmlResults() {return _generateHtmlResults;}
    public void setGeneratingHtmlResults(boolean b) {_generateHtmlResults=b;}
    public final boolean isGeneratingTableResults() {return _generateTableResults;}
    public void setGeneratingTableResults(boolean b) {_generateTableResults=b;}
    public final boolean isPrintColumnHeaders() {return _printColumnHeaders;}
    public void setPrintColunHeaders(boolean b) {_printColumnHeaders=b;}
    public ResultsFormat getResultsFormat() {return _resultsFormat;}
    public void setResultsFormat(int ord) {try {_resultsFormat = ResultsFormat.values()[ord];} catch (ArrayIndexOutOfBoundsException e) {ThrowEnumException(ord, ResultsFormat.values());}}
    public ModelType getModelType() {return _modelType;}
    public void setModelType(int ord) {
        try {
            _modelType = ModelType.values()[ord];
            // reset model not applicable to uniform in the gui
            if (_modelType == ModelType.MODEL_NOT_APPLICABLE)  _modelType = ModelType.UNIFORM;
        } catch (ArrayIndexOutOfBoundsException e) {ThrowEnumException(ord, ModelType.values());}
    }
    public String getProbabilityRatioNumerator() {return _probability_ratio_numerator;}
    public void setProbabilityRatioNumerator(String s) {_probability_ratio_numerator = s;}
    public String getProbabilityRatioDenominator() {return _probability_ratio_denominator;}
    public void setProbabilityRatioDenominator(String s) {_probability_ratio_denominator = s;}
    public void ThrowEnumException(int ord, Enum[] e) { throw new RuntimeException("Ordinal index " + ord + " out of range [" + e[0].ordinal() + "," +  e[e.length - 1].ordinal() + "]."); }
    public ScanType getScanType() {return _scanType;}
    public void setScanType(int ord) {try {_scanType = ScanType.values()[ord];} catch (ArrayIndexOutOfBoundsException e) {ThrowEnumException(ord, ScanType.values());}}
    public ConditionalType getConditionalType() {return _conditionalType;}
    public void setConditionalType(int ord) {try { _conditionalType = ConditionalType.values()[ord]; } catch (ArrayIndexOutOfBoundsException e) { ThrowEnumException(ord, ConditionalType.values()); } }
    public final boolean isGeneratingLLRResults() {return _generate_llr_results;}
    public void setGeneratingLLRResults(boolean b) {_generate_llr_results=b;}

    public String getPowerBaselineProbabilityRatioNumerator() {return _power_baseline_probability_ratio_numerator;}
    public void setPowerBaselineProbabilityRatioNumerator(String i) {_power_baseline_probability_ratio_numerator = i;}
    public String getPowerBaselineProbabilityRatioDenominator() {return _power_baseline_probability_ratio_denominator;}
    public void setPowerBaselineProbabilityRatioDenominator(String i) {_power_baseline_probability_ratio_denominator = i;}
}
