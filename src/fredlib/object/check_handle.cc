/*
 * Copyright (C) 2013  CZ.NIC, z.s.p.o.
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
 *  @file check_keyset.cc
 *  keyset check
 */

#include <string>

#include <boost/regex.hpp>

#include "src/fredlib/keyset/check_keyset.h"
#include "src/fredlib/object/check_handle.h"

#include "src/fredlib/opcontext.h"
#include "src/fredlib/db_settings.h"


namespace Fred
{

    ///init registry handle ctor
    TestHandle::TestHandle(const std::string& handle)
    :handle_(handle)
    {}
    ///check handle syntax
    bool TestHandle::is_invalid_handle() const
    {
        static const boost::regex HANDLE_SYNTAX("[a-zA-Z0-9](-?[a-zA-Z0-9])*");
        if (handle_.length() > 30)
        {
            return true;
        }
        return !boost::regex_match(handle_, HANDLE_SYNTAX);
    }

    //check if handle is in protected period
    bool TestHandle::is_protected(OperationContext& ctx, const std::string& object_type_name) const
    {
        Database::Result protection_res = ctx.get_conn().exec_params(
        "SELECT "
            "COALESCE( "
                "MAX(oreg.erdate) "
                "+ "
                "( "
                    "( "
                        "SELECT val "
                            "FROM enum_parameters "
                            "WHERE name = 'handle_registration_protection_period' "
                    ") || ' month'"
                ")::interval "
                "> "
                "CURRENT_TIMESTAMP"
                ", false "
            ") "
        "FROM object_registry oreg "
        "WHERE "
            "oreg.erdate IS NOT NULL "
            "AND oreg.type = get_object_type_id($1::text) "
            "AND oreg.name=UPPER($2::text) "
        , Database::query_param_list(object_type_name)(handle_));

        if(static_cast<bool>(protection_res[0][0]) == true) return true;

        return false;
    }

    //check if handle is already registered, if true then set conflicting handle
    bool TestHandle::is_registered(OperationContext& ctx,
        const std::string& object_type_name,//from db enum_object_type.name
        std::string& conflicting_handle_out) const
    {
        Database::Result conflicting_handle_res  = ctx.get_conn().exec_params(
            "SELECT o.name, o.id FROM object_registry o JOIN enum_object_type eot on o.type = eot.id "
            " WHERE eot.name=$1::text AND o.erdate ISNULL AND o.name=UPPER($2::text) LIMIT 1"
        , Database::query_param_list(object_type_name)(handle_));

        if(conflicting_handle_res.size() > 0)//have conflicting_handle
        {
            conflicting_handle_out = static_cast<std::string>(conflicting_handle_res[0][0]);
            return true;
        }
        return false;
    }

    //check if handle is already registered, if true then set conflicting handle
    bool TestHandle::is_registered(OperationContext& ctx, const std::string& object_type_name) const {
        std::string dummy;

        return is_registered(ctx, object_type_name, dummy);
    }

}//namespace Fred

