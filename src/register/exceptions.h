#ifndef EXCEPTIONS_H_
#define EXCEPTIONS_H_

#include <string>
#include <exception>
#include <stdexcept>

namespace Register {

/// Exception when SQL error
struct SQL_ERROR : public std::runtime_error
{
    SQL_ERROR() : std::runtime_error("sql error") { }
};


/// Exception when specified object is not found
struct NOT_FOUND : public std::runtime_error
{
    NOT_FOUND() : std::runtime_error("not found") { }
};


/// Exception when specified object already exists
struct ALREADY_EXISTS : public std::runtime_error
{
    ALREADY_EXISTS() : std::runtime_error("already exists") { }
};

}


#endif
