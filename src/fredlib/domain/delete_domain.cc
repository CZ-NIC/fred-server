/*
 * Copyright (C) 2012  CZ.NIC, z.s.p.o.
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
 *  @file delete_domain.cc
 *  domain delete
 */

#include <string>

#include "fredlib/domain/delete_domain.h"
#include "fredlib/domain/domain_name.h"
#include "fredlib/object/object.h"

#include "fredlib/opcontext.h"
#include "fredlib/db_settings.h"
#include "fredlib/object_states.h"

namespace Fred
{
    DeleteDomain::DeleteDomain(const std::string& fqdn)
    : fqdn_(fqdn)
    {}
    void DeleteDomain::exec(OperationContext& ctx)
    {
        try
        {
            //remove optional root dot from fqdn
            std::string no_root_dot_fqdn = Fred::Domain::rem_trailing_dot(fqdn_);

            //get domain_id and lock object_registry row for update
            unsigned long long domain_id =0;
            {
                Database::Result domain_id_res = ctx.get_conn().exec_params(
                    "SELECT oreg.id FROM domain d "
                    " JOIN object_registry oreg ON d.id = oreg.id "
                    " JOIN enum_object_type eot ON oreg.type = eot.id AND eot.name = 'domain' "
                    " WHERE oreg.name = LOWER($1::text) AND oreg.erdate IS NULL "
                    " FOR UPDATE OF oreg"
                    , Database::query_param_list(no_root_dot_fqdn));

                if (domain_id_res.size() == 0)
                {
                    BOOST_THROW_EXCEPTION(Exception().set_unknown_domain_fqdn(fqdn_));
                }
                if (domain_id_res.size() != 1)
                {
                    BOOST_THROW_EXCEPTION(InternalError("failed to get domain"));
                }

                domain_id = domain_id_res[0][0];
            }

            ctx.get_conn().exec_params("DELETE FROM domain_contact_map WHERE domainid = $1::integer"
                , Database::query_param_list(domain_id));//delete 0..n rows, nothing to be checked in result

            Database::Result delete_enumval_res = ctx.get_conn().exec_params(
                "DELETE FROM enumval WHERE domainid = $1::integer RETURNING domainid"
                , Database::query_param_list(domain_id));//delete 0..1 row
            if(delete_enumval_res.size() > 1)
            {
                BOOST_THROW_EXCEPTION(Fred::InternalError("delete enumval failed"));
            }

            Database::Result delete_contact_res = ctx.get_conn().exec_params(
                "DELETE FROM domain WHERE id = $1::integer RETURNING id"
                    , Database::query_param_list(domain_id));
            if (delete_contact_res.size() != 1)
            {
                BOOST_THROW_EXCEPTION(Fred::InternalError("delete domain failed"));
            }

            Fred::DeleteObject(no_root_dot_fqdn,"domain").exec(ctx);

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }

    }//DeleteDomain::exec

    std::ostream& operator<<(std::ostream& os, const DeleteDomain& dd)
    {
        return os << "#DeleteDomain fqdn: " << dd.fqdn_
                ;
    }
    std::string DeleteDomain::to_string()
    {
        std::stringstream ss;
        ss << *this;
        return ss.str();
    }


}//namespace Fred

