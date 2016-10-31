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
 *  @file object.cc
 *  common object
 */

#include <string>
#include <boost/assign.hpp>
#include <boost/lexical_cast.hpp>

#include "object_impl.h"
#include "src/fredlib/registrar/registrar_impl.h"
#include "src/fredlib/object_state/object_state_name.h"
#include "src/fredlib/object/generate_authinfo_password.h"
#include "src/fredlib/opexception.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/db_settings.h"
#include "util/optional_value.h"
#include "util/log/log.h"
#include "object.h"

namespace Fred
{

    CreateObject::CreateObject(const std::string& object_type
        , const std::string& handle
        , const std::string& registrar)
    : object_type_(object_type)
    , handle_(handle)
    , registrar_(registrar)
    {}

    CreateObject::CreateObject(const std::string& object_type
        , const std::string& handle
        , const std::string& registrar
        , const Optional<std::string>& authinfo
        , const Nullable<unsigned long long>& logd_request_id)
    : object_type_(object_type)
    , handle_(handle)
    , registrar_(registrar)
    , authinfo_(authinfo)
    , logd_request_id_(logd_request_id)
    {}

    CreateObject& CreateObject::set_authinfo(const std::string& authinfo)
    {
        authinfo_ = authinfo;
        return *this;
    }

    CreateObject& CreateObject::set_logd_request_id(const Nullable<unsigned long long>& logd_request_id)
    {
        logd_request_id_ = logd_request_id;
        return *this;
    }

    CreateObject::Result  CreateObject::exec(OperationContext& ctx)
    {
        Result result;

        try
        {
            //check registrar
            unsigned long long registrar_id = Registrar::get_registrar_id_by_handle(
                ctx, registrar_, static_cast<Exception*>(0)//set throw
                , &Exception::set_unknown_registrar_handle);

            //check object type
            unsigned long long object_type_id = get_object_type_id(ctx, object_type_);

            //create object
            Database::Result id_res = ctx.get_conn().exec_params(
                "SELECT create_object($1::integer "//registrar
                    " , $2::text "//object handle
                    " , $3::integer )"//object type
                        , Database::query_param_list(registrar_id)(handle_)(object_type_id));
            if (id_res.size() != 1)
            {
                BOOST_THROW_EXCEPTION(InternalError("unable to call create_object"));
            }
            result.object_id = static_cast< unsigned long long >(id_res[0][0]);

            if (result.object_id == 0)
            {
                BOOST_THROW_EXCEPTION(Exception().set_invalid_object_handle(handle_));
            }

            if (authinfo_.get_value_or_default().empty())
            {
                authinfo_ = generate_authinfo_pw().password_;
            }

            ctx.get_conn().exec_params("INSERT INTO object(id, clid, authinfopw) VALUES ($1::bigint "//object id from create_object
                    " , $2::integer, $3::text)"
                    , Database::query_param_list(result.object_id)(registrar_id)(authinfo_.get_value()));

            result.history_id = Fred::InsertHistory(logd_request_id_, result.object_id).exec(ctx);

            //object_registry historyid
            Database::Result update_historyid_res = ctx.get_conn().exec_params(
                "UPDATE object_registry SET historyid = $1::bigint, crhistoryid = $1::bigint "
                    " WHERE id = $2::integer RETURNING id"
                    , Database::query_param_list(result.history_id)(result.object_id));
            if (update_historyid_res.size() != 1)
            {
                BOOST_THROW_EXCEPTION(Fred::InternalError("historyid update failed"));
            }


        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }

        return result;
    }

    /**
    * Dumps state of the instance into the string
    * @return string with description of the instance state
    */
    std::string CreateObject::to_string() const
    {
        return Util::format_operation_state("CreateObject",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("object_type",object_type_))
        (std::make_pair("handle",handle_))
        (std::make_pair("registrar",registrar_))
        (std::make_pair("authinfo",authinfo_.print_quoted()))
        (std::make_pair("logd_request_id",logd_request_id_.print_quoted()))
        );
    }

    UpdateObject::UpdateObject(const std::string& handle
        , const std::string& obj_type
        , const std::string& registrar)
    : handle_(handle)
    , obj_type_(obj_type)
    , registrar_(registrar)
    {}

    UpdateObject::UpdateObject(const std::string& handle
        , const std::string& obj_type
        , const std::string& registrar
        , const Optional<std::string>& authinfo
        , const Nullable<unsigned long long>& logd_request_id)
    : handle_(handle)
    , obj_type_(obj_type)
    , registrar_(registrar)
    , authinfo_(authinfo)
    , logd_request_id_(logd_request_id)
    {}

    UpdateObject& UpdateObject::set_authinfo(const std::string& authinfo)
    {
        authinfo_ = authinfo;
        return *this;
    }

    UpdateObject& UpdateObject::set_logd_request_id(const Nullable<unsigned long long>& logd_request_id)
    {
        logd_request_id_ = logd_request_id;
        return *this;
    }

    unsigned long long UpdateObject::exec(OperationContext& ctx)
    {
        unsigned long long history_id = 0;
        try
        {
            //check registrar
            unsigned long long registrar_id = Registrar::get_registrar_id_by_handle(
                ctx, registrar_, static_cast<Exception*>(0)//set throw
                , &Exception::set_unknown_registrar_handle);

            Exception update_object_exception;

            //get object id with lock
            unsigned long long object_id = get_object_id_by_handle_and_type_with_lock(
                ctx, true, handle_,obj_type_,&update_object_exception,
                &Exception::set_unknown_object_handle);

            Database::QueryParams params;//query params
            std::stringstream sql;
            params.push_back(registrar_id);
            sql <<"UPDATE object SET update = now() "
                ", upid = $"
                << params.size() << "::integer " ; //registrar from epp-session container by client_id from epp-params

            if(authinfo_.isset())
            {
                params.push_back(authinfo_);
                sql << " , authinfopw = $" << params.size() << "::text ";//set authinfo
            }

            params.push_back(object_id);
            sql <<" WHERE id = $" << params.size() << "::integer ";

            //check exception
            if(update_object_exception.throw_me())
            {
                BOOST_THROW_EXCEPTION(update_object_exception);
            }

            ctx.get_conn().exec_params(sql.str(), params);

            history_id = Fred::InsertHistory(logd_request_id_, object_id).exec(ctx);

            //object_registry historyid
            Database::Result update_historyid_res = ctx.get_conn().exec_params(
                "UPDATE object_registry SET historyid = $1::bigint "
                    " WHERE id = $2::integer RETURNING id"
                    , Database::query_param_list(history_id)(object_id));
            if (update_historyid_res.size() != 1)
            {
                BOOST_THROW_EXCEPTION(Fred::InternalError("historyid update failed"));
            }


        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }

        return history_id;
    }

    /**
    * Dumps state of the instance into the string
    * @return string with description of the instance state
    */
    std::string UpdateObject::to_string() const
    {
        return Util::format_operation_state("UpdateObject",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("obj_type",obj_type_))
        (std::make_pair("handle",handle_))
        (std::make_pair("registrar",registrar_))
        (std::make_pair("authinfo",authinfo_.print_quoted()))
        (std::make_pair("logd_request_id",logd_request_id_.print_quoted()))
        );
    }

    InsertHistory::InsertHistory(const Nullable<unsigned long long>& logd_request_id
        , unsigned long long object_id)
    : logd_request_id_(logd_request_id)
    , object_id_(object_id)
    {}

    unsigned long long InsertHistory::exec(OperationContext& ctx)
    {
        unsigned long long history_id = 0;
        try
        {
            Database::Result history_id_res = ctx.get_conn().exec_params(
                "INSERT INTO history(request_id) VALUES ($1::bigint) RETURNING id;"
                , Database::query_param_list(logd_request_id_));

            if (history_id_res.size() != 1)
            {
                BOOST_THROW_EXCEPTION(InternalError("unable to save history"));
            }

            history_id = static_cast<unsigned long long>(history_id_res[0][0]);

            //object_history
            ctx.get_conn().exec_params(
                "INSERT INTO object_history(historyid,id,clid, upid, trdate, update, authinfopw) "
                " SELECT $1::bigint, id,clid, upid, trdate, update, authinfopw FROM object "
                " WHERE id = $2::integer"
                , Database::query_param_list(history_id)(object_id_));
        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }

        return history_id;
    }

    std::string InsertHistory::to_string() const
    {
        return Util::format_operation_state("InsertHistory",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("object_id",boost::lexical_cast<std::string>(object_id_)))
        (std::make_pair("logd_request_id",logd_request_id_.print_quoted()))
        );
    }

    static void delete_object_impl(OperationContext& ctx, unsigned long long id) {
        Database::Result update_erdate_res = ctx.get_conn().exec_params(
            "UPDATE object_registry "
            "   SET erdate = now() "
            "   WHERE id = $1::integer RETURNING id",
            Database::query_param_list(id));

        if (update_erdate_res.size() != 1) {
            BOOST_THROW_EXCEPTION(Fred::InternalError("erdate update failed"));
        }

        Database::Result delete_object_res = ctx.get_conn().exec_params(
            "DELETE FROM object WHERE id = $1::integer RETURNING id",
            Database::query_param_list(id));

        if (delete_object_res.size() != 1) {
            BOOST_THROW_EXCEPTION(Fred::InternalError("delete object failed"));
        }
    }

    DeleteObjectByHandle::DeleteObjectByHandle(const std::string& handle
        , const std::string& obj_type)
    : handle_(handle)
    , obj_type_(obj_type)
    {}

    void DeleteObjectByHandle::exec(OperationContext& ctx)
    {
        try
        {
            //check object type
            get_object_type_id(ctx, obj_type_);

            unsigned long long object_id = get_object_id_by_handle_and_type_with_lock(
                ctx,
                true,
                handle_,
                obj_type_,
                static_cast<Exception*>(NULL),
                &Exception::set_unknown_object_handle);

            delete_object_impl(ctx, object_id);

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
    }

    std::string DeleteObjectByHandle::to_string() const
    {
        return Util::format_operation_state(
            "DeleteObjectByHandle",
            boost::assign::list_of
                (std::make_pair("handle", handle_))
                (std::make_pair("obj_type", obj_type_))
        );
    }

    DeleteObjectById::DeleteObjectById(unsigned long long id)
        : id_(id)
    { }

    void DeleteObjectById::exec(OperationContext& ctx) {
        try
        {
            get_object_id_by_object_id_with_lock(
                ctx,
                id_,
                static_cast<Exception*>(NULL),
                &Exception::set_unknown_object_id
            );

            delete_object_impl(ctx, id_);

        } catch(ExceptionStack& ex) {
            ex.add_exception_stack_info(to_string());
            throw;
        }
    }

    std::string DeleteObjectById::to_string() const {

        return Util::format_operation_state(
            "DeleteObjectById",
            boost::assign::list_of
                (std::make_pair("id", boost::lexical_cast<std::string>(id_) ))
        );
    }

}//namespace Fred
