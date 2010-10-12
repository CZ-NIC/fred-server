#ifndef CORBA_CONVERT_H_
#define CORBA_CONVERT_H_

#include "corba/MojeID.hh"
#include "register/mojeid/nullable.h"
#include "register/mojeid/contact.h"

#include <string>
#include <boost/algorithm/string.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>


using namespace Registry::MojeID;


char* corba_wrap_string(const char* _s)
{
    return CORBA::string_dup(_s);
}


const char* corba_wrap_string(const std::string &_s)
{
    return corba_wrap_string(_s.c_str());
}


NullableString* corba_wrap_nullable_string(const Nullable<std::string> &_v)
{
    if (_v.isnull()) {
        return 0;
    }
    else {
        return new NullableString(static_cast<std::string>(_v).c_str());
    }
}


NullableBoolean* corba_wrap_nullable_boolean(const Nullable<bool> &_v)
{
    if (_v.isnull()) {
        return 0;
    }
    else {
        return new NullableBoolean(static_cast<bool>(_v));
    }
}


NullableULongLong* corba_wrap_nullable_ulonglong(const Nullable<unsigned long long> &_v)
{
    if (_v.isnull()) {
        return 0;
    }
    else {
        return new NullableULongLong(static_cast<unsigned long long>(_v));
    }
}


NullableDate* corba_wrap_nullable_date(const Nullable<std::string> &_v)
{
    if (_v.isnull()) {
        return 0;
    }
    else {
        boost::gregorian::date tmp
            = boost::gregorian::from_string(static_cast<std::string>(_v));
        if (tmp.is_special()) {
            return 0;
        }
        else {
            Date d;
            d.year  = static_cast<int>(tmp.year());
            d.month = static_cast<int>(tmp.month());
            d.day   = static_cast<int>(tmp.day());
            return new NullableDate(d);
        }
    }
}


std::string corba_unwrap_string(const CORBA::String_member &_s)
{
    return static_cast<std::string>(_s);
}


Nullable<unsigned long long> corba_unwrap_nullable_ulonglong(const NullableULongLong *_v)
{
    if (_v) {
        return static_cast<unsigned long long>(_v->_value());
    }
    else {
        return Nullable<unsigned long long>();
    }
}


Nullable<std::string> corba_unwrap_nullable_string(const NullableString *_v)
{
    if (_v) {
        return std::string(_v->_value());
    }
    else {
        return Nullable<std::string>();
    }
}


Nullable<bool> corba_unwrap_nullable_boolean(const NullableBoolean *_v)
{
    if (_v) {
        return _v->_value();
    }
    else {
        return Nullable<bool>();
    }
}


Nullable<std::string> corba_unwrap_nullable_date(const NullableDate *_v)
{
    if (_v) {
        boost::gregorian::date tmp(_v->_value().year,
                                  _v->_value().month,
                                  _v->_value().day);
        return to_iso_extended_string(tmp);
    }
    else {
        return Nullable<std::string>();
    }
}


MojeID::Contact corba_unwrap_contact(const Contact &_contact)
{
    MojeID::Contact data;

    for (unsigned int i = 0; i < _contact.phones.length(); ++i) {
        std::string type = corba_unwrap_string(_contact.phones[i].type);
        if (type == "DEFAULT") {
            data.telephone = corba_unwrap_string(_contact.phones[i].number);
        }
        else if (type == "FAX") {
            data.fax = corba_unwrap_string(_contact.phones[i].number);
        }
    }

    for (unsigned int i = 0; i < _contact.emails.length(); ++i) {
        std::string type = corba_unwrap_string(_contact.emails[i].type);
        if (type == "DEFAULT") {
            data.email = corba_unwrap_string(_contact.emails[i].email_address);
        }
        else if (type == "NOTIFY") {
            data.notifyemail = corba_unwrap_string(_contact.emails[i].email_address);
        }
    }

    if (_contact.ssn_type) {
        std::string type = boost::to_upper_copy(static_cast<std::string>(_contact.ssn_type->_value()));
        data.ssntype = type;
        if (type == "OP")            data.ssn = corba_unwrap_nullable_string(_contact.id_card_num);
        else if (type == "PASS")     data.ssn = corba_unwrap_nullable_string(_contact.passport_num);
        else if (type == "ICO")      data.ssn = corba_unwrap_nullable_string(_contact.vat_id_num);
        else if (type == "MPSV")     data.ssn = corba_unwrap_nullable_string(_contact.ssn_id_num);
        else if (type == "BIRTHDAY") data.ssn = corba_unwrap_nullable_date(_contact.birth_date);
    }

    data.id = corba_unwrap_nullable_ulonglong(_contact.id);
    data.name = corba_unwrap_string(_contact.first_name) + " " + corba_unwrap_string(_contact.last_name);
    data.handle = corba_unwrap_string(_contact.username);
    data.organization = corba_unwrap_nullable_string(_contact.organization);
    data.street1 = corba_unwrap_string(_contact.addresses[0].street1);
    data.street2 = corba_unwrap_nullable_string(_contact.addresses[0].street2);
    data.street3 = corba_unwrap_nullable_string(_contact.addresses[0].street3);
    data.city = corba_unwrap_string(_contact.addresses[0].city);
    data.stateorprovince = corba_unwrap_nullable_string(_contact.addresses[0].state);
    data.postalcode = corba_unwrap_string(_contact.addresses[0].postal_code);
    data.country = corba_unwrap_string(_contact.addresses[0].country);
    data.vat = corba_unwrap_nullable_string(_contact.vat_reg_num);
    data.disclosename = corba_unwrap_nullable_boolean(_contact.disclose_name);
    data.discloseorganization = corba_unwrap_nullable_boolean(_contact.disclose_organization);
    data.discloseaddress = corba_unwrap_nullable_boolean(_contact.disclose_address);
    data.disclosetelephone = corba_unwrap_nullable_boolean(_contact.disclose_phone);
    data.disclosefax = corba_unwrap_nullable_boolean(_contact.disclose_fax);
    data.discloseemail = corba_unwrap_nullable_boolean(_contact.disclose_email);
    data.disclosenotifyemail = corba_unwrap_nullable_boolean(_contact.disclose_notify_email);
    data.disclosevat = corba_unwrap_nullable_boolean(_contact.disclose_vat);
    data.discloseident = corba_unwrap_nullable_boolean(_contact.disclose_ident);

    return data;
}

Contact* corba_wrap_contact(const MojeID::Contact &_contact)
{
    Contact *data = new Contact();
    data->id           = corba_wrap_nullable_ulonglong(_contact.id);
    data->username     = corba_wrap_string(_contact.handle);

    std::string name = static_cast<std::string>(_contact.name);
    std::size_t pos = name.find_last_of(" ");
    data->first_name   = corba_wrap_string(name.substr(0, pos));
    data->last_name    = corba_wrap_string(name.substr(pos + 1));

    data->organization = corba_wrap_nullable_string(_contact.organization);
    data->vat_reg_num  = corba_wrap_nullable_string(_contact.vat);
    data->ssn_type     = corba_wrap_nullable_string(_contact.ssntype);

    std::string type = static_cast<std::string>(_contact.ssntype);
    data->id_card_num  = type == "OP"       ? corba_wrap_nullable_string(_contact.ssn) : 0;
    data->passport_num = type == "PASS"     ? corba_wrap_nullable_string(_contact.ssn) : 0;
    data->vat_id_num   = type == "ICO"      ? corba_wrap_nullable_string(_contact.ssn) : 0;
    data->ssn_id_num   = type == "MPSV"     ? corba_wrap_nullable_string(_contact.ssn) : 0;
    data->birth_date   = type == "BIRTHDAY" ? corba_wrap_nullable_date(_contact.ssn) : 0;

    data->disclose_name         = corba_wrap_nullable_boolean(_contact.disclosename);
    data->disclose_organization = corba_wrap_nullable_boolean(_contact.discloseorganization);
    data->disclose_vat          = corba_wrap_nullable_boolean(_contact.disclosevat);
    data->disclose_ident        = corba_wrap_nullable_boolean(_contact.discloseident);
    data->disclose_email        = corba_wrap_nullable_boolean(_contact.discloseemail);
    data->disclose_notify_email = corba_wrap_nullable_boolean(_contact.disclosenotifyemail);
    data->disclose_address      = corba_wrap_nullable_boolean(_contact.discloseaddress);
    data->disclose_phone        = corba_wrap_nullable_boolean(_contact.disclosetelephone);
    data->disclose_fax          = corba_wrap_nullable_boolean(_contact.disclosefax);

    data->addresses.length(1);
    data->addresses[0].type         = "DEFAULT";
    data->addresses[0].street1      = corba_wrap_string(_contact.street1);
    data->addresses[0].street2      = corba_wrap_nullable_string(_contact.street2);
    data->addresses[0].street3      = corba_wrap_nullable_string(_contact.street3);
    data->addresses[0].city         = corba_wrap_string(_contact.city);
    data->addresses[0].state        = corba_wrap_nullable_string(_contact.stateorprovince);
    data->addresses[0].postal_code  = corba_wrap_string(_contact.postalcode);
    data->addresses[0].country      = corba_wrap_string(_contact.country);

    data->emails.length(1);
    data->emails[0].type = "DEFAULT";
    data->emails[0].email_address = corba_wrap_string(_contact.email);
    std::string notify_email = static_cast<std::string>(_contact.notifyemail);
    if (notify_email.size()) {
        data->emails.length(2);
        data->emails[1].type = "NOTIFY";
        data->emails[1].email_address = corba_wrap_string(notify_email);
    }

    std::string telephone = static_cast<std::string>(_contact.telephone);
    std::string fax = static_cast<std::string>(_contact.fax);
    if (telephone.size()) {
        data->phones.length(1);
        data->phones[0].type = "DEFAULT";
        data->phones[0].number = corba_wrap_string(telephone);
    }
    if (fax.size()) {
        unsigned int s = data->phones.length();
        data->phones.length(s + 1);
        data->phones[s].type = "FAX";
        data->phones[s].number = corba_wrap_string(fax);
    }

    return data;
}


#endif /*CORBA_CONVERT_H_*/
