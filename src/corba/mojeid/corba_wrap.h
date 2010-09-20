#ifndef CORBA_WRAP_H_
#define CORBA_WRAP_H_

#include "corba/MojeID.hh"
#include "register/db_settings.h"
#include <string>


const char* corba_wrap_string(const char* _s)
{
    return CORBA::string_dup(_s);
}


const char* corba_wrap_string(const std::string &_s)
{
    return corba_wrap_string(_s.c_str());
}


Registry::NullableString* corba_wrap_nullable_string(const Database::Value &_v)
{
    if (_v.isnull()) {
        return 0;
    }
    else {
        return new Registry::NullableString(
                static_cast<std::string>(_v).c_str());
    }
}


Registry::NullableBoolean* corba_wrap_nullable_boolean(const Database::Value &_v)
{
    if (_v.isnull()) {
        return 0;
    }
    else {
        return new Registry::NullableBoolean(static_cast<bool>(_v));
    }
}


#endif

