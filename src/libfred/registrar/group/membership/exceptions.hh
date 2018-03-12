#ifndef EXCEPIONS_H_2361234134_
#define EXCEPIONS_H_2361234134_

#include <exception>

struct WrongIntervalOrder : std::exception
{
    virtual const char* what() const throw()
    {
        return "date from is later than date to";
    }
};

struct IntervalIntersection : std::exception
{
    virtual const char* what() const throw()
    {
        return "new membership starts when old one is active";
    }
};

struct MembershipStartChange : std::exception
{
    virtual const char* what() const throw()
    {
        return "membership starting date must not be changed";
    }
};

struct WrongMembershipEnd : std::exception
{
    virtual const char* what() const throw()
    {
        return "membership infiniteness must not be altered";
    }
};

struct WrongRegistrar : std::exception
{
    virtual const char* what() const throw()
    {
        return "this membership has different registrar";
    }
};

struct WrongGroup : std::exception
{
    virtual const char* what() const throw()
    {
        return "this membership has different registrar group";
    }
};

struct MembershipNotFound : std::exception
{
    virtual const char* what() const throw()
    {
        return "no active membership with given registrar and group found";
    }
};

#endif
