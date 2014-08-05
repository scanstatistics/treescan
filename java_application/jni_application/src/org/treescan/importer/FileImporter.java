package org.treescan.importer;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.AbstractCollection;
import java.util.Iterator;
import java.util.Vector;
import javax.swing.JProgressBar;
import org.apache.commons.lang.StringUtils;
import org.treescan.app.*;

/**
 * Imports from data source to TreeScan formatted data file.
 */
public class FileImporter {
    public enum InputFileType      {Tree, Cuts, Case};
    public enum SourceDataFileType {Delimited, dBase, Excel};
    
    private final InputFileType _fileType;
    private final SourceDataFileType _sourceDataFileType;
    private final File _destinationFile;
    private final ImportDataSource _dataSource;
    private final Vector<ImportVariable> _importVariables;
    private final JProgressBar _progress;
    private boolean _cancelled=false;
    
    public FileImporter(ImportDataSource dataSource, Vector<ImportVariable> importVariables, InputFileType fileType, SourceDataFileType sourceDataFileType, File destinationFile, JProgressBar progress) {
        _dataSource = dataSource;
        _importVariables = importVariables;
        _fileType = fileType;
        _sourceDataFileType = sourceDataFileType;
        _destinationFile = destinationFile;
        _progress = progress;
    }
    
    /** This a temporary hack function that formats date fields to sFormat. This is needed because TreeScan
     * expects dates in human readable form such as '12/08/2002' as apposed to raw data form of 20021208. */
    private String formatDateField(String dateString) {
        StringBuilder builder = new StringBuilder();
        builder.append(dateString.substring(0, 4));
        builder.append("/");
        builder.append(dateString.substring(4, 6));
        builder.append("/");
        builder.append(dateString.substring(6, 8));
        return builder.toString();
    }
    
    public boolean getCancelled() {
        return _cancelled;
    }
    
    public void setCancelled() {
        _cancelled = true;
    }

    public static String join(AbstractCollection<String> s, String delimiter) {
        if (s.isEmpty()) return "";
        Iterator<String> iter = s.iterator();
        StringBuffer buffer = new StringBuffer(iter.next());
        while (iter.hasNext()) {
            String val = iter.next();
            if (!val.isEmpty())
                buffer.append(delimiter).append(val);
        }
        return buffer.toString();
    }

    /**
     * Imports file into destination file.
     */
    public void importFile(int skipCount) throws IOException, SecurityException {
        int iColumn=0;
        int iRow=0;
        String value;
        Vector<String> record = new Vector<String>();
        Vector<ImportVariable> mappedVariables = new Vector<ImportVariable>();
        
        //Attempt to open file writer and buffer ...
        FileWriter writer = new FileWriter(_destinationFile);
        BufferedWriter buffer = new BufferedWriter(writer);
        //initialize record and determine which field are actually imported
        for (int i=0; i < _importVariables.size(); ++i) {
            record.add(new String());
            if (_importVariables.get(i).getIsMappedToInputFileVariable())
                mappedVariables.add(_importVariables.get(i));
        }
        //initialize progress ...
        _progress.setMaximum(_dataSource.getNumRecords());
        //start reading and writing records ...
        Object[] values = _dataSource.readRow();
        while (values != null && !_cancelled) {
            if (iRow >= skipCount) {
                iColumn = 0;
                for (int i=0; i < mappedVariables.size(); ++i) {
                    iColumn = mappedVariables.get(i).getInputFileVariableIndex() - 1;
                    if (iColumn + 1 > values.length) {
                        value = ""; // imported variable indexed larger than number of fields in record -- default to blank value
                    } else {
                        value = (String)values[iColumn];
                    }
                    value = StringUtils.trimToEmpty(value);

                    //TODO: what about grouping character?

                    //if (StringUtils.isEmpty(value) || StringUtils.isBlank(value)) {
                    //    throw new ImportException(String.format("Record %d contains a 'Source File Variable' that is blank.\nTreeScan does not permit blank variables in data.", _dataSource.getCurrentRecordNum()));
                    //} else if (StringUtils.contains(value, " ")) {
                    //    throw new ImportException(String.format("Record %d contains a 'Source File Variable' that contains whitespace.\nTreeScan does not permit variable data to contain whitespace.", _dataSource.getCurrentRecordNum()));
                    //} else {
                        record.set(mappedVariables.get(i).getTargetFieldIndex(), new String(value));
                    //}
                }
                //append string to end of output file and clear input vector at the same time...
                buffer.write(join(record, ","));
                buffer.newLine();
            }
            values = _dataSource.readRow();
            _progress.setValue(++iRow);
        }
        buffer.close();
        writer.close();
    }
}
