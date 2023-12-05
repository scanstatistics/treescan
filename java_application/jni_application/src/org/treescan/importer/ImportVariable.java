package org.treescan.importer;

/**
 * Class which defines a field which is an import variable. This class is used both by the import wizard dialog and importer class.
 * @author hostovic
 */
public class ImportVariable implements Cloneable {
    private String _label="";
    private int _index=-1;
    private boolean _required=true;
    private int _source_field_index=0;
    private String _help_text=null;
    private boolean _showing=true;
    private String _default=null;

    /**
     * class constructor
     * @param label - string to display as variable label
     * @param index - relative index of variable in list
     * @param required - is field required?
     * @param help_text - any additional text to label
     * @param default_value - default value, if none specified
     */
    public ImportVariable(String label, int index, boolean required, String help_text, String default_value) {
        _required = required;
        _label = label;
        _index = index;
        if (!_required) {
            _help_text = "optional";
        } else if (_help_text == null) {
            _help_text = help_text;
        }
        _default = default_value;
    }

    /**
     * @return newly cloned object
     */
    @Override
    public Object clone() {
        try {
            ImportVariable newObject = (ImportVariable) super.clone();
            newObject._label = new String(_label);
            newObject._help_text = new String(_help_text);
            newObject._default = _default == null ? _default : new String(_default);
            return newObject;
        } catch (CloneNotSupportedException e) {
            throw new InternalError("But we are Cloneable!!!");
        }
    }

    /**
     * @return whether variable is showing in wizard dialog
     */
    public boolean getShowing() {
        return _showing;
    }

    /**
     * @param b - set whether variable is showing or not in wizard dialog
     * @return set status
     */
    public boolean setShowing(boolean b) {
        _showing = b;
        return _showing;
    }
    
    /**
     * @return variable default string, could be null
     */
    public final String getDefault() {
        return _default;
    }
    
    /**
     * @return whether variable has default string
     */
    public final boolean hasDefault() {
        return _default != null;
    }    
    
    /**
     * Sets the variables mapped index into ImportDataSource record.
     * @param idx - set index in data source record
     */
    public void setSourceFieldIndex(int idx) {
        _source_field_index = idx;
    }

    /**
     * @return index in data source record (ImportDataSource)
     */
    public int getSourceFieldIndex() {
        return _source_field_index;
    }

    /**
     * @return whether this variable is mapped to field of ImportDataSource record
     */
    public boolean isMappedToSourceField() {
        return _source_field_index > 0;
    }

    /**
     * @return whether this field is required, used primarily in wizard dialog
     */
    public boolean getIsRequiredField() {
        return _required;
    }

    /**
     * Sets variables relative index in collection.
     * @param idx 
     */
    public void setVariableIndex(int idx) {
        _index = idx;
    }
    
    /**
     * @return variables relative index in collection.
     */
    public int getVariableIndex() {
        return _index;
    }

    /**
     * @return variable label, possibly with help text
     */
    public String getDisplayLabel() {
        StringBuilder builder = new StringBuilder();
        builder.append(_label);
        if (_help_text != null && _help_text.length() > 0) {
            builder.append(" (").append(_help_text).append(")");
        }
        return builder.toString();
    }

    /**
     * @return variable label
     */
    public final String getVariableName() {
        return _label;
    }
}
