//*****************************************************************************
#ifndef __PARAMETERS_H_
#define __PARAMETERS_H_
//*****************************************************************************
#include "UtilityFunctions.h"
#include "FileName.h"
#include "DataTimeRanges.h"

class Parameters {
  public:
    typedef std::pair<std::string,std::string> ratio_t;
    enum ParameterType {// Input
                        TREE_FILE=1,
                        COUNT_FILE,
                        CONTROL_FILE,
                        DATE_PRECISION,
                        DATA_TIME_RANGES,
                        // Advanced Input
                        CUT_FILE,
                        CUT_TYPE,
                        DATA_ONLY_ON_LEAVES,
                        RELAXED_STUDY_DATA_PERIOD_CHECKING,
                        ALLOW_MULTI_PARENT_NODES,
                        ALLOW_MULTIPLE_ROOTS,
                        APPLY_RISK_WINDOW_RESTRICTION,
                        RISK_WINDOW_PERCENTAGE,
                        MINIMUM_CENSOR_TIME,
                        MINIMUM_CENSOR_PERCENTAGE,
                        RSK_WND_ALT_CENSOR_DENOM,
                        RSK_WND_CENSOR,
                        // Analysis
                        SCAN_TYPE,
                        CONDITIONAL_TYPE,
                        MODEL_TYPE,
                        EVENT_PROBABILITY,
                        VARIABLE_CASE_PROBABILITY,
                        SELF_CONTROL_DESIGN,
                        SEQUENTIAL_SCAN,
                        SEQUENTIAL_MIN_SIGNAL,
                        SEQUENTIAL_MAX_SIGNAL,
                        SEQUENTIAL_FILE,
                        SEQUENTIAL_ALPHA_OVERALL,
                        SEQUENTIAL_ALPHA_SPENDING,
                        PROSPECTIVE_ANALYSIS,
                        RESTRICTED_TIME_RANGE,
                        START_DATA_TIME_RANGE,
                        END_DATA_TIME_RANGE,
                        SCAN_RATE_TYPE,
                        // Advanced Analysis - Temporal Window
                        MAXIMUM_WINDOW_PERCENTAGE,
                        MAXIMUM_WINDOW_FIXED,
                        MAXIMUM_WINDOW_TYPE,
                        MINIMUM_WINDOW_FIXED,
                        // Advanced Analysis - Adjustments
                        DAYOFWEEK_ADJUSTMENT,
                        APPLY_EXCLUSION_RANGES,
                        EXCLUSION_RANGES,
                        // Advanced Analysis - Inference
                        REPLICATIONS,
                        RANDOMIZATION_SEED,
                        RANDOMLY_GENERATE_SEED,
                        RESTRICT_TREE_LEVELS,
                        RESTRICTED_TREE_LEVELS,
                        RESTRICT_EVALUATED_NODES,
                        NOT_EVALUATED_NODES_FILE,
                        MINIMUM_CASES_NODE,
                        PVALUE_REPORT_TYPE, /* p-value reporting type (enumeration) */
                        EARLY_TERM_THRESHOLD, /* early termination threshold (integer) */
                        /* Output */
                        RESULTS_FILE,
                        RESULTS_HTML,
                        RESULTS_CSV,
                        RESULTS_ASN,
                        RESULTS_NWK,
                        // Advanced Output - Additional Output
                        RESULTS_LLR,
                        REPORT_CRITICAL_VALUES,
                        REPORT_ATTR_RISK,
                        ATTR_RISK_NUM_EXPOSED,
                        INCLUDE_IDENTICAL_PARENT_CUTS,
                        OUTPUT_TEMPORAL_GRAPH,          // generate temporal graph output file
                        TEMPORAL_GRAPH_REPORT_TYPE,     // which clusters to generate temporal graph (enum)
                        TEMPORAL_GRAPH_MLC_COUNT,       // number of most likely clusters to generate temporal graph (integer)
                        TEMPORAL_GRAPH_CUTOFF,          // cutoff for signicant clusters when generating temporal graph (numeric)
                        // Advanced Analysis - power evaluations
                        POWER_EVALUATIONS,
                        POWER_EVALUATION_TYPE,
                        CRITICAL_VALUES_TYPE,
                        CRITICAL_VALUE_05,
                        CRITICAL_VALUE_01,
                        CRITICAL_VALUE_001,
                        POWER_EVALUATION_TOTALCASES,
                        POWER_EVALUATIONS_REPLICA,
                        POWER_EVALUATIONS_FILE,
                        POWER_BASELINE_PROBABILITY,
                        POWER_Z,
                        // power simulations
                        READ_SIMULATIONS,
                        INPUT_SIM_FILE,
                        WRITE_SIMULATIONS,
                        OUTPUT_SIM_FILE,
                        PROSPECTIVE_FREQ_TYPE,          // frequency of prospective analysis type
                        PROSPECTIVE_FREQ,               // frequency of prospective analysis type
                        // Run Options
                        PARALLEL_PROCESSES,
                        // System
                        CREATION_VERSION
    };
    /** Frequency of prospective analyses */
    enum ProspectiveFrequency {
        DAILY=0,
        WEEKLY,
        MONTHLY,
        QUARTERLY,
        YEARLY
    };
    /** Power evaluation method */
    enum PowerEvaluationType
    {
        PE_WITH_ANALYSIS=0,             // execute standard analysis and power evaluation together
        PE_ONLY_CASEFILE,               // execute only power evaluation, using total cases from case file
        PE_ONLY_SPECIFIED_CASES         // execute only power evaluation, using user specified total cases
    };
    /** Critical values calculation type */
    enum CriticalValuesType
    {
        CV_MONTECARLO=0,                // standard monte carlo
        CV_POWER_VALUES                 // user specified values
    };
    /** Temporal graph reporting type */
    enum TemporalGraphReportType
    {
        MLC_ONLY = 0,                  // generate a temporal graph for the most likley cluster only
        X_MCL_ONLY,                    // generate a temporal graph for the first X likley clusters
        SIGNIFICANT_ONLY               // generate a temporal graph for significant clusters only
    };
    enum ResultsFormat {TEXT=0};
    enum ParametersFormat {XML=0, JSON};
    enum ModelType {POISSON=0, BERNOULLI_TREE, UNIFORM, MODEL_NOT_APPLICABLE, BERNOULLI_TIME, SIGNED_RANK};
    enum CutType {SIMPLE=0, PAIRS, TRIPLETS, ORDINAL, COMBINATORIAL};
    enum ScanType {TREEONLY=0, TREETIME, TIMEONLY};
    enum ConditionalType {UNCONDITIONAL=0, TOTALCASES, NODE, NODEANDTIME};
    enum MaximumWindowType {PERCENTAGE_WINDOW=0, FIXED_LENGTH};    
    enum ScanRateType { HIGHRATE=0, LOWRATE, HIGHORLOWRATE };
    enum PValueReportingType { STANDARD_PVALUE=0, TERMINATION_PVALUE };
    typedef std::map<std::string,Parameters::CutType> cut_map_t;
    typedef std::pair<cut_map_t, cut_map_t> cut_maps_t;
    typedef std::vector<std::string> FileNameContainer_t;

    struct CreationVersion {
        unsigned int iMajor;
        unsigned int iMinor;
        unsigned int iRelease;

        CreationVersion() : iMajor(static_cast<unsigned int>(std::atoi(VERSION_MAJOR))),
            iMinor(static_cast<unsigned int>(std::atoi(VERSION_MINOR))),
            iRelease(static_cast<unsigned int>(std::atoi(VERSION_RELEASE))) {}
        CreationVersion(unsigned int major, unsigned int minor, unsigned int release) : iMajor(major), iMinor(minor), iRelease(release) {}

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

    class InputSource {
        private:
            SourceType _source_type;
            FieldMapContainer_t _fields_map;
            // CSV specific options
            std::string _delimiter;
            std::string _grouper;
            unsigned int _skip;
            bool _first_row_headers;

        public:
            InputSource() : _skip(0) {}

            InputSource(SourceType type, FieldMapContainer_t map):
                _source_type(type), _fields_map(map), 
                _delimiter(","), _grouper("\""), _skip(0), _first_row_headers(false) {}

            InputSource(SourceType type, FieldMapContainer_t map, std::string delimiter, std::string grouper, unsigned int skip, bool first_row_headers):
                _source_type(type), _fields_map(map), 
                _delimiter(delimiter), _grouper(grouper), _skip(skip), _first_row_headers(first_row_headers) {}

            SourceType getSourceType() const {return _source_type;}
            void setSourceType(SourceType e) {_source_type = e;}
            const FieldMapContainer_t & getFieldsMap() const {return _fields_map;}
            void setFieldsMap(const FieldMapContainer_t& m) {_fields_map = m;}
            void clearFieldsMap() {_fields_map.clear();}
            // CSV specific options
            const std::string & getDelimiter() const {return _delimiter;}
            void setDelimiter(const std::string& s) {_delimiter = s;}
            const std::string & getGroup() const {return _grouper;}
            void setGroup(const std::string& s) {_grouper = s;}
            unsigned int getSkip() const {return _skip;}
            void setSkip(unsigned int u) {_skip = u;}
            bool getFirstRowHeader() const {return _first_row_headers;}
            void setFirstRowHeader(bool b) {_first_row_headers = b;}
    };

  public:
    typedef std::pair<ParameterType, unsigned int> InputSourceKey_t; // ParameterType, index
    typedef std::map<InputSourceKey_t, InputSource> InputSourceContainer_t;
    typedef std::vector<unsigned int> RestrictTreeLevels_t;

  private:
    InputSourceContainer_t              _input_sources;
    unsigned int                        _numRequestedParallelProcesses;
    unsigned int                        _replications;
    std::string                         _parametersSourceFileName;

    FileNameContainer_t                 _treeFileNames;

    std::string                         _cutsFileName;
    std::string                         _countFileName;
    std::string                         _controlFileName;
    DataTimeRange::DatePrecisionType    _date_precision_type;
    std::string                         _data_time_range_str;
    DataTimeRangeSet                    _dataTimeRangeSet;
    bool                                _restrict_temporal_windows;
    std::string                         _temporal_start_range_str;
    DataTimeRange                       _temporalStartRange;
    std::string                         _temporal_end_range_str;
    DataTimeRange                       _temporalEndRange;
    std::string                         _outputFileName;
    ResultsFormat                       _resultsFormat;
    struct CreationVersion              _creationVersion;
    long                                _randomizationSeed;
    bool                                _randomlyGenerateSeed;
    bool                                _generateHtmlResults;
    bool                                _generateTableResults;
    bool                                _generateNCBIAsnResults;
    bool                                _generateNewickFile;
    bool                                _printColumnHeaders;
    ModelType                           _modelType;
    ratio_t                             _probablility_ratio;
    bool                                _variable_case_probablility;
    CutType                             _cut_type;
    bool                                _data_only_on_leaves;
    bool                                _relaxed_study_data_period_checking;
    bool                                _allow_multi_parent_nodes;
    bool                                _allow_multiple_roots;
    ScanType                            _scan_type;
    ConditionalType                     _conditional_type;
    ScanRateType                        _scan_rate_type;
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
    double                              _critical_value_05;  // user specified critical value at .05
    double                              _critical_value_01;  // user specified critical value at .01
    double                              _critical_value_001; // user specified critical value at .001
    int                                 _power_evaluation_totalcases;
    unsigned int                        _power_replica; // number of replicas in power step  of power evaluation
    std::string                         _power_alt_hypothesis_filename;
    ratio_t                             _power_baseline_probablility_ratio;
    bool                                _dayofweek_adjustment;
    bool                                _report_attributable_risk;
    unsigned int                        _attributable_risk_exposed;
    bool                                _include_identical_parent_cuts;
    bool                                _self_control_design;
    bool                                _restrict_tree_levels;
    RestrictTreeLevels_t                _restricted_tree_levels;
    bool                                _restrict_evaluated_tree_nodes;
    std::string                         _not_evaluated_nodes_filename;
    bool                                _sequential_scan;
    unsigned int                        _sequential_min_signal;
    unsigned int                        _sequential_max_signal;
    std::string                         _sequential_file;
    double                              _sequential_alpha_overall;
    double                              _sequential_alpha_spending;
    mutable unsigned int                _look_index;
    double                              _power_z;
    bool                                _apply_risk_window_restriction;
    double                              _risk_window_percentage;
    unsigned int                        _minimum_censor_time;
    unsigned int                        _minimum_censor_percentage;
    bool                                _apply_risk_window_restriction_censored;
    double                              _risk_window_censor_alt_denominator;

    bool                                _forced_censored_algorithm;
    bool                                _apply_exclusion_ranges;
    std::string                         _exclusion_time_range_str;
    DataTimeRangeSet                    _exclusion_time_ranges;

    ProspectiveFrequency                _prospective_frequency_type;
    unsigned int                        _prospective_frequency;
    bool                                _prospective_analysis;

    unsigned int                        _minimum_highrate_nodes_cases;
    unsigned int                        _minimum_lowrate_nodes_cases;

    /** Temporal clusters graph */
    bool                                _output_temporal_graph;                 // generate temporal graph output file
    TemporalGraphReportType             _temporal_graph_report_type;            // which clusters to report in temporal graph
    int                                 _temporal_graph_report_count;           // number of MLC clusters to graph with TemporalGraphReportType.X_MCL_ONLY
    double                              _temporal_graph_report_cutoff;          // P-Value used limit graphed clusters with TemporalGraphReportType.SIGNIFICANT_ONLY

    PValueReportingType                 _pvalue_reporting_type;
    unsigned int                        _early_term_threshold;

    void                                assignMissingPath(std::string & sInputFilename, bool bCheckWritable=false);
    void                                copy(const Parameters &rhs);
    const char                        * getRelativeToParameterName(const FileName& fParameterName, const std::string& sFilename, std::string& sValue) const;

  public:
    Parameters() {setAsDefaulted();}
    Parameters(const Parameters &other) {setAsDefaulted(); copy(other);}
    ~Parameters() {}

    static const int                    giNumParameters; // number enumerated parameters

    Parameters                        & operator=(const Parameters &rhs)  {if (this != &rhs) copy(rhs); return (*this);}
    bool                                operator==(const Parameters& rhs) const;
    bool                                operator!=(const Parameters& rhs) const {return !(*this == rhs);}

    bool                                getIsSelfControlVariableBerounlli() const;
    bool                                getIsTestStatistic() const;
    PValueReportingType                 getPValueReportingType() const { return _pvalue_reporting_type; }
    void                                setPValueReportingType(PValueReportingType e);
    unsigned int                        getEarlyTermThreshold() const { return _early_term_threshold; }
    void                                setEarlyTermThreshold(unsigned int i) { _early_term_threshold = i; }
    unsigned int                        getExecuteEarlyTermThreshold() const;
    bool                                getTerminateSimulationsEarly() const;
    bool                                getDataOnlyOnLeaves() const { return _data_only_on_leaves; }
    void                                setDataOnlyOnLeaves(bool b) { _data_only_on_leaves = b; }
    bool                                getRelaxedStudyDataPeriodChecking() const { return _relaxed_study_data_period_checking; }
    void                                setRelaxedStudyDataPeriodChecking(bool b) { _relaxed_study_data_period_checking = b; }
    bool                                getAllowMultiParentNodes() const { return _allow_multi_parent_nodes; }
    void                                setAllowMultiParentNodes(bool b) { _allow_multi_parent_nodes = b; }
    bool                                getAllowMultipleRoots() const { return _allow_multiple_roots; }
    void                                setAllowMultipleRoots(bool b) { _allow_multiple_roots = b; }
    ScanRateType                        getScanRateType() const { return _scan_rate_type; }
    void                                setScanRateType(ScanRateType e) { _scan_rate_type = e; }
    unsigned int                        getMinimumHighRateNodeCases() const { return _minimum_highrate_nodes_cases; }
    void                                setMinimumHighRateNodeCases(unsigned int i) { _minimum_highrate_nodes_cases = i; }
    unsigned int                        getMinimumLowRateNodeCases() const { return _minimum_lowrate_nodes_cases; }
    void                                setMinimumLowRateNodeCases(unsigned int i) { _minimum_lowrate_nodes_cases = i; }

    const std::string                 & getDataTimeRangeStr() const { return _data_time_range_str;}
    void                                setDataTimeRangeStr(const std::string& s) { _data_time_range_str = s; }
    const std::string                 & getTemporalStartRangeStr() const { return _temporal_start_range_str; }
    void                                setTemporalStartRangeStr(const std::string& s) { _temporal_start_range_str = s; }
    const std::string                 & getTemporalEndRangeStr() const { return _temporal_end_range_str; }
    void                                setTemporalEndRangeStr(const std::string& s) { _temporal_end_range_str = s; }

    const std::string                 & getExclusionTimeRangeStr() const { return _exclusion_time_range_str; }
    void                                setExclusionTimeRangeStr(const std::string& s) { _exclusion_time_range_str = s; }

    bool                                getRestrictTemporalWindows() const { return _restrict_temporal_windows; }
    void                                setRestrictTemporalWindows(bool b) { _restrict_temporal_windows = b; }

    bool                                getIsProspectiveAnalysis() const { return _prospective_analysis; }
    void                                setIsProspectiveAnalysis(bool b) { _prospective_analysis = b; }
    DataTimeRange::DatePrecisionType    getDatePrecisionType() const { return _date_precision_type; }
    void                                setDatePrecisionType(DataTimeRange::DatePrecisionType e) { _date_precision_type = e; }
    ProspectiveFrequency                getProspectiveFrequencyType() const { return _prospective_frequency_type; }
    void                                setProspectiveFrequencyType(ProspectiveFrequency e) { _prospective_frequency_type = e; }
    unsigned int                        getProspectiveFrequency() const { return _prospective_frequency; }
    void                                setProspectiveFrequency(unsigned int i) { _prospective_frequency = i; }

    bool                                getOutputTemporalGraphFile() const { return _output_temporal_graph; }
    void                                setOutputTemporalGraphFile(bool b) { _output_temporal_graph = b; }
    TemporalGraphReportType             getTemporalGraphReportType() const { return _temporal_graph_report_type; }
    void                                setTemporalGraphReportType(TemporalGraphReportType e);
    double                              getTemporalGraphSignificantCutoff() const { return _temporal_graph_report_cutoff; }
    void                                setTemporalGraphSignificantCutoff(double d) { _temporal_graph_report_cutoff = d; }
    int                                 getTemporalGraphMostLikelyCount() const { return _temporal_graph_report_count; }
    void                                setTemporalGraphMostLikelyCount(int i) { _temporal_graph_report_count = i; }

    void                                setCurrentLook(unsigned int u) const { _look_index = u; }
    unsigned int                        getCurrentLook() const { return _look_index; }

    bool                                isApplyingRiskWindowRestrictionCensored() const { return _apply_risk_window_restriction_censored; }
    void                                setApplyingRiskWindowRestrictionCensored(bool b) { _apply_risk_window_restriction_censored = b; }
    double                              getRiskWindowAltCensorDenominator() const { return _risk_window_censor_alt_denominator; }
    void                                setRiskWindowAltCensorDenominator(double d) { _risk_window_censor_alt_denominator = d; }
    unsigned int                        getMinimumCensorTime() const { return _minimum_censor_time; }
    void                                setMinimumCensorTime(unsigned int u) { _minimum_censor_time = u; }
    unsigned int                        getMinimumCensorPercentage() const { return _minimum_censor_percentage; }
    void                                setMinimumCensorPercentage(unsigned int u) { _minimum_censor_percentage = u; }

    bool                                isApplyingExclusionTimeRanges() const { return _apply_exclusion_ranges; }
    void                                setApplyingExclusionTimeRanges(bool b) { _apply_exclusion_ranges = b; }
    bool                                isForcedCensoredAlgorithm() const { return _forced_censored_algorithm; }
    void                                setForcedCensoredAlgorithm(bool b) { _forced_censored_algorithm = b; }
    bool                                isApplyingRiskWindowRestriction() const { return _apply_risk_window_restriction; }
    void                                setApplyingRiskWindowRestriction(bool b) { _apply_risk_window_restriction = b; }
    double                              getRiskWindowPercentage() const { return _risk_window_percentage; }
    void                                setRiskWindowPercentage(double d) { _risk_window_percentage = d; }
    void                                defineInputSource(ParameterType e, InputSource source, unsigned int idx=1) {_input_sources[std::make_pair(e,idx)] = source;}
    ConditionalType                     getConditionalType() const {return _conditional_type;}
    const std::string                 & getCountFileName() const {return _countFileName;}
    const std::string                 & getControlFileName() const { return _controlFileName; }
    const CreationVersion             & getCreationVersion() const {return _creationVersion;}
    double                              getCriticalValue05() const {return _critical_value_05;}
    double                              getCriticalValue01() const {return _critical_value_01;}
    double                              getCriticalValue001() const {return _critical_value_001;}
    CriticalValuesType                  getCriticalValuesType() const {return _critical_values_type;}
    const std::string                 & getCutsFileName() const {return _cutsFileName;}
    CutType                             getCutType() const {return _cut_type;}
    static cut_maps_t                   getCutTypeMap();
    const DataTimeRangeSet            & getDataTimeRangeSet() const {return _dataTimeRangeSet;}
    const DataTimeRangeSet            & getExclusionTimeRangeSet() const { return _exclusion_time_ranges; }
    const InputSourceContainer_t      & getInputSources() const {return _input_sources;}
    const InputSource                 * getInputSource(ParameterType e, unsigned int idx=1) const {
                                            InputSourceContainer_t::const_iterator itr = _input_sources.find(std::make_pair(e,idx));
                                            return itr == _input_sources.end() ? (const InputSource*)0 : &(itr->second);
                                        }
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
    double                              getPowerBaselineProbability() const;
    ratio_t                             getPowerBaselineProbabilityRatio() const {return _power_baseline_probablility_ratio;}
    double                              getPowerZ() const { return _power_z; }
    double                              getProbability() const;
    ratio_t                             getProbabilityRatio() const {return _probablility_ratio;}
    bool                                getVariableCaseProbability() const { return _variable_case_probablility; }
    long                                getRandomizationSeed() const {return _randomizationSeed;}
    bool                                getReportAttributableRisk() const {return _report_attributable_risk;}
    const std::string                 & getNotEvaluatedNodesFileName() const { return _not_evaluated_nodes_filename; }
    bool                                getIncludeIdenticalParentCuts() const {return _include_identical_parent_cuts;}
    bool                                getRestrictTreeLevels() const {return _restrict_tree_levels;}
    const RestrictTreeLevels_t        & getRestrictedTreeLevels() const {return _restricted_tree_levels;}
    bool                                getRestrictEvaluatedTreeNodes() const { return _restrict_evaluated_tree_nodes; }
    unsigned int                        getAttributableRiskExposed() const {return _attributable_risk_exposed;}
    bool                                getReportCriticalValues() const {return _report_critical_values;}
    ResultsFormat                       getResultsFormat() const {return _resultsFormat;}
    ScanType                            getScanType() const {return _scan_type;}
    bool                                getSelfControlDesign() const {return _self_control_design;}
    bool                                getSequentialScan() const {return _sequential_scan;}
    unsigned int                        getSequentialMinimumSignal() const {return _sequential_min_signal;}
    unsigned int                        getSequentialMaximumSignal() const {return _sequential_max_signal;}
    const std::string                 & getSequentialFilename() const {return _sequential_file;}
    double                              getSequentialAlphaOverall() const { return _sequential_alpha_overall; }
    double                              getSequentialAlphaSpending() const { return _sequential_alpha_spending; }
    const std::string                 & getSourceFileName() const {return _parametersSourceFileName;}
    const DataTimeRange               & getTemporalEndRange() const {return _temporalEndRange;}
    const DataTimeRange               & getTemporalStartRange() const {return _temporalStartRange;}
    const FileNameContainer_t         & getTreeFileNames() const {return _treeFileNames;}
    bool                                isGeneratingHtmlResults() const {return _generateHtmlResults;}
    bool                                isGeneratingLLRResults() const {return _generate_llr_results;}
    bool                                isGeneratingTableResults() const {return _generateTableResults;}
    bool                                isGeneratingNCBIAsnResults() const { return _generateNCBIAsnResults; }
    bool                                isGeneratingNewickFile() const { return _generateNewickFile; }
    bool                                isPerformingDayOfWeekAdjustment() const {return isTemporalScanType(_scan_type) && _dayofweek_adjustment;}
    bool                                isPrintColumnHeaders() const {return _printColumnHeaders;}
    bool                                isRandomlyGeneratingSeed() const {return _randomlyGenerateSeed;}
    bool                                isReadingSimulationData() const {return _read_simulations;}
    bool                                isSequentialScanPurelyTemporal() const;
    bool                                isSequentialScanBernoulli() const;
	bool                                isSequentialScanPoisson() const;
	bool                                isSequentialScanTreeOnly() const;
	static bool                         isSpatialScanType(ScanType e) {return e == Parameters::TREEONLY || e == Parameters::TREETIME;}
    static bool                         isTemporalScanType(ScanType e) {return e == Parameters::TIMEONLY || e == Parameters::TREETIME;}
    bool                                isWritingSimulationData() const {return _write_simulations;}

    void                                setAsDefaulted();
    void                                setConditionalType(ConditionalType e) {_conditional_type = e;}
    void                                setCountFileName(const char * sCountFileName, bool bCorrectForRelativePath=false);
    void                                setControlFileName(const char * sControlFileName, bool bCorrectForRelativePath = false);
    void                                setNotEvaluatedNodesFileName(const char* filename, bool bCorrectForRelativePath = false);
    void                                setCriticalValue05(double d) {_critical_value_05 = d;}
    void                                setCriticalValue01(double d) {_critical_value_01 = d;}
    void                                setCriticalValue001(double d) {_critical_value_001 = d;}
    void                                setCriticalValuesType(CriticalValuesType e) {_critical_values_type = e;}
    void                                setCutsFileName(const char * sCutsFileName, bool bCorrectForRelativePath=false);
    void                                setCutType(CutType e) {_cut_type = e;}
    void                                setDataTimeRangeSet(const DataTimeRangeSet& set) {_dataTimeRangeSet = set;}
    void                                setExclusionTimeRangeSet(const DataTimeRangeSet& set) { _exclusion_time_ranges = set; }
    void                                setGeneratingHtmlResults(bool b) {_generateHtmlResults = b;}
    void                                setGeneratingLLRResults(bool b) {_generate_llr_results = b;}
    void                                setGeneratingTableResults(bool b) {_generateTableResults = b;}
    void                                setGeneratingNCBIAsnResults(bool b) { _generateNCBIAsnResults = b; }
    void                                setGeneratingNewickFile(bool b) { _generateNewickFile = b; }
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
    void                                setPowerBaselineProbabilityRatio(ratio_t r) {_power_baseline_probablility_ratio = r;}
    void                                setPowerZ(double d) { _power_z = d; }
    void                                setPrintColumnHeaders(bool b) {_printColumnHeaders=b;}
    void                                setProbabilityRatio(ratio_t r) {_probablility_ratio = r;}
    void                                setVariableCaseProbability(bool b) { _variable_case_probablility = b; }
    void                                setRandomizationSeed(long lSeed) {_randomizationSeed = lSeed;}
    void                                setRandomlyGeneratingSeed(bool b) {_randomlyGenerateSeed = b;}
    void                                setReadingSimulationData(bool b) {_read_simulations = b;}
    void                                setReportAttributableRisk(bool b) {_report_attributable_risk = b;}
    void                                setIncludeIdenticalParentCuts(bool b) {_include_identical_parent_cuts = b;}
	void                                setRestrictTreeLevels(bool b) {_restrict_tree_levels = b;}
	void                                setRestrictedTreeLevels(const RestrictTreeLevels_t& r) {_restricted_tree_levels = r;}
    void                                setRestrictEvaluatedTreeNodes(bool b) { _restrict_evaluated_tree_nodes = b; }
    void                                setAttributableRiskExposed(unsigned int i) {_attributable_risk_exposed = i;}
    void                                setReportCriticalValues(bool b) {_report_critical_values = b;}
    void                                setResultsFormat(ResultsFormat e) {_resultsFormat = e;}
    void                                setScanType(ScanType e) {_scan_type = e;}
    void                                setSelfControlDesign(bool b) {_self_control_design = b;}

    void                                setSequentialScan(bool b) {_sequential_scan = b;}
    void                                setSequentialMinimumSignal(unsigned int i) {_sequential_min_signal = i;}
    void                                setSequentialMaximumSignal(unsigned int i) {_sequential_max_signal = i;}
    void                                setSequentialFilename(const char * s, bool bCorrectForRelativePath=false);
    void                                setSequentialAlphaOverall(double d) { _sequential_alpha_overall = d; }
    void                                setSequentialAlphaSpending(double d) { _sequential_alpha_spending = d; }

    void                                setSourceFileName(const char * sParametersSourceFileName);
    void                                setTemporalEndRange(const DataTimeRange& range) {_temporalEndRange = range;}
    void                                setTemporalStartRange(const DataTimeRange& range) {_temporalStartRange = range;}

    void                                setTreeFileName(const char * sTreeFileName, bool bCorrectForRelativePath=false, size_t treeIdx=1);

    void                                setWritingSimulationData(bool b) {_write_simulations = b;}
    void                                setVersion(const CreationVersion& vVersion) {_creationVersion = vVersion;}

    void read(const std::string &filename, ParametersFormat type);
    void write(const std::string &filename, ParametersFormat type) const;
};
//*****************************************************************************
#endif
