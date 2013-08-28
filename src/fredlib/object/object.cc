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

    CreateObjectOutput  CreateObject::exec(OperationContext& ctx)
    {
        CreateObjectOutput output;

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
            output.object_id = static_cast<unsigned long long>(id_res[0][0]);

            if (output.object_id == 0)
            {
                BOOST_THROW_EXCEPTION(Exception().set_invalid_object_handle(handle_));
            }

            if(authinfo_.get_value().empty())
            {
                authinfo_ = RandomDataGenerator().xnstring(8);//former PASS_LEN
            }

            ctx.get_conn().exec_params("INSERT INTO object(id, clid, authinfopw) VALUES ($1::bigint "//object id from create_object
                    " , $2::integer, $3::text)"
                    , Database::query_param_list(output.object_id)(registrar_id)(authinfo_));

            output.history_id = Fred::InsertHistory(logd_request_id_, output.object_id).exec(ctx);

            //object_registry historyid
            Database::Result update_historyid_res = ctx.get_conn().exec_params(
                "UPDATE object_registry SET historyid = $1::bigint, crhistoryid = $1::bigint "
                    " WHERE id = $2::integer RETURNING id"
                    , Database::query_param_list(output.history_id)(output.object_id));
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

        return output;
    }

    std::ostream& operator<<(std::ostream& os, const CreateObject& i)
    {
        os << "#CreateObject object_type: " << i.object_type_
            << " handle: " << i.handle_
            << " registrar: " << i.registrar_
            << " authinfo: " << i.authinfo_.print_quoted()
            << " logd_request_id: " << i.logd_request_id_.print_quoted()
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
        , const Optional<std::string>& sponsoring_registrar
        , const Optional<std::string>& authinfo
        , const Nullable<unsigned long long>& logd_request_id)
    : handle_(handle)
    , obj_type_(obj_type)
    , registrar_(registrar)
    , sponsoring_registrar_(sponsoring_registrar)
    , authinfo_(authinfo)
    , logd_request_id_(logd_request_id)
    {}

    UpdateObject& UpdateObject::set_sponsoring_registrar(const std::string& sponsoring_registrar)
    {
        sponsoring_registrar_ = sponsoring_registrar;
        return *this;
    }

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
                " ELSE UPPER($1::text) END AND oreg.erdate IS NULL "
                " FOR UPDATE OF oreg"
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

            //check sponsoring registrar
            unsigned long long sponsoring_registrar_id = 0;
            if(sponsoring_registrar_.isset())
            {
                Database::Result registrar_res = ctx.get_conn().exec_params(
                    "SELECT id FROM registrar WHERE handle = UPPER($1::text) FOR SHARE"
                    , Database::query_param_list(sponsoring_registrar_));
                if(registrar_res.size() == 0)
                {
                    BOOST_THROW_EXCEPTION(Exception().set_unknown_sponsoring_registrar_handle(sponsoring_registrar_));
                }
                if (registrar_res.size() != 1)
                {
                    BOOST_THROW_EXCEPTION(InternalError("failed to get registrar"));
                }
                sponsoring_registrar_id = static_cast<unsigned long long>(registrar_res[0][0]);
            }


            Database::QueryParams params;//query params
            std::stringstream sql;
            params.push_back(registrar_id);
            sql <<"UPDATE object SET update = now() "
                ", upid = $"
                << params.size() << "::integer " ; //registrar from epp-session container by client_id from epp-params

            if(sponsoring_registrar_.isset())
            {
                params.push_back(sponsoring_registrar_id);
                sql << " , clid = $" << params.size() << "::integer ";//set sponsoring registrar
            }

            if(authinfo_.isset())
            {
                params.push_back(authinfo_);
                sql << " , authinfopw = $" << params.size() << "::text ";//set authinfo
            }

            params.push_back(object_id);
            sql <<" WHERE id = $" << params.size() << "::integer ";

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

    std::ostream& operator<<(std::ostream& os, const UpdateObject& i)
    {
        os << "#UpdateObject obj_type: " << i.obj_type_
            << " handle: " << i.handle_
            << " registrar: " << i.registrar_
            << " sponsoring registrar: " << i.sponsoring_registrar_.print_quoted()
            << " authinfo: " << i.authinfo_.print_quoted()
            << " logd_request_id: " << i.logd_request_id_.print_quoted()
            ;
        return os;
    }

    std::string UpdateObject::to_string()
    {
        std::stringstream ss;
        ss << *this;
        return ss.str();
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

    std::ostream& operator<<(std::ostream& os, const InsertHistory& i)
    {
        os << "#InsertHistory logd_request_id: " << i.logd_request_id_.print_quoted()
            << " object_id: " << i.object_id_
            ;
        return os;
    }

    std::string InsertHistory::to_string()
    {
        std::stringstream ss;
        ss << *this;
        return ss.str();
    }

    DeleteObject::DeleteObject(const std::string& handle
        , const std::string& obj_type)
    : handle_(handle)
    , obj_type_(obj_type)
    {}

    void DeleteObject::exec(OperationContext& ctx)
    {
        try
        {
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
                " ELSE UPPER($1::text) END AND oreg.erdate IS NULL "
                " FOR UPDATE OF oreg"
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

            Database::Result update_erdate_res = ctx.get_conn().exec_params(
                "UPDATE object_registry SET erdate = now() "
                " WHERE id = $1::integer RETURNING id"
                , Database::query_param_list(object_id));
            if (update_erdate_res.size() != 1)
            {
                BOOST_THROW_EXCEPTION(Fred::InternalError("erdate update failed"));
            }

            Database::Result delete_object_res = ctx.get_conn().exec_params(
                "DELETE FROM object WHERE id = $1::integer RETURNING id"
                    , Database::query_param_list(object_id));
            if (delete_object_res.size() != 1)
            {
                BOOST_THROW_EXCEPTION(Fred::InternalError("delete object failed"));
            }

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
    }

    std::ostream& operator<<(std::ostream& os, const DeleteObject& i)
    {
        os << "#DeleteObject obj_type: " << i.obj_type_
            << " handle: " << i.handle_
            ;
        return os;
    }

    std::string DeleteObject::to_string()
    {
        std::stringstream ss;
        ss << *this;
        return ss.str();
    }

}//namespace Fred
