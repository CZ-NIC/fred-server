#include "log/logger.h"
#include "fredlib/db_settings.h"
#include "contact_verification_checkers.h"

#include <boost/algorithm/string.hpp>

namespace Fred {
namespace Contact {
namespace Verification {





bool contact_checker_name(const Contact &_data, FieldErrorMap &_errors)
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

bool generic_checker_phone_format(const std::string &_phone, const boost::regex &_pattern)
{
    if (boost::algorithm::trim_copy(_phone).length() > 0
                && !boost::regex_search(
                        _phone,
                        _pattern)) {
        return false;
    }
    return true;
}


bool contact_checker_phone_format(const Contact &_data, FieldErrorMap &_errors)
{
    bool result = true;

    if (generic_checker_phone_format(
                static_cast<std::string>(_data.telephone),
                PHONE_CZ_SK_PATTERN) == false) {
        result = false;
    }

    if (result == false) {
        _errors[field_phone] = INVALID;
    }
    return result;
}


bool contact_checker_fax_format(const Contact &_data, FieldErrorMap &_errors)
{
    bool result = generic_checker_phone_format(
            static_cast<std::string>(_data.fax),
            PHONE_PATTERN);
    if (result == false) {
        _errors[field_fax] = INVALID;
    }
    return result;
}


bool contact_checker_phone_required(const Contact &_data, FieldErrorMap &_errors)
{
    bool result = true;

    if (_data.telephone.isnull()
            || boost::algorithm::trim_copy(static_cast<std::string>(_data.telephone)).empty()) {
        _errors[field_phone] = REQUIRED;
        result = false;
    }

    return result;
}


bool contact_checker_phone_unique(const Contact &_data, FieldErrorMap &_errors)
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
                " AND os.valid_from > now() - $1::interval"
                " AND trim(both ' ' from ch.telephone) = trim(both ' ' from $2::text)"
                " LIMIT 1",
                Database::query_param_list
                    (EMAIL_PHONE_PROTECTION_PERIOD)
                    (static_cast<std::string>(_data.telephone)));

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


bool generic_checker_email_format(const std::string &_email)
{
    if (boost::algorithm::trim_copy(_email).length() > 0
            && !boost::regex_search(
                    _email,
                    EMAIL_PATTERN)) {
        return false;
    }
    return true;
}


bool contact_checker_email_format(const Contact &_data, FieldErrorMap &_errors)
{
    bool result = generic_checker_email_format(static_cast<std::string>(_data.email));
    if (result == false) {
        _errors[field_email] = INVALID;
    }
    return result;
}


bool contact_checker_notify_email_format(const Contact &_data, FieldErrorMap &_errors)
{
    bool result = generic_checker_email_format(static_cast<std::string>(_data.notifyemail));
    if (result == false) {
       _errors[field_notify_email] = INVALID;
    }
    return result;
}


bool contact_checker_email_required(const Contact &_data, FieldErrorMap &_errors)
{
    bool result = true;

    if (_data.email.isnull()
            || boost::algorithm::trim_copy(static_cast<std::string>(_data.email)).empty()) {
        _errors[field_email] = REQUIRED;
        result = false;
    }

    return result;
}


bool contact_checker_email_unique(const Contact &_data, FieldErrorMap &_errors)
{
    bool result = true;

    if (contact_checker_email_required(_data, _errors)) {
        Database::Connection conn = Database::Manager::acquire();
        Database::Result ucheck = conn.exec_params(
                "SELECT os.id, os.valid_from, ch.id, ch.historyid, ch.email, ch.telephone"
                " FROM contact_history ch"
                " JOIN object_state os ON os.ohid_from = ch.historyid"
                " JOIN enum_object_states eos ON eos.id = os.state_id"
                " WHERE eos.name = 'conditionallyIdentifiedContact'"
                " AND os.valid_from > now() - $1::interval"
                " AND trim(both ' ' from LOWER(ch.email)) = trim(both ' ' from LOWER($2::text))"
                " LIMIT 1",
                Database::query_param_list
                    (EMAIL_PHONE_PROTECTION_PERIOD)
                    (static_cast<std::string>(_data.email)));

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


bool contact_checker_address_required(const Contact &_data, FieldErrorMap &_errors)
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


bool contact_checker_address_postalcode_format_cz(const Contact &_data, FieldErrorMap &_errors)
{
    bool result = true;

    if (!boost::regex_search(
                    static_cast<std::string>(_data.postalcode),
                    POSTALCODE_CZ_PATTERN)) {
        _errors[field_postal_code] = INVALID;
        result = false;
    }

    return result;
}


bool contact_checker_address_country(const Contact &_data, FieldErrorMap &_errors)
{
    bool result = true;

    if ((static_cast<std::string>(_data.country) != "CZ")
        && (static_cast<std::string>(_data.country) != "SK")) {
        _errors[field_country] = INVALID;
        result = false;
    }

    return result;
}


bool contact_checker_birthday(const Contact &_data, FieldErrorMap &_errors)
{
    bool result = true;

    if (!_data.ssntype.isnull() && static_cast<std::string>(_data.ssntype) == "BIRTHDAY") {
        try {
            boost::gregorian::date tmp
                = boost::gregorian::from_string(static_cast<std::string>(_data.ssn));
            if (tmp.is_special()) {
                throw 0;
            }
        }
        catch (...) {
            _errors[field_birth_date] = INVALID;
            result = false;
        }
    }

    return result;
}

/// return true in case the contacts are equal in terms registry data
bool check_conditionally_identified_contact_diff(
        const Contact &_c1,
        const Contact &_c2)
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
    /* identification type */
    if (static_cast<std::string>(_c1.ssntype) != static_cast<std::string>(_c2.ssntype)) {
        return false;
    }
    /* identification regardless of type*/
    if (static_cast<std::string>(_c1.ssn) != static_cast<std::string>(_c2.ssn)) {

        if (static_cast<std::string>(_c1.ssntype) == "BIRTHDAY") {
            boost::gregorian::date before = boost::gregorian::from_string(static_cast<std::string>(_c1.ssn));
            boost::gregorian::date after = boost::gregorian::from_string(static_cast<std::string>(_c2.ssn));
            if (before != after) {
                return false;
            }

        }
        else {
            return false;
        }
    }
    /* telephone and email */
    if ((static_cast<std::string>(_c1.telephone) != static_cast<std::string>(_c2.telephone))
            || (static_cast<std::string>(_c1.fax) != static_cast<std::string>(_c2.fax))
            || (static_cast<std::string>(_c1.email) != static_cast<std::string>(_c2.email))
            || (static_cast<std::string>(_c1.notifyemail) != static_cast<std::string>(_c2.notifyemail))) {
        return false;
    }
    /* all disclose disclose flags */
    if ( (_c1.disclosename != _c2.disclosename)
            || (_c1.discloseorganization != _c2.discloseorganization)
            || (_c1.discloseaddress != _c2.discloseaddress)
            || (_c1.disclosetelephone != _c2.disclosetelephone)
            || (_c1.disclosefax != _c2.disclosefax)
            || (_c1.discloseemail != _c2.discloseemail)
            || (_c1.disclosevat != _c2.disclosevat)
            || (_c1.discloseident != _c2.discloseident)
            || (_c1.disclosenotifyemail != _c2.disclosenotifyemail)) {
        return false;
    }

    return true;
}


bool check_validated_contact_diff(
        const Contact &_c1,
        const Contact &_c2)
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
    /* identification type */
    if (static_cast<std::string>(_c1.ssntype) != static_cast<std::string>(_c2.ssntype)) {
        return false;
    }
    /* identification regardless of type*/
    if (static_cast<std::string>(_c1.ssn) != static_cast<std::string>(_c2.ssn)) {

        if(static_cast<std::string>(_c1.ssntype) == "BIRTHDAY") {
            boost::gregorian::date before = boost::gregorian::from_string(static_cast<std::string>(_c1.ssn));
            boost::gregorian::date after = boost::gregorian::from_string(static_cast<std::string>(_c2.ssn));
            if(before != after) {
                return false;
            }

        } else {
            return false;
        }
    }

    return true;
}


}
}
}


