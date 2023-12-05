package org.treescan.importer;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
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
        // create record template which is all import variables
        ArrayList<String> record_template = new ArrayList<>(); 
        // create sub-set collection of ImportVariable which were selected as import columns
        ArrayList<ImportVariable> mappedVariables = new ArrayList<>();
        for (ImportVariable variable: _importVariables) {
            record_template.add(null);
            if (variable.isMappedToSourceField() || variable.hasDefault()) {
                mappedVariables.add(variable);
            }
        }
        _progress.setMaximum(_dataSource.getNumRecords()); // initialize progress
        int iRow=0;
        String value;
        Object[] values = _dataSource.readRow(); // read first row of data
        FileWriter writer = new FileWriter(_destinationFile); // open file writer - overwriting contents
        BufferedWriter buffer = new BufferedWriter(writer);
        // Start processing and writing records. Stop when data source is exhausted or cancel flag is tripped.
        while (values != null && !_cancelled) {
            ArrayList<String> record = (ArrayList<String>)record_template.clone();
            // for each mapped variable, set value in cooresponding spot of record.
            for (ImportVariable variable: mappedVariables) {
                int iColumn = variable.getSourceFieldIndex(); // source field index is 1 based
                if (iColumn > values.length)
                    // If the source column index is greater than number of values in data source record, set to empty string or default.
                    value = variable.hasDefault() ? variable.getDefault() : "";
                else
                    value = (String)values[iColumn - 1];
                value = StringUtils.trimToEmpty(value);
                record.set(variable.getVariableIndex(), StringUtils.contains(value, _delimiter) ? _groupmarker + value + _groupmarker : value);
            }
            // Remove columns from record which did not receive an import value. 
            record.removeAll(Collections.singleton(null));
            // Add record values to write buffer - skipping those with all empty strings.
            if (Collections.frequency(Arrays.asList(record.toArray()), "") != record.size()) {
                buffer.write(StringUtils.join(record.toArray(), _delimiter));
                buffer.newLine();
            }
            values = _dataSource.readRow();
            _progress.setValue(++iRow);
        }
        buffer.close();
        writer.close();
    }
}
