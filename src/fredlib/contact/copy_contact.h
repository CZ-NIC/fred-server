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

#include "src/fredlib/opexception.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/object_state/typedefs.h"
#include "util/optional_value.h"

namespace Fred
{

    typedef unsigned long long RequestId;

    class CopyContact
    {
    public:
        enum DbConst { OBJECT_TYPE_ID_CONTACT = 1 };
        CopyContact(const std::string &_src_contact_handle,
            const std::string &_dst_contact_handle,
            RequestId _request_id);
        CopyContact(const std::string &_src_contact_handle,
            const std::string &_dst_contact_handle,
            const Optional< std::string > &_dst_registrar_handle,
            RequestId _request_id);
        CopyContact& set_registrar_handle(const std::string &_registrar_handle);
        ObjectId exec(OperationContext &_ctx);

    //exception impl
        DECLARE_EXCEPTION_DATA(src_contact_handle_not_found, std::string);
        DECLARE_EXCEPTION_DATA(dst_contact_handle_already_exist, std::string);
        DECLARE_EXCEPTION_DATA(create_contact_failed, std::string);

        struct Exception
        :   virtual Fred::OperationException,
            ExceptionData_src_contact_handle_not_found<Exception>,
            ExceptionData_dst_contact_handle_already_exist<Exception>,
            ExceptionData_create_contact_failed<Exception>
        {};
    private:
        const std::string src_contact_handle_;
        const std::string dst_contact_handle_;
        Optional< std::string > dst_registrar_handle_;
        const RequestId request_id_;
    };//class CopyContact


}//namespace Fred

#endif//COPY_CONTACT_H_
