#ifndef MOJEID_DATA_VALIDATION_H_
#define MOJEID_DATA_VALIDATION_H_

#include "contact.h"

#include <boost/regex.hpp>
#include <map>


namespace MojeID {


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


}


#endif /*MOJEID_DATA_VALIDATION_H_*/

