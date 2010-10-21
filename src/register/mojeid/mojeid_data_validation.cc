#include "register/db_settings.h"
#include "mojeid_data_validation.h"

#include <boost/algorithm/string.hpp>

namespace MojeID {


const boost::regex USERNAME_PATTERN("^[a-z0-9](-?[a-z0-9])*$");
const boost::regex PHONE_PATTERN("^\\+420\\.(60([1-8]|9([134]|2[1-5]))|7(0[0-9]|10|[237]))\\d+");
const boost::regex EMAIL_PATTERN("^[-!#$%&'*+/=?^_`{}|~0-9A-Za-z]+(\\.[-!#$%&'*+/=?^_`{}|~0-9A-Za-z]+)*"
                                 "@(?:[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?\\.)+[A-Za-z]{2,6}\\.?$");


void validate_contact_data(const ::MojeID::Contact &_data)
{
    FieldErrorMap errors;
    /* contact handle has to be in domain-token
     * pattern ^[a-z0-9](-?[a-z0-9])*$ and max length 30
     */
    if (_data.handle.length() == 0) {
        errors["username"] = REQUIRED;
    }
    else if (_data.handle.length() > 30) {
        errors["username"] = INVALID;
    }
    else if (!boost::regex_search(
                boost::to_lower_copy(_data.handle),
                USERNAME_PATTERN)) {
        errors["username"] = INVALID;
    }

    /* main phone has to be in format (czech mobile operators):
     * ^\+420\.(60([1-8]|9([134]|2[1-5]))|7(0[0-9]|10|[237]))\d+
     * and 14 characters long
     */
    if (static_cast<std::string>(_data.telephone).length() == 0) {
        errors["phone"] = REQUIRED;
    }
    else if (static_cast<std::string>(_data.telephone).length() > 14) {
        errors["phone"] = INVALID;
    }
    else if (!boost::regex_search(
                static_cast<std::string>(_data.telephone),
                PHONE_PATTERN)) {
        errors["phone"] = INVALID;
    }
    else {
        /* main phone has to be unique among all mojeid contacts */
        Database::Connection conn = Database::Manager::acquire();
        Database::Result unique_phone = conn.exec_params(
                "SELECT c.id FROM contact c"
                " JOIN object_state os ON os.object_id = c.id"
                " JOIN enum_object_states eos ON eos.id = os.state_id"
                " WHERE eos.name =ANY ($1::text[]) AND"
                " trim(both ' ' from c.telephone) = trim(both ' ' from $2::text)",
                Database::query_param_list
                    ("{conditionallyIdentifiedContact, identifiedContact, validatedContact}")
                    (static_cast<std::string>(_data.telephone)));
        if (unique_phone.size() > 0) {
            errors["phone"] = NOT_AVAILABLE;
        }
    }

    /* main address has to be from Czech Republic */
    if (static_cast<std::string>(_data.country) != "CZ") {
        errors["country"] = INVALID;
    }

    /* email should match following pattern */
    if (static_cast<std::string>(_data.email).length() == 0) {
        errors["email"] = REQUIRED;
    }
    else if (!boost::regex_search(
                static_cast<std::string>(_data.email),
                EMAIL_PATTERN)) {
        errors["email"] = INVALID;
    }
    else {
        /* main email has to be unique among all mojeid contacts */
        Database::Connection conn = Database::Manager::acquire();
        Database::Result unique_email = conn.exec_params(
                "SELECT c.id FROM contact c"
                " JOIN object_state os ON os.object_id = c.id"
                " JOIN enum_object_states eos ON eos.id = os.state_id"
                " WHERE eos.name =ANY ($1::text[]) AND"
                " trim(both ' ' from c.email) = trim(both ' ' from $2::text)",
                Database::query_param_list
                    ("{conditionallyIdentifiedContact, identifiedContact, validatedContact}")
                    (static_cast<std::string>(_data.email)));
        if (unique_email.size() > 0) {
            errors["email"] = NOT_AVAILABLE;
        }
    }

    if (!errors.empty()) {
        throw DataValidationError(errors);
    }
}


}

