/*
 * Utils.java
 *
 * Created on December 12, 2007, 9:14 AM
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */
package org.treescan.gui.utils;

import java.awt.Choice;
import java.time.LocalDate;
import java.time.temporal.ChronoUnit;
import java.util.Calendar;
import java.util.GregorianCalendar;
import javax.swing.JList;
import javax.swing.JTextField;
import javax.swing.JToggleButton;
import javax.swing.text.JTextComponent;
import javax.swing.undo.UndoManager;
import org.treescan.app.AppConstants;
import org.treescan.app.Parameters;

/**
 * Utilities class for graphical controls.
 * @author Hostovic
 */
public class Utils {

    /* Returns whether JToggleButton control is enabled and selected. */
    public static boolean selected(JToggleButton component) {
        return component.isEnabled() && component.isSelected();
    }
    
    /* If JToggleButton is enabled, returns whether select state matches expected value. Otherwise returns true. */
    public static boolean selected(JToggleButton component, boolean expected_value) {
        return component.isEnabled() ? component.isSelected() == expected_value : true;
    }    
    
    /* If JTextComponent is enabled, returns whether text matches expected value. Otherwise returns true. */
    public static boolean textIs(JTextComponent component, String expected_value) {
        return component.isEnabled() ? component.getText().equals(expected_value) : true;
    }     
    
    /* If JTextComponent is enabled, returns whether text as Double matches expected value. Otherwise returns true. */
    public static boolean doubleIs(JTextComponent component, Double expected_value) {
        try {
        return component.isEnabled() ? Double.valueOf(component.getText()).equals(expected_value) : true;
        } catch (NumberFormatException e) {
            return false;
        }
    }       

    /* If JTextComponent is enabled, returns whether text as Integer matches expected value. Otherwise returns true. */
    public static boolean integerIs(JTextComponent component, Integer expected_value) {
        try {
        return component.isEnabled() ? Integer.valueOf(component.getText()).equals(expected_value) : true;
        } catch (NumberFormatException e) {
            return false;
        }        
    }       

    /* If Choice is enabled, returns whether the selected index matches expected value. Otherwise returns true. */
    public static boolean selectionIs(Choice component, int expected_value) {
        return component.isEnabled() ? component.getSelectedIndex() == expected_value : true;
    }       
    
    /* If JList is enabled, returns whether model size matches expected value. Otherwise returns true. */
    public static boolean sizeIs(JList component, int expected_value) {
        return component.isEnabled() ? component.getModel().getSize() == expected_value : true;
    }         
    
    /* Validates that key typed is a positive integer or back space; otherwise consumes key. */
    public static void validatePostiveNumericKeyTyped(JTextField thisField, java.awt.event.KeyEvent e, int maxFieldLength) {
        if (!Character.isDigit(e.getKeyChar()) && e.getKeyCode() != java.awt.event.KeyEvent.VK_BACK_SPACE) {
            e.consume();
        }
        if (thisField.getSelectedText() == null && thisField.getText().length() >= maxFieldLength) {
            e.consume();
        }
    }

    /* Validates that key typed is an integer or back space; otherwise consumes key. */
    public static void validateNumericKeyTyped(JTextField thisField, java.awt.event.KeyEvent e, int maxFieldLength) {
        if (!(Character.isDigit(e.getKeyChar()) || e.getKeyChar() == java.awt.event.KeyEvent.VK_BACK_SPACE || e.getKeyChar() == '-')) {
            e.consume();
            return;
        }
        if (thisField.getSelectedText() == null && thisField.getText().length() >= maxFieldLength) {
            e.consume();
        }
    }

    /* Validates that key typed is a positive integer, period or back space; otherwise consumes key. */
    public static void validatePostiveFloatKeyTyped(JTextField thisField, java.awt.event.KeyEvent e, int maxFieldLength) {
        if (!(Character.isDigit(e.getKeyChar()) || e.getKeyChar() == '\b' || e.getKeyChar() == '.')) {
            e.consume();
            return;
        }
        if (thisField.getSelectedText() == null && thisField.getText().length() >= maxFieldLength) {
            e.consume();
        }
    }

    /* Validates that key typed is a positive integer, period, minus or back space; otherwise consumes key. */
    public static void validateFloatKeyTyped(JTextField thisField, java.awt.event.KeyEvent e, int maxFieldLength) {
        if (!(Character.isDigit(e.getKeyChar()) || e.getKeyChar() == '\b' || e.getKeyChar() == '-' || e.getKeyChar() == '.')) {
            e.consume();
            return;
        }
        if (thisField.getSelectedText() == null && thisField.getText().length() >= maxFieldLength) {
            e.consume();
        }
    }    

    /* Validates that generic date value is within allowable ranges. */
    public static void validateGenericDateField(JTextField genericControl, UndoManager undo) {
        if (genericControl.getText().length() == 0) {
            if (undo.canUndo()) {
                undo.undo();
            } else {
                genericControl.setText(Integer.toString(AppConstants.MAX_GENERIC));
            }
        } else {
            //set value to a valid setting if out of valid range
            if (Integer.parseInt(genericControl.getText()) < AppConstants.MIN_GENERIC) {
                genericControl.setText(Integer.toString(AppConstants.MIN_GENERIC));
            } else if (Integer.parseInt(genericControl.getText()) > AppConstants.MAX_GENERIC) {
                genericControl.setText(Integer.toString(AppConstants.MAX_GENERIC));
            }
        }

    }   
    
    /* validates date controls represented by three passed edit controls - prevents an invalid date */
    public static void validateDateControlGroup(JTextField YearControl, JTextField MonthControl, JTextField DayControl, UndoManager undo) {
        GregorianCalendar thisCalender = new GregorianCalendar();

        //first check year
        if (YearControl.getText().length() == 0) {
            if (undo.canUndo()) {
                undo.undo();
            } else {
                YearControl.setText(Integer.toString(AppConstants.MAX_YEAR));
            }
        }//YearControl.Undo();
        else {
            //set year to a valid setting if out of valid range
            if (Integer.parseInt(YearControl.getText()) < AppConstants.MIN_YEAR) {
                YearControl.setText(Integer.toString(AppConstants.MIN_YEAR));
            } else if (Integer.parseInt(YearControl.getText()) > AppConstants.MAX_YEAR) {
                YearControl.setText(Integer.toString(AppConstants.MAX_YEAR));
            }
        }
        thisCalender.set(Calendar.YEAR, Integer.parseInt(YearControl.getText()));
        //now check month
        if (MonthControl.getText().length() == 0) {
            if (undo.canUndo()) {
                undo.undo();
            } else {
                MonthControl.setText(Integer.toString(12));
            }
        }//MonthControl.Undo();
        else {
            //set month to a valid setting if out of valid range
            if (Integer.parseInt(MonthControl.getText()) < 1) {
                MonthControl.setText(Integer.toString(1));
            } else if (Integer.parseInt(MonthControl.getText()) > 12) {
                MonthControl.setText(Integer.toString(12));
            }
        }
        thisCalender.set(Calendar.MONTH, Integer.parseInt(MonthControl.getText()) - 1);
        //now check day
        int iDaysInMonth = thisCalender.getActualMaximum(Calendar.DAY_OF_MONTH);
        if (DayControl.getText().length() == 0) {
            if (undo.canUndo()) {
                undo.undo();
            } else {
                DayControl.setText(Integer.toString(iDaysInMonth));
            }
        }//DayControl.Undo();
        else {
            //set month to a valid setting if out of valid range
            if (Integer.parseInt(DayControl.getText()) < 1) {
                DayControl.setText(Integer.toString(1));
            } else if (Integer.parseInt(DayControl.getText()) > iDaysInMonth) {
                DayControl.setText(Integer.toString(iDaysInMonth));
            }
        }
    }
    
    /* Returns LocalDate object for passed input precision and control text. For precision of YEAR or MONTH,  this function assumes 
       that date inputs have been maintained correctly using DateComponentsGroup object. */
    public static LocalDate getLocalDate(final Parameters.DatePrecisionType precision, final String year, final String month, final String day, final String generic) {
        switch (precision) {
            case YEAR:
            case MONTH:
            case DAY: return LocalDate.of(Integer.parseInt(year), Integer.parseInt(month), Integer.parseInt(day));
            case GENERIC: return LocalDate.now().plusDays(Integer.parseInt(generic,10));
            default: throw new RuntimeException("getLocalDate() cannot be called for given precision");
        }
    }    
    
    /* Returns the number of time units in date range. */
    public static int getUnitsBetween(final Parameters.DatePrecisionType precision, LocalDate rangeStart, LocalDate rangeEnd) {
        if (rangeStart.isAfter(rangeEnd)) return 0;
        switch (precision) {
            case DAY:
            case GENERIC : return (int)ChronoUnit.DAYS.between(rangeStart, rangeEnd) + 1;
            case YEAR: return (int)ChronoUnit.YEARS.between(rangeStart, rangeEnd) + 1;
            case MONTH: return (int)ChronoUnit.MONTHS.between(rangeStart, rangeEnd) + 1;
                //int yearsInBetween = endData.get(Calendar.YEAR) - startDate.get(Calendar.YEAR); 
                //int monthsDiff = endData.get(Calendar.MONTH) - startDate.get(Calendar.MONTH); 
                //return yearsInBetween * 12 + monthsDiff;
            case NONE:
            default: return 0;
        }        
    }
    
    /* Parses date string and validates integer range. */
    public static void parseDateStringToControl(String sDateString, JTextField genericControl) {
        try {
            Integer iGeneric = Integer.parseInt(sDateString);
            if (iGeneric < AppConstants.MIN_GENERIC) {
                genericControl.setText(Integer.toString(AppConstants.MIN_GENERIC));
            } else if (iGeneric > AppConstants.MAX_GENERIC) {
                genericControl.setText(Integer.toString(AppConstants.MAX_GENERIC));
            } else {
                genericControl.setText(Integer.toString(iGeneric));
            }
        } catch (NumberFormatException e) {       
        }
    }

    /** parses up a date string and places it into the given month, day, year
     * interface text control (TEdit *). Defaults prospective survallience start
     * date to months/days to like study period end date.                       */
    public static void parseDateStringToControls(String sDateString, JTextField Year, JTextField Month, JTextField Day, boolean bEndDate) {
        String[] dateParts = sDateString.split("/");

        try {
            int iYear = 0, iMonth = 0, iDay = 0;
            //set values only if valid, prevent interface from having invalid date when first loaded.
            if (dateParts.length > 0) {
                switch (dateParts.length) {
                    case 1:
                        iYear = Integer.parseInt(dateParts[0]);
                        if (iYear >= AppConstants.MIN_YEAR && iYear <= AppConstants.MAX_YEAR) {
                            Year.setText(dateParts[0]);
                        }
                        break;
                    case 2:
                        iYear = Integer.parseInt(dateParts[0]);
                        iMonth = Integer.parseInt(dateParts[1]);
                        if (iYear >= AppConstants.MIN_YEAR && iYear <= AppConstants.MAX_YEAR && iMonth >= 1 && iMonth <= 12) {
                            Year.setText(dateParts[0]);
                            Month.setText(dateParts[1]);
                            if (bEndDate) {
                                GregorianCalendar thisCalender = new GregorianCalendar();
                                thisCalender.set(Calendar.YEAR, iYear);
                                thisCalender.set(Calendar.MONTH, iMonth - 1);
                                Day.setText(Integer.toString(thisCalender.getActualMaximum(Calendar.DAY_OF_MONTH)));
                            } else {
                                Day.setText("1");
                            }
                        }
                        break;
                    case 3:
                    default:
                        GregorianCalendar thisCalender = new GregorianCalendar();
                        iYear = Integer.parseInt(dateParts[0]);
                        iMonth = Integer.parseInt(dateParts[1]);
                        iDay = Integer.parseInt(dateParts[2]);
                        thisCalender.set(Calendar.YEAR, iYear);
                        thisCalender.set(Calendar.MONTH, iMonth - 1);
                        if (iYear >= AppConstants.MIN_YEAR && iYear <= AppConstants.MAX_YEAR && iMonth >= 1 && iMonth <= 12 &&
                                iDay >= 1 && iDay <= thisCalender.getActualMaximum(Calendar.DAY_OF_MONTH)) {
                            Year.setText(dateParts[0]);
                            Month.setText(dateParts[1]);
                            Day.setText(dateParts[2]);
                        }
                }
            }
        } catch (NumberFormatException e) {
        }
    }    
}
