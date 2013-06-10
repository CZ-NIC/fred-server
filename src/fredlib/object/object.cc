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

#include "fredlib/object/object.h"

#include "fredlib/opexception.h"
#include "fredlib/opcontext.h"
#include "fredlib/db_settings.h"
#include "util/optional_value.h"

#include "util/log/log.h"

#include "util/random_data_generator.h"


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
        , const Optional<std::string>& authinfo)
    : object_type_(object_type)
    , handle_(handle)
    , registrar_(registrar)
    , authinfo_(authinfo)
    {}

    CreateObject& CreateObject::set_authinfo(const std::string& authinfo)
    {
        authinfo_ = authinfo;
        return *this;
    }

    unsigned long long  CreateObject::exec(OperationContext& ctx)
    {
        unsigned long long object_id = 0;

        try
        {
            //check registrar
            unsigned long long registrar_id = 0;
            {
                Database::Result registrar_res = ctx.get_conn().exec_params(
                    "SELECT id FROM registrar WHERE handle = UPPER($1::text) FOR SHARE"
                    , Database::query_param_list(registrar_));
                if(registrar_res.size() == 0)
                {
                    BOOST_THROW_EXCEPTION(Exception().set_unknown_registrar_handle(registrar_));
                }
                if (registrar_res.size() != 1)
                {
                    BOOST_THROW_EXCEPTION(InternalError("failed to get registrar"));
                }
                registrar_id = static_cast<unsigned long long>(registrar_res[0][0]);
            }

            //check object type
            unsigned long long object_type_id = 0;
            {
                Database::Result object_type_res = ctx.get_conn().exec_params(
                    "SELECT id FROM enum_object_type WHERE name = $1::text FOR SHARE"
                    , Database::query_param_list(object_type_));
                if(object_type_res.size() == 0)
                {
                    BOOST_THROW_EXCEPTION(Exception().set_unknown_object_type(object_type_));
                }
                if (object_type_res.size() != 1)
                {
                    BOOST_THROW_EXCEPTION(InternalError("failed to get object type"));
                }
                object_type_id = static_cast<unsigned long long>(object_type_res[0][0]);
            }

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
            object_id = static_cast<unsigned long long>(id_res[0][0]);

            if (object_id == 0)
            {
                BOOST_THROW_EXCEPTION(Exception().set_invalid_object_handle(handle_));
            }

            if(authinfo_.get_value().empty())
            {
                authinfo_ = RandomDataGenerator().xnstring(8);//former PASS_LEN
            }

            ctx.get_conn().exec_params("INSERT INTO object(id, clid, authinfopw) VALUES ($1::bigint "//object id from create_object
                    " , $2::integer, $3::text)"
                    , Database::query_param_list(object_id)(registrar_id)(authinfo_));

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }

        return object_id;
    }

    std::ostream& operator<<(std::ostream& os, const CreateObject& i)
    {
        os << "#CreateObject object_type: " << i.object_type_
            << " handle: " << i.handle_
            << " registrar: " << i.registrar_
            << " authinfo: " << i.authinfo_.print_quoted()
            ;
        return os;
    }

    std::string CreateObject::to_string()
    {
        std::stringstream ss;
        ss << *this;
        return ss.str();
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
        , const Optional<std::string>& authinfo)
    : handle_(handle)
    , obj_type_(obj_type)
    , registrar_(registrar)
    , authinfo_(authinfo)
    {}

    UpdateObject& UpdateObject::set_authinfo(const std::string& authinfo)
    {
        authinfo_ = authinfo;
        return *this;
    }

    void UpdateObject::exec(OperationContext& ctx)
    {
        try
        {
            //check registrar
            unsigned long long registrar_id = 0;
            {
                Database::Result registrar_res = ctx.get_conn().exec_params(
                    "SELECT id FROM registrar WHERE handle = UPPER($1::text) FOR SHARE"
                    , Database::query_param_list(registrar_));
                if(registrar_res.size() == 0)
                {
                    BOOST_THROW_EXCEPTION(Exception().set_unknown_registrar_handle(registrar_));
                }
                if (registrar_res.size() != 1)
                {
                    BOOST_THROW_EXCEPTION(InternalError("failed to get registrar"));
                }
                registrar_id = static_cast<unsigned long long>(registrar_res[0][0]);
            }

            //check object type
            {
                Database::Result object_type_res = ctx.get_conn().exec_params(
                    "SELECT id FROM enum_object_type WHERE name = $1::text FOR SHARE"
                    , Database::query_param_list(obj_type_));
                if(object_type_res.size() == 0)
                {
                    BOOST_THROW_EXCEPTION(Exception().set_unknown_object_type(obj_type_));
                }
                if (object_type_res.size() != 1)
                {
                    BOOST_THROW_EXCEPTION(InternalError("failed to get object type"));
                }
            }

            unsigned long long object_id = 0;
            {
                Database::Result object_id_res = ctx.get_conn().exec_params(
                "SELECT oreg.id FROM object_registry oreg "
                " JOIN enum_object_type eot ON eot.id = oreg.type AND eot.name = $2::text "
                " WHERE oreg.name = CASE WHEN $2::text = 'domain'::text THEN LOWER($1::text) "
                " ELSE UPPER($1::text) END AND oreg.erdate IS NULL"
                , Database::query_param_list(handle_)(obj_type_));

                if(object_id_res.size() == 0)
                {
                    BOOST_THROW_EXCEPTION(Exception().set_unknown_object_handle(handle_));
                }
                if (object_id_res.size() != 1)
                {
                    BOOST_THROW_EXCEPTION(InternalError("failed to get object handle"));
                }
                object_id = static_cast<unsigned long long> (object_id_res[0][0]);
            }

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

        ctx.get_conn().exec_params(sql.str(), params);

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
    }

    std::ostream& operator<<(std::ostream& os, const UpdateObject& i)
    {
        os << "#UpdateObject obj_type: " << i.obj_type_
            << " handle: " << i.handle_
            << " registrar: " << i.registrar_
            << " authinfo: " << i.authinfo_.print_quoted()
            ;
        return os;
    }

    std::string UpdateObject::to_string()
    {
        std::stringstream ss;
        ss << *this;
        return ss.str();
    }


    InsertHistory::InsertHistory(const Nullable<unsigned long long>& logd_request_id)
        : logd_request_id_(logd_request_id)
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

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }

        return history_id;
    }

    std::ostream& operator<<(std::ostream& os, const InsertHistory& i)
    {
        os << "#InsertHistory logd_request_id: " << i.logd_request_id_.print_quoted()
            ;
        return os;
    }

    std::string InsertHistory::to_string()
    {
        std::stringstream ss;
        ss << *this;
        return ss.str();
    }


}//namespace Fred
