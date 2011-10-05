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

    public boolean isColumnDate(int iColumn);

    public long getCurrentRecordNum();

    public int getNumRecords();

    public Object[] readRow();
}
