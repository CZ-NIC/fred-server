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
        DeleteContact(const std::string& handle);
        void exec(OperationContext& ctx);
    };//class DeleteContact

    //exception impl
    class DeleteContactException
    : public OperationExceptionImpl<DeleteContactException, 8192>
    {
    public:
        DeleteContactException(const char* file
                , const int line
                , const char* function
                , const char* data)
        : OperationExceptionImpl<DeleteContactException, 8192>(file, line, function, data)
        {}

        ConstArr get_fail_param_impl() throw()
        {
            static const char* list[]={"not found:handle"
                    , "is linked:handle"
                };
            return ConstArr(list,sizeof(list)/sizeof(char*));
        }
    };//class DeleteContactException

    typedef DeleteContactException::OperationErrorType DeleteContactError;
#define DCEX(DATA) DeleteContactException(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))
#define DCERR(DATA) DeleteContactError(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))

}//namespace Fred

#endif//DELETE_CONTACT_H
