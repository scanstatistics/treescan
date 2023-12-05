/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package org.treescan.app;

/**
 * Exception class that notes the component that caused exception.
 */
public class RegionFeaturesException extends RuntimeException {

    private static final long serialVersionUID = 1L;
    public int _regionIndex=0;

    public RegionFeaturesException(String arg0) {
        super(arg0);
    }

    public RegionFeaturesException(String arg0, int regionIndex) {
        super(arg0);
        _regionIndex = regionIndex;
    }
    
    public RegionFeaturesException(String arg0, Throwable arg1) {
        super(arg0, arg1);
    }

    public RegionFeaturesException(Throwable arg0) {
        super(arg0);
    }
}
