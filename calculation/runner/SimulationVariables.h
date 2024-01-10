//*****************************************************************************
#ifndef __SimulationVariables_H
#define __SimulationVariables_H
//*****************************************************************************
#include "TreeScan.h"

class SimulationVariables {
    public:
        typedef std::pair<double,unsigned int> mlc_counter_t;
        typedef std::vector<mlc_counter_t> container_t;

    private:
        unsigned int    _simulations_count;
        double          _sum_llr;
        double          _sum_squared_llr;
        container_t     _mlc_llr;

        void init(double mlc_llr) {
            _simulations_count=0; 
            _sum_llr=0; 
            _sum_squared_llr=0; 
            _mlc_llr.clear();
            _mlc_llr.push_back(mlc_counter_t(mlc_llr,0));
        }

    public:
        SimulationVariables() {init(0.0);}
        virtual ~SimulationVariables() {}

        void add_additional_mlc(double mlc_llr) {
            _mlc_llr.push_back(mlc_counter_t(mlc_llr,0));
        }
        void add_llr(double llr) {
            _sum_llr += llr; 
            _sum_squared_llr += std::pow(llr, 2);
            for (container_t::iterator itr=_mlc_llr.begin(); itr != _mlc_llr.end(); ++itr)
                if (macro_less_than(itr->first, llr, DBL_CMP_TOLERANCE)) itr->second++;
        }
        double          get_mean() const {return _sum_llr/static_cast<double>(_simulations_count);}
        container_t     get_llr_counters() const {return _mlc_llr;}
        unsigned int    get_sim_count() const {return _simulations_count;}
        double          get_standard_deviation() const {return std::sqrt(get_variance());}
        double          get_variance() const {return _sum_squared_llr/static_cast<double>(_simulations_count) - std::pow(get_mean(), 2);}
        unsigned int    increment_sim_count() {return ++_simulations_count;}
        void            reset(double mlc_llr) {init(mlc_llr);}
        void            set_sim_count_explicit(unsigned int i) {_simulations_count = i;}
};
//*****************************************************************************
#endif
