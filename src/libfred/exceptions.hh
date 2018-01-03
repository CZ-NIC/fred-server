#ifndef EXCEPTIONS_HH_F24345F27301491EA2CE1945B673BC16
#define EXCEPTIONS_HH_F24345F27301491EA2CE1945B673BC16

#include <string>
#include <exception>
#include <stdexcept>

namespace LibFred {

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

struct INVALID_VALUE : public std::runtime_error
{
    INVALID_VALUE(const std::string &what) : std::runtime_error(what) { };

    INVALID_VALUE() : std::runtime_error("invalid value specified") { };
};

};

#endif
