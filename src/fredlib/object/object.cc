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

namespace Fred
{
    UpdateObject::UpdateObject(const std::string& handle
        , const std::string& registrar)
    : handle_(handle)
    , registrar_(registrar)
    {}

    UpdateObject::UpdateObject(const std::string& handle
        , const std::string& registrar
        , const Optional<std::string>& authinfo)
    : handle_(handle)
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

        Database::QueryParams params;//query params
        std::stringstream sql;
        params.push_back(registrar_);
        sql <<"UPDATE object SET update = now() "
            ", upid = raise_exception_ifnull((SELECT id FROM registrar WHERE UPPER(handle) = UPPER($"
            << params.size() << "::text)),'|| not found:registrar: '||ex_date($"<< params.size() <<"::text)||' |') " ; //registrar from epp-session container by client_id from epp-params

        if(authinfo_.isset())
        {
            params.push_back(authinfo_);
            sql << " , authinfopw = $" << params.size() << "::text ";//set authinfo
        }

        params.push_back(handle_);
        sql <<" WHERE id = raise_exception_ifnull((SELECT id FROM object_registry WHERE UPPER(name) = UPPER($"
                << params.size() << "::text)),'|| not found:handle: '||ex_date($"<< params.size() <<"::text)||' |'); ";//update object_id by handle

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
