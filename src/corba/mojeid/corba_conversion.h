#ifndef CORBA_CONVERT_H_
#define CORBA_CONVERT_H_

#include "src/corba/MojeID.hh"
#include "src/corba/common_wrappers.h"
#include "util/types/birthdate.h"

#include "src/fredlib/contact_verification/contact.h"
#include "src/fredlib/contact_verification/contact_validator.h"

#include <string>
#include <boost/algorithm/string.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/assign/list_of.hpp>


using namespace Registry::MojeID;

Date corba_wrap_date(const boost::gregorian::date &_v)
{
    Date d;
    if (_v.is_special()) {
        d.year  = 0;
        d.month = 0;
        d.day   = 0;
    }
    else {
        d.year  = static_cast<int>(_v.year());
        d.month = static_cast<int>(_v.month());
        d.day   = static_cast<int>(_v.day());
    }
    return d;
}

NullableString* corba_wrap_nullable_string(const Nullable<std::string> &_v)
{
    if (_v.isnull()) {
        return 0;
    }
    else {
        return new NullableString(_v.get_value().c_str());
    }
}

NullableBoolean* corba_wrap_nullable_boolean(const Nullable<bool> &_v)
{
    if (_v.isnull()) {
        return 0;
    }
    else {
        return new NullableBoolean(_v.get_value());
    }
}

NullableBoolean* corba_wrap_nullable_boolean(const bool _v)
{
    return new NullableBoolean(_v);
}


NullableULongLong* corba_wrap_nullable_ulonglong(const Nullable<unsigned long long> &_v)
{
    if (_v.isnull()) {
        return 0;
    }
    else {
        return new NullableULongLong(_v.get_value());
    }
}


NullableDate* corba_wrap_nullable_date(const Nullable<std::string> &_v)
{
    if (_v.isnull()) {
        return 0;
    }
    else {
        boost::gregorian::date tmp = birthdate_from_string_to_date(_v.get_value());
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

bool is_not_normal(int c)
{
    return c == '\r' || c == '\n' || c == '\t';
}

Nullable<std::string> corba_unwrap_normalize_nullable_string(const NullableString *_v)
{
    if (_v)
    {
        std::string tmp ( _v->_value());
        //normalize
        tmp.erase(std::remove_if(tmp.begin(), tmp.end(), is_not_normal)
                , tmp.end());

        return tmp;
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

bool corba_unwrap_nullable_boolean(const NullableBoolean *_v, const bool null_value)
{
    if (_v) {
        return _v->_value();
    }
    else {
        return null_value;
    }
}


Nullable<std::string> corba_unwrap_nullable_date(const NullableDate *_v)
{
    if (_v) {
        boost::format date_fmt = boost::format("%1%-%2$02d-%3$02d")
                            % _v->_value().year
                            % _v->_value().month
                            % _v->_value().day;

        return Nullable<std::string>(date_fmt.str());
    }
    else {
        return Nullable<std::string>();
    }
}


Nullable<std::pair<std::string, std::string> > corba_unwrap_ssn_value_by_priority(const Contact& _contact)
{
    typedef std::pair<std::string, Nullable<std::string> > SSN;
    /* NOTE: all detections for not set value on Nullables here is done by default value getter
     * because frontend don't preseve NULL values :( */

    /* priority order depends on organization value */
    bool is_org = !(corba_unwrap_nullable_string(_contact.organization).get_value_or_default().empty());

    SSN ico = std::make_pair("ICO", corba_unwrap_nullable_string(_contact.vat_id_num));
    SSN birthday = std::make_pair("BIRTHDAY", corba_unwrap_nullable_date(_contact.birth_date));

    /* list of all possible values ordered by priority (higher first) */
    const std::vector<SSN> all_values = boost::assign::list_of
        (is_org ? ico : birthday)
        (is_org ? birthday : ico)
        (std::make_pair("OP", corba_unwrap_nullable_string(_contact.id_card_num)))
        (std::make_pair("PASS", corba_unwrap_nullable_string(_contact.passport_num)))
        (std::make_pair("MPSV", corba_unwrap_nullable_string(_contact.ssn_id_num)));

    /* get first nonempty value */
    Nullable<std::pair<std::string, std::string> > ret;
    for (std::vector<SSN>::const_iterator it = all_values.begin(); it != all_values.end(); ++it)
    {
        if (!(it->second.get_value_or_default().empty()))
        {
            ret = std::make_pair(it->first, it->second.get_value_or_default());
            break;
        }
    }
    return ret;
}


Fred::Contact::Verification::Contact corba_unwrap_contact(const Contact &_contact)
{
    Fred::Contact::Verification::Contact data;

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

    for (unsigned int i = 0; i < _contact.addresses.length(); ++i) {
        std::string type = corba_unwrap_string(_contact.addresses[i].type);
        if (type == "DEFAULT") {
            data.street1 = corba_unwrap_string(_contact.addresses[i].street1);
            data.street2 = corba_unwrap_nullable_string(_contact.addresses[i].street2);
            data.street3 = corba_unwrap_nullable_string(_contact.addresses[i].street3);
            data.city = corba_unwrap_string(_contact.addresses[i].city);
            data.stateorprovince = corba_unwrap_nullable_string(_contact.addresses[i].state);
            data.postalcode = corba_unwrap_string(_contact.addresses[i].postal_code);
            data.country = corba_unwrap_string(_contact.addresses[i].country);
        }
        else {
            Fred::Contact::Verification::ContactAddress addr;
            addr.type = type;
            addr.company_name = corba_unwrap_nullable_string(_contact.addresses[i].company_name);
            addr.street1 = corba_unwrap_string(_contact.addresses[i].street1);
            addr.street2 = corba_unwrap_nullable_string(_contact.addresses[i].street2);
            addr.street3 = corba_unwrap_nullable_string(_contact.addresses[i].street3);
            addr.city = corba_unwrap_string(_contact.addresses[i].city);
            addr.stateorprovince = corba_unwrap_nullable_string(_contact.addresses[i].state);
            addr.postalcode = corba_unwrap_string(_contact.addresses[i].postal_code);
            addr.country = corba_unwrap_string(_contact.addresses[i].country);
            data.addresses.push_back(addr);
        }
    }

    data.id = corba_unwrap_nullable_ulonglong(_contact.id).get_value_or_default();
    data.name = corba_unwrap_string(_contact.first_name) + " " + corba_unwrap_string(_contact.last_name);
    data.handle = corba_unwrap_string(_contact.username);
    data.organization = corba_unwrap_nullable_string(_contact.organization);
    data.vat = corba_unwrap_nullable_string(_contact.vat_reg_num);

    Nullable<std::pair<std::string, std::string> > ssn = corba_unwrap_ssn_value_by_priority(_contact);
    if (!ssn.isnull())
    {
        data.ssntype = ssn.get_value().first;
        data.ssn = ssn.get_value().second;
    }

    return data;
}

Contact* corba_wrap_contact(const Fred::Contact::Verification::Contact &_contact)
{
    Contact *data = new Contact();
    data->id           = corba_wrap_nullable_ulonglong(_contact.id);
    data->username     = corba_wrap_string(_contact.handle);

    std::string name = boost::algorithm::trim_copy(static_cast<std::string>(_contact.name));
    std::size_t pos = name.find_last_of(" ");
    data->first_name   = corba_wrap_string(name.substr(0, pos));
    data->last_name    = (pos == std::string::npos) ? corba_wrap_string("")
                                                    : corba_wrap_string(name.substr(pos + 1));

    data->organization = corba_wrap_nullable_string(_contact.organization);
    data->vat_reg_num  = corba_wrap_nullable_string(_contact.vat);

    std::string type = _contact.ssntype.get_value_or_default();
    data->id_card_num  = type == "OP"       ? corba_wrap_nullable_string(_contact.ssn) : 0;
    data->passport_num = type == "PASS"     ? corba_wrap_nullable_string(_contact.ssn) : 0;
    data->vat_id_num   = type == "ICO"      ? corba_wrap_nullable_string(_contact.ssn) : 0;
    data->ssn_id_num   = type == "MPSV"     ? corba_wrap_nullable_string(_contact.ssn) : 0;
    data->birth_date   = type == "BIRTHDAY" ? corba_wrap_nullable_date(_contact.ssn) : 0;

    data->addresses.length(1);
    data->addresses[0].type         = "DEFAULT";
    data->addresses[0].street1      = corba_wrap_string(_contact.street1.get_value_or_default());
    data->addresses[0].street2      = corba_wrap_nullable_string(_contact.street2);
    data->addresses[0].street3      = corba_wrap_nullable_string(_contact.street3);
    data->addresses[0].city         = corba_wrap_string(_contact.city.get_value_or_default());
    data->addresses[0].state        = corba_wrap_nullable_string(_contact.stateorprovince);
    data->addresses[0].postal_code  = corba_wrap_string(_contact.postalcode.get_value_or_default());
    data->addresses[0].country      = corba_wrap_string(_contact.country.get_value_or_default());

    for (std::vector<Fred::Contact::Verification::ContactAddress>::const_iterator i = _contact.addresses.begin();
            i != _contact.addresses.end(); ++i)
    {
        unsigned int j = data->addresses.length();
        data->addresses.length(j + 1);
        data->addresses[j].type         = corba_wrap_string(i->type);
        data->addresses[j].company_name = corba_wrap_nullable_string(i->company_name);
        data->addresses[j].street1      = corba_wrap_string(i->street1.get_value());
        data->addresses[j].street2      = corba_wrap_nullable_string(i->street2);
        data->addresses[j].street3      = corba_wrap_nullable_string(i->street3);
        data->addresses[j].city         = corba_wrap_string(i->city.get_value());
        data->addresses[j].state        = corba_wrap_nullable_string(i->stateorprovince);
        data->addresses[j].postal_code  = corba_wrap_string(i->postalcode.get_value());
        data->addresses[j].country      = corba_wrap_string(i->country.get_value());
    }

    data->emails.length(1);
    data->emails[0].type = "DEFAULT";
    data->emails[0].email_address = corba_wrap_string(_contact.email.get_value_or_default());
    std::string notify_email = _contact.notifyemail.get_value_or_default();
    if (notify_email.size()) {
        data->emails.length(2);
        data->emails[1].type = "NOTIFY";
        data->emails[1].email_address = corba_wrap_string(notify_email);
    }

    std::string telephone = _contact.telephone.get_value_or_default();
    std::string fax = _contact.fax.get_value_or_default();
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


Registry::MojeID::Server::ValidationError corba_wrap_validation_error(
        const Fred::Contact::Verification::ValidationError &_value)
{
    switch (_value) {
        case Fred::Contact::Verification::NOT_AVAILABLE:
            return Registry::MojeID::Server::NOT_AVAILABLE;
        case Fred::Contact::Verification::INVALID:
            return Registry::MojeID::Server::INVALID;
        case Fred::Contact::Verification::REQUIRED:
            return Registry::MojeID::Server::REQUIRED;
        default:
            throw std::runtime_error("unknown validation error type");
    }
}


Registry::MojeID::Server::ValidationErrorList_var corba_wrap_validation_error_list(
        const Fred::Contact::Verification::FieldErrorMap &_errors)
{
    Registry::MojeID::Server::ValidationErrorList_var cerrors
        = new Registry::MojeID::Server::ValidationErrorList;
    cerrors->length(_errors.size());

    Fred::Contact::Verification::FieldErrorMap::const_iterator it = _errors.begin();
    Fred::Contact::Verification::FieldErrorMap::size_type i = 0;
    for (; it != _errors.end(); ++it, ++i) {
        cerrors[i].name = corba_wrap_string(it->first);
        cerrors[i].error = corba_wrap_validation_error(it->second);
    }

    return cerrors;
}



#endif /*CORBA_CONVERT_H_*/

