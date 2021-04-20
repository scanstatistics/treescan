/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package org.treescan.gui.utils;

import java.util.Calendar;
import java.util.GregorianCalendar;
import javax.swing.JTextField;
import javax.swing.event.UndoableEditEvent;
import javax.swing.event.UndoableEditListener;
import javax.swing.undo.UndoManager;

/**
 *
 * @author hostovic
 */
public class DateComponentsGroup {
    protected final UndoManager _undo;
    protected final JTextField _yearField;
    protected final JTextField _monthField;
    protected final JTextField _dayField;
    protected int _stickyYear;
    protected int _stickyMonth;
    protected int _stickyDay;
    
    public DateComponentsGroup(final UndoManager undo, 
                               final JTextField yearField, 
                               final JTextField monthField, 
                               final JTextField dayField,
                               int initStickyYear,
                               int initStickyMonth,
                               int initStickyDay,
                               boolean isEndDate) {
        _undo = undo;
        _yearField = yearField;
        _monthField = monthField;
        _dayField = dayField;
        _stickyYear = initStickyYear;
        _stickyMonth = initStickyMonth;
        _stickyDay = initStickyDay;

        //add undoable edit listeners
        _yearField.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                _undo.addEdit(evt.getEdit());
            }
        });        
        _monthField.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                _undo.addEdit(evt.getEdit());
            }
        });        
        _dayField.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                _undo.addEdit(evt.getEdit());
            }
        });     
        // add key listeners
        _yearField.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validatePostiveNumericKeyTyped(_yearField, e, 4);         
            }
        }); 
        _monthField.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validatePostiveNumericKeyTyped(_monthField, e, 2);
            }
        });
        _dayField.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validatePostiveNumericKeyTyped(_dayField, e, 2);
            }
        }); 
        // add focus listeners
        _yearField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                validateGroup();
            }
        });
        _monthField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                validateGroup();
            }
        });        
        _dayField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                validateGroup();
            }
        });   
        // add addditional listeners for end dates
        if (isEndDate) {
            // add focus listeners
            _yearField.addFocusListener(new java.awt.event.FocusAdapter() {
                public void focusLost(java.awt.event.FocusEvent e) {
                    // day field is disabled, then set to last day of month
                    if (!_monthField.isEnabled()) {
                        _stickyMonth = Integer.parseInt(_monthField.getText());
                        _monthField.setText("12");                    
                    }
                    if (!_dayField.isEnabled()) {
                        _stickyDay = Integer.parseInt(_dayField.getText());
                        GregorianCalendar thisCalender = new GregorianCalendar();
                        thisCalender.set(Calendar.YEAR, Integer.parseInt(_yearField.getText()));
                        thisCalender.set(Calendar.MONTH, Integer.parseInt(_monthField.getText()) - 1);
                        _dayField.setText(Integer.toString(thisCalender.getActualMaximum(Calendar.DAY_OF_MONTH)));                    
                    }
                }
            });
            _monthField.addFocusListener(new java.awt.event.FocusAdapter() {
                public void focusLost(java.awt.event.FocusEvent e) {
                    // day field is disabled, then set to last day of month
                    if (!_dayField.isEnabled()) {
                        _stickyDay = Integer.parseInt(_dayField.getText());
                        GregorianCalendar thisCalender = new GregorianCalendar();
                        thisCalender.set(Calendar.YEAR, Integer.parseInt(_yearField.getText()));
                        thisCalender.set(Calendar.MONTH, Integer.parseInt(_monthField.getText()) - 1);
                        _dayField.setText(Integer.toString(thisCalender.getActualMaximum(Calendar.DAY_OF_MONTH)));                    
                    }
                }
            }); 
        }
    }
    
    public void setYear(int year) {
        _stickyYear = Integer.parseInt(_yearField.getText());
        _yearField.setText(Integer.toString(year));
        Utils.validateDateControlGroup(_yearField, _monthField, _dayField, _undo);
    }
    public void restoreYear() {
        _yearField.setText(Integer.toString(_stickyYear));
        validateGroup();
    }
    public void setMonth(int month) {
        _stickyMonth = Integer.parseInt(_monthField.getText());
        _monthField.setText(Integer.toString(month));
        validateGroup();
    }
    public void restoreMonth() {
        _monthField.setText(Integer.toString(_stickyMonth));
        validateGroup();
    }
    public void setDay(int day) {
        _stickyDay = Integer.parseInt(_dayField.getText());
        _dayField.setText(Integer.toString(day));
        validateGroup();
    }    
    public void restoreDay() {
        _dayField.setText(Integer.toString(_stickyDay));
        validateGroup();
    }
    public void validateGroup() {
        Utils.validateDateControlGroup(_yearField, _monthField, _dayField, _undo);
    }
}
