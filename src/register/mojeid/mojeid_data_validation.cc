#include "log/logger.h"
#include "register/db_settings.h"
#include "mojeid_data_validation.h"

#include <boost/algorithm/string.hpp>

namespace MojeID {


const boost::regex USERNAME_PATTERN("^[a-z0-9](-?[a-z0-9])*$");
const boost::regex PHONE_PATTERN("^\\+420\\.(60([1-8]|9([134]|2[1-5]))|7(0[0-9]|10|[237]))\\d+");
const boost::regex EMAIL_PATTERN("^[-!#$%&'*+/=?^_`{}|~0-9A-Za-z]+(\\.[-!#$%&'*+/=?^_`{}|~0-9A-Za-z]+)*"
                                 "@(?:[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?\\.)+[A-Za-z]{2,6}\\.?$");

const std::string EMAIL_PHONE_PROTECTION_PERIOD = "1 month";


ContactValidator create_default_contact_validator()
{
    ContactValidator tmp;
    tmp.add_checker(contact_checker_name);
    tmp.add_checker(contact_checker_username);
    tmp.add_checker(contact_checker_address_required);
    tmp.add_checker(contact_checker_email_format);
    tmp.add_checker(contact_checker_email_required);
    tmp.add_checker(contact_checker_birthday);
    return tmp;
}


ContactValidator create_conditional_identification_validator()
{
    ContactValidator tmp = create_default_contact_validator();
    tmp.add_checker(contact_checker_email_unique);
    tmp.add_checker(contact_checker_address_country_cz);
    tmp.add_checker(contact_checker_phone_format);
    tmp.add_checker(contact_checker_phone_required);
    tmp.add_checker(contact_checker_phone_unique);
    return tmp;
}


ContactValidator create_identification_validator()
{
    ContactValidator tmp = create_default_contact_validator();
    tmp.add_checker(contact_checker_address_country_cz);
    tmp.add_checker(contact_checker_email_unique);
    tmp.add_checker(contact_checker_phone_format);
    return tmp;
}


ContactValidator create_contact_update_validator()
{
    ContactValidator tmp = create_default_contact_validator();
    tmp.add_checker(contact_checker_phone_format);
    return tmp;
}


bool contact_checker_name(const ::MojeID::Contact &_data, FieldErrorMap &_errors)
{
    bool result = true;

    std::string name = boost::algorithm::trim_copy(static_cast<std::string>(_data.name));
    std::size_t pos = name.find_last_of(" ");
    std::string fname = name.substr(0, pos);
    std::string lname = (pos == std::string::npos) ? "" : name.substr(pos + 1);
    if (fname.empty()) {
        _errors[field_first_name] = REQUIRED;
        result = false;
    }
    if (lname.empty()) {
        _errors[field_last_name] = REQUIRED;
        result = false;
    }

    return result;
}


bool contact_checker_username(const ::MojeID::Contact &_data, FieldErrorMap &_errors)
{
    bool result = true;
    /* contact handle has to be in domain-token
     * pattern ^[a-z0-9](-?[a-z0-9])*$ and max length 30
     */
    if (boost::algorithm::trim_copy(_data.handle).empty()) {
        _errors[field_username] = REQUIRED;
        result = false;
    }
    else if (_data.handle.length() > 30) {
        _errors[field_username] = INVALID;
        result = false;
    }
    else if (!boost::regex_search(
                boost::to_lower_copy(_data.handle),
                USERNAME_PATTERN)) {
        _errors[field_username] = INVALID;
        result = false;
    }

    return result;
}


bool contact_checker_phone_format(const ::MojeID::Contact &_data, FieldErrorMap &_errors)
{
    bool result = true;
    /* main phone has to be in format (czech mobile operators):
     * ^\+420\.(60([1-8]|9([134]|2[1-5]))|7(0[0-9]|10|[237]))\d+
     * and 14 characters long
     */
    if (boost::algorithm::trim_copy(static_cast<std::string>(_data.telephone)).length() > 0
                && static_cast<std::string>(_data.telephone).length() != 14) {
        _errors[field_phone] = INVALID;
        result = false;
    }
    else if (boost::algorithm::trim_copy(static_cast<std::string>(_data.telephone)).length() > 0
                && !boost::regex_search(
                        static_cast<std::string>(_data.telephone),
                        PHONE_PATTERN)) {
        _errors[field_phone] = INVALID;
        result = false;
    }

    return result;
}


bool contact_checker_phone_required(const ::MojeID::Contact &_data, FieldErrorMap &_errors)
{
    bool result = true;

    if (_data.telephone.isnull()
            || boost::algorithm::trim_copy(static_cast<std::string>(_data.telephone)).empty()) {
        _errors[field_phone] = REQUIRED;
        result = false;
    }

    return result;
}


bool contact_checker_phone_unique(const ::MojeID::Contact &_data, FieldErrorMap &_errors)
{
    bool result = true;

    if (contact_checker_phone_required(_data, _errors)) {
        Database::Connection conn = Database::Manager::acquire();
        Database::Result ucheck = conn.exec_params(
                "SELECT os.id, os.valid_from, ch.id, ch.historyid, ch.email, ch.telephone"
                " FROM contact_history ch"
                " JOIN object_state os ON os.ohid_from = ch.historyid"
                " JOIN enum_object_states eos ON eos.id = os.state_id"
                " WHERE eos.name = 'conditionallyIdentifiedContact'"
                " AND os.valid_from + $1::interval > now()"
                " AND trim(both ' ' from ch.telephone) = trim(both ' ' from $2::text)"
                " AND ch.id != $3::integer"
                " ORDER BY os.valid_from ASC"
                " LIMIT 1",
                Database::query_param_list
                    (EMAIL_PHONE_PROTECTION_PERIOD)
                    (static_cast<std::string>(_data.telephone))
                    (static_cast<unsigned long long>(_data.id)));

        if (ucheck.size() > 0) {
            _errors[field_phone] = NOT_AVAILABLE;
            result = false;
        }
    }
    else {
        result = false;
    }

    return result;
}


bool contact_checker_email_format(const ::MojeID::Contact &_data, FieldErrorMap &_errors)
{
    bool result = true;

    if (boost::algorithm::trim_copy(static_cast<std::string>(_data.email)).length() > 0
            && !boost::regex_search(
                    static_cast<std::string>(_data.email),
                    EMAIL_PATTERN)) {
        _errors[field_email] = INVALID;
        result = false;
    }

    return result;
}


bool contact_checker_email_required(const ::MojeID::Contact &_data, FieldErrorMap &_errors)
{
    bool result = true;

    if (_data.email.isnull()
            || boost::algorithm::trim_copy(static_cast<std::string>(_data.email)).empty()) {
        _errors[field_email] = REQUIRED;
        result = false;
    }

    return result;
}


bool contact_checker_email_unique(const ::MojeID::Contact &_data, FieldErrorMap &_errors)
{
    bool result = true;

    if (contact_checker_email_required(_data, _errors)) {
        Database::Connection conn = Database::Manager::acquire();
        Database::Result ucheck = conn.exec_params(
                "SELECT os.id, os.valid_from, ch.id, ch.historyid, ch.email, ch.telephone"
                " FROM contact_history ch"
                " JOIN object_state os ON os.ohid_from = ch.historyid"
                " JOIN enum_object_states eos ON eos.id = os.state_id"
                " WHERE eos.name =ANY ('{conditionallyIdentifiedContact, identifiedContact}'::text[])"
                " AND os.valid_from + $1::interval > now()"
                " AND trim(both ' ' from LOWER(ch.email)) = trim(both ' ' from LOWER($2::text))"
                " AND ch.id != $3::integer"
                " ORDER BY os.valid_from ASC"
                " LIMIT 1",
                Database::query_param_list
                    (EMAIL_PHONE_PROTECTION_PERIOD)
                    (static_cast<std::string>(_data.email))
                    (static_cast<unsigned long long>(_data.id)));

        if (ucheck.size() > 0) {
            _errors[field_email] = NOT_AVAILABLE;
            result = false;
        }
    }
    else {
        result = false;
    }

    return result;
}


bool contact_checker_address_required(const ::MojeID::Contact &_data, FieldErrorMap &_errors)
{
    bool result = true;
    /* main address is required */
    if (_data.street1.isnull()
            || boost::algorithm::trim_copy(static_cast<std::string>(_data.street1)).empty()) {
        _errors[field_street1] = REQUIRED;
        result = false;
    }
    if (_data.city.isnull()
            || boost::algorithm::trim_copy(static_cast<std::string>(_data.city)).empty()) {
        _errors[field_city] = REQUIRED;
        result = false;
    }
    if (_data.postalcode.isnull()
            || boost::algorithm::trim_copy(static_cast<std::string>(_data.postalcode)).empty()) {
        _errors[field_postal_code] = REQUIRED;
        result = false;
    }
    if (_data.country.isnull()
            || boost::algorithm::trim_copy(static_cast<std::string>(_data.country)).empty()) {
        _errors[field_country] = REQUIRED;
        result = false;
    }

    return result;
}


bool contact_checker_address_country_cz(const ::MojeID::Contact &_data, FieldErrorMap &_errors)
{
    bool result = true;

    if (static_cast<std::string>(_data.country) != "CZ") {
        _errors[field_country] = INVALID;
        result = false;
    }
}


bool contact_checker_birthday(const ::MojeID::Contact &_data, FieldErrorMap &_errors)
{
    bool result = true;

    if (!_data.ssntype.isnull() && static_cast<std::string>(_data.ssntype) == "BIRTHDAY") {
        try {
            boost::gregorian::date tmp
                = boost::gregorian::from_string(static_cast<std::string>(_data.ssn));
            if (tmp.is_special()) {
                throw;
            }
        }
        catch (...) {
            _errors[field_birth_date] = INVALID;
            result = false;
        }
    }

    return result;
}



bool check_validated_contact_diff(
        const ::MojeID::Contact &_c1,
        const ::MojeID::Contact &_c2)
{
    /* name */
    if (_c1.name != _c2.name) {
        return false;
    }
    /* organization */
    if (static_cast<std::string>(_c1.organization) != static_cast<std::string>(_c2.organization)) {
        return false;
    }
    /* dic */
    if (static_cast<std::string>(_c1.vat) != static_cast<std::string>(_c2.vat)) {
        return false;
    }
    /* address */
    if ((static_cast<std::string>(_c1.street1) != static_cast<std::string>(_c2.street1))
            || (static_cast<std::string>(_c1.street2) != static_cast<std::string>(_c2.street2))
            || (static_cast<std::string>(_c1.street3) != static_cast<std::string>(_c2.street3))
            || (static_cast<std::string>(_c1.city) != static_cast<std::string>(_c2.city))
            || (static_cast<std::string>(_c1.stateorprovince) != static_cast<std::string>(_c2.stateorprovince))
            || (static_cast<std::string>(_c1.country) != static_cast<std::string>(_c2.country))
            || (static_cast<std::string>(_c1.postalcode) != static_cast<std::string>(_c2.postalcode))) {
        return false;
    }
    /* birthday and ico*/
    if (static_cast<std::string>(_c1.ssn) != static_cast<std::string>(_c2.ssn)) {
        return false;
    }

    return true;
}


}

