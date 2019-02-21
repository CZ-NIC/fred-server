/*
 * Copyright (C) 2019  CZ.NIC, z.s.p.o.
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

#include "libfred/db_settings.hh"
#include "libfred/object/object_state.hh"
#include "libfred/object/object_type.hh"
#include "libfred/object_state/object_has_state.hh"
#include "libfred/registrable_object/domain/info_domain.hh"
#include "libfred/registrable_object/domain/info_domain_data.hh"
#include "test/backend/admin_block/util.hh"
#include "util/db/query_param.hh"

#include <exception>
#include <sstream>
#include <string>

namespace Test {

std::size_t count_blocked_objects(const Fred::Backend::AdministrativeBlocking::IdlDomainIdList& _domain_list,
      const ::LibFred::StatusList& _status_list,
      const ::LibFred::ObjectType& _object_type)
{
    if (_domain_list.size() == 0)
    {
        return 0;
    }

    std::string id_column;
    if (_object_type == ::LibFred::Object_Type::domain)
    {
        id_column = "id";
    }
    else if (_object_type == ::LibFred::Object_Type::contact)
    {
        id_column = "registrant";
    }
    else
    {
        throw std::runtime_error("Unsupported object type");
    }

    Database::QueryParams params;

    std::ostringstream domains_type;
    std::for_each(_domain_list.begin(), _domain_list.end(),
            [&params, &domains_type](const auto& _s)->void{
                params.push_back(_s);
                domains_type << (domains_type.str().empty()? "":", ") << "$" << params.size() << "::bigint";
            });

    std::ostringstream states_type;
    params.push_back(Conversion::Enums::to_db_handle(LibFred::Object_State::server_blocked));
    states_type << "$" << params.size() << "::text";
    std::for_each(_status_list.begin(), _status_list.end(),
            [&params, &states_type](const auto& _s)->void{
                params.push_back(_s);
                states_type << ", $" << params.size() << "::text";
            });

    std::ostringstream query;
    query << "SELECT d." << id_column << " FROM domain d "
          << "JOIN object_registry obr ON obr.id = d." << id_column << " "
          << "JOIN object_state_now osn ON osn.object_id = obr.id "
          << "WHERE d.id IN (" << domains_type.str() << ") "
          << "AND osn.states @> ( "
          << "SELECT ARRAY( "
          << "SELECT eos.id FROM enum_object_states eos "
          << "WHERE obr.type = ANY(eos.types) "
          << "AND eos.name IN (" << states_type.str() << "))) "
          << "GROUP BY d." << id_column;

    LibFred::OperationContextCreator ctx;
    const Database::Result db_result = ctx.get_conn().exec_params(
                query.str(),
                params);
    return db_result.size();
}

std::size_t count_blocked_domains(const Fred::Backend::AdministrativeBlocking::IdlDomainIdList& _domain_list,
        const ::LibFred::StatusList& _status_list)
{
    return count_blocked_objects(_domain_list, _status_list, ::LibFred::Object_Type::domain);
}

std::size_t count_blocked_domain_owners(const Fred::Backend::AdministrativeBlocking::IdlDomainIdList& _domain_list,
        const ::LibFred::StatusList& _status_list)
{
    return count_blocked_objects(_domain_list, _status_list, ::LibFred::Object_Type::contact);
}

std::size_t count_blocked_origin_contacts(
        const std::set<std::pair<LibFred::ObjectId, std::string> >& _contact_list)
{
    if (_contact_list.size() == 0)
    {
        return 0;
    }

    Database::QueryParams params;
    params.push_back(Conversion::Enums::to_db_handle(LibFred::Object_State::server_blocked));

    std::ostringstream contacts_type;
    std::for_each(_contact_list.begin(), _contact_list.end(),
            [&params, &contacts_type](const auto& _s)->void{
                params.push_back(_s.first);
                contacts_type << (contacts_type.str().empty()? "":", ") << "$" << params.size() << "::bigint";
            });

    std::ostringstream query;
    query << "SELECT obr.id FROM object_registry obr "
          << "JOIN object_state_now osn ON osn.object_id = obr.id "
          << "WHERE osn.states @> ( "
          << "SELECT ARRAY( "
          << "SELECT eos.id FROM enum_object_states eos "
          << "WHERE obr.type = ANY(eos.types) "
          << "AND eos.name = $1::text)) "
          << "AND obr.id IN (" << contacts_type.str() << ") "
          << "GROUP BY obr.id";

    LibFred::OperationContextCreator ctx;
    const Database::Result db_result = ctx.get_conn().exec_params(
                query.str(),
                params);
    return db_result.size();
}

std::size_t count_domains_with_admin_contacts(
        const Fred::Backend::AdministrativeBlocking::IdlDomainIdList& _domain_list)
{
    LibFred::OperationContextCreator ctx;
    unsigned int result = 0;
    for (const auto& domain : _domain_list)
    {
        std::size_t count_of_admin_c = LibFred::InfoDomainById(domain).exec(ctx).info_domain_data.admin_contacts.size();
        if (count_of_admin_c > 0)
        {
            result++;
        }
    }
    return result;
}

std::size_t count_domains_with_user_blocking(
        const Fred::Backend::AdministrativeBlocking::IdlDomainIdList& _domain_list)
{
    LibFred::OperationContextCreator ctx;
    unsigned int result = 0;
    for (const auto& domain : _domain_list)
    {
        bool user_blocked_domain =
                !LibFred::ObjectHasState(domain, LibFred::Object_State::server_blocked).exec(ctx) &&
                (LibFred::ObjectHasState(domain, LibFred::Object_State::server_transfer_prohibited).exec(ctx) ||
                    LibFred::ObjectHasState(domain, LibFred::Object_State::server_update_prohibited).exec(ctx));
        if (user_blocked_domain)
        {
            result++;
        }
    }
    return result;
}

} // namespace Test
