//******************************************************************************
#ifndef __PermutationDataRandomizer_H
#define __PermutationDataRandomizer_H
//******************************************************************************
#include "Randomization.h"

/** class representing the stationary attribute in a permuted randomization. */
template <class T>
class StationaryAttribute {
    protected:
        T   gStationaryVariable;

    public:
        StationaryAttribute(T Variable) : gStationaryVariable(Variable) {}
        virtual ~StationaryAttribute() {}
        virtual StationaryAttribute * Clone() const {return new StationaryAttribute(*this);}
        bool operator==(const StationaryAttribute& rhs) const{return (this->gStationaryVariable == rhs.gStationaryVariable);}
        bool operator!=(const StationaryAttribute& rhs) const{return !(this->gStationaryVariable == rhs.gStationaryVariable);}
        inline const T & GetStationaryVariable() const {return gStationaryVariable;}
};

/** class representing the permuted attribute in a permuted randomization. */
template <class T>
class PermutedAttribute {
    protected:
        T   gPermutedVariable;
        float   gfRandomNumber;

    public:
        PermutedAttribute(T Variable) : gPermutedVariable(Variable), gfRandomNumber(0) {}
        virtual ~PermutedAttribute() {}
        virtual PermutedAttribute  * Clone() const {return new PermutedAttribute(*this);}

        inline const T & GetPermutedVariable() const {return gPermutedVariable;}
        inline float GetRandomNumber() const {return gfRandomNumber;}
        inline T & ReferencePermutedVariable() {return gPermutedVariable;}
        inline void SetRandomNumber(float f) {gfRandomNumber = f;}
};

/** Function object used to compare permuted attributes. */
template <class T>
class ComparePermutedAttribute {
    public:
        inline bool operator() (const T& plhs, const T& prhs) {
            return (plhs.GetRandomNumber() < prhs.GetRandomNumber());
        }
};

/** Function object used to assign random number to permuted attribute. */
template <class T>
class AssignPermutedAttribute {
    protected:
        RandomNumberGenerator & gGenerator;

    public:
        AssignPermutedAttribute(RandomNumberGenerator & Generator) : gGenerator(Generator) {}
        ~AssignPermutedAttribute() {}

        inline void operator() (T& pAttribute);
};

template <class T>
inline void AssignPermutedAttribute<T>::operator() (T& Attribute) {
    Attribute.SetRandomNumber(gGenerator.GetRandomFloat());
}

// ******************************************************************************************************

/** abstract permutation randomizer class */
template <class S, class P>
class AbstractPermutedDataRandomizer /*: public AbstractRandomizer*/ {
    protected:
        bool _dayOfWeekAdjustment;

    public:
        typedef std::vector<S>                      StationaryContainer_t;
        typedef std::vector<StationaryContainer_t>  StationaryContainerCollection_t;
        typedef std::vector<P>                      PermutedContainer_t;
        typedef std::vector<PermutedContainer_t>    PermutedContainerCollection_t;

    protected:
        StationaryContainerCollection_t gvStationaryAttributeCollections;
        PermutedContainerCollection_t gvOriginalPermutedAttributeCollections;
        PermutedContainerCollection_t gvPermutedAttributeCollections;

        virtual void AssignRandomizedData(const AbstractNodesProxy& treeNodes, SimNodeContainer_t& treeSimNodes) = 0;
        virtual void SortPermutedAttribute(RandomNumberGenerator& rng);
        virtual void randomize_data(unsigned int iSimulation, const AbstractNodesProxy& treeNodes, SimNodeContainer_t& treeSimNodes, RandomNumberGenerator& rng);

    public:
        AbstractPermutedDataRandomizer(const Parameters& parameters, long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
        virtual ~AbstractPermutedDataRandomizer() {}
};

/** constructor */
template <class S, class P>
AbstractPermutedDataRandomizer<S, P>::AbstractPermutedDataRandomizer(const Parameters& parameters, long lInitialSeed) : _dayOfWeekAdjustment(parameters.isPerformingDayOfWeekAdjustment()) {
    if (_dayOfWeekAdjustment) {
        // with day of week adjustment, we will keep each week day separate
        gvStationaryAttributeCollections.resize(7);
        gvOriginalPermutedAttributeCollections.resize(7);
        gvPermutedAttributeCollections.resize(7);
    } else {
        gvStationaryAttributeCollections.resize(1);
        gvOriginalPermutedAttributeCollections.resize(1);
        gvPermutedAttributeCollections.resize(1);
    }
}

/** randomizes data of dataset */
template <class S, class P>
void AbstractPermutedDataRandomizer<S, P>::randomize_data(unsigned int iSimulation, const AbstractNodesProxy& treeNodes, SimNodeContainer_t& treeSimNodes, RandomNumberGenerator& rng) {
    //assign random numbers to permuted attribute and sort
    SortPermutedAttribute(rng);
    //re-assign dataset's simulation data
    AssignRandomizedData(treeNodes, treeSimNodes);
}

/** re-initializes and  sorts permutated attribute */
template <class S, class P>
void AbstractPermutedDataRandomizer<S, P>::SortPermutedAttribute(RandomNumberGenerator& rng) {
    // Reset permuted attributes to original order - this is needed to maintain
    // consistancy of output when running in parallel.
    gvPermutedAttributeCollections = gvOriginalPermutedAttributeCollections;

    for (typename PermutedContainerCollection_t::iterator itr=gvPermutedAttributeCollections.begin(); itr != gvPermutedAttributeCollections.end(); ++itr) {
        std::for_each(itr->begin(), itr->end(), AssignPermutedAttribute<P>(rng));
        std::sort(itr->begin(), itr->end(), ComparePermutedAttribute<P>());
    }
}
//******************************************************************************
#endif
