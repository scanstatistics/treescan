package org.treescan.importer;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import javax.swing.JProgressBar;
import org.apache.commons.lang3.StringUtils;

/**
 * Imports from data source to TreeScan formatted data file.
 * @author hostovic
 */
public class FileImporter {
    private final File _destinationFile;
    private final ImportDataSource _dataSource;
    private final ArrayList<ImportVariable> _importVariables;
    private final JProgressBar _progress;
    private boolean _cancelled=false;
    private final String _delimiter=",";
    private final String _groupmarker="\"";
    
    /**
     * 
     * @param dataSource - import data source
     * @param importVariables - collection of all import variables
     * @param destinationFile - file where import records will be written.
     * @param progress - JProgressBar which is updated as import progresses.
     */
    public FileImporter(ImportDataSource dataSource, ArrayList<ImportVariable> importVariables, File destinationFile, JProgressBar progress) {
        _dataSource = dataSource;
        _importVariables = importVariables;
        _destinationFile = destinationFile;
        _progress = progress;
    }
    
    /**
     * Returns whether the canceled flag is true or false.
     * @return boolean
     */
    public boolean getCancelled() {
        return _cancelled;
    }
    
    /**
     * Sets the canceled flag as true.
     */
    public void setCancelled() {
        _cancelled = true;
    }

    /**
     * Imports data source records into destination file as CSV records using
     * ImportVariable collection to map data source fields to destination fields.
     * @throws IOException
     * @throws SecurityException 
     */
    public void importFile() throws IOException, SecurityException {
        /* Create temporary collection of import variables which are imported to from data source.
           Also allocation record buffer which will store field values until record is written to file. */
        ArrayList<String> record = new ArrayList<String>();
        ArrayList<ImportVariable> mappedVariables = new ArrayList<ImportVariable>();
        for (ImportVariable variable: _importVariables) {
            if (variable.isMappedToSourceField() || variable.hasDefault()) {
                mappedVariables.add(variable);
                record.add(new String());
            }
        }
        /* initialize progress */
        _progress.setMaximum(_dataSource.getNumRecords());
        int iRow=0;
        String value;
        Object[] values = _dataSource.readRow();
        /* Open file writer - overwrite contents. */
        FileWriter writer = new FileWriter(_destinationFile);
        BufferedWriter buffer = new BufferedWriter(writer);
        /* Start processing and writing records. Stop when data source is exhausted or cancel flag is tripped. */
        while (values != null && !_cancelled) {
            for (ImportVariable variable: mappedVariables) {
                /* get the zero based column index from mapping variables */
                int iColumn = variable.getSourceFieldIndex() - 1;
                /* If the source column index is greater than number of values in data source record, default to empty string. */
                value = StringUtils.trimToEmpty(iColumn + 1 > values.length ? "" :(String)values[iColumn]);
                /* Add value to record buffer -- taken grouping character into account. */
                record.set(variable.getVariableIndex(), StringUtils.contains(value, _delimiter) ? _groupmarker + value + _groupmarker : value);
            }
            buffer.write(StringUtils.join(record.toArray(), _delimiter));
            buffer.newLine();
            values = _dataSource.readRow();
            _progress.setValue(++iRow);
        }
        buffer.close();
        writer.close();
    }
}
