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

#include "src/fredlib/keyset/delete_keyset.h"
#include "src/fredlib/object/object.h"
#include "src/fredlib/object/object_impl.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/db_settings.h"
#include "src/fredlib/object_states.h"

namespace Fred
{
    DeleteKeyset::DeleteKeyset(const std::string& handle)
    : handle_(handle)
    {}

    void DeleteKeyset::exec(OperationContext& ctx)
    {
        try
        {
            //lock object_registry row for update and get keyset_id
            unsigned long long keyset_id = get_object_id_by_handle_and_type_with_lock(
                    ctx,handle_,"keyset",static_cast<Exception*>(0),
                    &Exception::set_unknown_keyset_handle);

            //check if object is linked
            Database::Result linked_result = ctx.get_conn().exec_params(
                "SELECT * FROM object_state os "
                " JOIN enum_object_states eos ON eos.id = os.state_id "
                " WHERE os.object_id = $1::integer AND eos.name = $2::text "
                " AND valid_to IS NULL",
                Database::query_param_list
                (keyset_id)
                (Fred::ObjectState::LINKED));

            if (linked_result.size() > 0)
            {
                BOOST_THROW_EXCEPTION(Exception().set_object_linked_to_keyset_handle(handle_));
            }

            ctx.get_conn().exec_params("DELETE FROM keyset_contact_map WHERE keysetid = $1::integer"
                , Database::query_param_list(keyset_id));

            ctx.get_conn().exec_params("DELETE FROM dnskey WHERE keysetid = $1::integer"
                , Database::query_param_list(keyset_id));

            ctx.get_conn().exec_params("DELETE FROM dsrecord WHERE keysetid = $1::integer"
                , Database::query_param_list(keyset_id));

            Database::Result delete_keyset_res = ctx.get_conn().exec_params(
                "DELETE FROM keyset WHERE id = $1::integer RETURNING id"
                    , Database::query_param_list(keyset_id));
            if (delete_keyset_res.size() != 1)
            {
                BOOST_THROW_EXCEPTION(Fred::InternalError("delete keyset failed"));
            }

            Fred::DeleteObject(handle_,"keyset").exec(ctx);
        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }

    }//DeleteKeyset::exec

    std::string DeleteKeyset::to_string() const
    {
        return Util::format_operation_state("DeleteKeyset",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("handle",handle_)));
    }


}//namespace Fred

