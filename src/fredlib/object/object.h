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
        DECLARE_EXCEPTION_DATA(invalid_object_handle, std::string);
        struct Exception
        : virtual Fred::OperationException
        , ExceptionData_unknown_object_type<Exception>
        , ExceptionData_invalid_object_handle<Exception>
        , ExceptionData_unknown_registrar_handle<Exception>
        {};

        CreateObject(const std::string& object_type
                , const std::string& handle
                , const std::string& registrar);
        CreateObject(const std::string& object_type
                , const std::string& handle
            , const std::string& registrar
            , const Optional<std::string>& authinfo);
        CreateObject& set_authinfo(const std::string& authinfo);
        unsigned long long exec(OperationContext& ctx);

        friend std::ostream& operator<<(std::ostream& os, const CreateObject& i);
        std::string to_string();
    };

    class UpdateObject
    {
        const std::string handle_;//object identifier
        const std::string obj_type_;//object type name
        const std::string registrar_;//set registrar
        Optional<std::string> authinfo_;//set authinfo
    public:
        DECLARE_EXCEPTION_DATA(unknown_object_handle, std::string);
        struct Exception
        : virtual Fred::OperationException
        , ExceptionData_unknown_object_type<Exception>
        , ExceptionData_unknown_object_handle<Exception>
        , ExceptionData_unknown_registrar_handle<Exception>
        {};

        UpdateObject(const std::string& handle
                , const std::string& obj_type
                , const std::string& registrar);
        UpdateObject(const std::string& handle
            , const std::string& obj_type
            , const std::string& registrar
            , const Optional<std::string>& authinfo);
        UpdateObject& set_authinfo(const std::string& authinfo);
        void exec(OperationContext& ctx);

        friend std::ostream& operator<<(std::ostream& os, const UpdateObject& i);
        std::string to_string();
    };

    class InsertHistory
    {
        Nullable<unsigned long long> logd_request_id_; //id of the new entry in log_entry database table, id is used in other calls to logging within current request
    public:
        InsertHistory(const Nullable<unsigned long long>& logd_request_id);
        unsigned long long exec(OperationContext& ctx);

        friend std::ostream& operator<<(std::ostream& os, const InsertHistory& i);
        std::string to_string();
    };

    class DeleteObject
    {
        const std::string handle_;//object identifier
        const std::string obj_type_;//object type name
    public:
        DECLARE_EXCEPTION_DATA(unknown_object_handle, std::string);
        struct Exception
        : virtual Fred::OperationException
        , ExceptionData_unknown_object_type<Exception>
        , ExceptionData_unknown_object_handle<Exception>
        {};
        DeleteObject(const std::string& handle
                , const std::string& obj_type);
        void exec(OperationContext& ctx);

        friend std::ostream& operator<<(std::ostream& os, const DeleteObject& i);
        std::string to_string();
    };

}//namespace Fred
#endif //OBJECT_H_
