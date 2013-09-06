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
 *  @file
 *  contact delete
 */

#ifndef DELETE_CONTACT_H
#define DELETE_CONTACT_H

#include <string>

#include "fredlib/opexception.h"
#include "fredlib/opcontext.h"

namespace Fred
{
    /**
    * Delete of the contact.
    * Contact handle to delete is set via constructor.
    * Delete is executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
    * When exception is thrown, changes to database are considered incosistent and should be rolled back by caller.
    * In case of wrong input data or other predictable and superable failure the instance of @ref DeleteContact::Exception is thrown with appropriate attributes set.
    * In case of other unsuperable failures and incostistencies the instance of @ref InternalError or other exception is thrown.
    */
    class DeleteContact
    {
        const std::string handle_;/**< contact identifier */
    public:

        DECLARE_EXCEPTION_DATA(unknown_contact_handle, std::string);/**< exception members for unknown contact handle generated by macro @ref DECLARE_EXCEPTION_DATA*/
        DECLARE_EXCEPTION_DATA(object_linked_to_contact_handle, std::string);/**< exception members for contact linked to other object generated by macro @ref DECLARE_EXCEPTION_DATA*/
        struct Exception
        : virtual Fred::OperationException
        , ExceptionData_unknown_contact_handle<Exception>
        , ExceptionData_object_linked_to_contact_handle<Exception>
        {};

        /**
        * Delete contact constructor with mandatory parameter.
        * @param handle sets contact identifier into @ref handle_ attribute
        */
        DeleteContact(const std::string& handle);

        /**
        * Executes delete.
        * @param ctx contains reference to database and logging interface
        */
        void exec(OperationContext& ctx);

        /**
        * Dumps state of the instance into stream
        * @param os contains output stream reference
        * @param i reference of instance to be dumped into the stream
        * @return output stream reference
        */
        friend std::ostream& operator<<(std::ostream& os, const DeleteContact& i);

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string();
    };//class DeleteContact

}//namespace Fred

#endif//DELETE_CONTACT_H
