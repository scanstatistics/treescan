//*****************************************************************************
#ifndef  __EXCEPTIONCLASS_H
#define  __EXCEPTIONCLASS_H
//*****************************************************************************
#include <exception>
#include <string>

/** Base exception class for SaTScan application. */
class prg_exception : public std::exception {
   protected:
     std::string                _what;
     std::string                _trace;

   public:
     prg_exception();
     prg_exception(const std::string& what_arg);
     prg_exception(const char * format, const char * method, ...);
     virtual ~prg_exception() throw();

     void                       addWhat(const char * message);
     void                       addTrace(const char * method);
     void                       addTrace(const char * method, const char * clazz);
     void                       addTrace(const char * file, int line);
     virtual const char       * trace() const throw();
     virtual const char       * what() const throw();
};

/** Exception type to throw when a program error is detected. */
class prg_error : public prg_exception {
   public:
     prg_error(const char * format, const char * method, ...);
     virtual ~prg_error() throw();
};

/** Exception type to throw when a program error is detected. */
class memory_exception : public std::bad_alloc {
  protected:
     std::string                _what;

  public:
     memory_exception(const char * message);
     virtual ~memory_exception() throw();
};

/** Exception type to throw when a user resolvable problem is detected (e.g. input). */
class resolvable_error : public prg_exception {
  protected:
    resolvable_error();

  public:
    resolvable_error(const char * format, ...);
    virtual ~resolvable_error() throw();
};

/** Exception type to throw when command-line arguments are invalid. */
class usage_error : public prg_exception {
  public:
    usage_error(const char * sExecutableFullpathName);
    virtual ~usage_error() throw();
};


/** Exception type to throw when a user resolvable problem is detected (e.g. input). */
class region_exception : public resolvable_error {
  protected:
    region_exception();

  public:
    region_exception(const char * format, ...);
    virtual ~region_exception() throw();
};

template <typename TYPE>
class CarrierException : public prg_exception {
private:
   TYPE      gCarried;

public:
   inline CarrierException(const TYPE &copyMe, const char * message=0, const char * method=0);
   inline CarrierException(TYPE &copyMe, const char * message=0, const char * method=0);
   virtual ~CarrierException() throw();

   TYPE *operator-> ()                               { return &gCarried; }
   TYPE &operator* ()                                { return gCarried; }

   const TYPE *operator-> () const                   { return &gCarried; }
   const TYPE &operator* () const                    { return gCarried; }
};

// Constructor. Makes a copy of copyMe to be carried with the exception.
template <typename TYPE>
inline CarrierException<TYPE>::CarrierException(const TYPE &copyMe, const char * message, const char * method)
                              :prg_exception(message, method), gCarried(copyMe) {};

// Constructor. Makes a copy of copyMe to be carried with the exception.
template <typename TYPE>
inline CarrierException<TYPE>::CarrierException(TYPE &copyMe, const char * message, const char * method)
                              :prg_exception(message, method), gCarried(copyMe) {};

template <typename TYPE>
CarrierException<TYPE>::~CarrierException() throw() {}

/** memory cache should system run out of memory, released when prg_new_handler() is invoked. */
extern char * out_of_memory_cache;
extern void reserve_memory_cache();
extern void release_memory_cache();
/** std::set_handler_new() replacement */
extern void prg_new_handler();
//*****************************************************************************
#endif

