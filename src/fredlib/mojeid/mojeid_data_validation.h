#ifndef MOJEID_DATA_VALIDATION_H_
#define MOJEID_DATA_VALIDATION_H_

#include "contact.h"

#include <boost/regex.hpp>
#include <boost/function.hpp>
#include <map>


namespace MojeID {

const boost::regex USERNAME_PATTERN("^[a-z0-9](-?[a-z0-9])*$");
const boost::regex PHONE_PATTERN("^\\+[0-9]{1,3}\\.[0-9]{1,14}$");
const boost::regex PHONE_CZ_SK_PATTERN("^\\+42(0\\.(60[1-9]|7[2-9]|91)|1\\.9(0[1-9]|[145]))[0-9]+$");
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


enum ValidationError
{
    NOT_AVAILABLE,
    INVALID,
    REQUIRED
};

typedef std::map<std::string, ValidationError> FieldErrorMap;

struct DataValidationError : public std::runtime_error
{
    DataValidationError(const FieldErrorMap &_e) :
        std::runtime_error("data validation error"),
        errors(_e)
    {
    }
    ~DataValidationError() throw () {}
    FieldErrorMap errors;
};


class ContactValidator
{
public:
    typedef boost::function<bool (const ::MojeID::Contact &_data, FieldErrorMap &_errors)> Checker;

    void add_checker(Checker _func)
    {
        checkers_.push_back(_func);
    }

    void check(const ::MojeID::Contact &_data) const
    {
        FieldErrorMap errors;

        std::vector<Checker>::const_iterator check = checkers_.begin();
        for (; check != checkers_.end(); ++check) {
            (*check)(_data, errors);
        }

        LOGGER(PACKAGE).debug(boost::format("data validation ran %1% check(s)"
                    " -- found %2% error(s)") % checkers_.size() % errors.size());
        if (!errors.empty()) {
            throw DataValidationError(errors);
        }
    }


private:
    std::vector<Checker> checkers_;
};



bool contact_checker_name(const ::MojeID::Contact &_data, FieldErrorMap &_errors);
bool contact_checker_username(const ::MojeID::Contact &_data, FieldErrorMap &_errors);
bool contact_checker_phone_format(const ::MojeID::Contact &_data, FieldErrorMap &_errors);
bool contact_checker_fax_format(const ::MojeID::Contact &_data, FieldErrorMap &_errors);
bool contact_checker_auth_info(const ::MojeID::Contact &_data, FieldErrorMap &_errors);
bool contact_checker_phone_required(const ::MojeID::Contact &_data, FieldErrorMap &_errors);
bool contact_checker_phone_unique(const ::MojeID::Contact &_data, FieldErrorMap &_errors);
bool contact_checker_email_format(const ::MojeID::Contact &_data, FieldErrorMap &_errors);
bool contact_checker_email_required(const ::MojeID::Contact &_data, FieldErrorMap &_errors);
bool contact_checker_email_unique(const ::MojeID::Contact &_data, FieldErrorMap &_errors);
bool contact_checker_notify_email_format(const ::MojeID::Contact &_data, FieldErrorMap &_errors);
bool contact_checker_address_required(const ::MojeID::Contact &_data, FieldErrorMap &_errors);
bool contact_checker_address_country(const ::MojeID::Contact &_data, FieldErrorMap &_errors);
bool contact_checker_address_postalcode_format_cz(const ::MojeID::Contact &_data, FieldErrorMap &_errors);
bool contact_checker_birthday(const ::MojeID::Contact &_data, FieldErrorMap &_errors);

ContactValidator create_conditional_identification_validator();
ContactValidator create_identification_validator();
ContactValidator create_finish_identification_validator();
ContactValidator create_contact_update_validator();

bool check_conditionally_identified_contact_diff(const ::MojeID::Contact &_c1, const ::MojeID::Contact &_c2);
bool check_validated_contact_diff(const ::MojeID::Contact &_c1, const ::MojeID::Contact &_c2);


}


#endif /*MOJEID_DATA_VALIDATION_H_*/

