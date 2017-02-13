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
#include "src/fredlib/documents.h"
#include "src/fredlib/mailer.h"
#include "util/optional_value.h"
#include "src/corba/mailer_manager.h"

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/shared_ptr.hpp>

#include <stdexcept>
#include <string>
#include <vector>

namespace Registry {

class PublicRequestImpl
{
public:
    struct ConfirmationMethod
    {
        enum Enum
        {
            email_with_qualified_certificate,
            letter_with_authenticated_signature
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

    struct Buffer
    {
        explicit Buffer(const std::string& s);
        const std::string value;
    };

    struct ObjectAlreadyBlocked : std::exception
    {
        virtual const char* what() const throw()
        {
            return "object is already blocked";
        }
    };

    struct ObjectNotBlocked : std::exception
    {
        virtual const char* what() const throw()
        {
            return "object is not blocked";
        }
    };

    struct HasDifferentBlock : std::exception
    {
        virtual const char* what() const throw()
        {
            return "a different unblock request has to be issued";
        }
    };

    struct ObjectNotFound : std::exception
    {
        virtual const char* what() const throw()
        {
            return "registry object with specified ID does not exist";
        }
    };

    struct InvalidPublicRequestType : std::exception
    {
        virtual const char* what() const throw()
        {
            return "public request is not of post type";
        }
    };

    struct NoContactEmail : std::exception
    {
        virtual const char* what() const throw()
        {
            return "no contact email associated with this object";
        }
    };

    struct InvalidContactEmail : std::exception
    {
        virtual const char* what() const throw()
        {
            return "invalid contact email associated with this object";
        }
    };

    unsigned long long create_authinfo_request_registry_email(
        ObjectType::Enum object_type,
        const std::string& object_handle,
        const Optional<unsigned long long>& log_request_id,
        boost::shared_ptr<Fred::Mailer::Manager> manager);

    unsigned long long create_authinfo_request_non_registry_email(
        ObjectType::Enum object_type,
        const std::string& object_handle,
        const Optional<unsigned long long>& log_request_id,
        ConfirmationMethod::Enum confirmation_method,
        const std::string& specified_email);

    unsigned long long create_block_unblock_request(
        ObjectType::Enum object_type,
        const std::string& object_handle,
        const Optional<unsigned long long>& log_request_id,
        ConfirmationMethod::Enum confirmation_method,
        LockRequestType::Enum lock_request_type);

    Buffer create_public_request_pdf(
        unsigned long long public_request_id,
        Language::Enum lang,
        boost::shared_ptr<Fred::Document::Manager> manager);

    static boost::shared_ptr<Fred::Mailer::Manager> get_default_mailer_manager();
    static boost::shared_ptr<Fred::Document::Manager> get_default_document_manager();
};

} // namespace Registry

#endif // PUBLIC_REQUEST_H_75462367
