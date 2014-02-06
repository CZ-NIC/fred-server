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
 *  @file
 *  nsset delete
 */

#include <string>
#include <boost/assign.hpp>
#include <boost/lexical_cast.hpp>

#include "src/fredlib/nsset/delete_nsset.h"
#include "src/fredlib/object/object.h"

#include "src/fredlib/opcontext.h"
#include "src/fredlib/db_settings.h"
#include "src/fredlib/object_states.h"

namespace Fred
{
    static void delete_nsset_impl(OperationContext& ctx, unsigned long long id) {
        ctx.get_conn().exec_params(
            "DELETE FROM nsset_contact_map "
            "   WHERE nssetid = $1::integer",
            Database::query_param_list(id));

        ctx.get_conn().exec_params(
            "DELETE FROM host_ipaddr_map "
            "   WHERE nssetid = $1::integer",
            Database::query_param_list(id));

        ctx.get_conn().exec_params(
            "DELETE FROM host "
            "   WHERE nssetid = $1::integer",
            Database::query_param_list(id));

        Database::Result delete_nsset_res = ctx.get_conn().exec_params(
            "DELETE FROM nsset "
            "   WHERE id = $1::integer RETURNING id",
            Database::query_param_list(id));
        if (delete_nsset_res.size() != 1) {
            BOOST_THROW_EXCEPTION(Fred::InternalError("delete nsset failed"));
        }
    }

    DeleteNssetByHandle::DeleteNssetByHandle(const std::string& handle)
    : handle_(handle)
    {}

    void DeleteNssetByHandle::exec(OperationContext& ctx)
    {
        try
        {
            unsigned long long nsset_id = lock_object_by_handle_and_type(
                ctx,
                handle_,
                "nsset",
                static_cast<Exception*>(NULL),
                &Exception::set_unknown_nsset_handle);

            if (is_object_linked(ctx, nsset_id)) {
                BOOST_THROW_EXCEPTION(Exception().set_object_linked_to_nsset_handle(handle_));
            }

            delete_nsset_impl(ctx, nsset_id);

            Fred::DeleteObjectByHandle(handle_,"nsset").exec(ctx);

        } catch(ExceptionStack& ex) {
            ex.add_exception_stack_info(to_string());
            throw;
        }
    }

    std::string DeleteNssetByHandle::to_string() const
    {
        return Util::format_operation_state(
            "DeleteNssetByHandle",
            boost::assign::list_of
              (std::make_pair("handle", handle_ ))
        );
    }

    DeleteNssetById::DeleteNssetById(unsigned long long id)
    : id_(id)
    {}

    void DeleteNssetById::exec(OperationContext& ctx)
    {
        try
        {
            unsigned long long nsset_id = lock_object_by_id(
                ctx,
                id_,
                static_cast<Exception*>(NULL),
                &Exception::set_unknown_nsset_id);

            if (is_object_linked(ctx, id_)) {
                BOOST_THROW_EXCEPTION(Exception().set_object_linked_to_nsset_id(id_));
            }

            delete_nsset_impl(ctx, id_);

            Fred::DeleteObjectById(id_).exec(ctx);

        } catch(ExceptionStack& ex) {
            ex.add_exception_stack_info(to_string());
            throw;
        }
    }

    std::string DeleteNssetById::to_string() const
    {
        return Util::format_operation_state(
            "DeleteNssetById",
            boost::assign::list_of
              (std::make_pair("id", boost::lexical_cast<std::string>(id_) ))
        );
    }
}//namespace Fred

