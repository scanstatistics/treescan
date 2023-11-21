/*
 * Copyright (C) 2007 Information Management Services, Inc.
 * Created on July 2, 2007
 */
package org.treescan.app;

import java.util.Enumeration;
import java.util.HashMap;
import java.util.Properties;

/**
 * 
 */
public final class AppConstants {

    private final static String APP_TITLE = "TreeScan - Software for the Tree-Based Scan Statistic";
    public static final int VERSION_MAJOR = 2;
    public static final int VERSION_MINOR = 2;
    public static final int VERSION_RELEASE = 0;
    public static final String VERSION_PHASE = "";

    public static final int MIN_YEAR = 1753;
    public static final int MAX_YEAR = 9999;
    public static final int MIN_GENERIC = -219145;
    public static final int MAX_GENERIC = 2921938;    
    
    public static final String getSoftwareTitle() {
        return APP_TITLE;
    }

    static public final String getGraphicalVersion() {
        StringBuilder version = new StringBuilder();
        version.append(VERSION_MAJOR).append(".").append(VERSION_MINOR);
        if (VERSION_RELEASE > 0) version.append(".").append(VERSION_RELEASE);
        if (VERSION_PHASE.length() > 0) version.append(" ").append(VERSION_PHASE);
        return version.toString();
    }
    
    static public final HashMap getEnviromentVariables() {
        HashMap variables = new HashMap();
        variables.put("TreeScan Application Version", getGraphicalVersion());
        variables.put("TreeScan Engine Version", getVersion());
        variables.put("TreeScan Version Id", getVersionId());
        Properties sysprops = System.getProperties();
        for ( Enumeration e = sysprops.propertyNames(); e.hasMoreElements(); ) {
            String key = (String)e.nextElement();
            variables.put(key, sysprops.getProperty( key )); 
        } 
        return variables;
    }

    native static public final String getVersion();

    native static public final String getWebSite();

    native static public final String getSubstantiveSupportEmail();

    native static public final String getTechnicalSupportEmail();

    native static public final String getReleaseDate();

    native static public final String getVersionId();
}
