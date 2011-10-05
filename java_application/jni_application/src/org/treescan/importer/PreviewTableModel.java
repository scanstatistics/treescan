package org.treescan.importer;

import java.util.ArrayList;
import javax.swing.table.AbstractTableModel;

/**
 * Extends AbstractTableModel to implement a preview table model.
 */
public class PreviewTableModel extends AbstractTableModel {

    private static final long serialVersionUID = 1L;
    public static final int DEFAULT_PREVIEW_LENGTH = 50;
    private int _previewLength = DEFAULT_PREVIEW_LENGTH;
    protected ArrayList<Object[]> _previewData = new ArrayList<Object[]>();
    protected int _maxFieldCount = 0;
    protected final boolean _useHeader;

    /**
     * Constructs a new PreviewTableModel object.
     */
    public PreviewTableModel(boolean useHeader) {
        super();
        _useHeader = useHeader;
    }

    /**
     * Returns the number of columns in table.
     */
    public int getColumnCount() {
        return _maxFieldCount;
    }

    /**
     * Returns the maximum number of rows displayed by preview.
     */
    public int getPreviewLength() {
        return _previewLength;
    }

    /**
     * Set the maximum number of rows displayed by preview.
     */
    public void setPreviewLength(int previewLength) {
        _previewLength = previewLength;
    }

    /**
     * Returns the number of rows in table excluding header row (if defined).
     */
    public int getRowCount() {
        int count = _previewData.size();
        if (_useHeader) {
            count--;
        }
        return count > 0 ? count : 0;
    }

    /**
     * Returns object at table row/column.
     */
    public Object getValueAt(int row, int col) {
        Object[] rowData = _previewData.get(row + (_useHeader ? 1 : 0));
        if (col < rowData.length) {
            return rowData[col];
        } else {
            return "";
        }
    }

    /**
     * Returns the name of column at index. If a header row is not defined,
     * returns "Column x" as name.
     */
    @Override
    public String getColumnName(int col) {
        Object[] rowData = _previewData.get(0);
        if (_useHeader && rowData != null && col < rowData.length && rowData[col] != null) {
            return rowData[col].toString();
        }
        return "Column " + (col + 1);
    }

    /**
     * Adds row to table cache and fires table date change event.
     */
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
