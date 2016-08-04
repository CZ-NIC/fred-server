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
 *  keyset check
 */

#include <string>

#include <boost/regex.hpp>

#include "src/fredlib/keyset/check_keyset.h"
#include "src/fredlib/object/check_handle.h"

#include "src/fredlib/opcontext.h"
#include "src/fredlib/db_settings.h"
#include "util/printable.h"


namespace Fred
{
    CheckKeyset::CheckKeyset(const std::string& handle)
    : handle_(handle)
    {}
    bool CheckKeyset::is_invalid_handle() const
    {
        try
        {
            return TestHandleOf< Object_Type::keyset >(handle_).is_invalid_handle();
        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
    }

    bool CheckKeyset::is_registered(OperationContext& ctx) const
    {
        try
        {
            return TestHandleOf< Object_Type::keyset >(handle_).is_registered(ctx);
        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
    }


    bool CheckKeyset::is_protected(OperationContext& ctx) const
    {
        try
        {
            return TestHandleOf< Object_Type::keyset >(handle_).is_protected(ctx);
        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
    }

    bool CheckKeyset::is_free(OperationContext& ctx) const
    {
        try
        {
            return !is_invalid_handle() &&
                   !is_registered(ctx) &&
                   !is_protected(ctx);
        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
    }

    std::string CheckKeyset::to_string() const
    {
        return Util::format_operation_state("CheckKeyset",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("handle",handle_)));
    }

}//namespace Fred

