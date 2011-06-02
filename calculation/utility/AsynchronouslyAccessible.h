//---------------------------------------------------------------------------

#ifndef AsynchronouslyAccessibleH
#define AsynchronouslyAccessibleH
//---------------------------------------------------------------------------
#include "boost/thread/recursive_mutex.hpp"

template <typename T>
class AsynchronouslyAccessible
{
public:
  typedef T value_type;
  typedef boost::recursive_mutex mutex_type;
  class LockWrapper
  {
  public:
    friend class AsynchronouslyAccessible<T>;
    typedef mutex_type::scoped_lock scoped_lock_type;
  private:
    T & grValue;
    mutex_type & grMutex;
    scoped_lock_type gLock;
    LockWrapper(T & rValue, mutex_type & rMutex) : grValue(rValue), grMutex(rMutex), gLock(rMutex) {  }
    LockWrapper(T & rValue, mutex_type & rMutex, bool bLock) : grValue(rValue), grMutex(rMutex), gLock(rMutex, boost::defer_lock) {  }
  public:
    LockWrapper(LockWrapper const & rhs) : grValue(rhs.grValue), grMutex(rhs.grMutex), gLock(rhs.grMutex) {  }

    T & Value() { return grValue; }
    T const & Value() const { return grValue; }
  };
private:
  T & grValue;
  mutable mutex_type gAccessMutex;
public:
  AsynchronouslyAccessible(T & rValue) : grValue(rValue) {  }

  LockWrapper const Locked() const { return LockWrapper(grValue, gAccessMutex); }
  LockWrapper Locked() { return LockWrapper(grValue, gAccessMutex); }
  LockWrapper const Unlocked() const { return LockWrapper(grValue, gAccessMutex, false); }
};



#endif

