/*
 * Copyright (C) 2013  CZ.NIC, z.s.p.o.
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
 *  @file create_keyset.h
 *  create keyset
 */


#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>

#include "fredlib/keyset/create_keyset.h"
#include "fredlib/object/object.h"

#include "fredlib/opcontext.h"
#include "fredlib/db_settings.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"

namespace Fred
{

    CreateKeyset::CreateKeyset(const std::string& handle
                , const std::string& registrar)
    : handle_(handle)
    , registrar_(registrar)
    {}

    CreateKeyset::CreateKeyset(const std::string& handle
            , const std::string& registrar
            , const Optional<std::string>& authinfo
            , const std::vector<DnsKey>& dns_keys
            , const std::vector<std::string>& tech_contacts
            , const Optional<unsigned long long> logd_request_id
            )
    : handle_(handle)
    , registrar_(registrar)
    , authinfo_(authinfo)
    , dns_keys_(dns_keys)
    , tech_contacts_(tech_contacts)
    , logd_request_id_(logd_request_id.isset()
            ? Nullable<unsigned long long>(logd_request_id.get_value())
            : Nullable<unsigned long long>())//is NULL if not set
    {}

    CreateKeyset& CreateKeyset::set_authinfo(const std::string& authinfo)
    {
        authinfo_ = authinfo;
        return *this;
    }

    CreateKeyset& CreateKeyset::set_dns_keys(const std::vector<DnsKey>& dns_keys)
    {
        dns_keys_ = dns_keys;
        return *this;
    }

    CreateKeyset& CreateKeyset::set_tech_contacts(const std::vector<std::string>& tech_contacts)
    {
        tech_contacts_ = tech_contacts;
        return *this;
    }

    CreateKeyset& CreateKeyset::set_logd_request_id(unsigned long long logd_request_id)
    {
        logd_request_id_ = logd_request_id;
        return *this;
    }

    boost::posix_time::ptime CreateKeyset::exec(OperationContext& ctx, const std::string& returned_timestamp_pg_time_zone_name)
    {
        boost::posix_time::ptime timestamp;

        try
        {
            unsigned long long object_id = CreateObject("keyset", handle_, registrar_, authinfo_).exec(ctx);
            //create keyset
            {
                //insert
                ctx.get_conn().exec_params("INSERT INTO keyset (id) VALUES ($1::integer)"
                        , Database::query_param_list(object_id));

                //set dns keys
                if(!dns_keys_.empty())
                {
                    for(std::vector<DnsKey>::iterator i = dns_keys_.begin(); i != dns_keys_.end(); ++i)
                    {
                        Database::Result dns_keys_res = ctx.get_conn().exec_params(
                            "INSERT INTO dnskey (keysetid, flags, protocol, alg, key) VALUES($1::integer "
                            ", $2::integer, $3::integer, $4::integer, $5::text) RETURNING id"
                            , Database::query_param_list(object_id)(i->get_flags())(i->get_protocol())(i->get_alg())(i->get_key()));
                        if (dns_keys_res.size() != 1)
                        {
                            std::string errmsg("dns keys || invalid:dns key: ");
                            errmsg += boost::replace_all_copy(static_cast<std::string>(*i),"|", "[pipe]");//quote pipes
                            errmsg += " |";
                            throw CKEX(errmsg.c_str());
                        }
                    }//for i
                }//if set dns keys


                //set tech contacts
                if(!tech_contacts_.empty())
                {
                    Database::QueryParams params;//query params
                    std::stringstream sql;

                    params.push_back(object_id);
                    sql << "INSERT INTO keyset_contact_map(keysetid, contactid) "
                            " VALUES ($" << params.size() << "::integer, ";

                    for(std::vector<std::string>::iterator i = tech_contacts_.begin(); i != tech_contacts_.end(); ++i)
                    {
                        //lock object_registry row for update
                        {
                            Database::Result lock_res = ctx.get_conn().exec_params(
                                "SELECT oreg.id FROM enum_object_type eot"
                                " JOIN object_registry oreg ON oreg.type = eot.id "
                                " AND oreg.name = UPPER($1::text) AND oreg.erdate IS NULL "
                                " WHERE eot.name = 'contact' FOR UPDATE OF oreg"
                                , Database::query_param_list(*i));

                            if (lock_res.size() != 1)
                            {
                                std::string errmsg("unable to lock || not found:tech contact: ");
                                errmsg += boost::replace_all_copy(*i,"|", "[pipe]");//quote pipes
                                errmsg += " |";
                                throw CKEX(errmsg.c_str());
                            }
                        }

                        Database::QueryParams params_i = params;//query params
                        std::stringstream sql_i;
                        sql_i << sql.str();

                        params_i.push_back(*i);

                        {//precheck uniqueness
                            Database::Result keyset_res = ctx.get_conn().exec_params(
                            "SELECT keysetid, contactid FROM keyset_contact_map "
                            " WHERE keysetid = $1::bigint "
                            "  AND contactid = raise_exception_ifnull("
                            "    (SELECT oreg.id FROM object_registry oreg "
                            "       JOIN contact c ON oreg.id = c.id "
                            "     WHERE oreg.name = UPPER($2::text) AND oreg.erdate IS NULL) "
                            "     ,'|| not found:tech contact: '||ex_data($2::text)||' |')"
                            , params_i);

                            if (keyset_res.size() == 1)
                            {
                                std::string errmsg("tech contact already set || already set:tech contact: ");
                                errmsg += boost::replace_all_copy(*i,"|", "[pipe]");//quote pipes
                                errmsg += " |";
                                throw CKEX(errmsg.c_str());
                            }
                        }

                        sql_i << " raise_exception_ifnull("
                            " (SELECT oreg.id FROM object_registry oreg JOIN contact c ON oreg.id = c.id "
                            " WHERE oreg.name = UPPER($"<< params_i.size() << "::text) AND oreg.erdate IS NULL) "
                            " ,'|| not found:tech contact: '||ex_data($"<< params.size() << "::text)||' |')) "
                            " RETURNING keysetid";

                        Database::Result keyset_add_check_res = ctx.get_conn().exec_params(sql_i.str(), params_i);
                        if (keyset_add_check_res.size() != 1)
                        {
                            std::string errmsg("add tech contact failed || invalid:handle: ");
                            errmsg += boost::replace_all_copy(handle_,"|", "[pipe]");//quote pipes
                            errmsg += " | invalid:tech contact: ";
                            errmsg += boost::replace_all_copy(*i,"|", "[pipe]");//quote pipes
                            errmsg += " |";
                            throw CKEX(errmsg.c_str());
                        }
                    }//for i
                }//if set tech contacts


                //get crdate from object_registry
                {
                    Database::Result crdate_res = ctx.get_conn().exec_params(
                            "SELECT crdate::timestamp AT TIME ZONE 'UTC' AT TIME ZONE $1::text "
                            "  FROM object_registry "
                            " WHERE id = $2::bigint"
                        , Database::query_param_list(returned_timestamp_pg_time_zone_name)(object_id));

                    if (crdate_res.size() != 1)
                    {
                        std::string errmsg("|| not found crdate:handle: ");
                        errmsg += boost::replace_all_copy(handle_,"|", "[pipe]");//quote pipes
                        errmsg += " |";
                        throw CKEX(errmsg.c_str());
                    }

                    timestamp = boost::posix_time::time_from_string(std::string(crdate_res[0][0]));
                }
            }

            //save history
            {
                unsigned long long history_id = Fred::InsertHistory(logd_request_id_).exec(ctx);

                //object_history
                ctx.get_conn().exec_params(
                    "INSERT INTO object_history(historyid,id,clid, upid, trdate, update, authinfopw) "
                    " SELECT $1::bigint, id,clid, upid, trdate, update, authinfopw FROM object "
                    " WHERE id = $2::integer"
                    , Database::query_param_list(history_id)(object_id));

                //object_registry historyid
                ctx.get_conn().exec_params(
                    "UPDATE object_registry SET historyid = $1::bigint, crhistoryid = $1::bigint  "
                        " WHERE id = $2::integer"
                        , Database::query_param_list(history_id)(object_id));

                //keyset_history
                ctx.get_conn().exec_params(
                    "INSERT INTO keyset_history(historyid,id) "
                    " SELECT $1::bigint, id FROM keyset "
                    " WHERE id = $2::integer"
                    , Database::query_param_list(history_id)(object_id));

                //dsrecord_history
                ctx.get_conn().exec_params(
                    "INSERT INTO dsrecord_history(historyid, id, keysetid, keytag, alg, digesttype, digest, maxsiglife) "
                    " SELECT $1::bigint, id, keysetid, keytag, alg, digesttype, digest, maxsiglife FROM dsrecord "
                    " WHERE keysetid = $2::integer"
                    , Database::query_param_list(history_id)(object_id));

                //dnskey_history
                ctx.get_conn().exec_params(
                    "INSERT INTO dnskey_history(historyid, id, keysetid, flags, protocol, alg, key) "
                    " SELECT $1::bigint, id, keysetid, flags, protocol, alg, key FROM dnskey "
                    " WHERE keysetid = $2::integer"
                    , Database::query_param_list(history_id)(object_id));

                //keyset_contact_map_history
                ctx.get_conn().exec_params(
                    "INSERT INTO keyset_contact_map_history(historyid,keysetid, contactid) "
                    " SELECT $1::bigint, keysetid, contactid FROM keyset_contact_map "
                        " WHERE keysetid = $2::integer"
                        , Database::query_param_list(history_id)(object_id));

            }//save history


        }//try
        catch(...)//common exception processing
        {
            handleOperationExceptions<CreateKeysetException>(__FILE__, __LINE__, __ASSERT_FUNCTION);
        }

        return timestamp;
    }

}//namespace Fred

