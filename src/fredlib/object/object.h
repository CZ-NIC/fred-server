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
 *  @file object.h
 *  common object
 */

#ifndef OBJECT_H_
#define OBJECT_H_

#include <string>


#include "fredlib/opexception.h"
#include "fredlib/opcontext.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"



namespace Fred
{

    class CreateObject
    {
        const std::string object_type_;//object type name
        const std::string handle_;//object identifier
        const std::string registrar_;//set registrar
        Optional<std::string> authinfo_;//set authinfo
    public:
        CreateObject(const std::string& object_type
                , const std::string& handle
                , const std::string& registrar);
        CreateObject(const std::string& object_type
                , const std::string& handle
            , const std::string& registrar
            , const Optional<std::string>& authinfo);
        CreateObject& set_authinfo(const std::string& authinfo);
        unsigned long long exec(OperationContext& ctx);
    };
    //exception impl
    class CreateObjectException
    : public OperationExceptionImpl<CreateObjectException, 8192>
    {
    public:
        CreateObjectException(const char* file
                , const int line
                , const char* function
                , const char* data)
        : OperationExceptionImpl<CreateObjectException, 8192>(file, line, function, data)
        {}

        ConstArr get_fail_param_impl() throw()
        {
            static const char* list[]={"not found:object type", "invalid:handle", "not found:registrar"};
            return ConstArr(list,sizeof(list)/sizeof(char*));
        }

    };//class CreateObjectException

    typedef CreateObjectException::OperationErrorType CreteaObjectError;
#define COEX(DATA) CreateObjectException(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))
#define COERR(DATA) CreteaObjectError(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))


    class UpdateObject
    {
        const std::string handle_;//object identifier
        const std::string obj_type_;//object type name
        const std::string registrar_;//set registrar
        Optional<std::string> authinfo_;//set authinfo
    public:
        UpdateObject(const std::string& handle
                , const std::string& obj_type
                , const std::string& registrar);
        UpdateObject(const std::string& handle
            , const std::string& obj_type
            , const std::string& registrar
            , const Optional<std::string>& authinfo);
        UpdateObject& set_authinfo(const std::string& authinfo);
        void exec(OperationContext& ctx);
    };

    //exception impl
    class UpdateObjectException
    : public OperationExceptionImpl<UpdateObjectException, 8192>
    {
    public:
        UpdateObjectException(const char* file
                , const int line
                , const char* function
                , const char* data)
        : OperationExceptionImpl<UpdateObjectException, 8192>(file, line, function, data)
        {}

        ConstArr get_fail_param_impl() throw()
        {
            static const char* list[]={"not found:handle", "not found:registrar", "not found:object type"};
            return ConstArr(list,sizeof(list)/sizeof(char*));
        }

    };//class UpdateObjectException

    typedef UpdateObjectException::OperationErrorType UpdateObjectError;
#define UOEX(DATA) UpdateObjectException(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))
#define UOERR(DATA) UpdateObjectError(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))


    class InsertHistory
    {
        Nullable<unsigned long long> logd_request_id_; //id of the new entry in log_entry database table, id is used in other calls to logging within current request
    public:
        InsertHistory(const Nullable<unsigned long long>& logd_request_id);
        unsigned long long exec(OperationContext& ctx);
    };

    //exception impl
        class InsertHistoryException
        : public OperationExceptionImpl<InsertHistoryException, 8192>
        {
        public:
            InsertHistoryException(const char* file
                    , const int line
                    , const char* function
                    , const char* data)
            : OperationExceptionImpl<InsertHistoryException, 8192>(file, line, function, data)
            {}

            ConstArr get_fail_param_impl() throw()
            {
                static const char* list[]={"invalid:logd_request_id"};
                return ConstArr(list,sizeof(list)/sizeof(char*));
            }

        };//class InsertHistoryException

    typedef InsertHistoryException::OperationErrorType InsertHistoryError;
#define IHEX(DATA) InsertHistoryException(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))
#define IHERR(DATA) InsertHistoryError(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))

}//namespace Fred
#endif //OBJECT_H_
