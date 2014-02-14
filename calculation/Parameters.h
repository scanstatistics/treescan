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
                        POPULATION_FILE,
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
                        /* Advanced Analysis */
                        REPLICATIONS,
                        RANDOMIZATION_SEED,
                        RANDOMLY_GENERATE_SEED,
                        /* Output */
                        RESULTS_FILE,
                        RESULTS_HTML,
                        RESULTS_CSV,
                        RESULTS_LLR,
                        /* Run Options */
                        PARALLEL_PROCESSES,
                        /* System */
                        CREATION_VERSION
    };
    struct CreationVersion {unsigned int iMajor; unsigned int iMinor; unsigned int iRelease;};
    enum ResultsFormat {TEXT=0};
    enum ParametersFormat {XML=0, JSON};
    enum ModelType {POISSON=0, BERNOULLI, TEMPORALSCAN};
    enum CutType {SIMPLE=0, PAIRS, TRIPLETS, ORDINAL, COMBINATORIAL};
    enum ScanType {TREEONLY=0, TREETIME};
    enum ConditionalType {UNCONDITIONAL=0, TOTALCASES, CASESEACHBRANCH};
    typedef std::map<std::string,Parameters::CutType> cut_map_t;
    typedef std::pair<cut_map_t, cut_map_t> cut_maps_t;

  private:
    unsigned int                        _numRequestedParallelProcesses;
    unsigned int                        _replications;
    std::string                         _parametersSourceFileName;
    std::string                         _treeFileName;
    std::string                         _cutsFileName;
    std::string                         _countFileName;
    std::string                         _populationFileName;
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
    bool                                _generate_llr_results;

    void                                assignMissingPath(std::string & sInputFilename, bool bCheckWritable=false);
    void                                copy(const Parameters &rhs);
    const char                        * getRelativeToParameterName(const FileName& fParameterName, const std::string& sFilename, std::string& sValue) const;

  public:
    Parameters() {setAsDefaulted();}
    Parameters(const Parameters &other) {setAsDefaulted(); copy(other);}
    ~Parameters() {}

    static const int                    giNumParameters;                        /** number enumerated parameters */

    Parameters                        & operator=(const Parameters &rhs)  {if (this != &rhs) copy(rhs); return (*this);}
    bool                                operator==(const Parameters& rhs) const;
    bool                                operator!=(const Parameters& rhs) const {return !(*this == rhs);}

    static cut_maps_t                   getCutTypeMap();

    ConditionalType                     getConditionalType() const {return _conditional_type;}
    CutType                             getCutType() const {return _cut_type;}
    ModelType                           getModelType() const {return _modelType;}
    const std::string                 & getCountFileName() const {return _countFileName;}
    const std::string                 & getPopulationFileName() const {return _populationFileName;}
    const CreationVersion             & getCreationVersion() const {return _creationVersion;}
    const std::string                 & getCutsFileName() const {return _cutsFileName;}
    const DataTimeRangeSet            & getDataTimeRangeSet() const {return _dataTimeRangeSet;}
    const DataTimeRange               & getTemporalStartRange() const {return _temporalStartRange;}
    const DataTimeRange               & getTemporalEndRange() const {return _temporalEndRange;}
    ratio_t                             getProbabilityRatio() const {return _probablility_ratio;}
    double                              getProbability() const {return static_cast<double>(_probablility_ratio.first)/static_cast<double>(_probablility_ratio.second);}
    ScanType                            getScanType() const {return _scan_type;}
    const std::string                 & getTreeFileName() const {return _treeFileName;}
    bool                                isDuplicates() const {return _duplicates;}
	bool                                isGeneratingHtmlResults() const {return _generateHtmlResults;}
	bool                                isGeneratingLLRResults() const {return _generate_llr_results;}
	bool                                isGeneratingTableResults() const {return _generateTableResults;}
    bool                                isPrintColumnHeaders() const {return _printColumnHeaders;}
    bool                                isRandomlyGeneratingSeed() const {return _randomlyGenerateSeed;}
    unsigned int                        getNumRequestedParallelProcesses() const {return _numRequestedParallelProcesses;}
    unsigned int                        getNumParallelProcessesToExecute() const;
    int                                 getNumReadParameters() const {return giNumParameters;}
    unsigned int                        getNumReplicationsRequested() const {return _replications;}
    const std::string                 & getOutputFileName() const {return _outputFileName; }
    long                                getRandomizationSeed() const {return _randomizationSeed;}
    ResultsFormat                       getResultsFormat() const {return _resultsFormat;}
    const std::string                 & getSourceFileName() const {return _parametersSourceFileName;}
    void                                setDataTimeRangeSet(const DataTimeRangeSet& set) {_dataTimeRangeSet = set;}
    void                                setTemporalStartRange(const DataTimeRange& range) {_temporalStartRange = range;}
    void                                setTemporalEndRange(const DataTimeRange& range) {_temporalEndRange = range;}
    void                                setConditionalType(ConditionalType e) {_conditional_type = e;}
    void                                setCutType(CutType e) {_cut_type = e;}
    void                                setModelType(ModelType e) {_modelType = e;}
    void                                setAsDefaulted();
    void                                setProbabilityRatio(ratio_t r) {_probablility_ratio = r;}
    void                                setCountFileName(const char * sCountFileName, bool bCorrectForRelativePath=false);
    void                                setPopulationFileName(const char * sPopulationFileName, bool bCorrectForRelativePath=false);
    void                                setCutsFileName(const char * sCutsFileName, bool bCorrectForRelativePath=false);
    void                                setTreeFileName(const char * sTreeFileName, bool bCorrectForRelativePath=false);
    void                                setDuplicates(bool b) {_duplicates = b;}
	void                                setGeneratingHtmlResults(bool b) {_generateHtmlResults = b;}
    void                                setGeneratingLLRResults(bool b) {_generate_llr_results = b;}
	void                                setGeneratingTableResults(bool b) {_generateTableResults = b;}
    void                                setRandomlyGeneratingSeed(bool b) {_randomlyGenerateSeed = b;}
    void                                setPrintColunHeaders(bool b) {_printColumnHeaders=b;}
    void                                setNumProcesses(unsigned int i) {_numRequestedParallelProcesses = i;}
    void                                setNumReplications(unsigned int replications) {_replications = replications;}
    void                                setOutputFileName(const char * sOutPutFileName, bool bCorrectForRelativePath=false);
    void                                setRandomizationSeed(long lSeed) {_randomizationSeed = lSeed;}
    void                                setResultsFormat(ResultsFormat e) {_resultsFormat = e;}
    void                                setScanType(ScanType e) {_scan_type = e;}
    void                                setSourceFileName(const char * sParametersSourceFileName);
    void                                setVersion(const CreationVersion& vVersion) {_creationVersion = vVersion;}

    void read(const std::string &filename, ParametersFormat type);
    void write(const std::string &filename, ParametersFormat type) const;
};
//*****************************************************************************
#endif
