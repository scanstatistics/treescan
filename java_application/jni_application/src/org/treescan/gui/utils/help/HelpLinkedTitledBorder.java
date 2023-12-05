package org.treescan.gui.utils.help;

/**
 * MySwing: Advanced Swing Utilites
 * Copyright (C) 2005  Santhosh Kumar T
 * <p/>
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * <p/>
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * Modified source by Scott Hostovich on 6/22/2012.
 */
import org.treescan.gui.utils.*;
import javax.swing.*;
import javax.swing.border.Border;
import java.awt.*;
import java.awt.event.*;
import java.awt.event.MouseListener;
import javax.swing.border.EtchedBorder;
import javax.swing.border.TitledBorder;
import org.treescan.gui.utils.JHyperLink;

public class HelpLinkedTitledBorder implements Border, MouseListener, MouseMotionListener, SwingConstants {
    private int offset = 7;
    private JHyperLink titleComponent;
    private JComponent container;
    private Rectangle rect;
    private Border border;
    private boolean mouseEntered = false;

    public HelpLinkedTitledBorder(JComponent container, final String helpID) {
        TitledBorder testBorder = (TitledBorder)container.getBorder();
        if (testBorder == null) {
            throw new RuntimeException("Expecting container to already have a titled border.");
        }
        _setup(container, testBorder.getTitle(), helpID);
    }

    public HelpLinkedTitledBorder(JComponent container, final String title, final String helpID) {
        _setup(container, title, helpID);
    }

    private void _setup(JComponent container, final String title, final String helpID) {
        titleComponent = new JHyperLink(title, Color.BLACK);
        titleComponent.addActionListener(new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                HelpShow.showHelp(helpID);
            }
        });
        this.container = container;
        border = javax.swing.BorderFactory.createEtchedBorder();
        container.addMouseListener(this);
        container.addMouseMotionListener(this);
    }

    public void addLinkActionListener(ActionListener l) {
        titleComponent.addActionListener(l);
    }

    @Override
    public boolean isBorderOpaque() {
        return true;
    }

    @Override
    public void paintBorder(Component c, Graphics g, int x, int y, int width, int height) {
        Insets borderInsets = border.getBorderInsets(c);
        Insets insets = getBorderInsets(c);
        int temp = (insets.top - borderInsets.top) / 2;
        //border.paintBorder(c, g, x, y + temp, width, height - temp);
        paintEtchedBorder((EtchedBorder)border, c, titleComponent, g, x, y + temp, width, height - temp);
        Dimension size = titleComponent.getPreferredSize();
        rect = new Rectangle(offset, 0, size.width, size.height);
        SwingUtilities.paintComponent(g, titleComponent, (Container) c, rect);
    }

    public void paintEtchedBorder(EtchedBorder b, Component c, Component comp, Graphics g, int x, int y, int width, int height) {
	int w = width;
	int h = height;

	g.translate(x, y);
        g.setColor(b.getEtchType() == EtchedBorder.LOWERED? b.getShadowColor(c) : b.getHighlightColor(c));

        // Instead if drawing rectangle("g.drawRect(0, 0, w-2, h-2);"), draw in line segments.
        int stringWidth = g.getFontMetrics().stringWidth(((JLabel)comp).getText());
        int xx = 0;
        int yy = 0;
        int wwidth = w-2;
        int hheight = h-2;
	g.drawLine(xx, yy, 4, yy);
	g.drawLine(stringWidth + 10, yy, xx + wwidth - 1, yy);
	g.drawLine(xx + wwidth, yy, xx + wwidth, yy + hheight - 1);
	g.drawLine(xx + wwidth, yy + hheight, xx + 1, yy + hheight);
	g.drawLine(xx, yy + hheight, xx, yy + 1);

	g.setColor(b.getEtchType() == EtchedBorder.LOWERED ? b.getHighlightColor(c) : b.getShadowColor(c));
	g.drawLine(1, h-3, 1, 1);
	g.drawLine(1, 1, w-3, 1);        
	g.drawLine(0, h-1, 4, h-1);
	g.drawLine(stringWidth + 10, h-1, w-1, h-1);
	g.drawLine(w-1, h-1, w-1, 0);

        g.translate(-x, -y);
    }

    @Override
    public Insets getBorderInsets(Component c) {
        Dimension size = titleComponent.getPreferredSize();
        Insets insets = border.getBorderInsets(c);
        insets.top = Math.max(insets.top, size.height) + 7;
        return insets;
    }

    private void dispatchEvent(MouseEvent me) {
        if (rect != null && rect.contains(me.getX(), me.getY())) {
            dispatchEvent(me, me.getID());
        }
    }

    private void dispatchEvent(MouseEvent me, int id) {
        Point pt = me.getPoint();
        pt.translate(-offset, 0);

        Dimension size = titleComponent.getPreferredSize();
        Rectangle cpm_rect = new Rectangle(offset, 0, size.width, size.height);

        if (cpm_rect.contains(pt)) {
           container.setCursor(Cursor.getPredefinedCursor(Cursor.HAND_CURSOR));
        } else {
           container.setCursor(Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
        }

        titleComponent.setSize(rect.width, rect.height);
        titleComponent.dispatchEvent(new MouseEvent(titleComponent, id, me.getWhen(),
                me.getModifiers(), pt.x, pt.y, me.getClickCount(),
                me.isPopupTrigger(), me.getButton()));

        if (!titleComponent.isValid()) {
            container.repaint();
        }
    }

    @Override
    public void mouseClicked(MouseEvent me) {
        dispatchEvent(me);
    }

    @Override
    public void mouseEntered(MouseEvent me) {
        dispatchEvent(me);
    }

    @Override
    public void mouseExited(MouseEvent me) {
        if (mouseEntered) {
            mouseEntered = false;
            dispatchEvent(me, MouseEvent.MOUSE_EXITED);
        }
    }

    @Override
    public void mousePressed(MouseEvent me) {
        dispatchEvent(me);
    }

    @Override
    public void mouseReleased(MouseEvent me) {
        dispatchEvent(me);
    }

    @Override
    public void mouseDragged(MouseEvent e) {
    }

    @Override
    public void mouseMoved(MouseEvent me) {
        if (rect == null) {
            return;
        }

        if (mouseEntered == false && rect.contains(me.getX(), me.getY())) {
            mouseEntered = true;
            dispatchEvent(me, MouseEvent.MOUSE_ENTERED);
        } else if (mouseEntered == true) {
            if (rect.contains(me.getX(), me.getY()) == false) {
                mouseEntered = false;
                dispatchEvent(me, MouseEvent.MOUSE_EXITED);
            } else {
                dispatchEvent(me, MouseEvent.MOUSE_MOVED);
            }
        }
    }
}