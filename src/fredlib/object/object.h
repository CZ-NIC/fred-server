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

#include "fredlib/opexception.h"
#include "fredlib/opcontext.h"
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
    class CreateObject
    {
        const std::string object_type_;//object type name
        const std::string handle_;//object identifier
        const std::string registrar_;//set registrar
        Optional<std::string> authinfo_;//set authinfo
        Nullable<unsigned long long> logd_request_id_;//logger request_id
    public:
        DECLARE_EXCEPTION_DATA(invalid_object_handle, std::string);
        DECLARE_EXCEPTION_DATA(unknown_object_type, std::string);
        DECLARE_EXCEPTION_DATA(unknown_registrar_handle, std::string);
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
            , const Optional<std::string>& authinfo
            , const Nullable<unsigned long long>& logd_request_id);
        CreateObject& set_authinfo(const std::string& authinfo);
        CreateObject& set_logd_request_id(const Nullable<unsigned long long>& logd_request_id);
        CreateObjectOutput exec(OperationContext& ctx);

        friend std::ostream& operator<<(std::ostream& os, const CreateObject& i);
        std::string to_string();
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
        DECLARE_EXCEPTION_DATA(unknown_object_type, std::string);
        DECLARE_EXCEPTION_DATA(unknown_registrar_handle, std::string);
        DECLARE_EXCEPTION_DATA(unknown_sponsoring_registrar_handle, std::string);
        struct Exception
        : virtual Fred::OperationException
        , ExceptionData_unknown_object_type<Exception>
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

    class InsertHistory
    {
        Nullable<unsigned long long> logd_request_id_; //id of the new entry in log_entry database table, id is used in other calls to logging within current request
        unsigned long long object_id_;
    public:
        InsertHistory(const Nullable<unsigned long long>& logd_request_id, unsigned long long object_id);
        unsigned long long exec(OperationContext& ctx);

        friend std::ostream& operator<<(std::ostream& os, const InsertHistory& i);
        std::string to_string();
    };

    /**
    * Check existence and lock object type for read.
    * @param EXCEPTION is type of exception used for reporting when object type is not found, deducible from type of @ref ex_ptr parameter
    * @param EXCEPTION_SETTER is EXCEPTION member function pointer used to report unknown object type
    * @param ctx contains reference to database and logging interface
    * @param obj_type is object type to look for in enum_object_type table
    * @param ex_ptr is  pointer to given exception instance to be set (don't throw), if ex_ptr is 0, new exception instance is created, set and thrown
    * @param ex_setter is EXCEPTION member function pointer used to report unknown obj_type
    */

    template <class EXCEPTION, typename EXCEPTION_SETTER>
    void check_object_type(OperationContext& ctx, const std::string& obj_type
            , EXCEPTION* ex_ptr, EXCEPTION_SETTER ex_setter)
    {
        Database::Result object_type_res = ctx.get_conn().exec_params(
            "SELECT id FROM enum_object_type WHERE name = $1::text FOR SHARE"
            , Database::query_param_list(obj_type));
        if(object_type_res.size() == 0)//obj_type not found
        {
            if(ex_ptr == 0)//make new exception instance, set data and throw
            {
                BOOST_THROW_EXCEPTION((EXCEPTION().*ex_setter)(obj_type));
            }
            else//set unknown handle to given exception instance (don't throw) and return 0
            {
                (ex_ptr->*ex_setter)(obj_type);
                return;
            }
        }
        if (object_type_res.size() != 1)//too many
        {
            BOOST_THROW_EXCEPTION(InternalError("failed to get object type"));
        }
    }


    /**
    * Gets object id by handle or fqdn and object type name and locks for update.
    * @param EXCEPTION is type of exception used for reporting when object is not found, deducible from type of @ref ex_ptr parameter
    * @param EXCEPTION_OBJECT_HANDLE_SETTER is EXCEPTION member function pointer used to report unknown object handle
    * @param EXCEPTION_OBJECT_TYPE_SETTER is EXCEPTION member function pointer used to report unknown object type
    * @param ctx contains reference to database and logging interface
    * @param object_handle is handle or fqdn to look for
    * @param object_type is name from enum_object_type
    * @param ex_ptr is  pointer to given exception instance to be set (don't throw except for object_type), if ex_ptr is 0, new exception instance is created, set and thrown
    * @param ex_handle_setter is EXCEPTION member function pointer used to report unknown object handle
    * @param ex_type_setter is EXCEPTION member function pointer used to report unknown object handle
    * @return database id of the object
    * , or throw @ref EXCEPTION if object handle or type was not found and external exception instance was not provided
    * , or set unknown object handle or fqdn into given external exception instance and return 0
    * , or throw InternalError or some other exception in case of failure.
    */
    template <class EXCEPTION, typename EXCEPTION_OBJECT_HANDLE_SETTER, typename EXCEPTION_OBJECT_TYPE_SETTER>
    unsigned long long get_object_id_by_handle_and_type_with_lock(OperationContext& ctx
            , const std::string& object_handle, const std::string& object_type
            , EXCEPTION* ex_ptr, EXCEPTION_OBJECT_HANDLE_SETTER ex_handle_setter, EXCEPTION_OBJECT_TYPE_SETTER ex_type_setter)
    {
        check_object_type(ctx, object_type, static_cast<EXCEPTION*>(0), ex_type_setter);

        Database::Result object_id_res = ctx.get_conn().exec_params(
        "SELECT oreg.id FROM object_registry oreg "
        " JOIN enum_object_type eot ON eot.id = oreg.type AND eot.name = $2::text "
        " WHERE oreg.name = CASE WHEN $2::text = 'domain'::text THEN LOWER($1::text) "
        " ELSE UPPER($1::text) END AND oreg.erdate IS NULL "
        " FOR UPDATE OF oreg"
        , Database::query_param_list(object_handle)(object_type));

        if(object_id_res.size() == 0)
        {
            if(ex_ptr == 0)//make new exception instance, set data and throw
            {
                BOOST_THROW_EXCEPTION((EXCEPTION().*ex_handle_setter)(object_handle));
            }
            else//set unknown handle to given exception instance (don't throw) and return 0
            {
                (ex_ptr->*ex_handle_setter)(object_handle);
                return 0;
            }
        }
        if (object_id_res.size() != 1)
        {
            BOOST_THROW_EXCEPTION(InternalError("failed to get object handle"));
        }
        return  static_cast<unsigned long long> (object_id_res[0][0]);
    }

    class DeleteObject
    {
        const std::string handle_;//object identifier
        const std::string obj_type_;//object type name
    public:
        DECLARE_EXCEPTION_DATA(unknown_object_handle, std::string);
        DECLARE_EXCEPTION_DATA(unknown_object_type, std::string);
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
