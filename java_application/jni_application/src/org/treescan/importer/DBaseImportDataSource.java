
package org.treescan.importer;

import com.linuxense.javadbf.DBFDataType;
import com.linuxense.javadbf.DBFException;
import com.linuxense.javadbf.DBFReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.GregorianCalendar;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * DBaseImportDataSource acts as a data source for the Importer by reading a dBase file.
 */
public class DBaseImportDataSource implements ImportDataSource {

    private final File _source_file;
    InputStream _input_stream=null;
    private final DBFReader _reader;
    private int _current_row = 0;
    private boolean _format_dates;
    private final String DATE_FORMAT = "EEE MMM dd HH:mm:ss z yyyy"; //"Fri Aug 30 00:00:00 EDT 2002"
    private ArrayList<Object> _column_names = new ArrayList<>();

    /** Creates a new instance of DBaseImportDataSource */
    public DBaseImportDataSource(File file, boolean formatDates) {
        _source_file = file;
        _format_dates = formatDates;
        try {
            _input_stream = new FileInputStream(_source_file);
            _reader = new DBFReader(_input_stream);
            _column_names.add("One Count");
            for (int i=0; i < _reader.getFieldCount(); ++i) {
                String name = _reader.getField(i).getName();
                _column_names.add(name.isEmpty() ? ("Column " + (i + 1)) : name);
            }            
        } catch (FileNotFoundException e) {
            throw new RuntimeException(e.getMessage(), e);
        }
    }

    @Override
    public void close() {        
        try {
            if (_input_stream != null) {_input_stream.close(); _input_stream=null;}
        } catch (IOException ex) {
            Logger.getLogger(XLSImportDataSource.class.getName()).log(Level.SEVERE, null, ex);
        }
    }   
    
    /**
     * Returns number of records in file.
     */
    @Override
    public int getNumRecords() {
        return _reader.getRecordCount();
    }

    /**
     * Returns whether column at index is a date field.
     */
    @Override
    public boolean isColumnDate(int iColumn) {
        if (iColumn <= 0) return false; // oneCount column is not date field
        try {
            return _reader.getField(iColumn).getType() == DBFDataType.DATE;
        } catch (DBFException e) {
            throw new RuntimeException(e.getMessage(), e);
        }
    }

    /** Returns whether column at index is a numeric field. */
    public boolean isColumnNumeric(int iColumn) {
        if (iColumn <= 0) return false; // // oneCount column is not numeric field
        try {
            return _reader.getField(iColumn).getType() == DBFDataType.NUMERIC;
        } catch (DBFException e) {
            throw new RuntimeException(e.getMessage(), e);
        }
    }

    /** Returns current row index. */
    @Override
    public long getCurrentRecordNum() {
        return _current_row;
    }

    /** Returns array of objects that define the column names of table. */
    @Override
    public Object[] getColumnNames() {
        return _column_names.toArray();
    }

    @Override
    public int getColumnIndex(String name) {
        for (int i=0; i < _column_names.size(); ++i) {
            String column_name = (String)_column_names.get(i);
            if (column_name.equals(name)) {
                return i + 1;
            }
        } return 0;
    }
    
    /** Advances row index and reads data into object array. Returns array of objects if not end of file, otherwise returns null. */
    @Override
    public Object[] readRow() {
        ArrayList<Object> values = new ArrayList<>();
        try {
            Object[] record = _reader.nextRecord();
            if (record == null) {
                return null;
            }
            values.add("1");
            for (int i=0; i < record.length; ++i) {
                if (record[i] == null) { // replace null values with empty string
                    values.add("");
                } else if (isColumnDate(i + 2)) {
                    // reformat date to format that we want either YYYYMMDD or YYYY/MM/DD
                    GregorianCalendar date = new GregorianCalendar();
                    date.setTime(new SimpleDateFormat(DATE_FORMAT).parse(record[i].toString()));
                    StringBuilder str = new StringBuilder();
                    str.append(date.get(GregorianCalendar.YEAR));
                    if (_format_dates) {
                        str.append("/");
                    }
                    int month = date.get(GregorianCalendar.MONTH);
                    if (month < 9) {
                        str.append("0");
                    }
                    str.append(month + 1);
                    if (_format_dates) {
                        str.append("/");
                    }
                    int day = date.get(GregorianCalendar.DAY_OF_MONTH);
                    if (day < 10) {
                        str.append("0");
                    }
                    str.append(day);
                    values.add(str.toString());
                } else if (isColumnNumeric(i + 2)) {
                    // reformat numeric fields with zero precision to have no decimal
                    String value = record[i].toString();
                    if (_reader.getField(i).getDecimalCount() == 0 && value.indexOf('.') != -1) {
                        value = value.substring(0, value.indexOf('.'));
                    }
                    values.add(value);
                } else { // otherwise, accept value as it is
                    values.add(record[i].toString());
                }
            }
            ++_current_row;
        } catch (ParseException | DBFException e) {
            throw new RuntimeException(e.getMessage(), e);
        }
        return values.toArray();
    }
}
