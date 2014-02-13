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

#ifndef OBJECT__H_
#define OBJECT__H_

#include <string>

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "src/fredlib/opexception.h"
#include "src/fredlib/opcontext.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/printable.h"

namespace Fred
{
    struct CreateObjectOutput
    {
        unsigned long long object_id;
        unsigned long long history_id;
        CreateObjectOutput()
        : object_id(0)
        , history_id(0)
        {}
    };
    /**
     * Creates common part of registry object.
     */
    class CreateObject : public virtual Util::Printable
    {
        const std::string object_type_;//object type name
        const std::string handle_;//object identifier
        const std::string registrar_;//set registrar
        Optional<std::string> authinfo_;//set authinfo
        Nullable<unsigned long long> logd_request_id_;//logger request_id
    public:
        DECLARE_EXCEPTION_DATA(invalid_object_handle, std::string);
        DECLARE_EXCEPTION_DATA(unknown_registrar_handle, std::string);
        struct Exception
        : virtual Fred::OperationException
        , ExceptionData_invalid_object_handle<Exception>
        , ExceptionData_unknown_registrar_handle<Exception>
        {};

        CreateObject(const std::string& object_type
                , const std::string& handle
                , const std::string& registrar);
        CreateObject(const std::string& object_type
                , const std::string& handle
            , const std::string& registrar
            , const Optional<std::string>& authinfo
            , const Nullable<unsigned long long>& logd_request_id);
        CreateObject& set_authinfo(const std::string& authinfo);
        CreateObject& set_logd_request_id(const Nullable<unsigned long long>& logd_request_id);
        CreateObjectOutput exec(OperationContext& ctx);

        std::string to_string() const;
    };

    /**
     * Updates some common parts of registry object.
     */
    class UpdateObject : public virtual Util::Printable
    {
        const std::string handle_;//object identifier
        const std::string obj_type_;//object type name
        const std::string registrar_;//set registrar performing the update
        Optional<std::string> sponsoring_registrar_;//set registrar administering the object
        Optional<std::string> authinfo_;//set authinfo
        Nullable<unsigned long long> logd_request_id_;//logger request_id

    public:
        DECLARE_EXCEPTION_DATA(unknown_object_handle, std::string);
        DECLARE_EXCEPTION_DATA(unknown_registrar_handle, std::string);
        DECLARE_EXCEPTION_DATA(unknown_sponsoring_registrar_handle, std::string);
        struct Exception
        : virtual Fred::OperationException
        , ExceptionData_unknown_object_handle<Exception>
        , ExceptionData_unknown_registrar_handle<Exception>
        , ExceptionData_unknown_sponsoring_registrar_handle<Exception>
        {};

        UpdateObject(const std::string& handle
                , const std::string& obj_type
                , const std::string& registrar);
        UpdateObject(const std::string& handle
            , const std::string& obj_type
            , const std::string& registrar
            , const Optional<std::string>& sponsoring_registrar
            , const Optional<std::string>& authinfo
            , const Nullable<unsigned long long>& logd_request_id
        );
        UpdateObject& set_sponsoring_registrar(const std::string& sponsoring_registrar);
        UpdateObject& set_authinfo(const std::string& authinfo);
        UpdateObject& set_logd_request_id(const Nullable<unsigned long long>& logd_request_id);

        unsigned long long exec(OperationContext& ctx);//return history_id

        std::string to_string() const;
    };

    class InsertHistory : public virtual Util::Printable
    {
        Nullable<unsigned long long> logd_request_id_; //id of the new entry in log_entry database table, id is used in other calls to logging within current request
        unsigned long long object_id_;
    public:
        InsertHistory(const Nullable<unsigned long long>& logd_request_id, unsigned long long object_id);
        unsigned long long exec(OperationContext& ctx);

        std::string to_string() const;
    };

    class DeleteObject : public virtual Util::Printable
    {
        const std::string handle_;//object identifier
        const std::string obj_type_;//object type name
    public:
        DECLARE_EXCEPTION_DATA(unknown_object_handle, std::string);
        struct Exception
        : virtual Fred::OperationException
        , ExceptionData_unknown_object_handle<Exception>
        {};
        DeleteObject(const std::string& handle
                , const std::string& obj_type);
        void exec(OperationContext& ctx);

        std::string to_string() const;
    };

}//namespace Fred
#endif //OBJECT_H_
