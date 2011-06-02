//******************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//******************************************************************************
#include "MonteCarloSimFunctor.h"
#include "PrjException.h"

/* constructor */
MCSimSuccessiveFunctor::MCSimSuccessiveFunctor(boost::mutex& Mutex, 
                                               boost::shared_ptr<AbstractRandomizer> randomizer,
                                               const ScanRunner& scanRunner) : gMutex(Mutex), _randomizer(randomizer), _scanRunner(scanRunner) {
    _simData.resize(_scanRunner._Nodes.size(), std::make_pair(0,0));
    _loglikelihood = _scanRunner.getLoglikelihood();
}

MCSimSuccessiveFunctor::result_type MCSimSuccessiveFunctor::operator() (MCSimSuccessiveFunctor::param_type const & param) {
    result_type temp_result;
    try {
        //randomize data
        int TotalSimC = _randomizer.get()->RandomizeData(param, _scanRunner._Nodes, _simData);
        
        //print simulation data to file, if requested
        //if (gDataHub.GetParameters().GetOutputSimulationData()) {
        //    boost::mutex::scoped_lock     lock(gMutex);
        //    for (size_t t=0; t < gpSimulationDataContainer->size(); ++t)
        //        gDataWriter->write((*(*gpSimulationDataContainer)[t]), gDataHub.GetParameters());
        //}

        //--------------------- SCANNING THE TREE, SIMULATIONS -------------------------
        double SimLogLikelihood=0;

        for (size_t i=0; i < _scanRunner._Nodes.size(); i++) {
            //if (_Nodes.at(i)->_SimBrC > 1)
            if (_simData.at(i).second > 1)
                SimLogLikelihood = std::max(SimLogLikelihood, _loglikelihood->LogLikelihood(_simData.at(i).second/*_Nodes.at(i)->_SimBrC*/, _scanRunner._Nodes.at(i)->_BrN));
        } // for i<nNodes

        temp_result.dSuccessfulResult = std::make_pair(SimLogLikelihood,TotalSimC);
        temp_result.bUnExceptional = true;
    } catch (memory_exception & e) {
        temp_result.eException_type = MCSimJobSource::result_type::memory;
        temp_result.Exception = prg_exception(e.what(), "MCSimSuccessiveFunctor");
        temp_result.bUnExceptional = false;
    } catch (prg_exception & e) {
        temp_result.eException_type = MCSimJobSource::result_type::prg;
        temp_result.Exception = e;
        temp_result.bUnExceptional = false;
    } catch (std::exception & e) {
        temp_result.eException_type = MCSimJobSource::result_type::std;
        temp_result.Exception = prg_exception(e.what(), "MCSimSuccessiveFunctor");
        temp_result.bUnExceptional = false;
    } catch (...) {
        temp_result.eException_type = MCSimJobSource::result_type::unknown;
        temp_result.Exception = prg_exception("(...) -- unknown error", "MCSimSuccessiveFunctor");
        temp_result.bUnExceptional = false;
    }

    if (!temp_result.bUnExceptional) {
        temp_result.Exception.addTrace("operator()", "MCSimSuccessiveFunctor");
    }

    return temp_result;
}
