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
#include <boost/assign.hpp>
#include <boost/lexical_cast.hpp>

#include "src/libfred/registrable_object/domain/delete_domain.hh"
#include "src/libfred/registrable_object/domain/domain_name.hh"
#include "src/libfred/zone/zone.hh"
#include "src/libfred/object/object.hh"
#include "src/libfred/object/object_impl.hh"
#include "src/libfred/registrar/registrar_impl.hh"
#include "src/libfred/opcontext.hh"
#include "src/libfred/db_settings.hh"
#include "src/libfred/object_states.hh"

namespace LibFred
{
    DeleteDomainByFqdn::DeleteDomainByFqdn(const std::string& _fqdn)
    : fqdn_(_fqdn)
    {}

    static void delete_domain_impl(OperationContext& ctx, unsigned long long id) {
        ctx.get_conn().exec_params(
            "DELETE FROM domain_contact_map "
            "   WHERE domainid = $1::integer",
            Database::query_param_list(id));    // delete 0..n rows, nothing to be checked in result

        Database::Result delete_enumval_res = ctx.get_conn().exec_params(
            "DELETE FROM enumval"
            "   WHERE domainid = $1::integer RETURNING domainid",
            Database::query_param_list(id));    // delete 0..1 row

        if(delete_enumval_res.size() > 1) {
            BOOST_THROW_EXCEPTION(LibFred::InternalError("delete enumval failed"));
        }

        Database::Result delete_contact_res = ctx.get_conn().exec_params(
            "DELETE FROM domain "
            "   WHERE id = $1::integer RETURNING id",
            Database::query_param_list(id));

        if (delete_contact_res.size() != 1) {
            BOOST_THROW_EXCEPTION(LibFred::InternalError("delete domain failed"));
        }
    }

    void DeleteDomainByFqdn::exec(OperationContext& _ctx)
    {
        try
        {
            //remove optional root dot from fqdn
            std::string no_root_dot_fqdn = LibFred::Zone::rem_trailing_dot(fqdn_);

            //get domain_id and lock object_registry row for update
            unsigned long long domain_id = get_object_id_by_handle_and_type_with_lock(
                _ctx,
                true,
                no_root_dot_fqdn,
                "domain",
                static_cast<Exception*>(NULL),
                &Exception::set_unknown_domain_fqdn
            );

            delete_domain_impl(_ctx, domain_id);

            LibFred::DeleteObjectByHandle(no_root_dot_fqdn,"domain").exec(_ctx);

        } catch(ExceptionStack& ex) {

            ex.add_exception_stack_info(to_string());
            throw;
        }

    }

    std::string DeleteDomainByFqdn::to_string() const
    {
        return Util::format_operation_state(
            "DeleteDomainByFqdn",
            boost::assign::list_of
                (std::make_pair("fqdn", fqdn_ ))
        );
    }

    DeleteDomainById::DeleteDomainById(unsigned long long _id)
        : id_(_id)
    { }

    void DeleteDomainById::exec(OperationContext& _ctx) {
        try
        {
            get_object_id_by_object_id_with_lock(
                _ctx,
                id_,
                static_cast<Exception*>(NULL),
                &Exception::set_unknown_domain_id
            );

            delete_domain_impl(_ctx, id_);

            LibFred::DeleteObjectById(id_).exec(_ctx);

        } catch(ExceptionStack& ex) {
            ex.add_exception_stack_info(to_string());
            throw;
        }
    }

    std::string DeleteDomainById::to_string() const {
        return Util::format_operation_state(
            "DeleteDomainById",
            boost::assign::list_of
                (std::make_pair("id", boost::lexical_cast<std::string>(id_) ))
        );
    }

} // namespace LibFred

