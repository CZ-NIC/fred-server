/*
 * Copyright (C) 2018  CZ.NIC, z.s.p.o.
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

#include "create_expired_domain.hh"

#include "libfred/object_state/perform_object_state_request.hh"
#include "libfred/opcontext.hh"
#include "libfred/registrable_object/domain/domain.hh"
#include "libfred/registrable_object/domain/create_domain.hh"
#include "libfred/registrable_object/domain/delete_domain.hh"
#include "libfred/registrar/info_registrar.hh"
#include "libfred/zone/zone.hh"

#include <boost/optional.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

namespace Admin {
namespace Domain {

void
create_expired_domain(
        LibFred::Logger::LoggerClient& _logger_client,
        const std::string& _fqdn,
        const std::string& _registrant,
        const std::string& _cltrid,
        bool _delete_existing,
        const std::string& _registrar)
{
    LibFred::OperationContextCreator ctx;
    unsigned long long req_id;
    try
    {
        req_id = _logger_client.createRequest("", "Admin", "",
                boost::assign::list_of
                    (LibFred::Logger::RequestProperty("command", "create_expired_domain", false))
                    (LibFred::Logger::RequestProperty("handle", _fqdn, true))
                    (LibFred::Logger::RequestProperty("registrant", _registrant, true))
                    (LibFred::Logger::RequestProperty("cltrid", _cltrid, true)),
                LibFred::Logger::ObjectReferences(),
                "CreateExpiredDomain", 0);
        if (req_id == 0) {
            throw std::runtime_error("Logger - create request: unable to log create expired domain request");
        }
    }
    catch (const std::exception& e)
    {
        boost::format msg("Logger - create request: %1%");
        msg % e.what();
        throw std::runtime_error(msg.str());
    }

    bool is_system_registrar;
    try {
        LibFred::InfoRegistrarData session_registrar =
                LibFred::InfoRegistrarByHandle(_registrar).exec(ctx).info_registrar_data;

        is_system_registrar = session_registrar.system.get_value();

        if (!is_system_registrar)
        {
            logger_create_expired_domain_close(_logger_client, "Fail", req_id, boost::none, boost::none);
            throw NotSystemRegistrar();
        }
    }
    catch (const LibFred::InfoRegistrarByHandle::Exception& e)
    {
        logger_create_expired_domain_close(_logger_client, "Fail", req_id, boost::none, boost::none);
        if (e.is_set_unknown_registrar_handle())
        {
            throw SystemRegistrarNotExists();
        }
        throw;
    }

    if (!get_id_by_handle<LibFred::Object_Type::contact>(ctx, _registrant))
    {
        logger_create_expired_domain_close(_logger_client, "Fail", req_id, boost::none, boost::none);
        throw RegistrantNotExists();
    }

    const LibFred::Domain::DomainRegistrability::Enum domain_registrability =
            LibFred::Domain::get_domain_registrability_by_domain_fqdn(ctx, _fqdn, is_system_registrar);

    if (domain_registrability == LibFred::Domain::DomainRegistrability::zone_not_in_registry)
    {
        logger_create_expired_domain_close(_logger_client, "Fail", req_id, boost::none, boost::none);
        throw ZoneNotExists();
    }

    if (LibFred::Domain::get_domain_fqdn_syntax_validity(ctx, _fqdn, is_system_registrar) != LibFred::Domain::DomainFqdnSyntaxValidity::valid)
    {
        logger_create_expired_domain_close(_logger_client, "Fail", req_id, boost::none, boost::none);
        throw InvalidFQDNSyntax();
    }

    boost::optional<unsigned long long> existing_domain_id;
    if (domain_registrability == LibFred::Domain::DomainRegistrability::registered)
    {
        existing_domain_id = get_id_by_handle<LibFred::Object_Type::domain>(ctx, LibFred::Zone::rem_trailing_dot(_fqdn));
    }

    if (existing_domain_id && !_delete_existing)
    {
        logger_create_expired_domain_close(_logger_client, "Fail", req_id, existing_domain_id, boost::none);
        throw DomainExists();
    }
    if (!existing_domain_id && _delete_existing)
    {
        logger_create_expired_domain_close(_logger_client, "Fail", req_id, boost::none, boost::none);
        throw DomainNotExists();
    }

    const bool do_delete = existing_domain_id && _delete_existing;

    if (do_delete)
    {
        LibFred::DeleteDomainByFqdn(_fqdn).exec(ctx);
    }

    boost::gregorian::date current_date(boost::gregorian::day_clock::local_day());
    LibFred::CreateDomain::Result result;
    try
    {
        result = LibFred::CreateDomain(
            _fqdn,
            _registrar,
            _registrant).set_expiration_date(current_date).exec(ctx);
    }
    catch (const LibFred::CreateDomain::Exception& e)
    {
        logger_create_expired_domain_close(_logger_client, "Fail", req_id, existing_domain_id, boost::none);
        boost::format msg("Create domain failed: %1%");
        if (e.is_set_invalid_fqdn_syntax())
        {
            msg % "Invalid FQDN syntax";
            throw std::runtime_error(msg.str());
        }
        if (e.is_set_unknown_zone_fqdn())
        {
            msg % "Unknown FQDN zone";
            throw std::runtime_error(msg.str());
        }
        throw;
    }
    catch (const std::exception& e)
    {
        logger_create_expired_domain_close(_logger_client, "Fail", req_id, existing_domain_id, boost::none);
        boost::format msg("Create domain failed: %1%");
        msg % e.what();
        throw std::runtime_error(msg.str());
    }
    LibFred::PerformObjectStateRequest(result.create_object_result.object_id).exec(ctx);
    ctx.commit_transaction();
    logger_create_expired_domain_close(_logger_client, "Success", req_id, existing_domain_id, result.create_object_result.object_id);
}

void
logger_create_expired_domain_close(
        LibFred::Logger::LoggerClient& _logger_client,
        const std::string& _result,
        unsigned long long _req_id,
        const boost::optional<unsigned long long> _deleted_domain_id,
        const boost::optional<unsigned long long> _new_domain_id)
{
    try
    {
        LibFred::Logger::RequestProperties properties;
        LibFred::Logger::ObjectReferences references;
        properties.push_back(LibFred::Logger::RequestProperty("opTRID", Util::make_svtrid(_req_id), false));
        if (_deleted_domain_id)
        {
            references.push_back(LibFred::Logger::ObjectReference("domain", *_deleted_domain_id));
        }
        if (_new_domain_id)
        {
            references.push_back(LibFred::Logger::ObjectReference("domain", *_new_domain_id));
        }
        _logger_client.closeRequest(_req_id, "Admin", "",
                properties,
                references,
                _result, 0);
    }
    catch (std::exception& e)
    {
        boost::format msg("Logger - close request: %1%");
        msg % e.what();
        throw std::runtime_error(msg.str());
    }
}

}//namespace Admin::Domain
}//namespace Admin
