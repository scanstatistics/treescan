//***************************************************************************
#ifndef __AsciiPrintFormat_H
#define __AsciiPrintFormat_H
//***************************************************************************
#include <iostream>
#include <fstream>

/** Print formatter for aiding in the process of creating the primary ASCII
    results output file.

    NOTE: The values of the static variables are somewhat determined
          empirically. The code has been written to be tested to be adjustable
          only in repects to expanding the width of the output area. */
class AsciiPrintFormat {
   private:
     unsigned int               giLeftMargin;
     unsigned int               giDataLeftMargin;
     unsigned int               giLabelWidth;
     bool                       gbOneDataSet;
     static const unsigned int  giOneDataSetCutLabelWidth;
     static const unsigned int  giOneDataSetSummuaryLabelWidth;
     static const unsigned int  giMultiDataSetCutLabelWidth;
     static const unsigned int  giMultiDataSetSummaryLabelWidth;
     static const unsigned int  giRunTimeComponentsLabelWidth;
     static const unsigned int  giRightMargin;
     static const unsigned int  giVersionHeaderWidth;

   public:
     AsciiPrintFormat(bool bOneDataSet=true);
     virtual ~AsciiPrintFormat();

     void                       PrintAlignedMarginsDataString(std::ofstream& out, const std::string& sDataString, unsigned int iPostNewlines=1) const;
     void                       PrintNonRightMarginedDataString(std::ofstream& out, const std::string& sDataString, bool bPadLeftMargin, unsigned int iPostNewlines=1) const;
     void                       PrintSectionLabel(std::ofstream& out, const char* sText, bool bPadLeftMargin) const;
     static void                PrintSectionSeparatorString(std::ofstream& out, unsigned int iPreNewlines=0, unsigned int iPostNewlines=1, char cSeparator='_');
     static void                PrintVersionHeader(std::ofstream& out);
     void                       SetMarginsAsCutSection(unsigned int iNumber);
     void                       SetMarginsAsOverviewSection();
     void                       SetMarginsAsRunTimeReportSection();
     void                       SetMarginsAsSummarySection();
};
//***************************************************************************
#endif
