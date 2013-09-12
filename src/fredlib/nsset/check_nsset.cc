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
 *  @file
 *  nsset check
 */

#include <string>

#include <boost/regex.hpp>

#include "fredlib/nsset/check_nsset.h"
#include "fredlib/object/check_handle.h"

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
            if(TestHandle(handle_).is_invalid_handle()) return true;
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
            if(TestHandle(handle_).is_registered(ctx,"nsset",conflicting_handle_out)) return true;
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
            if(TestHandle(handle_).is_protected(ctx,"nsset")) return true;
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
        return true;//meaning ok
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

