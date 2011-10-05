/*
 * DBaseImportDataSource.java
 *
 * Created on December 20, 2007, 9:41 AM
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */
package org.treescan.importer;

import com.linuxense.javadbf.DBFException;
import com.linuxense.javadbf.DBFField;
import com.linuxense.javadbf.DBFReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.InputStream;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.GregorianCalendar;
import java.util.Vector;
import org.treescan.app.*;

/**
 *
 * @author Hostovic
 */
public class DBaseImportDataSource implements ImportDataSource {

    private final File _sourceFile;
    private final DBFReader _reader;
    private int _currentRowNumber = 0;
    private boolean _formatDates;
    private final String _libDateFormat = "EEE MMM dd HH:mm:ss z yyyy"; //"Fri Aug 30 00:00:00 EDT 2002"

    /** Creates a new instance of DBaseImportDataSource */
    public DBaseImportDataSource(File file, boolean formatDates) {
        _sourceFile = file;
        _formatDates = formatDates;

        try {
            InputStream inputStream = new FileInputStream(_sourceFile);
            _reader = new DBFReader(inputStream);
        } catch (Exception e) {
            throw new RuntimeException(e.getMessage(), e);
        }
    }

    /**
     * Returns number of records in file.
     */
    public int getNumRecords() {
        return _reader.getRecordCount();
    }

    /**
     * Returns whether column at index is a date field.
     */
    public boolean isColumnDate(int iColumn) {
        try {
            return _reader.getField(iColumn).getDataType() == DBFField.FIELD_TYPE_D;
        } catch (DBFException e) {
            throw new RuntimeException(e.getMessage(), e);
        }
    }

    /**
     * Returns whether column at index is a numeric field.
     */
    public boolean isColumnNumeric(int iColumn) {
        try {
            return _reader.getField(iColumn).getDataType() == DBFField.FIELD_TYPE_N;
        } catch (DBFException e) {
            throw new RuntimeException(e.getMessage(), e);
        }
    }

    /**
     * Returns current row index.
     */
    public long getCurrentRecordNum() {
        return _currentRowNumber;
    }

    /**
     * Returns array of objects that define the column names of table.
     */
    public Object[] getColumnNames() {
        Vector<Object> names = new Vector<Object>();
        try {
            for (int i = 0; i < _reader.getFieldCount(); ++i) {
                names.add(_reader.getField(i).getName());
            }
        } catch (DBFException e) {
            throw new RuntimeException(e.getMessage(), e);
        }
        return names.toArray();
    }

    /**
     * Advances row index and reads data into object array. Returns array of objects
     * if not end of file, otherwise returns null.
     */
    public Object[] readRow() {
        Vector<Object> values = new Vector<Object>();
        try {
            Object[] record = _reader.nextRecord();
            if (record == null) {
                return null;
            }
            for (int i = 0; i < record.length; ++i) {
                if (record[i] == null) //replace null values with empty string
                {
                    values.add("");
                } else if (isColumnDate(i)) {
                    //reformat date to format that we want either YYYYMMDD or YYYY/MM/DD
                    GregorianCalendar date = new GregorianCalendar();
                    date.setTime(new SimpleDateFormat(_libDateFormat).parse(record[i].toString()));
                    StringBuilder str = new StringBuilder();
                    str.append(date.get(GregorianCalendar.YEAR));
                    if (_formatDates) {
                        str.append("/");
                    }
                    int month = date.get(GregorianCalendar.MONTH);
                    if (month < 9) {
                        str.append("0");
                    }
                    str.append(month + 1);
                    if (_formatDates) {
                        str.append("/");
                    }
                    int day = date.get(GregorianCalendar.DAY_OF_MONTH);
                    if (day < 10) {
                        str.append("0");
                    }
                    str.append(day);
                    values.add(str.toString());
                } else if (isColumnNumeric(i)) {
                    //reformat numeric fields with zero precision to have no decimal
                    String value = record[i].toString();
                    if (_reader.getField(i).getDecimalCount() == 0 && value.indexOf('.') != -1) {
                        value = value.substring(0, value.indexOf('.'));
                    }
                    values.add(value);
                } else //otherwise, accept value as it is
                {
                    values.add(record[i].toString());
                }
            }
            ++_currentRowNumber;
        } catch (ParseException e) {
            throw new RuntimeException(e.getMessage(), e);
        } catch (DBFException e) {
            throw new RuntimeException(e.getMessage(), e);
        }
        return values.toArray();
    }
}
