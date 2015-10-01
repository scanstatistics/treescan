package org.treescan.importer;

import java.io.*;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
import org.apache.poi.hssf.record.RecordFormatException;
import org.apache.poi.hssf.usermodel.HSSFWorkbook;
import org.apache.poi.ss.usermodel.BuiltinFormats;
import org.apache.poi.xssf.usermodel.XSSFWorkbook;
import org.apache.poi.ss.usermodel.Cell;
import org.apache.poi.ss.usermodel.DataFormat;
import org.apache.poi.ss.usermodel.DateUtil;
import org.apache.poi.ss.usermodel.Row;
import org.apache.poi.ss.usermodel.Sheet;
import org.apache.poi.ss.usermodel.Workbook;
import org.treescan.utils.FileAccess;

/* A data source for reading from Excel (XLS) files natively. */
public class XLSImportDataSource implements ImportDataSource {

    protected Workbook _workbook;
    protected Sheet _sheet;
    protected int _current_row;
    private static final String VERSION_ERRROR = "The Excel file could not be opened.\n" +
            "This error may be caused by opening an unsupported XLS version (Excel 5.0/7.0 format).\n" +
            "To test whether you are importing an unsupported version, re-save the file to versions 97-2003 and try again.";
    private String _file_path;
    private int _sheet_index = 0;
    private ArrayList<Object> _column_names = new ArrayList<Object>();
    private boolean _hasHeader=false;
    
    public XLSImportDataSource(File file, boolean hasHeader) {
        _current_row = 0;
        _hasHeader = hasHeader;
        try {
            if (FileAccess.getExtension(file).equals("xlsx"))
                _workbook = new XSSFWorkbook(new FileInputStream(file));
            else
                _workbook = new HSSFWorkbook(new FileInputStream(file));
        } catch (RecordFormatException e) {
            throw new RuntimeException(e);
        } catch (FileNotFoundException e) {
            throw new RuntimeException(e);
        } catch (org.apache.poi.hssf.OldExcelFormatException e) {
            throw new ImportDataSource.UnsupportedException(VERSION_ERRROR, e);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
        _file_path = file.getAbsolutePath();
        //Set the first sheet to be initial default.
        setSheet(0);
        
        if (_hasHeader) {
            Object[] row = readRow();
            for (int i=0; i < row.length; ++i)
                _column_names.add(row[i]);
            _current_row = 1;
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

    public void close() throws IOException {
        //does nothing because the HSSF lib does not need the file
        //after constructing the workbook so I close it right away
    }

    /* Sets what sheet to read from. */
    public void setSheet(int i) {
        _sheet = _workbook.getSheetAt(i);
        _sheet_index = i;
    }

    /* Gets the index of the sheet to read from. */
    public int getSheetIndex() {
        return _sheet_index;
    }

    /**
     * @return An array of strings each of which represent a name
     * of a sheet in the workbook.
     */
    public String[] getSheetList() {
        int sheetCount = _workbook.getNumberOfSheets();
        String[] sheetList = new String[sheetCount];
        for (int i = 0; i < sheetCount; i++) {
            sheetList[i] = _workbook.getSheetName(i);
        }
        return sheetList;
    }

    @Override
    public Object[] readRow() {
        if (_sheet == null) { //If _sheet is null then throw IOException so the caller is alerted to an unexpected error.
            throw new RuntimeException("Null Sheet Reference");
        }

        boolean containedNonEmptyCells = false;
        //returns the next non-empty row or null if none are left
        while (true) {
            if (_current_row <= _sheet.getLastRowNum()) {
                Row row = _sheet.getRow(_current_row);
                _current_row++;
                if (row != null && row.getPhysicalNumberOfCells() > 0) {
                    ArrayList<Object> obj = new ArrayList<Object>();
                    
                    //Get the number of defined cells in this row
                    int totalDefinedCells = row.getPhysicalNumberOfCells();
                    //Initialize the number of defined cells found so far
                    int definedCellsFound = 0;
                    //set number of cells that exist (defined or not)
                    int lastCellInRow = row.getLastCellNum();
                    //Stop for loop after processing all the defined cells of this row
                    for (int i=0; i < lastCellInRow; i++) {
                        //if still processing defined cells add them accordingly
                        if (definedCellsFound < totalDefinedCells) {
                            Cell cell = row.getCell((short) i);
                            if (cell != null) {
                                //If this cell is defined increment count
                                definedCellsFound++;
                                String cellContents = getCellValue(cell).trim();
                                //keep a flag to see if any of the cells contains real data, not just ""
                                if (!cellContents.equals("")) {
                                    containedNonEmptyCells = true;
                                }
                                obj.add(cellContents);
                            } else { //If the cell is undefined add an empty string
                                obj.add("");
                            }
                        } else {//done with defined cells but there are still some null trailing cells
                            obj.add("");
                        }
                    }
                    //normal return when row has data
                    if (containedNonEmptyCells) {
                        return obj.toArray();
                    }
                }
            } else { //gCurrentRow > _sheet.getLastRowNum()
                //If there are no more rows to read return null.
                return null;
            }
        }
    }

    public void reset() {
        _current_row = _hasHeader ? 1 : 0;
    }

    public int getCurrentRow() {
        return _current_row;
    }

    public String getAbsolutePath() {
        return _file_path;
    }

    /**
     * make a date string out of the excel representation (i.e. M/D/YYYY [HH:MM])
     * @param cellValue excel representation
     * @return a string formatted date
     */
    private String formatDate(double cellValue) {
        //create date and set calendar to be used in setting dateValue
        Date date = DateUtil.getJavaDate(cellValue);
        Calendar cal = Calendar.getInstance();
        cal.setTime(date);

        StringBuilder dateValue = new StringBuilder();
        dateValue.append(cal.get(Calendar.YEAR));
        dateValue.append("/");
        int month = cal.get(Calendar.MONTH) + 1;
        if (month < 10) {
            dateValue.append("0");
        }
        dateValue.append(month);
        dateValue.append("/");
        int day = cal.get(Calendar.DAY_OF_MONTH);
        if (day < 10) {
            dateValue.append("0");
        }
        dateValue.append(day);
        return dateValue.toString();
    }

    /**
     * The only types of cell supported are:<br>
     * NUMERIC, STRING, FORMULA, and BLANK.
     *
     * @param cell the cell to have its value converted to a String.
     * @return the String representation of the value in the cell. If the
     * cell is blank or unsupported an empty String is returned.
     */
    private String getCellValue(Cell cell) {
        int cellType = cell.getCellType();
        switch (cellType) {
            case Cell.CELL_TYPE_NUMERIC:
                double cellValue = cell.getNumericCellValue();
                //check if this is a date
                boolean isPoiKnownDate = DateUtil.isCellDateFormatted(cell) || DateUtil.isInternalDateFormat(cell.getCellStyle().getDataFormat());
                if (isPoiKnownDate) {
                    return formatDate(cellValue);
                } else {
                    //try to see if this is a user defined date format (unknown by POI's HSSFDateUtil)
                    short formatIndex = cell.getCellStyle().getDataFormat();
                    try {
                        BuiltinFormats.getBuiltinFormat(formatIndex);
                    } catch (ArrayIndexOutOfBoundsException e) { //this is a user defined format
                        DataFormat dataFormat = _workbook.createDataFormat();
                        String format = dataFormat.getFormat(formatIndex);

                        boolean isValid = DateUtil.isValidExcelDate(cellValue);
                        //see if there are any date formatting strings in the format
                        boolean hasDateFormating = format.matches(".*[mM]{2,}.*|.*[dD]{2,}.*|.*[yY]{2,}.*|.*[hH]{2,}.*|.*[sS]{2,}.*|.*[qQ]{2,}.*|.*[nN]{2,}.*|.*[wW]{2,}.*");

                        if (isValid && hasDateFormating) {
                            return formatDate(cellValue);
                        }

                    }
                    //if this is not a date process as a number
                    Double number = new Double(cellValue);
                    //if this is an int create the string as such
                    if (number.doubleValue() == number.intValue()) {
                        return (Integer.toString(number.intValue()));
                    }
                    return (number.toString());
                }

            case Cell.CELL_TYPE_BOOLEAN: return cell.getBooleanCellValue() + "";
                
            case Cell.CELL_TYPE_STRING: return (cell.getStringCellValue());
                
            //If this is a formula evaluate it to a value
            case Cell.CELL_TYPE_FORMULA:
                String cellResult = String.valueOf(cell.getNumericCellValue());
                return cellResult.equals("NaN") ? cell.getStringCellValue() : cellResult;

            //If this cell is not of a supported type fill array in
            //with empty string (includes HSSFCell.CELL_TYPE_BLANK)
            default: return "";
        }
    }

    @Override
    public boolean isColumnDate(int iColumn) {
        return false;
    }

    @Override
    public long getCurrentRecordNum() {
        return _current_row;
    }

    @Override
    public int getNumRecords() {
        return _sheet.getLastRowNum();
    }

    /** Returns column names, if any.*/
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
}
