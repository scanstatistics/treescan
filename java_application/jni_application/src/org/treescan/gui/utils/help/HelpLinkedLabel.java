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
 */
import java.awt.Color;
import org.treescan.gui.utils.*;
import java.awt.event.*;

public class HelpLinkedLabel extends JHyperLink {
    public HelpLinkedLabel(final String title, final String helpID) {
        super(title, Color.BLACK);
        this.addActionListener(new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                HelpShow.showHelp(helpID);
            }
        });
    }
}