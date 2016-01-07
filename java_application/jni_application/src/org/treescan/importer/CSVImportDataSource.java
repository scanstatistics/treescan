package org.treescan.importer;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;
import java.util.ArrayList;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * CSVImportDataSource acts as a data source for the Importer by reading
 * from a text file.  The parameters which dictate how a file is
 * interpreted are the delimiters for: row, column, and group. 
 */
public class CSVImportDataSource implements ImportDataSource {
    protected File _sourceFile;
    protected InputStream _inputStream=null;
    protected char _rowDelimiter='\n';
    protected char _colDelimiter=',';
    protected char _groupDelimiter='"';
    private int _skip=0;
    private int _currentRowNumber=0;
    private int _totalRows=0;
    private boolean _hasHeader=false;
    private ArrayList<Object> _column_names;

    public CSVImportDataSource(File file, boolean hasHeader, char rowDelimiter, char colDelimiter, char groupDelimiter, int skip) throws FileNotFoundException {
        _sourceFile = file;
        _inputStream = new FileInputStream(_sourceFile);
        _hasHeader = hasHeader;
        _rowDelimiter = rowDelimiter;
        _colDelimiter = colDelimiter;
        _groupDelimiter = groupDelimiter;
        _skip = skip;
        _column_names = new ArrayList<Object>();
        _totalRows = countLines(_sourceFile);
        if (_hasHeader) {
            Object[] row = readRow();
            for (int i=0; i < row.length; ++i)
                _column_names.add(row[i]);
            _skip += 1;
        } else {
            int sample_count = 0;
            int maxCols = 0;
            Object[] row = readRow();
            while (row != null && sample_count < 200) {
                sample_count++;
                maxCols = Math.max(maxCols, row.length);
                row = readRow();
            }
            for (int i=1; i <= maxCols; ++i) {
                _column_names.add("Column " + i);
            }
            reset();            
        }
    }

    public void close() {        
        try {
            if (_inputStream != null) {_inputStream.close(); _inputStream=null;}
        } catch (IOException ex) {
            Logger.getLogger(XLSImportDataSource.class.getName()).log(Level.SEVERE, null, ex);
        }
    }
    
    /** 
     * @return number of records in file
     */
    @Override
    public int getNumRecords() {
        return _totalRows;
    }

    /** 
     * @return Returns column names
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
                    if (buffer[charIndex] == _rowDelimiter) {
                        lineCount++;
                    }
                }
            }
            reader.close();
        } catch (Exception e) {
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

        try {
            while (_currentRowNumber < _skip) {
                line = readLine();
                if (line == null) break;
                _currentRowNumber++;
            }
            line = readLine();
        } catch (IOException ex) {
            ex.printStackTrace();
        }
        if (line == null) {
            return null;
        }
        ArrayList row = ImportUtils.parseLine(line, Character.toString(_colDelimiter), Character.toString(_groupDelimiter));
        _currentRowNumber++;
        return row.toArray();
    }

    /**
     * Resets the input source stream to beginning of file.
     */
    public void reset() {
        try {
            if (_inputStream != null) {
                _inputStream.close();
            }
            _inputStream = new FileInputStream(_sourceFile);
        } catch (IOException e) {
            _inputStream = null;
        }
        _currentRowNumber = 0;
    }

    /**
     * reads line from source stream
     * @return a String representing an unparsed version of the next row.
     * @throws IOException if there is a read error or if the EOF is reached.
     */
    private String readLine() throws IOException {
        //if gStream is null no line can be read
        if (_inputStream == null) {
            throw new IOException("Null Stream");
        }

        StringBuilder line = new StringBuilder();
        int c = _inputStream.read();
        if (c < 0) {
            return null;
        }//throw new EOFException();
        while ((c != _rowDelimiter) && (c >= 0)) {
            if ((char)c != '\r') {
                line.append((char) c);
            }
            c = _inputStream.read();
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
        return _currentRowNumber;
    }
}
