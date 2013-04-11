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
 *  @file update_keyset.cc
 *  keyset update
 */

#include <string>
#include <vector>
#include <algorithm>

#include "fredlib/keyset/update_keyset.h"
#include "fredlib/object/object.h"

#include "fredlib/opcontext.h"
#include "fredlib/db_settings.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/log/log.h"

namespace Fred
{
    UpdateKeyset::UpdateKeyset(const std::string& handle
            , const std::string& registrar)
    : handle_(handle)
    , registrar_(registrar)
    {}

    UpdateKeyset::UpdateKeyset(const std::string& handle
            , const std::string& registrar
            , const Optional<std::string>& authinfo
            , const std::vector<std::string>& add_tech_contact
            , const std::vector<std::string>& rem_tech_contact
            , const std::vector<DnsKey>& add_dns_key
            , const std::vector<DnsKey>& rem_dns_key
            , const Optional<unsigned long long> logd_request_id
            )
    : handle_(handle)
    , registrar_(registrar)
    , authinfo_(authinfo)
    , add_tech_contact_(add_tech_contact)
    , rem_tech_contact_(rem_tech_contact)
    , add_dns_key_(add_dns_key)
    , rem_dns_key_(rem_dns_key)
    , logd_request_id_(logd_request_id.isset()
        ? Nullable<unsigned long long>(logd_request_id.get_value())
        : Nullable<unsigned long long>())//is NULL if not set
    {}

    UpdateKeyset& UpdateKeyset::set_authinfo(const std::string& authinfo)
    {
        authinfo_ = authinfo;
        return *this;
    }

    UpdateKeyset& UpdateKeyset::add_tech_contact(const std::string& tech_contact)
    {
        add_tech_contact_.push_back(tech_contact);
        return *this;
    }

    UpdateKeyset& UpdateKeyset::rem_tech_contact(const std::string& tech_contact)
    {
        rem_tech_contact_.push_back(tech_contact);
        return *this;
    }

    UpdateKeyset& UpdateKeyset::add_dns_key(const DnsKey& dns_key)
    {
        add_dns_key_.push_back(dns_key);
        return *this;
    }

    UpdateKeyset& UpdateKeyset::rem_dns_key(const DnsKey& dns_key)
    {
        rem_dns_key_.push_back(dns_key);
        return *this;
    }

    UpdateKeyset& UpdateKeyset::set_logd_request_id(unsigned long long logd_request_id)
    {
        logd_request_id_ = logd_request_id;
        return *this;
    }

    unsigned long long UpdateKeyset::exec(OperationContext& ctx)
    {
        unsigned long long history_id = 0;

        try
        {

        //lock object_registry row for update
        {
            Database::Result lock_res = ctx.get_conn().exec_params(
                "SELECT oreg.id FROM enum_object_type eot"
                " JOIN object_registry oreg ON oreg.type = eot.id "
                " AND oreg.name = UPPER($1::text) AND oreg.erdate IS NULL "
                " WHERE eot.name = 'keyset' FOR UPDATE OF oreg"
                , Database::query_param_list(handle_));

            if (lock_res.size() != 1)
            {
                std::string errmsg("unable to lock || not found:handle: ");
                errmsg += boost::replace_all_copy(handle_,"|", "[pipe]");//quote pipes
                errmsg += " |";
                throw UKEX(errmsg.c_str());
            }
        }

        //get keyset_id
        unsigned long long keyset_id =0;
        {
            Database::Result keyset_id_res = ctx.get_conn().exec_params(
                "SELECT oreg.id FROM keyset k "
                " JOIN object_registry oreg ON k.id = oreg.id "
                " WHERE oreg.name = UPPER($1::text) AND oreg.erdate IS NULL"
                , Database::query_param_list(handle_));

            if (keyset_id_res.size() != 1)
            {
                std::string errmsg("|| not found:handle: ");
                errmsg += boost::replace_all_copy(handle_,"|", "[pipe]");//quote pipes
                errmsg += " |";
                throw UKEX(errmsg.c_str());
            }

            keyset_id = keyset_id_res[0][0];
        }

        Fred::UpdateObject(handle_,"keyset", registrar_, authinfo_).exec(ctx);

        //add tech contacts
        if(!add_tech_contact_.empty())
        {
            Database::QueryParams params;//query params
            std::stringstream sql;

            params.push_back(keyset_id);
            sql << "INSERT INTO keyset_contact_map(keysetid, contactid) "
                    " VALUES ($" << params.size() << "::integer, ";

            for(std::vector<std::string>::iterator i = add_tech_contact_.begin(); i != add_tech_contact_.end(); ++i)
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
                        throw UKEX(errmsg.c_str());
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
                        throw UKEX(errmsg.c_str());
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
                    std::string errmsg("add tech contact failed || invalid:tech contact: ");
                    errmsg += boost::replace_all_copy(*i,"|", "[pipe]");//quote pipes
                    errmsg += " |";
                    throw UKEX(errmsg.c_str());
                }
            }//for i
        }//if add tech contacts

        //delete tech contacts
        if(!rem_tech_contact_.empty())
        {
            Database::QueryParams params;//query params
            std::stringstream sql;

            params.push_back(keyset_id);
            sql << "DELETE FROM keyset_contact_map WHERE keysetid = "
                    " $" << params.size() << "::integer AND ";

            for(std::vector<std::string>::iterator i = rem_tech_contact_.begin(); i != rem_tech_contact_.end(); ++i)
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
                        throw UKEX(errmsg.c_str());
                    }
                }

                Database::QueryParams params_i = params;//query params
                std::stringstream sql_i;
                sql_i << sql.str();

                params_i.push_back(*i);
                sql_i << "contactid = raise_exception_ifnull( "
                    " (SELECT oreg.id FROM object_registry oreg "
                    " JOIN contact c ON oreg.id = c.id WHERE oreg.name = UPPER($"<< params_i.size() << "::text) AND oreg.erdate IS NULL) "
                    " ,'|| not found:tech contact: '||ex_data($"<< params.size() << "::text)||' |') "
                    " RETURNING keysetid";
                Database::Result keyset_del_res = ctx.get_conn().exec_params(sql_i.str(), params_i);
                if (keyset_del_res.size() != 1)
                {
                    std::string errmsg("delete tech contact failed || invalid:tech contact: ");
                    errmsg += boost::replace_all_copy(*i,"|", "[pipe]");//quote pipes
                    errmsg += " |";
                    throw UKEX(errmsg.c_str());
                }
            }//for i
        }//if delete tech contacts

        //delete dns keys - before adding new ones
        if(!rem_dns_key_.empty())
        {
            for(std::vector<DnsKey>::iterator i = rem_dns_key_.begin(); i != rem_dns_key_.end(); ++i)
            {
                Database::Result rem_dns_key_res = ctx.get_conn().exec_params(
                    "DELETE FROM dnskey WHERE keysetid = $1::integer "
                    " AND flags = $2::integer AND protocol = $3::integer AND alg = $4::integer AND key = $5::text "
                    " RETURNING id"
                    , Database::query_param_list(keyset_id)(i->get_flags())(i->get_protocol())(i->get_alg())(i->get_key()));
                if (rem_dns_key_res.size() != 1)
                {
                    std::string errmsg("delete dns keys || not found:dns key: ");
                    errmsg += boost::replace_all_copy(static_cast<std::string>(*i),"|", "[pipe]");//quote pipes
                    errmsg += " |";
                    throw UKEX(errmsg.c_str());
                }
            }//for i
        }//if delete dns keys

        //add dns keys
        if(!add_dns_key_.empty())
        {
            for(std::vector<DnsKey>::iterator i = add_dns_key_.begin(); i != add_dns_key_.end(); ++i)
            {
                try
                {
                    ctx.get_conn().exec_params(
                    "INSERT INTO dnskey (keysetid, flags, protocol, alg, key) VALUES($1::integer "
                    ", $2::integer, $3::integer, $4::integer, $5::text)"
                    , Database::query_param_list(keyset_id)(i->get_flags())(i->get_protocol())(i->get_alg())(i->get_key()));
                }
                catch(Database::ResultFailed& ex)
                {
                    std::string errmsg = ex.what();
                    errmsg += " add dns keys || invalid:dns key: ";
                    errmsg += boost::replace_all_copy(static_cast<std::string>(*i),"|", "[pipe]");//quote pipes
                    errmsg += " |";
                    throw UKEX(errmsg.c_str());
                }

            }//for i
        }//if add dns keys

        //save history
        {
            history_id = Fred::InsertHistory(logd_request_id_).exec(ctx);

            //object_history
            ctx.get_conn().exec_params(
                "INSERT INTO object_history(historyid,id,clid, upid, trdate, update, authinfopw) "
                " SELECT $1::bigint, id,clid, upid, trdate, update, authinfopw FROM object "
                " WHERE id = $2::integer"
                , Database::query_param_list(history_id)(keyset_id));

            //object_registry historyid
            ctx.get_conn().exec_params(
                "UPDATE object_registry SET historyid = $1::bigint "
                    " WHERE id = $2::integer"
                    , Database::query_param_list(history_id)(keyset_id));

            //keyset_history
            ctx.get_conn().exec_params(
                "INSERT INTO keyset_history(historyid,id) "
                " SELECT $1::bigint, id FROM keyset "
                " WHERE id = $2::integer"
                , Database::query_param_list(history_id)(keyset_id));

            //dsrecord_history
            ctx.get_conn().exec_params(
                "INSERT INTO dsrecord_history(historyid, id, keysetid, keytag, alg, digesttype, digest, maxsiglife) "
                " SELECT $1::bigint, id, keysetid, keytag, alg, digesttype, digest, maxsiglife FROM dsrecord "
                " WHERE keysetid = $2::integer"
                , Database::query_param_list(history_id)(keyset_id));

            //dnskey_history
            ctx.get_conn().exec_params(
                "INSERT INTO dnskey_history(historyid, id, keysetid, flags, protocol, alg, key) "
                " SELECT $1::bigint, id, keysetid, flags, protocol, alg, key FROM dnskey "
                " WHERE keysetid = $2::integer"
                , Database::query_param_list(history_id)(keyset_id));

            //keyset_contact_map_history
            ctx.get_conn().exec_params(
                "INSERT INTO keyset_contact_map_history(historyid,keysetid, contactid) "
                " SELECT $1::bigint, keysetid, contactid FROM keyset_contact_map "
                    " WHERE keysetid = $2::integer"
                    , Database::query_param_list(history_id)(keyset_id));
        }//save history

        }//try
        catch(...)//common exception processing
        {
            handleOperationExceptions<UpdateKeysetException>(__FILE__, __LINE__, __ASSERT_FUNCTION);
        }
        return history_id;
    }//UpdateKeyset::exec

}//namespace Fred
