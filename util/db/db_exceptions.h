#ifndef DB_EXCEPTIONS_H_
#define DB_EXCEPTIONS_H_

#include <sstream>

#include "base_exception.h"

namespace Database {

class Exception : public ::Exception {
public:
  Exception(const std::string& _what) : ::Exception(_what) {
  }
};

class ConnectionFailed : public Database::Exception {
public:
  ConnectionFailed(const std::string& _conn_info) : Exception("Connection failed: " + _conn_info) {
  }
};


class ResultFailed : public Database::Exception {
public:
  ResultFailed(const std::string& _query) : Exception("Result failed: " + _query) {
  }
};


class OutOfRange : public Database::Exception {
public:
  OutOfRange(unsigned _min, unsigned _max, unsigned _given) : Exception("Out of range:") {
    std::stringstream what;
    what << " field column " << _given << " is out of range [" << _min << ".." << _max << ")";
    what_ += what.str();
  }
};


class NoSuchField : public Database::Exception {
public:
  NoSuchField(const std::string& _name) : Exception("No such field name: '" + _name + "'") { }
};


class NoDataFound : public Database::Exception {
public:
  NoDataFound(const std::string& _reason) : Exception("No data found: " + _reason ) { }
};

}

#endif
