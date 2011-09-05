#ifndef EXCEPTIONS_H_
#define EXCEPTIONS_H_

#include <string>
#include <exception>
#include <stdexcept>

namespace Fred {

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

/// object is not blocked - e.g. registrar not blocked
struct NOT_BLOCKED : public std::runtime_error
{
    NOT_BLOCKED() : std::runtime_error("object is not blocked") { };
};

};

#endif
