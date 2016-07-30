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

#include "src/fredlib/opcontext.h"
#include "src/fredlib/object/object_type.h"

namespace Fred
{

template < Object_Type::Enum OBJECT_TYPE >
class TestHandleOf
{
public:
    TestHandleOf(const std::string &_handle);

    //check handle syntax
    bool is_invalid_handle()const;

    //check if handle is in protected period
    bool is_protected(OperationContext &_ctx)const;

    //check if handle is already registered
    bool is_registered(OperationContext &_ctx)const;
private:
    const std::string handle_;
};

//domain handle is called "fqdn" => no handle operations for domains
template < >
class TestHandleOf< Object_Type::domain > { };

}//namespace Fred

#endif//CHECK_HANDLE_H
