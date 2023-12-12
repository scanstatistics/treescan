package org.treescan.importer;

import java.util.ArrayList;
import java.util.Arrays;
import javax.swing.table.AbstractTableModel;
import org.treescan.gui.FileSourceWizard;

/**
 * Extends AbstractTableModel to implement a preview table model.
 */
public class PreviewTableModel extends AbstractTableModel {

    private static final long serialVersionUID = 1L;
    public static final int DEFAULT_PREVIEW_LENGTH = 50;
    private int _previewLength = DEFAULT_PREVIEW_LENGTH;
    protected ArrayList<Object[]> _previewData = new ArrayList<>();
    protected int _maxFieldCount = 0;
    protected final ImportDataSource _data_source;
    ArrayList<Object> _column_names = new ArrayList<>();

    /** Constructs a new PreviewTableModel object. */
    public PreviewTableModel(ImportDataSource data_source) {
        super();
        _data_source = data_source;
        _column_names = new ArrayList(Arrays.asList(_data_source.getColumnNames()));
        for (int i=0; i < getPreviewLength(); ++i) {
            Object[] values = _data_source.readRow();
            if (values != null) {
                addRow(values);
            }
        }        
    }

    public void close() {
        _data_source.close();
    }
    
    /** Returns the number of columns in table. */
    public int getColumnCount() {
        return _maxFieldCount;
    }

    /** Returns the maximum number of rows displayed by preview. */
    public int getPreviewLength() {
        return _previewLength;
    }

    /** Set the maximum number of rows displayed by preview. */
    public void setPreviewLength(int previewLength) {
        _previewLength = previewLength;
    }

    /** Returns the number of rows in table excluding header row (if defined). */
    public int getRowCount() {
        return _previewData.size();
    }

    /** Returns object at table row/column. */
    @Override
    public Object getValueAt(int row, int col) {
        Object[] rowData = _previewData.get(row);
        if (col < rowData.length) {
            return rowData[col];
        } else {
            return "";
        }
    }

    public String getNonSuffixedColumnName(int idx) {
        if (idx < _column_names.size()) {
           return (String)_column_names.get(idx);
        } else {
           return FileSourceWizard._unassigned_variable; 
        }        
    }
    
    /** Returns the name of column at index. If a header row is not defined, returns "Column x" as name. */
    @Override
    public String getColumnName(int idx) {
        return getNonSuffixedColumnName(idx);
    }

    /** Returns the number of columns in data source. */
    public int getDataSourceColumnNameCount() {
        return _data_source.getColumnNames().length;
    }
    
    /** Returns the data source column name at index. */
    public String getDataSourceColumnName(int colIdx) {
        Object[] names = _data_source.getColumnNames();
        return colIdx < names.length ? (String)_data_source.getColumnNames()[colIdx] : "";
    }
    
    /** Returns the data source column index that matches name. */
    public int getDataSourceColumnIndex(String name) {
        return _data_source.getColumnIndex(name);
    }
    
    /** Adds row to table cache and fires table date change event. */
    public void addRow(Object[] row) {
        if (row != null) {
            _previewData.add(row);
            if (row.length > _maxFieldCount) {
                _maxFieldCount = row.length;
            }
            fireTableDataChanged();
        }
    }
}
