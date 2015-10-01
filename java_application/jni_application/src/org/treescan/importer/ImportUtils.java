
package org.treescan.importer;

import java.util.ArrayList;
import org.apache.commons.lang.StringUtils;

/**
 * 
 * @author hostovic
 */
public class ImportUtils {
    /**
     * Strips sOpenGroupMarker and sCloseGroupMarker from the ends of sLine.
     * @param sLine - line to parse
     * @param sOpenGroupMarker - open group marker
     * @param sCloseGroupMarker - close group marker
     * @return string group markers removed
     */
    private static String trimGroupMarkers(String sLine, String sOpenGroupMarker, String sCloseGroupMarker) {
        //starts at either end, then chews its way toward the center, until a non-whitespace, non-groupmarker is encountered.
        int iStart;
        int iEnd;
        int iPositionInMarker;

        //at beginning of string
        iStart = 0;
        iPositionInMarker = 0;

        while (iStart < sLine.length() && iPositionInMarker < sOpenGroupMarker.length() &&
                (sLine.charAt(iStart) == sOpenGroupMarker.charAt(iPositionInMarker) ||
                Character.isWhitespace(sLine.charAt(iStart)))) {

            if (sLine.charAt(iStart) == sOpenGroupMarker.charAt(iPositionInMarker)) {
                iPositionInMarker++;
            } else {
                iPositionInMarker = 0;
            }

            iStart++;
        }

        //now shift to end of string

        iPositionInMarker = sCloseGroupMarker.length() - 1;
        iEnd = sLine.length();

        while (iEnd > iStart && iPositionInMarker >= 0 &&
                (sLine.charAt(iEnd - 1) == sCloseGroupMarker.charAt(iPositionInMarker) ||
                Character.isWhitespace(sLine.charAt(iEnd - 1)))) {

            if (sLine.charAt(iEnd - 1) == sCloseGroupMarker.charAt(iPositionInMarker)) {
                iPositionInMarker--;
            } else {
                iPositionInMarker = sCloseGroupMarker.length() - 1;
            }

            iEnd--;
        }

        return sLine.substring(iStart, iEnd);
    }

    /**
     * parseLine() with identical opening & closing group markers, always trims whitespace and eats group markers.
     * @param sLine - line to parse
     * @param sDelimiter - delimiter string
     * @param sGroupMarker - grouping string
     * @return 
     */
    public static ArrayList<String> parseLine(String sLine, String sDelimiter, String sGroupMarker) {
        return parseLine(sLine, sDelimiter, sGroupMarker, sGroupMarker, true, true);
    }

    /**
     * calls parseLine, providing sGroupMarker as both opening and closing group markers
     * @param sLine - line to parse
     * @param sDelimiter - delimiter string
     * @param sGroupMarker - grouping string
     * @param bTrimWhitespace - boolean, trim whitespace or not
     * @param bStripGroupMarkers - boolean, trim group markers or not
     * @return collection of values from parsed string
     */
    public static ArrayList<String> parseLine(String sLine, String sDelimiter, String sGroupMarker, boolean bTrimWhitespace, boolean bStripGroupMarkers) {
        return parseLine(sLine, sDelimiter, sGroupMarker, sGroupMarker, bTrimWhitespace, bStripGroupMarkers);
    }

    /** parses a string delimited by instances of sDelimiter.  Groups (blocks of text that should not
    be separated, even if a delimiter occurs in the middle of them) are denoted by placing sOpenGroupMarker
    where the group begins, and sCloseGroupMarker where the group ends.  Callers may indicate that they
    1.) want White space trimmed from the cell values by specifying bTrimWhitespace = true, and
    2.) want Group markers stripped (once the group has been identified and handled appropriately) by
    specifying bStripGroupMarkers = true.  NOTE that group markers will ONLY be stripped if nothing but
    whitespace lies between them and the surrounding delimiters.
    @param sLine The line to parse
    @param sDelimiter The field delimiter (often "," or ";")
    @param sOpenGroupMarker The open-quote string (often "\"")
    @param sCloseGroupMarker The close-quote string (often "\"")
    @param bTrimWhitespace   True to return strings with no leading or trailing whitespace.
    False to leave all whitespace between delimiters.
    @param bStripGroupMarkers True to
    Returns a ArrayList of Cells parsed from sLine. */
    public static ArrayList<String> parseLine(String sLine, String sDelimiter, String sOpenGroupMarker, String sCloseGroupMarker,
            boolean bTrimWhitespace, boolean bStripGroupMarkers) {

        //because we accept strings as delimiter and grouping markers, we have to do overlapping compares
        //to find matches.  We step through the length of sLine character by character.

        ArrayList<String> vList = new ArrayList<String>();
        StringBuffer buffer = new StringBuffer(""); //contains the text of our working cell

        boolean bGroupOpen = false; //indicates whether we're currently building a group (true
        //if we've found an instance of sOpenGroupMarker and are looking
        //for sCloseGroupMarker).

        int iPosition = 0;

        String s;

        sLine = StringUtils.strip(sLine);
        //if the delimiter is a single whitespace character, pre-process the line
        if (sDelimiter.length() == 1 && Character.isWhitespace(sDelimiter.charAt(0))) {
            StringBuilder temp = new StringBuilder();
            boolean lastIsWhiteSpace = false;
            //replace whitespace with single space character and compress contiguous whitespace
            for (int i = 0; i < sLine.length(); ++i) {
                if (Character.isWhitespace(sLine.charAt(i))) {
                    if (!lastIsWhiteSpace) {
                        temp.append(' ');
                    }
                    lastIsWhiteSpace = true;
                } else {
                    temp.append(sLine.charAt(i));
                    lastIsWhiteSpace = false;
                }
            }
            sLine = temp.toString();
        }

        while (iPosition < sLine.length()) {

            if (!bGroupOpen) { //we don't have a group open, so we're looking for either a delimiter or an open group marker
                //first check whether sOpenGroupMarker occurs at our current position in sLine.

                if (sLine.regionMatches(iPosition, sOpenGroupMarker, 0, sOpenGroupMarker.length())) {

                    //if we found sOpenGroupMarker, increment iPosition by its length and append
                    //the string to our buffer (we'll have the option to trim it later)
                    iPosition += sOpenGroupMarker.length();
//               buffer.append( sLine.substring( iOldPosition , iPosition ) );
                    buffer.append(sOpenGroupMarker);

                    bGroupOpen = true;

                //otherwise, if we didn't find sOpenGroupMarker check whether sDelimiter occurs at our current Position
                //of the line.
                } else if (sLine.regionMatches(iPosition, sDelimiter, 0, sDelimiter.length())) {

                    //if we found sDelimiter, increment iPosition by sDelimiter's length.
                    iPosition += sDelimiter.length();
                    s = buffer.toString();

                    //next convert buffer's contents to a String.  Strip whitespace and
                    //group markers if requested.
                    if (bStripGroupMarkers) {
                        s = trimGroupMarkers(s, sOpenGroupMarker, sCloseGroupMarker);
                    }

                    if (bTrimWhitespace) {
                        s = s.trim();
                    }

                    //add s to our ArrayList of cells and initialize buffer with a new, empty
                    //string.
                    vList.add(s);
                    buffer = new StringBuffer("");

                } else {
                    //if we didn't find either sOpenGroupMarker or sDelimiter, increment
                    //iPosition.
                    //Append the character at iPosition to buffer.
                    buffer.append(sLine.charAt(iPosition));

                    iPosition++;
                }

            } else {
                //we have a group open, so now we're only looking for sCloseGroupMarker.
                //sDelimiter is ignored.

                if (sLine.regionMatches(iPosition, sCloseGroupMarker, 0, sCloseGroupMarker.length())) {
                    iPosition += sCloseGroupMarker.length();
                    buffer.append(sCloseGroupMarker);

                    bGroupOpen = false;

                } else { //if we didn't find sCloseGroupMarker, append char at iPosition to buffer.
                    buffer.append(sLine.charAt(iPosition));

                    iPosition++;
                }
            }
        }

        //add remaining buffer to list
        s = buffer.toString();

        if (bStripGroupMarkers) {
            s = trimGroupMarkers(s, sOpenGroupMarker, sCloseGroupMarker);
        }

        if (bTrimWhitespace) {
            s = s.trim();
        }

        vList.add(s);

        return vList;
    }
}
