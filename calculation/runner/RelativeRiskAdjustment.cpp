//******************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//******************************************************************************
#include "RelativeRiskAdjustment.h"
#include "PrjException.h"

void RelativeRiskAdjustment::multiplyRisk(double rr) {
    try {
        if (rr > std::numeric_limits<double>::max()/_relative_risk)
            throw resolvable_error("Error: Data overflow occurs when adjusting expected number of cases.\n"
                                   "       The combined relative risk %g and %g in the adjustment file\n"
                                   "       is too large.\n", _relative_risk, rr);

        _relative_risk *= rr;
    } catch (prg_exception& x) {
        x.addTrace("multiplyRisk()", "RelativeRiskAdjustment");
        throw;
    }
}

////////////////// RelativeRiskAdjustmentHandler /////////////////////////

/** defines relative risk adjustment for passed data */
void RelativeRiskAdjustmentHandler::add(size_t nodeId, double rr, DataTimeRange::index_t startIdx, DataTimeRange::index_t endIdx) {
    size_t iStartIndex, iEndIndex;
    RelativeRiskContainerIterator_t itr_Start, itr_End;

    try {
        //find std::deque for node id
        if (_adjustments.find(nodeId) == _adjustments.end())
            _adjustments[nodeId] = RelativeRiskContainer_t();
        RelativeRiskContainer_t & nodeAdjustments = _adjustments[nodeId];

        //determine adjustment period indexes into existing adjustment periods
        if ((itr_Start = GetMaxPeriodIndex(nodeAdjustments, startIdx)) == nodeAdjustments.end()) {
            //start date does not exist or is beyond existing periods - just push onto back
            nodeAdjustments.push_back(RelativeRiskAdjustment(rr, startIdx, endIdx));
            return;
        } else
            iStartIndex = std::distance(nodeAdjustments.begin(), itr_Start);

        //create new period if new adjustment period goes beyond any existing period
        if ((itr_End = GetMaxPeriodIndex(nodeAdjustments, endIdx)) == nodeAdjustments.end()) {
            nodeAdjustments.push_back(RelativeRiskAdjustment(1, nodeAdjustments.back().getEndIdx() + 1, endIdx));
            iEndIndex = nodeAdjustments.size() - 1;
        } else
            iEndIndex = std::distance(nodeAdjustments.begin(), itr_End);

        if (iStartIndex == iEndIndex && startIdx < nodeAdjustments[iStartIndex].getStartIdx() && endIdx < nodeAdjustments[iStartIndex].getStartIdx())
            //new adjustment period entirely within period were there are no existing adjustments
            nodeAdjustments.insert(nodeAdjustments.begin() + iStartIndex, RelativeRiskAdjustment(rr, startIdx, endIdx));
        else if (iStartIndex == iEndIndex && startIdx > nodeAdjustments[iStartIndex].getStartIdx() && endIdx < nodeAdjustments[iStartIndex].getEndIdx()) {
            // neither date of new adjustment period is on existing adjustment periods boundry
            DataTimeRange::index_t TempIdx = nodeAdjustments[iStartIndex].getEndIdx();
            double TempRisk = nodeAdjustments[iStartIndex].getRelativeRisk();
            nodeAdjustments[iStartIndex].setEndIdx(startIdx - 1);
            nodeAdjustments.insert(nodeAdjustments.begin() + (iStartIndex + 1), RelativeRiskAdjustment(TempRisk * rr, startIdx, endIdx));
            nodeAdjustments.insert(nodeAdjustments.begin() + (iStartIndex + 2), RelativeRiskAdjustment(TempRisk, endIdx + 1, TempIdx));
        } else if (iStartIndex == iEndIndex && startIdx == nodeAdjustments[iStartIndex].getStartIdx() && endIdx == nodeAdjustments[iStartIndex].getEndIdx())
            nodeAdjustments[iStartIndex].multiplyRisk(rr);
        else { //now work with StartDate and EndDate separately
            if (startIdx < nodeAdjustments[iStartIndex].getStartIdx()) {
                //insert new adjustment before existing - the relative risk will be set in loop below
                nodeAdjustments.insert(nodeAdjustments.begin() + iStartIndex, RelativeRiskAdjustment(1, startIdx, nodeAdjustments[iStartIndex].getStartIdx() - 1));
                ++iEndIndex;
            } else if (startIdx > nodeAdjustments[iStartIndex].getStartIdx()) {
                DataTimeRange::index_t TempIdx = nodeAdjustments[iStartIndex].getEndIdx();
                double TempRisk = nodeAdjustments[iStartIndex].getRelativeRisk();
                //adjust existing periods end date and add new adjustment after it
                //the relative risk for added period will be updated in loop below
                nodeAdjustments[iStartIndex].setEndIdx(startIdx - 1);
                nodeAdjustments.insert(nodeAdjustments.begin() + (iStartIndex + 1), RelativeRiskAdjustment(TempRisk, startIdx, TempIdx));
                ++iStartIndex;
                ++iEndIndex;
            }
            //else falls on boundry -- nothing to do for StartDate
            //... and now the end date of new adjustment period
            if (endIdx < nodeAdjustments[iEndIndex].getStartIdx())
                //insert new adjustment period that fills period down to next lower existing period
                //the relative risk will be set below, before loop
                nodeAdjustments.insert(nodeAdjustments.begin() + iEndIndex, RelativeRiskAdjustment(1, nodeAdjustments[iEndIndex - 1].getEndIdx() + 1, endIdx));
            else if (endIdx >= nodeAdjustments[iEndIndex].getStartIdx() && endIdx < nodeAdjustments[iEndIndex].getEndIdx()) {
                DataTimeRange::index_t TempIdx = nodeAdjustments[iEndIndex].getStartIdx();
                double TempRisk = nodeAdjustments[iEndIndex].getRelativeRisk();
                //insert new period with upper limit determined by new adjustment end date
                //the relative risk will be set below, before loop
                nodeAdjustments[iEndIndex].setStartIdx(endIdx + 1);
                nodeAdjustments.insert(nodeAdjustments.begin() + iEndIndex, RelativeRiskAdjustment(TempRisk, TempIdx, endIdx));
            }
            //else falls on boundry - nothing to do for EndDate
            //at this point, adjustment at iEndIndex represents upper boundry for new period
            //that why we don't update risk in above code
            nodeAdjustments[iEndIndex].multiplyRisk(rr);
            //create new adjustment periods in existing adjustment period between StartDate and EndDate to cause continious adjustment period
            //ensure two things - the period from StartDate to EndDate is contiguous(i.e. fill 'open' periods between adjustments dates)
            //                  - update relative risk for existing adjustments within new period
            for (size_t i=iEndIndex; i > iStartIndex; --i) {
                if (nodeAdjustments[i].getStartIdx() - 1 != nodeAdjustments[i - 1].getEndIdx()) {
                    nodeAdjustments.insert(nodeAdjustments.begin() + i, RelativeRiskAdjustment(rr, nodeAdjustments[i - 1].getEndIdx() + 1, nodeAdjustments[i].getStartIdx() - 1));
                    ++i; // still need to check between inserted item and current i - 1 element
                } else //update relative risk
                    nodeAdjustments[i - 1].multiplyRisk(rr);
            }
        }
    } catch (prg_exception& x) {
        x.addTrace("add()", "RelativeRiskAdjustmentHandler");
        throw;
    }
}

void RelativeRiskAdjustmentHandler::apply(NodesExpectedContainer_t& nodeExpected) const {
    for (AdjustmentsContainer_t::const_iterator itr=_adjustments.begin(); itr != _adjustments.end(); ++itr) {
        NodeStructure::ExpectedContainer_t::iterator itrB = nodeExpected.at(itr->first).begin(), itrE = nodeExpected.at(itr->first).end();
        for (; itrB != itrE; ++itrB) {
            RelativeRiskContainer_t::const_iterator itrR = itr->second.begin(), itrRE = itr->second.end();
            for (; itrR != itrRE; ++itrR) {
                double rr = itrR->getRelativeRisk();
                *itrB *= itrR->getRelativeRisk();
            }
        }
    }
}

/* Gets the alternative hypothesis values as event probabilities. */
RelativeRiskAdjustmentHandler::NodesAdjustmentsContainer_t& RelativeRiskAdjustmentHandler::getAsProbabilities(NodesAdjustmentsContainer_t& nodeProbabilities, double initial) const {
    // iterate through collection of adjustments
    for (AdjustmentsContainer_t::const_iterator itr=_adjustments.begin(); itr != _adjustments.end(); ++itr) {
        // ensure that the container of nodes is large enough for this node's value
        if ((itr->first + 1) > nodeProbabilities.size()) nodeProbabilities.resize((itr->first + 1), initial);
        // sanity checks -- alternative hypothesis values much be a single value
        if (itr->second.size() != 1)
            throw prg_error("Number of adjustments expected to be one, got %u.", "RelativeRiskAdjustmentHandler::getAsProbabilities()", itr->second.size());
        // assign event probability to node
        nodeProbabilities.at(itr->first) = itr->second.front().getRelativeRisk();
    }
    return nodeProbabilities;
}

/** Returns the top most bounding index from existing adjustments periods.
    Iterator returned indicates that either date is within items period or in
    'open' period below item. If Date is beyond any existing items, iterator
    equals container.end().                                                   */
RelativeRiskContainerIterator_t RelativeRiskAdjustmentHandler::GetMaxPeriodIndex(RelativeRiskContainer_t & Container, DataTimeRange::index_t idx) {
    //find index
    RelativeRiskContainerIterator_t itr_index = Container.end();
    for (RelativeRiskContainerIterator_t itr=Container.begin(); itr != Container.end() && itr_index == Container.end(); ++itr)
        if (idx <= itr->getEndIdx())
            itr_index = itr;
    return itr_index;
}

/** Prints all defined adjustments to text file. */
void RelativeRiskAdjustmentHandler::print() {
/*

  AdjustmentsIterator_t                 itr;
  TractContainerIteratorConst_t         itr_deque;
  std::string                           sStart, sEnd;
  FILE                                * pFile=0;

  if ((pFile = fopen("c:\\Adustments.txt", "w")) == NULL)
    throw prg_error("Unable to create adjustments outpt file.","PrintAdjustments()");

  for (itr=gTractAdjustments.begin(); itr != gTractAdjustments.end(); ++itr) {
     const TractContainer_t & tract_deque = itr->second;
     fprintf(pFile, "Tract %s:\n", tHandler.getLocations().at(itr->first)->getIndentifier());
     for (itr_deque=tract_deque.begin(); itr_deque != tract_deque.end(); ++itr_deque) {
        JulianToString(sStart,(*itr_deque).GetStartDate(), DAY);
        JulianToString(sEnd, (*itr_deque).GetEndDate(), DAY);
        fprintf(pFile, "%lf\t%s\t%s\n", (*itr_deque).GetRelativeRisk(), sStart.c_str(), sEnd.c_str());
     }
     fprintf(pFile, "\n\n");
  }
  if (pFile) fclose(pFile);

*/
}

