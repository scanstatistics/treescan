package org.treescan.gui;

import java.awt.Component;
import java.awt.Container;
import javax.swing.ImageIcon;
import javax.swing.JPanel;
import javax.swing.JRootPane;
import javax.swing.event.MouseInputAdapter;
import javax.swing.event.UndoableEditEvent;
import javax.swing.event.UndoableEditListener;
import javax.swing.undo.UndoManager;
import org.treescan.app.AdvFeaturesExpectionSequential;
import org.treescan.app.Parameters;
import org.treescan.gui.utils.Utils;

public class AdvancedParameterSettingsSequentialFrame extends javax.swing.JInternalFrame {
    private JPanel _glass = null;
    private final JRootPane _rootPane;
    private final Component _rootPaneInitialGlass;
    private final UndoManager undo = new UndoManager();
    private final ParameterSettingsSequentialScanFrame _settings_window;

    /**
     * Creates new form ParameterSettingsFrame
     */
    public AdvancedParameterSettingsSequentialFrame(final JRootPane rootPane, final ParameterSettingsSequentialScanFrame analysisSettingsWindow) {
        initComponents();

        setFrameIcon(new ImageIcon(getClass().getResource("/TreeScan.png")));
        _rootPane = rootPane;
        _settings_window = analysisSettingsWindow;
        _rootPaneInitialGlass = rootPane.getGlassPane();
        // create opaque glass pane
        _glass = new JPanel();
        _glass.setOpaque(false);
        // Attach mouse listeners
        MouseInputAdapter adapter = new MouseInputAdapter() {};
        _glass.addMouseListener(adapter);
        _glass.addMouseMotionListener(adapter);
        // Add modal internal frame to glass pane
        _glass.add(this);
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
    public void setVisible(Component focusComponent) {
        //set tab set visible
        setVisibleWindow(true);
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
    public void setVisibleWindow(boolean value) {
        if (value == false) {
            _closeButton.requestFocus();
        } //cause any text controls to loose focus
        super.setVisible(value);
        if (value) {
            startModal();
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
        _setDefaultButton.setEnabled(!getDefaultsSetForOutputOptions());
    }

    public boolean getDefaultsSetForOutputOptions() {
        boolean bReturn = true;
        bReturn &= _reportLLRResultsAsCsvTable.isSelected() == false;
        bReturn &= _reportCriticalValuesCheckBox.isSelected() == false;
        bReturn &= _chk_rpt_attributable_risk.isSelected() == false;
        bReturn &= _attributable_risk_exposed.getText().equals("");
        return bReturn;
    }

    private synchronized void startModal() {
        if (_glass != null) {
            _rootPane.setGlassPane(_glass);
            _glass.setVisible(true); // Change glass pane to our panel
        }
        setFocusedTabSet();
    }

    private void setFocusedTabSet() {
        jTabbedPane1.removeAll();
        setTitle("Advanced Output Options");
       jTabbedPane1.addTab("Additional Output", null, _advanced_output_tab, null);
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
        parameters.setGeneratingHtmlResults(_reportResultsAsHTML.isSelected());
        parameters.setGeneratingTableResults(_reportResultsAsCsvTable.isSelected());
        parameters.setGeneratingLLRResults(_reportLLRResultsAsCsvTable.isSelected());
        parameters.setReportCriticalValues(_reportCriticalValuesCheckBox.isSelected());
        parameters.setReportAttributableRisk(_chk_rpt_attributable_risk.isEnabled() && _chk_rpt_attributable_risk.isSelected());
        parameters.setAttributableRiskExposed(_attributable_risk_exposed.getText().length() > 0 ? Integer.parseInt(_attributable_risk_exposed.getText()): 0);
    }

    /**
     * Resets advanced settings to default values
     */
    private void setDefaultsClick() {
        setDefaultsForOutputTab();
        enableSetDefaultsButton();
    }

    public void setupInterface(final Parameters parameters) {
        _reportResultsAsHTML.setSelected(parameters.isGeneratingHtmlResults());
        _reportResultsAsCsvTable.setSelected(parameters.isGeneratingTableResults());
        _reportCriticalValuesCheckBox.setSelected(parameters.getReportCriticalValues());
        _reportLLRResultsAsCsvTable.setSelected(parameters.isGeneratingLLRResults());
        _chk_rpt_attributable_risk.setSelected(parameters.getReportAttributableRisk());
        _attributable_risk_exposed.setText(parameters.getAttributableRiskExposed() > 0 ? Integer.toString(parameters.getAttributableRiskExposed()) : "");
    }

    /**
     * Sets default values for Output related tab and respective controls pulled
     * these default values from the CParameter class
     */
    private void setDefaultsForOutputTab() {
        _reportResultsAsHTML.setSelected(false);
        _reportResultsAsCsvTable.setSelected(false);
        _reportLLRResultsAsCsvTable.setSelected(false);
        _reportCriticalValuesCheckBox.setSelected(false);
        _chk_rpt_attributable_risk.setSelected(false);
        _attributable_risk_exposed.setText("");
    }

    /*
     * Verifies that settings are valid in the context of all other parameter settings.
     */
    public void CheckSettings() {
        CheckAdditionalOutputOptions();
    }

    /*
     * Verifies that additional output settings are valid in the context of all parameter settings.
     */
    private void CheckAdditionalOutputOptions() throws NumberFormatException, AdvFeaturesExpectionSequential {
        if (_chk_rpt_attributable_risk.isEnabled() && _chk_rpt_attributable_risk.isSelected()) {
            if (_attributable_risk_exposed.getText().trim().length() == 0) {
                throw new AdvFeaturesExpectionSequential("Please specify a number exposed for the attributable risk.", (Component)_attributable_risk_exposed);
            }
            if (Integer.parseInt(_attributable_risk_exposed.getText().trim()) < 1) {
                throw new AdvFeaturesExpectionSequential("The number exposed for the attributable risk must be greater than zero.", (Component) _attributable_risk_exposed);
            }
        }
    }

    /** enables options of the Additional Output tab */
    public void enableAdditionalOutputOptions() {
        _chk_rpt_attributable_risk.setEnabled(true);
        _attributable_risk_exposed.setEnabled(_chk_rpt_attributable_risk.isEnabled() && _chk_rpt_attributable_risk.isSelected());
        _chk_attributable_risk_extra.setEnabled(_chk_rpt_attributable_risk.isEnabled());
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
        _advanced_output_tab = new javax.swing.JPanel();
        _log_likelihood_ratios_group = new javax.swing.JPanel();
        _reportLLRResultsAsCsvTable = new javax.swing.JCheckBox();
        _report_critical_values_group = new javax.swing.JPanel();
        _reportCriticalValuesCheckBox = new javax.swing.JCheckBox();
        jPanel2 = new javax.swing.JPanel();
        _chk_rpt_attributable_risk = new javax.swing.JCheckBox();
        _attributable_risk_exposed = new javax.swing.JTextField();
        _chk_attributable_risk_extra = new javax.swing.JLabel();
        jPanel1 = new javax.swing.JPanel();
        _reportResultsAsHTML = new javax.swing.JCheckBox();
        _reportResultsAsCsvTable = new javax.swing.JCheckBox();
        _closeButton = new javax.swing.JButton();
        _setDefaultButton = new javax.swing.JButton();

        setDefaultCloseOperation(javax.swing.WindowConstants.HIDE_ON_CLOSE);
        setPreferredSize(new java.awt.Dimension(575, 440));

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

        jPanel1.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Additional Output Files"));

        _reportResultsAsHTML.setText("Report Results as HTML");

        _reportResultsAsCsvTable.setText("Report Results as CSV Table");

        javax.swing.GroupLayout jPanel1Layout = new javax.swing.GroupLayout(jPanel1);
        jPanel1.setLayout(jPanel1Layout);
        jPanel1Layout.setHorizontalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel1Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_reportResultsAsCsvTable, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_reportResultsAsHTML, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        jPanel1Layout.setVerticalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel1Layout.createSequentialGroup()
                .addComponent(_reportResultsAsHTML)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_reportResultsAsCsvTable)
                .addContainerGap(9, Short.MAX_VALUE))
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
                    .addComponent(jPanel2, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(jPanel1, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        _advanced_output_tabLayout.setVerticalGroup(
            _advanced_output_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _advanced_output_tabLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jPanel1, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jPanel2, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(_log_likelihood_ratios_group, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_report_critical_values_group, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(20, Short.MAX_VALUE))
        );

        jTabbedPane1.addTab("Additional Output", _advanced_output_tab);

        _closeButton.setText("Close"); // NOI18N
        _closeButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                setVisibleWindow(false);
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
                .addComponent(jTabbedPane1, javax.swing.GroupLayout.DEFAULT_SIZE, 344, Short.MAX_VALUE)
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
    private javax.swing.JPanel _advanced_output_tab;
    private javax.swing.JTextField _attributable_risk_exposed;
    private javax.swing.JLabel _chk_attributable_risk_extra;
    private javax.swing.JCheckBox _chk_rpt_attributable_risk;
    private javax.swing.JButton _closeButton;
    private javax.swing.JPanel _log_likelihood_ratios_group;
    private javax.swing.ButtonGroup _powerEstimationButtonGroup;
    private javax.swing.JCheckBox _reportCriticalValuesCheckBox;
    private javax.swing.JCheckBox _reportLLRResultsAsCsvTable;
    private javax.swing.JCheckBox _reportResultsAsCsvTable;
    private javax.swing.JCheckBox _reportResultsAsHTML;
    private javax.swing.JPanel _report_critical_values_group;
    private javax.swing.JButton _setDefaultButton;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JPanel jPanel2;
    private javax.swing.JTabbedPane jTabbedPane1;
    private javax.swing.ButtonGroup maximumWindowButtonGroup;
    // End of variables declaration//GEN-END:variables
}
