/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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
 *  header of get_objects_id method
 */

#ifndef GET_PRESENT_OBJECT_ID_H_72462423417
#define GET_PRESENT_OBJECT_ID_H_72462423417

#include "src/fredlib/object/object_type.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/db_settings.h"

#include <string>

namespace Fred
{
    struct UnknownObject : std::exception
    {
        virtual const char* what() const throw()
        {
            return "unknown registry object type or handle";
        }
    };

    unsigned long long get_present_object_id(OperationContext& ctx, Fred::Object_Type::Enum object_type, const std::string& handle)
    {
         Database::Result id = ctx.get_conn().exec_params(
                 "SELECT id "
                 "FROM object_registry "
                 "WHERE type = get_object_type_id($1) "
                   "AND name = $2::text "
                   "AND erdate IS NULL ",
                   Database::query_param_list(Conversion::Enums::to_db_handle(object_type))(handle));
         if (id.size() < 1)
         {
             throw UnknownObject();
         }
         return id[0][0];
    }
}

#endif // GET_PRESENT_OBJECT_ID_H_72462423417
