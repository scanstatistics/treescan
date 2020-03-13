package org.treescan.gui;

import java.awt.Component;
import java.util.Iterator;
import java.util.Map;
import javax.swing.JOptionPane;
import javax.swing.JRootPane;
import javax.swing.event.InternalFrameEvent;
import javax.swing.event.UndoableEditEvent;
import javax.swing.event.UndoableEditListener;
import org.treescan.app.AdvFeaturesExpectionSequential;
import org.treescan.app.Parameters;
import org.treescan.gui.utils.FileSelectionDialog;
import org.treescan.gui.utils.Utils;
import org.treescan.importer.InputSourceSettings;
import org.treescan.utils.FileAccess;

/**
 *
 * @author hostovic
 */
public class ParameterSettingsSequentialScanFrame extends AbstractParameterSettingsFrame {
    private boolean _reopen_full_window = false;
    private AdvancedParameterSettingsSequentialFrame _advancedParametersSetting;

    /**
     * Creates new form ParameterSettingsSequentialScanFrame
     * @param rootPane
     * @param parameters
     */
    public ParameterSettingsSequentialScanFrame(final JRootPane rootPane, Parameters parameters) {
        super(rootPane, parameters);
    }

    @Override
    protected void initFrameComponents() {
        initComponents();
    }

    @Override
    public void internalFrameClosing(InternalFrameEvent e) {
        if (!_reopen_full_window)
            super.internalFrameClosing(e);
    }

    @Override
    public void internalFrameClosed(InternalFrameEvent e) {
        if (_reopen_full_window) {
            _reopen_full_window = false;
            TreeScanApplication.getInstance().openSessionWindow(_parameters, true);
        }
    }

    private void setAlphSpentingToDateLabel(Parameters parameters) {
        double spent = Parameters.getAlphaSpentToDate(parameters.getOutputFileName());
        _alpha_spent_to_date_label.setText("(Alpha Spent to Date is " + (spent <= 0 ? 0.0 : spent) + ")");
        _sequential_alpha_spending.setText(Double.toString(parameters.getSequentialAlphaSpending()));
    }

    /** setup interface from parameter settings
     * @param parameters */
    @Override
    protected void setupInterface(final Parameters parameters) {
        _advancedParametersSetting = new AdvancedParameterSettingsSequentialFrame(_rootPane, this/*, parameters*/);
        title = parameters.getSourceFileName();
        if (title == null || title.length() == 0) {
            title = "New Session";
        }

        _sequential_alpha_label.setText("Alpha Overall = " + parameters.getSequentialAlphaOverall());
        setAlphSpentingToDateLabel(parameters);

        // Since this is a follow-up analysis, the user will need to specify a new count and control file.
        //_countFileTextField.setText(parameters.getCountFileName());
        //_countFileTextField.setCaretPosition(0);
        //_controlFileTextField.setText(parameters.getControlFileName());
        //_controlFileTextField.setCaretPosition(0);

        _input_source_map.clear();
        for (int i=0; i < parameters.getInputSourceSettings().size(); ++i) {
            InputSourceSettings iss = parameters.getInputSourceSettings().get(i);
            if (iss.getInputFileType() != InputSourceSettings.InputFileType.Counts)
                _input_source_map.put(iss.getInputFileType().toString() + iss.getIndex(), iss);
        }
        _advancedParametersSetting.setupInterface(parameters);
    }

    /**
     * sets CParameters class with settings in form
     */
    protected void saveParameterSettings(Parameters parameters) {
        setTitle(parameters.getSourceFileName());

        parameters.setSequentialAlphaSpending(Double.parseDouble(_sequential_alpha_spending.getText()));

        parameters.setCountFileName(_countFileTextField.getText());
        parameters.setControlFileName(_controlFileTextField.getText());

        // TODO -- clear only count file source setting!
        parameters.clearInputSourceSettings();

        Iterator it = _input_source_map.entrySet().iterator();
        while (it.hasNext()) {
            Map.Entry pairs = (Map.Entry)it.next();
            InputSourceSettings iss = (InputSourceSettings)pairs.getValue();
            if (iss.isSet()) {
                parameters.addInputSourceSettings((InputSourceSettings)pairs.getValue());
            }
        }
        getAdvancedParameterInternalFrame().saveParameterSettings(parameters);
    }

    /**
     * Returns reference to associated advanced parameters frame.
     */
    private AdvancedParameterSettingsSequentialFrame getAdvancedParameterInternalFrame() {
        return _advancedParametersSetting;
    }    
    
    /**
     * enables correct advanced settings button on Analysis and Output tabs
     */
    public void enableAdvancedButtons() {
        // Output tab Advanced button
        if (!getAdvancedParameterInternalFrame().getDefaultsSetForOutputOptions()) {
            _advancedOutputButton.setFont(new java.awt.Font("Tahoma", java.awt.Font.BOLD, 11));
        } else {
            _advancedOutputButton.setFont(new java.awt.Font("Tahoma", java.awt.Font.PLAIN, 11));
        }
    }    
    
    /* Returns indication as to whether this trimmed down version should be shown instead of the full setting window. */
    static boolean shouldShowWindow(Parameters parameters) {
        boolean shouldShow = true;
        // Is this a sequential bernoulli model scan?
        shouldShow &= parameters.isSequentialScanBernoulli();
        // Is the tree filename defined and exist?
        shouldShow &= parameters.getTreeFileName(1).length() != 0 && FileAccess.ValidateFileAccess(parameters.getTreeFileName(1), false);
        // Is the output filename defined and exist/writable?
        shouldShow &= parameters.getOutputFileName().length() != 0 && FileAccess.ValidateFileAccess(parameters.getOutputFileName(), true);
        // Is this a follow-up alpha spending iteration?
        return shouldShow && Parameters.getAlphaSpentToDate(parameters.getOutputFileName()) != 0.0;
    }

    @Override
    public boolean CheckSettings() {
        try {
            if (_countFileTextField.getText().length() == 0)
                throw new SettingsException("Please specify a count file.", (Component) _countFileTextField);
            if (!FileAccess.ValidateFileAccess(_countFileTextField.getText(), false))
                throw new SettingsException("The count file could not be opened for reading.\n\nPlease confirm that the path and/or file name are valid and that you have permissions to read from this directory and file.", (Component) _countFileTextField);
            if (_controlFileTextField.getText().length() == 0)
                throw new SettingsException("Please specify a control file.", (Component) _controlFileTextField);
            if (!FileAccess.ValidateFileAccess(_controlFileTextField.getText(), false))
                throw new SettingsException("The control file could not be opened for reading.\n\nPlease confirm that the path and/or file name are valid and that you have permissions to read from this directory and file.", (Component) _controlFileTextField);
            /* Check output settings */
        } catch (SettingsException e) {
            focusWindow();
            JOptionPane.showInternalMessageDialog(this, e.getMessage());
            e.setControlFocus();
            return false;
        }
        try {
            getAdvancedParameterInternalFrame().CheckSettings();
        } catch (AdvFeaturesExpectionSequential e) {
            focusWindow();
            JOptionPane.showInternalMessageDialog(this, e.getMessage());
            getAdvancedParameterInternalFrame().setVisible(e.focusComponent);
            enableAdvancedButtons();
            return false;
        }        
        return true;
    }

    @Override
    public Parameters.ScanType getScanType() {
        return Parameters.ScanType.TREEONLY;
    }

    @Override
    public Parameters.ConditionalType getConditionalType() {
        return Parameters.ConditionalType.UNCONDITIONAL;
    }

    @Override
    public Parameters.ModelType getModelType() {
        return Parameters.ModelType.BERNOULLI_TREE;
    }

    @Override
    public void internalFrameActivated(InternalFrameEvent e) {
        setAlphSpentingToDateLabel(_parameters);
    }

    /**
     * This method is called from within the constructor to initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is always
     * regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        jPanel2 = new javax.swing.JPanel();
        _sequential_alpha_label = new javax.swing.JLabel();
        _sequential_alpha_spending_label = new javax.swing.JLabel();
        _sequential_alpha_spending = new javax.swing.JTextField();
        _alpha_spent_to_date_label = new javax.swing.JLabel();
        jPanel3 = new javax.swing.JPanel();
        _countFileLabel = new javax.swing.JLabel();
        _countFileTextField = new javax.swing.JTextField();
        _countFileImportButton = new javax.swing.JButton();
        _controlFileLabel = new javax.swing.JLabel();
        _controlFileTextField = new javax.swing.JTextField();
        _controlFileImportButton = new javax.swing.JButton();
        jLabel1 = new javax.swing.JLabel();
        jLabel2 = new javax.swing.JLabel();
        _advancedOutputButton = new javax.swing.JButton();

        setClosable(true);
        setDefaultCloseOperation(javax.swing.WindowConstants.DO_NOTHING_ON_CLOSE);
        setIconifiable(true);
        setResizable(true);

        jPanel2.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Alpha Spending"));

        _sequential_alpha_label.setText("Alpha Overall = 0.0");

        _sequential_alpha_spending_label.setText("Alpha Spend Current Look");
        _sequential_alpha_spending_label.setToolTipText("");

        _sequential_alpha_spending.setText("0.01");
        _sequential_alpha_spending.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                while (_sequential_alpha_spending.getText().length() == 0)
                if (undo.canUndo()) undo.undo(); else _sequential_alpha_spending.setText("0.01");
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

        javax.swing.GroupLayout jPanel2Layout = new javax.swing.GroupLayout(jPanel2);
        jPanel2.setLayout(jPanel2Layout);
        jPanel2Layout.setHorizontalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel2Layout.createSequentialGroup()
                        .addComponent(_sequential_alpha_spending_label, javax.swing.GroupLayout.PREFERRED_SIZE, 139, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_sequential_alpha_spending, javax.swing.GroupLayout.PREFERRED_SIZE, 91, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                        .addComponent(_alpha_spent_to_date_label, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                    .addComponent(_sequential_alpha_label, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        jPanel2Layout.setVerticalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addComponent(_sequential_alpha_label)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_sequential_alpha_spending_label)
                    .addComponent(_sequential_alpha_spending, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_alpha_spent_to_date_label))
                .addGap(0, 11, Short.MAX_VALUE))
        );

        jPanel3.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Input"));

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
                selectionDialog.browse_inputsource(_countFileTextField, inputSourceSettings, ParameterSettingsSequentialScanFrame.this, false);
            }
        });

        _controlFileLabel.setText("Control File:"); // NOI18N

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
                selectionDialog.browse_inputsource(_controlFileTextField, inputSourceSettings, ParameterSettingsSequentialScanFrame.this, true);
            }
        });

        javax.swing.GroupLayout jPanel3Layout = new javax.swing.GroupLayout(jPanel3);
        jPanel3.setLayout(jPanel3Layout);
        jPanel3Layout.setHorizontalGroup(
            jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel3Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel3Layout.createSequentialGroup()
                        .addComponent(_countFileTextField)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_countFileImportButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addContainerGap())
                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel3Layout.createSequentialGroup()
                        .addComponent(_controlFileLabel, javax.swing.GroupLayout.DEFAULT_SIZE, 581, Short.MAX_VALUE)
                        .addGap(31, 31, 31))
                    .addGroup(jPanel3Layout.createSequentialGroup()
                        .addGroup(jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addGroup(jPanel3Layout.createSequentialGroup()
                                .addComponent(_countFileLabel)
                                .addGap(0, 0, Short.MAX_VALUE))
                            .addGroup(jPanel3Layout.createSequentialGroup()
                                .addComponent(_controlFileTextField)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(_controlFileImportButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE)))
                        .addContainerGap())))
        );
        jPanel3Layout.setVerticalGroup(
            jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel3Layout.createSequentialGroup()
                .addGroup(jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                    .addComponent(_controlFileImportButton)
                    .addGroup(jPanel3Layout.createSequentialGroup()
                        .addComponent(_countFileLabel)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(_countFileImportButton)
                            .addComponent(_countFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                        .addComponent(_controlFileLabel)
                        .addGap(9, 9, 9)
                        .addComponent(_controlFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        jLabel1.setFont(new java.awt.Font("Tahoma", 1, 11)); // NOI18N
        jLabel1.setText("Sequential  Analysis Window");

        jLabel2.setText("Analysis: Tree only, Unconditional, Bernoulli model");

        _advancedOutputButton.setText("Advanced >>"); // NOI18N
        _advancedOutputButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                getAdvancedParameterInternalFrame().setVisibleWindow(true);
                getAdvancedParameterInternalFrame().requestFocus();
            }
        });

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(jPanel3, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(jPanel2, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(jLabel1, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(jLabel2, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, layout.createSequentialGroup()
                        .addGap(0, 0, Short.MAX_VALUE)
                        .addComponent(_advancedOutputButton)))
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jLabel1)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jLabel2)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jPanel2, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jPanel3, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addComponent(_advancedOutputButton)
                .addContainerGap())
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton _advancedOutputButton;
    private javax.swing.JLabel _alpha_spent_to_date_label;
    private javax.swing.JButton _controlFileImportButton;
    private javax.swing.JLabel _controlFileLabel;
    private javax.swing.JTextField _controlFileTextField;
    private javax.swing.JButton _countFileImportButton;
    private javax.swing.JLabel _countFileLabel;
    private javax.swing.JTextField _countFileTextField;
    private javax.swing.JLabel _sequential_alpha_label;
    private javax.swing.JTextField _sequential_alpha_spending;
    private javax.swing.JLabel _sequential_alpha_spending_label;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JPanel jPanel2;
    private javax.swing.JPanel jPanel3;
    // End of variables declaration//GEN-END:variables
}
