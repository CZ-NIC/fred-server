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

#include "src/libfred/opcontext.hh"
#include "src/libfred/registrable_object/domain/create_domain.hh"
#include "src/libfred/registrable_object/domain/delete_domain.hh"

#include <boost/date_time/gregorian/gregorian.hpp>

namespace Admin {
namespace Domain {

void
create_expired_domain(LibFred::Logger::LoggerClient& _logger_client,
        const std::string& _fqdn,
        const std::string& _registrant,
        const std::string& _cltrid,
        bool _delete_existing,
        const std::string& _registrar)
{

    LibFred::OperationContextCreator ctx;
    unsigned long long deleted_domain_id = 0;
    unsigned long long new_domain_id = 0;

    unsigned long long req_id = _logger_client.createRequest("", "Admin", "",
            boost::assign::list_of
                (LibFred::Logger::RequestProperty("command", "create_expired_domain", false))
                (LibFred::Logger::RequestProperty("handle", _fqdn, true))
                (LibFred::Logger::RequestProperty("registrant", _registrant, true))
                (LibFred::Logger::RequestProperty("cltrid", _cltrid, true)),
            LibFred::Logger::ObjectReferences(),
            "CreateExpiredDomain", 0);
    if (req_id == 0) {
        throw std::runtime_error("unable to log create expired domain request");
    }

    Database::Result registrant_res = ctx.get_conn().exec_params(
            "SELECT id FROM object_registry WHERE type = get_object_type_id('contact') AND name = $1::text AND erdate is NULL",
            Database::query_param_list(_registrant));
    if (registrant_res.size() != 1)
    {
        boost::format msg("Contact with handle %1% not found in database.");
        msg % _registrant;
        logger_create_expired_domain_close(_logger_client, "Fail", req_id, deleted_domain_id, new_domain_id);
        throw std::runtime_error(msg.str());
    }

    Database::Result fqdn_res = ctx.get_conn().exec_params(
            "SELECT id FROM object_registry WHERE type = get_object_type_id('domain') AND name = $1::text AND erdate is NULL",
            Database::query_param_list(_fqdn));

    if (fqdn_res.size() == 1)
    {
        if (_delete_existing)
        {
            LibFred::DeleteDomainByFqdn(_fqdn).exec(ctx);
            deleted_domain_id = static_cast<unsigned long long>(fqdn_res[0][0]);
        }
        else
        {
            boost::format msg("Domain with fqdn %1% already exists in database.");
            msg % _fqdn;
            logger_create_expired_domain_close(_logger_client, "Fail", req_id, deleted_domain_id, new_domain_id);
            throw std::runtime_error(msg.str());
        }
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
    catch (std::exception&)
    {
        logger_create_expired_domain_close(_logger_client, "Fail", req_id, deleted_domain_id, new_domain_id);
        throw std::runtime_error("domain create failed");
    }
    ctx.commit_transaction();
    logger_create_expired_domain_close(_logger_client, "Success", req_id, deleted_domain_id, result.create_object_result.object_id);
}

void
logger_create_expired_domain_close(
        LibFred::Logger::LoggerClient& _logger_client,
        const std::string& _result,
        const unsigned long long _req_id,
        const unsigned long long _deleted_domain_id,
        const unsigned long long _new_domain_id)
{
    LibFred::Logger::RequestProperties properties;
    LibFred::Logger::ObjectReferences references;
    properties.push_back(LibFred::Logger::RequestProperty("opTRID", Util::make_svtrid(_req_id), false));
    if (_deleted_domain_id > 0)
    {
        references.push_back(LibFred::Logger::ObjectReference("domain", _deleted_domain_id));
    }
    if (_new_domain_id > 0)
    {
        references.push_back(LibFred::Logger::ObjectReference("domain", _new_domain_id));
    }
    _logger_client.closeRequest(_req_id, "Admin", "",
            properties,
            references,
            _result, 0);
}

} // namespace Domain;
} // namespace Admin;
