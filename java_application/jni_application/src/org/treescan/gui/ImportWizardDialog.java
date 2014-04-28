/*
 * ImportWizardDialog.java
 *
 * Created on December 12, 2007, 3:11 PM
 */
package org.treescan.gui;

import java.awt.CardLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.Vector;
import java.util.prefs.Preferences;
import javax.swing.DefaultCellEditor;
import javax.swing.JComboBox;
import javax.swing.JOptionPane;
import javax.swing.JScrollBar;
import javax.swing.SwingWorker;
import javax.swing.event.UndoableEditEvent;
import javax.swing.event.UndoableEditListener;
import javax.swing.table.AbstractTableModel;
import javax.swing.table.DefaultTableModel;
import javax.swing.table.TableColumn;
import javax.swing.undo.UndoManager;
import org.treescan.importer.CSVImportDataSource;
import org.treescan.importer.DBaseImportDataSource;
import org.treescan.importer.FileImporter;
import org.treescan.importer.ImportDataSource;
import org.treescan.importer.ImportException;
import org.treescan.importer.ImportVariable;
import org.treescan.importer.PreviewTableModel;
import org.treescan.app.UnknownEnumException;
import org.treescan.gui.utils.AutofitTableColumns;
import org.treescan.gui.utils.FileSelectionDialog;
import org.treescan.gui.utils.InputFileFilter;
import org.treescan.importer.XLSImportDataSource;
import org.treescan.gui.utils.Utils;
import org.treescan.gui.utils.WaitCursor;
import static org.treescan.importer.FileImporter.InputFileType.Cuts;
import static org.treescan.importer.FileImporter.InputFileType.Tree;

/**
 *
 * @author  Hostovic
 */
public class ImportWizardDialog extends javax.swing.JDialog implements PropertyChangeListener {

    private Preferences _prefs = Preferences.userNodeForPackage(getClass());
    private static final String _prefLastBackup = new String("import.destination");
    private final String _fileFormatCardName = "File Format";
    private final String _dataMappingCardName = "Mapping Panel";
    private final String _outPutSettingsCardName = "Output Settings";
    private final String _fileFormatDelimitedCardName = "cvsPanel";
    private final String _fileFormatFixedColumnCardName = "fixedPanel";
    private final String _unAssignedVariableName = "unassigned";
    private final String _importFilePrefix = "import";
    private final int _dateVariableColumn = 2;
    private final int _sourceFileLineSample = 200;
    private final UndoManager undo = new UndoManager();
    private Vector<ImportVariable> _importVariables = new Vector<ImportVariable>();
    private final String _sourceFile;
    private final FileImporter.InputFileType _fileType;
    private FileImporter.SourceDataFileType _sourceDataFileType;
    private String _showingCard;
    private boolean _errorSamplingSourceFile = true;
    private PreviewTableModel _previewTableModel = null;
    private boolean _cancelled = true;
    private File _destinationFile = null;
    private File _suggested_import_filename;    

    /** Creates new form ImportWizardDialog */
    public ImportWizardDialog(java.awt.Frame parent, 
                              final String sourceFile, 
                              final String suggested_filename,
                              FileImporter.InputFileType fileType) {
        super(parent, true);
        setSuggestedImportName(sourceFile, suggested_filename, fileType);
        initComponents();
        _sourceFile = sourceFile;
        _fileType = fileType;
        
        _sourceDataFileType = getSourceFileType();
        _progressBar.setVisible(false);
        configureForDestinationFileType();
        setLocationRelativeTo(parent);
    }

    private void setSuggestedImportName(final String sourceFile, final String suggested_filename, final FileImporter.InputFileType filetype) {
        String defaultName = "";
        String extension = "";
        switch (filetype) {
            case Tree : defaultName = "Tree"; extension =  ".tre"; break;
            case Case: defaultName = "Cases"; extension =  ".cas"; break;
            case Population: defaultName = "Population"; extension =  ".pop";  break;
            case Cuts : defaultName = "Cuts"; extension =  ".cut"; break;
            default: throw new UnknownEnumException(filetype);
        }
        if (suggested_filename.trim().isEmpty()) {
            _suggested_import_filename = new File(_prefs.get(_prefLastBackup, System.getProperty("user.home")) + System.getProperty("file.separator") + defaultName + extension);
        } else {
            int lastDot = suggested_filename.trim().lastIndexOf(".");
            if (lastDot != -1) 
                _suggested_import_filename = new File(suggested_filename.trim().substring(0, lastDot) + extension); 
            else 
                _suggested_import_filename = new File(suggested_filename.trim() + extension); 
        }
    }
    
    /**
     * Returns whether the user canceled import.
     */
    public boolean getCancelled() {
        return _cancelled;
    }
    /* Create import target file. */
    private void createDestinationInformation() throws IOException {
        _destinationFile = new File(_outputDirectoryTextField.getText());
        _destinationFile.createNewFile();
    }

    /**
     * Returns import destination filename.
     */
    public String getDestinationFilename() {
        return _destinationFile != null ? _destinationFile.getAbsolutePath() : "";
    }

    /**
     * Returns file type given source file extension.
     */
    private FileImporter.SourceDataFileType getSourceFileType() {
        int pos = _sourceFile.lastIndexOf('.');
        if (pos != -1 && _sourceFile.substring(pos + 1).equalsIgnoreCase("dbf")) {
            return FileImporter.SourceDataFileType.dBase;
        }
        if (pos != -1 && (_sourceFile.substring(pos + 1).equalsIgnoreCase("xls") || _sourceFile.substring(pos + 1).equalsIgnoreCase("xlsx"))) {
            return FileImporter.SourceDataFileType.Excel;
        }
        return FileImporter.SourceDataFileType.Delimited;
    }

    private char getColumnDelimiter() {
        if (_commaRadioButton.isSelected()) {
            return ',';
        } else if (_semiColonRadioButton.isSelected()) {
            return ';';
        } else if (_whitespaceRadioButton.isSelected()) {
            return ' ';
        } else if (_otherRadioButton.isSelected()) {
            return _otherFieldSeparatorTextField.getText().toCharArray()[0];
        } else {
            throw new RuntimeException("Unknown column delimiter.");
        }
    }

    private char getGroupMarker() {
        if (_doubleQuotesRadioButton.isSelected()) {
            return '"';
        } else if (_singleQuotesRadioButton.isSelected()) {
            return '\'';
        } else {
            throw new RuntimeException("Unknown group marker.");
        }
    }

    /**
     * Sets which panels and the order of showing panels.
     */
    private void setPanelsToShow(FileImporter.SourceDataFileType eType) {
        _basePanel.removeAll();
        if (eType != FileImporter.SourceDataFileType.dBase && eType != FileImporter.SourceDataFileType.Excel) {
            _basePanel.add(_fileFormatPanel, _fileFormatCardName);
        }
        _basePanel.add(_dataMappingPanel, _dataMappingCardName);
        _basePanel.add(_outputSettingsPanel, _outPutSettingsCardName);
        //reset class variable to that which we are showing
        _sourceDataFileType = eType;
    }

    /**
     * Updates which file format options panel is showing.
     */
    private void showFileTypeFormatOptionsPanel(FileImporter.SourceDataFileType eFileType) {
        CardLayout cl = (CardLayout) (_sourceFileTypeOptions.getLayout());
        switch (eFileType) {
            case Delimited:
                cl.show(_sourceFileTypeOptions, _fileFormatDelimitedCardName);
                _sourceDataFileType = FileImporter.SourceDataFileType.Delimited;
                break;
            default:
                throw new UnknownEnumException(eFileType);
        }
    }

    /**
     * Shows/hides variables based upon destination file type and model/coordindates type.
     */
    private void setShowingVariables() {
        VariableMappingTableModel model = (VariableMappingTableModel) _fieldMapTable.getModel();
        for (int t = 0; t < _importVariables.size(); ++t) {
           model.setShowing(_importVariables.get(t), true);
        }
        model.fireTableDataChanged();
    }

    /** Opens source file as dBase file. */
    private void previewSourceAsDBaseFile() throws Exception {
        try {
            DBaseImportDataSource source = new DBaseImportDataSource(new File(_sourceFile), true);
            _previewTableModel = new PreviewTableModel(true);
            _previewTableModel.addRow(source.getColumnNames());
            for (int i = 0; i < _previewTableModel.getPreviewLength(); ++i) {
                Object[] values = source.readRow();
                if (values != null) {
                    _previewTableModel.addRow(values);
                }
            }
        } catch (Exception e) {
            throw e;
        }
    }

    /** Opens source file as dBase file. */
    private void previewSourceAsExcelFile() throws Exception {
        try {
            XLSImportDataSource source = new XLSImportDataSource(new File(_sourceFile), false);
            _previewTableModel = new PreviewTableModel(true);
            for (int i = 0; i < _previewTableModel.getPreviewLength(); ++i) {
                Object[] values = source.readRow();
                if (values != null) {
                    _previewTableModel.addRow(values);
                }
            }
        } catch (Exception e) {
            throw e;
        }
    }

    /** Opens source file as character delimited source file. */
    private void previewSourceAsCSVFile() throws FileNotFoundException {
        File file = new File(_sourceFile);
        CSVImportDataSource source = new CSVImportDataSource(file, false, '\n', getColumnDelimiter(), getGroupMarker());
        _previewTableModel = new PreviewTableModel(_firstRowColumnHeadersCheckBox.isSelected());
        int skipRows = Integer.parseInt(_ignoreRowsTextField.getText());
        for (int i = 0; i < _previewTableModel.getPreviewLength() + skipRows; ++i) {
            Object[] values = source.readRow();
            if (values != null && i >= skipRows) {
                _previewTableModel.addRow(values);
            }
        }
    }

    private ImportDataSource getImportSource() throws FileNotFoundException {
        if (_sourceDataFileType == FileImporter.SourceDataFileType.dBase) {
            return new DBaseImportDataSource(new File(_sourceFile), false);
        } //return new NativeImportDataSource(new File(_sourceFile));
        else if (_sourceDataFileType == FileImporter.SourceDataFileType.Excel) {
            return new XLSImportDataSource(new File(_sourceFile), true);
        } else {
            return new CSVImportDataSource(new File(_sourceFile), _firstRowColumnHeadersCheckBox.isSelected(),
                    '\n', getColumnDelimiter(), getGroupMarker());
        } 
    }

    /** Opening source as specified by file type. */
    private void previewSource() throws Exception {
        //set the import tables model to default until we have an instance of the native model avaiable
        _importTableDataTable.setModel(new DefaultTableModel());

        //create the table model
        if (_sourceDataFileType == FileImporter.SourceDataFileType.dBase) {
            previewSourceAsDBaseFile();
        } else if (_sourceDataFileType == FileImporter.SourceDataFileType.Excel) {
            previewSourceAsExcelFile();
        } else {
            previewSourceAsCSVFile();
        } 
        //now assign model to table object
        if (_previewTableModel != null) {
            _importTableDataTable.setModel(_previewTableModel);
        }

        int widthTotal = 0;
        //calculate the column widths to fit header/data
        Vector<Integer> colWidths = new Vector<Integer>();
        for (int c=0; c < _importTableDataTable.getColumnCount(); ++c) {
            colWidths.add(AutofitTableColumns.getMaxColumnWidth(_importTableDataTable, c, true, 20));
            widthTotal += colWidths.lastElement();
        }   
        int additional = Math.max(0, _importTableScrollPane.getViewport().getSize().width - widthTotal - 20/*scrollbar width?*/)/colWidths.size();
        for (int c=0; c < colWidths.size(); ++c) {
            _importTableDataTable.getColumnModel().getColumn(c).setMinWidth(colWidths.elementAt(c) + additional);            
        }
    }

    /**
     * Enables navigation button based upon active panel and settings.
     */
    private void enableNavigationButtons() {
        previousButton.setEnabled(false);
        nextButton.setEnabled(false);
        executeButton.setEnabled(false);

        if (_showingCard == null) {
            return;
        } else if (_showingCard.equals(_fileFormatCardName)) {
            nextButton.setEnabled(!_errorSamplingSourceFile &&
                                  (_otherRadioButton.isSelected() ? _otherFieldSeparatorTextField.getText().length() > 0 : true));
        } else if (_showingCard.equals(_dataMappingCardName)) {
            previousButton.setEnabled(_sourceDataFileType != FileImporter.SourceDataFileType.dBase &&
                    _sourceDataFileType != FileImporter.SourceDataFileType.Excel);
            if (_importTableDataTable.getModel().getRowCount() > 0) {
                VariableMappingTableModel model = (VariableMappingTableModel) _fieldMapTable.getModel();
                for (int i = 0; i < model.variables_visible.size() && !nextButton.isEnabled(); ++i) {
                    nextButton.setEnabled(model.variables_visible.get(i).getIsMappedToInputFileVariable());
                }
            }
        } else if (_showingCard.equals(_outPutSettingsCardName)) {
            previousButton.setEnabled(true);
            executeButton.setEnabled(_outputDirectoryTextField.getText().length() > 0);
        }
    //else nop
    }

    /** Event response to changing import file type. */
    private void OnFieldDefinitionChange() {
    /*int   iStartColumn, iSelStart=0, iSelLength=0;
    if (rdoFileType->ItemIndex == Fixed_Column) {
    AddFixedColDefinitionEnable();
    UpdateFixedColDefinitionEnable();
    if (edtStartColumn->GetTextLen()) {
    iStartColumn = StrToInt(edtStartColumn->Text);
    if (iStartColumn > 0) {
    iSelStart = StrToInt(iStartColumn - 1);
    if (edtFieldLength->GetTextLen())
    iSelLength = StrToInt(edtFieldLength->Text);
    }
    }
    }
    memRawData->SelStart = iSelStart;
    memRawData->SelLength = iSelLength;*/
    }

    /**
     * Preparation for viewing file format panel.
     */
    private void prepFileFormatPanel() {
        WaitCursor waitCursor = new WaitCursor(this);
        try {
            readDataFileIntoRawDisplayField();
            showFileTypeFormatOptionsPanel(FileImporter.SourceDataFileType.Delimited);
            //edtFieldName->Text = GetFixedColumnFieldName(1, sFieldName).GetCString();
            enableNavigationButtons();
            OnFieldDefinitionChange();
        } finally {
            waitCursor.restore();
        }
    }

    /**
     * Clears and reassigns values of VariableMappingTableModel combobox column.
     */
    private void setMappingTableComboCells() {
        TableColumn selectionColumn = _fieldMapTable.getColumnModel().getColumn(1);
        VariableMappingTableModel model = (VariableMappingTableModel) _fieldMapTable.getModel();
        //if (model.comboBox.getItemCount() == 0) {
        model.comboBox.removeAllItems();
        model.comboBox.addItem(_unAssignedVariableName);
        for (int i = 0; i < _importTableDataTable.getModel().getColumnCount(); ++i) {
            model.comboBox.addItem(_importTableDataTable.getModel().getColumnName(i));
        }
        selectionColumn.setCellEditor(new DefaultCellEditor(model.comboBox));
        model.fireTableDataChanged();
        clearTreeScanVariableFieldIndexes();

    }

    /** Clears field mapping selections. */
    private void clearTreeScanVariableFieldIndexes() {
        for (int t = 0; t < _importVariables.size(); ++t) {
            _importVariables.get(t).setInputFileVariableIndex(0);
        }
        VariableMappingTableModel model = (VariableMappingTableModel) _fieldMapTable.getModel();
        model.fireTableDataChanged();
    }

    /**
     * Returns the destination file type as string.
     */
    private String getInputFileTypeString() {
        switch (_fileType) {
            case Case:
                return "Case File";
            case Tree:
                return "Tree File";
            case Cuts:
                return "Cuts File";
            case Population:
                return "Population File";
            default:
                throw new UnknownEnumException(_fileType);
        }
    }

    /**
     * Validates that required TreeScan Variables has been specified with an input
     * file field to import from. Displays message if variables are missing.
     */
    private boolean checkForRequiredVariables() {
        StringBuilder message = new StringBuilder();
        Vector<ImportVariable> missing = new Vector<ImportVariable>();
        VariableMappingTableModel model = (VariableMappingTableModel) _fieldMapTable.getModel();

        for (int t = 0; t < _importVariables.size(); ++t) {
            ImportVariable variable = _importVariables.get(t);
            if (model.isShowing(variable.getVariableName()) && !variable.getIsMappedToInputFileVariable() && variable.getIsRequiredField()) {
                missing.add(variable);
            }
        }

        if (missing.size() > 0) {
            message.append("For the " + getInputFileTypeString());
            message.append(", the following TreeScan Variable(s) are required\nand an Input File Variable must");
            message.append(" be selected for each before import can proceed.\n\nTreeScan Variable(s): ");
            for (int t = 0; t < missing.size(); ++t) {
                message.append(missing.get(t).getVariableName());
                if (t < missing.size() - 1) {
                    message.append(", ");
                }
            }
            JOptionPane.showMessageDialog(this, message.toString(), "Note", JOptionPane.WARNING_MESSAGE);
            return true;
        }
        return false;
    }

    /**
     * Preparation for viewing mapping panel.
     */
    private void prepMappingPanel() throws Exception {
        WaitCursor waitCursor = new WaitCursor(this);
        try {
            previewSource();
            setMappingTableComboCells();
            //AutoAlign();
            setShowingVariables();
            enableNavigationButtons();
        } finally {
            waitCursor.restore();
        }
    }

    /**
     * Causes showCard to be the active card.
     */
    private void bringPanelToFront(String showCard) {
        ((CardLayout) _basePanel.getLayout()).show(_basePanel, showCard);
        _showingCard = showCard;
        enableNavigationButtons();
        if (executeButton.isEnabled()) {
            executeButton.requestFocus();
        } else if (nextButton.isEnabled()) {
            nextButton.requestFocus();
        }
    }

    private void prepOutputSettingsPanel() {
        //reset import table model
        _importTableDataTable.setModel(new DefaultTableModel());
    }

    /**
     * Calls appropriate preparation methods then shows panel.
     */
    private void makeActivePanel(String targetCardName) throws Exception {
        if (targetCardName.equals(_fileFormatCardName)) {
            prepFileFormatPanel();
        } else if (targetCardName.equals(_dataMappingCardName)) {
            prepMappingPanel();
        } else if (targetCardName.equals(_outPutSettingsCardName)) {
            prepOutputSettingsPanel();
        }
        bringPanelToFront(targetCardName);
    }

    /**
     * Reads in a sample of data file into a memo field to help user
     * to determine structure of source file.
     */
    private void readDataFileIntoRawDisplayField() {
        _fileContentsTextArea.removeAll();
        _errorSamplingSourceFile = false;

        //Attempt to open source file reader...
        FileReader fileSample = null;
        try {
            fileSample = new FileReader(_sourceFile);
        } catch (FileNotFoundException e) {
            _fileContentsTextArea.append("* Unable to view source file. *");
            _fileContentsTextArea.append("* File does not exist. *");
            _errorSamplingSourceFile = true;
        } catch (SecurityException e) {
            _fileContentsTextArea.append("* Unable to view source file. *");
            _fileContentsTextArea.append("* File permissions deny read access. *");
            _errorSamplingSourceFile = true;
        }
        //Attempt to read the first X lines of file and add to sample text area
        try {
            BufferedReader buffer = new BufferedReader(fileSample);
            for (int i = 0; i < _sourceFileLineSample; ++i) {
                String line = null;
                line = buffer.readLine();
                if (line == null) {
                    break;
                }
                _fileContentsTextArea.append(line + "\n");
            }
            buffer.close();
            fileSample.close();
        } catch (IOException e) {
            _fileContentsTextArea.removeAll();
            _fileContentsTextArea.append("* Unable to view source file. *");
            _errorSamplingSourceFile = true;
        }
        //Indicate whether the source file had any data...
        if (_fileContentsTextArea.getLineCount() == 0) {
            _fileContentsTextArea.append("* Source file contains no data. *");
            _errorSamplingSourceFile = true;
        } else {
            _errorSamplingSourceFile = false;
        }
        JScrollBar vbar = jScrollPane1.getVerticalScrollBar();
        vbar.setValue(vbar.getMinimum());
        _fileContentsTextArea.setCaretPosition(0);
    }

    /**
     * Configures the variables vector based upon the target file type.
     */
    private void configureForDestinationFileType() {
        switch (_fileType) {
            case Case:
                setCountsFileVariables();
                break;
            case Tree:
                setTreeFileVariables();
                break;
            case Cuts:
                setCutsFileVariables();
                break;
            case Population:
                setPopulationFileVariables();
                break;
            default:
                throw new UnknownEnumException(_fileType);
        }
    }

    /**
     * Prepares the dialog for the initial
     */
    @Override
    public void setVisible(boolean value) {
        try {
            if (value) {
                if (_sourceDataFileType == FileImporter.SourceDataFileType.dBase || _sourceDataFileType == FileImporter.SourceDataFileType.Excel) {
                    try {
                        setPanelsToShow(_sourceDataFileType);
                        makeActivePanel(_dataMappingCardName);
                        super.setVisible(value);
                        return;
                    } catch (Throwable e) { // try opening as default delimited ascii file
                        _sourceDataFileType = FileImporter.SourceDataFileType.Delimited;
                    }
                }
                setPanelsToShow(_sourceDataFileType);
                makeActivePanel(_fileFormatCardName);
            }
            super.setVisible(value);
        } catch (Exception e) {
            new ExceptionDialog(this, e).setVisible(true);
        }
    }

    /**
     * Setup field descriptors for case file.
     */
    private void setCountsFileVariables() {
        _importVariables.clear();
        _importVariables.addElement(new ImportVariable("Node ID", 0, true, null));
        _importVariables.addElement(new ImportVariable("Cases", 1, true, null));
        //_importVariables.addElement(new ImportVariable("Duplicate Case", 2, false, null));
        _importVariables.addElement(new ImportVariable("Time", 2, false, null));
    }

    /**
     * Setup field descriptors for control file.
     */
    private void setTreeFileVariables() {
        _importVariables.clear();
        _importVariables.addElement(new ImportVariable("Node ID", 0, true, null));
        _importVariables.addElement(new ImportVariable("Node Parent ID", 1, true, null));
    }

    /**
     * Setup field descriptors for cuts file.
     */
    private void setCutsFileVariables() {
        _importVariables.clear();
        _importVariables.addElement(new ImportVariable("Node ID", 0, true, null));
        _importVariables.addElement(new ImportVariable("Cut Type", 1, true, null));
    }

    /**
     * Setup field descriptors for population file.
     */
    private void setPopulationFileVariables() {
        _importVariables.clear();
        _importVariables.addElement(new ImportVariable("Node ID", 0, true, null));
        _importVariables.addElement(new ImportVariable("population", 1, true, null));
        _importVariables.addElement(new ImportVariable("Time", 2, false, null));
    }    
    
    /**
     * Invoked when task's progress property changes.
     */
    public void propertyChange(PropertyChangeEvent evt) {
        if (evt.getNewValue() instanceof Integer) {
            _progressBar.setValue(((Integer) evt.getNewValue()).intValue());
        }
    }

    /**
     * This method is called from within the constructor to
     * configureForDestinationFileType the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        _fieldSeparatorButtonGroup = new javax.swing.ButtonGroup();
        _groupIndicatorButtonGroup = new javax.swing.ButtonGroup();
        cancelButton = new javax.swing.JButton();
        nextButton = new javax.swing.JButton();
        previousButton = new javax.swing.JButton();
        executeButton = new javax.swing.JButton();
        _basePanel = new javax.swing.JPanel();
        _fileFormatPanel = new javax.swing.JPanel();
        jLabel1 = new javax.swing.JLabel();
        jScrollPane1 = new javax.swing.JScrollPane();
        _fileContentsTextArea = new javax.swing.JTextArea();
        jLabel4 = new javax.swing.JLabel();
        _ignoreRowsTextField = new javax.swing.JTextField();
        jLabel5 = new javax.swing.JLabel();
        _firstRowColumnHeadersCheckBox = new javax.swing.JCheckBox();
        _sourceFileTypeOptions = new javax.swing.JPanel();
        _cSVDefsPanel = new javax.swing.JPanel();
        fieldSeparatorGroup = new javax.swing.JPanel();
        _commaRadioButton = new javax.swing.JRadioButton();
        _semiColonRadioButton = new javax.swing.JRadioButton();
        _whitespaceRadioButton = new javax.swing.JRadioButton();
        _otherRadioButton = new javax.swing.JRadioButton();
        _otherFieldSeparatorTextField = new javax.swing.JTextField();
        _groupIndiocatorGroup = new javax.swing.JPanel();
        _doubleQuotesRadioButton = new javax.swing.JRadioButton();
        _singleQuotesRadioButton = new javax.swing.JRadioButton();
        _dataMappingPanel = new javax.swing.JPanel();
        jSplitPane1 = new javax.swing.JSplitPane();
        _dataMappingTopPanel = new javax.swing.JPanel();
        jScrollPane2 = new javax.swing.JScrollPane();
        _fieldMapTable = new javax.swing.JTable();
        _clearSelectionButton = new javax.swing.JButton();
        _dataMappingBottomPanel = new javax.swing.JPanel();
        _importTableScrollPane = new javax.swing.JScrollPane();
        _importTableDataTable = new javax.swing.JTable();
        _outputSettingsPanel = new javax.swing.JPanel();
        jLabel3 = new javax.swing.JLabel();
        _outputDirectoryTextField = new javax.swing.JTextField();
        _changeSaveDirectoryButton = new javax.swing.JButton();
        _progressBar = new javax.swing.JProgressBar();

        _fieldSeparatorButtonGroup.add(_commaRadioButton);
        _fieldSeparatorButtonGroup.add(_semiColonRadioButton);
        _fieldSeparatorButtonGroup.add(_whitespaceRadioButton);
        _fieldSeparatorButtonGroup.add(_otherRadioButton);

        _groupIndicatorButtonGroup.add(_doubleQuotesRadioButton);
        _groupIndicatorButtonGroup.add(_singleQuotesRadioButton);

        setDefaultCloseOperation(javax.swing.WindowConstants.DISPOSE_ON_CLOSE);
        setTitle("Import Wizard"); // NOI18N
        setModal(true);
        setResizable(false);

        cancelButton.setText("Cancel"); // NOI18N
        cancelButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                _cancelled = true;
                setVisible(false);
            }
        });

        nextButton.setText("Next >"); // NOI18N
        nextButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                try {
                    if (_showingCard == _fileFormatCardName)
                    makeActivePanel(_dataMappingCardName);
                    else if (_showingCard == _dataMappingCardName) {
                        if (checkForRequiredVariables()) return;
                        makeActivePanel(_outPutSettingsCardName);
                    }
                    //else nop
                } catch (Throwable t) {
                    new ExceptionDialog(ImportWizardDialog.this, t).setVisible(true);
                }
            }
        });

        previousButton.setText("< Previous"); // NOI18N
        previousButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                try {
                    if (_showingCard == _dataMappingCardName)
                    makeActivePanel(_fileFormatCardName);
                    else if (_showingCard == _outPutSettingsCardName)
                    makeActivePanel(_dataMappingCardName);
                    //else nop
                } catch (Throwable t) {
                    new ExceptionDialog(ImportWizardDialog.this, t).setVisible(true);
                }
            }
        });

        executeButton.setText("Execute"); // NOI18N
        executeButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                try {
                    try {
                        createDestinationInformation();
                    } catch (IOException i) {
                        JOptionPane.showMessageDialog(ImportWizardDialog.this,
                            "The import wizard encountered an error attempting to create the import file.\n" +
                            "This is most likely occuring because write permissions are not granted for\n" +
                            "specified directory. Please check path or review user permissions for specified directory.\n", "Note", JOptionPane.WARNING_MESSAGE);
                        return;
                    }
                    ImportTask task = new ImportTask();
                    task.addPropertyChangeListener(ImportWizardDialog.this);
                    task.execute();
                } catch (Throwable t) {
                    new ExceptionDialog(ImportWizardDialog.this, t).setVisible(true);
                }
            }
        });

        _basePanel.setLayout(new java.awt.CardLayout());

        jLabel1.setText("File Contents:"); // NOI18N

        _fileContentsTextArea.setColumns(20);
        _fileContentsTextArea.setRows(5);
        jScrollPane1.setViewportView(_fileContentsTextArea);

        jLabel4.setText("Ignore first"); // NOI18N

        _ignoreRowsTextField.setText("0"); // NOI18N
        _ignoreRowsTextField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                while (_ignoreRowsTextField.getText().length() == 0)
                if (undo.canUndo()) undo.undo(); else _ignoreRowsTextField.setText("0");
            }
        });
        _ignoreRowsTextField.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validatePostiveNumericKeyTyped(_ignoreRowsTextField, e, 5);
            }
        });
        _ignoreRowsTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });

        jLabel5.setText("rows"); // NOI18N

        _firstRowColumnHeadersCheckBox.setText("First row is column name"); // NOI18N
        _firstRowColumnHeadersCheckBox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _firstRowColumnHeadersCheckBox.setMargin(new java.awt.Insets(0, 0, 0, 0));

        _sourceFileTypeOptions.setLayout(new java.awt.CardLayout());

        fieldSeparatorGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Field Separator"));

        _commaRadioButton.setSelected(true);
        _commaRadioButton.setText("Comma"); // NOI18N
        _commaRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _commaRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));

        _semiColonRadioButton.setText("Semicolon"); // NOI18N
        _semiColonRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _semiColonRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));

        _whitespaceRadioButton.setText("Whitespace"); // NOI18N
        _whitespaceRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _whitespaceRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));

        _otherRadioButton.setText("Other"); // NOI18N
        _otherRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _otherRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));

        _otherFieldSeparatorTextField.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                //Utils.validatePostiveNumericKeyTyped(_otherFieldSeparatorTextField, e, 1);
                enableNavigationButtons();
            }
        });
        _otherFieldSeparatorTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
                enableNavigationButtons();
            }
        });

        javax.swing.GroupLayout fieldSeparatorGroupLayout = new javax.swing.GroupLayout(fieldSeparatorGroup);
        fieldSeparatorGroup.setLayout(fieldSeparatorGroupLayout);
        fieldSeparatorGroupLayout.setHorizontalGroup(
            fieldSeparatorGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(fieldSeparatorGroupLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_commaRadioButton)
                .addGap(20, 20, 20)
                .addComponent(_semiColonRadioButton)
                .addGap(20, 20, 20)
                .addComponent(_whitespaceRadioButton)
                .addGap(20, 20, 20)
                .addComponent(_otherRadioButton)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_otherFieldSeparatorTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 20, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(177, Short.MAX_VALUE))
        );
        fieldSeparatorGroupLayout.setVerticalGroup(
            fieldSeparatorGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(fieldSeparatorGroupLayout.createSequentialGroup()
                .addGroup(fieldSeparatorGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_commaRadioButton)
                    .addComponent(_semiColonRadioButton)
                    .addComponent(_whitespaceRadioButton)
                    .addComponent(_otherRadioButton)
                    .addComponent(_otherFieldSeparatorTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        _groupIndiocatorGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Group Indicator"));

        _doubleQuotesRadioButton.setSelected(true);
        _doubleQuotesRadioButton.setText("Double Quotes"); // NOI18N
        _doubleQuotesRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _doubleQuotesRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));

        _singleQuotesRadioButton.setText("Single Quotes"); // NOI18N
        _singleQuotesRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _singleQuotesRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));

        javax.swing.GroupLayout _groupIndiocatorGroupLayout = new javax.swing.GroupLayout(_groupIndiocatorGroup);
        _groupIndiocatorGroup.setLayout(_groupIndiocatorGroupLayout);
        _groupIndiocatorGroupLayout.setHorizontalGroup(
            _groupIndiocatorGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_groupIndiocatorGroupLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_doubleQuotesRadioButton)
                .addGap(20, 20, 20)
                .addComponent(_singleQuotesRadioButton)
                .addContainerGap(307, Short.MAX_VALUE))
        );
        _groupIndiocatorGroupLayout.setVerticalGroup(
            _groupIndiocatorGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_groupIndiocatorGroupLayout.createSequentialGroup()
                .addGroup(_groupIndiocatorGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_doubleQuotesRadioButton)
                    .addComponent(_singleQuotesRadioButton))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        javax.swing.GroupLayout _cSVDefsPanelLayout = new javax.swing.GroupLayout(_cSVDefsPanel);
        _cSVDefsPanel.setLayout(_cSVDefsPanelLayout);
        _cSVDefsPanelLayout.setHorizontalGroup(
            _cSVDefsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _cSVDefsPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_cSVDefsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                    .addComponent(_groupIndiocatorGroup, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(fieldSeparatorGroup, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        _cSVDefsPanelLayout.setVerticalGroup(
            _cSVDefsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_cSVDefsPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(fieldSeparatorGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_groupIndiocatorGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(41, Short.MAX_VALUE))
        );

        _sourceFileTypeOptions.add(_cSVDefsPanel, "cvsPanel");

        javax.swing.GroupLayout _fileFormatPanelLayout = new javax.swing.GroupLayout(_fileFormatPanel);
        _fileFormatPanel.setLayout(_fileFormatPanelLayout);
        _fileFormatPanelLayout.setHorizontalGroup(
            _fileFormatPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_fileFormatPanelLayout.createSequentialGroup()
                .addComponent(jLabel1)
                .addContainerGap())
            .addComponent(_sourceFileTypeOptions, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
            .addComponent(jScrollPane1)
            .addGroup(_fileFormatPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jLabel4)
                .addGap(5, 5, 5)
                .addComponent(_ignoreRowsTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 42, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addGap(5, 5, 5)
                .addComponent(jLabel5)
                .addGap(18, 18, 18)
                .addComponent(_firstRowColumnHeadersCheckBox, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addGap(164, 164, 164))
        );
        _fileFormatPanelLayout.setVerticalGroup(
            _fileFormatPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _fileFormatPanelLayout.createSequentialGroup()
                .addComponent(jLabel1)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jScrollPane1, javax.swing.GroupLayout.DEFAULT_SIZE, 284, Short.MAX_VALUE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_fileFormatPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(jLabel4)
                    .addComponent(jLabel5)
                    .addComponent(_ignoreRowsTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_firstRowColumnHeadersCheckBox))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_sourceFileTypeOptions, javax.swing.GroupLayout.PREFERRED_SIZE, 167, javax.swing.GroupLayout.PREFERRED_SIZE))
        );

        _basePanel.add(_fileFormatPanel, "File Format");

        jSplitPane1.setBorder(null);
        jSplitPane1.setDividerLocation(250);
        jSplitPane1.setOrientation(javax.swing.JSplitPane.VERTICAL_SPLIT);

        _fieldMapTable.setRowSelectionAllowed(false);
        _fieldMapTable.setModel(new VariableMappingTableModel(_importVariables));
        jScrollPane2.setViewportView(_fieldMapTable);

        _clearSelectionButton.setText("Clear"); // NOI18N
        _clearSelectionButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                clearTreeScanVariableFieldIndexes();
            }
        });

        javax.swing.GroupLayout _dataMappingTopPanelLayout = new javax.swing.GroupLayout(_dataMappingTopPanel);
        _dataMappingTopPanel.setLayout(_dataMappingTopPanelLayout);
        _dataMappingTopPanelLayout.setHorizontalGroup(
            _dataMappingTopPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _dataMappingTopPanelLayout.createSequentialGroup()
                .addComponent(jScrollPane2, javax.swing.GroupLayout.DEFAULT_SIZE, 468, Short.MAX_VALUE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_clearSelectionButton, javax.swing.GroupLayout.PREFERRED_SIZE, 57, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap())
        );
        _dataMappingTopPanelLayout.setVerticalGroup(
            _dataMappingTopPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_dataMappingTopPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_dataMappingTopPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_clearSelectionButton)
                    .addComponent(jScrollPane2, javax.swing.GroupLayout.DEFAULT_SIZE, 228, Short.MAX_VALUE))
                .addContainerGap())
        );

        jSplitPane1.setTopComponent(_dataMappingTopPanel);

        _importTableDataTable.setModel(new javax.swing.table.DefaultTableModel(
            new Object [][] {
                {null, null, null, null},
                {null, null, null, null},
                {null, null, null, null},
                {null, null, null, null}
            },
            new String [] {
                "Title 1", "Title 2", "Title 3", "Title 4"
            }
        ));
        _importTableDataTable.setAutoResizeMode(javax.swing.JTable.AUTO_RESIZE_OFF);
        _importTableDataTable.setRowSelectionAllowed(false);
        _importTableScrollPane.setViewportView(_importTableDataTable);

        javax.swing.GroupLayout _dataMappingBottomPanelLayout = new javax.swing.GroupLayout(_dataMappingBottomPanel);
        _dataMappingBottomPanel.setLayout(_dataMappingBottomPanelLayout);
        _dataMappingBottomPanelLayout.setHorizontalGroup(
            _dataMappingBottomPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(_importTableScrollPane, javax.swing.GroupLayout.DEFAULT_SIZE, 541, Short.MAX_VALUE)
        );
        _dataMappingBottomPanelLayout.setVerticalGroup(
            _dataMappingBottomPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_dataMappingBottomPanelLayout.createSequentialGroup()
                .addComponent(_importTableScrollPane, javax.swing.GroupLayout.DEFAULT_SIZE, 237, Short.MAX_VALUE)
                .addContainerGap())
        );

        jSplitPane1.setRightComponent(_dataMappingBottomPanel);

        javax.swing.GroupLayout _dataMappingPanelLayout = new javax.swing.GroupLayout(_dataMappingPanel);
        _dataMappingPanel.setLayout(_dataMappingPanelLayout);
        _dataMappingPanelLayout.setHorizontalGroup(
            _dataMappingPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(jSplitPane1)
        );
        _dataMappingPanelLayout.setVerticalGroup(
            _dataMappingPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(jSplitPane1)
        );

        _basePanel.add(_dataMappingPanel, "Mapping Panel");

        jLabel3.setText("Save imported file as:"); // NOI18N

        _outputDirectoryTextField.setText(_suggested_import_filename.getAbsolutePath());

        _changeSaveDirectoryButton.setText("Change"); // NOI18N
        _changeSaveDirectoryButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                InputFileFilter[] filters = new InputFileFilter[]{};
                FileSelectionDialog select = new FileSelectionDialog(org.treescan.gui.TreeScanApplication.getInstance(), "Select filename to save imported file", filters, org.treescan.gui.TreeScanApplication.getInstance().lastBrowseDirectory);
                File file = select.browse_saveas();
                if (file != null) {
                    org.treescan.gui.TreeScanApplication.getInstance().lastBrowseDirectory = select.getDirectory();
                    _outputDirectoryTextField.setText(file.getAbsolutePath());
                    _prefs.put(_prefLastBackup, select.getDirectory().getAbsolutePath());
                }
            }
        });

        javax.swing.GroupLayout _outputSettingsPanelLayout = new javax.swing.GroupLayout(_outputSettingsPanel);
        _outputSettingsPanel.setLayout(_outputSettingsPanelLayout);
        _outputSettingsPanelLayout.setHorizontalGroup(
            _outputSettingsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_outputSettingsPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_outputSettingsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(jLabel3, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_progressBar, javax.swing.GroupLayout.DEFAULT_SIZE, 521, Short.MAX_VALUE)
                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _outputSettingsPanelLayout.createSequentialGroup()
                        .addComponent(_outputDirectoryTextField, javax.swing.GroupLayout.DEFAULT_SIZE, 446, Short.MAX_VALUE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_changeSaveDirectoryButton)))
                .addContainerGap())
        );
        _outputSettingsPanelLayout.setVerticalGroup(
            _outputSettingsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_outputSettingsPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jLabel3)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_outputSettingsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_changeSaveDirectoryButton)
                    .addComponent(_outputDirectoryTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addGap(408, 408, 408)
                .addComponent(_progressBar, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(27, Short.MAX_VALUE))
        );

        _basePanel.add(_outputSettingsPanel, "Output Settings");

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, layout.createSequentialGroup()
                        .addComponent(previousButton, javax.swing.GroupLayout.PREFERRED_SIZE, 100, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(nextButton, javax.swing.GroupLayout.PREFERRED_SIZE, 100, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(executeButton, javax.swing.GroupLayout.PREFERRED_SIZE, 100, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(cancelButton, javax.swing.GroupLayout.PREFERRED_SIZE, 100, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addComponent(_basePanel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_basePanel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(cancelButton)
                    .addComponent(executeButton)
                    .addComponent(nextButton)
                    .addComponent(previousButton))
                .addContainerGap())
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JPanel _basePanel;
    private javax.swing.JPanel _cSVDefsPanel;
    private javax.swing.JButton _changeSaveDirectoryButton;
    private javax.swing.JButton _clearSelectionButton;
    private javax.swing.JRadioButton _commaRadioButton;
    private javax.swing.JPanel _dataMappingBottomPanel;
    private javax.swing.JPanel _dataMappingPanel;
    private javax.swing.JPanel _dataMappingTopPanel;
    private javax.swing.JRadioButton _doubleQuotesRadioButton;
    private javax.swing.JTable _fieldMapTable;
    private javax.swing.ButtonGroup _fieldSeparatorButtonGroup;
    private javax.swing.JTextArea _fileContentsTextArea;
    private javax.swing.JPanel _fileFormatPanel;
    private javax.swing.JCheckBox _firstRowColumnHeadersCheckBox;
    private javax.swing.ButtonGroup _groupIndicatorButtonGroup;
    private javax.swing.JPanel _groupIndiocatorGroup;
    private javax.swing.JTextField _ignoreRowsTextField;
    private javax.swing.JTable _importTableDataTable;
    private javax.swing.JScrollPane _importTableScrollPane;
    private javax.swing.JTextField _otherFieldSeparatorTextField;
    private javax.swing.JRadioButton _otherRadioButton;
    private javax.swing.JTextField _outputDirectoryTextField;
    private javax.swing.JPanel _outputSettingsPanel;
    private javax.swing.JProgressBar _progressBar;
    private javax.swing.JRadioButton _semiColonRadioButton;
    private javax.swing.JRadioButton _singleQuotesRadioButton;
    private javax.swing.JPanel _sourceFileTypeOptions;
    private javax.swing.JRadioButton _whitespaceRadioButton;
    private javax.swing.JButton cancelButton;
    private javax.swing.JButton executeButton;
    private javax.swing.JPanel fieldSeparatorGroup;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel3;
    private javax.swing.JLabel jLabel4;
    private javax.swing.JLabel jLabel5;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JScrollPane jScrollPane2;
    private javax.swing.JSplitPane jSplitPane1;
    private javax.swing.JButton nextButton;
    private javax.swing.JButton previousButton;
    // End of variables declaration//GEN-END:variables
    /**
     *
     */
    class VariableMappingTableModel extends AbstractTableModel {

        private static final long serialVersionUID = 1L;
        private String[] columnNames = {"TreeScan Variable", "Source File Variable"};
        private final Vector<ImportVariable> variables_static;
        private Vector<ImportVariable> variables_visible;
        public JComboBox comboBox = new JComboBox();

        public VariableMappingTableModel(Vector<ImportVariable> variables) {
            this.variables_static = variables;
            variables_visible = (Vector) this.variables_static.clone();
        }

        public int getColumnCount() {
            return columnNames.length;
        }

        public int getRowCount() {
            return variables_visible.size();
        }

        @Override
        public Class getColumnClass(int c) {
            return getValueAt(0, c).getClass();
        }

        @Override
        public String getColumnName(int col) {
            return columnNames[col];
        }

        public Object getValueAt(int row, int col) {
            if (col == 0) {
                return variables_visible.get(row).getVariableDisplayName();
            } else {
                Object obj = comboBox.getItemAt(variables_visible.get(row).getInputFileVariableIndex());
                if (obj != null) {
                    return obj;
                } else {
                    return "";
                }
            }
        }

        @Override
        public boolean isCellEditable(int row, int col) {
            return col == 0 ? false : true;
        }

        public void setShowing(final ImportVariable variable, boolean showing) {
            if (showing) {
                showVariable(variable.getVariableName());
            } else {
                hideVariable(variable.getVariableName());
            }
        }

        public boolean isShowing(final String variableName) {
            for (int i = 0; i < variables_visible.size(); ++i) {
                if (variableName.equals(variables_visible.get(i).getVariableName())) {
                    return true;
                }
            }
            return false;
        }

        public void showVariable(final String variableName) {
            //first search to see if aleady showing
            for (int i = 0; i < variables_visible.size(); ++i) {
                if (variableName.equals(variables_visible.get(i).getVariableName())) {
                    return; //already showing
                }
            }
            //find index in variables vector
            for (int i = 0; i < variables_static.size(); ++i) {
                if (variableName.equals(variables_static.get(i).getVariableName())) {
                    variables_visible.add(variables_static.get(i));
                    return; //already showing
                }
            }

        }

        public void hideVariable(final String variableName) {
            for (int i = 0; i < variables_visible.size(); ++i) {
                if (variableName.equals(variables_visible.get(i).getVariableName())) {
                    variables_visible.remove(i);
                    return;
                }
            }
        }

        public void hideAll() {
            variables_visible.clear();
        }        
        
        @Override
        public void setValueAt(Object value, int row, int col) {
            if (value == null) {
                return;
            }
            for (int i = 0; i < comboBox.getItemCount(); ++i) {
                if (comboBox.getItemAt(i).equals(value)) {
                    variables_visible.get(row).setInputFileVariableIndex(i);
                    enableNavigationButtons();
                    fireTableCellUpdated(row, col);
                    return;
                }
            }
        }
    }

    /*
     * Import worker; executed in background thread.
     */
    class ImportTask extends SwingWorker<Void, Void> implements ActionListener {

        private WaitCursor waitCursor = new WaitCursor(ImportWizardDialog.this);
        private FileImporter _importer;

        @Override
        public Void doInBackground() {
            try {
                cancelButton.addActionListener(this);
                previousButton.setEnabled(false);
                executeButton.setEnabled(false);
                _progressBar.setVisible(true);
                _progressBar.setValue(0);
                VariableMappingTableModel model = (VariableMappingTableModel) _fieldMapTable.getModel();
                _importer = new FileImporter(getImportSource(),
                        model.variables_static,
                        _fileType,
                        _sourceDataFileType,
                        _destinationFile,
                        _progressBar);
                _importer.importFile(Integer.parseInt(_ignoreRowsTextField.getText()));
                _cancelled = _importer.getCancelled();
                setVisible(false);
            } catch (IOException e) {
                JOptionPane.showMessageDialog(ImportWizardDialog.this,
                        "The import wizard encountered an error attempting to create the import file.\n" +
                        "This is most likely occuring because write permissions are not granted for\n" +
                        "specified directory. Please check path or review user permissions for specified directory.\n", "Note", JOptionPane.WARNING_MESSAGE);
            } catch (SecurityException e) {
                JOptionPane.showMessageDialog(ImportWizardDialog.this,
                        "The import wizard encountered an error attempting to create the import file.\n" +
                        "This is most likely occuring because write permissions are not granted for\n" +
                        "specified directory. Please check path or review user permissions for specified directory.\n", "Note", JOptionPane.WARNING_MESSAGE);
            } catch (ImportException e) {
                JOptionPane.showMessageDialog(ImportWizardDialog.this, e.getMessage(), "Note", JOptionPane.ERROR_MESSAGE);
            } catch (Throwable t) {
                new ExceptionDialog(ImportWizardDialog.this, t).setVisible(true);
            } finally {
                cancelButton.removeActionListener(this);
            }
            return null;
        }

        /*
         * Executed in event dispatching thread
         */
        @Override
        public void done() {
            executeButton.setEnabled(true);
            previousButton.setEnabled(true);
            waitCursor.restore();
        }

        public void actionPerformed(ActionEvent e) {
            _importer.setCancelled();
        }
    }
}
