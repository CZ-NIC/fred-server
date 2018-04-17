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

#ifndef PUBLIC_REQUEST_HH_609D44EB1F5A4BC98E8A88B662EE3761
#define PUBLIC_REQUEST_HH_609D44EB1F5A4BC98E8A88B662EE3761

#include "src/backend/buffer.hh"
#include "src/bin/corba/mailer_manager.hh"
#include "src/libfred/documents.hh"
#include "src/libfred/mailer.hh"
#include "src/libfred/object/object_type.hh"
#include "src/util/optional_value.hh"

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <memory>

#include <stdexcept>
#include <string>
#include <vector>

namespace Fred {
namespace Backend {
namespace PublicRequest {

class PublicRequestImpl
{
public:
    PublicRequestImpl(const std::string& _server_name);
    ~PublicRequestImpl();

    const std::string& get_server_name() const;

    struct ConfirmedBy
    {
        enum Enum
        {
            email,
            letter
        };
    };

    struct LockRequestType
    {
        enum Enum
        {
            block_transfer,
            block_transfer_and_update,
            unblock_transfer,
            unblock_transfer_and_update,
        };
    };

    struct Language
    {
        enum Enum
        {
            en,
            cs
        };
    };

    struct ObjectType
    {
        enum Enum
        {
            contact,
            nsset,
            domain,
            keyset,
        };
    };

    struct ObjectAlreadyBlocked : std::exception
    {
        virtual const char* what() const noexcept
        {
            return "object is already blocked";
        }
    };

    struct ObjectTransferProhibited : std::exception
    {
        virtual const char* what() const noexcept
        {
            return "object transfer is prohibited";
        }
    };

    struct ObjectNotBlocked : std::exception
    {
        virtual const char* what() const noexcept
        {
            return "object is not blocked";
        }
    };

    struct HasDifferentBlock : std::exception
    {
        virtual const char* what() const noexcept
        {
            return "a different unblock request has to be issued";
        }
    };

    struct ObjectNotFound : std::exception
    {
        virtual const char* what() const noexcept
        {
            return "registry object with specified ID does not exist";
        }
    };

    struct InvalidPublicRequestType : std::exception
    {
        virtual const char* what() const noexcept
        {
            return "public request is not of post type";
        }
    };

    struct NoContactEmail : std::exception
    {
        virtual const char* what() const noexcept
        {
            return "no contact email associated with this object";
        }
    };

    struct InvalidContactEmail : std::exception
    {
        virtual const char* what() const noexcept
        {
            return "invalid contact email associated with this object";
        }
    };

    struct OperationProhibited : std::exception
    {
        virtual const char* what() const noexcept
        {
            return "operation is prohibited";
        }
    };

    unsigned long long create_authinfo_request_registry_email(
            ObjectType::Enum object_type,
            const std::string& object_handle,
            const Optional<unsigned long long>& log_request_id,
            std::shared_ptr<LibFred::Mailer::Manager> manager) const;

    unsigned long long create_authinfo_request_non_registry_email(
            ObjectType::Enum object_type,
            const std::string& object_handle,
            const Optional<unsigned long long>& log_request_id,
            ConfirmedBy::Enum confirmation_method,
            const std::string& specified_email) const;

    unsigned long long create_block_unblock_request(
            ObjectType::Enum object_type,
            const std::string& object_handle,
            const Optional<unsigned long long>& log_request_id,
            ConfirmedBy::Enum confirmation_method,
            LockRequestType::Enum lock_request_type) const;

    unsigned long long create_personal_info_request_registry_email(
            const std::string& contact_handle,
            const Optional<unsigned long long>& log_request_id,
            std::shared_ptr<LibFred::Mailer::Manager> manager) const;

    unsigned long long create_personal_info_request_non_registry_email(
            const std::string& contact_handle,
            const Optional<unsigned long long>& log_request_id,
            ConfirmedBy::Enum confirmation_method,
            const std::string& specified_email) const;

    Fred::Backend::Buffer create_public_request_pdf(
            unsigned long long public_request_id,
            Language::Enum lang,
            std::shared_ptr<LibFred::Document::Manager> manager) const;

    static std::shared_ptr<LibFred::Mailer::Manager> get_default_mailer_manager();
    static std::shared_ptr<LibFred::Document::Manager> get_default_document_manager();

private:
    const std::string server_name_;
};

} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred

#endif
