#ifndef CONTACT_VERIFICATION_CORBA_CONVERTION_H_
#define CONTACT_VERIFICATION_CORBA_CONVERTION_H_

#include "src/bin/corba/ContactVerification.hh"
#include "src/bin/corba/common_wrappers.hh"

#include "src/libfred/contact_verification/contact.hh"
#include "src/libfred/contact_verification/contact_verification_validators.hh"

#include <string>
#include <boost/algorithm/string.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>


Registry::ContactVerification::ValidationError corba_wrap_validation_error(
        const Registry::Contact::Verification::VALIDATION_ERROR::Type &_value)
{
    switch (_value) {
        case Registry::Contact::Verification::VALIDATION_ERROR::NOT_AVAILABLE:
            return Registry::ContactVerification::NOT_AVAILABLE;
        case Registry::Contact::Verification::VALIDATION_ERROR::INVALID:
            return Registry::ContactVerification::INVALID;
        case Registry::Contact::Verification::VALIDATION_ERROR::REQUIRED:
            return Registry::ContactVerification::REQUIRED;
        default:
            throw std::runtime_error("unknown validation error type");
    }
}


Registry::ContactVerification::ValidationErrorList_var corba_wrap_validation_error_list(
        const Registry::Contact::Verification::FIELD_ERROR_MAP &_errors)
{
    Registry::ContactVerification::ValidationErrorList_var cerrors
        = new Registry::ContactVerification::ValidationErrorList;
    cerrors->length(_errors.size());

    Registry::Contact::Verification::FIELD_ERROR_MAP::const_iterator it = _errors.begin();
    Registry::Contact::Verification::FIELD_ERROR_MAP::size_type i = 0;
    for (; it != _errors.end(); ++it, ++i) {
        cerrors[i].name = corba_wrap_string(it->first);
        cerrors[i].error = corba_wrap_validation_error(it->second);
    }

    return cerrors;
}


#endif //CONTACT_VERIFICATION_CORBA_CONVERTION_H_

