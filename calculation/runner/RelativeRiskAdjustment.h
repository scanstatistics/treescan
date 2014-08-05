//*****************************************************************************
#ifndef __ADJUSTMENTHANDLER_H
#define __ADJUSTMENTHANDLER_H
//*****************************************************************************
#include "TreeScan.h"
#include "DataTimeRanges.h"
#include <deque>
#include "ScanRunner.h"

class RelativeRiskAdjustment {
    private:
        double _relative_risk;
        DataTimeRange::index_t _start_idx;
        DataTimeRange::index_t _end_idx;

    public:
        RelativeRiskAdjustment(double rr) : _relative_risk(rr), _start_idx(0), _end_idx(0) {}
        RelativeRiskAdjustment(double rr, DataTimeRange::index_t startIdx, DataTimeRange::index_t endIdx) : _relative_risk(rr), _start_idx(startIdx), _end_idx(endIdx) {}

        DataTimeRange::index_t  getEndIdx() const {return _end_idx;}
        double                  getRelativeRisk() const {return _relative_risk;}
        DataTimeRange::index_t  getStartIdx() const {return _start_idx;}
        void                    multiplyRisk(double dRisk);
        void                    setEndIdx(DataTimeRange::index_t idx) {_end_idx = idx;}
        void                    setRelativeRisk(double rr) {_relative_risk = rr;}
        void                    setStartIdx(DataTimeRange::index_t idx) {_start_idx = idx;}
};

typedef std::deque<RelativeRiskAdjustment>          RelativeRiskContainer_t;
typedef RelativeRiskContainer_t::iterator           RelativeRiskContainerIterator_t;
typedef RelativeRiskContainer_t::const_iterator     RelativeRiskContainerIteratorConst_t;
typedef std::map<size_t, RelativeRiskContainer_t>   AdjustmentsContainer_t;
typedef AdjustmentsContainer_t::const_iterator      AdjustmentsIterator_t;

class RelativeRiskAdjustmentHandler {
    public:
        typedef std::vector<NodeStructure::ExpectedContainer_t> NodesExpectedContainer_t;
        typedef std::vector<double> NodesAdjustmentsContainer_t;

    private:
        AdjustmentsContainer_t  _adjustments;

        //count_t                   getCaseCount(count_t ** ppCumulativeCases, int iInterval, tract_t tTract) const;
        RelativeRiskContainerIterator_t  GetMaxPeriodIndex(RelativeRiskContainer_t& Container, DataTimeRange::index_t idx);

    public:
        RelativeRiskAdjustmentHandler() {}

        void                            add(size_t nodeId, double rr, DataTimeRange::index_t startIdx=0, DataTimeRange::index_t endIdx=0);
        void                            apply(NodesExpectedContainer_t& nodeExpected) const;
        NodesAdjustmentsContainer_t  &  getAsProbabilities(NodesAdjustmentsContainer_t& nodeProbabilities, double initial) const;
        void                            empty() {_adjustments.clear();}
        const AdjustmentsContainer_t &  get() const {return _adjustments;}
        void                            print();
};
#endif
