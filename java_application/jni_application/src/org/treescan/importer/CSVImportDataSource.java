package org.treescan.importer;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PushbackInputStream;
import java.io.Reader;
import java.util.ArrayList;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * CSVImportDataSource acts as a data source for the Importer by reading from a text file.
 * The constructor parameters dictate how a file lines are to be parsed.
 */
public class CSVImportDataSource implements ImportDataSource {
    protected File _source_file;
    protected PushbackInputStream _pushback_stream;
    protected char _row_delimiter='\n';
    protected char _column_delimiter=',';
    protected char _group_delimiter='"';
    private int _skip=0;
    private final int SAMPLE_ROWS = 200;
    private int _current_row=0;
    private int _total_rows=0;
    private boolean _has_header=false;
    private final ArrayList<Object> _column_names = new ArrayList<>();

    public CSVImportDataSource(File file, boolean hasHeader, char rowDelimiter, char colDelimiter, char groupDelimiter, int skip) throws FileNotFoundException {
        _source_file = file;
        setInputStream();
        _has_header = hasHeader;
        _row_delimiter = rowDelimiter;
        _column_delimiter = colDelimiter;
        _group_delimiter = groupDelimiter;
        _skip = skip;
        _total_rows = countLines(_source_file);
        _column_names.add("One Count");
        if (_has_header) {
            Object[] row = readRow();
            for (int i=1; i < row.length; ++i)
                _column_names.add(row[i]);
            _skip += 1;
        } else {
            int sample_count = 0;
            int maxCols = 0;
            Object[] row = readRow();
            while (row != null && sample_count < SAMPLE_ROWS) {
                sample_count++;
                maxCols = Math.max(maxCols, row.length);
                row = readRow();
            }
            for (int i=1; i <= maxCols; ++i) {
                _column_names.add("Column " + i);
            }
            setInputStream(); // re-open the input stream
        }
    }

    @Override
    public void close() {        
        try {
            if (_pushback_stream != null) {_pushback_stream.close(); _pushback_stream=null;}
        } catch (IOException ex) {
            Logger.getLogger(XLSImportDataSource.class.getName()).log(Level.SEVERE, null, ex);
        }
    }
    
    /** 
     * @return number of records in file
     */
    @Override
    public int getNumRecords() {
        return _total_rows;
    }

    /** 
     * @return column names
     */
    @Override
    public Object[] getColumnNames() {
        return _column_names.toArray();
    }
    
    /**
     * @param columnname - name of column
     * @return index of column, returns 0 if not found
     */
    @Override
    public int getColumnIndex(String columnname) {
        for (int i=0; i < _column_names.size(); ++i) {
            String column_name = (String)_column_names.get(i);
            if (column_name.equals(columnname)) {
                return i + 1;
            }
        } return 0;
    }    
    
    /**
     * Determines the number of lines in data file.
     * @param file
     * @return number of lines in file
     */
    public int countLines(File file) {
        int lineCount = 0;
        try {
            Reader reader = new InputStreamReader(new FileInputStream(file));
            char[] buffer = new char[4096];
            for (int charsRead = reader.read(buffer); charsRead >= 0; charsRead = reader.read(buffer)) {
                for (int charIndex = 0; charIndex < charsRead; charIndex++) {
                    if (buffer[charIndex] == _row_delimiter) {
                        lineCount++;
                    }
                }
            }
            reader.close();
        } catch (IOException e) {
            throw new RuntimeException(e.getMessage(), e);
        }
        return lineCount;
    }

    /**
     * Reads row from source file and parses into collection.
     * @return collection of strings parsed from read record
     */
    @Override
    public Object[] readRow() {
        String line = null;
        // Skip rows as necessary.
        try {
            while (_current_row < _skip) {
                line = readLine();
                if (line == null) return null;
                _current_row++;
            }
        } catch (IOException ex) {
            return null;
        }
        // Read next row -- skipping blank records.
        ArrayList row = new ArrayList();
        do {
            try {
                line = readLine();
                if (line == null) return null;
                row = ImportUtils.parseLine(line, Character.toString(_column_delimiter), Character.toString(_group_delimiter));
                _current_row++;
            } catch (IOException ex) {
                return null;
            }
        } while (row.isEmpty());
        if (line == null) return null;
        // Tack on the generated columns.
        row.add(0, "1");        
        return row.toArray();
    }

    /**
     * Assigns PushbackInputStream object for file read.
     */
    private void setInputStream() {
        try {
            if (_pushback_stream != null) {
                _pushback_stream.close();
            }
            _pushback_stream = new PushbackInputStream(new FileInputStream(_source_file));
        } catch (IOException e) {
            _pushback_stream = null;
        }
        _current_row = 0;
    }

    /**
     * reads line from source stream
     * @return a String representing an unparsed version of the next row.
     * @throws IOException if there is a read error or if the EOF is reached.
     */
    private String readLine() throws IOException {
        //if gStream is null no line can be read
        if (_pushback_stream == null) {
            throw new IOException("Null Stream");
        }

        StringBuilder line = new StringBuilder();
        int c = _pushback_stream.read();
        if (c < 0) {
            return null;
        }//throw new EOFException();
        while (c >= 0) {
            if (c == _row_delimiter) {
                break;
            }
            if (c == '\r') {
                // peek at next character, to see if it is delimiter
                c = _pushback_stream.read();
                if (c != _row_delimiter) {
                    _pushback_stream.unread(c);
                }
                break;
            } else {
                line.append((char) c);
            }
            c = _pushback_stream.read();
        }
        return line.toString();
    }

    /**
     * @return returns where column is date -- always false
     */
    @Override
    public boolean isColumnDate(int iColumn) {
        return false;
    }

    /**
     * @return current data record row number
     */
    @Override
    public long getCurrentRecordNum() {
        return _current_row;
    }
}
