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
 *  @file
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
            //check registrar
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
            }

            CreateObjectOutput create_object_output = CreateObject("keyset", handle_, registrar_, authinfo_, logd_request_id_).exec(ctx);

            Exception create_keyset_exception;

            //create keyset
            {
                //insert
                ctx.get_conn().exec_params("INSERT INTO keyset (id) VALUES ($1::integer)"
                        , Database::query_param_list(create_object_output.object_id));

                //set dns keys
                if(!dns_keys_.empty())
                {
                    for(std::vector<DnsKey>::iterator i = dns_keys_.begin(); i != dns_keys_.end(); ++i)
                    {
                        try
                        {
                            ctx.get_conn().exec("SAVEPOINT dnskey");
                            ctx.get_conn().exec_params(
                            "INSERT INTO dnskey (keysetid, flags, protocol, alg, key) VALUES($1::integer "
                            ", $2::integer, $3::integer, $4::integer, $5::text)"
                            , Database::query_param_list(create_object_output.object_id)(i->get_flags())(i->get_protocol())(i->get_alg())(i->get_key()));
                            ctx.get_conn().exec("RELEASE SAVEPOINT dnskey");
                        }
                        catch(const std::exception& ex)
                        {
                            std::string what_string(ex.what());
                            if(what_string.find("dnskey_unique_key") != std::string::npos)
                            {
                                create_keyset_exception.add_already_set_dns_key(*i);
                                ctx.get_conn().exec("ROLLBACK TO SAVEPOINT dnskey");
                            }
                            else
                                throw;
                        }
                    }//for i
                }//if set dns keys


                //set tech contacts
                if(!tech_contacts_.empty())
                {
                    Database::QueryParams params;//query params
                    std::stringstream sql;

                    params.push_back(create_object_output.object_id);
                    sql << "INSERT INTO keyset_contact_map(keysetid, contactid) "
                            " VALUES ($" << params.size() << "::integer, ";

                    for(std::vector<std::string>::iterator i = tech_contacts_.begin(); i != tech_contacts_.end(); ++i)
                    {
                        //lock object_registry row for update and get id
                        unsigned long long tech_contact_id = 0;
                        {
                            Database::Result lock_res = ctx.get_conn().exec_params(
                                "SELECT oreg.id FROM enum_object_type eot"
                                " JOIN object_registry oreg ON oreg.type = eot.id "
                                " JOIN contact c ON oreg.id = c.id "
                                " AND oreg.name = UPPER($1::text) AND oreg.erdate IS NULL "
                                " WHERE eot.name = 'contact' FOR UPDATE OF oreg"
                                , Database::query_param_list(*i));

                            if (lock_res.size() == 0)
                            {
                                create_keyset_exception.add_unknown_technical_contact_handle(*i);
                                continue;//for tech_contacts_
                            }
                            if (lock_res.size() != 1)
                            {
                                BOOST_THROW_EXCEPTION(InternalError("failed to get technical contact"));
                            }
                            tech_contact_id = static_cast<unsigned long long> (lock_res[0][0]);
                        }

                        Database::QueryParams params_i = params;//query params
                        std::stringstream sql_i;
                        sql_i << sql.str();

                        params_i.push_back(tech_contact_id);
                        sql_i << " $" << params_i.size() << "::integer) ";

                        try
                        {
                            ctx.get_conn().exec("SAVEPOINT tech_contact");
                            ctx.get_conn().exec_params(sql_i.str(), params_i);
                            ctx.get_conn().exec("RELEASE SAVEPOINT tech_contact");
                        }
                        catch(const std::exception& ex)
                        {
                            std::string what_string(ex.what());
                            if(what_string.find("keyset_contact_map_pkey") != std::string::npos)
                            {
                                create_keyset_exception.add_already_set_technical_contact_handle(*i);
                                ctx.get_conn().exec("ROLLBACK TO SAVEPOINT tech_contact");
                            }
                            else
                                throw;
                        }
                    }//for i
                }//if set tech contacts


                //get crdate from object_registry
                {
                    Database::Result crdate_res = ctx.get_conn().exec_params(
                            "SELECT crdate::timestamp AT TIME ZONE 'UTC' AT TIME ZONE $1::text "
                            "  FROM object_registry "
                            " WHERE id = $2::bigint"
                        , Database::query_param_list(returned_timestamp_pg_time_zone_name)(create_object_output.object_id));

                    if (crdate_res.size() != 1)
                    {
                        BOOST_THROW_EXCEPTION(Fred::InternalError("timestamp of the keyset creation was not found"));
                    }

                    timestamp = boost::posix_time::time_from_string(std::string(crdate_res[0][0]));
                }
            }

            //check exception
            if(create_keyset_exception.throw_me())
                BOOST_THROW_EXCEPTION(create_keyset_exception);

            //save history
            {
                //keyset_history
                ctx.get_conn().exec_params(
                    "INSERT INTO keyset_history(historyid,id) "
                    " SELECT $1::bigint, id FROM keyset "
                    " WHERE id = $2::integer"
                    , Database::query_param_list(create_object_output.history_id)(create_object_output.object_id));

                //dsrecord_history
                ctx.get_conn().exec_params(
                    "INSERT INTO dsrecord_history(historyid, id, keysetid, keytag, alg, digesttype, digest, maxsiglife) "
                    " SELECT $1::bigint, id, keysetid, keytag, alg, digesttype, digest, maxsiglife FROM dsrecord "
                    " WHERE keysetid = $2::integer"
                    , Database::query_param_list(create_object_output.history_id)(create_object_output.object_id));

                //dnskey_history
                ctx.get_conn().exec_params(
                    "INSERT INTO dnskey_history(historyid, id, keysetid, flags, protocol, alg, key) "
                    " SELECT $1::bigint, id, keysetid, flags, protocol, alg, key FROM dnskey "
                    " WHERE keysetid = $2::integer"
                    , Database::query_param_list(create_object_output.history_id)(create_object_output.object_id));

                //keyset_contact_map_history
                ctx.get_conn().exec_params(
                    "INSERT INTO keyset_contact_map_history(historyid,keysetid, contactid) "
                    " SELECT $1::bigint, keysetid, contactid FROM keyset_contact_map "
                        " WHERE keysetid = $2::integer"
                        , Database::query_param_list(create_object_output.history_id)(create_object_output.object_id));

            }//save history

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }

        return timestamp;
    }

    std::string CreateKeyset::to_string() const
    {
        return Util::format_operation_state("CreateKeyset",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("handle",handle_))
        (std::make_pair("registrar",registrar_))
        (std::make_pair("authinfo",authinfo_.print_quoted()))
        (std::make_pair("dns_keys", Util::format_vector(dns_keys_)))
        (std::make_pair("tech_contacts",Util::format_vector(tech_contacts_)))
        (std::make_pair("logd_request_id",logd_request_id_.print_quoted()))
        );
    }

}//namespace Fred

