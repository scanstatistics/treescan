package org.treescan.gui;

import java.awt.Component;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.beans.PropertyVetoException;
import java.io.File;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.swing.JOptionPane;
import javax.swing.JRootPane;
import javax.swing.event.InternalFrameEvent;
import javax.swing.event.UndoableEditEvent;
import javax.swing.event.UndoableEditListener;
import org.treescan.app.ParameterHistory;
import org.treescan.app.Parameters;
import org.treescan.gui.utils.FileSelectionDialog;
import org.treescan.gui.utils.InputFileFilter;
import org.treescan.gui.utils.JHyperLink;
import org.treescan.gui.utils.Utils;
import org.treescan.importer.InputSourceSettings;
import org.treescan.utils.FileAccess;

/**
 *
 * @author hostovic
 */
public class ParameterSettingsSequentialScanFrame extends AbstractParameterSettingsFrame {
    private boolean _reopen_full_window = false;

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

    /** setup interface from parameter settings */
    @Override
    protected void setupInterface(final Parameters parameters) {
        title = parameters.getSourceFileName();
        if (title == null || title.length() == 0) {
            title = "New Session";
        }

        _sequential_alpha_label.setText("Alpha Overall = " + parameters.getSequentialAlphaOverall());
        setAlphSpentingToDateLabel(parameters);

        _treelFileTextField.setText(parameters.getTreeFileNames().get(0));
        _treelFileTextField.setCaretPosition(0);
        _countFileTextField.setText(parameters.getCountFileName());
        _countFileTextField.setCaretPosition(0);

        _outputFileTextField.setText(parameters.getOutputFileName());
        _outputFileTextField.setCaretPosition(0);
        _reportResultsAsHTML.setSelected(parameters.isGeneratingHtmlResults());
        _reportResultsAsCsvTable.setSelected(parameters.isGeneratingTableResults());

        _input_source_map.clear();
        for (int i=0; i < parameters.getInputSourceSettings().size(); ++i) {
            InputSourceSettings iss = parameters.getInputSourceSettings().get(i);
            _input_source_map.put(iss.getInputFileType().toString() + iss.getIndex(), iss);
        }
    }

    /**
     * sets CParameters class with settings in form
     */
    protected void saveParameterSettings(Parameters parameters) {
        setTitle(parameters.getSourceFileName());

        parameters.setSequentialAlphaSpending(Double.parseDouble(_sequential_alpha_spending.getText()));

        parameters.setTreeFileName(_treelFileTextField.getText(), 1);
        parameters.setCountFileName(_countFileTextField.getText());

        parameters.setOutputFileName(_outputFileTextField.getText());
        parameters.setGeneratingHtmlResults(_reportResultsAsHTML.isSelected());
        parameters.setGeneratingTableResults(_reportResultsAsCsvTable.isSelected());

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

    public boolean CheckSettings() {
        try {
            /* Check input settings */
            if (_treelFileTextField.getText().length() == 0)
                throw new SettingsException("Please specify a tree file.", (Component) _treelFileTextField);
            if (!FileAccess.ValidateFileAccess(_treelFileTextField.getText(), false))
                throw new SettingsException("The tree file could not be opened for reading.\n\nPlease confirm that the path and/or file name are valid and that you have permissions to read from this directory and file.", (Component) _treelFileTextField);
            String validationString = validateInputSourceDataFile(_treelFileTextField.getText(), InputSourceSettings.InputFileType.Tree.toString() + "1", "tree");
            if (validationString != null) throw new SettingsException(validationString, (Component) _treelFileTextField);
            if (_countFileTextField.getText().length() == 0)
                throw new SettingsException("Please specify a count file.", (Component) _countFileTextField);
            if (!FileAccess.ValidateFileAccess(_countFileTextField.getText(), false))
                throw new SettingsException("The count file could not be opened for reading.\n\nPlease confirm that the path and/or file name are valid and that you have permissions to read from this directory and file.", (Component) _countFileTextField);
            /* Check output settings */
            if (_outputFileTextField.getText().length() == 0)
                throw new SettingsException("Please specify a results file.", (Component) _outputFileTextField);
            if (!FileAccess.ValidateFileAccess(_outputFileTextField.getText(), true))
                throw new SettingsException("Results file could not be opened for writing.\n" + "Please confirm that the path and/or file name\n" +
                                            "are valid and that you have permissions to write\nto this directory and file.",
                                            (Component) _outputFileTextField);
        } catch (SettingsException e) {
            focusWindow();
            JOptionPane.showInternalMessageDialog(this, e.getMessage());
            e.setControlFocus();
            return false;
        }
        return true;
    }

    public Parameters.ScanType getScanType() {
        return Parameters.ScanType.TREEONLY;
    }

    public Parameters.ConditionalType getConditionalType() {
        return Parameters.ConditionalType.UNCONDITIONAL;
    }

    public Parameters.ModelType getModelType() {
        return Parameters.ModelType.BERNOULLI;
    }

    /**
     * This method is called from within the constructor to initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is always
     * regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        jPanel1 = new javax.swing.JPanel();
        _resultsFileLabel = new javax.swing.JLabel();
        _outputFileTextField = new javax.swing.JTextField();
        _resultsFileBrowseButton = new javax.swing.JButton();
        _resultsFileLabel1 = new javax.swing.JLabel();
        _reportResultsAsHTML = new javax.swing.JCheckBox();
        _reportResultsAsCsvTable = new javax.swing.JCheckBox();
        jPanel2 = new javax.swing.JPanel();
        _sequential_alpha_label = new javax.swing.JLabel();
        _sequential_alpha_spending_label = new javax.swing.JLabel();
        _sequential_alpha_spending = new javax.swing.JTextField();
        _alpha_spent_to_date_label = new javax.swing.JLabel();
        jPanel3 = new javax.swing.JPanel();
        _treeFileLabel = new javax.swing.JLabel();
        _treelFileTextField = new javax.swing.JTextField();
        _treeFileImportButton = new javax.swing.JButton();
        _countFileLabel = new javax.swing.JLabel();
        _countFileTextField = new javax.swing.JTextField();
        _countFileImportButton = new javax.swing.JButton();
        jLabel1 = new javax.swing.JLabel();
        _re_launch_full = new JHyperLink("Reopen in full settings window.");

        setClosable(true);
        setDefaultCloseOperation(javax.swing.WindowConstants.DO_NOTHING_ON_CLOSE);
        setIconifiable(true);
        setResizable(true);

        jPanel1.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Output"));

        _resultsFileLabel.setText("Results File:"); // NOI18N

        _outputFileTextField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                setAlphSpentingToDateLabel(_parameters);
            }
        });

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
                setAlphSpentingToDateLabel(_parameters);
            }
        });

        _resultsFileLabel1.setText("Additional Output Files:"); // NOI18N

        _reportResultsAsHTML.setText("Report Results as HTML");

        _reportResultsAsCsvTable.setText("Report Results as CSV Table");

        javax.swing.GroupLayout jPanel1Layout = new javax.swing.GroupLayout(jPanel1);
        jPanel1.setLayout(jPanel1Layout);
        jPanel1Layout.setHorizontalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel1Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel1Layout.createSequentialGroup()
                        .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addGroup(jPanel1Layout.createSequentialGroup()
                                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                    .addGroup(jPanel1Layout.createSequentialGroup()
                                        .addComponent(_resultsFileLabel)
                                        .addGap(0, 0, Short.MAX_VALUE))
                                    .addComponent(_reportResultsAsHTML, javax.swing.GroupLayout.DEFAULT_SIZE, 424, Short.MAX_VALUE)
                                    .addComponent(_resultsFileLabel1, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                                .addGap(111, 111, 111))
                            .addGroup(jPanel1Layout.createSequentialGroup()
                                .addComponent(_outputFileTextField)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)))
                        .addComponent(_resultsFileBrowseButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addComponent(_reportResultsAsCsvTable, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        jPanel1Layout.setVerticalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel1Layout.createSequentialGroup()
                .addComponent(_resultsFileLabel)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_resultsFileBrowseButton)
                    .addComponent(_outputFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_resultsFileLabel1)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_reportResultsAsHTML)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_reportResultsAsCsvTable))
        );

        jPanel2.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Analysis"));

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
                Utils.validatePostiveNumericKeyTyped(_sequential_alpha_spending, e, 10);
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
                selectionDialog.browse_inputsource(_treelFileTextField, inputSourceSettings, ParameterSettingsSequentialScanFrame.this, false);
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
                selectionDialog.browse_inputsource(_countFileTextField, inputSourceSettings, ParameterSettingsSequentialScanFrame.this, false);
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
                        .addComponent(_treeFileLabel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addGap(29, 29, 29))
                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel3Layout.createSequentialGroup()
                        .addGroup(jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addGroup(jPanel3Layout.createSequentialGroup()
                                .addComponent(_countFileTextField)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(_countFileImportButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE))
                            .addGroup(jPanel3Layout.createSequentialGroup()
                                .addComponent(_treelFileTextField)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(_treeFileImportButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE)))
                        .addContainerGap())
                    .addGroup(jPanel3Layout.createSequentialGroup()
                        .addComponent(_countFileLabel)
                        .addGap(0, 0, Short.MAX_VALUE))))
        );
        jPanel3Layout.setVerticalGroup(
            jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel3Layout.createSequentialGroup()
                .addComponent(_treeFileLabel)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_treelFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_treeFileImportButton))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_countFileLabel)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_countFileImportButton)
                    .addComponent(_countFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)))
        );

        jLabel1.setFont(new java.awt.Font("Tahoma", 1, 11)); // NOI18N
        jLabel1.setText("Tree Only Sequential Scan with Unconditional Bernoulli Model");

        _re_launch_full.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
        _re_launch_full.setText("Reopen in full settings window.");
        _re_launch_full.setVerticalAlignment(javax.swing.SwingConstants.TOP);
        ((JHyperLink)_re_launch_full).addActionListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                //if (queryWindowCanClose()) {
                    _reopen_full_window = true;
                    try {
                        ParameterSettingsSequentialScanFrame.this.setClosed(true);
                    } catch (PropertyVetoException ex) {
                        Logger.getLogger(TreeScanApplication.class.getName()).log(Level.SEVERE, null, ex);
                    }
                    //}
            }
        });

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(jPanel1, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(jPanel3, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(jPanel2, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(jLabel1, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_re_launch_full, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jLabel1)
                .addGap(13, 13, 13)
                .addComponent(jPanel2, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jPanel3, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jPanel1, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_re_launch_full)
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JLabel _alpha_spent_to_date_label;
    private javax.swing.JButton _countFileImportButton;
    private javax.swing.JLabel _countFileLabel;
    private javax.swing.JTextField _countFileTextField;
    public javax.swing.JTextField _outputFileTextField;
    private javax.swing.JLabel _re_launch_full;
    private javax.swing.JCheckBox _reportResultsAsCsvTable;
    private javax.swing.JCheckBox _reportResultsAsHTML;
    private javax.swing.JButton _resultsFileBrowseButton;
    private javax.swing.JLabel _resultsFileLabel;
    private javax.swing.JLabel _resultsFileLabel1;
    private javax.swing.JLabel _sequential_alpha_label;
    private javax.swing.JTextField _sequential_alpha_spending;
    private javax.swing.JLabel _sequential_alpha_spending_label;
    private javax.swing.JButton _treeFileImportButton;
    private javax.swing.JLabel _treeFileLabel;
    private javax.swing.JTextField _treelFileTextField;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JPanel jPanel2;
    private javax.swing.JPanel jPanel3;
    // End of variables declaration//GEN-END:variables
}
