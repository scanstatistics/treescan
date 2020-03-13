package org.treescan.app;

import java.awt.Component;

/**
 * Exception class that note the component and containing that 
 * that caused exception.
 */
public class AdvFeaturesExpectionSequential extends RuntimeException {

    private static final long serialVersionUID = 1L;
    public final Component focusComponent;

    public AdvFeaturesExpectionSequential(Component focusComponent) {
        super();
        this.focusComponent = focusComponent;
    }

    public AdvFeaturesExpectionSequential(String arg0, Component focusComponent) {
        super(arg0);
        this.focusComponent = focusComponent;
    }

    public AdvFeaturesExpectionSequential(String arg0, Throwable arg1, Component focusComponent) {
        super(arg0, arg1);
        this.focusComponent = focusComponent;
    }

    public AdvFeaturesExpectionSequential(Throwable arg0, Component focusComponent) {
        super(arg0);
        this.focusComponent = focusComponent;
    }
}
