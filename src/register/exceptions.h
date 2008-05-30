#ifndef EXCEPTIONS_H_
#define EXCEPTIONS_H_

#include <string>
#include <exception>

namespace Register {

/*
 * Base exception;
 * all register exception should be inherited from this
 */
class Exception : public std::exception {
public:
  Exception(const Exception& _ex) throw() :
    std::exception(_ex), what_(_ex.what_) {
  }
  Exception& operator=(const Exception& _ex) throw() {
    what_ = _ex.what_;
    return *this;
  }
  ~Exception() throw() {
  }
  virtual const char* what() const throw() {
    return what_.c_str();
  }
  Exception(const std::string& _what) throw() :
    what_(_what) {
  }
protected:
  std::string what_;
};

/// Exception when SQL error
class SQL_ERROR {
};


/// Exception when specified object is not found
class NOT_FOUND {
};


/// Exception when specified object already exists
class ALREADY_EXISTS {
};

}


#endif
