/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 *  @file
 *  header of public request implementation
 */

#ifndef PUBLIC_REQUEST_H_75462367
#define PUBLIC_REQUEST_H_75462367

#include "src/fredlib/object/object_type.h"

#include "util/optional_value.h"
#include "src/fredlib/mailer.h"

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>
#include <stdexcept>

namespace Registry
{
namespace PublicRequestImpl
{

enum ConfirmationMethod
{
    EMAIL_WITH_QUALIFIED_CERTIFICATE,
    LETTER_WITH_AUTHENTICATED_SIGNATURE
};

enum LockRequestType
{
    BLOCK_TRANSFER,
    BLOCK_TRANSFER_AND_UPDATE,
    UNBLOCK_TRANSFER,
    UNBLOCK_TRANSFER_AND_UPDATE
};

class ObjectAlreadyBlocked : std::exception
{
public:
    virtual const char* what() const throw()
    {
        return "object is already blocked";
    }
};

class ObjectNotBlocked : std::exception
{
public:
    virtual const char* what() const throw()
    {
        return "object is not blocked";
    }
};

class HasDifferentBlock : std::exception
{
public:
    virtual const char* what() const throw()
    {
        return "a different unblock request has to be issued";
    }
};

class PublicRequest
{
public:
    unsigned long long create_authinfo_request_registry_email(
        Fred::Object_Type::Enum object_type,
        const std::string& object_handle,
        const std::string& reason,
        const Optional<unsigned long long>& log_request_id,
        boost::shared_ptr<Fred::Mailer::Manager> manager);

    unsigned long long create_authinfo_request_non_registry_email(
        Fred::Object_Type::Enum object_type,
        const std::string& object_handle,
        const std::string& reason,
        const Optional<unsigned long long>& log_request_id,
        ConfirmationMethod confirmation_method,
        const std::string& specified_email);

    unsigned long long create_block_unblock_request(
        Fred::Object_Type::Enum object_type,
        const std::string& object_handle,
        const Optional<unsigned long long>& log_request_id,
        ConfirmationMethod confirmation_method,
        LockRequestType lock_request_type);
};

} // namespace PublicRequestImpl
} // namespace Registry

#endif // PUBLIC_REQUEST_H_75462367
