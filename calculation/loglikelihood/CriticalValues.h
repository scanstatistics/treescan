//**********************************************************************************
#ifndef __CRITICAL_VALUES_H
#define __CRITICAL_VALUES_H
//**********************************************************************************
#include "TreeScan.h"

/** Maintains ordered list of significant log likelihood ratios as calculated during simulation process. */
class CriticalValues {
    public:
        typedef std::vector<double> container_t;
        typedef std::pair<bool,double> alpha_t;

        container_t _ratios;
        unsigned int _numReplications;

    public:
        CriticalValues(unsigned int iNumReplications);

        bool    add(double llr);
        alpha_t getAlpha01() const;
        alpha_t getAlpha05() const;
        alpha_t getAlpha001() const;
        alpha_t getAlpha0001() const;
        alpha_t getAlpha00001() const;
        void    reset();
};
//**********************************************************************************
#endif
