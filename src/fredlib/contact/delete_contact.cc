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
 *  contact delete
 */

#include <string>
#include <boost/assign.hpp>
#include <boost/lexical_cast.hpp>

#include "src/fredlib/contact/delete_contact.h"
#include "src/fredlib/object/object.h"
#include "src/fredlib/object/object_impl.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/db_settings.h"
#include "src/fredlib/object_state/object_has_state.h"
#include "src/fredlib/object_state/object_state_name.h"

namespace Fred
{
    static void delete_contact_impl(OperationContext& _ctx, unsigned long long _id) {
        const Database::query_param_list params(_id);
        _ctx.get_conn().exec_params(
            "DELETE FROM contact_address WHERE contactid=$1::BIGINT", params);
        Database::Result delete_contact_res = _ctx.get_conn().exec_params(
            "DELETE FROM contact WHERE id=$1::BIGINT RETURNING id", params);

        if (delete_contact_res.size() != 1) {
            BOOST_THROW_EXCEPTION(Fred::InternalError("delete contact failed"));
        }
    }

    DeleteContactByHandle::DeleteContactByHandle(const std::string& handle)
    : handle_(handle)
    {}

    void DeleteContactByHandle::exec(OperationContext& _ctx)
    {
        try
        {
            unsigned long long contact_id = get_object_id_by_handle_and_type_with_lock(
                _ctx,
                handle_,
                "contact",
                static_cast<Exception*>(NULL),
                &Exception::set_unknown_contact_handle);

            if (ObjectHasState(contact_id, ObjectState::LINKED).exec(_ctx)) {
                BOOST_THROW_EXCEPTION(Exception().set_object_linked_to_contact_handle(handle_));
            }

            delete_contact_impl(_ctx, contact_id);

            Fred::DeleteObjectByHandle(handle_,"contact").exec(_ctx);

        } catch(ExceptionStack& ex) {
            ex.add_exception_stack_info(to_string());
            throw;
        }

    }

    std::string DeleteContactByHandle::to_string() const
    {
        return Util::format_operation_state(
            "DeleteContactByHandle",
            boost::assign::list_of
                (std::make_pair("handle", handle_ ))
        );
    }


    DeleteContactById::DeleteContactById(unsigned long long _id)
        : id_(_id)
    { }

    void DeleteContactById::exec(OperationContext& _ctx)
    {
        try
        {
            get_object_id_by_object_id_with_lock(
                _ctx,
                id_,
                static_cast<Exception*>(NULL),
                &Exception::set_unknown_contact_id
            );

            if (ObjectHasState(id_, ObjectState::LINKED).exec(_ctx)) {
                BOOST_THROW_EXCEPTION(Exception().set_object_linked_to_contact_id(id_));
            }

            delete_contact_impl(_ctx, id_);

            Fred::DeleteObjectById(id_).exec(_ctx);

        } catch(ExceptionStack& ex) {
            ex.add_exception_stack_info(to_string());
            throw;
        }

    }

    std::string DeleteContactById::to_string() const
    {
        return Util::format_operation_state(
            "DeleteContactById",
            boost::assign::list_of
                (std::make_pair("id", boost::lexical_cast<std::string>(id_) ))
        );
    }
}//namespace Fred

