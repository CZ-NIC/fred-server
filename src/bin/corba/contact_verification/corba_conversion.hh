/*
 * Copyright (C) 2012-2020  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef CORBA_CONVERSION_HH_B83A1597635A431BB4B96DE06A4FCCF7
#define CORBA_CONVERSION_HH_B83A1597635A431BB4B96DE06A4FCCF7

#include "src/backend/contact_verification/contact_verification_impl.hh"
#include "corba/ContactVerification.hh"
#include "src/bin/corba/common_wrappers.hh"

#include "src/deprecated/libfred/contact_verification/contact.hh"
#include "src/deprecated/libfred/contact_verification/contact_verification_validators.hh"

#include <boost/algorithm/string.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include <string>

Registry::ContactVerification::ValidationError corba_wrap_validation_error(
        const Fred::Backend::ContactVerification::VALIDATION_ERROR::Type &_value)
{
    switch (_value) {
        case Fred::Backend::ContactVerification::VALIDATION_ERROR::NOT_AVAILABLE:
            return Registry::ContactVerification::NOT_AVAILABLE;
        case Fred::Backend::ContactVerification::VALIDATION_ERROR::INVALID:
            return Registry::ContactVerification::INVALID;
        case Fred::Backend::ContactVerification::VALIDATION_ERROR::REQUIRED:
            return Registry::ContactVerification::REQUIRED;
        default:
            throw std::runtime_error("unknown validation error type");
    }
}


Registry::ContactVerification::ValidationErrorList_var corba_wrap_validation_error_list(
        const Fred::Backend::ContactVerification::FIELD_ERROR_MAP &_errors)
{
    Registry::ContactVerification::ValidationErrorList_var cerrors
        = new Registry::ContactVerification::ValidationErrorList;
    cerrors->length(_errors.size());

    Fred::Backend::ContactVerification::FIELD_ERROR_MAP::const_iterator it = _errors.begin();
    Fred::Backend::ContactVerification::FIELD_ERROR_MAP::size_type i = 0;
    for (; it != _errors.end(); ++it, ++i) {
        cerrors[i].name = corba_wrap_string(it->first);
        cerrors[i].error = corba_wrap_validation_error(it->second);
    }

    return cerrors;
}


#endif
