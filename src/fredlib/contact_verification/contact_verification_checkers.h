#ifndef CONTACT_VERIFICATION_CHECKERS_H__
#define CONTACT_VERIFICATION_CHECKERS_H__

#include "contact_validator.h"

#include <boost/regex.hpp>
#include <string>


namespace Fred {
namespace Contact {
namespace Verification {

const boost::regex USERNAME_PATTERN("^[a-z0-9](-?[a-z0-9])*$");
const boost::regex PHONE_PATTERN("^\\+[0-9]{1,3}\\.[0-9]{1,14}$");
const boost::regex PHONE_CZ_SK_PATTERN("^\\+42(0\\.(60[1-9]|7\\d\\d|91\\d)|1\\.9(0[1-9]|[145]\\d))[0-9]{6}$");
const boost::regex EMAIL_PATTERN("^[-!#$%&'*+/=?^_`{}|~0-9A-Za-z]+(\\.[-!#$%&'*+/=?^_`{}|~0-9A-Za-z]+)*"
                                 "@(?:[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?\\.)+[A-Za-z]{2,6}\\.?$");
const boost::regex POSTALCODE_CZ_PATTERN("^[0-9]{3} ?[0-9]{2}$");
const std::string EMAIL_PHONE_PROTECTION_PERIOD = "1 month";

const std::string field_username     = "contact.username";
const std::string field_phone        = "phone.number";
const std::string field_fax          = "phone.fax";
const std::string field_first_name   = "contact.first_name";
const std::string field_last_name    = "contact.last_name";
const std::string field_street1      = "address.street1";
const std::string field_country      = "address.country";
const std::string field_city         = "address.city";
const std::string field_postal_code  = "address.postal_code";
const std::string field_email        = "email.email";
const std::string field_notify_email = "email.notify_email";
const std::string field_birth_date   = "contact.birth_date";
const std::string field_auth_info    = "contact.auth_info";
const std::string field_status       = "contact.status";





bool contact_checker_name(const Contact &_data, FieldErrorMap &_errors);
bool contact_checker_username(const Contact &_data, FieldErrorMap &_errors);
bool contact_checker_phone_format(const Contact &_data, FieldErrorMap &_errors);
bool contact_checker_fax_format(const Contact &_data, FieldErrorMap &_errors);
bool contact_checker_auth_info(const Contact &_data, FieldErrorMap &_errors);
bool contact_checker_phone_required(const Contact &_data, FieldErrorMap &_errors);
bool contact_checker_phone_unique(const Contact &_data, FieldErrorMap &_errors);
bool contact_checker_email_format(const Contact &_data, FieldErrorMap &_errors);
bool contact_checker_email_required(const Contact &_data, FieldErrorMap &_errors);
bool contact_checker_email_unique(const Contact &_data, FieldErrorMap &_errors);
bool contact_checker_notify_email_format(const Contact &_data, FieldErrorMap &_errors);
bool contact_checker_address_required(const Contact &_data, FieldErrorMap &_errors);
bool contact_checker_address_country(const Contact &_data, FieldErrorMap &_errors);
bool contact_checker_address_postalcode_format_cz(const Contact &_data, FieldErrorMap &_errors);
bool contact_checker_birthday(const Contact &_data, FieldErrorMap &_errors);



}
}
}

#endif /*CONTACT_VERIFICATION_CHECKERS_H__*/

