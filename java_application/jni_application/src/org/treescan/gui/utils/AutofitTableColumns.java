/*
 * AutofitTableColumns.java
 *
 * Created on October 31, 2007, 9:35 AM
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */
package org.treescan.gui.utils;

/**
 *
 * @author Hostovic
 */
import java.awt.Dimension;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.Component;

import javax.swing.text.JTextComponent;
import javax.swing.SwingUtilities;
import javax.swing.JLabel;

import javax.swing.JTable;
import javax.swing.table.JTableHeader;
import javax.swing.table.TableColumnModel;
import javax.swing.table.TableColumn;
import javax.swing.table.TableCellRenderer;

public class AutofitTableColumns {

    private static final int DEFAULT_COLUMN_PADDING = 20;

    /*
     * @param JTable aTable, the JTable to autoresize the columns on
     * @param boolean includeColumnHeaderWidth, use the Column Header width as a minimum width
     * @returns The table width, just in case the caller wants it...
     */
    public static int autoResizeTable(JTable aTable, boolean includeColumnHeaderWidth) {
        return autoResizeTable(aTable, includeColumnHeaderWidth, DEFAULT_COLUMN_PADDING);
    }

    /*
     * @param JTable aTable, the JTable to autoresize the columns on
     * @param boolean includeColumnHeaderWidth, use the Column Header width as a minimum width
     * @param int columnPadding, how many extra pixels do you want on the end of each column
     * @returns The table width, just in case the caller wants it...
     */
    public static int autoResizeTable(JTable aTable, boolean includeColumnHeaderWidth, int columnPadding) {
        int columnCount = aTable.getColumnCount();
        int tableWidth = 0;

        Dimension cellSpacing = aTable.getIntercellSpacing();
        if (columnCount > 0) {// must have columns !
            // STEP ONE : Work out the column widths
            int columnWidth[] = new int[columnCount];
            for (int i = 0; i < columnCount; i++) {
                columnWidth[i] = getMaxColumnWidth(aTable, i, true, columnPadding);
                tableWidth += columnWidth[i];
            }
            // account for cell spacing too
            tableWidth += ((columnCount - 1) * cellSpacing.width);

            // STEP TWO : Dynamically resize each column
            // try changing the size of the column names area
            JTableHeader tableHeader = aTable.getTableHeader();
            Dimension dim = tableHeader.getPreferredSize();
            dim.width = tableWidth;
            tableHeader.setPreferredSize(dim);
            dim = aTable.getPreferredSize();
            dim.width = tableWidth;
            aTable.setPreferredSize(dim);
            TableColumnModel tableColumnModel = aTable.getColumnModel();
            TableColumn tableColumn;
            for (int i = 0; i < 1 /*columnCount*/; i++) {
                // rowLabelTable
                tableColumn = tableColumnModel.getColumn(i);
                tableColumn.setPreferredWidth(columnWidth[i]);
            }
            aTable.doLayout();
        }
        return tableWidth;
    }

    /*
     * @param JTable aTable, the JTable to autoresize the columns on
     * @param int columnNo, the column number, starting at zero, to calculate the maximum width on
     * @param boolean includeColumnHeaderWidth, use the Column Header width as a minimum width
     * @param int columnPadding, how many extra pixels do you want on the end of each column
     * @returns The table width, just in case the caller wants it...
     */
    public static int getMaxColumnWidth(JTable aTable, int columnNo, boolean includeColumnHeaderWidth, int columnPadding) {
        TableColumn column = aTable.getColumnModel().getColumn(columnNo);
        Component comp = null;
        int maxWidth = 0;

        if (includeColumnHeaderWidth) {
            TableCellRenderer headerRenderer = column.getHeaderRenderer();
            if (headerRenderer != null) {
                comp = headerRenderer.getTableCellRendererComponent(aTable, column.getHeaderValue(), false, false, 0, columnNo);
                if (comp instanceof JTextComponent) {
                    JTextComponent jtextComp = (JTextComponent) comp;
                    String text = jtextComp.getText();
                    Font font = jtextComp.getFont();
                    FontMetrics fontMetrics = jtextComp.getFontMetrics(font);
                    maxWidth = SwingUtilities.computeStringWidth(fontMetrics, text);
                } else {
                    maxWidth = comp.getPreferredSize().width;
                }
            } else {
                try {
                    String headerText = (String) column.getHeaderValue();
                    JLabel defaultLabel = new JLabel(headerText);
                    Font font = defaultLabel.getFont();
                    FontMetrics fontMetrics = defaultLabel.getFontMetrics(font);
                    maxWidth = SwingUtilities.computeStringWidth(fontMetrics, headerText);
                } catch (ClassCastException x) {
                    // Can't work out the header column width..
                    maxWidth = 0;
                }
            }
        }

        TableCellRenderer tableCellRenderer;
        // Component comp;
        int cellWidth = 0;
        for (int i = 0; i < aTable.getRowCount(); i++) {
            tableCellRenderer = aTable.getCellRenderer(i, columnNo);
            comp = tableCellRenderer.getTableCellRendererComponent(aTable, aTable.getValueAt(i, columnNo), false, false, i, columnNo);
            if (comp instanceof JTextComponent) {
                JTextComponent jtextComp = (JTextComponent) comp;
                String text = jtextComp.getText();
                Font font = jtextComp.getFont();
                FontMetrics fontMetrics = jtextComp.getFontMetrics(font);
                int textWidth = SwingUtilities.computeStringWidth(fontMetrics, text);
                maxWidth = Math.max(maxWidth, textWidth);
            } else {
                cellWidth = comp.getPreferredSize().width;
                // maxWidth = Math.max ( headerWidth, cellWidth );
                maxWidth = Math.max(maxWidth, cellWidth);
            }
        }
        return maxWidth + columnPadding;
    }
}

