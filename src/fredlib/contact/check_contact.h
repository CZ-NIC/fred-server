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
 *  @file check_contact.h
 *  contact check
 */

#ifndef CHECK_CONTACT_H
#define CHECK_CONTACT_H

#include <string>

#include "fredlib/opexception.h"
#include "fredlib/opcontext.h"

namespace Fred
{

    class CheckContact
    {
        const std::string handle_;//contact identifier
    public:
        CheckContact(const std::string& handle);
        //check contact handle syntax
        bool is_invalid_handle();
        //check if contact handle is already registered, if true then set conflicting handle
        bool is_registered(OperationContext& ctx, std::string& conflicting_handle_out);
        bool is_registered(OperationContext& ctx);
        //check if contact handle is in protected period
        bool is_protected(OperationContext& ctx);
        //check if contact handle is free for registration
       bool is_free(OperationContext& ctx);

        friend std::ostream& operator<<(std::ostream& os, const CheckContact& i);
        std::string to_string();
    };//class CheckContact

}//namespace Fred

#endif//CHECK_CONTACT_H
