package org.treescan.gui;

import java.awt.Component;
import java.awt.Container;
import java.beans.PropertyVetoException;
import java.io.File;
import javax.swing.ImageIcon;
import javax.swing.JOptionPane;
import javax.swing.JRootPane;
import javax.swing.event.InternalFrameEvent;
import javax.swing.event.InternalFrameListener;
import javax.swing.event.UndoableEditEvent;
import javax.swing.event.UndoableEditListener;
import javax.swing.undo.UndoManager;
import org.treescan.app.AdvFeaturesExpection;
import org.treescan.utils.FileAccess;
import org.treescan.importer.FileImporter;
import org.treescan.app.ParameterHistory;
import org.treescan.app.Parameters;
import org.treescan.app.UnknownEnumException;
import org.treescan.gui.utils.FileSelectionDialog;
import org.treescan.gui.utils.InputFileFilter;
import org.treescan.gui.utils.Utils;

/**
 * Parameter settings window.
 *
 * @author Hostovic
 */
public class ParameterSettingsFrame extends javax.swing.JInternalFrame implements InternalFrameListener {

    private Parameters _parameters = new Parameters();
    private Parameters _initialParameters = new Parameters();
    private boolean gbPromptOnExist = true;
    private final UndoManager undo = new UndoManager();
    private final JRootPane _rootPane;
    final static String STUDY_COMPLETE = "study_complete";
    final static String STUDY_GENERIC = "study_generic";
    private AdvancedParameterSettingsFrame _advancedParametersSetting = null;

    /**
     * Creates new form ParameterSettingsFrame
     */
    public ParameterSettingsFrame(final JRootPane rootPane, final String sParameterFilename) {
        initComponents();
        setFrameIcon(new ImageIcon(getClass().getResource("/TreeScan.png")));
        _rootPane = rootPane;
        addInternalFrameListener(this);
        setUp(sParameterFilename);
    }

    /**
     * launches 'save as' dialog to permit user saving current settings to
     * parameter file
     */
    public boolean SaveAs() {
        boolean bSaved = true;

        InputFileFilter[] filters = new InputFileFilter[]{new InputFileFilter("prm", "Settings Files (*.prm)")};
        FileSelectionDialog select = new FileSelectionDialog(org.treescan.gui.TreeScanApplication.getInstance(), "Select Parameters File", filters, org.treescan.gui.TreeScanApplication.getInstance().lastBrowseDirectory);
        File file = select.browse_saveas();
        if (file != null) {
            org.treescan.gui.TreeScanApplication.getInstance().lastBrowseDirectory = select.getDirectory();
            String filename = file.getAbsolutePath();
            if (new File(filename).getName().lastIndexOf('.') == -1) {
                filename = filename + ".prm";
            }
            WriteSession(filename);
            setTitle(filename);
        } else {
            bSaved = false;
        }
        return bSaved;
    }

    /**
     * Writes the session information to disk
     */
    public boolean WriteSession(String sParameterFilename) {
        String sFilename = sParameterFilename;
        boolean bSaved = true;

        if (sFilename.equals("")) {
            sFilename = _parameters.getSourceFileName();
        }
        if (sFilename == null || sFilename.equals("")) {
            bSaved = SaveAs();
        } else {
            if (!FileAccess.ValidateFileAccess(sFilename, true)) {
                JOptionPane.showInternalMessageDialog(this, "Unable to save session parameters.\n" + "Please confirm that the path and/or file name\n" + "are valid and that you have permissions to write\nto this directory and file.");
            } else {
                saveParameterSettings(_parameters);
                _parameters.setSourceFileName(sFilename);
                _parameters.Write(sFilename);
                _initialParameters = (Parameters) _parameters.clone();
            }
        }
        return bSaved;
    }

    /**
     * Returns reference to associated advanced parameters frame.
     */
    private AdvancedParameterSettingsFrame getAdvancedParameterInternalFrame() {
        return _advancedParametersSetting;
    }

    /**
     * enables correct advanced settings button on Analysis and Output tabs
     */
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

    /**
     * If necessary, removes from from iconized state and brings to front.
     */
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

    /**
     * Determines whether window can be closed by comparing parameter settings
     * contained in window verse intial parameter settings.
     */
    public boolean QueryWindowCanClose() {
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

    /**
     * Resets parameters that are not present in interface to default value.
     * Hidden features are to be used only in command line version at this time.
     */
    private void defaultHiddenParameters() {
        //TODO
    }

    /**
     * Reads parameter settings from file and loads frames controls.
     */
    private void setUp(final String sParameterFileName) {
        if (sParameterFileName.length() > 0) {
            _parameters.Read(sParameterFileName);
        }
        //catch (ZdException &x) {
        //  x.SetLevel(ZdException::Notify);
        //  x.SetErrorMessage((const char*)"SaTScan is unable to read parameters from file \"%s\".\n", sParameterFileName);
        //  throw;
        //}
        defaultHiddenParameters();
        setupInterface(_parameters);
        enableSettingsForStatisticModelCombination();
        enableAdvancedButtons();

        //Save orginal parameter settings to compare against when window closes but
        //first save what the interface has produced for the settings read from file.
        saveParameterSettings(_parameters);
        _initialParameters = (Parameters) _parameters.clone();
    }

    /*
     * Verifies that input settings are valid in the context of all parameter settings.
     */   
    private void CheckInputSettings() {
        //validate the tree file
        if (!_timeonlyScanType.isSelected() && _treelFileTextField.getText().length() == 0) {
            throw new SettingsException("Please specify a tree file.", (Component) _treelFileTextField);
        }
        if (!_timeonlyScanType.isSelected() && !FileAccess.ValidateFileAccess(_treelFileTextField.getText(), false)) {
            throw new SettingsException("The tree file could not be opened for reading.\n\nPlease confirm that the path and/or file name are valid and that you have permissions to read from this directory and file.", (Component) _treelFileTextField);
        }

        //validate the case file
        if (_countFileTextField.getText().length() == 0) {
            throw new SettingsException("Please specify a count file.", (Component) _countFileTextField);
        }
        if (!FileAccess.ValidateFileAccess(_countFileTextField.getText(), false)) {
            throw new SettingsException("The count file could not be opened for reading.\n\nPlease confirm that the path and/or file name are valid and that you have permissions to read from this directory and file.", (Component) _countFileTextField);
        }

        if (_treetimeScanType.isSelected() || _timeonlyScanType.isSelected()) {
            if (Integer.parseInt(_dataTimeRangeBegin.getText().trim()) >= Integer.parseInt(_dataTimeRangeEnd.getText().trim())) {
                throw new SettingsException("The data time range start must be before the data time range end.", (Component) _temporalEndWindowBegin);
            }
        }
    }

    /* Returns the number of time units in the specified data time range. */
    public int getNumUnitsInRange() {
        int start = Integer.parseInt(_dataTimeRangeBegin.getText().trim());
        int end = Integer.parseInt(_dataTimeRangeEnd.getText().trim());
        return start >= end ? 0 : end - start + 1;
    }

    /* Returns the number of time units in the specified temporal window. */
    public int getNumUnitsInTemporalWindow() {
        int start = Integer.parseInt(_temporalStartWindowBegin.getText().trim());
        int end = Integer.parseInt(_temporalEndWindowEnd.getText().trim());
        return start > end ? 0 : end - start + 1;
    }

    /* Returns the number of time units in the shortest period of specified temporal window. */
    public int getNumUnitsInShortestTemporalWindow() {
        int start_end = Integer.parseInt(_temporalStartWindowEnd.getText().trim());
        int end_start = Integer.parseInt(_temporalEndWindowBegin.getText().trim());
        // if the end of the start range overlaps the end range, the minimum is one unit.
        if (start_end >= end_start) {
            return 1;
        }
        int end_end = Integer.parseInt(_temporalEndWindowEnd.getText().trim());
        return start_end > end_end ? 0 : end_end - start_end + 1;
    }

    /*
     * Verifies that analysis settings are valid in the context of all parameter settings.
     */   
    private void CheckAnalysisSettings() {
        if (_BernoulliButton.isSelected()) {
            int eventProbNumerator = Integer.parseInt(_eventProbabiltyNumerator.getText().trim());
            int eventProbDenominator = Integer.parseInt(_eventProbabiltyDenominator.getText().trim());
            if (eventProbNumerator == 0 || eventProbDenominator == 0 || eventProbNumerator >= eventProbDenominator) {
                throw new SettingsException("Please specify an event probabilty that is between zero and one.", (Component) _eventProbabiltyNumerator);
            }
        }
        if (_treetimeScanType.isSelected() || _timeonlyScanType.isSelected()) {
            int temporalStartWindowBegin = Integer.parseInt(_temporalStartWindowBegin.getText().trim());
            int temporalStartWindowEnd = Integer.parseInt(_temporalStartWindowEnd.getText().trim());
            if (temporalStartWindowBegin > temporalStartWindowEnd) {
                throw new SettingsException("The temporal window start time range is invalid.\nThe start time must be less than or equal to end time.", (Component) _temporalStartWindowBegin);
            }
            int temporalEndWindowBegin = Integer.parseInt(_temporalEndWindowBegin.getText().trim());
            int temporalEndWindowEnd = Integer.parseInt(_temporalEndWindowEnd.getText().trim());
            if (temporalEndWindowBegin > temporalEndWindowEnd) {
                throw new SettingsException("The temporal window end time range is invalid.\nThe start time must be less than or equal to end time.", (Component) _temporalEndWindowBegin);
            }
            if (temporalEndWindowEnd < temporalStartWindowBegin) {
                throw new SettingsException("The temporal window end time range is invalid.\nThe end windows end time must be greater than or equal to start windows start time.", (Component) _temporalEndWindowBegin);
            }            
        }
    }

    /*
     * Verifies that output settings are valid in the context of all parameter settings.
     */   
    private void CheckOutputSettings() {
        if (_outputFileTextField.getText().length() == 0) {
            throw new SettingsException("Please specify a results file.", (Component) _outputFileTextField);
        }
        if (!FileAccess.ValidateFileAccess(_outputFileTextField.getText(), true)) {
            throw new SettingsException("Results file could not be opened for writing.\n" + "Please confirm that the path and/or file name\n" + "are valid and that you have permissions to write\nto this directory and file.",
                    (Component) _outputFileTextField);
        }
    }

    /*
     * Verifies that settings are valid in the context of all other parameter settings.
     */       
    public boolean CheckSettings() {
        try {
            CheckInputSettings();
            CheckAnalysisSettings();
            CheckOutputSettings();
            
            // inter-checks between data time range and temporal windows range
            int datarange_start = Integer.parseInt(_dataTimeRangeBegin.getText().trim());
            int datarange_end = Integer.parseInt(_dataTimeRangeEnd.getText().trim());

            int temporalstart_start = Integer.parseInt(_temporalStartWindowBegin.getText().trim());
            int temporalstart_end = Integer.parseInt(_temporalStartWindowEnd.getText().trim());           
            // Does the temporal start range reside within data range?
            if (!(datarange_start <= temporalstart_start && temporalstart_end <= datarange_end)) {
                throw new SettingsException("The temporal window start range is not within the data time range.", (Component) _temporalStartWindowBegin);            
            }
            int temporalend_start = Integer.parseInt(_temporalEndWindowBegin.getText().trim());
            int temporalend_end = Integer.parseInt(_temporalEndWindowEnd.getText().trim());
            // Does the temporal end range reside within data range?
            if (!(datarange_start <= temporalend_start && temporalend_end <= datarange_end)) {
                throw new SettingsException("The temporal window end range is not within the data time range.", (Component) _temporalEndWindowBegin);            
            }
            // Does the temporal window end range happen before start range.
            if (temporalend_end < temporalstart_start) {
                throw new SettingsException("The temporal window end range completely preceeds the start range.", (Component) _temporalEndWindowBegin);            
            }                        
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

    /**
     * setup interface from parameter settings
     */
    private void setupInterface(final Parameters parameters) {
        _advancedParametersSetting = new AdvancedParameterSettingsFrame(_rootPane, this, parameters);
        title = parameters.getSourceFileName();
        if (title == null || title.length() == 0) {
            title = "New Session";
        }
        _treelFileTextField.setText(parameters.getTreeFileName());
        _treelFileTextField.setCaretPosition(0);
        _countFileTextField.setText(parameters.getCountFileName());
        _countFileTextField.setCaretPosition(0);
        _dataTimeRangeBegin.setText(Integer.toString(parameters.getDataTimeRangeBegin()));
        _dataTimeRangeEnd.setText(Integer.toString(parameters.getDataTimeRangeClose()));

        _treeOnlyScanType.setSelected(parameters.getScanType() == Parameters.ScanType.TREEONLY);
        _treetimeScanType.setSelected(parameters.getScanType() == Parameters.ScanType.TREETIME);
        _timeonlyScanType.setSelected(parameters.getScanType() == Parameters.ScanType.TIMEONLY);
        _unconditionalButton.setSelected(parameters.getConditionalType() == Parameters.ConditionalType.UNCONDITIONAL);
        _conditionalTotalCasesButton.setSelected(parameters.getConditionalType() == Parameters.ConditionalType.TOTALCASES);
        _conditionalBranchCasesButton.setSelected(parameters.getConditionalType() == Parameters.ConditionalType.NODE);
        _conditionalNodeTimeButton.setSelected(parameters.getConditionalType() == Parameters.ConditionalType.NODEANDTIME);
        _PoissonButton.setSelected(parameters.getModelType() == Parameters.ModelType.POISSON);
        _BernoulliButton.setSelected(parameters.getModelType() == Parameters.ModelType.BERNOULLI);
        _uniformButton.setSelected(parameters.getModelType() == Parameters.ModelType.UNIFORM);
        _eventProbabiltyNumerator.setText(Integer.toString(parameters.getProbabilityRatioNumerator()));
        _eventProbabiltyDenominator.setText(Integer.toString(parameters.getProbabilityRatioDenominator()));
        _temporalStartWindowBegin.setText(Integer.toString(parameters.getTemporalStartRangeBegin()));
        _temporalStartWindowEnd.setText(Integer.toString(parameters.getTemporalStartRangeClose()));
        _temporalEndWindowBegin.setText(Integer.toString(parameters.getTemporalEndRangeBegin()));
        _temporalEndWindowEnd.setText(Integer.toString(parameters.getTemporalEndRangeClose()));

        _outputFileTextField.setText(parameters.getOutputFileName());
        _outputFileTextField.setCaretPosition(0);
        _reportResultsAsHTML.setSelected(parameters.isGeneratingHtmlResults());
        _reportResultsAsCsvTable.setSelected(parameters.isGeneratingTableResults());
    }

    /**
     * sets CParameters class with settings in form
     */
    private void saveParameterSettings(Parameters parameters) {
        setTitle(parameters.getSourceFileName());
        parameters.setTreeFileName(_treelFileTextField.getText());
        parameters.setCountFileName(_countFileTextField.getText());
        parameters.setDataTimeRangeBegin(Integer.parseInt(_dataTimeRangeBegin.getText()));
        parameters.setDataTimeRangeClose(Integer.parseInt(_dataTimeRangeEnd.getText()));

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
            parameters.setModelType(Parameters.ModelType.BERNOULLI.ordinal());
        } else if (_uniformButton.isEnabled() && _uniformButton.isSelected()) {
            parameters.setModelType(Parameters.ModelType.UNIFORM.ordinal());
        }
        parameters.setProbabilityRatioNumerator(Integer.parseInt(_eventProbabiltyNumerator.getText()));
        parameters.setProbabilityRatioDenominator(Integer.parseInt(_eventProbabiltyDenominator.getText()));

        parameters.setTemporalStartRangeBegin(Integer.parseInt(_temporalStartWindowBegin.getText()));
        parameters.setTemporalStartRangeClose(Integer.parseInt(_temporalStartWindowEnd.getText()));
        parameters.setTemporalEndRangeBegin(Integer.parseInt(_temporalEndWindowBegin.getText()));
        parameters.setTemporalEndRangeClose(Integer.parseInt(_temporalEndWindowEnd.getText()));

        parameters.setOutputFileName(_outputFileTextField.getText());
        parameters.setGeneratingHtmlResults(_reportResultsAsHTML.isSelected());
        parameters.setGeneratingTableResults(_reportResultsAsCsvTable.isSelected());
        getAdvancedParameterInternalFrame().saveParameterSettings(parameters);
    }

    public final Parameters getParameterSettings() {
        saveParameterSettings(_parameters);
        return _parameters;
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
            return Parameters.ModelType.BERNOULLI;
        if (_uniformButton.isSelected() && _uniformButton.isEnabled())
            return Parameters.ModelType.UNIFORM;        
        return null;
    }
    
    public void showExecOptionsDialog(java.awt.Frame parent) {
        new ExecutionOptionsDialog(parent, _parameters).setVisible(true);
    }

    /** Modally shows import dialog. */
    public void LaunchImporter(String sFileName, FileImporter.InputFileType eFileType) {
        ImportWizardDialog wizard = new ImportWizardDialog(TreeScanApplication.getInstance(), sFileName, _parameters.getSourceFileName(), eFileType, getModelType(), getScanType());
        wizard.setVisible(true);
        if (!wizard.getCancelled()) {
            switch (eFileType) {  // set parameters
                case Tree:
                    _treelFileTextField.setText(wizard.getDestinationFilename());
                    break;
                case Cuts:
                    getAdvancedParameterInternalFrame()._cutFileTextField.setText(wizard.getDestinationFilename());
                    break;
                case Case:
                    _countFileTextField.setText(wizard.getDestinationFilename());
                    break;
                default:
                    throw new UnknownEnumException(eFileType);
            }
        }
    }

    private void enableSettingsForStatisticModelCombination() {
        boolean treeOnly = _treeOnlyScanType.isSelected();
        boolean treeAndTime = _treetimeScanType.isSelected();
        boolean timeOnly = _timeonlyScanType.isSelected();

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
        // event probability inputs only available for unconditional Bernoulli
        boolean enabled = _BernoulliButton.isEnabled() && _BernoulliButton.isSelected() && _unconditionalButton.isSelected();
        _eventProbabilityLabel.setEnabled(enabled);
        _eventProbabilityLabel2.setEnabled(enabled);
        _eventProbabiltyNumerator.setEnabled(enabled);
        _eventProbabiltyDenominator.setEnabled(enabled);
        // temporal window inputs only available for tree-time or time-only
        _temporalWindowGroup.setEnabled(treeAndTime || timeOnly);
        _temporalStartWindowLabel.setEnabled(_temporalWindowGroup.isEnabled());
        _temporalStartWindowBegin.setEnabled(_temporalWindowGroup.isEnabled());
        _temporalStartWindowToLabel.setEnabled(_temporalWindowGroup.isEnabled());
        _temporalStartWindowEnd.setEnabled(_temporalWindowGroup.isEnabled());
        _temporalEndWindowLabel.setEnabled(_temporalWindowGroup.isEnabled());
        _temporalEndWindowBegin.setEnabled(_temporalWindowGroup.isEnabled());
        _temporalEndWindowToLabel.setEnabled(_temporalWindowGroup.isEnabled());
        _temporalEndWindowEnd.setEnabled(_temporalWindowGroup.isEnabled());
        _advancedParametersSetting.enableTemporalOptionsGroup(treeAndTime || timeOnly);
        _advancedParametersSetting.enableAdjustmentsOptions();
        _advancedParametersSetting.enablePowerEvaluationsGroup(); 
        _advancedParametersSetting.enableAdditionalOutputOptions();
        // data time range group
        _data_time_range_group.setEnabled(treeAndTime || timeOnly);
        _data_time_range_start_label.setEnabled(_data_time_range_group.isEnabled());
        _dataTimeRangeBegin.setEnabled(_data_time_range_group.isEnabled());
        _data_time_range_end_label.setEnabled(_data_time_range_group.isEnabled());
        _dataTimeRangeEnd.setEnabled(_data_time_range_group.isEnabled());
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
        jTabbedPane1 = new javax.swing.JTabbedPane();
        _analysisTab = new javax.swing.JPanel();
        _probabilityModelPanel = new javax.swing.JPanel();
        _PoissonButton = new javax.swing.JRadioButton();
        _BernoulliButton = new javax.swing.JRadioButton();
        _eventProbabiltyDenominator = new javax.swing.JTextField();
        _eventProbabiltyNumerator = new javax.swing.JTextField();
        _eventProbabilityLabel = new javax.swing.JLabel();
        _eventProbabilityLabel2 = new javax.swing.JLabel();
        _scanStatisticPanel = new javax.swing.JPanel();
        _conditionalTotalCasesButton = new javax.swing.JRadioButton();
        _unconditionalButton = new javax.swing.JRadioButton();
        _conditionalBranchCasesButton = new javax.swing.JRadioButton();
        _conditionalNodeTimeButton = new javax.swing.JRadioButton();
        _temporalWindowGroup = new javax.swing.JPanel();
        _temporalStartWindowLabel = new javax.swing.JLabel();
        _temporalStartWindowBegin = new javax.swing.JTextField();
        _temporalStartWindowToLabel = new javax.swing.JLabel();
        _temporalStartWindowEnd = new javax.swing.JTextField();
        _temporalEndWindowLabel = new javax.swing.JLabel();
        _temporalEndWindowBegin = new javax.swing.JTextField();
        _temporalEndWindowToLabel = new javax.swing.JLabel();
        _temporalEndWindowEnd = new javax.swing.JTextField();
        _scanStatisticPanel1 = new javax.swing.JPanel();
        _treetimeScanType = new javax.swing.JRadioButton();
        _treeOnlyScanType = new javax.swing.JRadioButton();
        _timeonlyScanType = new javax.swing.JRadioButton();
        _probabilityModelPanel1 = new javax.swing.JPanel();
        _uniformButton = new javax.swing.JRadioButton();
        _advancedAnalysisButton = new javax.swing.JButton();
        _inputTab = new javax.swing.JPanel();
        _treelFileTextField = new javax.swing.JTextField();
        _controlFileLabel = new javax.swing.JLabel();
        _treeFileBrowseButton = new javax.swing.JButton();
        _treeFileImportButton = new javax.swing.JButton();
        _countFileLabel = new javax.swing.JLabel();
        _countFileTextField = new javax.swing.JTextField();
        _countFileBrowseButton = new javax.swing.JButton();
        _countFileImportButton = new javax.swing.JButton();
        _data_time_range_group = new javax.swing.JPanel();
        _data_time_range_start_label = new javax.swing.JLabel();
        _dataTimeRangeBegin = new javax.swing.JTextField();
        _data_time_range_end_label = new javax.swing.JLabel();
        _dataTimeRangeEnd = new javax.swing.JTextField();
        _advancedInputButton = new javax.swing.JButton();
        _outputTab = new javax.swing.JPanel();
        _reportResultsAsHTML = new javax.swing.JCheckBox();
        _outputFileTextField = new javax.swing.JTextField();
        _resultsFileLabel = new javax.swing.JLabel();
        _resultsFileBrowseButton = new javax.swing.JButton();
        _resultsFileLabel1 = new javax.swing.JLabel();
        _reportResultsAsCsvTable = new javax.swing.JCheckBox();
        _advancedOutputButton = new javax.swing.JButton();

        setClosable(true);
        setDefaultCloseOperation(javax.swing.WindowConstants.DO_NOTHING_ON_CLOSE);
        setIconifiable(true);
        setResizable(true);

        _probabilityModelPanel.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Probability Model - Tree"));
        _probabilityModelPanel.setPreferredSize(new java.awt.Dimension(100, 108));

        treeModelButtonGroup.add(_PoissonButton);
        _PoissonButton.setSelected(true);
        _PoissonButton.setText("Poisson");
        _PoissonButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                    enableSettingsForStatisticModelCombination();
                }
            }
        });

        treeModelButtonGroup.add(_BernoulliButton);
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

        javax.swing.GroupLayout _probabilityModelPanelLayout = new javax.swing.GroupLayout(_probabilityModelPanel);
        _probabilityModelPanel.setLayout(_probabilityModelPanelLayout);
        _probabilityModelPanelLayout.setHorizontalGroup(
            _probabilityModelPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_probabilityModelPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_probabilityModelPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(_probabilityModelPanelLayout.createSequentialGroup()
                        .addGap(21, 21, 21)
                        .addComponent(_eventProbabilityLabel)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_eventProbabiltyNumerator, javax.swing.GroupLayout.PREFERRED_SIZE, 40, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_eventProbabilityLabel2)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_eventProbabiltyDenominator, javax.swing.GroupLayout.PREFERRED_SIZE, 40, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _probabilityModelPanelLayout.createSequentialGroup()
                        .addGroup(_probabilityModelPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                            .addComponent(_BernoulliButton, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .addComponent(_PoissonButton, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                        .addContainerGap())))
        );
        _probabilityModelPanelLayout.setVerticalGroup(
            _probabilityModelPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_probabilityModelPanelLayout.createSequentialGroup()
                .addComponent(_PoissonButton)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(_BernoulliButton)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_probabilityModelPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_eventProbabilityLabel)
                    .addComponent(_eventProbabiltyNumerator, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_eventProbabilityLabel2)
                    .addComponent(_eventProbabiltyDenominator, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        _scanStatisticPanel.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Conditional"));

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
                .addComponent(_unconditionalButton, javax.swing.GroupLayout.DEFAULT_SIZE, 151, Short.MAX_VALUE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(_conditionalTotalCasesButton, javax.swing.GroupLayout.DEFAULT_SIZE, 119, Short.MAX_VALUE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(_conditionalBranchCasesButton, javax.swing.GroupLayout.DEFAULT_SIZE, 91, Short.MAX_VALUE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(_conditionalNodeTimeButton, javax.swing.GroupLayout.DEFAULT_SIZE, 135, Short.MAX_VALUE)
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

        _temporalWindowGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Temporal Window"));

        _temporalStartWindowLabel.setText("Start Time in Range");

        _temporalStartWindowBegin.setText("0");
        _temporalStartWindowBegin.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                while (_temporalStartWindowBegin.getText().length() == 0)
                if (undo.canUndo()) undo.undo(); else _temporalStartWindowBegin.setText("0");
            }
        });
        _temporalStartWindowBegin.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validateNumericKeyTyped(_temporalStartWindowBegin, e, 10);
            }
        });
        _temporalStartWindowBegin.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });

        _temporalStartWindowToLabel.setText("to");

        _temporalStartWindowEnd.setText("100");
        _temporalStartWindowEnd.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                while (_temporalStartWindowEnd.getText().length() == 0)
                if (undo.canUndo()) undo.undo(); else _temporalStartWindowEnd.setText("100");
            }
        });
        _temporalStartWindowEnd.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validateNumericKeyTyped(_temporalStartWindowEnd, e, 10);
            }
        });
        _temporalStartWindowEnd.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });

        _temporalEndWindowLabel.setText("End Time in Range");

        _temporalEndWindowBegin.setText("0");
        _temporalEndWindowBegin.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                while (_temporalEndWindowBegin.getText().length() == 0)
                if (undo.canUndo()) undo.undo(); else _temporalEndWindowBegin.setText("0");
            }
        });
        _temporalEndWindowBegin.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validateNumericKeyTyped(_temporalEndWindowBegin, e, 10);
            }
        });
        _temporalEndWindowBegin.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });

        _temporalEndWindowToLabel.setText("to");

        _temporalEndWindowEnd.setText("100");
        _temporalEndWindowEnd.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                while (_temporalEndWindowEnd.getText().length() == 0)
                if (undo.canUndo()) undo.undo(); else _temporalEndWindowEnd.setText("100");
            }
        });
        _temporalEndWindowEnd.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validateNumericKeyTyped(_temporalEndWindowEnd, e, 10);
            }
        });
        _temporalEndWindowEnd.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });

        javax.swing.GroupLayout _temporalWindowGroupLayout = new javax.swing.GroupLayout(_temporalWindowGroup);
        _temporalWindowGroup.setLayout(_temporalWindowGroupLayout);
        _temporalWindowGroupLayout.setHorizontalGroup(
            _temporalWindowGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_temporalWindowGroupLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_temporalWindowGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                    .addComponent(_temporalEndWindowLabel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_temporalStartWindowLabel, javax.swing.GroupLayout.DEFAULT_SIZE, 97, Short.MAX_VALUE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_temporalWindowGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(_temporalWindowGroupLayout.createSequentialGroup()
                        .addComponent(_temporalStartWindowBegin, javax.swing.GroupLayout.PREFERRED_SIZE, 94, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                        .addComponent(_temporalStartWindowToLabel, javax.swing.GroupLayout.PREFERRED_SIZE, 30, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_temporalStartWindowEnd, javax.swing.GroupLayout.PREFERRED_SIZE, 91, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addGroup(_temporalWindowGroupLayout.createSequentialGroup()
                        .addComponent(_temporalEndWindowBegin, javax.swing.GroupLayout.PREFERRED_SIZE, 94, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                        .addComponent(_temporalEndWindowToLabel, javax.swing.GroupLayout.PREFERRED_SIZE, 30, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_temporalEndWindowEnd, javax.swing.GroupLayout.PREFERRED_SIZE, 91, javax.swing.GroupLayout.PREFERRED_SIZE)))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );
        _temporalWindowGroupLayout.setVerticalGroup(
            _temporalWindowGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_temporalWindowGroupLayout.createSequentialGroup()
                .addGroup(_temporalWindowGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_temporalStartWindowLabel)
                    .addComponent(_temporalStartWindowBegin, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_temporalStartWindowToLabel)
                    .addComponent(_temporalStartWindowEnd, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addGroup(_temporalWindowGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_temporalEndWindowLabel)
                    .addComponent(_temporalEndWindowBegin, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_temporalEndWindowToLabel)
                    .addComponent(_temporalEndWindowEnd, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
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
        _PoissonButton.addItemListener(new java.awt.event.ItemListener() {
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
                .addComponent(_uniformButton, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addContainerGap())
        );
        _probabilityModelPanel1Layout.setVerticalGroup(
            _probabilityModelPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_probabilityModelPanel1Layout.createSequentialGroup()
                .addComponent(_uniformButton)
                .addGap(0, 59, Short.MAX_VALUE))
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
                    .addComponent(_temporalWindowGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addGroup(_analysisTabLayout.createSequentialGroup()
                        .addComponent(_probabilityModelPanel, javax.swing.GroupLayout.DEFAULT_SIZE, 273, Short.MAX_VALUE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_probabilityModelPanel1, javax.swing.GroupLayout.DEFAULT_SIZE, 247, Short.MAX_VALUE))
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
                    .addComponent(_probabilityModelPanel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_probabilityModelPanel1, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_temporalWindowGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, 31, Short.MAX_VALUE)
                .addComponent(_advancedAnalysisButton)
                .addContainerGap())
        );

        jTabbedPane1.addTab("Analysis", _analysisTab);

        _controlFileLabel.setText("Tree File:"); // NOI18N

        _treeFileBrowseButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/folder_open_small.png"))); // NOI18N
        _treeFileBrowseButton.setToolTipText("Browse for tree file ..."); // NOI18N
        _treeFileBrowseButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                InputFileFilter[] filters = new InputFileFilter[]{new InputFileFilter("csv","CSV Files (*.csv)"), new InputFileFilter("txt","Text Files (*.txt)"), new InputFileFilter("tre","Tree Files (*.tre)")};
                FileSelectionDialog select = new FileSelectionDialog(org.treescan.gui.TreeScanApplication.getInstance(), "Select Tree File", filters, org.treescan.gui.TreeScanApplication.getInstance().lastBrowseDirectory);
                File file = select.browse_load(true);
                if (file != null) {
                    org.treescan.gui.TreeScanApplication.getInstance().lastBrowseDirectory = select.getDirectory();
                    _treelFileTextField.setText(file.getAbsolutePath());
                }
            }
        });

        _treeFileImportButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/document_add_small.png"))); // NOI18N
        _treeFileImportButton.setToolTipText("Import tree file ..."); // NOI18N
        _treeFileImportButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                try {
                    InputFileFilter[] filters = new InputFileFilter[]{new InputFileFilter("dbf","dBase Files (*.dbf)"),
                        new InputFileFilter("csv","Delimited Files (*.csv)"),
                        new InputFileFilter("xls","Excel Files (*.xls)"),
                        new InputFileFilter("txt","Text Files (*.txt)"),
                        new InputFileFilter("tre","Tree Files (*.tre)")};

                    FileSelectionDialog select = new FileSelectionDialog(org.treescan.gui.TreeScanApplication.getInstance(), "Select Tree File Import Source", filters, org.treescan.gui.TreeScanApplication.getInstance().lastBrowseDirectory);
                    File file = select.browse_load(true);
                    if (file != null) {
                        org.treescan.gui.TreeScanApplication.getInstance().lastBrowseDirectory = select.getDirectory();
                        LaunchImporter(file.getAbsolutePath(), FileImporter.InputFileType.Tree);
                    }
                } catch (Throwable t) {
                    new ExceptionDialog(org.treescan.gui.TreeScanApplication.getInstance(), t).setVisible(true);
                }
            }
        });

        _countFileLabel.setText("Count File:"); // NOI18N

        _countFileBrowseButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/folder_open_small.png"))); // NOI18N
        _countFileBrowseButton.setToolTipText("Browse for count file ..."); // NOI18N
        _countFileBrowseButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                InputFileFilter[] filters = new InputFileFilter[]{new InputFileFilter("csv","CSV Files (*.csv)"), new InputFileFilter("txt","Text Files (*.txt)"), new InputFileFilter("cas","Count Files (*.cas)")};
                FileSelectionDialog select = new FileSelectionDialog(org.treescan.gui.TreeScanApplication.getInstance(), "Select Count File", filters, org.treescan.gui.TreeScanApplication.getInstance().lastBrowseDirectory);
                File file = select.browse_load(true);
                if (file != null) {
                    org.treescan.gui.TreeScanApplication.getInstance().lastBrowseDirectory = select.getDirectory();
                    _countFileTextField.setText(file.getAbsolutePath());
                }
            }
        });

        _countFileImportButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/document_add_small.png"))); // NOI18N
        _countFileImportButton.setToolTipText("Import count file ..."); // NOI18N
        _countFileImportButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                try {
                    InputFileFilter[] filters = new InputFileFilter[]{new InputFileFilter("dbf","dBase Files (*.dbf)"),
                        new InputFileFilter("csv","Delimited Files (*.csv)"),
                        new InputFileFilter("xls","Excel Files (*.xls)"),
                        new InputFileFilter("txt","Text Files (*.txt)"),
                        new InputFileFilter("cas","Count Files (*.cas)")};

                    FileSelectionDialog select = new FileSelectionDialog(org.treescan.gui.TreeScanApplication.getInstance(), "Select Count File Import Source", filters, org.treescan.gui.TreeScanApplication.getInstance().lastBrowseDirectory);
                    File file = select.browse_load(true);
                    if (file != null) {
                        org.treescan.gui.TreeScanApplication.getInstance().lastBrowseDirectory = select.getDirectory();
                        LaunchImporter(file.getAbsolutePath(), FileImporter.InputFileType.Case);
                    }
                } catch (Throwable t) {
                    new ExceptionDialog(org.treescan.gui.TreeScanApplication.getInstance(), t).setVisible(true);
                }
            }
        });

        _data_time_range_group.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Data Time Range"));

        _data_time_range_start_label.setText("Range Start");

        _dataTimeRangeBegin.setText("0");
        _dataTimeRangeBegin.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                while (_dataTimeRangeBegin.getText().length() == 0)
                if (undo.canUndo()) undo.undo(); else _dataTimeRangeBegin.setText("0");
            }
        });
        _dataTimeRangeBegin.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validateNumericKeyTyped(_dataTimeRangeBegin, e, 10);
            }
        });
        _dataTimeRangeBegin.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });

        _data_time_range_end_label.setText("Range End");

        _dataTimeRangeEnd.setText("100");
        _dataTimeRangeEnd.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                while (_dataTimeRangeEnd.getText().length() == 0)
                if (undo.canUndo()) undo.undo(); else _dataTimeRangeEnd.setText("100");
            }
        });
        _dataTimeRangeEnd.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validateNumericKeyTyped(_dataTimeRangeEnd, e, 10);
            }
        });
        _dataTimeRangeEnd.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });

        javax.swing.GroupLayout _data_time_range_groupLayout = new javax.swing.GroupLayout(_data_time_range_group);
        _data_time_range_group.setLayout(_data_time_range_groupLayout);
        _data_time_range_groupLayout.setHorizontalGroup(
            _data_time_range_groupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _data_time_range_groupLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_data_time_range_start_label)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_dataTimeRangeBegin, javax.swing.GroupLayout.PREFERRED_SIZE, 82, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addGap(18, 18, 18)
                .addComponent(_data_time_range_end_label)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_dataTimeRangeEnd, javax.swing.GroupLayout.PREFERRED_SIZE, 91, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );
        _data_time_range_groupLayout.setVerticalGroup(
            _data_time_range_groupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_data_time_range_groupLayout.createSequentialGroup()
                .addGroup(_data_time_range_groupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_dataTimeRangeBegin, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_dataTimeRangeEnd, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_data_time_range_start_label)
                    .addComponent(_data_time_range_end_label))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        _advancedInputButton.setText("Advanced >>"); // NOI18N
        _advancedInputButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                getAdvancedParameterInternalFrame().setVisible(true, AdvancedParameterSettingsFrame.FocusedTabSet.INPUT);
                getAdvancedParameterInternalFrame().requestFocus();
            }
        });

        javax.swing.GroupLayout _inputTabLayout = new javax.swing.GroupLayout(_inputTab);
        _inputTab.setLayout(_inputTabLayout);
        _inputTabLayout.setHorizontalGroup(
            _inputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_inputTabLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_inputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_data_time_range_group, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _inputTabLayout.createSequentialGroup()
                        .addComponent(_treelFileTextField)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_treeFileBrowseButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_treeFileImportButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _inputTabLayout.createSequentialGroup()
                        .addComponent(_countFileTextField)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_countFileBrowseButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_countFileImportButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addGroup(_inputTabLayout.createSequentialGroup()
                        .addGroup(_inputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(_controlFileLabel)
                            .addComponent(_countFileLabel))
                        .addGap(0, 0, Short.MAX_VALUE))
                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _inputTabLayout.createSequentialGroup()
                        .addGap(0, 427, Short.MAX_VALUE)
                        .addComponent(_advancedInputButton)))
                .addContainerGap())
        );
        _inputTabLayout.setVerticalGroup(
            _inputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_inputTabLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_controlFileLabel)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_inputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_treelFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_treeFileImportButton)
                    .addComponent(_treeFileBrowseButton))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(_countFileLabel)
                .addGap(9, 9, 9)
                .addGroup(_inputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_countFileBrowseButton)
                    .addComponent(_countFileImportButton)
                    .addComponent(_countFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(_data_time_range_group, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, 185, Short.MAX_VALUE)
                .addComponent(_advancedInputButton)
                .addContainerGap())
        );

        jTabbedPane1.addTab("Input", _inputTab);

        _reportResultsAsHTML.setText("Report Results as HTML");

        _resultsFileLabel.setText("Results File:"); // NOI18N

        _resultsFileBrowseButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/folder_open_small.png"))); // NOI18N
        _resultsFileBrowseButton.setToolTipText("Browse for results file ...");
        _resultsFileBrowseButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                InputFileFilter[] filters = new InputFileFilter[]{new InputFileFilter("txt","Results Files (*.txt)")};
                FileSelectionDialog select = new FileSelectionDialog(org.treescan.gui.TreeScanApplication.getInstance(), "Select Results File", filters, org.treescan.gui.TreeScanApplication.getInstance().lastBrowseDirectory);
                File file = select.browse_saveas();
                if (file != null) {
                    org.treescan.gui.TreeScanApplication.getInstance().lastBrowseDirectory = select.getDirectory();
                    String filename = file.getAbsolutePath();
                    if (new File(filename).getName().lastIndexOf('.') == -1){
                        filename = filename + ".txt";
                    }
                    _outputFileTextField.setText(filename);
                }
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

        javax.swing.GroupLayout _outputTabLayout = new javax.swing.GroupLayout(_outputTab);
        _outputTab.setLayout(_outputTabLayout);
        _outputTabLayout.setHorizontalGroup(
            _outputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_outputTabLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_outputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(_outputTabLayout.createSequentialGroup()
                        .addComponent(_outputFileTextField, javax.swing.GroupLayout.DEFAULT_SIZE, 495, Short.MAX_VALUE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_resultsFileBrowseButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addGroup(_outputTabLayout.createSequentialGroup()
                        .addComponent(_resultsFileLabel)
                        .addGap(0, 0, Short.MAX_VALUE))
                    .addComponent(_reportResultsAsCsvTable, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_reportResultsAsHTML, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_resultsFileLabel1, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _outputTabLayout.createSequentialGroup()
                        .addGap(0, 0, Short.MAX_VALUE)
                        .addComponent(_advancedOutputButton)))
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
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, 244, Short.MAX_VALUE)
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
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JRadioButton _BernoulliButton;
    private javax.swing.JRadioButton _PoissonButton;
    private javax.swing.JButton _advancedAnalysisButton;
    private javax.swing.JButton _advancedInputButton;
    private javax.swing.JButton _advancedOutputButton;
    private javax.swing.JPanel _analysisTab;
    private javax.swing.JRadioButton _conditionalBranchCasesButton;
    private javax.swing.JRadioButton _conditionalNodeTimeButton;
    private javax.swing.JRadioButton _conditionalTotalCasesButton;
    private javax.swing.JLabel _controlFileLabel;
    private javax.swing.JButton _countFileBrowseButton;
    private javax.swing.JButton _countFileImportButton;
    private javax.swing.JLabel _countFileLabel;
    private javax.swing.JTextField _countFileTextField;
    private javax.swing.JTextField _dataTimeRangeBegin;
    private javax.swing.JTextField _dataTimeRangeEnd;
    private javax.swing.JLabel _data_time_range_end_label;
    private javax.swing.JPanel _data_time_range_group;
    private javax.swing.JLabel _data_time_range_start_label;
    private javax.swing.JLabel _eventProbabilityLabel;
    private javax.swing.JLabel _eventProbabilityLabel2;
    private javax.swing.JTextField _eventProbabiltyDenominator;
    private javax.swing.JTextField _eventProbabiltyNumerator;
    private javax.swing.JPanel _inputTab;
    private javax.swing.JTextField _outputFileTextField;
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
    private javax.swing.JTextField _temporalEndWindowBegin;
    private javax.swing.JTextField _temporalEndWindowEnd;
    private javax.swing.JLabel _temporalEndWindowLabel;
    private javax.swing.JLabel _temporalEndWindowToLabel;
    private javax.swing.JTextField _temporalStartWindowBegin;
    private javax.swing.JTextField _temporalStartWindowEnd;
    private javax.swing.JLabel _temporalStartWindowLabel;
    private javax.swing.JLabel _temporalStartWindowToLabel;
    private javax.swing.JPanel _temporalWindowGroup;
    private javax.swing.JRadioButton _timeonlyScanType;
    private javax.swing.JButton _treeFileBrowseButton;
    private javax.swing.JButton _treeFileImportButton;
    private javax.swing.JRadioButton _treeOnlyScanType;
    private javax.swing.JTextField _treelFileTextField;
    private javax.swing.JRadioButton _treetimeScanType;
    private javax.swing.JRadioButton _unconditionalButton;
    private javax.swing.JRadioButton _uniformButton;
    private javax.swing.ButtonGroup conditionalButtonGroup;
    private javax.swing.JTabbedPane jTabbedPane1;
    private javax.swing.ButtonGroup scanButtonGroup;
    private javax.swing.ButtonGroup timtModelButtonGoup;
    private javax.swing.ButtonGroup treeModelButtonGroup;
    // End of variables declaration//GEN-END:variables

    @Override
    public void internalFrameOpened(InternalFrameEvent e) {
    }

    @Override
    public void internalFrameClosing(InternalFrameEvent e) {
        if ((gbPromptOnExist ? (QueryWindowCanClose() ? true : false) : true) == true) {
            ParameterHistory.getInstance().AddParameterToHistory(_parameters.getSourceFileName());
            dispose();
        }
    }

    @Override
    public void internalFrameClosed(InternalFrameEvent e) {
    }

    public void internalFrameIconified(InternalFrameEvent e) {
    }

    public void internalFrameDeiconified(InternalFrameEvent e) {
    }

    public void internalFrameActivated(InternalFrameEvent e) {
    }

    public void internalFrameDeactivated(InternalFrameEvent e) {
    }
    /* Exception class that notes the Component that caused the exceptional situation. */

    public class SettingsException extends RuntimeException {

        private static final long serialVersionUID = 1L;
        public final Component focusComponent;

        public SettingsException(Component focusComponent) {
            super();
            this.focusComponent = focusComponent;
        }

        public SettingsException(String arg0, Component focusComponent) {
            super(arg0);
            this.focusComponent = focusComponent;
        }

        public SettingsException(String arg0, Throwable arg1, Component focusComponent) {
            super(arg0, arg1);
            this.focusComponent = focusComponent;
        }

        public SettingsException(Throwable arg0, Component focusComponent) {
            super(arg0);
            this.focusComponent = focusComponent;
        }

        /**
         * recursively searches Container objects contained in 'rootComponent'
         * for for 'searchComponent'.
         */
        boolean isContainedComponent(Component rootComponent, Component searchComponent) {
            if (rootComponent == searchComponent) {
                return true;
            }
            try {
                if (Class.forName("java.awt.Container").isInstance(rootComponent)) {
                    Container rootContainer = (Container) rootComponent;
                    for (int j = 0; j < rootContainer.getComponentCount(); ++j) {
                        if (isContainedComponent(rootContainer.getComponent(j), searchComponent)) {
                            return true;
                        }
                    }
                }
            } catch (ClassNotFoundException e) {
            }
            return false;
        }

        public void setControlFocus() {
            focusComponent.requestFocus();
        }
    }
}
