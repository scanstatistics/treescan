package org.treescan.importer;

import java.io.*;
import java.util.*;

/**
 * CSVImportDataSource acts as a data source for the Importer by reading
 * from a text file.  The parameters which dictate how a file is
 * interpreted are the delimiters for: row, column, and group.
 *
 *
 * @author watsond
 */
public class CSVImportDataSource implements ImportDataSource {

    protected File _sourceFile;
    protected InputStream _inputStream;
    protected char _rowDelimiter;
    protected char _colDelimiter;
    protected char _groupDelimiter;
    private int _currentRowNumber = 0;
    private int _totalRows = 0;
    private boolean _hasHeader;

    public CSVImportDataSource(File file, boolean hasHeader, char rowDelimiter, char colDelimiter, char groupDelimiter) throws FileNotFoundException {
        _sourceFile = file;
        _totalRows = countLines(_sourceFile);
        _inputStream = new FileInputStream(_sourceFile);
        /*Set defaults*/
        _hasHeader = hasHeader;
        _rowDelimiter = rowDelimiter;
        _colDelimiter = colDelimiter;
        _groupDelimiter = groupDelimiter;
    }

    /**
     * Returns number of records in file.
     */
    public int getNumRecords() {
        return _totalRows;
    }

    public static int countLines(File file) {
        int lineCount = 0;
        try {
            Reader reader = new InputStreamReader(new FileInputStream(file));

            char[] buffer = new char[4096];
            for (int charsRead = reader.read(buffer); charsRead >= 0; charsRead = reader.read(buffer)) {
                for (int charIndex = 0; charIndex < charsRead; charIndex++) {
                    if (buffer[charIndex] == '\n') {
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

    public void close() {
        try {
            if (_inputStream != null) {
                _inputStream.close();
            }
        } catch (IOException ex) {
        }
    }

    public String getAbsolutePath() {
        return _sourceFile.getAbsolutePath();
    }

    /* (non-Javadoc)
     * @see bsi.client.table.importer.ImportDataSource#readRow()
     */
    public Object[] readRow() {
        String line = null;

        try {
            line = readLine();
            if (_hasHeader && _currentRowNumber == 0 && line != null) {
                line = readLine();
            }
        } catch (IOException ex) {
            ex.printStackTrace();
        }
        if (line == null) {
            return null;
        }
        _currentRowNumber++;
        Vector row = ImportUtils.parseLine(line, Character.toString(_colDelimiter), Character.toString(_groupDelimiter));
        return row.toArray();
    }

    /* (non-Javadoc)
     * @see bsi.client.table.importer.ImportDataSource#reset()
     */
    public void reset() throws IOException {
        try {
            if (_inputStream != null) {
                _inputStream.close();
            }
            _inputStream = new FileInputStream(_sourceFile);
        } catch (IOException e) {
            _inputStream = null;
            throw e;
        }
        _currentRowNumber = 0;
    }

    /* (non-Javadoc)
     * @see bsi.client.table.importer.ImportDataSource#getCurrentRow()
     */
    public int getCurrentRow() {
        return _currentRowNumber;
    }

    /**
     * @return a String representing an unparsed version of the next row.
     * @throws IOException if there is a read error or if the EOF is reached.
     */
    private String readLine() throws IOException {
        //if gStream is null no line can be read
        if (_inputStream == null) {
            throw new IOException("Null Stream");
        }

        StringBuffer line = new StringBuffer();
        int c = _inputStream.read();
        if (c < 0) {
            return null;
        }//throw new EOFException();
        while ((c != _rowDelimiter) && (c >= 0)) {
            if (c != '\r') {
                line.append((char) c);
            }
            c = _inputStream.read();
        }
        return line.toString();
    }

    public boolean isColumnDate(int iColumn) {
        return false;
    }

    public long getCurrentRecordNum() {
        return _currentRowNumber;
    }
}
