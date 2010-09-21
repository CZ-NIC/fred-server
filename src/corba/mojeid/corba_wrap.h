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


Registry::NullableDate* corba_wrap_nullable_date(const Database::Value &_v)
{
    if (_v.isnull()) {
        return 0;
    }
    else {
        boost::gregorian::date tmp = from_string(static_cast<std::string>(_v));
        if (tmp.is_special()) {
            return 0;
        }
        else {
            Registry::Date d;
            d.year  = static_cast<int>(tmp.year());
            d.month = static_cast<int>(tmp.month());
            d.day   = static_cast<int>(tmp.day());
            return new Registry::NullableDate(d);
        }
    }
}


std::string corba_unwrap_string(const CORBA::String_member &_s)
{
    return static_cast<std::string>(_s);
}


Database::QueryParam corba_unwrap_nullable_string(const Registry::NullableString *_v)
{
    if (_v) {
        return std::string(_v->_value());
    }
    else {
        return Database::QueryParam();
    }
}


Database::QueryParam corba_unwrap_nullable_boolean(const Registry::NullableBoolean *_v)
{
    if (_v) {
        return _v->_value();
    }
    else {
        return Database::QueryParam();
    }
}


Database::QueryParam corba_unwrap_nullable_date(const Registry::NullableDate *_v)
{
    if (_v) {
        return boost::gregorian::date(_v->_value().year,
                                      _v->_value().month,
                                      _v->_value().day);
    }
    else {
        return Database::QueryParam();
    }
}



#endif

