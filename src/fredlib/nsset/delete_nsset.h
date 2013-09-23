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
 *  nsset delete
 */

#ifndef DELETE_NSSET_H
#define DELETE_NSSET_H

#include <string>

#include "fredlib/opexception.h"
#include "fredlib/opcontext.h"
#include "util/printable.h"

namespace Fred
{

    /**
    * Delete of nsset.
    * Nsset handle to delete is set via constructor.
    * Delete is executed by @ref exec method with database connection supplied in @ref OperationContext parameter.
    * When exception is thrown, changes to database are considered incosistent and should be rolled back by the caller.
    * In case of wrong input data or other predictable and superable failure, an instance of @ref DeleteNsset::Exception is thrown with appropriate attributes set.
    * In case of other unsuperable failures and inconstistencies, an instance of @ref InternalError or other exception is thrown.
    */
    class DeleteNsset : public Util::Printable
    {
        const std::string handle_;/**< nsset identifier */
    public:
        DECLARE_EXCEPTION_DATA(unknown_nsset_handle, std::string);/**< exception members for unknown nsset handle generated by macro @ref DECLARE_EXCEPTION_DATA*/
        DECLARE_EXCEPTION_DATA(object_linked_to_nsset_handle, std::string);/**< exception members for nsset linked to other object generated by macro @ref DECLARE_EXCEPTION_DATA*/
        struct Exception
        : virtual Fred::OperationException
        , ExceptionData_unknown_nsset_handle<Exception>
        , ExceptionData_object_linked_to_nsset_handle<Exception>
        {};

        /**
        * Delete nsset constructor with mandatory parameter.
        * @param handle sets nsset identifier into @ref handle_ attribute
        */
        DeleteNsset(const std::string& handle);

        /**
        * Executes delete.
        * @param ctx contains reference to database and logging interface
        */
        void exec(OperationContext& ctx);

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;
    };//class DeleteNsset

}//namespace Fred

#endif//DELETE_NSSET_H
