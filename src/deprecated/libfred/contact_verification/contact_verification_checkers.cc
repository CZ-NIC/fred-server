#include "util/log/logger.hh"
#include "libfred/db_settings.hh"
#include "src/util/types/birthdate.hh"
#include "util/idn_utils.hh"
#include "src/deprecated/libfred/contact_verification/contact_verification_checkers.hh"
#include "src/deprecated/libfred/contact_verification/contact_verification_validators.hh"

#include <boost/algorithm/string.hpp>
#include "libfred/contact_verification/django_email_format.hh"

namespace LibFred {
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
                        boost::algorithm::trim_copy(_phone),
                        _pattern)) {
        return false;
    }
    return true;
}


bool contact_checker_phone_format(const Contact &_data, FieldErrorMap &_errors)
{
    bool result = generic_checker_phone_format(
            _data.telephone.get_value_or_default(),
            PHONE_PATTERN);
    if (result == false) {
        _errors[field_phone] = INVALID;
    }
    return result;
}



bool contact_checker_phone_cz_sk_format(const Contact &_data, FieldErrorMap &_errors)
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

bool contact_checker_email_format(const Contact &_data, FieldErrorMap &_errors)
{
    const std::string::size_type MAX_MOJEID_EMAIL_LENGTH = 200;
    const std::string contact_email = _data.email.get_value_or_default();

    bool result = true;

    if(!contact_email.empty())
    {
        result = ((Util::get_utf8_char_len(contact_email) <= MAX_MOJEID_EMAIL_LENGTH)
            && DjangoEmailFormat().check(contact_email));
    }

    if (result == false)
    {
        _errors[field_email] = INVALID;
    }

    return result;
}

bool contact_checker_notify_email_format(const Contact &_data, FieldErrorMap &_errors)
{
    FieldErrorMap dummy_errors;
    bool result = contact_checker_email_format(_data, dummy_errors);
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
    if (_c1.organization.get_value_or_default() != _c2.organization.get_value_or_default()) {
        return false;
    }
    /* dic */
    if (_c1.vat.get_value_or_default() != _c2.vat.get_value_or_default()) {
        return false;
    }
    /* address */
    if ((_c1.street1.get_value_or_default() != _c2.street1.get_value_or_default())
            || (_c1.street2.get_value_or_default() != _c2.street2.get_value_or_default())
            || (_c1.street3.get_value_or_default() != _c2.street3.get_value_or_default())
            || (_c1.city.get_value_or_default() != _c2.city.get_value_or_default())
            || (_c1.stateorprovince.get_value_or_default() != _c2.stateorprovince.get_value_or_default())
            || (_c1.country.get_value_or_default() != _c2.country.get_value_or_default())
            || (_c1.postalcode.get_value_or_default() != _c2.postalcode.get_value_or_default())) {
        return false;
    }
    /* identification type */
    if (_c1.ssntype.get_value_or_default() != _c2.ssntype.get_value_or_default()) {
        return false;
    }
    /* identification regardless of type*/
    if (_c1.ssn.get_value_or_default() != _c2.ssn.get_value_or_default()) {

        if (_c1.ssntype.get_value_or_default() == "BIRTHDAY") {
            boost::gregorian::date before = birthdate_from_string_to_date(_c1.ssn.get_value_or_default());
            boost::gregorian::date after = birthdate_from_string_to_date(_c2.ssn.get_value_or_default());
            if (before != after) {
                return false;
            }

        }
        else {
            return false;
        }
    }
    /* telephone and email */
    if ((_c1.telephone.get_value_or_default() != _c2.telephone.get_value_or_default())
            || (_c1.fax.get_value_or_default() != _c2.fax.get_value_or_default())
            || (_c1.email.get_value_or_default() != _c2.email.get_value_or_default())
            || (_c1.notifyemail.get_value_or_default() != _c2.notifyemail.get_value_or_default())) {
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
    if (_c1.organization.get_value_or_default() != _c2.organization.get_value_or_default()) {
        return false;
    }
    /* address */
    if ((_c1.street1.get_value_or_default() != _c2.street1.get_value_or_default())
            || (_c1.street2.get_value_or_default() != _c2.street2.get_value_or_default())
            || (_c1.street3.get_value_or_default() != _c2.street3.get_value_or_default())
            || (_c1.city.get_value_or_default() != _c2.city.get_value_or_default())
            || (_c1.stateorprovince.get_value_or_default() != _c2.stateorprovince.get_value_or_default())
            || (_c1.country.get_value_or_default() != _c2.country.get_value_or_default())
            || (_c1.postalcode.get_value_or_default() != _c2.postalcode.get_value_or_default())) {
        return false;
    }
    /* identification type */
    if (_c1.ssntype.get_value_or_default() != _c2.ssntype.get_value_or_default()) {
        return false;
    }
    /* identification regardless of type*/
    if (_c1.ssn.get_value_or_default() != _c2.ssn.get_value_or_default()) {

        if(_c1.ssntype.get_value_or_default() == "BIRTHDAY") {
            boost::gregorian::date before = birthdate_from_string_to_date(_c1.ssn.get_value_or_default());
            boost::gregorian::date after = birthdate_from_string_to_date(_c2.ssn.get_value_or_default());
            if(before != after) {
                return false;
            }
        }
        else {
            return false;
        }
    }

    return true;
}

AreTheSame check_identified_contact_diff(const Contact &_c1, const Contact &_c2)
{
    /* name */
    if (_c1.name != _c2.name) {
        return false;
    }
    /* mailing address */
    if (_c1.get_mailing_address() != _c2.get_mailing_address()) {
        return false;
    }
    return true;
}


}
}
}


