/*
 * ImportVariable.java
 *
 * Created on December 14, 2007, 3:45 PM
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */
package org.treescan.importer;

/**
 *
 * @author Hostovic
 */
/**
 * Import variable.
 */
public class ImportVariable implements Cloneable {

    private String gsVariableName;
    private int gwTargetFieldIndex;
    private boolean gbRequiredVariable;
    private int gwInputFileVariableIndex;
    private String gsHelpText;

    public ImportVariable() {
        initialize();
    }

    public ImportVariable(String sVariableName, int wTargetFieldIndex, boolean bRequiredVariable, String sHelpText) {
        initialize();
        gbRequiredVariable = bRequiredVariable;
        gsVariableName = sVariableName;
        gwTargetFieldIndex = wTargetFieldIndex;
        if (!gbRequiredVariable) {
            gsHelpText = "optional";
        } else if (gsHelpText == null) {
            gsHelpText = sHelpText;
        }
    }

    private void initialize() {
        gwTargetFieldIndex = -1;
        gbRequiredVariable = true;
        gwInputFileVariableIndex = 0;
    }

    @Override
    public Object clone() {
        try {
            ImportVariable newObject = (ImportVariable) super.clone();
            newObject.gsVariableName = new String(gsVariableName);
            newObject.gsHelpText = new String(gsHelpText);
            return newObject;
        } catch (CloneNotSupportedException e) {
            throw new InternalError("But we are Cloneable!!!");
        }
    }

    public final String getHelpText() {
        return gsHelpText;
    }

    public int getInputFileVariableIndex() {
        return gwInputFileVariableIndex;
    }

    public boolean getIsMappedToInputFileVariable() {
        return gwInputFileVariableIndex > 0;
    }

    public boolean getIsRequiredField() {
        return gbRequiredVariable;
    }

    public int getTargetFieldIndex() {
        return gwTargetFieldIndex;
    }

    public String getVariableDisplayName() {
        String Value;
        if (gsHelpText != null && gsHelpText.length() > 0) {
            Value = gsVariableName + " (" + gsHelpText + ")";
        } else {
            Value = gsVariableName;
        }
        return Value;
    }

    public final String getVariableName() {
        return gsVariableName;
    }

    public void setInputFileVariableIndex(int wInputFileVariableIndex) {
        gwInputFileVariableIndex = wInputFileVariableIndex;
    }
}
