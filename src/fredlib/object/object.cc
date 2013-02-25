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

            Database::Result id_res = ctx.get_conn().exec_params(
                "SELECT create_object(raise_exception_ifnull( "
                    " (SELECT id FROM registrar WHERE handle = UPPER($1::text)) "
                    " ,'|| not found:registrar: '||ex_data($1::text)||' |') "//registrar handle
                    " , $2::text "//object handle
                    " , raise_exception_ifnull( "
                    " (SELECT id FROM enum_object_type WHERE name = $3::text) "
                    " ,'|| not found:object type: '||ex_data($3::text)||' |') )"//object type
                        , Database::query_param_list(registrar_)(handle_)(object_type_));

            if (id_res.size() != 1)
            {
                throw COERR("unable to call create_object");
            }

            object_id = id_res[0][0];

            if (object_id == 0)
            {
                std::string errmsg("unable to create object || invalid:handle: ");
                errmsg += boost::replace_all_copy(handle_,"|", "[pipe]");//quote pipes
                errmsg += " |";
                throw COEX(errmsg.c_str());
            }

            if(authinfo_.get_value().empty())
            {
                authinfo_ = RandomDataGenerator().xnstring(8);//former PASS_LEN
            }

            ctx.get_conn().exec_params("INSERT INTO object(id, clid, authinfopw) VALUES ($1::bigint "//object id from create_object
                    " , raise_exception_ifnull( "
                    " (SELECT id FROM registrar WHERE handle = UPPER($2::text)) "
                    " ,'|| not found:registrar: '||ex_data($2::text)||' |') "//registrar handle
                    " , $3::text)"
                    , Database::query_param_list(object_id)(registrar_)(authinfo_));

        }//try
        catch(...)//common exception processing
        {
            handleOperationExceptions<CreateObjectException>(__FILE__, __LINE__, __ASSERT_FUNCTION);
        }

        return object_id;
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

            unsigned long long object_id = 0;

            {
                Database::Result object_id_res = ctx.get_conn().exec_params(
                "SELECT oreg.id FROM object_registry oreg "
                " JOIN enum_object_type eot ON eot.id = oreg.type AND eot.name = $2::text "
                " WHERE oreg.name = CASE WHEN $2::text = 'domain'::text THEN LOWER($1::text) "
                " ELSE UPPER($1::text) END AND oreg.erdate IS NULL"
                , Database::query_param_list(handle_)(obj_type_));

                if (object_id_res.size() != 1)
                {
                    std::string errmsg("object id || not found:handle: ");
                    errmsg += boost::replace_all_copy(handle_,"|", "[pipe]");//quote pipes
                    errmsg += " | not found:object type: ";
                    errmsg += boost::replace_all_copy(obj_type_,"|", "[pipe]");//quote pipes
                    errmsg += " |";
                    throw UOEX(errmsg.c_str());
                }

                object_id = static_cast<unsigned long long> (object_id_res[0][0]);
            }

        Database::QueryParams params;//query params
        std::stringstream sql;
        params.push_back(registrar_);
        sql <<"UPDATE object SET update = now() "
            ", upid = raise_exception_ifnull((SELECT id FROM registrar WHERE handle = UPPER($"
            << params.size() << "::text)),'|| not found:registrar: '||ex_data($"<< params.size() <<"::text)||' |') " ; //registrar from epp-session container by client_id from epp-params

        if(authinfo_.isset())
        {
            params.push_back(authinfo_);
            sql << " , authinfopw = $" << params.size() << "::text ";//set authinfo
        }

        params.push_back(object_id);
        sql <<" WHERE id = $" << params.size() << "::integer ";

        ctx.get_conn().exec_params(sql.str(), params);

        }//try
        catch(...)//common exception processing
        {
            handleOperationExceptions<UpdateObjectException>(__FILE__, __LINE__, __ASSERT_FUNCTION);
        }
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
            throw IHERR("unable to get history_id");
        }

        history_id = history_id_res[0][0];

        }//try
        catch(...)//common exception processing
        {
            handleOperationExceptions<InsertHistoryException>(__FILE__, __LINE__, __ASSERT_FUNCTION);
        }

        return history_id;
    }

}//namespace Fred
