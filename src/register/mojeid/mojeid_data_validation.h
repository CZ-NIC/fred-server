#ifndef MOJEID_DATA_VALIDATION_H_
#define MOJEID_DATA_VALIDATION_H_

#include "contact.h"

#include <boost/regex.hpp>
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


void validate_contact_data(const ::MojeID::Contact &_data);

bool check_validated_contact_diff(const ::MojeID::Contact &_c1, const ::MojeID::Contact &_c2);


}


#endif /*MOJEID_DATA_VALIDATION_H_*/

