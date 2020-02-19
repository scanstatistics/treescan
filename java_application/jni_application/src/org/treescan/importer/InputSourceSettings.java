package org.treescan.importer;

import java.util.ArrayList;
import org.treescan.app.UnknownEnumException;

/**
 * Class that defines how to read an input data source. This class largely
 * approximates the C++ class Parameters:InputSource.
 * @author hostovic
 */
public class InputSourceSettings implements Cloneable  {
    public enum SourceDataFileType {CSV, Excel97_2003, Excel};
    public enum InputFileType {Tree, Counts, Cut, Power_Evaluations, Controls};
    
    private SourceDataFileType _source_type=SourceDataFileType.CSV;
    private InputFileType _file_type=InputFileType.Tree;
    private ArrayList<String> _mappings = new ArrayList<String>();
    // CVS specific options
    private String _delimiter=",";
    private String _groupmarker="\"";
    private int _skip_lines=0;
    private boolean _first_row_headers=false;
    private int _index=1;
    
    /**
     * Default constructor -- called in JNI code.
     */
    public InputSourceSettings() {}
    
    /**
     * constructor
     * @param filetype 
     */
    public InputSourceSettings(InputFileType filetype) {
        _file_type = filetype;
    }
    
    /**
     * copy constructor
     * @param inputsource 
     */
    public InputSourceSettings(InputSourceSettings inputsource) {
        _source_type = inputsource._source_type;
        _file_type = inputsource._file_type;
        _mappings = new ArrayList<String>(inputsource._mappings);
        _delimiter = inputsource._delimiter;
        _groupmarker = inputsource._groupmarker;
        _skip_lines = inputsource._skip_lines;
        _first_row_headers = inputsource._first_row_headers;
        _index = inputsource._index;
    }
    
    @Override
    public InputSourceSettings clone() throws CloneNotSupportedException {
        InputSourceSettings iss = (InputSourceSettings) super.clone();
        iss._mappings = (ArrayList<String>)_mappings.clone();
        iss._delimiter = new String(_delimiter);
        iss._groupmarker = new String(_groupmarker);
        return iss;
    }
    
    public void copy(InputSourceSettings other) {
        _source_type = other._source_type;
        _file_type = other._file_type;
        _mappings = other._mappings;
        _delimiter = other._delimiter;
        _groupmarker = other._groupmarker;
        _skip_lines = other._skip_lines;
        _first_row_headers = other._first_row_headers;
        _index = other._index;
    }
    
    @Override
    public boolean equals(Object _rhs) {
        InputSourceSettings other = (InputSourceSettings)_rhs;
        if (_source_type != other._source_type) return false;
        if (_file_type != other._file_type) return false;
        if (!_mappings.equals(other._mappings)) return false;
        if (_index != other._index) return false;
        switch (_source_type) {
            case CSV : 
                if (!_delimiter.equals(other._delimiter)) return false;
                if (!_groupmarker.equals(other._groupmarker)) return false;
                if (_skip_lines != other._skip_lines) return false;
                if (_first_row_headers != other._first_row_headers) return false;
                break;
            case Excel :
            case Excel97_2003 : break;
           default: throw new UnknownEnumException(_source_type);
        }                
        return true;
    }

    @Override
    public int hashCode() {
        int hash = 7;
        hash = 31 * hash + (this._source_type != null ? this._source_type.hashCode() : 0);
        hash = 31 * hash + (this._file_type != null ? this._file_type.hashCode() : 0);
        hash = 31 * hash + (this._mappings != null ? this._mappings.hashCode() : 0);
        hash = 31 * hash + (this._delimiter != null ? this._delimiter.hashCode() : 0);
        hash = 31 * hash + (this._groupmarker != null ? this._groupmarker.hashCode() : 0);
        hash = 31 * hash + this._skip_lines;
        hash = 31 * hash + (this._first_row_headers ? 1 : 0);
        hash = 31 * hash + _index;
        return hash;
    }
    
    public void ThrowOrdinalIndexException(int iInvalidOrdinal, Enum[] e) {
        throw new RuntimeException("Ordinal index " + iInvalidOrdinal + " out of range [" +  e[0].ordinal() + "," +  e[e.length - 1].ordinal() + "].");
    }    
    
    public int getIndex() {return _index;}
    public void setIndex(int i) {_index = i;}
    
    public SourceDataFileType getSourceDataFileType() {return _source_type;}
    public void setSourceDataFileType(int iOrdinal) {
        try { setSourceDataFileType(SourceDataFileType.values()[iOrdinal]);
        } catch (ArrayIndexOutOfBoundsException e) { 
            ThrowOrdinalIndexException(iOrdinal, SourceDataFileType.values()); 
        }
    }
    public void setSourceDataFileType(SourceDataFileType e) {_source_type = e;}
    
    public InputFileType getInputFileType() {return _file_type;}
    public void setInputFileType(int iOrdinal) {
        try { setInputFileType(InputFileType.values()[iOrdinal]);
        } catch (ArrayIndexOutOfBoundsException e) { 
            ThrowOrdinalIndexException(iOrdinal, InputFileType.values()); 
        }
    }        
    public void setInputFileType(InputFileType e) {_file_type = e;}

    public ArrayList<String> getFieldMaps() {return _mappings;}
    public void addFieldMapping(String s) {_mappings.add(s);}    
    public void setFieldMaps(ArrayList<String> v) {_mappings = new ArrayList<String>(v);}
        
    public String getDelimiter() {return _delimiter;}
    public void setDelimiter(String s) {_delimiter = s;}
    
    public String getGroup() {return _groupmarker;}
    public void setGroup(String s) {_groupmarker = s;}
    
    public int getSkiplines() {return _skip_lines;}
    public void setSkiplines(int i) {_skip_lines = i;}
    
    public boolean getFirstRowHeader() {return _first_row_headers;}
    public void setFirstRowHeader(boolean b) {_first_row_headers = b;}
    
    public boolean isSet() {return _mappings.size() > 0;}
    public void reset() {
        _source_type=SourceDataFileType.CSV;
        _mappings.clear();
        _delimiter=",";
        _groupmarker="\"";
        _skip_lines=0;
        _first_row_headers=false;        
    }
}
