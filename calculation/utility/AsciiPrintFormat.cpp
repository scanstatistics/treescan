//***************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//***************************************************************************
#include "AsciiPrintFormat.h"
#include "UtilityFunctions.h"
#include "PrjException.h"
#include <errno.h>

/** width of label with one dataset for cut section */
const unsigned int AsciiPrintFormat::giOneDataSetCutLabelWidth   = 22;
/** width of label for summary section with one data set */
const unsigned int AsciiPrintFormat::giOneDataSetSummuaryLabelWidth  = 30;
/** width of label for cut section with multiple datasets */
const unsigned int AsciiPrintFormat::giMultiDataSetCutLabelWidth = 32;
/** width of label for summary section with multiple datasets */
const unsigned int AsciiPrintFormat::giMultiDataSetSummaryLabelWidth = 38;
/** width of label for run-time components section */
const unsigned int AsciiPrintFormat::giRunTimeComponentsLabelWidth   = 35;
/** width of output area - this value is untested below 64 */
const unsigned int AsciiPrintFormat::giRightMargin                   = 80;
/** width of version header section */
const unsigned int AsciiPrintFormat::giVersionHeaderWidth            = 29;

/** constructor */
AsciiPrintFormat::AsciiPrintFormat(bool bOneDataSet) : gbOneDataSet(bOneDataSet) {
  SetMarginsAsOverviewSection();
}

/** destructor */
AsciiPrintFormat::~AsciiPrintFormat() {}

/** Prints data supplied by sDataString parameter to file in a manner
    such that data is aligned with left margin of section label and wraps when
    data will exceed right margin.
    NOTE: It is assumed that the corresponding section label has been printed to
          file, so that the first line of data string is printed to the
          (giRightMargin - giDataLeftMargin)'th character beyond beginning of a
          line. Subsequent lines that wrap do incorporate appropriate padding of
          blanks. */
void AsciiPrintFormat::PrintAlignedMarginsDataString(std::ofstream& out, const std::string& sDataString, unsigned int iPostNewlines) const {
  unsigned int  iStart, iScan, iPrint, iDataPrintWidth;

  iStart = 0;
  //calculate number of characters in the data print area
  iDataPrintWidth = giRightMargin - giDataLeftMargin;
  //if data string is wider than print area then cause it to wrap
  while (iStart + iDataPrintWidth < sDataString.size()) {
      //scan backwards from iStart + iDataPrintWidth, looking for blank to replace
      iScan = iStart + iDataPrintWidth;
      while (iScan > iStart) {
           if (sDataString[iScan] == ' ' || sDataString[iScan] == '\n') {
             //found insertion point - first print characters up to iScan
             for (iPrint=iStart; iPrint < iScan; ++iPrint)
               out << sDataString[iPrint]; //putChar(sDataString[iPrint], fp);
             //print newline character - wrap
             out << std::endl; 
             //pad with blanks to align data
             for (iPrint=0; iPrint < giDataLeftMargin; ++iPrint)
               out << " ";
             iStart = iScan + 1/*replaced blank character*/;
             break;
           }
           iScan--;
      }
      //no blank found, so cause data to wrap at right margin
      if (iScan == iStart) {
        //print characters up to iDataPrintWidth
        for (iPrint=iStart; iPrint < iStart + iDataPrintWidth; ++iPrint)
           out << sDataString[iPrint]; //putChar(sDataString[iPrint], fp);
        //print newline - wrap
        out << std::endl;
        //pad with blanks to align data
        for (iPrint=0; iPrint < giDataLeftMargin; ++iPrint)
           out << " ";
        iStart += iDataPrintWidth;
      }
  }
  //print remaining characters of sDataString
  for (iPrint=iStart; iPrint < sDataString.size(); ++iPrint)
     out << sDataString[iPrint];
  //append newlines as requested
  while (iPostNewlines-- > 0)
     out << std::endl;
}

/** Prints data string to file stream without consideration to right margin. */
void AsciiPrintFormat::PrintNonRightMarginedDataString(std::ofstream& out, const std::string& sDataString, bool bPadLeftMargin, unsigned int iPostNewlines) const {
  unsigned int  iPrint;

  //pad with blanks to align data
  for (iPrint=0; bPadLeftMargin && iPrint < giDataLeftMargin; ++iPrint)
     out << " ";

  for (iPrint=0; iPrint < sDataString.size(); ++iPrint)
       out << sDataString[iPrint];

  //append newlines as requested
  while (iPostNewlines-- > 0)
     out << std::endl;
}

/** Prints section label to file stream. */
void AsciiPrintFormat::PrintSectionLabel(std::ofstream& out, const char * sText, bool bPadLeftMargin) const {
  unsigned int   iStringLength, iFillLength, iPad=0;

  iStringLength = 0;
  //add left margin spacing if requested
  while (bPadLeftMargin && iPad++ < giLeftMargin) {
       out << ' ';
       ++iStringLength;
  }
  //add label
  out << sText;
  iStringLength += strlen(sText); //+= fprintf(fp, sText);

  //check that created label isn't greater than defined maximum width of label
  if (iStringLength > (bPadLeftMargin ? giLabelWidth + giLeftMargin : giLabelWidth))
    throw prg_error("Label text has length of %u, but defined max length is %u.\n", "PrintSectionLabel()",
                    iStringLength, (bPadLeftMargin ? giLabelWidth + giLeftMargin : giLabelWidth));
  //calculate fill length
  iFillLength = (bPadLeftMargin ? giLeftMargin + giLabelWidth : giLabelWidth);
  //fill remaining label space with '.'
  while (iStringLength < iFillLength) {
       out << ".";
       ++iStringLength;
  }
  //append label colon
  out << ": ";
  ++iStringLength;
}

/** Prints character cSeparator giRightMargin'th times. Prefixes/postfixes separator
    string newline character as specified by parameters, respectively. */
void AsciiPrintFormat::PrintSectionSeparatorString(std::ofstream& out, unsigned int iPreNewlines, unsigned int iPostNewlines, char cSeparator) {
  unsigned int iPrint = 0;

  while (iPreNewlines-- > 0)
     out << std::endl;

  while (iPrint++ < giRightMargin)
     out << cSeparator;

  while (iPostNewlines-- > 0)
     out << std::endl;
}

/** Prints version header to file stream */
void AsciiPrintFormat::PrintVersionHeader(std::ofstream& out) {
  unsigned int  iSeparatorsMargin, iTextMargin, iPrint;
  std::string   buffer;

  //calculate padding to center separators
  iSeparatorsMargin = (giRightMargin - giVersionHeaderWidth)/2;

  iPrint=0;
  while (iPrint++ < iSeparatorsMargin)
      out << ' ';
  iPrint=0;
  while (iPrint++ < giVersionHeaderWidth)
      out << '_'; 
  out << std::endl << std::endl;
  printString(buffer, 
              "TreeScan v%s.%s%s%s%s%s", 
              VERSION_MAJOR, 
              VERSION_MINOR, 
              (!strcmp(VERSION_RELEASE, "0") ? "" : "."), 
              (!strcmp(VERSION_RELEASE, "0") ? "" : VERSION_RELEASE), 
              (strlen(VERSION_PHASE) ? " " : ""), 
              VERSION_PHASE);
  iTextMargin = (giRightMargin - buffer.size())/2;
  iPrint=0;
  while (iPrint++ < iTextMargin)
     out << ' ';
  out << buffer << std::endl;

  iPrint=0;
  while (iPrint++ < iSeparatorsMargin)
     out << ' ';
  iPrint=0;
  while (iPrint++ < giVersionHeaderWidth)
     out << '_';
  out << std::endl << std::endl;
}

/** Adjusts left margin give width of iNumber in text. */
void AsciiPrintFormat::SetMarginsAsCutSection(unsigned int iNumber) {
  giLeftMargin=2;
  int n = (int)floor(((double)iNumber)/10);
  while (n > 0) {
      ++giLeftMargin;
      n = (int)floor(((double)n)/10);
  }
  //set margin for data print
  giDataLeftMargin = (gbOneDataSet ? giOneDataSetCutLabelWidth : giMultiDataSetCutLabelWidth) + giLeftMargin + strlen(": ");
  giLabelWidth = (gbOneDataSet ? giOneDataSetCutLabelWidth : giMultiDataSetCutLabelWidth);
}

/** Adjusts margins for run overview section. The overview section contains
    no labels, only text. */
void AsciiPrintFormat::SetMarginsAsOverviewSection() {
  //no labels in overview section - purely wrapping text
  giLeftMargin = giDataLeftMargin = giLabelWidth = 0;
}

/** Adjusts margins for run-time components section. */
void AsciiPrintFormat::SetMarginsAsRunTimeReportSection() {
  giLeftMargin = 2;
  giLabelWidth = giRunTimeComponentsLabelWidth;
  giDataLeftMargin = giLabelWidth + giLeftMargin + strlen(": ");
}

/** Adjusts margins for summary of data section. The left margin is zero and the
    label width and data margins are calculated. */
void AsciiPrintFormat::SetMarginsAsSummarySection() {
  giLeftMargin = 0;
  giLabelWidth = (gbOneDataSet ? giOneDataSetSummuaryLabelWidth : giMultiDataSetSummaryLabelWidth);
  giDataLeftMargin = giLabelWidth + giLeftMargin + strlen(": ");
}

