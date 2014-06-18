#include "log/logger.h"
#include "src/fredlib/db_settings.h"
#include "util/types/birthdate.h"
#include "src/fredlib/contact_verification/contact_verification_checkers.h"

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
                _data.telephone.get_value_or_default(),
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
            _data.fax.get_value_or_default(),
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
            || boost::algorithm::trim_copy(_data.telephone.get_value()).empty()) {
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
                " AND ch.id != $3::bigint"
                " LIMIT 1",
                Database::query_param_list
                    (EMAIL_PHONE_PROTECTION_PERIOD)
                    (_data.telephone.get_value_or_default())
                    (_data.id));

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
    bool result = generic_checker_email_format(_data.email.get_value_or_default());
    if (result == false) {
        _errors[field_email] = INVALID;
    }
    return result;
}


bool contact_checker_notify_email_format(const Contact &_data, FieldErrorMap &_errors)
{
    bool result = generic_checker_email_format(_data.notifyemail.get_value_or_default());
    if (result == false) {
       _errors[field_notify_email] = INVALID;
    }
    return result;
}


bool contact_checker_email_required(const Contact &_data, FieldErrorMap &_errors)
{
    bool result = true;

    if (_data.email.isnull()
            || boost::algorithm::trim_copy(_data.email.get_value()).empty()) {
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
                " AND ch.id != $3::bigint"
                " LIMIT 1",
                Database::query_param_list
                    (EMAIL_PHONE_PROTECTION_PERIOD)
                    (_data.email.get_value_or_default())
                    (_data.id));

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
            || boost::algorithm::trim_copy(_data.street1.get_value()).empty()) {
        _errors[field_street1] = REQUIRED;
        result = false;
    }
    if (_data.city.isnull()
            || boost::algorithm::trim_copy(_data.city.get_value()).empty()) {
        _errors[field_city] = REQUIRED;
        result = false;
    }
    if (_data.postalcode.isnull()
            || boost::algorithm::trim_copy(_data.postalcode.get_value()).empty()) {
        _errors[field_postal_code] = REQUIRED;
        result = false;
    }
    if (_data.country.isnull()
            || boost::algorithm::trim_copy(_data.country.get_value()).empty()) {
        _errors[field_country] = REQUIRED;
        result = false;
    }

    return result;
}


bool contact_checker_address_postalcode_format_cz(const Contact &_data, FieldErrorMap &_errors)
{
    bool result = true;

    if (!boost::regex_search(
                    _data.postalcode.get_value_or_default(),
                    POSTALCODE_CZ_PATTERN)) {
        _errors[field_postal_code] = INVALID;
        result = false;
    }

    return result;
}


bool contact_checker_address_country(const Contact &_data, FieldErrorMap &_errors)
{
    bool result = true;

    if ((_data.country.get_value_or_default() != "CZ")
        && (_data.country.get_value_or_default() != "SK")) {
        _errors[field_country] = INVALID;
        result = false;
    }

    return result;
}


}
}
}


