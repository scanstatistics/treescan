package org.treescan.gui;

import java.awt.CardLayout;
import java.awt.Component;
import java.beans.PropertyVetoException;
import java.io.File;
import java.time.LocalDate;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import javax.swing.JOptionPane;
import javax.swing.JRootPane;
import javax.swing.event.UndoableEditEvent;
import javax.swing.event.UndoableEditListener;
import org.treescan.app.AdvFeaturesExpection;
import org.treescan.utils.FileAccess;
import org.treescan.app.Parameters;
import org.treescan.app.UnknownEnumException;
import org.treescan.gui.utils.DateComponentsGroup;
import org.treescan.gui.utils.FileSelectionDialog;
import org.treescan.gui.utils.InputFileFilter;
import org.treescan.gui.utils.Utils;
import org.treescan.importer.InputSourceSettings;

/**
 * Parameter settings window.
 *
 * @author Hostovic
 */
public class ParameterSettingsFrame extends AbstractParameterSettingsFrame {

    final static String STUDY_COMPLETE = "study_complete";
    final static String STUDY_GENERIC = "study_generic";
    private DateComponentsGroup _startDateComponentsGroup;
    private DateComponentsGroup _endDateComponentsGroup;
    //private final UndoManager undo = new UndoManager();    
    private AdvancedParameterSettingsFrame _advancedParametersSetting;

    /**
     * Creates new form ParameterSettingsFrame
     * @param rootPane
     * @param parameters
     */
    public ParameterSettingsFrame(final JRootPane rootPane, Parameters parameters) {
        super(rootPane, parameters);
        enableSettingsForStatisticModelCombination();
        enableAdvancedButtons();
    }

    @Override
    protected void initFrameComponents() {
        initComponents();
    }

    /* Returns reference to associated advanced parameters frame. */
    private AdvancedParameterSettingsFrame getAdvancedParameterInternalFrame() {
        return _advancedParametersSetting;
    }

    /* enables correct advanced settings button on Analysis and Output tabs */
    public void enableAdvancedButtons() {
        // Input tab Advanced button
        if (!getAdvancedParameterInternalFrame().getDefaultsSetForInputOptions()) {
            _advancedInputButton.setFont(new java.awt.Font("Tahoma", java.awt.Font.BOLD, 11));
        } else {
            _advancedInputButton.setFont(new java.awt.Font("Tahoma", java.awt.Font.PLAIN, 11));
        }
        // Analysis tab Advanced button
        if (!getAdvancedParameterInternalFrame().getDefaultsSetForAnalysisOptions()) {
            _advancedAnalysisButton.setFont(new java.awt.Font("Tahoma", java.awt.Font.BOLD, 11));
        } else {
            _advancedAnalysisButton.setFont(new java.awt.Font("Tahoma", java.awt.Font.PLAIN, 11));
        }
        // Output tab Advanced button
        if (!getAdvancedParameterInternalFrame().getDefaultsSetForOutputOptions()) {
            _advancedOutputButton.setFont(new java.awt.Font("Tahoma", java.awt.Font.BOLD, 11));
        } else {
            _advancedOutputButton.setFont(new java.awt.Font("Tahoma", java.awt.Font.PLAIN, 11));
        }
    }

    /* sets precision of times type control for DatePrecisionType */
    public void setPrecisionOfTimesControl(Parameters.DatePrecisionType eDatePrecisionType) {
        switch (eDatePrecisionType) {
            case YEAR:
                _timePrecisionYear.setSelected(true);
                break;
            case MONTH:
                _timePrecisionMonth.setSelected(true);
                break;
            case DAY:
                _timePrecisionDay.setSelected(true);
                break;
            case GENERIC : 
                _timePrecisionGeneric.setSelected(true);    
                break;
            case NONE:                
            default:
                _timePrecisionNone.setSelected(true);
        }
    }    
    
    /* returns precision of time type for precision control index */
    public Parameters.DatePrecisionType getPrecisionOfTimesControlType() {
        Parameters.DatePrecisionType eReturn = null;
        if (_timePrecisionNone.isSelected() == true) {
            eReturn = Parameters.DatePrecisionType.NONE;
        } else if (_timePrecisionYear.isSelected() == true) {
            eReturn = Parameters.DatePrecisionType.YEAR;
        } else if (_timePrecisionMonth.isSelected() == true) {
            eReturn = Parameters.DatePrecisionType.MONTH;
        } else if (_timePrecisionDay.isSelected() == true) {
            eReturn = Parameters.DatePrecisionType.DAY;
        } else if (_timePrecisionGeneric.isSelected() == true) {
            eReturn = Parameters.DatePrecisionType.GENERIC;
        } else {
            throw new RuntimeException("Unable to determine precision of times type.");
        }
        return eReturn;
    }    
    
    private void onCountTimePrecisionChange() {
        Parameters.DatePrecisionType eDatePrecisionType = getPrecisionOfTimesControlType();

        _treeOnlyScanType.setEnabled(eDatePrecisionType == Parameters.DatePrecisionType.NONE);
        _treetimeScanType.setEnabled(eDatePrecisionType != Parameters.DatePrecisionType.NONE);
        _timeonlyScanType.setEnabled(eDatePrecisionType != Parameters.DatePrecisionType.NONE);
        if ((!_timeonlyScanType.isEnabled() && _timeonlyScanType.isSelected()) || (!_treetimeScanType.isEnabled() && _treetimeScanType.isSelected()))
            _treeOnlyScanType.setSelected(true);
        else if (!_treeOnlyScanType.isEnabled() && _treeOnlyScanType.isSelected())
            _treetimeScanType.setSelected(true); 
        
        //enableSettingsForStatisticModelCombination();
        
        enableDatesByTimePrecisionUnits();
        getAdvancedParameterInternalFrame().enableAdjustmentsOptions();
    }    
    
    /* enabled study period and prospective date precision based on time interval unit */
    private void enableDatesByTimePrecisionUnits() {
        CardLayout cl = (CardLayout)(_studyPeriodGroup.getLayout());
        switch (getPrecisionOfTimesControlType()) {
            case NONE:
                enableStudyPeriodDates(false, false, false);
                enableStudyPeriodGeneric(false, false);
                break;
            case DAY:
                enableStudyPeriodDates(true, true, true);
                cl.show(_studyPeriodGroup, STUDY_COMPLETE);
                break;
            case YEAR:
                enableStudyPeriodDates(true, false, false);
                cl.show(_studyPeriodGroup, STUDY_COMPLETE);
                break;
            case MONTH:
                enableStudyPeriodDates(true, true, false);
                cl.show(_studyPeriodGroup, STUDY_COMPLETE);
                break;
            case GENERIC:
                cl.show(_studyPeriodGroup, STUDY_GENERIC);
                enableStudyPeriodGeneric(true, true);
                break;
            default:
                throw new UnknownEnumException(getPrecisionOfTimesControlType());
        }
        getAdvancedParameterInternalFrame().enableDatesByTimePrecisionUnits();
    }    

    /* enables or disables the study period group for generic date/day controls */
    private void enableStudyPeriodGeneric(boolean enableStart, boolean enableEnd) {
        _studyPeriodStartDateGenericTextField.setEnabled(enableStart);
        _studyPeriodEndDateGenericTextField.setEnabled(enableEnd);        
    }    
    /* enables or disables the study period group controls */
    private void enableStudyPeriodDates(boolean enableYear, boolean enableMonth, boolean enableDay) {
        //enable study period year controls
        _studyPeriodStartDateYearTextField.setEnabled(enableYear);
        _studyPeriodEndDateYearTextField.setEnabled(enableYear);
        
        // Start date month and day values.
        if (_studyPeriodStartDateMonthTextField.isEnabled() && !enableMonth) {
            // Start date month is going from enabled to disabled. Save month as January.
            _startDateComponentsGroup.setMonth(1);
        } else if (!_studyPeriodStartDateMonthTextField.isEnabled() && enableMonth) {
            // Start date month is going from disabled to enabled. Restore from sticky value.
            _startDateComponentsGroup.restoreMonth();
        }
        if (_studyPeriodStartDateDayTextField.isEnabled() && !enableDay) {
            // Start date day is going from enabled to disabled. Save as first day of month.
            _startDateComponentsGroup.setDay(1);
        } else if (!_studyPeriodStartDateDayTextField.isEnabled() && enableDay) {
            // Start date day is going from disabled to enabled. Restore from sticky value.
            _startDateComponentsGroup.restoreDay();
        }

        // End date month and day values.
        if (_studyPeriodEndDateMonthTextField.isEnabled() && !enableMonth) {
            // End date month is going from enabled to disabled. Save to month as December.
            _endDateComponentsGroup.setMonth(12);
        } else if (!_studyPeriodEndDateMonthTextField.isEnabled() && enableMonth) {
            // Start date month is going from disabled to enabled. Restore from sticky value.
            _endDateComponentsGroup.restoreMonth();
        }
        if (_studyPeriodEndDateDayTextField.isEnabled() && !enableDay) {
            // End date day is going from enabled to disabled. Save as last day of month.
            _endDateComponentsGroup.setDay(31);
        } else if (!_studyPeriodEndDateDayTextField.isEnabled() && enableDay) {
            // End date day is going from disabled to enabled. Restore from sticky value.
            _endDateComponentsGroup.restoreDay();
        } else if (!enableDay) {
            // End date is continuing to be disabled, make sure that day is last day of month.
            _endDateComponentsGroup.setDay(31);
        }
        
        //enable study period month controls
        _studyPeriodStartDateMonthTextField.setEnabled(enableMonth);
        _studyPeriodEndDateMonthTextField.setEnabled(enableMonth);
        //enable study period day controls
        _studyPeriodStartDateDayTextField.setEnabled(enableDay);
        _studyPeriodEndDateDayTextField.setEnabled(enableDay);
    }    
    
    /* If necessary, removes from from iconized state and brings to front. */
    public void focusWindow() {
        if (this.isIcon()) {
            try {
                this.setIcon(false);
            } catch (PropertyVetoException e) {
                return;
            }
        }
        toFront();
    }

    /* Determines whether window can be closed by comparing parameter settings contained in window verse initial parameter settings. */
    public boolean queryWindowCanClose() {
        boolean bReturn = true;

        saveParameterSettings(_parameters);
        if (!_parameters.equals(_initialParameters)) {
            focusWindow();
            switch (JOptionPane.showInternalConfirmDialog(this, "Parameter settings have changed. Do you want to save?", "Save?", JOptionPane.YES_NO_CANCEL_OPTION)) {
                case JOptionPane.YES_OPTION:
                    if (WriteSession("")) {
                        gbPromptOnExist = false;
                    } else {
                        bReturn = false;
                    }
                    break;
                case JOptionPane.CANCEL_OPTION:
                    bReturn = false;
                    break;
                default:
                    gbPromptOnExist = false;
            }
        }
        return bReturn;
    }

    /* Verifies that input settings are valid in the context of all parameter settings. */
    private void CheckInputSettings() {
        //validate the tree file
        if (!_timeonlyScanType.isSelected()) {
            if (_treelFileTextField.getText().length() == 0)
                throw new SettingsException("Please specify a tree file.", (Component) _treelFileTextField);
            if (!FileAccess.ValidateFileAccess(_treelFileTextField.getText(), false))
                throw new SettingsException("The tree file could not be opened for reading.\n\nPlease confirm that the path and/or file name are valid and that you have permissions to read from this directory and file.", (Component) _treelFileTextField);
            String validationString = validateInputSourceDataFile(_treelFileTextField.getText(), InputSourceSettings.InputFileType.Tree.toString() + "1", "tree");
            if (validationString != null) throw new SettingsException(validationString, (Component) _treelFileTextField);
        }

        //validate the case file
        if (_countFileTextField.getText().length() == 0)
            throw new SettingsException("Please specify a count file.", (Component) _countFileTextField);
        if (!FileAccess.ValidateFileAccess(_countFileTextField.getText(), false))
            throw new SettingsException("The count file could not be opened for reading.\n\nPlease confirm that the path and/or file name are valid and that you have permissions to read from this directory and file.", (Component) _countFileTextField);
        
        //validate the control file
        if ((getModelType() == Parameters.ModelType.BERNOULLI_TIME || getModelType() == Parameters.ModelType.BERNOULLI_TREE)) {
            if (_controlFileTextField.getText().length() == 0)
                throw new SettingsException("Please specify a control file.", (Component) _controlFileTextField);
            if(!FileAccess.ValidateFileAccess(_controlFileTextField.getText(), false))
                throw new SettingsException("The control file could not be opened for reading.\n\nPlease confirm that the path and/or file name are valid and that you have permissions to read from this directory and file.", (Component) _countFileTextField);
        }
        
        if (Utils.selected(_treetimeScanType) || Utils.selected(_timeonlyScanType)) {
            LocalDate startDate = getDataTimeRangeStartDate(), endDate = getDataTimeRangeEndDate();             
            if (endDate.isBefore(startDate) || endDate.isEqual(startDate)) {
                throw new SettingsException(
                    "The data time range start must be before the data time range end.",
                    (Component)(getPrecisionOfTimesControlType() == Parameters.DatePrecisionType.GENERIC ? _studyPeriodStartDateGenericTextField : _studyPeriodStartDateYearTextField)
                );
            }
        }        
    }

    /* Returns the number of time units in the specified data time range. */
    public int getNumUnitsInRange() {
        return Utils.getUnitsBetween(getPrecisionOfTimesControlType(), getDataTimeRangeStartDate(), getDataTimeRangeEndDate());
    }

    /* Verifies that analysis settings are valid in the context of all parameter settings. */
    private void CheckAnalysisSettings() {
        if (Utils.selected(_BernoulliButton)) {
            int eventProbNumerator = Integer.parseInt(_eventProbabiltyNumerator.getText().trim());
            int eventProbDenominator = Integer.parseInt(_eventProbabiltyDenominator.getText().trim());
            if (eventProbNumerator == 0 || eventProbDenominator == 0 || eventProbNumerator >= eventProbDenominator) {
                throw new SettingsException("Please specify an event probabilty that is between zero and one.", (Component) _eventProbabiltyNumerator);
            }
        }
    }

    /* Verifies that output settings are valid in the context of all parameter settings. */
    private void CheckOutputSettings() {
        if (_outputFileTextField.getText().length() == 0) {
            throw new SettingsException("Please specify a results file.", (Component) _outputFileTextField);
        }
        if (!FileAccess.ValidateFileAccess(_outputFileTextField.getText(), true)) {
            throw new SettingsException("Results file could not be opened for writing.\n" + "Please confirm that the path and/or file name\n" + "are valid and that you have permissions to write\nto this directory and file.",
                    (Component) _outputFileTextField);
        }
    }

    /* Verifies that settings are valid in the context of all other parameter settings. */
    public boolean CheckSettings() {
        try {
            CheckAnalysisSettings();
            CheckInputSettings();
            CheckOutputSettings();
        } catch (SettingsException e) {
            focusWindow();
            JOptionPane.showInternalMessageDialog(this, e.getMessage());
            e.setControlFocus();
            return false;
        }
        try {
            getAdvancedParameterInternalFrame().CheckSettings();
        } catch (AdvFeaturesExpection e) {
            focusWindow();
            JOptionPane.showInternalMessageDialog(this, e.getMessage());
            getAdvancedParameterInternalFrame().setVisible(e.focusTab, e.focusComponent);
            enableAdvancedButtons();
            return false;
        }
        return true;
    }

    /* setup interface from parameter settings */
    @Override
    protected void setupInterface(final Parameters parameters) {
        _startDateComponentsGroup = new DateComponentsGroup(undo,_studyPeriodStartDateYearTextField,_studyPeriodStartDateMonthTextField,_studyPeriodStartDateDayTextField, 2000, 1, 1, false);
        _endDateComponentsGroup = new DateComponentsGroup(undo,_studyPeriodEndDateYearTextField,_studyPeriodEndDateMonthTextField,_studyPeriodEndDateDayTextField, 2000, 1, 1, true);        

        _advancedParametersSetting = new AdvancedParameterSettingsFrame(_rootPane, this/*, parameters*/);
        title = parameters.getSourceFileName();
        if (title == null || title.length() == 0) {
            title = "New Session";
        }
        
        setPrecisionOfTimesControl(parameters.getPrecisionOfTimesType());
        
        _treelFileTextField.setText(parameters.getTreeFileNames().get(0));
        _treelFileTextField.setCaretPosition(0);
        // Since this is a follow-up sequentail analysis, the user will need to specify a new count file.
        boolean isFollowupSequential = _parameters.isSequentialScanBernoulli() && Parameters.getAlphaSpentToDate(parameters.getOutputFileName()) != 0.0;
        if (!isFollowupSequential) {
            _countFileTextField.setText(parameters.getCountFileName());
            _countFileTextField.setCaretPosition(0);
        }
        _controlFileTextField.setText(parameters.getControlFileName());
        _controlFileTextField.setCaretPosition(0);
        
        
        if (parameters.getPrecisionOfTimesType().equals(Parameters.DatePrecisionType.GENERIC)) {
            Utils.parseDateStringToControl(parameters.getDataTimeRangeBegin(), _studyPeriodStartDateGenericTextField);
            Utils.parseDateStringToControl(parameters.getDataTimeRangeClose(), _studyPeriodEndDateGenericTextField);
        } else {
            Utils.parseDateStringToControls(parameters.getDataTimeRangeBegin(), _studyPeriodStartDateYearTextField, _studyPeriodStartDateMonthTextField, _studyPeriodStartDateDayTextField, false);
            Utils.parseDateStringToControls(parameters.getDataTimeRangeClose(), _studyPeriodEndDateYearTextField, _studyPeriodEndDateMonthTextField, _studyPeriodEndDateDayTextField, true);
        }

        setControlsForAnalysisOptions(parameters.getScanType(), parameters.getConditionalType(), parameters.getModelType());
        _self_control_design.setSelected(parameters.getSelfControlDesign());
        _eventProbabiltyNumerator.setText(Integer.toString(parameters.getProbabilityRatioNumerator()));
        _eventProbabiltyDenominator.setText(Integer.toString(parameters.getProbabilityRatioDenominator()));

        _outputFileTextField.setText(parameters.getOutputFileName());
        _outputFileTextField.setCaretPosition(0);
        _reportResultsAsHTML.setSelected(parameters.isGeneratingHtmlResults());
        _reportResultsAsCsvTable.setSelected(parameters.isGeneratingTableResults());
        _generate_ncbi_asn.setSelected(parameters.isGeneratingNCBIAsnResults());
        _generate_newick.setSelected(parameters.isGeneratingNewickFile());

        _input_source_map.clear();
        for (int i=0; i < parameters.getInputSourceSettings().size(); ++i) {
            InputSourceSettings iss = parameters.getInputSourceSettings().get(i);
            if (!(isFollowupSequential && iss.getInputFileType() == InputSourceSettings.InputFileType.Counts))
                _input_source_map.put(iss.getInputFileType().toString() + iss.getIndex(), iss);
        }
        _advancedParametersSetting.setupInterface(parameters);
        onCountTimePrecisionChange();
    }

    /* Sets the settings on Analysis tab for Parameters.ScanType, Parameters.ConditionalType and Parameters.ModelType. */
    public void setControlsForAnalysisOptions(Parameters.ScanType s, Parameters.ConditionalType c, Parameters.ModelType m) {
        _treeOnlyScanType.setSelected(s == Parameters.ScanType.TREEONLY);
        _treetimeScanType.setSelected(s == Parameters.ScanType.TREETIME);
        _timeonlyScanType.setSelected(s == Parameters.ScanType.TIMEONLY);
        _unconditionalButton.setSelected(c == Parameters.ConditionalType.UNCONDITIONAL);
        _conditionalTotalCasesButton.setSelected(c == Parameters.ConditionalType.TOTALCASES);
        _conditionalBranchCasesButton.setSelected(c == Parameters.ConditionalType.NODE);
        _conditionalNodeTimeButton.setSelected(c == Parameters.ConditionalType.NODEANDTIME);
        _PoissonButton.setSelected(m == Parameters.ModelType.POISSON);
        _BernoulliButton.setSelected(m == Parameters.ModelType.BERNOULLI_TREE);
        _uniformButton.setSelected(m == Parameters.ModelType.UNIFORM);
        _bernoulliTimeButton.setSelected(m == Parameters.ModelType.BERNOULLI_TIME);
    }

    /* sets Parameters class with settings in form */
    protected void saveParameterSettings(Parameters parameters) {
        setTitle(parameters.getSourceFileName());
        parameters.setPrecisionOfTimesType(getPrecisionOfTimesControlType().ordinal());        
        parameters.setTreeFileName(_treelFileTextField.getText(), 1);
        parameters.setCountFileName(_countFileTextField.getText());
        parameters.setControlFileName(_controlFileTextField.getText());
        if (parameters.getPrecisionOfTimesType().equals(Parameters.DatePrecisionType.GENERIC)) {
            parameters.setDataTimeRangeBegin(_studyPeriodStartDateGenericTextField.getText());
            parameters.setDataTimeRangeClose(_studyPeriodEndDateGenericTextField.getText());
        } else if (!parameters.getPrecisionOfTimesType().equals(Parameters.DatePrecisionType.NONE)) {
            parameters.setDataTimeRangeBegin(
                _studyPeriodStartDateYearTextField.getText() + "/" + _studyPeriodStartDateMonthTextField.getText() + "/" + _studyPeriodStartDateDayTextField.getText()
            );
            parameters.setDataTimeRangeClose(
                _studyPeriodEndDateYearTextField.getText() + "/" + _studyPeriodEndDateMonthTextField.getText() + "/" + _studyPeriodEndDateDayTextField.getText()
            );
        } else {
            parameters.setDataTimeRangeBegin("0");
            parameters.setDataTimeRangeClose("0");
        }
        if (_treeOnlyScanType.isSelected()) {
            parameters.setScanType(Parameters.ScanType.TREEONLY.ordinal());
        } else if (_treetimeScanType.isSelected()) {
            parameters.setScanType(Parameters.ScanType.TREETIME.ordinal());
        } else if (_timeonlyScanType.isSelected()) {
            parameters.setScanType(Parameters.ScanType.TIMEONLY.ordinal());
        }
        if (_unconditionalButton.isSelected()) {
            parameters.setConditionalType(Parameters.ConditionalType.UNCONDITIONAL.ordinal());
        } else if (_conditionalTotalCasesButton.isSelected()) {
            parameters.setConditionalType(Parameters.ConditionalType.TOTALCASES.ordinal());
        } else if (_conditionalBranchCasesButton.isSelected()) {
            parameters.setConditionalType(Parameters.ConditionalType.NODE.ordinal());
        } else if (_conditionalNodeTimeButton.isSelected()) {
            parameters.setConditionalType(Parameters.ConditionalType.NODEANDTIME.ordinal());
        }
        if (_PoissonButton.isEnabled() && _PoissonButton.isSelected()) {
            parameters.setModelType(Parameters.ModelType.POISSON.ordinal());
        } else if (_BernoulliButton.isEnabled() && _BernoulliButton.isSelected()) {
            parameters.setModelType(Parameters.ModelType.BERNOULLI_TREE.ordinal());
        } else if (_uniformButton.isEnabled() && _uniformButton.isSelected()) {
            parameters.setModelType(Parameters.ModelType.UNIFORM.ordinal());
        } else if (_bernoulliTimeButton.isEnabled() && _bernoulliTimeButton.isSelected()) {
            parameters.setModelType(Parameters.ModelType.BERNOULLI_TIME.ordinal());
        }
        parameters.setSelfControlDesign(_self_control_design.isEnabled() && _self_control_design.isSelected());
        parameters.setProbabilityRatioNumerator(Integer.parseInt(_eventProbabiltyNumerator.getText()));
        parameters.setProbabilityRatioDenominator(Integer.parseInt(_eventProbabiltyDenominator.getText()));
        parameters.setOutputFileName(_outputFileTextField.getText());
        parameters.setGeneratingHtmlResults(_reportResultsAsHTML.isSelected());
        parameters.setGeneratingTableResults(Utils.selected(_reportResultsAsCsvTable));
        parameters.setGeneratingNCBIAsnResults(Utils.selected(_generate_ncbi_asn));
        parameters.setGeneratingNewickFile(Utils.selected(_generate_newick));
        getAdvancedParameterInternalFrame().saveParameterSettings(parameters);
        parameters.clearInputSourceSettings();
        Iterator it = _input_source_map.entrySet().iterator();
        while (it.hasNext()) {
            Map.Entry pairs = (Map.Entry)it.next();
            InputSourceSettings iss = (InputSourceSettings)pairs.getValue();
            if (iss.isSet()) {
                parameters.addInputSourceSettings((InputSourceSettings)pairs.getValue());
            }
        }
    }

    public Parameters.ScanType getScanType() {
        if (_treeOnlyScanType.isSelected())
            return Parameters.ScanType.TREEONLY;
        if (_treetimeScanType.isSelected())
            return Parameters.ScanType.TREETIME;
        if (_timeonlyScanType.isSelected())
            return Parameters.ScanType.TIMEONLY;
        return null;
    }

    public Parameters.ConditionalType getConditionalType() {
        if (_unconditionalButton.isSelected()) {
            return Parameters.ConditionalType.UNCONDITIONAL;
        } else if (_conditionalTotalCasesButton.isSelected()) {
            return Parameters.ConditionalType.TOTALCASES;
        } else if (_conditionalBranchCasesButton.isSelected()) {
            return Parameters.ConditionalType.NODE;
        } else if (_conditionalNodeTimeButton.isSelected()) {
            return Parameters.ConditionalType.NODEANDTIME;
        }
        return null;
    }

    public Parameters.ModelType getModelType() {
        if (_PoissonButton.isSelected() && _PoissonButton.isEnabled())
            return Parameters.ModelType.POISSON;
        if (_BernoulliButton.isSelected() && _BernoulliButton.isEnabled())
            return Parameters.ModelType.BERNOULLI_TREE;
        if (_uniformButton.isSelected() && _uniformButton.isEnabled())
            return Parameters.ModelType.UNIFORM;
        if (_bernoulliTimeButton.isSelected() && _bernoulliTimeButton.isEnabled())
            return Parameters.ModelType.BERNOULLI_TIME;
        return null;
    }

    public boolean getSelfControlDesign() {
       return _self_control_design.isEnabled() && _self_control_design.isSelected();
    }

    private void enableSettingsForStatisticModelCombination() {
        boolean treeOnly = _treeOnlyScanType.isSelected();
        boolean treeAndTime = _treetimeScanType.isSelected();
        boolean timeOnly = _timeonlyScanType.isSelected();

        enableDatesByTimePrecisionUnits();        
        // conditional on branch is only available with tree-time
        _unconditionalButton.setEnabled(treeOnly);
        _conditionalTotalCasesButton.setEnabled(treeOnly || timeOnly);
        _conditionalBranchCasesButton.setEnabled(treeAndTime);
        _conditionalNodeTimeButton.setEnabled(treeAndTime);
        if ((!_unconditionalButton.isEnabled() && _unconditionalButton.isSelected()) || (!_conditionalTotalCasesButton.isEnabled() && _conditionalTotalCasesButton.isSelected())) {
            _conditionalBranchCasesButton.setSelected(true);
        }
        if ((!_conditionalBranchCasesButton.isEnabled() && _conditionalBranchCasesButton.isSelected()) ||
            (!_conditionalNodeTimeButton.isEnabled() && _conditionalNodeTimeButton.isSelected())) {
            if (treeOnly) _unconditionalButton.setSelected(true);
            if (timeOnly) _conditionalTotalCasesButton.setSelected(true);
        }
        // Poisson and Bernoulli are only available with tree only
        _PoissonButton.setEnabled(treeOnly);
        _BernoulliButton.setEnabled(treeOnly);
        // uniform is only available with tree-time and conditional on branch or time-only
        _uniformButton.setEnabled((treeAndTime && _conditionalBranchCasesButton.isSelected()) || timeOnly);        
        _bernoulliTimeButton.setEnabled((treeAndTime && _conditionalBranchCasesButton.isSelected()) || timeOnly);        
        // the tree file is not used with the time only scan
        _treeFileLabel.setEnabled(!timeOnly);
        _treelFileTextField.setEnabled(!timeOnly);
        _treeFileImportButton.setEnabled(!timeOnly);
        // event probability inputs only available for unconditional Bernoulli
        boolean enabled = Utils.selected(_BernoulliButton) && Utils.selected(_unconditionalButton);
        _eventProbabilityLabel.setEnabled(enabled);
        _eventProbabilityLabel2.setEnabled(enabled);
        _eventProbabiltyNumerator.setEnabled(enabled);
        _eventProbabiltyDenominator.setEnabled(enabled);
        _self_control_design.setEnabled(enabled);
        _generate_ncbi_asn.setEnabled(!timeOnly);
        _generate_newick.setEnabled(!timeOnly);
        _advancedParametersSetting.enableAdvancedInputsSettings(!timeOnly);
        _advancedParametersSetting.enableTemporalOptionGroups(treeAndTime || timeOnly);
        _advancedParametersSetting.enableAdjustmentsOptions();
        _advancedParametersSetting.enablePowerEvaluationsGroup();
        _advancedParametersSetting.enableRestrictedLevelsGroup();
        _advancedParametersSetting.enableSequentialAnalysisGroup();
        _advancedParametersSetting.enableAdditionalOutputOptions();
        _advancedParametersSetting.enableTimeRangeExclusionsGroup();
        _advancedParametersSetting.enableTemporalGraphsGroup(treeAndTime || timeOnly);
    }

    /* Returns the data time range start date as LocalDate. */
    public LocalDate getDataTimeRangeStartDate()  {
        return Utils.getLocalDate(
            getPrecisionOfTimesControlType(),
            _studyPeriodStartDateYearTextField.getText(),
            _studyPeriodStartDateMonthTextField.getText(),
            _studyPeriodStartDateDayTextField.getText(),
            _studyPeriodStartDateGenericTextField.getText()
        );
    }
    
    /* Returns the data time range end date as LocalDate. */
    public LocalDate getDataTimeRangeEndDate() {
        return Utils.getLocalDate(
            getPrecisionOfTimesControlType(),
            _studyPeriodEndDateYearTextField.getText(),
            _studyPeriodEndDateMonthTextField.getText(),
            _studyPeriodEndDateDayTextField.getText(),
            _studyPeriodEndDateGenericTextField.getText()
        );
    }   
   
    /**
     * This method is called from within the constructor to initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is always
     * regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        treeModelButtonGroup = new javax.swing.ButtonGroup();
        conditionalButtonGroup = new javax.swing.ButtonGroup();
        scanButtonGroup = new javax.swing.ButtonGroup();
        timtModelButtonGoup = new javax.swing.ButtonGroup();
        _timePrecisionButtonGroup = new javax.swing.ButtonGroup();
        jTabbedPane1 = new javax.swing.JTabbedPane();
        _inputTab = new javax.swing.JPanel();
        _treelFileTextField = new javax.swing.JTextField();
        _treeFileLabel = new javax.swing.JLabel();
        _treeFileImportButton = new javax.swing.JButton();
        _countFileLabel = new javax.swing.JLabel();
        _countFileTextField = new javax.swing.JTextField();
        _countFileImportButton = new javax.swing.JButton();
        _advancedInputButton = new javax.swing.JButton();
        _controlFileLabel = new javax.swing.JLabel();
        _controlFileTextField = new javax.swing.JTextField();
        _controlFileImportButton = new javax.swing.JButton();
        _timePrecisionGroup = new javax.swing.JPanel();
        _timePrecisionNone = new javax.swing.JRadioButton();
        _timePrecisionYear = new javax.swing.JRadioButton();
        _timePrecisionMonth = new javax.swing.JRadioButton();
        _timePrecisionDay = new javax.swing.JRadioButton();
        _timePrecisionGeneric = new javax.swing.JRadioButton();
        _studyPeriodGroup = new javax.swing.JPanel();
        _studyPeriodComplete = new javax.swing.JPanel();
        _startDateLabel = new javax.swing.JLabel();
        _startDateYearLabel = new javax.swing.JLabel();
        _studyPeriodStartDateYearTextField = new javax.swing.JTextField();
        _studyPeriodStartDateMonthTextField = new javax.swing.JTextField();
        _startDateMonthLabel = new javax.swing.JLabel();
        _startDateDayLabel = new javax.swing.JLabel();
        _studyPeriodStartDateDayTextField = new javax.swing.JTextField();
        _endDateLabel = new javax.swing.JLabel();
        _studyPeriodEndDateYearTextField = new javax.swing.JTextField();
        _endDateYearLabel = new javax.swing.JLabel();
        _endDateMonthLabel = new javax.swing.JLabel();
        _studyPeriodEndDateMonthTextField = new javax.swing.JTextField();
        _studyPeriodEndDateDayTextField = new javax.swing.JTextField();
        _endDateDayLabel = new javax.swing.JLabel();
        _studyPeriodGeneric = new javax.swing.JPanel();
        _startRangeDateLabel = new javax.swing.JLabel();
        _startRangeYearLabel = new javax.swing.JLabel();
        _studyPeriodStartDateGenericTextField = new javax.swing.JTextField();
        _endRangeLabel = new javax.swing.JLabel();
        _studyPeriodEndDateGenericTextField = new javax.swing.JTextField();
        _endRangeYearLabel = new javax.swing.JLabel();
        _analysisTab = new javax.swing.JPanel();
        _probabilityModelPanel = new javax.swing.JPanel();
        _PoissonButton = new javax.swing.JRadioButton();
        _BernoulliButton = new javax.swing.JRadioButton();
        _eventProbabiltyDenominator = new javax.swing.JTextField();
        _eventProbabiltyNumerator = new javax.swing.JTextField();
        _eventProbabilityLabel = new javax.swing.JLabel();
        _eventProbabilityLabel2 = new javax.swing.JLabel();
        _self_control_design = new javax.swing.JCheckBox();
        _scanStatisticPanel = new javax.swing.JPanel();
        _conditionalTotalCasesButton = new javax.swing.JRadioButton();
        _unconditionalButton = new javax.swing.JRadioButton();
        _conditionalBranchCasesButton = new javax.swing.JRadioButton();
        _conditionalNodeTimeButton = new javax.swing.JRadioButton();
        _scanStatisticPanel1 = new javax.swing.JPanel();
        _treetimeScanType = new javax.swing.JRadioButton();
        _treeOnlyScanType = new javax.swing.JRadioButton();
        _timeonlyScanType = new javax.swing.JRadioButton();
        _probabilityModelPanel1 = new javax.swing.JPanel();
        _uniformButton = new javax.swing.JRadioButton();
        _bernoulliTimeButton = new javax.swing.JRadioButton();
        _advancedAnalysisButton = new javax.swing.JButton();
        _outputTab = new javax.swing.JPanel();
        _reportResultsAsHTML = new javax.swing.JCheckBox();
        _outputFileTextField = new javax.swing.JTextField();
        _resultsFileLabel = new javax.swing.JLabel();
        _resultsFileBrowseButton = new javax.swing.JButton();
        _resultsFileLabel1 = new javax.swing.JLabel();
        _reportResultsAsCsvTable = new javax.swing.JCheckBox();
        _advancedOutputButton = new javax.swing.JButton();
        _generate_ncbi_asn = new javax.swing.JCheckBox();
        _generate_newick = new javax.swing.JCheckBox();
        jLabel1 = new javax.swing.JLabel();

        _timePrecisionButtonGroup.add(_timePrecisionNone);
        _timePrecisionButtonGroup.add(_timePrecisionYear);
        _timePrecisionButtonGroup.add(_timePrecisionMonth);
        _timePrecisionButtonGroup.add(_timePrecisionDay);

        setClosable(true);
        setDefaultCloseOperation(javax.swing.WindowConstants.DO_NOTHING_ON_CLOSE);
        setIconifiable(true);
        setResizable(true);

        _treeFileLabel.setText("Tree File (not used for Time Only scan):"); // NOI18N

        _treeFileImportButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/document_add_small.png"))); // NOI18N
        _treeFileImportButton.setToolTipText("Import tree file ..."); // NOI18N
        _treeFileImportButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                String key = InputSourceSettings.InputFileType.Tree.toString() + "1";
                if (!_input_source_map.containsKey(key)) {
                    _input_source_map.put(key, new InputSourceSettings(InputSourceSettings.InputFileType.Tree));
                }
                InputSourceSettings inputSourceSettings = (InputSourceSettings)_input_source_map.get(key);
                // invoke the FileSelectionDialog to guide user through process of selecting the source file.
                FileSelectionDialog selectionDialog = new FileSelectionDialog(TreeScanApplication.getInstance(), inputSourceSettings.getInputFileType(), TreeScanApplication.getInstance().lastBrowseDirectory);
                selectionDialog.browse_inputsource(_treelFileTextField, inputSourceSettings, ParameterSettingsFrame.this, true);
            }
        });

        _countFileLabel.setText("Count File:"); // NOI18N

        _countFileImportButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/document_add_small.png"))); // NOI18N
        _countFileImportButton.setToolTipText("Import count file ..."); // NOI18N
        _countFileImportButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                String key = InputSourceSettings.InputFileType.Counts.toString() + "1";
                if (!_input_source_map.containsKey(key)) {
                    _input_source_map.put(key, new InputSourceSettings(InputSourceSettings.InputFileType.Counts));
                }
                InputSourceSettings inputSourceSettings = (InputSourceSettings)_input_source_map.get(key);
                // invoke the FileSelectionDialog to guide user through process of selecting the source file.
                FileSelectionDialog selectionDialog = new FileSelectionDialog(TreeScanApplication.getInstance(), inputSourceSettings.getInputFileType(), TreeScanApplication.getInstance().lastBrowseDirectory);
                selectionDialog.browse_inputsource(_countFileTextField, inputSourceSettings, ParameterSettingsFrame.this, true);
            }
        });

        _advancedInputButton.setText("Advanced >>"); // NOI18N
        _advancedInputButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                getAdvancedParameterInternalFrame().setVisible(true, AdvancedParameterSettingsFrame.FocusedTabSet.INPUT);
                getAdvancedParameterInternalFrame().requestFocus();
            }
        });

        _controlFileLabel.setText("Control File (Bernoulli Only):"); // NOI18N

        _controlFileImportButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/document_add_small.png"))); // NOI18N
        _controlFileImportButton.setToolTipText("Import count file ..."); // NOI18N
        _controlFileImportButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                String key = InputSourceSettings.InputFileType.Controls.toString() + "1";
                if (!_input_source_map.containsKey(key)) {
                    _input_source_map.put(key, new InputSourceSettings(InputSourceSettings.InputFileType.Controls));
                }
                InputSourceSettings inputSourceSettings = (InputSourceSettings)_input_source_map.get(key);
                // invoke the FileSelectionDialog to guide user through process of selecting the source file.
                FileSelectionDialog selectionDialog = new FileSelectionDialog(TreeScanApplication.getInstance(), inputSourceSettings.getInputFileType(), TreeScanApplication.getInstance().lastBrowseDirectory);
                selectionDialog.browse_inputsource(_controlFileTextField, inputSourceSettings, ParameterSettingsFrame.this, true);
            }
        });

        _timePrecisionGroup.setBorder(javax.swing.BorderFactory.createTitledBorder("Time Precision"));

        _timePrecisionButtonGroup.add(_timePrecisionNone);
        _timePrecisionNone.setSelected(true);
        _timePrecisionNone.setText("None"); // NOI18N
        _timePrecisionNone.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _timePrecisionNone.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _timePrecisionNone.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED)
                onCountTimePrecisionChange();
            }
        });

        _timePrecisionButtonGroup.add(_timePrecisionYear);
        _timePrecisionYear.setText("Year"); // NOI18N
        _timePrecisionYear.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _timePrecisionYear.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _timePrecisionYear.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED)
                onCountTimePrecisionChange();
            }
        });

        _timePrecisionButtonGroup.add(_timePrecisionMonth);
        _timePrecisionMonth.setText("Month"); // NOI18N
        _timePrecisionMonth.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _timePrecisionMonth.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _timePrecisionMonth.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED)
                onCountTimePrecisionChange();
            }
        });

        _timePrecisionButtonGroup.add(_timePrecisionDay);
        _timePrecisionDay.setText("Day"); // NOI18N
        _timePrecisionDay.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _timePrecisionDay.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _timePrecisionDay.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED)
                onCountTimePrecisionChange();
            }
        });

        _timePrecisionButtonGroup.add(_timePrecisionGeneric);
        _timePrecisionGeneric.setText("Generic"); // NOI18N
        _timePrecisionGeneric.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _timePrecisionGeneric.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _timePrecisionGeneric.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED)
                onCountTimePrecisionChange();
            }
        });

        javax.swing.GroupLayout _timePrecisionGroupLayout = new javax.swing.GroupLayout(_timePrecisionGroup);
        _timePrecisionGroup.setLayout(_timePrecisionGroupLayout);
        _timePrecisionGroupLayout.setHorizontalGroup(
            _timePrecisionGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_timePrecisionGroupLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_timePrecisionNone, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addGap(32, 32, 32)
                .addComponent(_timePrecisionGeneric, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addGap(28, 28, 28)
                .addComponent(_timePrecisionYear, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addGap(60, 60, 60)
                .addComponent(_timePrecisionMonth, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addGap(28, 28, 28)
                .addComponent(_timePrecisionDay, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addContainerGap())
        );
        _timePrecisionGroupLayout.setVerticalGroup(
            _timePrecisionGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_timePrecisionGroupLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_timePrecisionGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_timePrecisionNone)
                    .addComponent(_timePrecisionYear)
                    .addComponent(_timePrecisionMonth)
                    .addComponent(_timePrecisionDay)
                    .addComponent(_timePrecisionGeneric))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        _studyPeriodGroup.setLayout(new java.awt.CardLayout());

        _studyPeriodComplete.setBorder(javax.swing.BorderFactory.createTitledBorder("Data Time Range"));

        _startDateLabel.setText("Start Date:"); // NOI18N

        _startDateYearLabel.setFont(new java.awt.Font("Tahoma", 0, 10)); // NOI18N
        _startDateYearLabel.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        _startDateYearLabel.setText("Year"); // NOI18N

        _studyPeriodStartDateYearTextField.setText("2000"); // NOI18N

        _studyPeriodStartDateMonthTextField.setText("01"); // NOI18N

        _startDateMonthLabel.setFont(new java.awt.Font("Tahoma", 0, 10)); // NOI18N
        _startDateMonthLabel.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        _startDateMonthLabel.setText("Month"); // NOI18N

        _startDateDayLabel.setFont(new java.awt.Font("Tahoma", 0, 10)); // NOI18N
        _startDateDayLabel.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        _startDateDayLabel.setText("Day"); // NOI18N

        _studyPeriodStartDateDayTextField.setText("01"); // NOI18N

        _endDateLabel.setText("End Date:"); // NOI18N

        _studyPeriodEndDateYearTextField.setText("2000"); // NOI18N
        _studyPeriodEndDateYearTextField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                _advancedParametersSetting.enableDates();
            }
        });

        _endDateYearLabel.setFont(new java.awt.Font("Tahoma", 0, 10)); // NOI18N
        _endDateYearLabel.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        _endDateYearLabel.setText("Year"); // NOI18N

        _endDateMonthLabel.setFont(new java.awt.Font("Tahoma", 0, 10)); // NOI18N
        _endDateMonthLabel.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        _endDateMonthLabel.setText("Month"); // NOI18N

        _studyPeriodEndDateMonthTextField.setText("12"); // NOI18N
        _studyPeriodEndDateMonthTextField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                _advancedParametersSetting.enableDates();
            }
        });

        _studyPeriodEndDateDayTextField.setText("31"); // NOI18N
        _studyPeriodEndDateDayTextField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                _advancedParametersSetting.enableDates();
            }
        });

        _endDateDayLabel.setFont(new java.awt.Font("Tahoma", 0, 10)); // NOI18N
        _endDateDayLabel.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        _endDateDayLabel.setText("Day"); // NOI18N

        javax.swing.GroupLayout _studyPeriodCompleteLayout = new javax.swing.GroupLayout(_studyPeriodComplete);
        _studyPeriodComplete.setLayout(_studyPeriodCompleteLayout);
        _studyPeriodCompleteLayout.setHorizontalGroup(
            _studyPeriodCompleteLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_studyPeriodCompleteLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_startDateLabel)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_studyPeriodCompleteLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING, false)
                    .addComponent(_startDateYearLabel, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_studyPeriodStartDateYearTextField, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.PREFERRED_SIZE, 50, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_studyPeriodCompleteLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_startDateMonthLabel)
                    .addComponent(_studyPeriodStartDateMonthTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 30, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_studyPeriodCompleteLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                    .addComponent(_startDateDayLabel, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_studyPeriodStartDateDayTextField, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.PREFERRED_SIZE, 30, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addGap(50, 50, 50)
                .addComponent(_endDateLabel)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_studyPeriodCompleteLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING, false)
                    .addComponent(_endDateYearLabel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_studyPeriodEndDateYearTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 50, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_studyPeriodCompleteLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_endDateMonthLabel)
                    .addComponent(_studyPeriodEndDateMonthTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 30, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_studyPeriodCompleteLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                    .addComponent(_endDateDayLabel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_studyPeriodEndDateDayTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 30, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );
        _studyPeriodCompleteLayout.setVerticalGroup(
            _studyPeriodCompleteLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_studyPeriodCompleteLayout.createSequentialGroup()
                .addGroup(_studyPeriodCompleteLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_startDateYearLabel)
                    .addComponent(_startDateMonthLabel)
                    .addComponent(_endDateMonthLabel)
                    .addComponent(_endDateDayLabel)
                    .addComponent(_startDateDayLabel)
                    .addComponent(_endDateYearLabel))
                .addGap(0, 0, 0)
                .addGroup(_studyPeriodCompleteLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_studyPeriodStartDateYearTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_studyPeriodStartDateMonthTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_studyPeriodStartDateDayTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_endDateLabel)
                    .addComponent(_studyPeriodEndDateYearTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_studyPeriodEndDateMonthTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_startDateLabel)
                    .addComponent(_studyPeriodEndDateDayTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        _studyPeriodGroup.add(_studyPeriodComplete, "study_complete");

        _studyPeriodGeneric.setBorder(javax.swing.BorderFactory.createTitledBorder("Data Time Range"));

        _startRangeDateLabel.setText("Range Start"); // NOI18N

        _startRangeYearLabel.setFont(new java.awt.Font("Tahoma", 0, 10)); // NOI18N
        _startRangeYearLabel.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        _startRangeYearLabel.setText("Generic Unit"); // NOI18N

        _studyPeriodStartDateGenericTextField.setText("0"); // NOI18N
        _studyPeriodStartDateGenericTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                if (undo != null ) undo.addEdit(evt.getEdit());
            }
        });
        _studyPeriodStartDateGenericTextField.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validateNumericKeyTyped(_studyPeriodStartDateGenericTextField, e, 10);
            }
        });
        _studyPeriodStartDateGenericTextField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                if (ParameterSettingsFrame.this.getPrecisionOfTimesControlType() == Parameters.DatePrecisionType.GENERIC) {
                    Utils.validateGenericDateField(_studyPeriodStartDateGenericTextField, undo);
                }
            }
        });

        _endRangeLabel.setText("Range End"); // NOI18N

        _studyPeriodEndDateGenericTextField.setText("31"); // NOI18N
        _studyPeriodEndDateGenericTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                if (undo != null ) undo.addEdit(evt.getEdit());
            }
        });
        _studyPeriodEndDateGenericTextField.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validateNumericKeyTyped(_studyPeriodEndDateGenericTextField, e, 10);
            }
        });
        _studyPeriodEndDateGenericTextField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                if (ParameterSettingsFrame.this.getPrecisionOfTimesControlType() == Parameters.DatePrecisionType.GENERIC) {
                    Utils.validateGenericDateField(_studyPeriodEndDateGenericTextField, undo);
                    _advancedParametersSetting.enableDates();
                }
            }
        });

        _endRangeYearLabel.setFont(new java.awt.Font("Tahoma", 0, 10)); // NOI18N
        _endRangeYearLabel.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        _endRangeYearLabel.setText("Generic Unit"); // NOI18N

        javax.swing.GroupLayout _studyPeriodGenericLayout = new javax.swing.GroupLayout(_studyPeriodGeneric);
        _studyPeriodGeneric.setLayout(_studyPeriodGenericLayout);
        _studyPeriodGenericLayout.setHorizontalGroup(
            _studyPeriodGenericLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_studyPeriodGenericLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_startRangeDateLabel)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_studyPeriodGenericLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                    .addComponent(_startRangeYearLabel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_studyPeriodStartDateGenericTextField, javax.swing.GroupLayout.DEFAULT_SIZE, 87, Short.MAX_VALUE))
                .addGap(85, 85, 85)
                .addComponent(_endRangeLabel)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_studyPeriodGenericLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                    .addComponent(_endRangeYearLabel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_studyPeriodEndDateGenericTextField, javax.swing.GroupLayout.DEFAULT_SIZE, 80, Short.MAX_VALUE))
                .addContainerGap(229, Short.MAX_VALUE))
        );
        _studyPeriodGenericLayout.setVerticalGroup(
            _studyPeriodGenericLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_studyPeriodGenericLayout.createSequentialGroup()
                .addGroup(_studyPeriodGenericLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_startRangeYearLabel)
                    .addComponent(_endRangeYearLabel))
                .addGap(0, 0, 0)
                .addGroup(_studyPeriodGenericLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_studyPeriodStartDateGenericTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_endRangeLabel)
                    .addComponent(_studyPeriodEndDateGenericTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_startRangeDateLabel))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        _studyPeriodGroup.add(_studyPeriodGeneric, "study_generic");

        javax.swing.GroupLayout _inputTabLayout = new javax.swing.GroupLayout(_inputTab);
        _inputTab.setLayout(_inputTabLayout);
        _inputTabLayout.setHorizontalGroup(
            _inputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _inputTabLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_inputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                    .addComponent(_studyPeriodGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_timePrecisionGroup, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addGroup(_inputTabLayout.createSequentialGroup()
                        .addGap(0, 0, Short.MAX_VALUE)
                        .addComponent(_advancedInputButton))
                    .addGroup(_inputTabLayout.createSequentialGroup()
                        .addGroup(_inputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                            .addComponent(_treeFileLabel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .addComponent(_treelFileTextField))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_treeFileImportButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addGroup(_inputTabLayout.createSequentialGroup()
                        .addComponent(_controlFileTextField)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_controlFileImportButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addGroup(_inputTabLayout.createSequentialGroup()
                        .addGroup(_inputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                            .addComponent(_controlFileLabel, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .addComponent(_countFileLabel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .addComponent(_countFileTextField))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_countFileImportButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE)))
                .addContainerGap())
        );
        _inputTabLayout.setVerticalGroup(
            _inputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_inputTabLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_treeFileLabel)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_inputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_treelFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_treeFileImportButton))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(_countFileLabel)
                .addGap(9, 9, 9)
                .addGroup(_inputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_countFileImportButton)
                    .addComponent(_countFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_controlFileLabel)
                .addGap(9, 9, 9)
                .addGroup(_inputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_controlFileImportButton)
                    .addComponent(_controlFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(_timePrecisionGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_studyPeriodGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, 26, Short.MAX_VALUE)
                .addComponent(_advancedInputButton)
                .addContainerGap())
        );

        jTabbedPane1.addTab("Input", _inputTab);

        _probabilityModelPanel.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Probability Model - Tree"));
        _probabilityModelPanel.setPreferredSize(new java.awt.Dimension(100, 108));

        treeModelButtonGroup.add(_PoissonButton);
        _PoissonButton.setText("Poisson");
        _PoissonButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                    enableSettingsForStatisticModelCombination();
                }
            }
        });

        treeModelButtonGroup.add(_BernoulliButton);
        _BernoulliButton.setSelected(true);
        _BernoulliButton.setText("Bernoulli");
        _BernoulliButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                    enableSettingsForStatisticModelCombination();
                }
            }
        });

        _eventProbabiltyDenominator.setText("2");
        _eventProbabiltyDenominator.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                while (_eventProbabiltyDenominator.getText().length() == 0)
                if (undo.canUndo()) undo.undo(); else _eventProbabiltyDenominator.setText("2");
            }
        });
        _eventProbabiltyDenominator.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validatePostiveNumericKeyTyped(_eventProbabiltyDenominator, e, 6);
            }
        });
        _eventProbabiltyDenominator.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });

        _eventProbabiltyNumerator.setText("1");
        _eventProbabiltyNumerator.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                while (_eventProbabiltyNumerator.getText().length() == 0)
                if (undo.canUndo()) undo.undo(); else _eventProbabiltyNumerator.setText("1");
            }
        });
        _eventProbabiltyNumerator.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validatePostiveNumericKeyTyped(_eventProbabiltyNumerator, e, 6);
            }
        });
        _eventProbabiltyNumerator.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });

        _eventProbabilityLabel.setText("Case Probability:");

        _eventProbabilityLabel2.setText("/");

        _self_control_design.setText("Self-Control Design");
        _self_control_design.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                enableSettingsForStatisticModelCombination();
            }
        });

        javax.swing.GroupLayout _probabilityModelPanelLayout = new javax.swing.GroupLayout(_probabilityModelPanel);
        _probabilityModelPanel.setLayout(_probabilityModelPanelLayout);
        _probabilityModelPanelLayout.setHorizontalGroup(
            _probabilityModelPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_probabilityModelPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_probabilityModelPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_PoissonButton, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addGroup(_probabilityModelPanelLayout.createSequentialGroup()
                        .addGap(21, 21, 21)
                        .addComponent(_eventProbabilityLabel)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_eventProbabiltyNumerator, javax.swing.GroupLayout.PREFERRED_SIZE, 40, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_eventProbabilityLabel2)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_eventProbabiltyDenominator, javax.swing.GroupLayout.PREFERRED_SIZE, 40, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addGap(0, 0, Short.MAX_VALUE))
                    .addGroup(_probabilityModelPanelLayout.createSequentialGroup()
                        .addComponent(_BernoulliButton, javax.swing.GroupLayout.PREFERRED_SIZE, 111, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                        .addComponent(_self_control_design, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)))
                .addContainerGap())
        );
        _probabilityModelPanelLayout.setVerticalGroup(
            _probabilityModelPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_probabilityModelPanelLayout.createSequentialGroup()
                .addComponent(_PoissonButton)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addGroup(_probabilityModelPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_BernoulliButton)
                    .addComponent(_self_control_design))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addGroup(_probabilityModelPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_eventProbabilityLabel)
                    .addComponent(_eventProbabiltyNumerator, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_eventProbabilityLabel2)
                    .addComponent(_eventProbabiltyDenominator, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(15, Short.MAX_VALUE))
        );

        _scanStatisticPanel.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Conditional Analysis"));

        conditionalButtonGroup.add(_conditionalTotalCasesButton);
        _conditionalTotalCasesButton.setText("Total Cases");
        _conditionalTotalCasesButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                    enableSettingsForStatisticModelCombination();
                }
            }
        });

        conditionalButtonGroup.add(_unconditionalButton);
        _unconditionalButton.setSelected(true);
        _unconditionalButton.setText("No (unconditional)");
        _unconditionalButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                    enableSettingsForStatisticModelCombination();
                }
            }
        });

        conditionalButtonGroup.add(_conditionalBranchCasesButton);
        _conditionalBranchCasesButton.setText("Node");
        _conditionalBranchCasesButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                    enableSettingsForStatisticModelCombination();
                }
            }
        });

        conditionalButtonGroup.add(_conditionalNodeTimeButton);
        _conditionalNodeTimeButton.setText("Node and Time");
        _conditionalNodeTimeButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                    enableSettingsForStatisticModelCombination();
                }
            }
        });

        javax.swing.GroupLayout _scanStatisticPanelLayout = new javax.swing.GroupLayout(_scanStatisticPanel);
        _scanStatisticPanel.setLayout(_scanStatisticPanelLayout);
        _scanStatisticPanelLayout.setHorizontalGroup(
            _scanStatisticPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_scanStatisticPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_unconditionalButton, javax.swing.GroupLayout.DEFAULT_SIZE, 175, Short.MAX_VALUE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(_conditionalTotalCasesButton, javax.swing.GroupLayout.DEFAULT_SIZE, 143, Short.MAX_VALUE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(_conditionalBranchCasesButton, javax.swing.GroupLayout.DEFAULT_SIZE, 118, Short.MAX_VALUE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(_conditionalNodeTimeButton, javax.swing.GroupLayout.DEFAULT_SIZE, 159, Short.MAX_VALUE)
                .addContainerGap())
        );
        _scanStatisticPanelLayout.setVerticalGroup(
            _scanStatisticPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _scanStatisticPanelLayout.createSequentialGroup()
                .addGroup(_scanStatisticPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_unconditionalButton)
                    .addComponent(_conditionalTotalCasesButton)
                    .addComponent(_conditionalBranchCasesButton)
                    .addComponent(_conditionalNodeTimeButton))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        _scanStatisticPanel1.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Type of Scan"));

        scanButtonGroup.add(_treetimeScanType);
        _treetimeScanType.setText("Tree and Time");
        _treetimeScanType.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                    enableSettingsForStatisticModelCombination();
                }
            }
        });

        scanButtonGroup.add(_treeOnlyScanType);
        _treeOnlyScanType.setSelected(true);
        _treeOnlyScanType.setText("Tree Only");
        _treeOnlyScanType.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                    enableSettingsForStatisticModelCombination();
                }
            }
        });

        scanButtonGroup.add(_timeonlyScanType);
        _timeonlyScanType.setText("Time Only");
        _timeonlyScanType.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                    enableSettingsForStatisticModelCombination();
                }
            }
        });

        javax.swing.GroupLayout _scanStatisticPanel1Layout = new javax.swing.GroupLayout(_scanStatisticPanel1);
        _scanStatisticPanel1.setLayout(_scanStatisticPanel1Layout);
        _scanStatisticPanel1Layout.setHorizontalGroup(
            _scanStatisticPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_scanStatisticPanel1Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_treeOnlyScanType, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_treetimeScanType, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_timeonlyScanType, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addContainerGap())
        );
        _scanStatisticPanel1Layout.setVerticalGroup(
            _scanStatisticPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _scanStatisticPanel1Layout.createSequentialGroup()
                .addGroup(_scanStatisticPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_treeOnlyScanType)
                    .addComponent(_treetimeScanType)
                    .addComponent(_timeonlyScanType))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        _probabilityModelPanel1.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Probability Model - Time"));
        _probabilityModelPanel1.setPreferredSize(new java.awt.Dimension(200, 108));

        timtModelButtonGoup.add(_uniformButton);
        _uniformButton.setSelected(true);
        _uniformButton.setText("Uniform");
        _uniformButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                    enableSettingsForStatisticModelCombination();
                }
            }
        });

        timtModelButtonGoup.add(_bernoulliTimeButton);
        _bernoulliTimeButton.setText("Bernoulli");
        _bernoulliTimeButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                    enableSettingsForStatisticModelCombination();
                }
            }
        });

        javax.swing.GroupLayout _probabilityModelPanel1Layout = new javax.swing.GroupLayout(_probabilityModelPanel1);
        _probabilityModelPanel1.setLayout(_probabilityModelPanel1Layout);
        _probabilityModelPanel1Layout.setHorizontalGroup(
            _probabilityModelPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_probabilityModelPanel1Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_probabilityModelPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_uniformButton, javax.swing.GroupLayout.DEFAULT_SIZE, 273, Short.MAX_VALUE)
                    .addComponent(_bernoulliTimeButton, javax.swing.GroupLayout.DEFAULT_SIZE, 273, Short.MAX_VALUE))
                .addContainerGap())
        );
        _probabilityModelPanel1Layout.setVerticalGroup(
            _probabilityModelPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_probabilityModelPanel1Layout.createSequentialGroup()
                .addComponent(_uniformButton)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(_bernoulliTimeButton)
                .addGap(0, 0, Short.MAX_VALUE))
        );

        _advancedAnalysisButton.setText("Advanced >>"); // NOI18N
        _advancedAnalysisButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                getAdvancedParameterInternalFrame().setVisible(true, AdvancedParameterSettingsFrame.FocusedTabSet.ANALYSIS);
                getAdvancedParameterInternalFrame().requestFocus();
            }
        });

        javax.swing.GroupLayout _analysisTabLayout = new javax.swing.GroupLayout(_analysisTab);
        _analysisTab.setLayout(_analysisTabLayout);
        _analysisTabLayout.setHorizontalGroup(
            _analysisTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_analysisTabLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_analysisTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_scanStatisticPanel1, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_scanStatisticPanel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addGroup(_analysisTabLayout.createSequentialGroup()
                        .addComponent(_probabilityModelPanel, javax.swing.GroupLayout.DEFAULT_SIZE, 322, Short.MAX_VALUE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_probabilityModelPanel1, javax.swing.GroupLayout.DEFAULT_SIZE, 297, Short.MAX_VALUE))
                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _analysisTabLayout.createSequentialGroup()
                        .addGap(0, 0, Short.MAX_VALUE)
                        .addComponent(_advancedAnalysisButton)))
                .addContainerGap())
        );
        _analysisTabLayout.setVerticalGroup(
            _analysisTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_analysisTabLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_scanStatisticPanel1, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_scanStatisticPanel, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_analysisTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                    .addComponent(_probabilityModelPanel, javax.swing.GroupLayout.DEFAULT_SIZE, 113, Short.MAX_VALUE)
                    .addComponent(_probabilityModelPanel1, javax.swing.GroupLayout.DEFAULT_SIZE, 113, Short.MAX_VALUE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, 97, Short.MAX_VALUE)
                .addComponent(_advancedAnalysisButton)
                .addContainerGap())
        );

        jTabbedPane1.addTab("Analysis", _analysisTab);

        _reportResultsAsHTML.setText("Report Results as HTML");

        _outputFileTextField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                _advancedParametersSetting.enableSequentialAnalysisGroup();
            }
        });

        _resultsFileLabel.setText("Results File:"); // NOI18N

        _resultsFileBrowseButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/folder_open_small.png"))); // NOI18N
        _resultsFileBrowseButton.setToolTipText("Browse for results file ...");
        _resultsFileBrowseButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                List<InputFileFilter> filters = new ArrayList<InputFileFilter>();
                filters.add(new InputFileFilter("txt","Results Files (*.txt)"));
                FileSelectionDialog select = new FileSelectionDialog(TreeScanApplication.getInstance(), "Select Results File", filters, TreeScanApplication.getInstance().lastBrowseDirectory);
                File file = select.browse_saveas();
                if (file != null) {
                    TreeScanApplication.getInstance().lastBrowseDirectory = select.getDirectory();
                    String filename = file.getAbsolutePath();
                    if (new File(filename).getName().lastIndexOf('.') == -1){
                        filename = filename + ".txt";
                    }
                    _outputFileTextField.setText(filename);
                }
                _advancedParametersSetting.enableSequentialAnalysisGroup();
            }
        });

        _resultsFileLabel1.setText("Additional Output Files:"); // NOI18N

        _reportResultsAsCsvTable.setText("Report Results as CSV Table");

        _advancedOutputButton.setText("Advanced >>"); // NOI18N
        _advancedOutputButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                getAdvancedParameterInternalFrame().setVisible(true, AdvancedParameterSettingsFrame.FocusedTabSet.OUTPUT);
                getAdvancedParameterInternalFrame().requestFocus();
            }
        });

        _generate_ncbi_asn.setText("Generate NCBI Genome Workbench ASN1 File");
        _generate_ncbi_asn.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                _generate_ncbi_asnActionPerformed(evt);
            }
        });

        _generate_newick.setText("Generate Newick Tree Format File");

        jLabel1.setText("Whole Tree Visualizations:");

        javax.swing.GroupLayout _outputTabLayout = new javax.swing.GroupLayout(_outputTab);
        _outputTab.setLayout(_outputTabLayout);
        _outputTabLayout.setHorizontalGroup(
            _outputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_outputTabLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_outputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(_outputTabLayout.createSequentialGroup()
                        .addComponent(_outputFileTextField, javax.swing.GroupLayout.DEFAULT_SIZE, 594, Short.MAX_VALUE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_resultsFileBrowseButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addComponent(_reportResultsAsCsvTable, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_reportResultsAsHTML, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_resultsFileLabel1, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _outputTabLayout.createSequentialGroup()
                        .addGap(0, 0, Short.MAX_VALUE)
                        .addComponent(_advancedOutputButton))
                    .addGroup(_outputTabLayout.createSequentialGroup()
                        .addComponent(_resultsFileLabel)
                        .addGap(0, 0, Short.MAX_VALUE))
                    .addComponent(jLabel1, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_generate_ncbi_asn, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_generate_newick, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        _outputTabLayout.setVerticalGroup(
            _outputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_outputTabLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_resultsFileLabel)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_outputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_resultsFileBrowseButton)
                    .addComponent(_outputFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_resultsFileLabel1)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_reportResultsAsHTML)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_reportResultsAsCsvTable)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(jLabel1)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_generate_ncbi_asn)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(_generate_newick)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, 145, Short.MAX_VALUE)
                .addComponent(_advancedOutputButton)
                .addContainerGap())
        );

        jTabbedPane1.addTab("Output", _outputTab);

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jTabbedPane1)
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jTabbedPane1)
                .addContainerGap())
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private void _generate_ncbi_asnActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event__generate_ncbi_asnActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event__generate_ncbi_asnActionPerformed

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JRadioButton _BernoulliButton;
    private javax.swing.JRadioButton _PoissonButton;
    private javax.swing.JButton _advancedAnalysisButton;
    private javax.swing.JButton _advancedInputButton;
    private javax.swing.JButton _advancedOutputButton;
    private javax.swing.JPanel _analysisTab;
    private javax.swing.JRadioButton _bernoulliTimeButton;
    private javax.swing.JRadioButton _conditionalBranchCasesButton;
    private javax.swing.JRadioButton _conditionalNodeTimeButton;
    private javax.swing.JRadioButton _conditionalTotalCasesButton;
    private javax.swing.JButton _controlFileImportButton;
    private javax.swing.JLabel _controlFileLabel;
    private javax.swing.JTextField _controlFileTextField;
    private javax.swing.JButton _countFileImportButton;
    private javax.swing.JLabel _countFileLabel;
    private javax.swing.JTextField _countFileTextField;
    private javax.swing.JLabel _endDateDayLabel;
    private javax.swing.JLabel _endDateLabel;
    private javax.swing.JLabel _endDateMonthLabel;
    private javax.swing.JLabel _endDateYearLabel;
    private javax.swing.JLabel _endRangeLabel;
    private javax.swing.JLabel _endRangeYearLabel;
    private javax.swing.JLabel _eventProbabilityLabel;
    private javax.swing.JLabel _eventProbabilityLabel2;
    private javax.swing.JTextField _eventProbabiltyDenominator;
    private javax.swing.JTextField _eventProbabiltyNumerator;
    private javax.swing.JCheckBox _generate_ncbi_asn;
    private javax.swing.JCheckBox _generate_newick;
    private javax.swing.JPanel _inputTab;
    public javax.swing.JTextField _outputFileTextField;
    private javax.swing.JPanel _outputTab;
    private javax.swing.JPanel _probabilityModelPanel;
    private javax.swing.JPanel _probabilityModelPanel1;
    private javax.swing.JCheckBox _reportResultsAsCsvTable;
    private javax.swing.JCheckBox _reportResultsAsHTML;
    private javax.swing.JButton _resultsFileBrowseButton;
    private javax.swing.JLabel _resultsFileLabel;
    private javax.swing.JLabel _resultsFileLabel1;
    private javax.swing.JPanel _scanStatisticPanel;
    private javax.swing.JPanel _scanStatisticPanel1;
    private javax.swing.JCheckBox _self_control_design;
    private javax.swing.JLabel _startDateDayLabel;
    private javax.swing.JLabel _startDateLabel;
    private javax.swing.JLabel _startDateMonthLabel;
    private javax.swing.JLabel _startDateYearLabel;
    private javax.swing.JLabel _startRangeDateLabel;
    private javax.swing.JLabel _startRangeYearLabel;
    private javax.swing.JPanel _studyPeriodComplete;
    public javax.swing.JTextField _studyPeriodEndDateDayTextField;
    public javax.swing.JTextField _studyPeriodEndDateGenericTextField;
    public javax.swing.JTextField _studyPeriodEndDateMonthTextField;
    public javax.swing.JTextField _studyPeriodEndDateYearTextField;
    private javax.swing.JPanel _studyPeriodGeneric;
    private javax.swing.JPanel _studyPeriodGroup;
    private javax.swing.JTextField _studyPeriodStartDateDayTextField;
    private javax.swing.JTextField _studyPeriodStartDateGenericTextField;
    private javax.swing.JTextField _studyPeriodStartDateMonthTextField;
    private javax.swing.JTextField _studyPeriodStartDateYearTextField;
    private javax.swing.ButtonGroup _timePrecisionButtonGroup;
    private javax.swing.JRadioButton _timePrecisionDay;
    private javax.swing.JRadioButton _timePrecisionGeneric;
    private javax.swing.JPanel _timePrecisionGroup;
    private javax.swing.JRadioButton _timePrecisionMonth;
    private javax.swing.JRadioButton _timePrecisionNone;
    private javax.swing.JRadioButton _timePrecisionYear;
    private javax.swing.JRadioButton _timeonlyScanType;
    private javax.swing.JButton _treeFileImportButton;
    private javax.swing.JLabel _treeFileLabel;
    private javax.swing.JRadioButton _treeOnlyScanType;
    private javax.swing.JTextField _treelFileTextField;
    private javax.swing.JRadioButton _treetimeScanType;
    private javax.swing.JRadioButton _unconditionalButton;
    private javax.swing.JRadioButton _uniformButton;
    private javax.swing.ButtonGroup conditionalButtonGroup;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JTabbedPane jTabbedPane1;
    private javax.swing.ButtonGroup scanButtonGroup;
    private javax.swing.ButtonGroup timtModelButtonGoup;
    private javax.swing.ButtonGroup treeModelButtonGroup;
    // End of variables declaration//GEN-END:variables

}
