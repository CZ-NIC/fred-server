/*
 * Copyright (C) 2018-2019  CZ.NIC, z. s. p. o.
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
#ifndef CREATE_EXPIRED_DOMAIN_HH_04C50993A8B54BE18AD42AFC55B6985C
#define CREATE_EXPIRED_DOMAIN_HH_04C50993A8B54BE18AD42AFC55B6985C

#include "src/deprecated/libfred/logger_client.hh"
#include "libfred/object/object_type.hh"
#include "libfred/object/get_id_of_registered.hh"

namespace Admin {
namespace Domain {

struct SystemRegistrarNotExists : std::exception
{
    virtual const char* what() const noexcept
    {
        return "System registrar does not exist in database - check configuration.";
    }
};

struct NotSystemRegistrar : std::exception
{
    virtual const char* what() const noexcept
    {
        return "System registrar expected - check configuration.";
    }
};

struct InvalidFQDNSyntax : std::exception
{
    virtual const char* what() const noexcept
    {
        return "FQDN has invalid syntax.";
    }
};

struct ZoneNotExists : std::exception
{
    virtual const char* what() const noexcept
    {
        return "Domain zone is not in registry.";
    }
};

struct DomainExists : std::exception
{
    virtual const char* what() const noexcept
    {
        return "Domain already exists in database.";
    }
};

struct DomainNotExists : std::exception
{
    virtual const char* what() const noexcept
    {
        return "Domain does not exist in database.";
    }
};

struct RegistrantNotExists : std::exception
{
    virtual const char* what() const noexcept
    {
        return "Registrant does not exist in database.";
    }
};

void
create_expired_domain(
        LibFred::Logger::LoggerClient& _logger_client,
        const std::string& _fqdn,
        const std::string& _registrant,
        const std::string& _cltrid,
        bool _delete_existing,
        const std::string& _registrar);

void
logger_create_expired_domain_close(
        LibFred::Logger::LoggerClient& _logger_client,
        const std::string& _result,
        unsigned long long _req_id,
        const boost::optional<unsigned long long> _deleted_domain_id,
        const boost::optional<unsigned long long> _new_domain_id);

template <LibFred::Object_Type::Enum object_type>
boost::optional<unsigned long long>
get_id_by_handle(LibFred::OperationContextCreator& _ctx, const std::string& _handle)
{
    boost::optional<unsigned long long> id;
    try
    {
        id = LibFred::get_id_of_registered<object_type>(_ctx, _handle);
    }
    catch (const LibFred::UnknownObject&)
    { }  // Object may not exist

    return id;
}

template boost::optional<unsigned long long> get_id_by_handle<LibFred::Object_Type::domain>(LibFred::OperationContextCreator& _ctx, const std::string& _handle);
template boost::optional<unsigned long long> get_id_by_handle<LibFred::Object_Type::contact>(LibFred::OperationContextCreator& _ctx, const std::string& _handle);

} // namespace Domain;
} // namespace Admin;

#endif
