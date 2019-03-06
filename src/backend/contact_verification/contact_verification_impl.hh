/*
 * Copyright (C) 2012-2019  CZ.NIC, z. s. p. o.
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
#ifndef CONTACT_VERIFICATION_IMPL_HH_6E3C9F031A0B4D018EE2DB4613E5D82B
#define CONTACT_VERIFICATION_IMPL_HH_6E3C9F031A0B4D018EE2DB4613E5D82B

#include "libfred/mailer.hh"
#include "src/util/cfg/handle_registry_args.hh"

#include <memory>
#include <string>
#include <vector>

namespace Fred {
namespace Backend {
namespace ContactVerification {

struct IDENTIFICATION_FAILED
    : public std::runtime_error
{


    IDENTIFICATION_FAILED()
        : std::runtime_error("identification failed")
    {
    }

};

struct IDENTIFICATION_PROCESSED
    : public std::runtime_error
{


    IDENTIFICATION_PROCESSED()
        : std::runtime_error("identification procesed")
    {
    }

};

struct IDENTIFICATION_INVALIDATED
    : public std::runtime_error
{


    IDENTIFICATION_INVALIDATED()
        : std::runtime_error("identification invalidated")
    {
    }

};

struct OBJECT_CHANGED
    : public std::runtime_error
{


    OBJECT_CHANGED()
        : std::runtime_error("object changed")
    {
    }

};

struct OBJECT_NOT_EXISTS
    : public std::runtime_error
{


    OBJECT_NOT_EXISTS()
        : std::runtime_error("object does not exist")
    {
    }

};

struct REGISTRAR_NOT_EXISTS
    : public std::runtime_error
{


    REGISTRAR_NOT_EXISTS()
        : std::runtime_error("registrar does not exist")
    {
    }

};

struct VALIDATION_ERROR
{
    enum Type
    {
        NOT_AVAILABLE,
        INVALID,
        REQUIRED

    };

};

typedef std::map<std::string, VALIDATION_ERROR::Type> FIELD_ERROR_MAP;

struct DATA_VALIDATION_ERROR
    : public std::runtime_error
{


    DATA_VALIDATION_ERROR(const FIELD_ERROR_MAP& _e)
        : std::runtime_error("data validation error"),
          errors(_e)
    {
    }

    ~DATA_VALIDATION_ERROR()
    {
    }

    FIELD_ERROR_MAP errors;

};


class ContactVerificationImpl
{
    const HandleRegistryArgs* registry_conf_;
    const std::string server_name_;

    std::shared_ptr<LibFred::Mailer::Manager> mailer_;

public:
    ContactVerificationImpl(
            const std::string& _server_name,
            std::shared_ptr<LibFred::Mailer::Manager> _mailer);


    virtual ~ContactVerificationImpl();


    const std::string& get_server_name();


    unsigned long long createConditionalIdentification(
            const std::string& contact_handle,
            const std::string& registrar_handle,
            const unsigned long long log_id,
            std::string& request_id);


    unsigned long long processConditionalIdentification(
            const std::string& request_id,
            const std::string& password,
            const unsigned long long log_id);


    unsigned long long processIdentification(
            const std::string& contact_handle,
            const std::string& password,
            const unsigned long long log_id);


    std::string getRegistrarName(const std::string& registrar_handle);


};            // class ContactVerificationImpl

} // namespace Fred::Backend::ContactVerification
} // namespace Fred::Backend
} // namespace Fred

#endif // CONTACT_VERIFICATION_IMPL_H__
