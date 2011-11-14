package org.treescan.app;
import java.util.*;

public class Parameters implements Cloneable {	
    public native boolean Read(String filename);  
    public native void Write(String filename);
    public enum ResultsFormat {TEXT, HTML};
    public enum ModelType {POISSON, BERNOULLI};
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
    private String _treefilename="";
    private String _countfilename="";
    private String _outputfilename="";
    private ResultsFormat _resultsFormat=ResultsFormat.TEXT;
    private int _numprocesses=0;
    private int _replications=999;
    private int _randomizationSeed=12345678;
    private boolean _conditional=false;
    //private boolean _duplicates=false;
    private boolean _randomlyGenerateSeed=false;
    private boolean _printColumnHeaders;
    private ModelType _modelType=ModelType.POISSON;
    private int _probability_ratio_numerator=1;
    private int _probability_ratio_denominator=2;

    public Parameters() {
    	super();    	
    }
    @Override
    public Object clone() { 
      try {
    	  Parameters newObject = (Parameters)super.clone(); 
    	  newObject._sourcefilename = new String(_sourcefilename);
    	  newObject._treefilename = new String(_treefilename);
    	  newObject._countfilename = new String(_countfilename);
    	  newObject._outputfilename = new String(_outputfilename);
    	  return newObject; 
      } catch (CloneNotSupportedException e) {
        throw new InternalError("clone() failed!");
      }
    }   
    @Override
    public boolean equals(Object _rhs) {
    	  Parameters rhs = (Parameters)_rhs;    	  
    	  if (!_treefilename.equals(rhs._treefilename)) return false;
    	  if (!_countfilename.equals(rhs._countfilename)) return false;
    	  if (!_outputfilename.equals(rhs._outputfilename)) return false;
          if (_resultsFormat != rhs._resultsFormat) return false;
    	  if (!_sourcefilename.equals(rhs._sourcefilename)) return false;
    	  if (_replications != rhs._replications) return false;
    	  if (_conditional != rhs._conditional) return false;
    	  //if (_duplicates != rhs._duplicates) return false;
    	  if (_randomizationSeed != rhs._randomizationSeed) return false;
    	  if (_numprocesses != rhs._numprocesses) return false;
    	  if (_randomlyGenerateSeed != rhs._randomlyGenerateSeed) return false;
    	  if (_printColumnHeaders != rhs._printColumnHeaders) return false;
          if (_modelType != rhs._modelType) return false;
          if (_probability_ratio_numerator != rhs._probability_ratio_numerator) return false;
          if (_probability_ratio_denominator != rhs._probability_ratio_denominator) return false;
    	  return true;
    }
    public final CreationVersion getCreationVersion() {return _creationversion;}
    public void setCreationVersion(final CreationVersion v) {_creationversion = v;}
    public int getNumRequestedParallelProcesses() {return _numprocesses;}
    public void setNumProcesses(int i) {_numprocesses = i;}
    public int getNumReplicationsRequested() {return _replications;}
    public void setNumReplications(int i) {_replications = i;}
    public final String getSourceFileName() {return _sourcefilename;}
    public void setSourceFileName(final String s) {_sourcefilename = s;}
    public final String getTreeFileName() {return _treefilename;}
    public void setTreeFileName(final String s) {_treefilename = s;}
    public final String getCountFileName() {return _countfilename;}
    public void setCountFileName(final String s) {_countfilename = s;}
    public final String getOutputFileName() {return _outputfilename; }
    public void setOutputFileName(final String s) {_outputfilename = s;}
    public int getRandomizationSeed() {return _randomizationSeed;}
    public void setRandomizationSeed(int i) {_randomizationSeed = i;}
    public final boolean isRandomlyGeneratingSeed() {return _randomlyGenerateSeed;}
    public void setRandomlyGeneratingSeed(boolean b) {_randomlyGenerateSeed = b;}
    public final boolean isConditional() {return _conditional;}
    public void setConditional(boolean b) {_conditional = b;}
    //public final boolean isDuplicates() {return _duplicates;}
    //public void setDuplicates(boolean b) {_duplicates = b;}
    public final boolean isPrintColumnHeaders() {return _printColumnHeaders;}
    public void setPrintColunHeaders(boolean b) {_printColumnHeaders=b;}
    public ResultsFormat getResultsFormat() {return _resultsFormat;}
    public void setResultsFormat(int ord) {try {_resultsFormat = ResultsFormat.values()[ord];} catch (ArrayIndexOutOfBoundsException e) {ThrowEnumException(ord, ResultsFormat.values());}}
    public ModelType getModelType() {return _modelType;}
    public void setModelType(int ord) {try {_modelType = ModelType.values()[ord];} catch (ArrayIndexOutOfBoundsException e) {ThrowEnumException(ord, ModelType.values());}}
    public int getProbabilityRatioNumerator() {return _probability_ratio_numerator;}
    public void setProbabilityRatioNumerator(int i) {_probability_ratio_numerator = i;}
    public int getProbabilityRatioDenominator() {return _probability_ratio_denominator;}
    public void setProbabilityRatioDenominator(int i) {_probability_ratio_denominator = i;}
    public void ThrowEnumException(int ord, Enum[] e) {throw new RuntimeException("Ordinal index " + ord + " out of range [" + e[0].ordinal() + "," +  e[e.length - 1].ordinal() + "].");}
}
