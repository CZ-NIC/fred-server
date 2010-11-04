#ifndef MOJEID_DATA_VALIDATION_H_
#define MOJEID_DATA_VALIDATION_H_

#include "contact.h"

#include <boost/regex.hpp>
#include <boost/function.hpp>
#include <map>


namespace MojeID {


const std::string field_username    = "contact.username";
const std::string field_phone       = "auth_sms.phone_number";
const std::string field_first_name  = "contact.first_name";
const std::string field_last_name   = "contact.last_name";
const std::string field_street1     = "address.street1";
const std::string field_country     = "address.country";
const std::string field_city        = "address.city";
const std::string field_postal_code = "address.postal_code";
const std::string field_email       = "email.email";
const std::string field_birth_date  = "contact.birth_date";


enum ValidationError
{
    NOT_AVAILABLE,
    INVALID,
    REQUIRED
};

typedef std::map<std::string, ValidationError> FieldErrorMap;

struct DataValidationError : private std::exception
{
    DataValidationError(const FieldErrorMap &_e) :
        errors(_e)
    {
    }

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
bool contact_checker_phone_required(const ::MojeID::Contact &_data, FieldErrorMap &_errors);
bool contact_checker_phone_unique(const ::MojeID::Contact &_data, FieldErrorMap &_errors);
bool contact_checker_email_format(const ::MojeID::Contact &_data, FieldErrorMap &_errors);
bool contact_checker_email_required(const ::MojeID::Contact &_data, FieldErrorMap &_errors);
bool contact_checker_email_unique(const ::MojeID::Contact &_data, FieldErrorMap &_errors);
bool contact_checker_address_required(const ::MojeID::Contact &_data, FieldErrorMap &_errors);
bool contact_checker_address_country_cz(const ::MojeID::Contact &_data, FieldErrorMap &_errors);
bool contact_checker_birthday(const ::MojeID::Contact &_data, FieldErrorMap &_errors);

ContactValidator create_conditional_identification_validator();
ContactValidator create_identification_validator();
ContactValidator create_contact_update_validator();

bool check_validated_contact_diff(const ::MojeID::Contact &_c1, const ::MojeID::Contact &_c2);


}


#endif /*MOJEID_DATA_VALIDATION_H_*/

