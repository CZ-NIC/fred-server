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
 *  @file delete_contact.h
 *  contact delete
 */

#ifndef DELETE_CONTACT_H
#define DELETE_CONTACT_H

#include <string>

#include "fredlib/opexception.h"
#include "fredlib/opcontext.h"

namespace Fred
{

    class DeleteContact
    {
        const std::string handle_;//contact identifier
    public:

        DECLARE_EXCEPTION_DATA(object_linked_to_contact_handle, std::string);
        struct Exception
        : virtual Fred::OperationException
        , ExceptionData_unknown_contact_handle<Exception>
        , ExceptionData_object_linked_to_contact_handle<Exception>
        {};

        DeleteContact(const std::string& handle);
        void exec(OperationContext& ctx);

        friend std::ostream& operator<<(std::ostream& os, const DeleteContact& dc);
        std::string to_string();
    };//class DeleteContact

}//namespace Fred

#endif//DELETE_CONTACT_H
