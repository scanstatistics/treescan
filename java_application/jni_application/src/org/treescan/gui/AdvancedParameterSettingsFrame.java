package org.treescan.gui;

import java.awt.CardLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.Container;
import java.awt.Font;
import java.io.File;
import java.util.ArrayList;
import java.util.List;
import javax.swing.ImageIcon;
import javax.swing.JPanel;
import javax.swing.JRootPane;
import javax.swing.event.MouseInputAdapter;
import javax.swing.event.UndoableEditEvent;
import javax.swing.event.UndoableEditListener;
import javax.swing.undo.UndoManager;
import org.treescan.app.AdvFeaturesExpection;
import org.treescan.app.Parameters;
import org.treescan.app.UnknownEnumException;
import org.treescan.gui.utils.FileSelectionDialog;
import org.treescan.gui.utils.InputFileFilter;
import org.treescan.gui.utils.TextPrompt;
import org.treescan.gui.utils.Utils;
import org.treescan.importer.InputSourceSettings;
import org.treescan.utils.FileAccess;

public class AdvancedParameterSettingsFrame extends javax.swing.JInternalFrame {
    public enum FocusedTabSet { INPUT, ANALYSIS, OUTPUT };
    private JPanel _glass = null;
    private final JRootPane _rootPane;
    private final Component _rootPaneInitialGlass;
    private final UndoManager undo = new UndoManager();
    private final ParameterSettingsFrame _settings_window;
    private FocusedTabSet _focusedTabSet = FocusedTabSet.INPUT;
    private final String _sequential_timeonly_cardname = "sequential-timeonly";
    private final String _sequential_treeonly_cardname = "sequential-treeonly";

    /**
     * Creates new form ParameterSettingsFrame
     */
    public AdvancedParameterSettingsFrame(final JRootPane rootPane, final ParameterSettingsFrame analysisSettingsWindow/*, final Parameters parameters*/) {
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
        //setupInterface(parameters);
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
        // Inference tab
        bReturn &= (Integer.parseInt(_montCarloReplicationsTextField.getText()) == 999);
        bReturn &= (_restrict_evaluated_levels.isSelected() == false);
        bReturn &= _restricted_levels.getText().equals("");
        // Temporal Window tab
        bReturn &= _percentageTemporalRadioButton.isSelected();
        bReturn &= (Double.parseDouble(_maxTemporalClusterSizeTextField.getText()) == 50.0);
        bReturn &= (Integer.parseInt(_maxTemporalClusterSizeUnitsTextField.getText()) == 1);
        bReturn &= (Integer.parseInt(_minTemporalClusterSizeUnitsTextField.getText()) == 2);
        bReturn &= (_apply_risk_window_restriction.isSelected() == true);
        bReturn &= (Double.parseDouble(_risk_window_percentage.getText()) == 20.0);
        // Sequential Scan tab
        bReturn &= (_perform_sequential_scan.isSelected() == false);
        bReturn &= _maximum_cases_signal.getText().equals("200");
        bReturn &= _minimum_cases_signal.getText().equals("3");
        bReturn &= _sequential_analysis_file.getText().equals("");

        bReturn &= _sequentual_alpha_overall.getText().equals("0.05");
        bReturn &= _sequential_alpha_spending.getText().equals("0.01");

        // Power Evaluations tab
        bReturn &= (_performPowerEvaluations.isSelected() == false);
        bReturn &= (_partOfRegularAnalysis.isSelected() == true);
        bReturn &= _totalPowerCases.getText().equals("600");
        bReturn &= _alternativeHypothesisFilename.getText().equals("");
        bReturn &= _numberPowerReplications.getText().equals("1000");
        // Adjustments tab
        bReturn &= _perform_dayofweek_adjustments.isSelected() == false;
        bReturn &= _apply_time_range_restrictions.isSelected() == false;
        bReturn &= _time_range_restrictions.getText().equals("");
        return bReturn;
    }

    public boolean getDefaultsSetForOutputOptions() {
        boolean bReturn = true;
        bReturn &= _reportLLRResultsAsCsvTable.isSelected() == false;
        bReturn &= _reportCriticalValuesCheckBox.isSelected() == false;
        bReturn &= _chk_rpt_attributable_risk.isSelected() == false;
        bReturn &= _attributable_risk_exposed.getText().equals("");
        return bReturn;
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
        // Need this should sequential scan accumulated spent alpha have been updated.
        enableSequentialAnalysisGroup();
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
                jTabbedPane1.addTab("Sequential", null, _sequential_analysis_tab, null);
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
        parameters.setApplyingRiskWindowRestriction(_apply_risk_window_restriction.isEnabled() && _apply_risk_window_restriction.isSelected());
        parameters.setRiskWindowPercentage(Double.parseDouble(_risk_window_percentage.getText()));

        // Inference tab
        parameters.setNumReplications(Integer.parseInt(_montCarloReplicationsTextField.getText()));
        parameters.setRestrictTreeLevels(_restrict_evaluated_levels.isSelected());
        parameters.setRestrictedTreeLevels(_restricted_levels.getText());

        // Temporal Window tab
        parameters.setMaximumWindowPercentage(Double.parseDouble(_maxTemporalClusterSizeTextField.getText()));
        parameters.setMaximumWindowLength(Integer.parseInt(_maxTemporalClusterSizeUnitsTextField.getText()));
        parameters.setMaximumWindowType(_percentageTemporalRadioButton.isSelected() ? Parameters.MaximumWindowType.PERCENTAGE_WINDOW : Parameters.MaximumWindowType.FIXED_LENGTH);
        parameters.setMinimumWindowLength(Integer.parseInt(_minTemporalClusterSizeUnitsTextField.getText()));

        // Adjustments tab
        parameters.setPerformDayOfWeekAdjustment(_perform_dayofweek_adjustments.isEnabled() && _perform_dayofweek_adjustments.isSelected());
        parameters.setApplyingExclusionTimeRanges(_apply_time_range_restrictions.isEnabled() && _apply_time_range_restrictions.isSelected());
        parameters.setExclusionTimeRangeSet(_time_range_restrictions.getText());

        // Power Evaluations tab
        parameters.setPerformPowerEvaluations(_powerEvaluationsGroup.isEnabled() && _performPowerEvaluations.isSelected());
        parameters.setPowerEvaluationType(getPowerEvaluationMethodType().ordinal());
        parameters.setPowerEvaluationTotalCases(Integer.parseInt((_totalPowerCases.getText().length() > 0 ? _totalPowerCases.getText() : "600")));
        parameters.setPowerEvaluationReplications(Integer.parseInt(_numberPowerReplications.getText()));
        parameters.setPowerEvaluationAltHypothesisFilename(_alternativeHypothesisFilename.getText());
        parameters.setPowerBaselineProbabilityRatioNumerator(Integer.parseInt(_eventProbabiltyNumerator.getText()));
        parameters.setPowerBaselineProbabilityRatioDenominator(Integer.parseInt(this._eventProbabiltyDenominator.getText()));

        // Sequential Analysis tab
        parameters.setSequentialScan(_sequential_analysis_group.isEnabled() && _perform_sequential_scan.isSelected());
        parameters.setSequentialAlphaOverall(Double.parseDouble(_sequentual_alpha_overall.getText()));
        parameters.setSequentialAlphaSpending(Double.parseDouble(_sequential_alpha_spending.getText()));
        /* Not exposed in gui: https://www.squishlist.com/ims/treescan/62/
        parameters.setSequentialScan(_sequential_analysis_group.isEnabled() && _perform_sequential_scan.isSelected());
        parameters.setSequentialMaximumSignal(Integer.parseInt(_maximum_cases_signal.getText()));
        parameters.setSequentialMinimumSignal(Integer.parseInt(_minimum_cases_signal.getText()));
        parameters.setSequentialFilename(_sequential_analysis_file.getText());
        */

        // Additional Output tab
        parameters.setGeneratingLLRResults(_reportLLRResultsAsCsvTable.isSelected());
        parameters.setReportCriticalValues(_reportCriticalValuesCheckBox.isSelected());
        parameters.setReportAttributableRisk(_chk_rpt_attributable_risk.isEnabled() && _chk_rpt_attributable_risk.isSelected());
        parameters.setAttributableRiskExposed(_attributable_risk_exposed.getText().length() > 0 ? Integer.parseInt(_attributable_risk_exposed.getText()): 0);
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
        // Inference tab
        _montCarloReplicationsTextField.setText("999");
        _restrict_evaluated_levels.setSelected(false);
        _restricted_levels.setText("");
        // Temporal Window tab
        _percentageTemporalRadioButton.setSelected(true);
        _maxTemporalClusterSizeTextField.setText("50");
        _maxTemporalClusterSizeUnitsTextField.setText("1");
        _minTemporalClusterSizeUnitsTextField.setText("2");
        _apply_risk_window_restriction.setSelected(true);
        _risk_window_percentage.setText("20");
        // Adjustments tab
        _perform_dayofweek_adjustments.setSelected(false);
        _apply_time_range_restrictions.setSelected(false);
        _time_range_restrictions.setText("");
        // Sequential Scan tab
        _perform_sequential_scan.setSelected(false);
        _maximum_cases_signal.setText("200");
        _minimum_cases_signal.setText("3");
        _sequential_analysis_file.setText("");
        _sequentual_alpha_overall.setText("0.05");
        _sequential_alpha_spending.setText("0.01");
        // Power Evaluations tab
        _performPowerEvaluations.setSelected(false);
        _partOfRegularAnalysis.setSelected(true);
        _totalPowerCases.setText("600");
        _alternativeHypothesisFilename.setText("");
        _numberPowerReplications.setText("1000");
    }

    public void setupInterface(final Parameters parameters) {
        // Advanced Input tab
        _cutFileTextField.setText(parameters.getCutsFileName());
        _cutFileTextField.setCaretPosition(0);
        _apply_risk_window_restriction.setSelected(parameters.isApplyingRiskWindowRestriction());
        _risk_window_percentage.setText(Double.toString(parameters.getRiskWindowPercentage()));

        // Inference tab
        _montCarloReplicationsTextField.setText(Integer.toString(parameters.getNumReplicationsRequested()));
        _restrict_evaluated_levels.setSelected(parameters.getRestrictTreeLevels());
        _restricted_levels.setText(parameters.getRestrictedTreeLevels());

        // Temporal Window tab
        _percentageTemporalRadioButton.setSelected(parameters.getMaximumWindowType() == Parameters.MaximumWindowType.PERCENTAGE_WINDOW);
        _timeTemporalRadioButton.setSelected(parameters.getMaximumWindowType() == Parameters.MaximumWindowType.FIXED_LENGTH);
        _maxTemporalClusterSizeTextField.setText(Double.toString(parameters.getMaximumWindowPercentage()));
        _maxTemporalClusterSizeUnitsTextField.setText(Integer.toString(parameters.getMaximumWindowLength()));
        _minTemporalClusterSizeUnitsTextField.setText(Integer.toString(parameters.getMinimumWindowLength()));

        // Adjustments tab
        _perform_dayofweek_adjustments.setSelected(parameters.getPerformDayOfWeekAdjustment());
        _apply_time_range_restrictions.setSelected(parameters.isApplyingExclusionTimeRanges());
        _time_range_restrictions.setText(parameters.getExclusionTimeRangeSet());

        // Power Evaluations tab
        _performPowerEvaluations.setSelected(parameters.getPerformPowerEvaluations());
        _partOfRegularAnalysis.setSelected(parameters.getPowerEvaluationType() == Parameters.PowerEvaluationType.PE_ONLY_CASEFILE);
        _powerEvaluationWithCaseFile.setSelected(parameters.getPowerEvaluationType() == Parameters.PowerEvaluationType.PE_ONLY_CASEFILE);
        _powerEvaluationWithSpecifiedCases.setSelected(parameters.getPowerEvaluationType() == Parameters.PowerEvaluationType.PE_ONLY_SPECIFIED_CASES);
        _totalPowerCases.setText(Integer.toString(parameters.getPowerEvaluationTotalCases()));
        _numberPowerReplications.setText(Integer.toString(parameters.getPowerEvaluationReplications()));
        _alternativeHypothesisFilename.setText(parameters.getPowerEvaluationAltHypothesisFilename());
        _eventProbabiltyNumerator.setText(Integer.toString(parameters.getPowerBaselineProbabilityRatioNumerator()));
        _eventProbabiltyDenominator.setText(Integer.toString(parameters.getPowerBaselineProbabilityRatioDenominator()));

        // Seqential Analysis tab
        _perform_sequential_scan.setSelected(parameters.getSequentialScan());
        _sequentual_alpha_overall.setText(Double.toString(parameters.getSequentialAlphaOverall()));
        _sequential_alpha_spending.setText(Double.toString(parameters.getSequentialAlphaSpending()));
        /* Time only not exposed in gui: https://www.squishlist.com/ims/treescan/62/
        _perform_sequential_scan.setSelected(parameters.getSequentialScan());
        _maximum_cases_signal.setText(Integer.toString(parameters.getSequentialMaximumSignal()));
        _minimum_cases_signal.setText(Integer.toString(parameters.getSequentialMinimumSignal()));
        _sequential_analysis_file.setText(parameters.getSequentialFilename());
        */

        // Additional Output tab
        _reportCriticalValuesCheckBox.setSelected(parameters.getReportCriticalValues());
        _reportLLRResultsAsCsvTable.setSelected(parameters.isGeneratingLLRResults());
        _chk_rpt_attributable_risk.setSelected(parameters.getReportAttributableRisk());
        _attributable_risk_exposed.setText(parameters.getAttributableRiskExposed() > 0 ? Integer.toString(parameters.getAttributableRiskExposed()) : "");

        enablePowerEvaluationsGroup();
        enableSequentialAnalysisGroup();
    }

    /**
     * Sets default values for Output related tab and respective controls pulled
     * these default values from the CParameter class
     */
    private void setDefaultsForOutputTab() {
        _reportLLRResultsAsCsvTable.setSelected(false);
        _reportCriticalValuesCheckBox.setSelected(false);
        _chk_rpt_attributable_risk.setSelected(false);
        _attributable_risk_exposed.setText("");
    }

    /*
     * Verifies that settings are valid in the context of all other parameter settings.
     */
    public void CheckSettings() {
        CheckInputSettings();
        CheckInferenceSettings();
        CheckTemporalWindowSize();
        CheckAdjustmentSettings();
        CheckSequentialAnalysisSettings();
        CheckPowerEvaluationSettings();
        CheckAdditionalOutputOptions();
    }

    /*
     * Verifies that sequential scan settings are valid in the context of all parameter settings.
     */
    private void CheckSequentialAnalysisSettings() {
        if (_perform_sequential_scan.isEnabled() && _perform_sequential_scan.isSelected()) {
            Parameters.ScanType scanType = _settings_window.getScanType();
            Parameters.ModelType modelType = _settings_window.getModelType();
            Parameters.ConditionalType conditionType = _settings_window.getConditionalType();

            if (scanType == Parameters.ScanType.TIMEONLY) {
                int minimum_cases_to_signal = Integer.parseInt(_minimum_cases_signal.getText());
                if (minimum_cases_to_signal < 3) {
                    throw new AdvFeaturesExpection("The minimum number of cases to signal must be 3 or greater.\n", FocusedTabSet.ANALYSIS, (Component) _minimum_cases_signal);
                }
                int maximum_cases_to_signal = Integer.parseInt(_maximum_cases_signal.getText());
                if (minimum_cases_to_signal > maximum_cases_to_signal) {
                    throw new AdvFeaturesExpection("The minimum number of cases to signal must be than the maximum to signal.\n", FocusedTabSet.ANALYSIS, (Component) _minimum_cases_signal);
                }
                if (_sequential_analysis_file.getText().length() == 0) {
                    throw new AdvFeaturesExpection("Please specify a sequential analysis filename.", FocusedTabSet.ANALYSIS, (Component) _sequential_analysis_file);
                }
            } else {
                double alpha = Double.parseDouble(_sequentual_alpha_overall.getText());
                double alpha_spending = Double.parseDouble(_sequential_alpha_spending.getText());
                if (alpha_spending > alpha) {
                    throw new AdvFeaturesExpection("For sequential scan, alpha spending cannot be greater than alpha.", FocusedTabSet.ANALYSIS, (Component) _sequential_analysis_file);
                }
                double test = 1.0 / (Double.parseDouble(_montCarloReplicationsTextField.getText()) + 1.0);
                if (alpha_spending < test) {
                    throw new AdvFeaturesExpection("For sequential scan, alpha spending cannot be less than " + Double.toString(test) + " with " + _montCarloReplicationsTextField.getText() + " replications.", FocusedTabSet.ANALYSIS, (Component) _sequential_analysis_file);
                }
            }
        }
    }

    /*
     * Verifies that power evaluation settings are valid in the context of all parameter settings.
     */
    private void CheckPowerEvaluationSettings() {
        if (_performPowerEvaluations.isEnabled() && _performPowerEvaluations.isSelected()) {
            if (_powerEvaluationWithSpecifiedCases.isSelected()) {
                if (Integer.parseInt(_totalPowerCases.getText()) < 2) {
                    throw new AdvFeaturesExpection("The number of power evaluation cases must be two or more.\n", FocusedTabSet.ANALYSIS, (Component) _totalPowerCases);
                }
                Parameters.ModelType modelType = _settings_window.getModelType();
                Parameters.ConditionalType conditonalType = _settings_window.getConditionalType();
                if (!((modelType == Parameters.ModelType.POISSON ||
                       modelType == Parameters.ModelType.BERNOULLI_TREE ||
                       _settings_window.getScanType() == Parameters.ScanType.TIMEONLY) && conditonalType == Parameters.ConditionalType.TOTALCASES)) {
                    throw new AdvFeaturesExpection("The power evaluation option to define total cases is only permitted with the conditional Poisson model, Bernoulli model or time-only scan.\n", FocusedTabSet.ANALYSIS, (Component) _totalPowerCases);
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
            if (_settings_window.getModelType() == Parameters.ModelType.BERNOULLI_TREE && _settings_window.getConditionalType() == Parameters.ConditionalType.TOTALCASES) {
                int eventProbNumerator = Integer.parseInt(_eventProbabiltyNumerator.getText().trim());
                int eventProbDenominator = Integer.parseInt(_eventProbabiltyDenominator.getText().trim());
                if (eventProbNumerator == 0 || eventProbDenominator == 0 || eventProbNumerator >= eventProbDenominator) {
                    throw new AdvFeaturesExpection("Please specify an event probabilty that is between zero and one.", FocusedTabSet.ANALYSIS,(Component) _eventProbabiltyNumerator);
                }
            }
            // Power evaluation not implemented for sequential Bernoulli model
            if (_perform_sequential_scan.isEnabled() &&
                _perform_sequential_scan.isSelected() &&
                _settings_window.getScanType() == Parameters.ScanType.TREEONLY &&
                _settings_window.getModelType() == Parameters.ModelType.BERNOULLI_TREE &&
                _settings_window.getConditionalType() == Parameters.ConditionalType.UNCONDITIONAL) {
               throw new AdvFeaturesExpection("The power evaluation is not implemented for the sequential scan with unconditional Bernoulli model.", FocusedTabSet.ANALYSIS,(Component) _performPowerEvaluations);
            }
        }
    }

    /*
     * Verifies that input settings are valid in the context of all parameter settings.
     */
    private void CheckInputSettings() {
        //validate the cuts file
        if (_settings_window.getScanType() !=  Parameters.ScanType.TIMEONLY && _cutFileTextField.getText().length() > 0 && !FileAccess.ValidateFileAccess(_cutFileTextField.getText(), false)) {
            throw new AdvFeaturesExpection("The cuts file could not be opened for reading.\n\nPlease confirm that the path and/or file name are valid and that you have permissions to read from this directory and file.", FocusedTabSet.INPUT, (Component) _cutFileTextField);
        }
    }

    /*
     * Verifies that temporal window settings are valid in the context of all parameter settings.
     */
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

    /*
     * Verifies that additional output settings are valid in the context of all parameter settings.
     */
    private void CheckAdditionalOutputOptions() throws NumberFormatException, AdvFeaturesExpection {
        if (_chk_rpt_attributable_risk.isEnabled() && _chk_rpt_attributable_risk.isSelected()) {
            if (_attributable_risk_exposed.getText().trim().length() == 0) {
                throw new AdvFeaturesExpection("Please specify a number exposed for the attributable risk.", FocusedTabSet.OUTPUT, (Component)_attributable_risk_exposed);
            }
            if (Integer.parseInt(_attributable_risk_exposed.getText().trim()) < 1) {
                throw new AdvFeaturesExpection("The number exposed for the attributable risk must be greater than zero.", FocusedTabSet.OUTPUT, (Component) _attributable_risk_exposed);
            }
        }
    }

    /*
     * Verifies that inference settings are valid in the context of all parameter settings.
     */
    private void CheckInferenceSettings() {
        int dNumReplications;
        if (_montCarloReplicationsTextField.getText().trim().length() == 0) {
            throw new AdvFeaturesExpection("Please specify a number of Monte Carlo replications.\nChoices are: 0, 9, 999, or value ending in 999.", FocusedTabSet.ANALYSIS, (Component)_montCarloReplicationsTextField);
        }
        dNumReplications = Integer.parseInt(_montCarloReplicationsTextField.getText().trim());
        if (!((dNumReplications == 0 || dNumReplications == 9 || dNumReplications == 19 || (dNumReplications + 1) % 1000 == 0))) {
            throw new AdvFeaturesExpection("Invalid number of Monte Carlo replications.\nChoices are: 0, 9, 999, or value ending in 999.", FocusedTabSet.ANALYSIS, (Component) _montCarloReplicationsTextField);
        }
        if (_restrict_evaluated_levels.isEnabled() && _restrict_evaluated_levels.isSelected()) {
            if (_restricted_levels.getText().trim().length() == 0) {
                throw new AdvFeaturesExpection("Please specify a comma separated list of integers that represent node levels.", FocusedTabSet.ANALYSIS, (Component)_restricted_levels);
            }
            List<Integer> list = new ArrayList<Integer>();
            try {
                for (String field : _restricted_levels.getText().split(","))
                    list.add(Integer.parseInt(field.trim()));
            } catch (java.lang.NumberFormatException e) {
                throw new AdvFeaturesExpection("Please specify a comma separated list of integers that represent node levels.", FocusedTabSet.ANALYSIS, (Component)_restricted_levels);
            }
        }
    }

    /** Verifies that adjustment options are valid. */
    private void CheckAdjustmentSettings() {
        if (_apply_time_range_restrictions.isEnabled() && _apply_time_range_restrictions.isSelected()) {
            if (_time_range_restrictions.getText().trim().length() == 0)
                throw new AdvFeaturesExpection("Please specify a semi-colon separated list of ranges (e.g. [2,4];[7,20]).", FocusedTabSet.ANALYSIS, (Component)_time_range_restrictions);
            if (!_time_range_restrictions.getText().trim().replace(" ", "").matches("^\\[-?\\d+,-?\\d+\\](;\\[-?\\d+,-?\\d+\\])*$"))
                throw new AdvFeaturesExpection("Not a valid semi-colon separated list of ranges (e.g. [2,4];[7,20]).", FocusedTabSet.ANALYSIS, (Component)_time_range_restrictions);
        }
    }


    /**
     * enables or disables the advanced inputs group controls.
     */
    public void enableAdvancedInputsSettings(boolean enableCutFile, boolean enableRiskWindowGroup) {
        _cutFileLabel.setEnabled(enableCutFile);
        _cutFileTextField.setEnabled(enableCutFile);
        _cutFileImportButton.setEnabled(enableCutFile);

        _risk_window_group.setEnabled(enableRiskWindowGroup);
        _apply_risk_window_restriction.setEnabled(enableRiskWindowGroup);
        _risk_window_percentage.setEnabled(enableRiskWindowGroup && _apply_risk_window_restriction.isSelected());
        _risk_window_percentage_label.setEnabled(enableRiskWindowGroup);
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
        _perform_dayofweek_adjustments.setEnabled(_settings_window.getScanType() == Parameters.ScanType.TREETIME || _settings_window.getScanType() == Parameters.ScanType.TIMEONLY);
        if (_perform_dayofweek_adjustments.isEnabled()) {
            // switch label based upon condition type
            switch (_settings_window.getConditionalType()) {
                case TOTALCASES:
                case NODE: _perform_dayofweek_adjustments.setText("Perform Day-of-Week Adjustment"); break;
                case NODEANDTIME: _perform_dayofweek_adjustments.setText("Perform Node by Day-of-Week Adjustment"); break;
                default: throw new UnknownEnumException(_settings_window.getConditionalType());
            }
        }
    }

    public void enableTimeRangeExclusionsGroup() {
        _group_exclusions.setEnabled(_settings_window.getScanType() == Parameters.ScanType.TREETIME && _settings_window.getConditionalType() == Parameters.ConditionalType.NODEANDTIME);
        _apply_time_range_restrictions.setEnabled(_group_exclusions.isEnabled());
        _time_range_restrictions.setEnabled(_apply_time_range_restrictions.isEnabled() && _apply_time_range_restrictions.isSelected());
    }

    /** enables options of the Additional Output tab */
    public void enableAdditionalOutputOptions() {
        _chk_rpt_attributable_risk.setEnabled(true);
        _attributable_risk_exposed.setEnabled(_chk_rpt_attributable_risk.isEnabled() && _chk_rpt_attributable_risk.isSelected());
        _chk_attributable_risk_extra.setEnabled(_chk_rpt_attributable_risk.isEnabled());
    }

    /**
     * Enabled the power evaluations group based upon current settings.
     */
    public void enablePowerEvaluationsGroup() {
        Parameters.ScanType scanType = _settings_window.getScanType();
        Parameters.ModelType eModelType = _settings_window.getModelType();
        Parameters.ConditionalType eConditionType = _settings_window.getConditionalType();

        boolean bEnableGroup = (scanType == Parameters.ScanType.TREEONLY && (eModelType == Parameters.ModelType.POISSON || eModelType == Parameters.ModelType.BERNOULLI_TREE)) ||
                               (((scanType == Parameters.ScanType.TIMEONLY && eConditionType == Parameters.ConditionalType.TOTALCASES) ||
                                (scanType == Parameters.ScanType.TREETIME && eConditionType == Parameters.ConditionalType.NODE)) &&
                                _perform_dayofweek_adjustments.isSelected() == false
                               );
        _powerEvaluationsGroup.setEnabled(bEnableGroup);
        _performPowerEvaluations.setEnabled(bEnableGroup);
        _partOfRegularAnalysis.setEnabled(bEnableGroup && _performPowerEvaluations.isSelected());
        _powerEvaluationWithCaseFile.setEnabled(bEnableGroup && _performPowerEvaluations.isSelected());
        switch (_settings_window.getConditionalType()) {
            case NODE:
            case TOTALCASES: _powerEvaluationWithCaseFile.setText("Power Evaluation Only, Use Total Cases From Case File"); break;
            case UNCONDITIONAL: _powerEvaluationWithCaseFile.setText("Power Evaluation Only"); break;
            default:
        }
        _powerEvaluationWithSpecifiedCases.setEnabled(bEnableGroup && _performPowerEvaluations.isSelected() &&
                                                      (eModelType == Parameters.ModelType.POISSON || eModelType == Parameters.ModelType.BERNOULLI_TREE || scanType == Parameters.ScanType.TIMEONLY) &&
                                                      _settings_window.getConditionalType() == Parameters.ConditionalType.TOTALCASES);
        if (_powerEvaluationsGroup.isEnabled() && _powerEvaluationWithSpecifiedCases.isSelected() && !_powerEvaluationWithSpecifiedCases.isEnabled())
            _powerEvaluationWithCaseFile.setSelected(true);
        _totalPowerCases.setEnabled(bEnableGroup && _performPowerEvaluations.isSelected() && _powerEvaluationWithSpecifiedCases.isSelected());
        _powerEvaluationWithSpecifiedCasesLabel.setEnabled(bEnableGroup && _performPowerEvaluations.isSelected() && _powerEvaluationWithSpecifiedCases.isEnabled());
        _alternativeHypothesisFilenameLabel.setEnabled(bEnableGroup && _performPowerEvaluations.isSelected());
        _alternativeHypothesisFilename.setEnabled(bEnableGroup && _performPowerEvaluations.isSelected());
        _alternativeHypothesisFilenameButton.setEnabled(bEnableGroup && _performPowerEvaluations.isSelected());
        _numberPowerReplicationsLabel.setEnabled(bEnableGroup && _performPowerEvaluations.isSelected());
        _numberPowerReplications.setEnabled(bEnableGroup && _performPowerEvaluations.isSelected());

        boolean enableEvent = eModelType == Parameters.ModelType.BERNOULLI_TREE && _settings_window.getConditionalType() == Parameters.ConditionalType.TOTALCASES;
        _eventProbabilityLabel.setEnabled(_performPowerEvaluations.isSelected() && enableEvent);
        _eventProbabiltyNumerator.setEnabled(_performPowerEvaluations.isSelected() && enableEvent);
        _eventProbabilityLabel2.setEnabled(_performPowerEvaluations.isSelected() && enableEvent);
        _eventProbabiltyDenominator.setEnabled(_performPowerEvaluations.isSelected() && enableEvent);
    }

    /**
     * Enabled the restricted levels group based upon current settings.
     */
    public void enableRestrictedLevelsGroup() {
        boolean bEnableGroup = _settings_window.getScanType() != Parameters.ScanType.TIMEONLY;
        _restrict_evaluated_levels.setEnabled(bEnableGroup);
        _restricted_levels.setEnabled(bEnableGroup && _restrict_evaluated_levels.isSelected());
    }

    /**
     * Enabled the sequential analysis group based upon current settings.
     */
    public void enableSequentialAnalysisGroup() {
        Parameters.ScanType scanType = _settings_window.getScanType();
        Parameters.ModelType modelType = _settings_window.getModelType();
        Parameters.ConditionalType conditionType = _settings_window.getConditionalType();

        /* Time only not exposed in gui: https://www.squishlist.com/ims/treescan/62/ */

        //boolean bEnableGroup = true; scanType == Parameters.ScanType.TIMEONLY;
        boolean bEnableGroup = scanType == Parameters.ScanType.TREEONLY &&
                               modelType == Parameters.ModelType.BERNOULLI_TREE &&
                               conditionType == Parameters.ConditionalType.UNCONDITIONAL &&
                               !_settings_window.getSelfControlDesign();
        _sequential_analysis_group.setEnabled(bEnableGroup);
        _perform_sequential_scan.setEnabled(bEnableGroup);

        ((CardLayout) _panel_sequential_analysis.getLayout()).show(_panel_sequential_analysis, _sequential_treeonly_cardname);
        _panel_sequential_analysis_tree_only.setEnabled(bEnableGroup && _perform_sequential_scan.isSelected());
        _sequential_alpha_label.setEnabled(bEnableGroup && _perform_sequential_scan.isSelected());
        _sequentual_alpha_overall.setEnabled(bEnableGroup && _perform_sequential_scan.isSelected());
        _sequential_alpha_spending_label.setEnabled(bEnableGroup && _perform_sequential_scan.isSelected());
        _sequential_alpha_spending.setEnabled(bEnableGroup && _perform_sequential_scan.isSelected());
        _alpha_spent_to_date_label.setEnabled(bEnableGroup && _perform_sequential_scan.isSelected());
        if (bEnableGroup && _perform_sequential_scan.isSelected()) {
            double spent = Parameters.getAlphaSpentToDate(_settings_window._outputFileTextField.getText());
            _alpha_spent_to_date_label.setText("(Alpha Spent to Date is " + (spent <= 0 ? 0.0 : spent) + ")");
        } else {
            _alpha_spent_to_date_label.setText("(Alpha Spent to Date is N/A)");
        }

        /*if (scanType == Parameters.ScanType.TREEONLY) {
            ((CardLayout) _panel_sequential_analysis.getLayout()).show(_panel_sequential_analysis, _sequential_treeonly_cardname);
            _panel_sequential_analysis_tree_only.setEnabled(bEnableGroup && _perform_sequential_scan.isSelected());
            _sequential_alpha_label.setEnabled(bEnableGroup && _perform_sequential_scan.isSelected());
            _sequentual_alpha.setEnabled(bEnableGroup && _perform_sequential_scan.isSelected());
            _sequential_alpha_spending_label.setEnabled(bEnableGroup && _perform_sequential_scan.isSelected());
            _sequential_alpha_spending.setEnabled(bEnableGroup && _perform_sequential_scan.isSelected());
            _sequential_signal_cutoff_label.setEnabled(bEnableGroup && _perform_sequential_scan.isSelected());
            _sequential_signal_cutoff.setEnabled(bEnableGroup && _perform_sequential_scan.isSelected());
        } else {
            ((CardLayout) _perform_sequential_scan.getLayout()).show(_perform_sequential_scan, _sequential_timeonly_cardname);
            _maximum_cases_signal_label.setEnabled(bEnableGroup && _perform_sequential_scan.isSelected());
            _maximum_cases_signal.setEnabled(bEnableGroup && _perform_sequential_scan.isSelected());
            _minimum_cases_signal_label.setEnabled(bEnableGroup && _perform_sequential_scan.isSelected());
            _minimum_cases_signal.setEnabled(bEnableGroup && _perform_sequential_scan.isSelected());
            _sequential_analysis_file_label.setEnabled(bEnableGroup && _perform_sequential_scan.isSelected());
            _sequential_analysis_file.setEnabled(bEnableGroup && _perform_sequential_scan.isSelected());
            _sequential_analysis_file_browse.setEnabled(bEnableGroup && _perform_sequential_scan.isSelected());
        }*/
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
        _risk_window_group = new javax.swing.JPanel();
        _apply_risk_window_restriction = new javax.swing.JCheckBox();
        _risk_window_percentage = new javax.swing.JTextField();
        _risk_window_percentage_label = new javax.swing.JLabel();
        _advanced_inferenece_tab = new javax.swing.JPanel();
        jPanel1 = new javax.swing.JPanel();
        _labelMonteCarloReplications = new javax.swing.JLabel();
        _montCarloReplicationsTextField = new javax.swing.JTextField();
        jPanel3 = new javax.swing.JPanel();
        _restricted_levels = new javax.swing.JTextField();
        _restrict_evaluated_levels = new javax.swing.JCheckBox();
        _advanced_power_evaluation_tab = new javax.swing.JPanel();
        _powerEvaluationsGroup = new javax.swing.JPanel();
        _performPowerEvaluations = new javax.swing.JCheckBox();
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
        _eventProbabilityLabel = new javax.swing.JLabel();
        _eventProbabiltyNumerator = new javax.swing.JTextField();
        _eventProbabilityLabel2 = new javax.swing.JLabel();
        _eventProbabiltyDenominator = new javax.swing.JTextField();
        _advanced_output_tab = new javax.swing.JPanel();
        _log_likelihood_ratios_group = new javax.swing.JPanel();
        _reportLLRResultsAsCsvTable = new javax.swing.JCheckBox();
        _report_critical_values_group = new javax.swing.JPanel();
        _reportCriticalValuesCheckBox = new javax.swing.JCheckBox();
        jPanel2 = new javax.swing.JPanel();
        _chk_rpt_attributable_risk = new javax.swing.JCheckBox();
        _attributable_risk_exposed = new javax.swing.JTextField();
        _chk_attributable_risk_extra = new javax.swing.JLabel();
        _advanced_adjustments_tab = new javax.swing.JPanel();
        _group_exclusions = new javax.swing.JPanel();
        _time_range_restrictions = new javax.swing.JTextField();
        _apply_time_range_restrictions = new javax.swing.JCheckBox();
        jPanel5 = new javax.swing.JPanel();
        _perform_dayofweek_adjustments = new javax.swing.JCheckBox();
        _sequential_analysis_tab = new javax.swing.JPanel();
        _sequential_analysis_group = new javax.swing.JPanel();
        _perform_sequential_scan = new javax.swing.JCheckBox();
        _panel_sequential_analysis = new javax.swing.JPanel();
        _panel_sequential_analysis_time_only = new javax.swing.JPanel();
        _maximum_cases_signal_label = new javax.swing.JLabel();
        _maximum_cases_signal = new javax.swing.JTextField();
        _minimum_cases_signal_label = new javax.swing.JLabel();
        _minimum_cases_signal = new javax.swing.JTextField();
        _sequential_analysis_file_label = new javax.swing.JLabel();
        _sequential_analysis_file = new javax.swing.JTextField();
        _sequential_analysis_file_browse = new javax.swing.JButton();
        _panel_sequential_analysis_tree_only = new javax.swing.JPanel();
        _sequential_alpha_label = new javax.swing.JLabel();
        _sequentual_alpha_overall = new javax.swing.JTextField();
        _sequential_alpha_spending_label = new javax.swing.JLabel();
        _sequential_alpha_spending = new javax.swing.JTextField();
        _alpha_spent_to_date_label = new javax.swing.JLabel();
        _closeButton = new javax.swing.JButton();
        _setDefaultButton = new javax.swing.JButton();

        setDefaultCloseOperation(javax.swing.WindowConstants.HIDE_ON_CLOSE);
        setPreferredSize(new java.awt.Dimension(575, 440));

        _cutFileLabel.setText("Cut File:"); // NOI18N

        _cutFileImportButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/document_add_small.png"))); // NOI18N
        _cutFileImportButton.setToolTipText("Import cut file ..."); // NOI18N
        _cutFileImportButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                String key = InputSourceSettings.InputFileType.Cut.toString() + "1";
                if (!_settings_window._input_source_map.containsKey(key)) {
                    _settings_window._input_source_map.put(key, new InputSourceSettings(InputSourceSettings.InputFileType.Cut));
                }
                InputSourceSettings inputSourceSettings = (InputSourceSettings)_settings_window._input_source_map.get(key);
                // invoke the FileSelectionDialog to guide user through process of selecting the source file.
                FileSelectionDialog selectionDialog = new FileSelectionDialog(TreeScanApplication.getInstance(), inputSourceSettings.getInputFileType(), TreeScanApplication.getInstance().lastBrowseDirectory);
                selectionDialog.browse_inputsource(_cutFileTextField, inputSourceSettings, _settings_window, false);
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
                        .addComponent(_cutFileLabel)
                        .addGap(0, 0, Short.MAX_VALUE))
                    .addGroup(_advanced_input_tabLayout.createSequentialGroup()
                        .addComponent(_cutFileTextField, javax.swing.GroupLayout.DEFAULT_SIZE, 503, Short.MAX_VALUE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_cutFileImportButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addContainerGap())))
        );
        _advanced_input_tabLayout.setVerticalGroup(
            _advanced_input_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_advanced_input_tabLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_cutFileLabel)
                .addGap(9, 9, 9)
                .addGroup(_advanced_input_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_cutFileImportButton)
                    .addComponent(_cutFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(268, Short.MAX_VALUE))
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
                .addContainerGap(231, Short.MAX_VALUE))
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
                .addContainerGap(392, Short.MAX_VALUE))
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

        _risk_window_group.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Risk Window"));

        _apply_risk_window_restriction.setSelected(true);
        _apply_risk_window_restriction.setText("Restriction risk windows to");
        _apply_risk_window_restriction.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                _risk_window_percentage.setEnabled(e.getStateChange() == java.awt.event.ItemEvent.SELECTED);
                enableSetDefaultsButton();
            }
        });
        _apply_risk_window_restriction.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                _apply_risk_window_restrictionActionPerformed(evt);
            }
        });

        _risk_window_percentage.setText("20");

        _risk_window_percentage.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                double dMaxValue =  100.0;
                while (_risk_window_percentage.getText().length() == 0 ||
                    Double.parseDouble(_risk_window_percentage.getText()) < 0 ||
                    Double.parseDouble(_risk_window_percentage.getText()) > dMaxValue) {
                    if (undo.canUndo()) undo.undo(); else _risk_window_percentage.setText("20");
                }
                enableSetDefaultsButton();
            }
        });

        _risk_window_percentage.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                if (_risk_window_percentage.getText().length() > 0) {
                    Utils.validatePostiveFloatKeyTyped(_risk_window_percentage, e, 5);
                }
            }
        });

        _risk_window_percentage.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });

        _risk_window_percentage_label.setText(" percent of evaluated windows.");

        javax.swing.GroupLayout _risk_window_groupLayout = new javax.swing.GroupLayout(_risk_window_group);
        _risk_window_group.setLayout(_risk_window_groupLayout);
        _risk_window_groupLayout.setHorizontalGroup(
            _risk_window_groupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_risk_window_groupLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_apply_risk_window_restriction)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_risk_window_percentage, javax.swing.GroupLayout.PREFERRED_SIZE, 70, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_risk_window_percentage_label, javax.swing.GroupLayout.DEFAULT_SIZE, 277, Short.MAX_VALUE)
                .addContainerGap())
        );
        _risk_window_groupLayout.setVerticalGroup(
            _risk_window_groupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_risk_window_groupLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_risk_window_groupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                    .addGroup(_risk_window_groupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_risk_window_percentage, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(_risk_window_percentage_label))
                    .addComponent(_apply_risk_window_restriction))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        javax.swing.GroupLayout _advanced_temporal_window_tabLayout = new javax.swing.GroupLayout(_advanced_temporal_window_tab);
        _advanced_temporal_window_tab.setLayout(_advanced_temporal_window_tabLayout);
        _advanced_temporal_window_tabLayout.setHorizontalGroup(
            _advanced_temporal_window_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_advanced_temporal_window_tabLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_advanced_temporal_window_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_maxTemporalOptionsGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_minTemporalOptionsGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_risk_window_group, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        _advanced_temporal_window_tabLayout.setVerticalGroup(
            _advanced_temporal_window_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_advanced_temporal_window_tabLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_maxTemporalOptionsGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_minTemporalOptionsGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_risk_window_group, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(82, Short.MAX_VALUE))
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
                .addContainerGap(171, Short.MAX_VALUE))
        );
        jPanel1Layout.setVerticalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel1Layout.createSequentialGroup()
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_labelMonteCarloReplications)
                    .addComponent(_montCarloReplicationsTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        jPanel3.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Tree Levels"));

        TextPrompt tp = new TextPrompt("enter comma separated list of integers - root is 1", _restricted_levels);
        tp.setForeground( Color.BLUE );
        tp.changeAlpha(0.5f);
        tp.changeStyle(Font.BOLD + Font.ITALIC);

        _restrict_evaluated_levels.setText("Do not evaluate tree levels:");
        _restrict_evaluated_levels.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                enableRestrictedLevelsGroup();
                enableSetDefaultsButton();
            }
        });

        javax.swing.GroupLayout jPanel3Layout = new javax.swing.GroupLayout(jPanel3);
        jPanel3.setLayout(jPanel3Layout);
        jPanel3Layout.setHorizontalGroup(
            jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel3Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_restrict_evaluated_levels)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_restricted_levels)
                .addContainerGap())
        );
        jPanel3Layout.setVerticalGroup(
            jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel3Layout.createSequentialGroup()
                .addGroup(jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_restrict_evaluated_levels)
                    .addComponent(_restricted_levels, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        javax.swing.GroupLayout _advanced_inferenece_tabLayout = new javax.swing.GroupLayout(_advanced_inferenece_tab);
        _advanced_inferenece_tab.setLayout(_advanced_inferenece_tabLayout);
        _advanced_inferenece_tabLayout.setHorizontalGroup(
            _advanced_inferenece_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_advanced_inferenece_tabLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_advanced_inferenece_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(jPanel1, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(jPanel3, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        _advanced_inferenece_tabLayout.setVerticalGroup(
            _advanced_inferenece_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_advanced_inferenece_tabLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jPanel1, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jPanel3, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(203, Short.MAX_VALUE))
        );

        jTabbedPane1.addTab("Inference", _advanced_inferenece_tab);

        _powerEvaluationsGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Statistical Power Evaluation"));

        _performPowerEvaluations.setText("Perform Power Evaluations");
        _performPowerEvaluations.addItemListener(new java.awt.event.ItemListener() {
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

        _alternativeHypothesisFilenameButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/document_add_small.png"))); // NOI18N
        _alternativeHypothesisFilenameButton.setToolTipText("Open file wizard for alternative hypothesis file ..."); // NOI18N
        _alternativeHypothesisFilenameButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                String key = InputSourceSettings.InputFileType.Power_Evaluations.toString() + "1";
                if (!_settings_window._input_source_map.containsKey(key)) {
                    _settings_window._input_source_map.put(key, new InputSourceSettings(InputSourceSettings.InputFileType.Power_Evaluations));
                }
                InputSourceSettings inputSourceSettings = (InputSourceSettings)_settings_window._input_source_map.get(key);
                // invoke the FileSelectionDialog to guide user through process of selecting the source file.
                FileSelectionDialog selectionDialog = new FileSelectionDialog(TreeScanApplication.getInstance(), inputSourceSettings.getInputFileType(), TreeScanApplication.getInstance().lastBrowseDirectory);
                selectionDialog.browse_inputsource(_alternativeHypothesisFilename, inputSourceSettings, _settings_window, false);
            }
        });

        _eventProbabilityLabel.setText("Case Probability:");

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

        _eventProbabilityLabel2.setText("/");

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

        javax.swing.GroupLayout _powerEvaluationsGroupLayout = new javax.swing.GroupLayout(_powerEvaluationsGroup);
        _powerEvaluationsGroup.setLayout(_powerEvaluationsGroupLayout);
        _powerEvaluationsGroupLayout.setHorizontalGroup(
            _powerEvaluationsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_powerEvaluationsGroupLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_powerEvaluationsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_performPowerEvaluations, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_partOfRegularAnalysis, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_powerEvaluationWithCaseFile, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addGroup(_powerEvaluationsGroupLayout.createSequentialGroup()
                        .addComponent(_powerEvaluationWithSpecifiedCases)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_totalPowerCases, javax.swing.GroupLayout.PREFERRED_SIZE, 78, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_powerEvaluationWithSpecifiedCasesLabel, javax.swing.GroupLayout.DEFAULT_SIZE, 257, Short.MAX_VALUE))
                    .addComponent(_alternativeHypothesisFilenameLabel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addGroup(_powerEvaluationsGroupLayout.createSequentialGroup()
                        .addComponent(_alternativeHypothesisFilename)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_alternativeHypothesisFilenameButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addGroup(_powerEvaluationsGroupLayout.createSequentialGroup()
                        .addGroup(_powerEvaluationsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addGroup(_powerEvaluationsGroupLayout.createSequentialGroup()
                                .addComponent(_eventProbabilityLabel)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(_eventProbabiltyNumerator, javax.swing.GroupLayout.PREFERRED_SIZE, 40, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(_eventProbabilityLabel2)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(_eventProbabiltyDenominator, javax.swing.GroupLayout.PREFERRED_SIZE, 40, javax.swing.GroupLayout.PREFERRED_SIZE))
                            .addGroup(_powerEvaluationsGroupLayout.createSequentialGroup()
                                .addComponent(_numberPowerReplicationsLabel)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(_numberPowerReplications, javax.swing.GroupLayout.PREFERRED_SIZE, 58, javax.swing.GroupLayout.PREFERRED_SIZE)))
                        .addGap(0, 183, Short.MAX_VALUE)))
                .addContainerGap())
        );
        _powerEvaluationsGroupLayout.setVerticalGroup(
            _powerEvaluationsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_powerEvaluationsGroupLayout.createSequentialGroup()
                .addComponent(_performPowerEvaluations)
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
                    .addComponent(_eventProbabilityLabel)
                    .addComponent(_eventProbabiltyNumerator, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_eventProbabilityLabel2)
                    .addComponent(_eventProbabiltyDenominator, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_powerEvaluationsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_numberPowerReplicationsLabel)
                    .addComponent(_numberPowerReplications, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(_alternativeHypothesisFilenameLabel)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_powerEvaluationsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_alternativeHypothesisFilename, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_alternativeHypothesisFilenameButton))
                .addContainerGap(101, Short.MAX_VALUE))
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
            .addComponent(_powerEvaluationsGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
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
                .addComponent(_reportLLRResultsAsCsvTable, javax.swing.GroupLayout.DEFAULT_SIZE, 510, Short.MAX_VALUE)
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
                .addComponent(_reportCriticalValuesCheckBox, javax.swing.GroupLayout.DEFAULT_SIZE, 502, Short.MAX_VALUE)
                .addContainerGap())
        );
        _report_critical_values_groupLayout.setVerticalGroup(
            _report_critical_values_groupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_report_critical_values_groupLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_reportCriticalValuesCheckBox)
                .addContainerGap(14, Short.MAX_VALUE))
        );

        jPanel2.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Attributable Risk"));

        _chk_rpt_attributable_risk.setText("Report attributable risk based on ");
        _chk_rpt_attributable_risk.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                _attributable_risk_exposed.setEnabled(e.getStateChange() == java.awt.event.ItemEvent.SELECTED);
                enableSetDefaultsButton();
            }
        });

        _attributable_risk_exposed.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                enableSetDefaultsButton();
            }
        });

        _attributable_risk_exposed.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                if (_attributable_risk_exposed.getText().length() > 0) {
                    Utils.validatePostiveNumericKeyTyped(_attributable_risk_exposed, e, 10);
                }
            }
        });
        _attributable_risk_exposed.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });

        _chk_attributable_risk_extra.setText("exposed.");

        javax.swing.GroupLayout jPanel2Layout = new javax.swing.GroupLayout(jPanel2);
        jPanel2.setLayout(jPanel2Layout);
        jPanel2Layout.setHorizontalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_chk_rpt_attributable_risk)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_attributable_risk_exposed, javax.swing.GroupLayout.PREFERRED_SIZE, 70, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_chk_attributable_risk_extra, javax.swing.GroupLayout.DEFAULT_SIZE, 243, Short.MAX_VALUE)
                .addContainerGap())
        );
        jPanel2Layout.setVerticalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                    .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_attributable_risk_exposed, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(_chk_attributable_risk_extra))
                    .addComponent(_chk_rpt_attributable_risk))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        javax.swing.GroupLayout _advanced_output_tabLayout = new javax.swing.GroupLayout(_advanced_output_tab);
        _advanced_output_tab.setLayout(_advanced_output_tabLayout);
        _advanced_output_tabLayout.setHorizontalGroup(
            _advanced_output_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_advanced_output_tabLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_advanced_output_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_log_likelihood_ratios_group, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_report_critical_values_group, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(jPanel2, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        _advanced_output_tabLayout.setVerticalGroup(
            _advanced_output_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_advanced_output_tabLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jPanel2, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(_log_likelihood_ratios_group, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_report_critical_values_group, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(114, Short.MAX_VALUE))
        );

        jTabbedPane1.addTab("Additional Output", _advanced_output_tab);

        _group_exclusions.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Data Time Range Exclusions"));

        TextPrompt trp = new TextPrompt("enter semi-colon separated range list (e.g. [-7,0];[12,18])", _time_range_restrictions);
        trp.setForeground( Color.BLUE );
        trp.changeAlpha(0.5f);
        trp.changeStyle(Font.BOLD + Font.ITALIC);

        _apply_time_range_restrictions.setText("Apply Range Exclusions");
        _apply_time_range_restrictions.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                enableTimeRangeExclusionsGroup();
                enableSetDefaultsButton();
            }
        });

        javax.swing.GroupLayout _group_exclusionsLayout = new javax.swing.GroupLayout(_group_exclusions);
        _group_exclusions.setLayout(_group_exclusionsLayout);
        _group_exclusionsLayout.setHorizontalGroup(
            _group_exclusionsLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_group_exclusionsLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_apply_time_range_restrictions)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_time_range_restrictions, javax.swing.GroupLayout.DEFAULT_SIZE, 365, Short.MAX_VALUE)
                .addContainerGap())
        );
        _group_exclusionsLayout.setVerticalGroup(
            _group_exclusionsLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_group_exclusionsLayout.createSequentialGroup()
                .addGroup(_group_exclusionsLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(_group_exclusionsLayout.createSequentialGroup()
                        .addGap(3, 3, 3)
                        .addComponent(_time_range_restrictions))
                    .addComponent(_apply_time_range_restrictions))
                .addGap(11, 11, 11))
        );

        _perform_dayofweek_adjustments.setText("Perform Day of Week Adjustments");
        _perform_dayofweek_adjustments.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                enablePowerEvaluationsGroup();
                enableSetDefaultsButton();
            }
        });

        javax.swing.GroupLayout jPanel5Layout = new javax.swing.GroupLayout(jPanel5);
        jPanel5.setLayout(jPanel5Layout);
        jPanel5Layout.setHorizontalGroup(
            jPanel5Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(_perform_dayofweek_adjustments, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, 534, Short.MAX_VALUE)
        );
        jPanel5Layout.setVerticalGroup(
            jPanel5Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(_perform_dayofweek_adjustments)
        );

        javax.swing.GroupLayout _advanced_adjustments_tabLayout = new javax.swing.GroupLayout(_advanced_adjustments_tab);
        _advanced_adjustments_tab.setLayout(_advanced_adjustments_tabLayout);
        _advanced_adjustments_tabLayout.setHorizontalGroup(
            _advanced_adjustments_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_advanced_adjustments_tabLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_advanced_adjustments_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(jPanel5, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_group_exclusions, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        _advanced_adjustments_tabLayout.setVerticalGroup(
            _advanced_adjustments_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_advanced_adjustments_tabLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jPanel5, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(_group_exclusions, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap())
        );

        jTabbedPane1.addTab("Adjustments", _advanced_adjustments_tab);

        _sequential_analysis_group.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Sequential Analysis"));

        _perform_sequential_scan.setText("Perform Sequential Analysis");
        _perform_sequential_scan.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                enableSequentialAnalysisGroup();
                enableSetDefaultsButton();
            }
        });

        _panel_sequential_analysis.setLayout(new java.awt.CardLayout());

        _maximum_cases_signal_label.setText("Maximum Cases to Signal");

        _maximum_cases_signal.setText("200");
        _maximum_cases_signal.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                while (_maximum_cases_signal.getText().length() == 0)
                if (undo.canUndo()) undo.undo(); else _maximum_cases_signal.setText("200");
                enableSetDefaultsButton();
            }
        });
        _maximum_cases_signal.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validatePostiveNumericKeyTyped(_maximum_cases_signal, e, 10);
            }
        });
        _maximum_cases_signal.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });

        _minimum_cases_signal_label.setText("Minimum Cases to Signal");

        _minimum_cases_signal.setText("3");
        _minimum_cases_signal.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                while (_minimum_cases_signal.getText().length() == 0)
                if (undo.canUndo()) undo.undo(); else _minimum_cases_signal.setText("200");
                enableSetDefaultsButton();
            }
        });
        _minimum_cases_signal.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validatePostiveNumericKeyTyped(_minimum_cases_signal, e, 10);
            }
        });
        _minimum_cases_signal.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });

        _sequential_analysis_file_label.setText("Sequential Analysis File:"); // NOI18N

        _sequential_analysis_file.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                enableSetDefaultsButton();
            }
        });

        _sequential_analysis_file_browse.setIcon(new javax.swing.ImageIcon(getClass().getResource("/document_add_small.png"))); // NOI18N
        _sequential_analysis_file_browse.setToolTipText("Open file wizard for sequential analysis file ..."); // NOI18N
        _sequential_analysis_file_browse.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                List<InputFileFilter> filters = new ArrayList<InputFileFilter>();
                filters.add(new InputFileFilter("csv","Sequential Scan Files (*.csv)"));
                FileSelectionDialog select = new FileSelectionDialog(TreeScanApplication.getInstance(), "Select Sequential Scan File", filters, TreeScanApplication.getInstance().lastBrowseDirectory);
                File file = select.browse_saveas();
                if (file != null) {
                    TreeScanApplication.getInstance().lastBrowseDirectory = select.getDirectory();
                    String filename = file.getAbsolutePath();
                    if (new File(filename).getName().lastIndexOf('.') == -1){
                        filename = filename + ".csv";
                    }
                    _sequential_analysis_file.setText(filename);
                }
            }
        });

        javax.swing.GroupLayout _panel_sequential_analysis_time_onlyLayout = new javax.swing.GroupLayout(_panel_sequential_analysis_time_only);
        _panel_sequential_analysis_time_only.setLayout(_panel_sequential_analysis_time_onlyLayout);
        _panel_sequential_analysis_time_onlyLayout.setHorizontalGroup(
            _panel_sequential_analysis_time_onlyLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_panel_sequential_analysis_time_onlyLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_panel_sequential_analysis_time_onlyLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(_panel_sequential_analysis_time_onlyLayout.createSequentialGroup()
                        .addComponent(_sequential_analysis_file)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_sequential_analysis_file_browse, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addGroup(_panel_sequential_analysis_time_onlyLayout.createSequentialGroup()
                        .addGroup(_panel_sequential_analysis_time_onlyLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                            .addComponent(_maximum_cases_signal_label, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .addComponent(_minimum_cases_signal_label, javax.swing.GroupLayout.PREFERRED_SIZE, 144, javax.swing.GroupLayout.PREFERRED_SIZE))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(_panel_sequential_analysis_time_onlyLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                            .addComponent(_maximum_cases_signal, javax.swing.GroupLayout.DEFAULT_SIZE, 80, Short.MAX_VALUE)
                            .addComponent(_minimum_cases_signal))
                        .addGap(0, 264, Short.MAX_VALUE))
                    .addComponent(_sequential_analysis_file_label, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)))
        );
        _panel_sequential_analysis_time_onlyLayout.setVerticalGroup(
            _panel_sequential_analysis_time_onlyLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_panel_sequential_analysis_time_onlyLayout.createSequentialGroup()
                .addGroup(_panel_sequential_analysis_time_onlyLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_maximum_cases_signal_label)
                    .addComponent(_maximum_cases_signal, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addGroup(_panel_sequential_analysis_time_onlyLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_minimum_cases_signal_label)
                    .addComponent(_minimum_cases_signal, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(_sequential_analysis_file_label)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_panel_sequential_analysis_time_onlyLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_sequential_analysis_file, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_sequential_analysis_file_browse))
                .addGap(0, 0, Short.MAX_VALUE))
        );

        _panel_sequential_analysis.add(_panel_sequential_analysis_time_only, "sequential-timeonly");

        _sequential_alpha_label.setText("Alpha Overall");

        _sequentual_alpha_overall.setText("0.05");
        _sequentual_alpha_overall.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                while (_sequentual_alpha_overall.getText().length() == 0)
                if (undo.canUndo()) undo.undo(); else _sequentual_alpha_overall.setText("0.05");
                enableSetDefaultsButton();
            }
        });
        _sequentual_alpha_overall.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validatePostiveFloatKeyTyped(_sequentual_alpha_overall, e, 10);
            }
        });
        _sequentual_alpha_overall.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });

        _sequential_alpha_spending_label.setText("Alpha Spend Current Look");
        _sequential_alpha_spending_label.setToolTipText("");

        _sequential_alpha_spending.setText("0.01");
        _sequential_alpha_spending.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                while (_sequential_alpha_spending.getText().length() == 0)
                if (undo.canUndo()) undo.undo(); else _sequential_alpha_spending.setText("0.01");
                enableSetDefaultsButton();
            }
        });
        _sequential_alpha_spending.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validatePostiveFloatKeyTyped(_sequential_alpha_spending, e, 10);
            }
        });
        _sequential_alpha_spending.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });

        _alpha_spent_to_date_label.setText("(Alpha Spent to Date)");

        javax.swing.GroupLayout _panel_sequential_analysis_tree_onlyLayout = new javax.swing.GroupLayout(_panel_sequential_analysis_tree_only);
        _panel_sequential_analysis_tree_only.setLayout(_panel_sequential_analysis_tree_onlyLayout);
        _panel_sequential_analysis_tree_onlyLayout.setHorizontalGroup(
            _panel_sequential_analysis_tree_onlyLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_panel_sequential_analysis_tree_onlyLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_panel_sequential_analysis_tree_onlyLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                    .addComponent(_sequential_alpha_spending_label, javax.swing.GroupLayout.DEFAULT_SIZE, 139, Short.MAX_VALUE)
                    .addComponent(_sequential_alpha_label, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_panel_sequential_analysis_tree_onlyLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_sequentual_alpha_overall, javax.swing.GroupLayout.PREFERRED_SIZE, 91, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addGroup(_panel_sequential_analysis_tree_onlyLayout.createSequentialGroup()
                        .addComponent(_sequential_alpha_spending, javax.swing.GroupLayout.PREFERRED_SIZE, 91, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                        .addComponent(_alpha_spent_to_date_label, javax.swing.GroupLayout.PREFERRED_SIZE, 238, javax.swing.GroupLayout.PREFERRED_SIZE)))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );
        _panel_sequential_analysis_tree_onlyLayout.setVerticalGroup(
            _panel_sequential_analysis_tree_onlyLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_panel_sequential_analysis_tree_onlyLayout.createSequentialGroup()
                .addGroup(_panel_sequential_analysis_tree_onlyLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_sequential_alpha_label)
                    .addComponent(_sequentual_alpha_overall, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_panel_sequential_analysis_tree_onlyLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_sequential_alpha_spending_label)
                    .addComponent(_sequential_alpha_spending, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_alpha_spent_to_date_label))
                .addGap(0, 201, Short.MAX_VALUE))
        );

        _panel_sequential_analysis.add(_panel_sequential_analysis_tree_only, "sequential-treeonly");

        javax.swing.GroupLayout _sequential_analysis_groupLayout = new javax.swing.GroupLayout(_sequential_analysis_group);
        _sequential_analysis_group.setLayout(_sequential_analysis_groupLayout);
        _sequential_analysis_groupLayout.setHorizontalGroup(
            _sequential_analysis_groupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_sequential_analysis_groupLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_sequential_analysis_groupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_perform_sequential_scan, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_panel_sequential_analysis, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        _sequential_analysis_groupLayout.setVerticalGroup(
            _sequential_analysis_groupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_sequential_analysis_groupLayout.createSequentialGroup()
                .addComponent(_perform_sequential_scan)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_panel_sequential_analysis, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addContainerGap())
        );

        javax.swing.GroupLayout _sequential_analysis_tabLayout = new javax.swing.GroupLayout(_sequential_analysis_tab);
        _sequential_analysis_tab.setLayout(_sequential_analysis_tabLayout);
        _sequential_analysis_tabLayout.setHorizontalGroup(
            _sequential_analysis_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_sequential_analysis_tabLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_sequential_analysis_group, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addContainerGap())
        );
        _sequential_analysis_tabLayout.setVerticalGroup(
            _sequential_analysis_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_sequential_analysis_tabLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_sequential_analysis_group, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addContainerGap())
        );

        jTabbedPane1.addTab("Sequential", _sequential_analysis_tab);

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

    private void _apply_risk_window_restrictionActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event__apply_risk_window_restrictionActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event__apply_risk_window_restrictionActionPerformed

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JPanel _advanced_adjustments_tab;
    private javax.swing.JPanel _advanced_inferenece_tab;
    private javax.swing.JPanel _advanced_input_tab;
    private javax.swing.JPanel _advanced_output_tab;
    private javax.swing.JPanel _advanced_power_evaluation_tab;
    private javax.swing.JPanel _advanced_temporal_window_tab;
    private javax.swing.JLabel _alpha_spent_to_date_label;
    private javax.swing.JTextField _alternativeHypothesisFilename;
    private javax.swing.JButton _alternativeHypothesisFilenameButton;
    private javax.swing.JLabel _alternativeHypothesisFilenameLabel;
    private javax.swing.JCheckBox _apply_risk_window_restriction;
    private javax.swing.JCheckBox _apply_time_range_restrictions;
    private javax.swing.JTextField _attributable_risk_exposed;
    private javax.swing.JLabel _chk_attributable_risk_extra;
    private javax.swing.JCheckBox _chk_rpt_attributable_risk;
    private javax.swing.JButton _closeButton;
    private javax.swing.JButton _cutFileImportButton;
    private javax.swing.JLabel _cutFileLabel;
    public javax.swing.JTextField _cutFileTextField;
    private javax.swing.JLabel _eventProbabilityLabel;
    private javax.swing.JLabel _eventProbabilityLabel2;
    private javax.swing.JTextField _eventProbabiltyDenominator;
    private javax.swing.JTextField _eventProbabiltyNumerator;
    private javax.swing.JPanel _group_exclusions;
    private javax.swing.JLabel _labelMonteCarloReplications;
    private javax.swing.JPanel _log_likelihood_ratios_group;
    private javax.swing.JTextField _maxTemporalClusterSizeTextField;
    private javax.swing.JTextField _maxTemporalClusterSizeUnitsTextField;
    private javax.swing.JPanel _maxTemporalOptionsGroup;
    private javax.swing.JLabel _maxTemporalTimeUnitsLabel;
    private javax.swing.JTextField _maximum_cases_signal;
    private javax.swing.JLabel _maximum_cases_signal_label;
    private javax.swing.JTextField _minTemporalClusterSizeUnitsTextField;
    private javax.swing.JPanel _minTemporalOptionsGroup;
    private javax.swing.JLabel _minTemporalTimeUnitsLabel;
    private javax.swing.JTextField _minimum_cases_signal;
    private javax.swing.JLabel _minimum_cases_signal_label;
    private javax.swing.JTextField _montCarloReplicationsTextField;
    private javax.swing.JTextField _numberPowerReplications;
    private javax.swing.JLabel _numberPowerReplicationsLabel;
    private javax.swing.JPanel _panel_sequential_analysis;
    private javax.swing.JPanel _panel_sequential_analysis_time_only;
    private javax.swing.JPanel _panel_sequential_analysis_tree_only;
    private javax.swing.JRadioButton _partOfRegularAnalysis;
    private javax.swing.JLabel _percentageOfStudyPeriodLabel;
    private javax.swing.JRadioButton _percentageTemporalRadioButton;
    private javax.swing.JCheckBox _performPowerEvaluations;
    private javax.swing.JCheckBox _perform_dayofweek_adjustments;
    private javax.swing.JCheckBox _perform_sequential_scan;
    private javax.swing.ButtonGroup _powerEstimationButtonGroup;
    private javax.swing.JRadioButton _powerEvaluationWithCaseFile;
    private javax.swing.JRadioButton _powerEvaluationWithSpecifiedCases;
    private javax.swing.JLabel _powerEvaluationWithSpecifiedCasesLabel;
    private javax.swing.JPanel _powerEvaluationsGroup;
    private javax.swing.JCheckBox _reportCriticalValuesCheckBox;
    private javax.swing.JCheckBox _reportLLRResultsAsCsvTable;
    private javax.swing.JPanel _report_critical_values_group;
    private javax.swing.JCheckBox _restrict_evaluated_levels;
    private javax.swing.JTextField _restricted_levels;
    private javax.swing.JPanel _risk_window_group;
    private javax.swing.JTextField _risk_window_percentage;
    private javax.swing.JLabel _risk_window_percentage_label;
    private javax.swing.JLabel _sequential_alpha_label;
    private javax.swing.JTextField _sequential_alpha_spending;
    private javax.swing.JLabel _sequential_alpha_spending_label;
    private javax.swing.JTextField _sequential_analysis_file;
    private javax.swing.JButton _sequential_analysis_file_browse;
    private javax.swing.JLabel _sequential_analysis_file_label;
    private javax.swing.JPanel _sequential_analysis_group;
    private javax.swing.JPanel _sequential_analysis_tab;
    private javax.swing.JTextField _sequentual_alpha_overall;
    private javax.swing.JButton _setDefaultButton;
    private javax.swing.JRadioButton _timeTemporalRadioButton;
    private javax.swing.JTextField _time_range_restrictions;
    private javax.swing.JTextField _totalPowerCases;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JPanel jPanel2;
    private javax.swing.JPanel jPanel3;
    private javax.swing.JPanel jPanel5;
    private javax.swing.JTabbedPane jTabbedPane1;
    private javax.swing.ButtonGroup maximumWindowButtonGroup;
    // End of variables declaration//GEN-END:variables
}
