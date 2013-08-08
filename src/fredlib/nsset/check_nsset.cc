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
 *  @file check_nsset.cc
 *  nsset check
 */

#include <string>

#include <boost/regex.hpp>

#include "fredlib/nsset/check_nsset.h"

#include "fredlib/opcontext.h"
#include "fredlib/db_settings.h"


namespace Fred
{
    CheckNsset::CheckNsset(const std::string& handle)
    : handle_(handle)
    {}
    bool CheckNsset::is_invalid_handle()
    {
        try
        {
            static const boost::regex NSSET_HANDLE_SYNTAX("[a-zA-Z0-9_:.-]{1,63}");
            if(!boost::regex_match(handle_, NSSET_HANDLE_SYNTAX)) return true;
        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return false;//meaning handle syntax is ok
    }

    bool CheckNsset::is_registered(OperationContext& ctx, std::string& conflicting_handle_out)
    {
        try
        {
            Database::Result conflicting_handle_res  = ctx.get_conn().exec_params(
                "SELECT o.name, o.id FROM object_registry o JOIN enum_object_type eot on o.type = eot.id "
                " WHERE eot.name='nsset' AND o.erdate ISNULL AND o.name=$1::text LIMIT 1"
            , Database::query_param_list(handle_));
            if(conflicting_handle_res.size() > 0)//have conflicting_handle
            {
                conflicting_handle_out = static_cast<std::string>(conflicting_handle_res[0][0]);
                return true;
            }
        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return false;//meaning not protected
    }

    bool CheckNsset::is_registered(OperationContext& ctx)
    {
        std::string conflicting_handle_out;
        return is_registered(ctx, conflicting_handle_out);
    }


    bool CheckNsset::is_protected(OperationContext& ctx)
    {
        try
        {
            Database::Result protection_res = ctx.get_conn().exec_params(
            "SELECT COALESCE(MAX(oreg.erdate) + ((SELECT val FROM enum_parameters "
                " WHERE name = 'handle_registration_protection_period') || ' month')::interval > CURRENT_TIMESTAMP, false) "
            " FROM object_registry oreg "
            " JOIN enum_object_type eot ON oreg.type = eot.id "
            " WHERE oreg.erdate IS NOT NULL "
            " AND eot.name='nsset' "
            " AND oreg.name=UPPER($1::text)"
            , Database::query_param_list(handle_));

            if(static_cast<bool>(protection_res[0][0]) == true) return true;

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return false;//meaning not protected
    }

    bool CheckNsset::is_free(OperationContext& ctx)
    {
        try
        {
            if(is_invalid_handle()
            || is_registered(ctx)
            || is_protected(ctx))
            {
                return false;
            }
        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return false;//meaning ok
    }

    std::ostream& operator<<(std::ostream& os, const CheckNsset& i)
    {
        return os << "#CheckNsset handle: " << i.handle_
                ;
    }
    std::string CheckNsset::to_string()
    {
        std::stringstream ss;
        ss << *this;
        return ss.str();
    }

}//namespace Fred

