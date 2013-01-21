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
 *  @file create_contact.h
 *  create contact
 */

#ifndef CREATE_CONTACT_H_
#define CREATE_CONTACT_H_

#include <string>
#include <vector>

#include "fredlib/opcontext.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"


namespace Fred
{

    class CreateContact
    {
        const std::string handle_;//contact identifier
        const std::string registrar_;//registrar identifier
        Optional<std::string> authinfo_;//set authinfo
    public:
        CreateContact(const std::string& handle
                , const std::string& registrar);
        CreateContact(const std::string& handle
                , const std::string& registrar
                , const Optional<std::string>& authinfo
                );

        CreateContact& set_authinfo(const std::string& authinfo);

        std::string exec(OperationContext& ctx);

    };//CreateContact

    //exception impl
    class CreateContactException
    : public OperationExceptionImpl<CreateContactException, 2048>
    {
    public:
        CreateContactException(const char* file
                , const int line
                , const char* function
                , const char* data)
        : OperationExceptionImpl<CreateContactException, 2048>(file, line, function, data)
        {}

        ConstArr get_fail_param_impl() throw()
        {
            static const char* list[]={"not found:object type", "invalid:handle", "not found:registrar"};
            return ConstArr(list,sizeof(list)/sizeof(char*));
        }

    };//class CreateContactException

    typedef CreateContactException::OperationErrorType CreteaContactError;
#define CCEX(DATA) CreateContactException(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))
#define CCERR(DATA) CreteaContactError(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))

}
#endif // CREATE_CONTACT_H_
