//******************************************************************************
#ifndef ptr_vector_H
#define ptr_vector_H
//******************************************************************************
#include <vector>

/**  Vector class designed to hold objects allocated with "new". This class
     automatically "delete"s its members when it goes out of scope.

     This class has limitations and usage kaveats. For instance, calling the
     following methods will case a memory leak:
     std::vector::clear() or std::vector::erase(...)

     Elements should be added to vector using std::vector::push_back(new X())

     Ultimately we would like to replace this class with boost::ptr_vector,
     alas our current target compilers will not compile.                      */
template <typename TYPE, typename VECTOR = std::vector<TYPE *> >
class ptr_vector : public VECTOR {
  private:
   void    copy(const ptr_vector<TYPE,VECTOR> &rhs);

  public:
   typedef TYPE *                                       value_type;
   typedef typename VECTOR::allocator_type              allocator_type;

   typedef typename VECTOR::size_type                   size_type;
   typedef typename VECTOR::difference_type             difference_type;
   typedef typename VECTOR::reference                   reference;
   typedef typename VECTOR::const_reference             const_reference;
   typedef typename VECTOR::pointer                     pointer;
   typedef typename VECTOR::const_pointer               const_pointer;

   typedef typename VECTOR::iterator                    iterator;
   typedef typename VECTOR::const_iterator              const_iterator;
   typedef typename VECTOR::reverse_iterator            reverse_iterator;
   typedef typename VECTOR::const_reverse_iterator      const_reverse_iterator;

   inline ptr_vector();
   inline explicit ptr_vector(size_type n);
   ptr_vector(size_type n, size_type iStartPos, const_reference theCopy=0);
   ptr_vector(const ptr_vector<TYPE,VECTOR> &rhs);
   virtual ~ptr_vector();

   inline ptr_vector<TYPE,VECTOR> &operator=(const ptr_vector<TYPE,VECTOR> &rhs);

   inline void killAll();
   inline void kill(iterator delMe);
   void        kill(iterator first, iterator last);
};

// Constructor. Simply passes through to the base VECTOR constructor
template <typename TYPE, typename VECTOR>
inline ptr_vector<TYPE,VECTOR>::ptr_vector() : VECTOR() {}

// Constructor. Simply passes through to the std::vector constructor
template <typename TYPE, typename VECTOR>
inline ptr_vector<TYPE,VECTOR>::ptr_vector(size_type n) : VECTOR(n) {}

// Constructor. Simply passes through to the std::vector constructor
template <typename TYPE, typename VECTOR>
ptr_vector<TYPE,VECTOR>::ptr_vector(size_type n, size_type iStartPos, const_reference theCopy) : VECTOR(n, iStartPos, 0) {
  if (theCopy)
    for (size_type i = 0; i < n; i++)
       (*this)[i] = theCopy->Clone();
}

// Copy constructor. Since ptr_vector will have already copied the pointers, this
// constructor does an "inplace copy".
template <typename TYPE, typename VECTOR>
ptr_vector<TYPE,VECTOR>::ptr_vector(const ptr_vector<TYPE,VECTOR> &rhs) : VECTOR(rhs) {
  iterator     pCurrent;
  iterator     pEnd;

  pCurrent = this->begin();
  pEnd = this->end();
  while (pCurrent != pEnd) {
      *pCurrent = (*pCurrent)->Clone();
      pCurrent++;
  }
}

// Destructor. "delete"s all of the data in the vector.
template <typename TYPE, typename VECTOR>
ptr_vector<TYPE,VECTOR>::~ptr_vector() {
  try {
    kill(this->begin(), this->end());
  }
  catch ( ... ) {}
}

// Assignment operator.
template <typename TYPE, typename VECTOR>
inline ptr_vector<TYPE,VECTOR> &ptr_vector<TYPE,VECTOR>::operator=(const ptr_vector<TYPE,VECTOR> &rhs) {
  if (this != &rhs) {
    kill(this->begin(), this->end());
    copy(rhs);
  }
  return *this;
}

// Copy function. We need this since we have to instantiate new copies of the
// elements when we copy.
template <typename TYPE, typename VECTOR>
void ptr_vector<TYPE,VECTOR>::copy(const ptr_vector<TYPE,VECTOR> &rhs) {
  const_iterator   pElements;    // First element
  const_iterator   pEnd;         // Immediately AFTER last element

  pElements = rhs.begin();
  pEnd = rhs.end();
  while (pElements != pEnd) {
       this->push_back((*pElements)->Clone());
       pElements++;
  }
}

// "delete"s all elements in the array.
template <typename TYPE, typename VECTOR>
inline void ptr_vector<TYPE,VECTOR>::killAll() {
  kill(this->begin(), this->end());
}

// This version of kill() deletes the element pointed to by the iterator. This
// function is an extension to the STL. Thus, it may throw a std::exception.
template <typename TYPE, typename VECTOR>
inline void ptr_vector<TYPE,VECTOR>::kill(iterator delMe) {
  delete *delMe;
  this->erase(delMe);
}

// kill() goes through and delete's all objects in the given range and then
// erase()s them.
template <typename TYPE, typename VECTOR>
void ptr_vector<TYPE,VECTOR>::kill(iterator first, iterator last) {
  iterator   pCopy(first);

  while (pCopy != last) {
      delete *pCopy;
      pCopy++;
  }
  this->erase(first, last);
}
//******************************************************************************
#endif

