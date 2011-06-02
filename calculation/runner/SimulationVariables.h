//*****************************************************************************
#ifndef __SimulationVariables_H
#define __SimulationVariables_H
//*****************************************************************************
#include "TreeScan.h"

class SimulationVariables {
  private:
    unsigned int    _simulations_count;
	double		    _sum_llr;
	double		    _sum_squared_llr;
    double          _mlc_llr;
    unsigned int    _greater_llr_count;

    //power X, Y ???

    void            init(double mlc_llr) {
                        _simulations_count=0;
                        _sum_llr=0;
                        _sum_squared_llr=0;
                        _mlc_llr=mlc_llr;
                        _greater_llr_count=0;
                    }

  public:
    SimulationVariables() {init(0.0);}
    virtual ~SimulationVariables() {}

    void            add_llr(double llr) {_sum_llr += llr; 
                                         _sum_squared_llr += std::pow(llr, 2);
                                         if (macro_less_than(_mlc_llr, llr, DBL_CMP_TOLERANCE))  ++_greater_llr_count;
                    }
    double          get_mean() const {return _sum_llr/static_cast<double>(_simulations_count);}
    unsigned int    get_greater_llr_count() const {return _greater_llr_count;}
    unsigned int    get_sim_count() const {return _simulations_count;}
    double          get_standard_deviation() const {return std::sqrt(get_variance());}
    double          get_variance() const {return _sum_squared_llr/static_cast<double>(_simulations_count) - std::pow(get_mean(), 2);}
    unsigned int    increment_sim_count() {return ++_simulations_count;}
    void            reset(double mlc_llr) {init(mlc_llr);}
    void            set_sim_count_explicit(unsigned int i) {_simulations_count = i;}
};

//*****************************************************************************
#endif
