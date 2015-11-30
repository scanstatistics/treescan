package org.treescan.app;

import java.util.ArrayList;
import org.treescan.importer.InputSourceSettings;

public class Parameters implements Cloneable {	
    public native boolean Read(String filename);  
    public native void Write(String filename);
    public enum ResultsFormat {TEXT};
    public enum ModelType {POISSON, BERNOULLI, UNIFORM, MODEL_NOT_APPLICABLE};
    public enum ScanType {TREEONLY, TREETIME, TIMEONLY};
    public enum ConditionalType {UNCONDITIONAL, TOTALCASES, NODE, NODEANDTIME};
    public enum MaximumWindowType {PERCENTAGE_WINDOW, FIXED_LENGTH};
    public enum PowerEvaluationType {PE_WITH_ANALYSIS, PE_ONLY_CASEFILE,PE_ONLY_SPECIFIED_CASES};    
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
    private String _outputfilename="";
    private ResultsFormat _resultsFormat=ResultsFormat.TEXT;
    private int _numprocesses=0;
    private int _replications=999;
    private int _randomizationSeed=12345678;
    //private boolean _duplicates=false;
    private boolean _randomlyGenerateSeed=false;
    private boolean _generateHtmlResults=false;
    private boolean _generateTableResults=false;
    private boolean _printColumnHeaders=true;
    private ModelType _modelType=ModelType.POISSON;
    private ScanType _scanType=ScanType.TREEONLY;
    private ConditionalType _conditionalType=ConditionalType.UNCONDITIONAL;
    private int _probability_ratio_numerator=1;
    private int _probability_ratio_denominator=2;
    private int _data_time_range_start=0;
    private int _data_time_range_end=0;
    private int _temporal_start_range_begin=0;
    private int _temporal_start_range_close=0;
    private int _temporal_end_range_begin=0;
    private int _temporal_end_range_close=0;
    private boolean _generate_llr_results=false;
    private boolean _report_critical_values=false;
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

    private ArrayList<InputSourceSettings>     _input_sources;
    
    public Parameters() {
    	super();
        _input_sources = new ArrayList<InputSourceSettings>();
        _treefilenames = new ArrayList<String>();
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
    	  newObject._outputfilename = new String(_outputfilename);
    	  newObject._power_alt_hypothesis_filename = new String(_power_alt_hypothesis_filename);
          newObject._input_sources = new ArrayList<InputSourceSettings>();
          for (InputSourceSettings iss : _input_sources) {
            newObject._input_sources.add(iss.clone());
          }          
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
          if (_probability_ratio_numerator != rhs._probability_ratio_numerator) return false;
          if (_probability_ratio_denominator != rhs._probability_ratio_denominator) return false;
          if (_self_control_design != rhs._self_control_design) return false;
   	  if (_temporal_start_range_begin != rhs._temporal_start_range_begin) return false;
   	  if (_temporal_start_range_close != rhs._temporal_start_range_close) return false;
   	  if (_temporal_end_range_begin != rhs._temporal_end_range_begin) return false;
   	  if (_temporal_end_range_close != rhs._temporal_end_range_close) return false;
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
          
    	  if (!_treefilenames.equals(rhs._treefilenames)) return false;
    	  if (!_countfilename.equals(rhs._countfilename)) return false;
   	  if (_data_time_range_start != rhs._data_time_range_start) return false;
   	  if (_data_time_range_end != rhs._data_time_range_end) return false;
          if (!_cutsfilename.equals(rhs._cutsfilename)) return false;
          
    	  if (!_outputfilename.equals(rhs._outputfilename)) return false;
    	  if (_generateHtmlResults != rhs._generateHtmlResults) return false;
    	  if (_generateTableResults != rhs._generateTableResults) return false;
          if (_report_attributable_risk != rhs._report_attributable_risk) return false;
          if (_attributable_risk_exposed != rhs._attributable_risk_exposed) return false;
          if (_generate_llr_results != rhs._generate_llr_results) return false;
          if (_report_critical_values != rhs._report_critical_values) return false;
          
          if (_resultsFormat != rhs._resultsFormat) return false;
    	  if (!_sourcefilename.equals(rhs._sourcefilename)) return false;
          if (!_input_sources.equals(rhs._input_sources)) return false;
    	  if (_randomizationSeed != rhs._randomizationSeed) return false;
    	  if (_numprocesses != rhs._numprocesses) return false;
    	  if (_randomlyGenerateSeed != rhs._randomlyGenerateSeed) return false;
    	  if (_printColumnHeaders != rhs._printColumnHeaders) return false;
          
          return true;
    }
    
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
    public boolean getReportCriticalValues() {return _report_critical_values;}
    public void setReportCriticalValues(boolean b) {_report_critical_values = b;}
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
    
    public int getDataTimeRangeBegin() {return _data_time_range_start;}
    public void setDataTimeRangeBegin(int i) { _data_time_range_start = i;}
    public int getDataTimeRangeClose() {return _data_time_range_end;}
    public void setDataTimeRangeClose(int i) {_data_time_range_end = i;}
    
    public int getTemporalStartRangeBegin() {return _temporal_start_range_begin;}
    public void setTemporalStartRangeBegin(int i) {_temporal_start_range_begin = i;}
    public int getTemporalStartRangeClose() {return _temporal_start_range_close;}
    public void setTemporalStartRangeClose(int i) {_temporal_start_range_close = i;}
    
    public int getTemporalEndRangeBegin() {return _temporal_end_range_begin;}
    public void setTemporalEndRangeBegin(int i) {_temporal_end_range_begin = i;}
    
    public int getTemporalEndRangeClose() {return _temporal_end_range_close;}
    public void setTemporalEndRangeClose(int i) {_temporal_end_range_close = i;}
    
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
    public int getProbabilityRatioNumerator() {return _probability_ratio_numerator;}
    public void setProbabilityRatioNumerator(int i) {_probability_ratio_numerator = i;}
    public int getProbabilityRatioDenominator() {return _probability_ratio_denominator;}
    public void setProbabilityRatioDenominator(int i) {_probability_ratio_denominator = i;}
    public void ThrowEnumException(int ord, Enum[] e) { throw new RuntimeException("Ordinal index " + ord + " out of range [" + e[0].ordinal() + "," +  e[e.length - 1].ordinal() + "]."); }
    public ScanType getScanType() {return _scanType;}
    public void setScanType(int ord) {try {_scanType = ScanType.values()[ord];} catch (ArrayIndexOutOfBoundsException e) {ThrowEnumException(ord, ScanType.values());}}
    public ConditionalType getConditionalType() {return _conditionalType;}
    public void setConditionalType(int ord) {try { _conditionalType = ConditionalType.values()[ord]; } catch (ArrayIndexOutOfBoundsException e) { ThrowEnumException(ord, ConditionalType.values()); } }
    public final boolean isGeneratingLLRResults() {return _generate_llr_results;}
    public void setGeneratingLLRResults(boolean b) {_generate_llr_results=b;}    
}
