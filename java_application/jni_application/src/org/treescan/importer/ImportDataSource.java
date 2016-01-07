/*
 * ImportDataSource.java
 *
 * Created on December 14, 2007, 2:27 PM
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */
package org.treescan.importer;

/**
 *
 * @author Hostovic
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
