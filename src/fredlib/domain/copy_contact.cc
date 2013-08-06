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
 *  @file copy_contact.cc
 *  copy contact
 */

#include "fredlib/domain/copy_contact.h"
#include "fredlib/domain/get_blocking_status_desc_list.h"
#include "fredlib/domain/get_object_state_id_map.h"
#include "fredlib/opcontext.h"
#include "fredlib/db_settings.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"
#include "fredlib/object.h"

#include <boost/algorithm/string.hpp>

namespace Fred
{
    CopyContact::CopyContact(const std::string &_src_contact_handle,
        const std::string &_dst_contact_handle,
        RequestId _request_id)
    :   src_contact_handle_(_src_contact_handle),
        dst_contact_handle_(_dst_contact_handle),
        request_id_(_request_id)
    {}

    ObjectId CopyContact::exec(OperationContext &_ctx)
    {
        enum ColumnIdx
        {
            COL_OBJ_REG_ID = 0,
            COL_OBJ_REG_CRID = 1,
        };
        Database::Result src_obj_info_res = _ctx.get_conn().exec_params(
            "SELECT id,crid "
            "FROM object_registry "
            "WHERE type=$1::integer AND "
                  "name=UPPER($2::text) AND "
                  "erdate IS NULL "
            "FOR UPDATE",
            Database::query_param_list
                (OBJECT_TYPE_ID_CONTACT)(src_contact_handle_));

        if (src_obj_info_res.size() != 1) {
            BOOST_THROW_EXCEPTION(Exception().set_src_contact_handle_not_found(src_contact_handle_));
        }
        const ObjectId src_object_id   = src_obj_info_res[0][COL_OBJ_REG_ID];
        const ObjectId src_object_crid = src_obj_info_res[0][COL_OBJ_REG_CRID];

        Database::Result roreg = _ctx.get_conn().exec_params(
            "SELECT create_object($1::integer,$2::text, $3::integer)",
            Database::query_param_list(src_object_crid)(dst_contact_handle_)(OBJECT_TYPE_ID_CONTACT));
        if (roreg.size() != 1) {
            BOOST_THROW_EXCEPTION(Exception().set_create_contact_failed(dst_contact_handle_));
        }
        const ObjectId dst_object_id = static_cast< ObjectId >(roreg[0][0]);
        if (dst_object_id == 0) {
            BOOST_THROW_EXCEPTION(Exception().set_dst_contact_handle_already_exist(dst_contact_handle_));
        }
        /* object record */
        _ctx.get_conn().exec_params(
            "INSERT INTO object (id,clid,authinfopw) VALUES ("
                "$1::integer,"
                "(SELECT clid FROM object WHERE id=$2::integer),"
                "(SELECT authinfopw FROM object WHERE id=$2::integer))",
            Database::query_param_list(dst_object_id)
                (src_object_id));
        /* contact record */
        _ctx.get_conn().exec_params(
            "INSERT INTO contact ("
                "id,name,organization,street1,street2,street3,city,stateorprovince,postalcode,country,"
                "telephone,fax,email,disclosename,discloseorganization,discloseaddress,disclosetelephone,disclosefax,"
                "discloseemail,notifyemail,vat,ssn,ssntype,disclosevat,discloseident,disclosenotifyemail) "
            "SELECT "
                "$1::integer,name,organization,street1,street2,street3,city,stateorprovince,postalcode,country,"
                "telephone,fax,email,disclosename,discloseorganization,discloseaddress,disclosetelephone,disclosefax,"
                "discloseemail,notifyemail,vat,ssn,ssntype,disclosevat,discloseident,disclosenotifyemail "
            "FROM contact WHERE id=$2::integer",
            Database::query_param_list(dst_object_id)
                (src_object_id));

        _ctx.get_conn().exec_params(
            "INSERT INTO history (request_id) VALUES ($1::bigint)",
            Database::query_param_list(request_id_));
        Database::Result rhistory = _ctx.get_conn().exec("SELECT currval('history_id_seq')");
        const unsigned long long history_id = rhistory[0][0];
        if (history_id == 0) {
            throw std::runtime_error("cannot save new history");
        }

        _ctx.get_conn().exec_params(
            "UPDATE object_registry SET historyid=$1::integer "
                "WHERE id=$2::integer",
            Database::query_param_list(history_id)(dst_object_id));

        _ctx.get_conn().exec_params(
            "INSERT INTO object_history (historyid,id,clid,upid,trdate,update,authinfopw) "
            "SELECT $1::integer,o.id,o.clid,o.upid,o.trdate,o.update,o.authinfopw "
            "FROM object o "
            "WHERE o.id=$2::integer",
            Database::query_param_list(history_id)(dst_object_id));

        _ctx.get_conn().exec_params(
            "INSERT INTO contact_history ("
                "historyid,id,name,organization,street1,street2,street3,"
                "city,stateorprovince,postalcode,country,telephone,fax,email,disclosename,"
                "discloseorganization,discloseaddress,disclosetelephone,disclosefax,discloseemail,"
                "notifyemail,vat,ssn,ssntype,disclosevat,discloseident,disclosenotifyemail) "
            "SELECT "
                "$1::integer,id,name,organization,street1,street2,street3,"
                "city,stateorprovince,postalcode,country,telephone,fax,email,disclosename,"
                "discloseorganization,discloseaddress,disclosetelephone,disclosefax,discloseemail,"
                "notifyemail,vat,ssn,ssntype,disclosevat,discloseident,disclosenotifyemail "
            "FROM contact "
            "WHERE id=$2::integer",
                Database::query_param_list(history_id)(dst_object_id));

        return dst_object_id;
    }//CopyContact::exec

}//namespace Fred
