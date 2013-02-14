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
 *  @file delete_contact.cc
 *  contact delete
 */

#include <string>

#include "fredlib/contact/delete_contact.h"

#include "fredlib/opcontext.h"
#include "fredlib/db_settings.h"
#include "fredlib/object_states.h"

namespace Fred
{
    DeleteContact::DeleteContact(const std::string& handle)
    : handle_(handle)
    {}

    void DeleteContact::exec(OperationContext& ctx)
    {
        //lock object_registry row for update
        {
            Database::Result lock_res = ctx.get_conn().exec_params(
                "SELECT oreg.id FROM enum_object_type eot"
                " JOIN object_registry oreg ON oreg.type = eot.id "
                " AND UPPER(oreg.name) = UPPER($1::text) "
                " WHERE eot.name = 'contact' FOR UPDATE OF oreg"
                , Database::query_param_list(handle_));

            if (lock_res.size() != 1)
            {
                std::string errmsg("unable to lock || not found:handle: ");
                errmsg += boost::replace_all_copy(handle_,"|", "[pipe]");//quote pipes
                errmsg += " |";
                throw DCEX(errmsg.c_str());
            }
        }

        //get contact_id
        unsigned long long contact_id =0;
        {
            Database::Result contact_id_res = ctx.get_conn().exec_params(
                "SELECT oreg.id FROM contact c "
                " JOIN object_registry oreg ON c.id = oreg.id "
                " WHERE UPPER(oreg.name) = UPPER($1::text)"
                , Database::query_param_list(handle_));

            if (contact_id_res.size() != 1)
            {
                std::string errmsg("|| not found:handle: ");
                errmsg += boost::replace_all_copy(handle_,"|", "[pipe]");//quote pipes
                errmsg += " |";
                throw DCEX(errmsg.c_str());
            }

            contact_id = contact_id_res[0][0];
        }

        //check if object is linked
        if (Fred::object_has_state(contact_id, Fred::ObjectState::LINKED) == true)
        {
            std::string errmsg("|| is linked:handle: ");
            errmsg += boost::replace_all_copy(handle_,"|", "[pipe]");//quote pipes
            errmsg += " |";
            throw DCEX(errmsg.c_str());
        }

        ctx.get_conn().exec_params(
            "UPDATE object_registry SET erdate = now() "
            " WHERE id = $1::integer"
            , Database::query_param_list(contact_id));

        ctx.get_conn().exec_params(
            "DELETE FROM contact "
                " WHERE id = $1::integer"
                , Database::query_param_list(contact_id));

        ctx.get_conn().exec_params(
            "DELETE FROM object "
                " WHERE id = $1::integer"
                , Database::query_param_list(contact_id));
    }//DeleteContact::exec

}//namespace Fred

