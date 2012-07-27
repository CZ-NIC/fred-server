#ifndef CONTACT_VERIFICATION_CORBA_CONVERTION_H_
#define CONTACT_VERIFICATION_CORBA_CONVERTION_H_

#include "corba/ContactVerification.hh"
#include "corba/common_wrappers.h"

#include "fredlib/contact_verification/contact.h"
#include "fredlib/contact_verification/data_validation.h"

#include <string>
#include <boost/algorithm/string.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>


Registry::ContactVerification::ValidationError corba_wrap_validation_error(
        const Fred::Contact::Verification::ValidationError &_value)
{
    switch (_value) {
        case Fred::Contact::Verification::NOT_AVAILABLE:
            return Registry::ContactVerification::NOT_AVAILABLE;
        case Fred::Contact::Verification::INVALID:
            return Registry::ContactVerification::INVALID;
        case Fred::Contact::Verification::REQUIRED:
            return Registry::ContactVerification::REQUIRED;
        default:
            throw std::runtime_error("unknown validation error type");
    }
}


Registry::ContactVerification::ValidationErrorList_var corba_wrap_validation_error_list(
        const Fred::Contact::Verification::FieldErrorMap &_errors)
{
    Registry::ContactVerification::ValidationErrorList_var cerrors
        = new Registry::ContactVerification::ValidationErrorList;
    cerrors->length(_errors.size());

    Fred::Contact::Verification::FieldErrorMap::const_iterator it = _errors.begin();
    Fred::Contact::Verification::FieldErrorMap::size_type i = 0;
    for (; it != _errors.end(); ++it, ++i) {
        cerrors[i].name = corba_wrap_string(it->first);
        cerrors[i].error = corba_wrap_validation_error(it->second);
    }

    return cerrors;
}


#endif //CONTACT_VERIFICATION_CORBA_CONVERTION_H_

