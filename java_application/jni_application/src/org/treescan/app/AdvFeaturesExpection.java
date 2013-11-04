package org.treescan.app;

import java.awt.Component;
import org.treescan.gui.AdvancedParameterSettingsFrame;

/**
 * Exception class that note the component and containing that 
 * that caused exception.
 */
public class AdvFeaturesExpection extends RuntimeException {

    private static final long serialVersionUID = 1L;
    public final Component focusComponent;
    public final AdvancedParameterSettingsFrame.FocusedTabSet focusTab;

    public AdvFeaturesExpection(AdvancedParameterSettingsFrame.FocusedTabSet focusTab, Component focusComponent) {
        super();
        this.focusComponent = focusComponent;
        this.focusTab = focusTab;
    }

    public AdvFeaturesExpection(String arg0, AdvancedParameterSettingsFrame.FocusedTabSet focusTab, Component focusComponent) {
        super(arg0);
        this.focusComponent = focusComponent;
        this.focusTab = focusTab;
    }

    public AdvFeaturesExpection(String arg0, Throwable arg1, AdvancedParameterSettingsFrame.FocusedTabSet focusTab, Component focusComponent) {
        super(arg0, arg1);
        this.focusComponent = focusComponent;
        this.focusTab = focusTab;
    }

    public AdvFeaturesExpection(Throwable arg0, AdvancedParameterSettingsFrame.FocusedTabSet focusTab, Component focusComponent) {
        super(arg0);
        this.focusComponent = focusComponent;
        this.focusTab = focusTab;
    }
}
