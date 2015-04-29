package org.treescan.gui;

import java.awt.Component;
import java.awt.Container;
import java.io.File;
import javax.swing.DefaultListModel;
import javax.swing.ImageIcon;
import javax.swing.JPanel;
import javax.swing.JRootPane;
import javax.swing.event.MouseInputAdapter;
import javax.swing.event.UndoableEditEvent;
import javax.swing.event.UndoableEditListener;
import javax.swing.undo.UndoManager;
import org.treescan.app.AdvFeaturesExpection;
import org.treescan.utils.FileAccess;
import org.treescan.app.Parameters;
import org.treescan.gui.utils.FileSelectionDialog;
import org.treescan.gui.utils.InputFileFilter;
import org.treescan.gui.utils.Utils;
import org.treescan.importer.FileImporter;
import org.treescan.app.UnknownEnumException;

public class AdvancedParameterSettingsFrame extends javax.swing.JInternalFrame {
    public enum FocusedTabSet { INPUT, ANALYSIS, OUTPUT };
    private JPanel _glass = null;
    private final JRootPane _rootPane;
    private final Component _rootPaneInitialGlass;
    private final UndoManager undo = new UndoManager();
    private final ParameterSettingsFrame _settings_window;
    private DefaultListModel _dataSetsListModel = new DefaultListModel();
    private FocusedTabSet _focusedTabSet = FocusedTabSet.INPUT;

    /**
     * Creates new form ParameterSettingsFrame
     */
    public AdvancedParameterSettingsFrame(final JRootPane rootPane, final ParameterSettingsFrame analysisSettingsWindow, final Parameters parameters) {
        initComponents();

        setFrameIcon(new ImageIcon(getClass().getResource("/TreeScan.png")));
        _rootPane = rootPane;
        _settings_window = analysisSettingsWindow;
        _rootPaneInitialGlass = rootPane.getGlassPane();
        // create opaque glass pane
        _glass = new JPanel();
        _glass.setOpaque(false);
        // Attach mouse listeners
        MouseInputAdapter adapter = new MouseInputAdapter() {
        };
        _glass.addMouseListener(adapter);
        _glass.addMouseMotionListener(adapter);
        // Add modal internal frame to glass pane
        _glass.add(this);
        setupInterface(parameters);
    }

    /**
     * recursively searches Container objects contained in 'rootComponent' for
     * for 'searchComponent'.
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

    /**
     * Sets tab set visible and attempts to set focus on 'focusComponent'.
     */
    public void setVisible(FocusedTabSet focusedTabSet, Component focusComponent) {
        //set tab set visible
        setVisible(true, focusedTabSet);
        //find focus component and request focus
        for (int i = 0; i < jTabbedPane1.getTabCount(); ++i) {
            if (isContainedComponent(jTabbedPane1.getComponentAt(i), focusComponent)) {
                jTabbedPane1.setSelectedIndex(i);
                focusComponent.requestFocus();
                return;
            }
        }
    }

    /** */
    public void setVisible(boolean value, FocusedTabSet focusedTabSet) {
        if (value == false) {
            _closeButton.requestFocus();
        } //cause any text controls to loose focus
        super.setVisible(value);
        if (value) {
            startModal(focusedTabSet);
            enableSetDefaultsButton();
        } else {
            stopModal();
            _settings_window.enableAdvancedButtons();
        }
    }

    /**
     * enables 'Set Defaults' button
     */
    private void enableSetDefaultsButton() {
        // update enable/disable of Set Defaults button
        switch (_focusedTabSet) {
            case INPUT:
                _setDefaultButton.setEnabled(!getDefaultsSetForInputOptions());
                break;
            case ANALYSIS:
                _setDefaultButton.setEnabled(!getDefaultsSetForAnalysisOptions());
                break;
            case OUTPUT:
                _setDefaultButton.setEnabled(!getDefaultsSetForOutputOptions());
                break;
        }
    }

    /**
     * Checks to determine if only default values are set in the dialog Returns
     * true if only default values are set Returns false if user specified a
     * value other than a default
     */
    public boolean getDefaultsSetForInputOptions() {
        boolean bReturn = true;
        bReturn &= (_cutFileTextField.getText().equals(""));
        return bReturn;
    }

    public boolean getDefaultsSetForAnalysisOptions() {
        boolean bReturn = true;
        bReturn &= (Integer.parseInt(_montCarloReplicationsTextField.getText()) == 999);
        bReturn &= _percentageTemporalRadioButton.isSelected();
        bReturn &= (Double.parseDouble(_maxTemporalClusterSizeTextField.getText()) == 50.0);
        bReturn &= (Integer.parseInt(_maxTemporalClusterSizeUnitsTextField.getText()) == 1);
        bReturn &= (Integer.parseInt(_minTemporalClusterSizeUnitsTextField.getText()) == 2);        
        // Power Evaluations tab
        bReturn &= (_performPowerEvalautions.isSelected() == false);
        bReturn &= (_partOfRegularAnalysis.isSelected() == true);
        bReturn &= _totalPowerCases.getText().equals("600");
        bReturn &= _alternativeHypothesisFilename.getText().equals("");
        bReturn &= _numberPowerReplications.getText().equals("1000");  
        bReturn &= _perform_dayofweek_adjustments.isSelected() == false;
        return bReturn;
    }

    public boolean getDefaultsSetForOutputOptions() {
        return _reportLLRResultsAsCsvTable.isSelected() == false && _reportCriticalValuesCheckBox.isSelected() == false; 
    }

    private Parameters.PowerEvaluationType getPowerEvaluationMethodType() {
        Parameters.PowerEvaluationType eReturn = null;

        if (_partOfRegularAnalysis.isSelected()) {
            eReturn = Parameters.PowerEvaluationType.PE_WITH_ANALYSIS;
        } else if (_powerEvaluationWithCaseFile.isSelected()) {
            eReturn = Parameters.PowerEvaluationType.PE_ONLY_CASEFILE;
        } else if (_powerEvaluationWithSpecifiedCases.isSelected()) {
            eReturn = Parameters.PowerEvaluationType.PE_ONLY_SPECIFIED_CASES;
        } else {
            throw new IllegalArgumentException("No power evaluation option selected.");
        }
        return eReturn;
    }
    
    private synchronized void startModal(FocusedTabSet focusedTabSet) {
        if (_glass != null) {
            _rootPane.setGlassPane(_glass);
            _glass.setVisible(true); // Change glass pane to our panel
        }
        setFocusedTabSet(focusedTabSet);
    }

    private void setFocusedTabSet(FocusedTabSet focusedTabSet) {
        jTabbedPane1.removeAll();
        if (_focusedTabSet != null) {
            _focusedTabSet = focusedTabSet;
        }
        switch (_focusedTabSet) {
            case OUTPUT:
                setTitle("Advanced Output Options");
                jTabbedPane1.addTab("Additional Output", null, _advanced_output_tab, null);
                break;
            case ANALYSIS:
                setTitle("Advanced Analysis Options");
                jTabbedPane1.addTab("Temporal Window", null, _advanced_temporal_window_tab, null);
                jTabbedPane1.addTab("Adjustments", null, _advanced_adjustments_tab, null);                
                jTabbedPane1.addTab("Inference", null, _advanced_inferenece_tab, null);
                jTabbedPane1.addTab("Power Evaluation", null, _advanced_power_evaluation_tab, null);                
                break;
            case INPUT:
            default:
                setTitle("Advanced Input Options");
                jTabbedPane1.addTab("Advanced Input", null, _advanced_input_tab, null);
        }
    }

    private synchronized void stopModal() {
        if (_glass != null) {
            _glass.setVisible(false);
            //reset root pane glass to original
            _rootPane.setGlassPane(_rootPaneInitialGlass);
        }
    }
   
    /**
     * sets Parameters class with settings in form
     */
    public void saveParameterSettings(Parameters parameters) {
        // Input tab
        parameters.setCutsFileName(_cutFileTextField.getText());

        // Inference tab
        parameters.setNumReplications(Integer.parseInt(_montCarloReplicationsTextField.getText()));

        // Temporal Window tab
        parameters.setMaximumWindowPercentage(Double.parseDouble(_maxTemporalClusterSizeTextField.getText()));
        parameters.setMaximumWindowLength(Integer.parseInt(_maxTemporalClusterSizeUnitsTextField.getText()));
        parameters.setMaximumWindowType(_percentageTemporalRadioButton.isSelected() ? Parameters.MaximumWindowType.PERCENTAGE_WINDOW : Parameters.MaximumWindowType.FIXED_LENGTH);
        parameters.setMinimumWindowLength(Integer.parseInt(_minTemporalClusterSizeUnitsTextField.getText()));
        
        // Adjustments tab
        parameters.setPerformDayOfWeekAdjustment(_perform_dayofweek_adjustments.isEnabled() && _perform_dayofweek_adjustments.isSelected());
        
        // Power Evaluations tab
        parameters.setPerformPowerEvaluations(_powerEvaluationsGroup.isEnabled() && _performPowerEvalautions.isSelected());
        parameters.setPowerEvaluationType(getPowerEvaluationMethodType().ordinal());
        parameters.setPowerEvaluationTotalCases(Integer.parseInt((_totalPowerCases.getText().length() > 0 ? _totalPowerCases.getText() : "600")));
        parameters.setPowerEvaluationReplications(Integer.parseInt(_numberPowerReplications.getText()));
        parameters.setPowerEvaluationAltHypothesisFilename(_alternativeHypothesisFilename.getText());
        
        // Additional Output tab
        parameters.setGeneratingLLRResults(_reportLLRResultsAsCsvTable.isSelected());
        parameters.setReportCriticalValues(_reportCriticalValuesCheckBox.isSelected());
    }

    /**
     * Resets advanced settings to default values
     */
    private void setDefaultsClick() {
        switch (_focusedTabSet) {
            case INPUT:
                setDefaultsForInputTab();
                break;
            case ANALYSIS:
                setDefaultsForAnalysisTabs();
                break;
            case OUTPUT:
                setDefaultsForOutputTab();
                break;
        }
        enableSetDefaultsButton();
    }

    /**
     * Sets default values for Input related tab and respective controls
     */
    private void setDefaultsForInputTab() {
        _cutFileTextField.setText("");
    }

    /**
     * Sets default values for Analysis related tabs and their respective
     * controls pulled these default values from the CParameter class
     */
    private void setDefaultsForAnalysisTabs() {
        _montCarloReplicationsTextField.setText("999");
        _percentageTemporalRadioButton.setSelected(true);
        _maxTemporalClusterSizeTextField.setText("50");
        _maxTemporalClusterSizeUnitsTextField.setText("1");
        _minTemporalClusterSizeUnitsTextField.setText("2");
        // Adjustments tab
        _perform_dayofweek_adjustments.setSelected(false);
        // Power Evaluations tab
        _performPowerEvalautions.setSelected(false);
        _partOfRegularAnalysis.setSelected(true);
        _totalPowerCases.setText("600");
        _alternativeHypothesisFilename.setText("");
        _numberPowerReplications.setText("1000");       
    }

    private void setupInterface(final Parameters parameters) {
        // Advanced Input tab
        _cutFileTextField.setText(parameters.getCutsFileName());
        _cutFileTextField.setCaretPosition(0);
        
        // Inference tab
        _montCarloReplicationsTextField.setText(Integer.toString(parameters.getNumReplicationsRequested()));
        
        // Temporal Window tab
        _percentageTemporalRadioButton.setSelected(parameters.getMaximumWindowType() == Parameters.MaximumWindowType.PERCENTAGE_WINDOW);
        _timeTemporalRadioButton.setSelected(parameters.getMaximumWindowType() == Parameters.MaximumWindowType.FIXED_LENGTH);
        _maxTemporalClusterSizeTextField.setText(Double.toString(parameters.getMaximumWindowPercentage()));
        _maxTemporalClusterSizeUnitsTextField.setText(Integer.toString(parameters.getMaximumWindowLength()));
        _minTemporalClusterSizeUnitsTextField.setText(Integer.toString(parameters.getMinimumWindowLength()));
        
        // Adjustments tab
        _perform_dayofweek_adjustments.setSelected(parameters.getPerformDayOfWeekAdjustment());
        
        // Power Evaluations tab
        _performPowerEvalautions.setSelected(parameters.getPerformPowerEvaluations());
        _partOfRegularAnalysis.setSelected(parameters.getPowerEvaluationType() == Parameters.PowerEvaluationType.PE_ONLY_CASEFILE);
        _powerEvaluationWithCaseFile.setSelected(parameters.getPowerEvaluationType() == Parameters.PowerEvaluationType.PE_ONLY_CASEFILE);
        _powerEvaluationWithSpecifiedCases.setSelected(parameters.getPowerEvaluationType() == Parameters.PowerEvaluationType.PE_ONLY_SPECIFIED_CASES);
        _totalPowerCases.setText(Integer.toString(parameters.getPowerEvaluationTotalCases()));
        _numberPowerReplications.setText(Integer.toString(parameters.getPowerEvaluationReplications()));
        _alternativeHypothesisFilename.setText(parameters.getPowerEvaluationAltHypothesisFilename());
        
        // Additional Output tab
        _reportCriticalValuesCheckBox.setSelected(parameters.getReportCriticalValues());
        _reportLLRResultsAsCsvTable.setSelected(parameters.isGeneratingLLRResults());
        
        enablePowerEvaluationsGroup();
    }

    /**
     * Sets default values for Output related tab and respective controls pulled
     * these default values from the CParameter class
     */
    private void setDefaultsForOutputTab() {
        _reportLLRResultsAsCsvTable.setSelected(false);
        _reportCriticalValuesCheckBox.setSelected(false);
    }

    /** validates all the settings in this dialog */
    public void CheckSettings() {
        CheckInputSettings();
        CheckInferenceSettings();
        CheckTemporalWindowSize();
        CheckPowerEvaluationSettings();
    }

    /**
     * Validates the power evaluations parameters in conjunction with other
     * selected parameters.
     */
    private void CheckPowerEvaluationSettings() {
        if (_performPowerEvalautions.isEnabled() && _performPowerEvalautions.isSelected()) {
            if (_powerEvaluationWithSpecifiedCases.isSelected()) {
                if (Integer.parseInt(_totalPowerCases.getText()) < 2) {
                    throw new AdvFeaturesExpection("The number of power evaluation cases must be two or more.\n", FocusedTabSet.ANALYSIS, (Component) _totalPowerCases);
                }
                Parameters.ModelType modelType = _settings_window.getModelType();
                Parameters.ConditionalType conditonalType = _settings_window.getConditionalType();
                if (!(modelType == Parameters.ModelType.POISSON && conditonalType == Parameters.ConditionalType.TOTALCASES)) {
                    throw new AdvFeaturesExpection("The power evaluation option to define total cases is only permitted with the conditional Poisson model.\n", FocusedTabSet.ANALYSIS, (Component) _totalPowerCases);
                }
            }
            int replica = Integer.parseInt(_montCarloReplicationsTextField.getText());
            int replicaPE = Integer.parseInt(_numberPowerReplications.getText());
            if (replica < 999) {
                throw new AdvFeaturesExpection("The minimum number of standard replications in the power evaluation is 999.\n", FocusedTabSet.ANALYSIS, (Component) _montCarloReplicationsTextField);
            }
            if (replicaPE < 100) {
                throw new AdvFeaturesExpection("The minimum number of power replications in the power evaluation is 100.\n", FocusedTabSet.ANALYSIS, (Component) _numberPowerReplications);
            }
            if (replicaPE % 100 != 0) {
                throw new AdvFeaturesExpection("The number of power replications in the power evaluation must be a multiple of 100.\n", FocusedTabSet.ANALYSIS, (Component) _numberPowerReplications);
            }
            if (replicaPE > replica + 1) {
                throw new AdvFeaturesExpection("The number of standard replications must be at most one less than the number of power replications.\n", FocusedTabSet.ANALYSIS, (Component) _montCarloReplicationsTextField);
            }
            if (_alternativeHypothesisFilename.getText().length() == 0) {
                throw new AdvFeaturesExpection("Please specify an alternative hypothesis  filename.", FocusedTabSet.ANALYSIS, (Component) _alternativeHypothesisFilename);
            }
            if (!FileAccess.ValidateFileAccess(_alternativeHypothesisFilename.getText(), false)) {
                throw new AdvFeaturesExpection("The alternative hypothesis file could not be opened for reading.\n" + "Please confirm that the path and/or file name are valid\n" + "and that you have permissions to read from this directory\nand file.",
                        FocusedTabSet.ANALYSIS, (Component) _alternativeHypothesisFilename);
            }
        }
    }
    
    private void CheckInputSettings() {
        //validate the cuts file
        if (_settings_window.getScanType() !=  Parameters.ScanType.TIMEONLY && _cutFileTextField.getText().length() > 0 && !FileAccess.ValidateFileAccess(_cutFileTextField.getText(), false)) {
            throw new AdvFeaturesExpection("The cuts file could not be opened for reading.\n\nPlease confirm that the path and/or file name are valid and that you have permissions to read from this directory and file.", FocusedTabSet.INPUT, (Component) _cutFileTextField);
        }
    }

    private void CheckTemporalWindowSize() {
        String sErrorMessage;
        double maximumUnitsTemporalSize = 0;

        //check whether we are specifiying temporal information
        if (!_maxTemporalOptionsGroup.isEnabled()) {
            return;
        }

        if (_percentageTemporalRadioButton.isSelected()) {
            if (_maxTemporalClusterSizeTextField.getText().length() == 0 || Double.parseDouble(_maxTemporalClusterSizeTextField.getText()) == 0) {
                throw new AdvFeaturesExpection("Please specify a maximum temporal size.", FocusedTabSet.ANALYSIS, (Component) _maxTemporalClusterSizeTextField);
            }
            //check maximum temporal size does not exceed maximum value of 50%
            if (Double.parseDouble(_maxTemporalClusterSizeTextField.getText()) > 50.0) {
                sErrorMessage = "For the maximum temporal size, as a percent of the data time range, is 50 percent.";
                throw new AdvFeaturesExpection(sErrorMessage, FocusedTabSet.ANALYSIS, (Component) _maxTemporalClusterSizeTextField);
            }
            //check that the maximum temporal size of the data time range is at least one time unit
            int unitsInDataTimeRange = _settings_window.getNumUnitsInRange();
            maximumUnitsTemporalSize = Math.floor((double)unitsInDataTimeRange * Double.parseDouble(_maxTemporalClusterSizeTextField.getText()) / 100.0);
            if (maximumUnitsTemporalSize < 1) {
                sErrorMessage = "A maximum temporal cluster size as " + _maxTemporalClusterSizeTextField.getText() + "% of a " + unitsInDataTimeRange + " unit data time range\n" + "results in a maximum temporal size that is less than one time unit.\n";
                throw new AdvFeaturesExpection(sErrorMessage, FocusedTabSet.ANALYSIS, (Component) _maxTemporalClusterSizeTextField);
            }
        } else if (_timeTemporalRadioButton.isSelected()) {
            if (_maxTemporalClusterSizeUnitsTextField.getText().length() == 0 || Integer.parseInt(_maxTemporalClusterSizeUnitsTextField.getText()) == 0) {
                throw new AdvFeaturesExpection("Please specify a maximum temporal size.", FocusedTabSet.ANALYSIS, (Component) _maxTemporalClusterSizeUnitsTextField);
            }
            int unitsInDataTimeRange = _settings_window.getNumUnitsInRange();
            maximumUnitsTemporalSize = Math.floor((double)unitsInDataTimeRange * (50.0) / 100.0);
            if (Double.parseDouble(_maxTemporalClusterSizeUnitsTextField.getText()) > maximumUnitsTemporalSize) {
                sErrorMessage = "A maximum temporal size of " + _maxTemporalClusterSizeUnitsTextField.getText() + " time units exceeds 50% of a " + unitsInDataTimeRange + " unit data time range.\n" + "Note that current settings limit the maximum to " + (int)Math.floor(maximumUnitsTemporalSize) + " time units.";
                throw new AdvFeaturesExpection(sErrorMessage, FocusedTabSet.ANALYSIS, (Component) _maxTemporalClusterSizeUnitsTextField);
            }
            maximumUnitsTemporalSize = Integer.parseInt(_maxTemporalClusterSizeUnitsTextField.getText());
        }

        // validate the minimum temporal cluster size setting
        int minimumUnitsTemporalSize = Integer.parseInt(_minTemporalClusterSizeUnitsTextField.getText());
        if (minimumUnitsTemporalSize < 1) {
            sErrorMessage = "The minimum temporal size is 1 time unit";
            throw new AdvFeaturesExpection(sErrorMessage, FocusedTabSet.ANALYSIS, (Component) _minTemporalClusterSizeUnitsTextField);
        }
        // compare the maximum temporal cluster size to the minimum temporal cluster size
        if (minimumUnitsTemporalSize > maximumUnitsTemporalSize) {
            sErrorMessage = "The minimum temporal size is greater than the maximum temporal cluster size of " + (int)maximumUnitsTemporalSize + " time units.";
            throw new AdvFeaturesExpection(sErrorMessage, FocusedTabSet.ANALYSIS, (Component) _minTemporalClusterSizeUnitsTextField);
        }

        int shortestTemporalWindow = _settings_window.getNumUnitsInShortestTemporalWindow();
        // check whether any cuts will be evaluated given the specified maximum temporal window size and temporal window ranges
        if (maximumUnitsTemporalSize < shortestTemporalWindow) {
            throw new AdvFeaturesExpection("No cuts will be evaluated since the maximum window size is " + (int)maximumUnitsTemporalSize + " time units\nyet the minimum number of time units in temporal window is " + shortestTemporalWindow + ".",
                                           FocusedTabSet.ANALYSIS, (Component) _minTemporalClusterSizeUnitsTextField);
        }
        // check whether any cuts will be evaluated given the specified minimum temporal window size and temporal window ranges
        int unitsInTemporalWindow = _settings_window.getNumUnitsInTemporalWindow();
        if (minimumUnitsTemporalSize > unitsInTemporalWindow) {
            throw new AdvFeaturesExpection("No cuts will be evaluated since the minimum window size is " + minimumUnitsTemporalSize + "\nyet the maximum number of time units in temporal window is " + unitsInTemporalWindow + ".",
                                           FocusedTabSet.ANALYSIS, (Component) _minTemporalClusterSizeUnitsTextField);
        }
    }

    private void CheckInferenceSettings() {
        int dNumReplications;
        if (_montCarloReplicationsTextField.getText().trim().length() == 0) {
            throw new AdvFeaturesExpection("Please specify a number of Monte Carlo replications.\nChoices are: 0, 9, 999, or value ending in 999.", FocusedTabSet.ANALYSIS, (Component)_montCarloReplicationsTextField);
        }
        dNumReplications = Integer.parseInt(_montCarloReplicationsTextField.getText().trim());
        if (!((dNumReplications == 0 || dNumReplications == 9 || dNumReplications == 19 || (dNumReplications + 1) % 1000 == 0))) {
            throw new AdvFeaturesExpection("Invalid number of Monte Carlo replications.\nChoices are: 0, 9, 999, or value ending in 999.", FocusedTabSet.ANALYSIS, (Component) _montCarloReplicationsTextField);
        }
    }

    /**
     * enables or disables the temporal options group control
     */
    public void enableTemporalOptionsGroup(boolean bEnable) {
        _maxTemporalOptionsGroup.setEnabled(bEnable);
        _percentageTemporalRadioButton.setEnabled(bEnable);
        _maxTemporalClusterSizeTextField.setEnabled(bEnable && _percentageTemporalRadioButton.isSelected());
        _percentageOfStudyPeriodLabel.setEnabled(bEnable);
        _timeTemporalRadioButton.setEnabled(bEnable);
        _maxTemporalClusterSizeUnitsTextField.setEnabled(bEnable && _timeTemporalRadioButton.isSelected());
        _maxTemporalTimeUnitsLabel.setEnabled(bEnable);
        _minTemporalOptionsGroup.setEnabled(bEnable);
        _minTemporalClusterSizeUnitsTextField.setEnabled(bEnable);
        _minTemporalTimeUnitsLabel.setEnabled(bEnable);
    }
    
    /** enables options of the Adjustments tab */
    public void enableAdjustmentsOptions() {
        _perform_dayofweek_adjustments.setEnabled(_settings_window.getScanType() == Parameters.ScanType.TREETIME);
        if (_perform_dayofweek_adjustments.isEnabled()) {
            // switch label based upon condition type
            switch (_settings_window.getConditionalType()) {
                case NODE: _perform_dayofweek_adjustments.setText("Perform Day-of-Week Adjustment"); break;
                case NODEANDTIME: _perform_dayofweek_adjustments.setText("Perform Node by Day-of-Week Adjustment"); break;
                default: throw new UnknownEnumException(_settings_window.getConditionalType());
            }
        }
    }
    
    /**
     * Enabled the power evaluations group based upon current settings.
     */
    public void enablePowerEvaluationsGroup() {
        Parameters.ScanType scanType = _settings_window.getScanType();
        Parameters.ModelType eModelType = _settings_window.getModelType();

        boolean bEnableGroup = scanType == Parameters.ScanType.TREEONLY && 
                               (eModelType == Parameters.ModelType.POISSON || 
                                (eModelType == Parameters.ModelType.BERNOULLI && _settings_window.getConditionalType() == Parameters.ConditionalType.UNCONDITIONAL));
        _powerEvaluationsGroup.setEnabled(bEnableGroup);
        _performPowerEvalautions.setEnabled(bEnableGroup);
        _partOfRegularAnalysis.setEnabled(bEnableGroup && _performPowerEvalautions.isSelected());
        _powerEvaluationWithCaseFile.setEnabled(bEnableGroup && _performPowerEvalautions.isSelected());
        switch (_settings_window.getConditionalType()) {
            case TOTALCASES: _powerEvaluationWithCaseFile.setText("Power Evaluation Only, Use Total Cases From Case File"); break;
            case UNCONDITIONAL: _powerEvaluationWithCaseFile.setText("Power Evaluation Only"); break;
            default:
        }                
        _powerEvaluationWithSpecifiedCases.setEnabled(bEnableGroup && _performPowerEvalautions.isSelected() && 
                                                      eModelType == Parameters.ModelType.POISSON &&
                                                      _settings_window.getConditionalType() == Parameters.ConditionalType.TOTALCASES);        
        if (_powerEvaluationsGroup.isEnabled() && _powerEvaluationWithSpecifiedCases.isSelected() && !_powerEvaluationWithSpecifiedCases.isEnabled())
            _powerEvaluationWithCaseFile.setSelected(true);            
        _totalPowerCases.setEnabled(bEnableGroup && _performPowerEvalautions.isSelected() && _powerEvaluationWithSpecifiedCases.isSelected());
        _powerEvaluationWithSpecifiedCasesLabel.setEnabled(bEnableGroup && _performPowerEvalautions.isSelected() && _powerEvaluationWithSpecifiedCases.isSelected());
        _alternativeHypothesisFilenameLabel.setEnabled(bEnableGroup && _performPowerEvalautions.isSelected());
        _alternativeHypothesisFilename.setEnabled(bEnableGroup && _performPowerEvalautions.isSelected());
        _alternativeHypothesisFilenameButton.setEnabled(bEnableGroup && _performPowerEvalautions.isSelected());
        _numberPowerReplicationsLabel.setEnabled(bEnableGroup && _performPowerEvalautions.isSelected());
        _numberPowerReplications.setEnabled(bEnableGroup && _performPowerEvalautions.isSelected());
    }

    /**
     * This method is called from within the constructor to initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is always
     * regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        maximumWindowButtonGroup = new javax.swing.ButtonGroup();
        _powerEstimationButtonGroup = new javax.swing.ButtonGroup();
        jTabbedPane1 = new javax.swing.JTabbedPane();
        _advanced_input_tab = new javax.swing.JPanel();
        _cutFileLabel = new javax.swing.JLabel();
        _cutFileTextField = new javax.swing.JTextField();
        _cutFileBrowseButton = new javax.swing.JButton();
        _cutFileImportButton = new javax.swing.JButton();
        _advanced_temporal_window_tab = new javax.swing.JPanel();
        _maxTemporalOptionsGroup = new javax.swing.JPanel();
        _percentageTemporalRadioButton = new javax.swing.JRadioButton();
        _maxTemporalClusterSizeTextField = new javax.swing.JTextField();
        _percentageOfStudyPeriodLabel = new javax.swing.JLabel();
        _timeTemporalRadioButton = new javax.swing.JRadioButton();
        _maxTemporalTimeUnitsLabel = new javax.swing.JLabel();
        _maxTemporalClusterSizeUnitsTextField = new javax.swing.JTextField();
        _minTemporalOptionsGroup = new javax.swing.JPanel();
        _minTemporalClusterSizeUnitsTextField = new javax.swing.JTextField();
        _minTemporalTimeUnitsLabel = new javax.swing.JLabel();
        _advanced_inferenece_tab = new javax.swing.JPanel();
        jPanel1 = new javax.swing.JPanel();
        _labelMonteCarloReplications = new javax.swing.JLabel();
        _montCarloReplicationsTextField = new javax.swing.JTextField();
        _advanced_power_evaluation_tab = new javax.swing.JPanel();
        _powerEvaluationsGroup = new javax.swing.JPanel();
        _performPowerEvalautions = new javax.swing.JCheckBox();
        _partOfRegularAnalysis = new javax.swing.JRadioButton();
        _powerEvaluationWithCaseFile = new javax.swing.JRadioButton();
        _powerEvaluationWithSpecifiedCases = new javax.swing.JRadioButton();
        _totalPowerCases = new javax.swing.JTextField();
        _powerEvaluationWithSpecifiedCasesLabel = new javax.swing.JLabel();
        _numberPowerReplicationsLabel = new javax.swing.JLabel();
        _numberPowerReplications = new javax.swing.JTextField();
        _alternativeHypothesisFilenameLabel = new javax.swing.JLabel();
        _alternativeHypothesisFilename = new javax.swing.JTextField();
        _alternativeHypothesisFilenameButton = new javax.swing.JButton();
        _advanced_output_tab = new javax.swing.JPanel();
        _log_likelihood_ratios_group = new javax.swing.JPanel();
        _reportLLRResultsAsCsvTable = new javax.swing.JCheckBox();
        _report_critical_values_group = new javax.swing.JPanel();
        _reportCriticalValuesCheckBox = new javax.swing.JCheckBox();
        _advanced_adjustments_tab = new javax.swing.JPanel();
        _perform_dayofweek_adjustments = new javax.swing.JCheckBox();
        _closeButton = new javax.swing.JButton();
        _setDefaultButton = new javax.swing.JButton();

        setDefaultCloseOperation(javax.swing.WindowConstants.HIDE_ON_CLOSE);

        _cutFileLabel.setText("Cut File:"); // NOI18N

        _cutFileBrowseButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/folder_open_small.png"))); // NOI18N
        _cutFileBrowseButton.setToolTipText("Browse for cut file ..."); // NOI18N
        _cutFileBrowseButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                InputFileFilter[] filters = new InputFileFilter[]{new InputFileFilter("csv","CSV Files (*.csv)"), new InputFileFilter("txt","Text Files (*.txt)"), new InputFileFilter("cut","Cut Files (*.cut)")};
                FileSelectionDialog select = new FileSelectionDialog(org.treescan.gui.TreeScanApplication.getInstance(), "Select Cut File", filters, org.treescan.gui.TreeScanApplication.getInstance().lastBrowseDirectory);
                File file = select.browse_load(true);
                if (file != null) {
                    org.treescan.gui.TreeScanApplication.getInstance().lastBrowseDirectory = select.getDirectory();
                    _cutFileTextField.setText(file.getAbsolutePath());
                }
            }
        });

        _cutFileImportButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/document_add_small.png"))); // NOI18N
        _cutFileImportButton.setToolTipText("Import cut file ..."); // NOI18N
        _cutFileImportButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                try {
                    InputFileFilter[] filters = new InputFileFilter[]{new InputFileFilter("dbf","dBase Files (*.dbf)"),
                        new InputFileFilter("csv","Delimited Files (*.csv)"),
                        new InputFileFilter("xls","Excel Files (*.xls)"),
                        new InputFileFilter("txt","Text Files (*.txt)"),
                        new InputFileFilter("cut","Cut Files (*.cut)")};

                    FileSelectionDialog select = new FileSelectionDialog(org.treescan.gui.TreeScanApplication.getInstance(), "Select Cuts File Import Source", filters, org.treescan.gui.TreeScanApplication.getInstance().lastBrowseDirectory);
                    File file = select.browse_load(true);
                    if (file != null) {
                        org.treescan.gui.TreeScanApplication.getInstance().lastBrowseDirectory = select.getDirectory();
                        _settings_window.LaunchImporter(file.getAbsolutePath(), FileImporter.InputFileType.Cuts);
                    }
                } catch (Throwable t) {
                    new ExceptionDialog(org.treescan.gui.TreeScanApplication.getInstance(), t).setVisible(true);
                }
            }
        });

        javax.swing.GroupLayout _advanced_input_tabLayout = new javax.swing.GroupLayout(_advanced_input_tab);
        _advanced_input_tab.setLayout(_advanced_input_tabLayout);
        _advanced_input_tabLayout.setHorizontalGroup(
            _advanced_input_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_advanced_input_tabLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_advanced_input_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(_advanced_input_tabLayout.createSequentialGroup()
                        .addComponent(_cutFileTextField)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_cutFileBrowseButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_cutFileImportButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addContainerGap())
                    .addGroup(_advanced_input_tabLayout.createSequentialGroup()
                        .addComponent(_cutFileLabel)
                        .addGap(0, 552, Short.MAX_VALUE))))
        );
        _advanced_input_tabLayout.setVerticalGroup(
            _advanced_input_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_advanced_input_tabLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_cutFileLabel)
                .addGap(9, 9, 9)
                .addGroup(_advanced_input_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_cutFileBrowseButton)
                    .addComponent(_cutFileImportButton)
                    .addComponent(_cutFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(217, Short.MAX_VALUE))
        );

        jTabbedPane1.addTab("Advanced Input", _advanced_input_tab);

        _maxTemporalOptionsGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Maximum Temporal Size"));

        maximumWindowButtonGroup.add(_percentageTemporalRadioButton);
        _percentageTemporalRadioButton.setSelected(true);
        _percentageTemporalRadioButton.setText("is"); // NOI18N
        _percentageTemporalRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _percentageTemporalRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _percentageTemporalRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                    //cause enabling to be refreshed based upon clicked radio button
                    enableTemporalOptionsGroup(_maxTemporalOptionsGroup.isEnabled());
                    enableSetDefaultsButton();
                }
            }
        });

        _maxTemporalClusterSizeTextField.setText("50"); // NOI18N
        _maxTemporalClusterSizeTextField.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validatePostiveFloatKeyTyped(_maxTemporalClusterSizeTextField, e, 5);
            }
        });
        _maxTemporalClusterSizeTextField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                double dMaxValue =  50.0;
                while (_maxTemporalClusterSizeTextField.getText().length() == 0 ||
                    Double.parseDouble(_maxTemporalClusterSizeTextField.getText()) == 0 ||
                    Double.parseDouble(_maxTemporalClusterSizeTextField.getText()) > dMaxValue) {
                    if (undo.canUndo()) undo.undo(); else _maxTemporalClusterSizeTextField.setText("50");
                }
                enableSetDefaultsButton();
            }
        });
        _maxTemporalClusterSizeTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });

        _percentageOfStudyPeriodLabel.setText("percent of the data time range (<= 50%)"); // NOI18N

        maximumWindowButtonGroup.add(_timeTemporalRadioButton);
        _timeTemporalRadioButton.setText("is"); // NOI18N
        _timeTemporalRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _timeTemporalRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _timeTemporalRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                    //cause enabling to be refreshed based upon clicked radio button
                    enableTemporalOptionsGroup(_maxTemporalOptionsGroup.isEnabled());
                    enableSetDefaultsButton();
                }
            }
        });

        _maxTemporalTimeUnitsLabel.setText("data time units"); // NOI18N

        _maxTemporalClusterSizeUnitsTextField.setText("1"); // NOI18N
        _maxTemporalClusterSizeUnitsTextField.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validatePostiveNumericKeyTyped(_maxTemporalClusterSizeUnitsTextField, e, 6);
            }
        });
        _maxTemporalClusterSizeUnitsTextField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                while (_maxTemporalClusterSizeUnitsTextField.getText().length() == 0 || Double.parseDouble(_maxTemporalClusterSizeUnitsTextField.getText()) == 0) {
                    if (undo.canUndo()) undo.undo(); else _maxTemporalClusterSizeUnitsTextField.setText("1");
                }
                enableSetDefaultsButton();
            }
        });
        _maxTemporalClusterSizeUnitsTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });

        javax.swing.GroupLayout _maxTemporalOptionsGroupLayout = new javax.swing.GroupLayout(_maxTemporalOptionsGroup);
        _maxTemporalOptionsGroup.setLayout(_maxTemporalOptionsGroupLayout);
        _maxTemporalOptionsGroupLayout.setHorizontalGroup(
            _maxTemporalOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_maxTemporalOptionsGroupLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_maxTemporalOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(_maxTemporalOptionsGroupLayout.createSequentialGroup()
                        .addComponent(_percentageTemporalRadioButton)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_maxTemporalClusterSizeTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 45, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_percentageOfStudyPeriodLabel))
                    .addGroup(_maxTemporalOptionsGroupLayout.createSequentialGroup()
                        .addComponent(_timeTemporalRadioButton)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_maxTemporalClusterSizeUnitsTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 45, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_maxTemporalTimeUnitsLabel)))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );
        _maxTemporalOptionsGroupLayout.setVerticalGroup(
            _maxTemporalOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_maxTemporalOptionsGroupLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_maxTemporalOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_percentageTemporalRadioButton)
                    .addComponent(_maxTemporalClusterSizeTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_percentageOfStudyPeriodLabel))
                .addGap(10, 10, 10)
                .addGroup(_maxTemporalOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_timeTemporalRadioButton)
                    .addComponent(_maxTemporalClusterSizeUnitsTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_maxTemporalTimeUnitsLabel))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        _minTemporalOptionsGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Minimum Temporal Size"));

        _minTemporalClusterSizeUnitsTextField.setText("1"); // NOI18N
        _minTemporalClusterSizeUnitsTextField.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validatePostiveNumericKeyTyped(_minTemporalClusterSizeUnitsTextField, e, 6);
            }
        });
        _minTemporalClusterSizeUnitsTextField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                while (_minTemporalClusterSizeUnitsTextField.getText().length() == 0 || Integer.parseInt(_minTemporalClusterSizeUnitsTextField.getText()) == 0) {
                    if (undo.canUndo()) undo.undo(); else _minTemporalClusterSizeUnitsTextField.setText("1");
                }
                enableSetDefaultsButton();
            }
        });
        _minTemporalClusterSizeUnitsTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });

        _minTemporalTimeUnitsLabel.setText("data time units"); // NOI18N

        javax.swing.GroupLayout _minTemporalOptionsGroupLayout = new javax.swing.GroupLayout(_minTemporalOptionsGroup);
        _minTemporalOptionsGroup.setLayout(_minTemporalOptionsGroupLayout);
        _minTemporalOptionsGroupLayout.setHorizontalGroup(
            _minTemporalOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_minTemporalOptionsGroupLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_minTemporalClusterSizeUnitsTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 45, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_minTemporalTimeUnitsLabel)
                .addContainerGap(440, Short.MAX_VALUE))
        );
        _minTemporalOptionsGroupLayout.setVerticalGroup(
            _minTemporalOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _minTemporalOptionsGroupLayout.createSequentialGroup()
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addGroup(_minTemporalOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_minTemporalClusterSizeUnitsTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_minTemporalTimeUnitsLabel))
                .addContainerGap())
        );

        javax.swing.GroupLayout _advanced_temporal_window_tabLayout = new javax.swing.GroupLayout(_advanced_temporal_window_tab);
        _advanced_temporal_window_tab.setLayout(_advanced_temporal_window_tabLayout);
        _advanced_temporal_window_tabLayout.setHorizontalGroup(
            _advanced_temporal_window_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_advanced_temporal_window_tabLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_advanced_temporal_window_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_maxTemporalOptionsGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_minTemporalOptionsGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        _advanced_temporal_window_tabLayout.setVerticalGroup(
            _advanced_temporal_window_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_advanced_temporal_window_tabLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_maxTemporalOptionsGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_minTemporalOptionsGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(93, Short.MAX_VALUE))
        );

        jTabbedPane1.addTab("Temporal Window", _advanced_temporal_window_tab);

        jPanel1.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Monte Carlo Replications"));

        _labelMonteCarloReplications.setText("Number of replications (0, 9, 999, or value ending in 999):"); // NOI18N

        _montCarloReplicationsTextField.setText("999"); // NOI18N
        _montCarloReplicationsTextField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                while (_montCarloReplicationsTextField.getText().length() == 0)
                if (undo.canUndo()) undo.undo(); else _montCarloReplicationsTextField.setText("999");
            }
        });
        _montCarloReplicationsTextField.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validatePostiveNumericKeyTyped(_montCarloReplicationsTextField, e, 10);
            }
        });
        _montCarloReplicationsTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });

        javax.swing.GroupLayout jPanel1Layout = new javax.swing.GroupLayout(jPanel1);
        jPanel1.setLayout(jPanel1Layout);
        jPanel1Layout.setHorizontalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel1Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_labelMonteCarloReplications)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_montCarloReplicationsTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 58, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(219, Short.MAX_VALUE))
        );
        jPanel1Layout.setVerticalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel1Layout.createSequentialGroup()
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_labelMonteCarloReplications)
                    .addComponent(_montCarloReplicationsTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        javax.swing.GroupLayout _advanced_inferenece_tabLayout = new javax.swing.GroupLayout(_advanced_inferenece_tab);
        _advanced_inferenece_tab.setLayout(_advanced_inferenece_tabLayout);
        _advanced_inferenece_tabLayout.setHorizontalGroup(
            _advanced_inferenece_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_advanced_inferenece_tabLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jPanel1, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addContainerGap())
        );
        _advanced_inferenece_tabLayout.setVerticalGroup(
            _advanced_inferenece_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_advanced_inferenece_tabLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jPanel1, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(208, Short.MAX_VALUE))
        );

        jTabbedPane1.addTab("Inference", _advanced_inferenece_tab);

        _powerEvaluationsGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Statistical Power Evaluation"));

        _performPowerEvalautions.setText("Perform Power Evaluations");
        _performPowerEvalautions.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                enablePowerEvaluationsGroup();
                enableSetDefaultsButton();
            }
        });

        _powerEstimationButtonGroup.add(_partOfRegularAnalysis);
        _partOfRegularAnalysis.setSelected(true);
        _partOfRegularAnalysis.setText("As Part of Regular Analysis");
        _partOfRegularAnalysis.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                enablePowerEvaluationsGroup();
                enableSetDefaultsButton();
            }
        });

        _powerEstimationButtonGroup.add(_powerEvaluationWithCaseFile);
        _powerEvaluationWithCaseFile.setText("Power Evaluation Only, Use Total Cases From Case File");
        _powerEvaluationWithCaseFile.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                enablePowerEvaluationsGroup();
                enableSetDefaultsButton();
            }
        });

        _powerEstimationButtonGroup.add(_powerEvaluationWithSpecifiedCases);
        _powerEvaluationWithSpecifiedCases.setText("Power Evaluation Only, Use ");
        _powerEvaluationWithSpecifiedCases.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                enablePowerEvaluationsGroup();
                enableSetDefaultsButton();
            }
        });

        _totalPowerCases.setText("600");
        _totalPowerCases.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                while (_totalPowerCases.getText().length() == 0)
                if (undo.canUndo()) undo.undo(); else _totalPowerCases.setText("600");
                enableSetDefaultsButton();
            }
        });
        _totalPowerCases.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validatePostiveNumericKeyTyped(_totalPowerCases, e, 10);
            }
        });
        _totalPowerCases.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });

        _powerEvaluationWithSpecifiedCasesLabel.setText("Total Cases");

        _numberPowerReplicationsLabel.setText("Number of replications (100, 1000 or multiple of 100):"); // NOI18N

        _numberPowerReplications.setText("1000"); // NOI18N
        _numberPowerReplications.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                while (_numberPowerReplications.getText().length() == 0)
                if (undo.canUndo()) undo.undo(); else _numberPowerReplications.setText("1000");
                enableSetDefaultsButton();
            }
        });
        _numberPowerReplications.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validatePostiveNumericKeyTyped(_numberPowerReplications, e, 10);
            }
        });
        _numberPowerReplications.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });

        _alternativeHypothesisFilenameLabel.setText("Alternative Hypothesis File:"); // NOI18N

        _alternativeHypothesisFilename.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                enableSetDefaultsButton();
            }
        });

        _alternativeHypothesisFilenameButton.setText("..."); // NOI18N
        _alternativeHypothesisFilenameButton.setToolTipText("Open file wizard for alternative hypothesis file ..."); // NOI18N
        _alternativeHypothesisFilenameButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                InputFileFilter[] filters = new InputFileFilter[]{new InputFileFilter("csv","CSV Files (*.csv)"), new InputFileFilter("txt","Text Files (*.txt)"), new InputFileFilter("ha","Alternative Hypothesis Files (*.ha)")};
                FileSelectionDialog select = new FileSelectionDialog(org.treescan.gui.TreeScanApplication.getInstance(), "Select Alternative Hypothesis File", filters, org.treescan.gui.TreeScanApplication.getInstance().lastBrowseDirectory);
                File file = select.browse_load(true);
                if (file != null) {
                    org.treescan.gui.TreeScanApplication.getInstance().lastBrowseDirectory = select.getDirectory();
                    _alternativeHypothesisFilename.setText(file.getAbsolutePath());
                }
            }
        });

        javax.swing.GroupLayout _powerEvaluationsGroupLayout = new javax.swing.GroupLayout(_powerEvaluationsGroup);
        _powerEvaluationsGroup.setLayout(_powerEvaluationsGroupLayout);
        _powerEvaluationsGroupLayout.setHorizontalGroup(
            _powerEvaluationsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_powerEvaluationsGroupLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_powerEvaluationsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_performPowerEvalautions, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_partOfRegularAnalysis, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_powerEvaluationWithCaseFile, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addGroup(_powerEvaluationsGroupLayout.createSequentialGroup()
                        .addComponent(_powerEvaluationWithSpecifiedCases)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_totalPowerCases, javax.swing.GroupLayout.PREFERRED_SIZE, 78, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_powerEvaluationWithSpecifiedCasesLabel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                    .addComponent(_alternativeHypothesisFilenameLabel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addGroup(_powerEvaluationsGroupLayout.createSequentialGroup()
                        .addComponent(_numberPowerReplicationsLabel)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_numberPowerReplications, javax.swing.GroupLayout.PREFERRED_SIZE, 58, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addGap(0, 231, Short.MAX_VALUE))
                    .addGroup(_powerEvaluationsGroupLayout.createSequentialGroup()
                        .addComponent(_alternativeHypothesisFilename)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_alternativeHypothesisFilenameButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE)))
                .addContainerGap())
        );
        _powerEvaluationsGroupLayout.setVerticalGroup(
            _powerEvaluationsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_powerEvaluationsGroupLayout.createSequentialGroup()
                .addComponent(_performPowerEvalautions)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_partOfRegularAnalysis)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_powerEvaluationWithCaseFile)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_powerEvaluationsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_powerEvaluationWithSpecifiedCases)
                    .addComponent(_totalPowerCases, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_powerEvaluationWithSpecifiedCasesLabel))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addGroup(_powerEvaluationsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_numberPowerReplicationsLabel)
                    .addComponent(_numberPowerReplications, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(_alternativeHypothesisFilenameLabel)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_powerEvaluationsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_alternativeHypothesisFilename, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_alternativeHypothesisFilenameButton))
                .addGap(0, 52, Short.MAX_VALUE))
        );

        javax.swing.GroupLayout _advanced_power_evaluation_tabLayout = new javax.swing.GroupLayout(_advanced_power_evaluation_tab);
        _advanced_power_evaluation_tab.setLayout(_advanced_power_evaluation_tabLayout);
        _advanced_power_evaluation_tabLayout.setHorizontalGroup(
            _advanced_power_evaluation_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_advanced_power_evaluation_tabLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_powerEvaluationsGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addContainerGap())
        );
        _advanced_power_evaluation_tabLayout.setVerticalGroup(
            _advanced_power_evaluation_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_advanced_power_evaluation_tabLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_powerEvaluationsGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addContainerGap())
        );

        jTabbedPane1.addTab("Power Evaluation", _advanced_power_evaluation_tab);

        _log_likelihood_ratios_group.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Log Likelihood Ratios"));

        _reportLLRResultsAsCsvTable.setText("Report Simulated Log Likelihood Ratios");
        _reportLLRResultsAsCsvTable.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                enableSetDefaultsButton();
            }
        });

        javax.swing.GroupLayout _log_likelihood_ratios_groupLayout = new javax.swing.GroupLayout(_log_likelihood_ratios_group);
        _log_likelihood_ratios_group.setLayout(_log_likelihood_ratios_groupLayout);
        _log_likelihood_ratios_groupLayout.setHorizontalGroup(
            _log_likelihood_ratios_groupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_log_likelihood_ratios_groupLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_reportLLRResultsAsCsvTable, javax.swing.GroupLayout.DEFAULT_SIZE, 558, Short.MAX_VALUE)
                .addContainerGap())
        );
        _log_likelihood_ratios_groupLayout.setVerticalGroup(
            _log_likelihood_ratios_groupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_log_likelihood_ratios_groupLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_reportLLRResultsAsCsvTable)
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        _report_critical_values_group.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Critical Values"));

        _reportCriticalValuesCheckBox.setText("Report critical values for an observed cut to be significant"); // NOI18N
        _reportCriticalValuesCheckBox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _reportCriticalValuesCheckBox.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _reportCriticalValuesCheckBox.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                enableSetDefaultsButton();
            }
        });

        javax.swing.GroupLayout _report_critical_values_groupLayout = new javax.swing.GroupLayout(_report_critical_values_group);
        _report_critical_values_group.setLayout(_report_critical_values_groupLayout);
        _report_critical_values_groupLayout.setHorizontalGroup(
            _report_critical_values_groupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_report_critical_values_groupLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_reportCriticalValuesCheckBox, javax.swing.GroupLayout.DEFAULT_SIZE, 550, Short.MAX_VALUE)
                .addContainerGap())
        );
        _report_critical_values_groupLayout.setVerticalGroup(
            _report_critical_values_groupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_report_critical_values_groupLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_reportCriticalValuesCheckBox)
                .addContainerGap(14, Short.MAX_VALUE))
        );

        javax.swing.GroupLayout _advanced_output_tabLayout = new javax.swing.GroupLayout(_advanced_output_tab);
        _advanced_output_tab.setLayout(_advanced_output_tabLayout);
        _advanced_output_tabLayout.setHorizontalGroup(
            _advanced_output_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_advanced_output_tabLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_advanced_output_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_log_likelihood_ratios_group, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_report_critical_values_group, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        _advanced_output_tabLayout.setVerticalGroup(
            _advanced_output_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_advanced_output_tabLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_log_likelihood_ratios_group, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_report_critical_values_group, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(130, Short.MAX_VALUE))
        );

        jTabbedPane1.addTab("Additional Output", _advanced_output_tab);

        _perform_dayofweek_adjustments.setText("Perform Day of Week Adjustments");
        _perform_dayofweek_adjustments.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                enableSetDefaultsButton();
            }
        });

        javax.swing.GroupLayout _advanced_adjustments_tabLayout = new javax.swing.GroupLayout(_advanced_adjustments_tab);
        _advanced_adjustments_tab.setLayout(_advanced_adjustments_tabLayout);
        _advanced_adjustments_tabLayout.setHorizontalGroup(
            _advanced_adjustments_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_advanced_adjustments_tabLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_perform_dayofweek_adjustments, javax.swing.GroupLayout.DEFAULT_SIZE, 590, Short.MAX_VALUE)
                .addContainerGap())
        );
        _advanced_adjustments_tabLayout.setVerticalGroup(
            _advanced_adjustments_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_advanced_adjustments_tabLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_perform_dayofweek_adjustments)
                .addContainerGap(246, Short.MAX_VALUE))
        );

        jTabbedPane1.addTab("Adjustments", _advanced_adjustments_tab);

        _closeButton.setText("Close"); // NOI18N
        _closeButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                setVisible(false, null);
            }
        });

        _setDefaultButton.setText("Set Defaults"); // NOI18N
        _setDefaultButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                setDefaultsClick();
            }
        });

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, layout.createSequentialGroup()
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addComponent(_setDefaultButton, javax.swing.GroupLayout.PREFERRED_SIZE, 120, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_closeButton, javax.swing.GroupLayout.PREFERRED_SIZE, 105, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap())
            .addComponent(jTabbedPane1)
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, layout.createSequentialGroup()
                .addComponent(jTabbedPane1)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_setDefaultButton)
                    .addComponent(_closeButton))
                .addContainerGap())
        );

        jTabbedPane1.getAccessibleContext().setAccessibleName("Cuts");

        pack();
    }// </editor-fold>//GEN-END:initComponents
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JPanel _advanced_adjustments_tab;
    private javax.swing.JPanel _advanced_inferenece_tab;
    private javax.swing.JPanel _advanced_input_tab;
    private javax.swing.JPanel _advanced_output_tab;
    private javax.swing.JPanel _advanced_power_evaluation_tab;
    private javax.swing.JPanel _advanced_temporal_window_tab;
    private javax.swing.JTextField _alternativeHypothesisFilename;
    private javax.swing.JButton _alternativeHypothesisFilenameButton;
    private javax.swing.JLabel _alternativeHypothesisFilenameLabel;
    private javax.swing.JButton _closeButton;
    private javax.swing.JButton _cutFileBrowseButton;
    private javax.swing.JButton _cutFileImportButton;
    private javax.swing.JLabel _cutFileLabel;
    public javax.swing.JTextField _cutFileTextField;
    private javax.swing.JLabel _labelMonteCarloReplications;
    private javax.swing.JPanel _log_likelihood_ratios_group;
    private javax.swing.JTextField _maxTemporalClusterSizeTextField;
    private javax.swing.JTextField _maxTemporalClusterSizeUnitsTextField;
    private javax.swing.JPanel _maxTemporalOptionsGroup;
    private javax.swing.JLabel _maxTemporalTimeUnitsLabel;
    private javax.swing.JTextField _minTemporalClusterSizeUnitsTextField;
    private javax.swing.JPanel _minTemporalOptionsGroup;
    private javax.swing.JLabel _minTemporalTimeUnitsLabel;
    private javax.swing.JTextField _montCarloReplicationsTextField;
    private javax.swing.JTextField _numberPowerReplications;
    private javax.swing.JLabel _numberPowerReplicationsLabel;
    private javax.swing.JRadioButton _partOfRegularAnalysis;
    private javax.swing.JLabel _percentageOfStudyPeriodLabel;
    private javax.swing.JRadioButton _percentageTemporalRadioButton;
    private javax.swing.JCheckBox _performPowerEvalautions;
    private javax.swing.JCheckBox _perform_dayofweek_adjustments;
    private javax.swing.ButtonGroup _powerEstimationButtonGroup;
    private javax.swing.JRadioButton _powerEvaluationWithCaseFile;
    private javax.swing.JRadioButton _powerEvaluationWithSpecifiedCases;
    private javax.swing.JLabel _powerEvaluationWithSpecifiedCasesLabel;
    private javax.swing.JPanel _powerEvaluationsGroup;
    private javax.swing.JCheckBox _reportCriticalValuesCheckBox;
    private javax.swing.JCheckBox _reportLLRResultsAsCsvTable;
    private javax.swing.JPanel _report_critical_values_group;
    private javax.swing.JButton _setDefaultButton;
    private javax.swing.JRadioButton _timeTemporalRadioButton;
    private javax.swing.JTextField _totalPowerCases;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JTabbedPane jTabbedPane1;
    private javax.swing.ButtonGroup maximumWindowButtonGroup;
    // End of variables declaration//GEN-END:variables
}
