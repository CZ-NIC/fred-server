/*
 * Copyright (C) 2014  CZ.NIC, z.s.p.o.
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
 *  object impl
 */

#ifndef OBJECT_IMPL_H_54ff2fba491943db8a52ff835e32741d
#define OBJECT_IMPL_H_54ff2fba491943db8a52ff835e32741d

#include <string>

#include "src/fredlib/opexception.h"
#include "src/fredlib/opcontext.h"

namespace Fred
{
    /**
    * Check existence, get database id and lock object type for read.
    * @param ctx contains reference to database and logging interface
    * @param obj_type is object type to look for in enum_object_type table, throw InternalError if not found
    * @return database id of the object type
    * , or throw InternalError or some other exception in case of failure.
    */

    unsigned long long get_object_type_id(OperationContext& ctx, const std::string& obj_type);

    /**
    * Gets object id by handle or fqdn and object type name and locks for update.
    * @param EXCEPTION is type of exception used for reporting when object is not found, deducible from type of @ref ex_ptr parameter
    * @param EXCEPTION_OBJECT_HANDLE_SETTER is EXCEPTION member function pointer used to report unknown object handle
    * @param ctx contains reference to database and logging interface
    * @param object_handle is handle or fqdn to look for
    * @param object_type is name from enum_object_type, if not found throws InternallError
    * @param ex_ptr is  pointer to given exception instance to be set (don't throw except for object_type), if ex_ptr is 0, new exception instance is created, set and thrown
    * @param ex_handle_setter is EXCEPTION member function pointer used to report unknown object handle
    * @return database id of the object
    * , or throw @ref EXCEPTION if object handle was not found and external exception instance was not provided
    * , or set unknown object handle or fqdn into given external exception instance and return 0
    * , or throw InternalError or some other exception in case of failure.
    */
    template <class EXCEPTION, typename EXCEPTION_OBJECT_HANDLE_SETTER>
    unsigned long long get_object_id_by_handle_and_type_with_lock(OperationContext& ctx
            , const std::string& object_handle, const std::string& object_type
            , EXCEPTION* ex_ptr, EXCEPTION_OBJECT_HANDLE_SETTER ex_handle_setter)
    {
        get_object_type_id(ctx, object_type);

        Database::Result object_id_res = ctx.get_conn().exec_params(
        "SELECT oreg.id FROM object_registry oreg "
        " JOIN enum_object_type eot ON eot.id = oreg.type AND eot.name = $2::text "
        " WHERE oreg.name = CASE WHEN $2::text = 'domain'::text THEN LOWER($1::text) "
        " ELSE UPPER($1::text) END AND oreg.erdate IS NULL "
        " FOR UPDATE OF oreg"
        , Database::query_param_list(object_handle)(object_type));

        if(object_id_res.size() == 0)
        {
            if(ex_ptr == 0)//make new exception instance, set data and throw
            {
                BOOST_THROW_EXCEPTION((EXCEPTION().*ex_handle_setter)(object_handle));
            }
            else//set unknown handle to given exception instance (don't throw) and return 0
            {
                (ex_ptr->*ex_handle_setter)(object_handle);
                return 0;
            }
        }
        if (object_id_res.size() != 1)
        {
            BOOST_THROW_EXCEPTION(InternalError("failed to get object handle"));
        }
        return  static_cast<unsigned long long> (object_id_res[0][0]);
    }

}//namespace Fred
#endif //OBJECT_IMPL_H_54ff2fba491943db8a52ff835e32741d
