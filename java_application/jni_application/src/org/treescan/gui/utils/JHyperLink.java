/*
 * JHyperLink.java
 *
 * Created on December 11, 2007, 9:36 AM
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

package org.treescan.gui.utils;


import java.awt.Color;
import java.awt.Cursor;
import java.awt.Graphics;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.geom.Rectangle2D;
import java.util.LinkedList;
import javax.swing.BoxLayout;
import javax.swing.JFrame;
import javax.swing.JLabel;

public class JHyperLink extends JLabel {
    protected final Color LINK_COLOR  = Color.blue;
    protected LinkedList<ActionListener> actionListenerList = new LinkedList<ActionListener>();
    protected boolean underline;
    
    protected MouseListener mouseListener = new MouseAdapter() {
        public void mouseEntered(MouseEvent me) {
            underline = true;
            repaint();
        }
        
        public void mouseExited(MouseEvent me) {
            underline = false;
            repaint();
        }
        
        public void mouseClicked(MouseEvent me) {
            fireActionEvent();
        }
    };
    
    public JHyperLink() {
        this("");
    }
    
    public JHyperLink(String text) {
        super(text);
        
        setCursor(Cursor.getPredefinedCursor(Cursor.HAND_CURSOR));
        
        setForeground(LINK_COLOR);
        
        addMouseListener( mouseListener );
        
    }
    
    public void addActionListener(ActionListener l) {
        if(!actionListenerList.contains( l )) {
            actionListenerList.add( l );
        }
    }
    
    public void removeActionListener(ActionListener l) {
        actionListenerList.remove( l );
    }
    
    protected void fireActionEvent() {
        ActionEvent e = new ActionEvent(this, ActionEvent.ACTION_PERFORMED, getText());
        for(ActionListener l : actionListenerList ) {
            l.actionPerformed( e );
        }
    }
    
    public void paint(Graphics g) {
        super.paint(g);
        if (underline) {
            // really all this size stuff below only needs to be recalculated if font or text changes
            Rectangle2D textBounds =  getFontMetrics(getFont()).getStringBounds(getText(), g);
            
            //this layout stuff assumes the icon is to the left, or null
            int y = getHeight()/2 + (int)(textBounds.getHeight()/2);
            int w = (int) textBounds.getWidth();
            int x = getIcon()==null ? 0 : getIcon().getIconWidth() + getIconTextGap();
            
            g.setColor(getForeground());
            g.drawLine(0, y, x + w, y);
        }
    }
    
    
    
    public static void main( String[] args ) {
        JFrame f = new JFrame("Test");
        JHyperLink link = new JHyperLink("This is a link");
        link.addActionListener( new ActionListener() {
            
            public void actionPerformed( ActionEvent e ) {
                System.out.println("Link clicked: " + e);
            }
            
        } );
        
        
        f.getContentPane().setLayout( new BoxLayout( f.getContentPane(), BoxLayout.LINE_AXIS )  );
        f.getContentPane().add( link );
        f.pack();
        f.setVisible( true );
        
        link.setMaximumSize( link.getSize() );
    }
    
}
