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
 *  @file copy_contact.h
 *  copy contact
 */

#ifndef COPY_CONTACT_H_
#define COPY_CONTACT_H_

#include "create_object_state_request.h"

namespace Fred
{

    typedef unsigned long long RequestId;

    class CopyContact
    {
    public:
        enum { OBJECT_TYPE_ID_CONTACT = 1 };
        CopyContact(const std::string &_src_contact_handle,
            const std::string &_dst_contact_handle,
            RequestId _request_id);
        ObjectId exec(OperationContext &_ctx);

    //exception impl
        class Exception
        : public OperationExceptionImpl< Exception, 2048 >
        {
        public:
            Exception(const char* file,
                const int line,
                const char* function,
                const char* data)
            :   OperationExceptionImpl< Exception, 2048 >(file, line, function, data)
            {}

            ConstArr get_fail_param_impl() throw()
            {
                static const char* list[] = {"not found:src_contact_handle", "create failed:dst_contact_handle"};
                return ConstArr(list, sizeof(list) / sizeof(char*));
            }
        };//class CopyContactException

        typedef Exception::OperationErrorType Error;
    private:
        const std::string src_contact_handle_;
        const std::string dst_contact_handle_;
        const RequestId request_id_;
    };//class CopyContact


}//namespace Fred

#endif//COPY_CONTACT_H_
