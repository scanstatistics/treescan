package org.treescan.importer;

import java.io.*;
import java.util.Calendar;
import java.util.Date;

import org.apache.poi.hssf.record.RecordFormatException;
import org.apache.poi.hssf.usermodel.*;
import org.treescan.app.*;

/**
 * A data source for reading from Excel (XLS) files natively.
 *
 * @author watsond
 */
public class XLSImportDataSource implements ImportDataSource {

    protected HSSFWorkbook gWorkbook;
    protected HSSFSheet gSheet;
    protected int gCurrentRow;
    private final boolean _hasHeader;
    private static final String VERSION_ERRROR = "The Selected Cannot Be Opened.\n\n" +
            "This error may be caused by opening an unsupported XLS version.\n" +
            "To test whether you are importing an unsupported version,\n" +
            "re-save the file to versions 97-2002 and try again.";
    private String gFilePath;
    private int gSheetIndex = 0;

    public XLSImportDataSource(File file, boolean hasHeader) {
        gCurrentRow = 0;
        _hasHeader = hasHeader;
        try {
            gWorkbook = new HSSFWorkbook(new FileInputStream(file));
        } catch (RecordFormatException e) {
            throw new RuntimeException(VERSION_ERRROR, e);
        } catch (FileNotFoundException e) {
            throw new RuntimeException(VERSION_ERRROR, e);
        } catch (Exception e) {
            throw new RuntimeException(VERSION_ERRROR, e);
        }

        gFilePath = file.getAbsolutePath();
        //Set the first sheet to be initial default.
        setSheet(0);
    }

    /* (non-Javadoc)
     * @see bsi.client.table.importer.ImportDataSource#close()
     */
    public void close()
            throws IOException {
    //does nothing because the HSSF lib does not need the file
    //after constructing the workbook so I close it right away
    }

    /**
     * Sets what sheet to read from.
     * @param i
     */
    public void setSheet(int i) {
        gSheet = gWorkbook.getSheetAt(i);
        gSheetIndex = i;
    }

    /**
     * Gets the index of the sheet to read from.
     *
     */
    public int getSheetIndex() {
        return gSheetIndex;
    }

    /**
     * @return An array of strings each of which represent a name
     * of a sheet in the workbook.
     */
    public String[] getSheetList() {
        int sheetCount = gWorkbook.getNumberOfSheets();
        String[] sheetList = new String[sheetCount];
        for (int i = 0; i < sheetCount; i++) {
            sheetList[i] = gWorkbook.getSheetName(i);
        }
        return sheetList;
    }

    /* (non-Javadoc)
     * @see bsi.client.table.importer.ImportDataSource#readRow()
     */
    public Object[] readRow() {
        if (gSheet == null) //If gSheet is null then throw IOException so the caller
        //is alerted to an unexpected error.
        {
            throw new RuntimeException("Null XLS Sheet Reference");
        }

        boolean containedNonEmptyCells = false;
        //returns the next non-empty row or null if none are left
        while (true) {
            if (gCurrentRow <= gSheet.getLastRowNum()) {
                if (_hasHeader && gCurrentRow == 0) {
                    gCurrentRow++;
                    continue;
                }
                HSSFRow row = gSheet.getRow(gCurrentRow);
                gCurrentRow++;
                if (row != null && row.getPhysicalNumberOfCells() > 0) {
                    Object[] obj = new String[row.getLastCellNum()];

                    //Get the number of defined cells in this row
                    int totalDefinedCells = row.getPhysicalNumberOfCells();
                    //Initialize the number of defined cells found so far
                    int definedCellsFound = 0;
                    //set number of cells that exist (defined or not)
                    int lastCellInRow = row.getLastCellNum();
                    //Stop for loop after processing all the defined cells of this row
                    for (int i = 0; i < lastCellInRow; i++) {
                        //if still processing defined cells add them accordingly
                        if (definedCellsFound < totalDefinedCells) {
                            HSSFCell cell = row.getCell((short) i);
                            if (cell != null) {
                                //If this cell is defined increment count
                                definedCellsFound++;
                                String cellContents = getCellValue(cell).trim();
                                //keep a flag to see if any of the cells contains real data, not just ""
                                if (!cellContents.equals("")) {
                                    containedNonEmptyCells = true;
                                }
                                obj[i] = cellContents;
                            } //If the cell is undefined add an empty string
                            else {
                                obj[i] = "";
                            }
                        } else //done with defined cells but there are still some null trailing cells
                        {
                            obj[i] = "";
                        }
                    }
                    //normal return when row has data
                    if (containedNonEmptyCells) {
                        return obj;
                    }
                }
            } else { //gCurrentRow > gSheet.getLastRowNum()
                //If there are no more rows to read return null.
                return null;
            }
        }
    }

    /* (non-Javadoc)
     * @see bsi.client.table.importer.ImportDataSource#reset()
     */
    public void reset() {
        gCurrentRow = 0;
    }

    /* (non-Javadoc)
     * @see bsi.client.table.importer.ImportDataSource#getCurrentRow()
     */
    public int getCurrentRow() {
        return gCurrentRow;
    }

    public String getAbsolutePath() {
        return gFilePath;
    }

    /**
     * make a date string out of the excel representation (i.e. M/D/YYYY [HH:MM])
     * @param cellValue excel representation
     * @return a string formatted date
     */
    private String formatDate(double cellValue) {
        //create date and set calendar to be used in setting dateValue
        Date date = HSSFDateUtil.getJavaDate(cellValue);
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

        //see if we should add time
        //boolean timeIsSet = cal.isSet(Calendar.MINUTE) && cal.isSet(Calendar.HOUR_OF_DAY);
        //if(timeIsSet){
        //   int hour   = cal.get(Calendar.HOUR_OF_DAY);
        //    int minute = cal.get(Calendar.MINUTE);
        //    //add a '0' so it doesn't cause an error in the table
        //    String hourString   = hour   > 0 ? String.valueOf(hour)   : "00";
        //   String minuteString = minute > 0 ? String.valueOf(minute) : "00";
        //    //add time
        //    dateValue += " " + hourString + ":" + minuteString;
        //}

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
    private String getCellValue(HSSFCell cell) {
        int cellType = cell.getCellType();
        switch (cellType) {
            case HSSFCell.CELL_TYPE_NUMERIC:
                double cellValue = cell.getNumericCellValue();
                //check if this is a date
                boolean isPoiKnownDate = HSSFDateUtil.isCellDateFormatted(cell) ||
                        HSSFDateUtil.isInternalDateFormat(cell.getCellStyle().getDataFormat());
                if (isPoiKnownDate) {
                    return formatDate(cellValue);
                } else {
                    //try to see if this is a user defined date format (unknown by POI's HSSFDateUtil)
                    short formatIndex = cell.getCellStyle().getDataFormat();
                    try {
                        HSSFDataFormat.getBuiltinFormat(formatIndex);
                    } catch (ArrayIndexOutOfBoundsException e) { //this is a user defined format
                        HSSFDataFormat dataFormat = gWorkbook.createDataFormat();
                        String format = dataFormat.getFormat(formatIndex);

                        boolean isValid = HSSFDateUtil.isValidExcelDate(cellValue);
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

            case HSSFCell.CELL_TYPE_STRING:
                return (cell.getStringCellValue());

            //If this is a formula evaluate it to a value
            case HSSFCell.CELL_TYPE_FORMULA:
                String cellResult = String.valueOf(cell.getNumericCellValue());
                return cellResult.equals("NaN") ? cell.getStringCellValue() : cellResult;


            //If this cell is not of a supported type fill array in
            //with empty string (includes HSSFCell.CELL_TYPE_BLANK)
            default:
                return "";
        }
    }

    public boolean isColumnDate(int iColumn) {
        return false;
    }

    public long getCurrentRecordNum() {
        return gCurrentRow;
    }

    public int getNumRecords() {
        return gSheet.getLastRowNum();
    }
}
