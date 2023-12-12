package org.treescan.importer;

/**
 * Defines file wizard data source interface.
 */
public interface ImportDataSource {
    public class UnsupportedException extends RuntimeException {
        public UnsupportedException(String message, Throwable cause) {
            super(message, cause);
        }
    }
    
    public boolean isColumnDate(int iColumn);

    public long getCurrentRecordNum();

    public Object[] getColumnNames();
    
    public int getColumnIndex(String name);
    
    public int getNumRecords();

    public Object[] readRow();
    
    public void close();
}
