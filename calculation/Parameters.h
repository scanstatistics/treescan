//*****************************************************************************
#ifndef __PARAMETERS_H_
#define __PARAMETERS_H_
//*****************************************************************************
#include "UtilityFunctions.h"
#include "FileName.h"

class Parameters {
  public:
    enum ParameterType {TREEFILE=0, COUNTFILE, OUTPUTFILE, CUTS, REPLICATIONS, CONDITIONAL, DUPLICATES, RANDOMIZATION_SEED, RANDOMLY_GENERATE_SEED, PRINT_COLUMN_HEADERS};
    struct CreationVersion {unsigned int iMajor; unsigned int iMinor; unsigned int iRelease;};

  private:
    unsigned int                        _numRequestedParallelProcesses;
    unsigned int                        _replications;
    unsigned int                        _cuts;
    std::string                         _parametersSourceFileName;
    std::string                         _treeFileName;
    std::string                         _countFileName;
    std::string                         _outputFileName;
    struct CreationVersion              _creationVersion;
    long                                _randomizationSeed;
    bool                                _randomlyGenerateSeed;
    bool                                _conditional;
    bool                                _duplicates;
    bool                                _printColumnHeaders;

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

    const std::string                 & getCountFileName() const {return _countFileName;}
    const CreationVersion             & getCreationVersion() const {return _creationVersion;}
    unsigned int                        getCuts() const {return _cuts;}
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
    const std::string                 & getSourceFileName() const {return _parametersSourceFileName;}
    void                                setAsDefaulted();
    void                                setCountFileName(const char * sCountFileName, bool bCorrectForRelativePath=false);
    void                                setCuts(unsigned int cuts) {_cuts = cuts;}
    void                                setTreeFileName(const char * sTreeFileName, bool bCorrectForRelativePath=false);
    void                                setConditional(bool b) {_conditional = b;}
    void                                setDuplicates(bool b) {_duplicates = b;}
    void                                setRandomlyGeneratingSeed(bool b) {_randomlyGenerateSeed = b;}
    void                                setPrintColunHeaders(bool b) {_printColumnHeaders=b;}
    void                                setNumProcesses(unsigned int i) {_numRequestedParallelProcesses = i;}
    void                                setNumReplications(unsigned int replications) {_replications = replications;}
    void                                setOutputFileName(const char * sOutPutFileName, bool bCorrectForRelativePath=false);
    void                                setRandomizationSeed(long lSeed) {_randomizationSeed = lSeed;}
    void                                setSourceFileName(const char * sParametersSourceFileName);
    void                                setVersion(const CreationVersion& vVersion) {_creationVersion = vVersion;}
};
//*****************************************************************************
#endif
