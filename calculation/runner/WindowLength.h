//*****************************************************************************
#ifndef __WINDOW_LENGTH_H
#define __WINDOW_LENGTH_H
//*****************************************************************************
#include "DataTimeRanges.h"
#include "Parameters.h"

/** abstract base class for getting minimum/maximum window lengths */
class AbstractWindowLength {
    protected:
        const Parameters& _parameters;
        const int _minimum;
        const int _maximum;

    public:
        AbstractWindowLength(const Parameters& parameters, int minimum, int maximum) : _parameters(parameters), _minimum(minimum), _maximum(maximum) { }
        virtual ~AbstractWindowLength() {}

        virtual void reset() {}
        virtual int minimum() { return _minimum; }
        virtual int maximum() { return _maximum; }
        virtual void windowstart(const DataTimeRange& startWindow, int iWindowEnd, int& iMinWindowStart, int& iWindowStart) = 0;
};

/** fixed length minimum/maximum window lengths */
class WindowLength : public AbstractWindowLength {
    public:
        WindowLength(const Parameters& parameters, int minimum, int maximum) :   AbstractWindowLength(parameters, minimum, maximum) {}
        virtual ~WindowLength() {}

        virtual void windowstart(const DataTimeRange& startWindow, int iWindowEnd, int& iMinWindowStart, int& iWindowStart) {
            iMinWindowStart = std::max(iWindowEnd - _maximum, startWindow.getStart());
            iWindowStart = std::min(startWindow.getEnd(), iWindowEnd - _minimum);
        }
};

/** fixed length minimum/maximum window lengths with risk window restriction */
class RiskPercentageWindowLength : public AbstractWindowLength {
    protected:
        int _translated_zero_idx;
        double _risk_percentage;

    public:
        RiskPercentageWindowLength(const Parameters& parameters, int minimum, int maximum, int zero_idx) :
            AbstractWindowLength(parameters, minimum, maximum), _translated_zero_idx(zero_idx), _risk_percentage(_parameters.getRiskWindowPercentage() / 100.0) {}
        virtual ~RiskPercentageWindowLength() {}

        virtual void windowstart(const DataTimeRange& startWindow, int iWindowEnd, int& iMinWindowStart, int& iWindowStart) {
            iMinWindowStart = std::max(iWindowEnd - _maximum, startWindow.getStart());
            iWindowStart = std::min(startWindow.getEnd(), iWindowEnd - _minimum);
            if (iWindowEnd != _translated_zero_idx) {
                if (iWindowEnd > _translated_zero_idx) {
                    ///* r = (ew - q + 1)/(ew - x + 1), => q = -1 * (r(ew - x + 1) - ew - 1) */
                    //double q = -1.0 * (_risk_percentage * static_cast<double>(iWindowEnd - _translated_zero_idx + 1) - static_cast<double>(iWindowEnd) - 1.0);

                    /* r = (ew - q + 1)/(ew - x), => q = -1 * (r(ew - x) - ew - 1) */
                    double q = -1.0 * (_risk_percentage * static_cast<double>(iWindowEnd - _translated_zero_idx) - static_cast<double>(iWindowEnd) - 1.0);

                    //int initMinWindowStart = iMinWindowStart;
                    //int initWindowStart = iWindowStart;

                    //iMinWindowStart = std::max(iMinWindowStart, _translated_zero_idx);
                    iMinWindowStart = std::max(iMinWindowStart, _translated_zero_idx + 1); // minimum is index of true one

                    iWindowStart = std::min(iWindowStart, static_cast<int>(std::floor(q)));

                    //printf("iWindowEnd %d iMinWindowStart,iWindowStart [%d, %d (%.2lf%%)] -> [%d, %d (%.2lf%%)]\n", iWindowEnd,
                    //       initMinWindowStart, initWindowStart, 100.0 * static_cast<double>(iWindowEnd - initWindowStart + 1) / static_cast<double>(iWindowEnd - _translated_zero_idx/* + 1*/),
                    //       iMinWindowStart, iWindowStart, 100.0 * static_cast<double>(iWindowEnd - iWindowStart + 1) / static_cast<double>(iWindowEnd - _translated_zero_idx/* + 1*/));
                } else {
                    ///* r = (ew - q + 1)/(x - q + 1), => q = (r(x + 1) - ew - 1)/(r - 1) */
                    //double q = (_risk_percentage * (static_cast<double>(_translated_zero_idx + 1)) - static_cast<double>(iWindowEnd) - 1.0) / (_risk_percentage - 1.0);

                    /* r = (ew - q + 1)/(x - q), => q = (rx - ew - 1)/(r - 1) */
                    double q = (_risk_percentage * (static_cast<double>(_translated_zero_idx)) - static_cast<double>(iWindowEnd) - 1.0) / (_risk_percentage - 1.0);

                    //int initWindowStart = iWindowStart;
                    iWindowStart = std::min(iWindowStart, static_cast<int>(std::floor(q)));

                    //printf("iWindowEnd %d iMinWindowStart,iWindowStart [%d, %d (%.2lf%%)] -> [%d, %d (%.2lf%%)]\n", iWindowEnd,
                    //       iMinWindowStart, initWindowStart, 100.0 * static_cast<double>(iWindowEnd - initWindowStart + 1) / static_cast<double>(_translated_zero_idx - initWindowStart /*+ 1*/),
                    //       iMinWindowStart, iWindowStart, 100.0 * static_cast<double>(iWindowEnd - iWindowStart + 1) / static_cast<double>(_translated_zero_idx - iWindowStart /*+ 1*/));
                }
            }
    }
};

//*****************************************************************************
#endif
