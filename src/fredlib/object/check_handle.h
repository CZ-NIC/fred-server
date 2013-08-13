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
 *  @file check_handle.h
 *  handle check
 */

#ifndef CHECK_HANDLE_H
#define CHECK_HANDLE_H

#include <string>

#include "fredlib/opcontext.h"

namespace Fred
{

    class TestHandle
    {
        const std::string handle_;
    public:
        TestHandle(const std::string& handle);
        //check handle syntax
        bool is_invalid_handle() const;
        //check if handle is in protected period
        bool is_protected(OperationContext& ctx, const std::string& object_type_name) const;
        //check if handle is already registered, if true then set conflicting handle
        bool is_registered(OperationContext& ctx,
            const std::string& object_type_name,//from db enum_object_type.name
            std::string& conflicting_handle_out) const;
    };

}//namespace Fred

#endif//CHECK_HANDLE_H
