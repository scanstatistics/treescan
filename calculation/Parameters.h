//*****************************************************************************
#ifndef __PARAMETERS_H_
#define __PARAMETERS_H_
//*****************************************************************************
#include "UtilityFunctions.h"
#include "FileName.h"
#include "DataTimeRanges.h"

class Parameters {
  public:
    typedef std::pair<unsigned int,unsigned int> ratio_t;
    enum ParameterType {/* Input */
                        TREE_FILE=1,
                        COUNT_FILE,
                        DATA_TIME_RANGES,
                        /* Advanced Input */
                        CUT_FILE,
                        CUT_TYPE,
                        DUPLICATES,
                        /* Analysis */
                        SCAN_TYPE,
                        CONDITIONAL_TYPE,
                        MODEL_TYPE,
                        EVENT_PROBABILITY,
                        START_DATA_TIME_RANGE,
                        END_DATA_TIME_RANGE,
                        /* Advanced Analysis - Temporal Window */
                        MAXIMUM_WINDOW_PERCENTAGE,
                        MAXIMUM_WINDOW_FIXED,
                        MAXIMUM_WINDOW_TYPE,
                        MINIMUM_WINDOW_FIXED,
                        /* Advanced Analysis - Adjustments */
                        DAYOFWEEK_ADJUSTMENT,
                        /* Advanced Analysis - Inference */
                        REPLICATIONS,
                        RANDOMIZATION_SEED,
                        RANDOMLY_GENERATE_SEED,
                        /* Output */
                        RESULTS_FILE,
                        RESULTS_HTML,
                        RESULTS_CSV,
                        RESULTS_LLR,
                        REPORT_CRITICAL_VALUES,
                        /* power evaluations */
                        POWER_EVALUATIONS,
                        POWER_EVALUATION_TYPE,
                        CRITICAL_VALUES_TYPE,
                        CRITICAL_VALUE_05,
                        CRITICAL_VALUE_01,
                        CRITICAL_VALUE_001,
                        POWER_EVALUATION_TOTALCASES,
                        POWER_EVALUATIONS_REPLICA,
                        POWER_EVALUATIONS_FILE,
                        /* power simulations */
                        READ_SIMULATIONS,
                        INPUT_SIM_FILE,
                        WRITE_SIMULATIONS,
                        OUTPUT_SIM_FILE,
                        /* Run Options */
                        PARALLEL_PROCESSES,
                        /* System */
                        CREATION_VERSION
    };
    /** power evaluation method */
    enum PowerEvaluationType
    {
        PE_WITH_ANALYSIS=0,             /* execute standard analysis and power evaluation together */
        PE_ONLY_CASEFILE,               /* execute only power evaluation, using total cases from case file */
        PE_ONLY_SPECIFIED_CASES         /* execute only power evaluation, using user specified total cases */
    };
    /** critical values calculation type */
    enum CriticalValuesType
    {
        CV_MONTECARLO=0,                /* standard monte carlo */
        CV_POWER_VALUES                 /* user specified values */
    };
    enum ResultsFormat {TEXT=0};
    enum ParametersFormat {XML=0, JSON};
    enum ModelType {POISSON=0, BERNOULLI, UNIFORM, MODEL_NOT_APPLICABLE};
    enum CutType {SIMPLE=0, PAIRS, TRIPLETS, ORDINAL, COMBINATORIAL};
    enum ScanType {TREEONLY=0, TREETIME, TIMEONLY};
    enum ConditionalType {UNCONDITIONAL=0, TOTALCASES, NODE, NODEANDTIME};
    enum MaximumWindowType {PERCENTAGE_WINDOW=0, FIXED_LENGTH};
    typedef std::map<std::string,Parameters::CutType> cut_map_t;
    typedef std::pair<cut_map_t, cut_map_t> cut_maps_t;

    struct CreationVersion {
        unsigned int iMajor;
        unsigned int iMinor;
        unsigned int iRelease;

        bool operator<(const CreationVersion& other) const {
            if (iMajor == other.iMajor) {
                if (iMinor == other.iMinor) {
                    return iRelease < other.iRelease;
                } else {
                    return iMinor < other.iMinor;
                }
            } else {
                return iMajor < other.iMajor;
            }
        }
        bool operator==(const CreationVersion& other) const {
            return iMajor == other.iMajor && iMinor == other.iMinor && iRelease == other.iRelease;
        }
        bool operator!=(const CreationVersion& other) const {
            return !(*this == other);
        }
    };

  private:
    unsigned int                        _numRequestedParallelProcesses;
    unsigned int                        _replications;
    std::string                         _parametersSourceFileName;
    std::string                         _treeFileName;
    std::string                         _cutsFileName;
    std::string                         _countFileName;
    DataTimeRangeSet                    _dataTimeRangeSet;
    DataTimeRange                       _temporalStartRange;
    DataTimeRange                       _temporalEndRange;
    std::string                         _outputFileName;
    ResultsFormat                       _resultsFormat;
    struct CreationVersion              _creationVersion;
    long                                _randomizationSeed;
    bool                                _randomlyGenerateSeed;
    bool                                _duplicates;
    bool                                _generateHtmlResults;
    bool                                _generateTableResults;
    bool                                _printColumnHeaders;
    ModelType                           _modelType;
    ratio_t                             _probablility_ratio;
    CutType                             _cut_type;
    ScanType                            _scan_type;
    ConditionalType                     _conditional_type;
    double                              _maximum_window_percentage;
    unsigned int                        _maximum_window_length;
    MaximumWindowType                   _maximum_window_type;
    unsigned int                        _minimum_window_length;
    bool                                _generate_llr_results;
    bool                                _read_simulations;
    std::string                         _input_sim_file;
    bool                                _write_simulations;
    std::string                         _output_sim_file;
    bool                                _report_critical_values;
    bool                                _perform_power_evaluations;
    PowerEvaluationType                 _power_evaluation_type;
    CriticalValuesType                  _critical_values_type;
    double                              _critical_value_05;  /* user specified critical value at .05 */
    double                              _critical_value_01;  /* user specified critical value at .01 */
    double                              _critical_value_001; /* user specified critical value at .001 */
    int                                 _power_evaluation_totalcases;
    unsigned int                        _power_replica; /** number of replicas in power step  of power evaluation */
    std::string                         _power_alt_hypothesis_filename;
    bool                                _dayofweek_adjustment;

    void                                assignMissingPath(std::string & sInputFilename, bool bCheckWritable=false);
    void                                copy(const Parameters &rhs);
    const char                        * getRelativeToParameterName(const FileName& fParameterName, const std::string& sFilename, std::string& sValue) const;

  public:
    Parameters() {setAsDefaulted();}
    Parameters(const Parameters &other) {setAsDefaulted(); copy(other);}
    ~Parameters() {}

    static const int                    giNumParameters; /** number enumerated parameters */

    Parameters                        & operator=(const Parameters &rhs)  {if (this != &rhs) copy(rhs); return (*this);}
    bool                                operator==(const Parameters& rhs) const;
    bool                                operator!=(const Parameters& rhs) const {return !(*this == rhs);}

    ConditionalType                     getConditionalType() const {return _conditional_type;}
    const std::string                 & getCountFileName() const {return _countFileName;}
    const CreationVersion             & getCreationVersion() const {return _creationVersion;}
    double                              getCriticalValue05() const {return _critical_value_05;}
    double                              getCriticalValue01() const {return _critical_value_01;}
    double                              getCriticalValue001() const {return _critical_value_001;}
    CriticalValuesType                  getCriticalValuesType() const {return _critical_values_type;}
    const std::string                 & getCutsFileName() const {return _cutsFileName;}
    CutType                             getCutType() const {return _cut_type;}
    static cut_maps_t                   getCutTypeMap();
    const DataTimeRangeSet            & getDataTimeRangeSet() const {return _dataTimeRangeSet;}
    const std::string                 & getInputSimulationsFilename() const {return _input_sim_file;}
    unsigned int                        getMaximumWindowInTimeUnits() const;
    unsigned int                        getMaximumWindowLength() const {return _maximum_window_length;}
    double                              getMaximumWindowPercentage() const {return _maximum_window_percentage;}
    MaximumWindowType                   getMaximumWindowType() const {return _maximum_window_type;}
    unsigned int                        getMinimumWindowLength() const {return _minimum_window_length;}
    ModelType                           getModelType() const {return _modelType;}
    unsigned int                        getNumParallelProcessesToExecute() const;
    int                                 getNumReadParameters() const {return giNumParameters;}
    unsigned int                        getNumReplicationsRequested() const {return _replications;}
    unsigned int                        getNumRequestedParallelProcesses() const {return _numRequestedParallelProcesses;}
    const std::string                 & getOutputFileName() const {return _outputFileName; }
    const std::string                 & getOutputSimulationsFilename() const {return _output_sim_file;}
    bool                                getPerformDayOfWeekAdjustment() const {return _dayofweek_adjustment;}
    bool                                getPerformPowerEvaluations() const {return _perform_power_evaluations;}
    const std::string                 & getPowerEvaluationAltHypothesisFilename() const {return _power_alt_hypothesis_filename;}
    int                                 getPowerEvaluationTotalCases() const {return _power_evaluation_totalcases;}
    PowerEvaluationType                 getPowerEvaluationType() const {return _power_evaluation_type;}
    unsigned int                        getPowerEvaluationReplications() const {return _power_replica;}
    double                              getProbability() const {return static_cast<double>(_probablility_ratio.first)/static_cast<double>(_probablility_ratio.second);}
    ratio_t                             getProbabilityRatio() const {return _probablility_ratio;}
    long                                getRandomizationSeed() const {return _randomizationSeed;}
    bool                                getReportCriticalValues() const {return _report_critical_values;}
    ResultsFormat                       getResultsFormat() const {return _resultsFormat;}
    ScanType                            getScanType() const {return _scan_type;}
    const std::string                 & getSourceFileName() const {return _parametersSourceFileName;}
    const DataTimeRange               & getTemporalEndRange() const {return _temporalEndRange;}
    const DataTimeRange               & getTemporalStartRange() const {return _temporalStartRange;}
    const std::string                 & getTreeFileName() const {return _treeFileName;}

    bool                                isDuplicates() const {return _duplicates;}
    bool                                isGeneratingHtmlResults() const {return _generateHtmlResults;}
    bool                                isGeneratingLLRResults() const {return _generate_llr_results;}
    bool                                isGeneratingTableResults() const {return _generateTableResults;}
    bool                                isPerformingDayOfWeekAdjustment() const {return isTemporalScanType(_scan_type) && _dayofweek_adjustment;}
    bool                                isPrintColumnHeaders() const {return _printColumnHeaders;}
    bool                                isRandomlyGeneratingSeed() const {return _randomlyGenerateSeed;}
    bool                                isReadingSimulationData() const {return _read_simulations;}
    static bool                         isSpatialScanType(ScanType e) {return e == Parameters::TREEONLY || e == Parameters::TREETIME;}
    static bool                         isTemporalScanType(ScanType e) {return e == Parameters::TIMEONLY || e == Parameters::TREETIME;}
    bool                                isWritingSimulationData() const {return _write_simulations;}

    void                                setAsDefaulted();
    void                                setConditionalType(ConditionalType e) {_conditional_type = e;}
    void                                setCountFileName(const char * sCountFileName, bool bCorrectForRelativePath=false);
    void                                setCriticalValue05(double d) {_critical_value_05 = d;}
    void                                setCriticalValue01(double d) {_critical_value_01 = d;}
    void                                setCriticalValue001(double d) {_critical_value_001 = d;}
    void                                setCriticalValuesType(CriticalValuesType e) {_critical_values_type = e;}
    void                                setCutsFileName(const char * sCutsFileName, bool bCorrectForRelativePath=false);
    void                                setCutType(CutType e) {_cut_type = e;}
    void                                setDataTimeRangeSet(const DataTimeRangeSet& set) {_dataTimeRangeSet = set;}
    void                                setDuplicates(bool b) {_duplicates = b;}
    void                                setGeneratingHtmlResults(bool b) {_generateHtmlResults = b;}
    void                                setGeneratingLLRResults(bool b) {_generate_llr_results = b;}
    void                                setGeneratingTableResults(bool b) {_generateTableResults = b;}
    void                                setInputSimulationsFilename(const char * s, bool bCorrectForRelativePath=false);
    void                                setMaximumWindowLength(unsigned int u) {_maximum_window_length = u;}
    void                                setMaximumWindowPercentage(double d) {_maximum_window_percentage = d;}
    void                                setMaximumWindowType(MaximumWindowType e) {_maximum_window_type = e;}
    void                                setMinimumWindowLength(unsigned int u) {_minimum_window_length = u;}
    void                                setModelType(ModelType e) {_modelType = e;}
    void                                setNumProcesses(unsigned int i) {_numRequestedParallelProcesses = i;}
    void                                setNumReplications(unsigned int replications) {_replications = replications;}
    void                                setOutputFileName(const char * sOutPutFileName, bool bCorrectForRelativePath=false);
    void                                setOutputSimulationsFilename(const char * s, bool bCorrectForRelativePath=false);
    void                                setPerformDayOfWeekAdjustment(bool b) {_dayofweek_adjustment = b;}
    void                                setPerformPowerEvaluations(bool b) {_perform_power_evaluations = b;}
    void                                setPowerEvaluationReplications(unsigned int i) {_power_replica = i;}
    void                                setPowerEvaluationAltHypothesisFilename(const char * s, bool bCorrectForRelativePath=false);
    void                                setPowerEvaluationTotalCases(int i) {_power_evaluation_totalcases = i;}
    void                                setPowerEvaluationType(PowerEvaluationType e) {_power_evaluation_type = e;}
    void                                setPrintColumnHeaders(bool b) {_printColumnHeaders=b;}
    void                                setProbabilityRatio(ratio_t r) {_probablility_ratio = r;}
    void                                setRandomizationSeed(long lSeed) {_randomizationSeed = lSeed;}
    void                                setRandomlyGeneratingSeed(bool b) {_randomlyGenerateSeed = b;}
    void                                setReadingSimulationData(bool b) {_read_simulations = b;}
    void                                setReportCriticalValues(bool b) {_report_critical_values = b;}
    void                                setResultsFormat(ResultsFormat e) {_resultsFormat = e;}
    void                                setScanType(ScanType e) {_scan_type = e;}
    void                                setSourceFileName(const char * sParametersSourceFileName);
    void                                setTemporalEndRange(const DataTimeRange& range) {_temporalEndRange = range;}
    void                                setTemporalStartRange(const DataTimeRange& range) {_temporalStartRange = range;}
    void                                setTreeFileName(const char * sTreeFileName, bool bCorrectForRelativePath=false);
    void                                setWritingSimulationData(bool b) {_write_simulations = b;}
    void                                setVersion(const CreationVersion& vVersion) {_creationVersion = vVersion;}

    void read(const std::string &filename, ParametersFormat type);
    void write(const std::string &filename, ParametersFormat type) const;
};
//*****************************************************************************
#endif
