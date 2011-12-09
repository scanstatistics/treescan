//*****************************************************************************
#ifndef __PARAMETERS_H_
#define __PARAMETERS_H_
//*****************************************************************************
#include "UtilityFunctions.h"
#include "FileName.h"

class Parameters {
  public:
    typedef std::pair<unsigned int,unsigned int> ratio_t;
    enum ParameterType {TREEFILE=0, COUNTFILE, OUTPUTFILE, CUTS, REPLICATIONS, CONDITIONAL, DUPLICATES, RANDOMIZATION_SEED, RANDOMLY_GENERATE_SEED, PRINT_COLUMN_HEADERS, MODEL};
    struct CreationVersion {unsigned int iMajor; unsigned int iMinor; unsigned int iRelease;};
    enum ResultsFormat {TEXT=0, HTML};
    enum ParametersFormat {XML=0, INI, JSON};
    enum ModelType {POISSON=0, BERNOULLI};
    enum CutType {SIMPLE=0, PAIRS, TRIPLETS, ORDINAL, COMBINATORIAL, MIXED};

  private:
    unsigned int                        _numRequestedParallelProcesses;
    unsigned int                        _replications;
    std::string                         _parametersSourceFileName;
    std::string                         _treeFileName;
    std::string                         _countFileName;
    std::string                         _outputFileName;
    ResultsFormat                       _resultsFormat;
    struct CreationVersion              _creationVersion;
    long                                _randomizationSeed;
    bool                                _randomlyGenerateSeed;
    bool                                _conditional;
    bool                                _duplicates;
    bool                                _printColumnHeaders;
    ModelType                           _modelType;    
    ratio_t                             _probablility_ratio;
    CutType                             _cut_type;

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

    CutType                             getCutType() const {return _cut_type;}
    ModelType                           getModelType() const {return _modelType;}
    const std::string                 & getCountFileName() const {return _countFileName;}
    const CreationVersion             & getCreationVersion() const {return _creationVersion;}
    ratio_t                             getProbabilityRatio() const {return _probablility_ratio;}
    double                              getProbability() const {return static_cast<double>(_probablility_ratio.first)/static_cast<double>(_probablility_ratio.second);}
    const std::string                 & getTreeFileName() const {return _treeFileName;}
    bool                                isConditional() const {return _conditional;}
    bool                                isDuplicates() const {return _duplicates;}
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
    void                                setCutType(CutType e) {_cut_type = e;}
    void                                setModelType(ModelType e) {_modelType = e;}
    void                                setAsDefaulted();
    void                                setProbabilityRatio(ratio_t r) {_probablility_ratio = r;}
    void                                setCountFileName(const char * sCountFileName, bool bCorrectForRelativePath=false);
    void                                setTreeFileName(const char * sTreeFileName, bool bCorrectForRelativePath=false);
    void                                setConditional(bool b) {_conditional = b;}
    void                                setDuplicates(bool b) {_duplicates = b;}
    void                                setRandomlyGeneratingSeed(bool b) {_randomlyGenerateSeed = b;}
    void                                setPrintColunHeaders(bool b) {_printColumnHeaders=b;}
    void                                setNumProcesses(unsigned int i) {_numRequestedParallelProcesses = i;}
    void                                setNumReplications(unsigned int replications) {_replications = replications;}
    void                                setOutputFileName(const char * sOutPutFileName, bool bCorrectForRelativePath=false);
    void                                setRandomizationSeed(long lSeed) {_randomizationSeed = lSeed;}
    void                                setResultsFormat(ResultsFormat e) {_resultsFormat = e;}
    void                                setSourceFileName(const char * sParametersSourceFileName);
    void                                setVersion(const CreationVersion& vVersion) {_creationVersion = vVersion;}

    void read(const std::string &filename, ParametersFormat type);
    void write(const std::string &filename, ParametersFormat type) const;
};
//*****************************************************************************
#endif
