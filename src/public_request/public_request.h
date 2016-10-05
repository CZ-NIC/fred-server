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

#include "src/fredlib/public_request/public_request_type_iface.h"
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

class EnumPublicRequestType : public Fred::PublicRequestTypeIface
{
public:
    const Fred::PublicRequestTypeIface& iface() const { return *this; }
    std::string get_public_request_type() const = 0;

protected:
    PublicRequestTypes get_public_request_types_to_cancel_on_create() const = 0;

    PublicRequestTypes get_public_request_types_to_cancel_on_update(
        Fred::PublicRequest::Status::Enum _old_status,
        Fred::PublicRequest::Status::Enum _new_status) const
    {
        throw std::runtime_error("method must not be called");
    }
};

class AuthinfoAuto : public EnumPublicRequestType
{
public:
    std::string get_public_request_type() const { return "authinfo_auto_pif"; }

private:
    PublicRequestTypes get_public_request_types_to_cancel_on_create() const
    {
        PublicRequestTypes res;
        res.insert(IfacePtr(new AuthinfoAuto));
        return res;
    }
};

class AuthinfoEmail : public EnumPublicRequestType
{
public:
    std::string get_public_request_type() const { return "authinfo_email_pif"; }

private:
    PublicRequestTypes get_public_request_types_to_cancel_on_create() const
    {
        PublicRequestTypes res;
        res.insert(IfacePtr(new AuthinfoEmail));
        return res;
    }
};

class AuthinfoPost : public EnumPublicRequestType
{
public:
    std::string get_public_request_type() const { return "authinfo_post_pif"; }

private:
    PublicRequestTypes get_public_request_types_to_cancel_on_create() const
    {
        PublicRequestTypes res;
        res.insert(IfacePtr(new AuthinfoPost));
        return res;
    }
};

class BlockChangesEmail : public EnumPublicRequestType
{
public:
    std::string get_public_request_type() const { return "block_changes_email_pif"; }

private:
    PublicRequestTypes get_public_request_types_to_cancel_on_create() const
    {
        PublicRequestTypes res;
        res.insert(IfacePtr(new BlockChangesEmail));
        return res;
    }
};

class BlockChangesPost : public EnumPublicRequestType
{
public:
    std::string get_public_request_type() const { return "block_changes_post_pif"; }

private:
    PublicRequestTypes get_public_request_types_to_cancel_on_create() const
    {
        PublicRequestTypes res;
        res.insert(IfacePtr(new BlockChangesPost));
        return res;
    }
};

class BlockTransferEmail : public EnumPublicRequestType
{
public:
    std::string get_public_request_type() const { return "block_transfer_email_pif"; }

private:
    PublicRequestTypes get_public_request_types_to_cancel_on_create() const
    {
        PublicRequestTypes res;
        res.insert(IfacePtr(new BlockTransferEmail));
        return res;
    }
};

class BlockTransferPost : public EnumPublicRequestType
{
public:
    std::string get_public_request_type() const { return "block_transfer_post_pif"; }

private:
    PublicRequestTypes get_public_request_types_to_cancel_on_create() const
    {
        PublicRequestTypes res;
        res.insert(IfacePtr(new BlockTransferPost));
        return res;
    }
};

class UnblockChangesEmail : public EnumPublicRequestType
{
public:
    std::string get_public_request_type() const { return "unblock_changes_email_pif"; }

private:
    PublicRequestTypes get_public_request_types_to_cancel_on_create() const
    {
        PublicRequestTypes res;
        res.insert(IfacePtr(new UnblockChangesEmail));
        return res;
    }
};

class UnblockChangesPost : public EnumPublicRequestType
{
public:
    std::string get_public_request_type() const { return "unblock_changes_post_pif"; }

private:
    PublicRequestTypes get_public_request_types_to_cancel_on_create() const
    {
        PublicRequestTypes res;
        res.insert(IfacePtr(new UnblockChangesPost));
        return res;
    }
};

class UnblockTransferEmail : public EnumPublicRequestType
{
public:
    std::string get_public_request_type() const { return "unblock_transfer_email_pif"; }

private:
    PublicRequestTypes get_public_request_types_to_cancel_on_create() const
    {
        PublicRequestTypes res;
        res.insert(IfacePtr(new UnblockTransferEmail));
        return res;
    }
};

class UnblockTransferPost : public EnumPublicRequestType
{
public:
    std::string get_public_request_type() const { return "unblock_transfer_post_pif"; }

private:
    PublicRequestTypes get_public_request_types_to_cancel_on_create() const
    {
        PublicRequestTypes res;
        res.insert(IfacePtr(new UnblockTransferPost));
        return res;
    }
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

} // namespace Registry
} // namespace PublicRequestImpl

#endif // PUBLIC_REQUEST_H_75462367
