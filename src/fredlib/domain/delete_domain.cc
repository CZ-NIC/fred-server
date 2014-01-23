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

#include "src/fredlib/domain/delete_domain.h"
#include "src/fredlib/domain/domain_name.h"
#include "src/fredlib/zone/zone.h"
#include "src/fredlib/object/object.h"
#include "src/fredlib/registrar/registrar_impl.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/db_settings.h"
#include "src/fredlib/object_states.h"

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
            std::string no_root_dot_fqdn = Fred::Zone::rem_trailing_dot(fqdn_);

            //get domain_id and lock object_registry row for update
            unsigned long long domain_id = get_object_id_by_handle_and_type_with_lock(
                    ctx,no_root_dot_fqdn,"domain",static_cast<Exception*>(0),
                    &Exception::set_unknown_domain_fqdn);

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

    std::string DeleteDomain::to_string() const
    {
        return Util::format_operation_state("DeleteDomain",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("fqdn",fqdn_))
        );
    }


}//namespace Fred

