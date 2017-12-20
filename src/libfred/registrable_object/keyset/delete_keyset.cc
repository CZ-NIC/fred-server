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
 *  keyset delete
 */

#include <string>
#include <boost/assign.hpp>
#include <boost/lexical_cast.hpp>

#include "src/libfred/registrable_object/keyset/delete_keyset.hh"
#include "src/libfred/object/object.hh"
#include "src/libfred/object/object_impl.hh"
#include "src/libfred/opcontext.hh"
#include "src/libfred/db_settings.hh"
#include "src/libfred/object_states.hh"
#include "src/libfred/object_state/object_has_state.hh"
#include "src/libfred/object_state/object_state_name.hh"

namespace LibFred
{
    void delete_keyset_impl(OperationContext& ctx, unsigned long long id) {
        ctx.get_conn().exec_params(
            "DELETE FROM keyset_contact_map "
            "   WHERE keysetid = $1::integer",
            Database::query_param_list(id));

        ctx.get_conn().exec_params(
            "DELETE FROM dnskey "
            "   WHERE keysetid = $1::integer",
            Database::query_param_list(id));

        ctx.get_conn().exec_params(
            "DELETE FROM dsrecord "
            "   WHERE keysetid = $1::integer",
            Database::query_param_list(id));

        Database::Result delete_keyset_res = ctx.get_conn().exec_params(
            "DELETE FROM keyset "
            "   WHERE id = $1::integer RETURNING id",
            Database::query_param_list(id));

        if (delete_keyset_res.size() != 1) {
            BOOST_THROW_EXCEPTION(LibFred::InternalError("delete keyset failed"));
        }
    }

    DeleteKeysetByHandle::DeleteKeysetByHandle(const std::string& handle)
        : handle_(handle)
    { }

    void DeleteKeysetByHandle::exec(OperationContext& ctx)
    {
        try
        {
            unsigned long long keyset_id = get_object_id_by_handle_and_type_with_lock(
                ctx,
                true,
                handle_,
                "keyset",
                static_cast<Exception*>(NULL),
                &Exception::set_unknown_keyset_handle);

            if (ObjectHasState(keyset_id, ObjectState::LINKED).exec(ctx)) {
                BOOST_THROW_EXCEPTION(Exception().set_object_linked_to_keyset_handle(handle_));
            }

            delete_keyset_impl(ctx, keyset_id);

            LibFred::DeleteObjectByHandle(handle_, "keyset").exec(ctx);

        } catch(ExceptionStack& ex) {
            ex.add_exception_stack_info(to_string());
            throw;
        }

    }

    std::string DeleteKeysetByHandle::to_string() const {
        return Util::format_operation_state(
            "DeleteKeysetByHandle",
            boost::assign::list_of
                (std::make_pair("handle", handle_ ))
        );
    }

    DeleteKeysetById::DeleteKeysetById(unsigned long long _id)
        : id_(_id)
    { }

    void DeleteKeysetById::exec(OperationContext& ctx) {
        try {
            get_object_id_by_object_id_with_lock(
                ctx,
                id_,
                static_cast<Exception*>(NULL),
                &Exception::set_unknown_keyset_id);

            if (ObjectHasState(id_, ObjectState::LINKED).exec(ctx)) {
                BOOST_THROW_EXCEPTION(Exception().set_object_linked_to_keyset_id(id_));
            }

            delete_keyset_impl(ctx, id_);

            LibFred::DeleteObjectById(id_).exec(ctx);

        } catch(ExceptionStack& ex) {

            ex.add_exception_stack_info(to_string());
            throw;
        }

    }

    std::string DeleteKeysetById::to_string() const {

        return Util::format_operation_state(
            "DeleteKeysetById",
            boost::assign::list_of
                (std::make_pair("id", boost::lexical_cast<std::string>(id_) ))
        );
    }


} // namespace LibFred

