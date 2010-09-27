#ifndef CORBA_WRAP_H_
#define CORBA_WRAP_H_

#include "corba/MojeID.hh"
#include "register/db_settings.h"
#include <string>
#include <boost/algorithm/string.hpp>



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


Registry::NullableULongLong* corba_wrap_nullable_ulonglong(const Database::Value &_v)
{
    if (_v.isnull()) {
        return 0;
    }
    else {
        return new Registry::NullableULongLong(static_cast<unsigned long long>(_v));
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


Database::QueryParam corba_unwrap_nullable_ulonglong(const Registry::NullableULongLong *_v)
{
    if (_v) {
        return static_cast<unsigned long long>(_v->_value());
    }
    else {
        return Database::QueryParam();
    }
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


Database::QueryParams corba_unwrap_contact(const Registry::Contact &_contact)
{
    Database::QueryParam telephone = Database::QueryParam();
    Database::QueryParam fax = Database::QueryParam();
    for (unsigned int i = 0; i <  _contact.phones.length(); ++i) {
        std::string type = corba_unwrap_string(_contact.phones[i].type);
        if (type == "DEFAULT") {
            telephone = corba_unwrap_string(_contact.phones[i].number);
        }
        else if (type == "FAX") {
            fax = corba_unwrap_string(_contact.phones[i].number);
        }
    }

    Database::QueryParam email = Database::QueryParam();
    Database::QueryParam notify_email = Database::QueryParam();
    for (unsigned int i = 0; i < _contact.emails.length(); ++i) {
        std::string type = corba_unwrap_string(_contact.emails[i].type);
        if (type == "DEFAULT") {
            email = corba_unwrap_string(_contact.emails[i].email_address);
        }
        else if (type == "NOTIFY") {
            notify_email = corba_unwrap_string(_contact.emails[i].email_address);
        }
    }

    Database::QueryParam ssn = Database::QueryParam();
    Database::QueryParam ssn_type = Database::QueryParam();
    if (_contact.ssn_type) {
        std::string type = boost::to_upper_copy(static_cast<std::string>(_contact.ssn_type->_value()));
        ssn_type = type;
        if (type == "OP")            ssn = corba_unwrap_nullable_string(_contact.id_card_num);
        else if (type == "PASS")     ssn = corba_unwrap_nullable_string(_contact.passport_num);
        else if (type == "ICO")      ssn = corba_unwrap_nullable_string(_contact.vat_id_num);
        else if (type == "MPSV")     ssn = corba_unwrap_nullable_string(_contact.ssn_id_num);
        else if (type == "BIRTHDAY") ssn = corba_unwrap_nullable_date(_contact.birth_date);
    }

    Database::QueryParams pcontact = Database::query_param_list
            (corba_unwrap_string(_contact.first_name)
                + " " + corba_unwrap_string(_contact.last_name))
            (corba_unwrap_nullable_string(_contact.organization))
            (corba_unwrap_string(_contact.addresses[0].street1))
            (corba_unwrap_nullable_string(_contact.addresses[0].street2))
            (corba_unwrap_nullable_string(_contact.addresses[0].street3))
            (corba_unwrap_string(_contact.addresses[0].city))
            (corba_unwrap_nullable_string(_contact.addresses[0].state))
            (corba_unwrap_string(_contact.addresses[0].postal_code))
            (corba_unwrap_string(_contact.addresses[0].country))
            (telephone)
            (fax)
            (email)
            (notify_email)
            (corba_unwrap_nullable_string(_contact.vat_reg_num))
            (ssn)
            (ssn_type)
            (corba_unwrap_nullable_boolean(_contact.disclose_name))
            (corba_unwrap_nullable_boolean(_contact.disclose_organization))
            (corba_unwrap_nullable_boolean(_contact.disclose_address))
            (corba_unwrap_nullable_boolean(_contact.disclose_phone))
            (corba_unwrap_nullable_boolean(_contact.disclose_fax))
            (corba_unwrap_nullable_boolean(_contact.disclose_email))
            (corba_unwrap_nullable_boolean(_contact.disclose_notify_email))
            (corba_unwrap_nullable_boolean(_contact.disclose_vat))
            (corba_unwrap_nullable_boolean(_contact.disclose_ident));

    return pcontact;
}



#endif

