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
 *  create nsset
 */


#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>

#include "src/fredlib/nsset/create_nsset.h"
#include "src/fredlib/object/object.h"
#include "src/fredlib/registrar/registrar_impl.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/db_settings.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"

namespace Fred
{

    CreateNsset::CreateNsset(const std::string& handle
                , const std::string& registrar)
    : handle_(handle)
    , registrar_(registrar)
    {}

    CreateNsset::CreateNsset(const std::string& handle
            , const std::string& registrar
            , const Optional<std::string>& authinfo
            , const Optional<short>& tech_check_level
            , const std::vector<DnsHost>& dns_hosts
            , const std::vector<std::string>& tech_contacts
            , const Optional<unsigned long long> logd_request_id
            )
    : handle_(handle)
    , registrar_(registrar)
    , authinfo_(authinfo)
    , tech_check_level_(tech_check_level)
    , dns_hosts_(dns_hosts)
    , tech_contacts_(tech_contacts)
    , logd_request_id_(logd_request_id.isset()
            ? Nullable<unsigned long long>(logd_request_id.get_value())
            : Nullable<unsigned long long>())//is NULL if not set
    {}

    CreateNsset& CreateNsset::set_authinfo(const std::string& authinfo)
    {
        authinfo_ = authinfo;
        return *this;
    }

    CreateNsset& CreateNsset::set_tech_check_level(short tech_check_level)
    {
        tech_check_level_ = tech_check_level;
        return *this;
    }

    CreateNsset& CreateNsset::set_dns_hosts(const std::vector<DnsHost>& dns_hosts)
    {
        dns_hosts_ = dns_hosts;
        return *this;
    }

    CreateNsset& CreateNsset::set_tech_contacts(const std::vector<std::string>& tech_contacts)
    {
        tech_contacts_ = tech_contacts;
        return *this;
    }

    CreateNsset& CreateNsset::set_logd_request_id(unsigned long long logd_request_id)
    {
        logd_request_id_ = logd_request_id;
        return *this;
    }

    boost::posix_time::ptime CreateNsset::exec(OperationContext& ctx, const std::string& returned_timestamp_pg_time_zone_name)
    {
        boost::posix_time::ptime timestamp;

        try
        {
            //check registrar
            Registrar::get_registrar_id_by_handle(
                ctx, registrar_, static_cast<Exception*>(0)//set throw
                , &Exception::set_unknown_registrar_handle);

            CreateObjectOutput create_object_output = CreateObject("nsset", handle_, registrar_, authinfo_, logd_request_id_).exec(ctx);

            Exception create_nsset_exception;

            //create nsset
            {
                Database::QueryParams params;//query params
                std::stringstream col_sql, val_sql;
                Util::HeadSeparator col_separator("",", "), val_separator("",", ");

                col_sql <<"INSERT INTO nsset (";
                val_sql << " VALUES (";

                //id
                params.push_back(create_object_output.object_id);
                col_sql << col_separator.get() << "id";
                val_sql << val_separator.get() << "$" << params.size() <<"::integer";

                if(tech_check_level_.isset())
                {
                    params.push_back(tech_check_level_.get_value());
                    col_sql << col_separator.get() << "checklevel";
                    val_sql << val_separator.get() << "$" << params.size() <<"::smallint";
                }

                col_sql <<")";
                val_sql << ")";
                //insert
                ctx.get_conn().exec_params(col_sql.str() + val_sql.str(), params);

                //set dns hosts
                if(!dns_hosts_.empty())
                {
                    for(std::vector<DnsHost>::iterator i = dns_hosts_.begin(); i != dns_hosts_.end(); ++i)
                    {
                        unsigned long long add_host_id = 0;
                        try
                        {
                            ctx.get_conn().exec("SAVEPOINT dnshost");
                            Database::Result add_host_id_res = ctx.get_conn().exec_params(
                            "INSERT INTO host (nssetid, fqdn) VALUES( "
                            " $1::integer, LOWER($2::text)) RETURNING id"
                            , Database::query_param_list(create_object_output.object_id)(i->get_fqdn()));
                            ctx.get_conn().exec("RELEASE SAVEPOINT dnshost");

                            add_host_id = static_cast<unsigned long long>(add_host_id_res[0][0]);
                        }
                        catch(const std::exception& ex)
                        {
                            std::string what_string(ex.what());
                            if(what_string.find("host_nssetid_fqdn_key") != std::string::npos)
                            {
                                create_nsset_exception.add_already_set_dns_host(i->get_fqdn());
                                ctx.get_conn().exec("ROLLBACK TO SAVEPOINT dnshost");
                            }
                            else
                                throw;
                        }

                        std::vector<std::string> dns_host_ip = i->get_inet_addr();

                        for(std::vector<std::string>::iterator j = dns_host_ip.begin(); j != dns_host_ip.end(); ++j)
                        {
                            try
                            {
                                ctx.get_conn().exec("SAVEPOINT dnshostipaddr");
                                ctx.get_conn().exec_params(
                                "INSERT INTO host_ipaddr_map (hostid, nssetid, ipaddr) "
                                " VALUES($1::integer, $2::integer, $3::inet)"
                                , Database::query_param_list(add_host_id)(create_object_output.object_id)(*j));
                                ctx.get_conn().exec("RELEASE SAVEPOINT dnshostipaddr");
                            }
                            catch(const std::exception& ex)
                            {
                                std::string what_string(ex.what());
                                if(what_string.find("syntax for type inet") != std::string::npos)
                                {
                                    create_nsset_exception.add_invalid_dns_host_ipaddr(*j);
                                    ctx.get_conn().exec("ROLLBACK TO SAVEPOINT dnshostipaddr");
                                }
                                else
                                    throw;
                            }
                        }//for j
                    }//for i
                }//if set dns hosts


                //set tech contacts
                if(!tech_contacts_.empty())
                {
                    Database::QueryParams params;//query params
                    std::stringstream sql;

                    params.push_back(create_object_output.object_id);
                    sql << "INSERT INTO nsset_contact_map(nssetid, contactid) "
                            " VALUES ($" << params.size() << "::integer, ";

                    for(std::vector<std::string>::iterator i = tech_contacts_.begin(); i != tech_contacts_.end(); ++i)
                    {
                        //lock object_registry row for update and get id
                        unsigned long long tech_contact_id = lock_object_by_handle_and_type(
                                ctx,*i,"contact",&create_nsset_exception,
                                &Exception::add_unknown_technical_contact_handle);
                        if(tech_contact_id == 0) continue;

                        Database::QueryParams params_i = params;//query params
                        std::stringstream sql_i;
                        sql_i << sql.str();

                        params_i.push_back(tech_contact_id);
                        sql_i << " $"<< params_i.size() << "::integer )";

                        try
                        {
                            ctx.get_conn().exec("SAVEPOINT tech_contact");
                            ctx.get_conn().exec_params(sql_i.str(), params_i);
                            ctx.get_conn().exec("RELEASE SAVEPOINT tech_contact");
                        }
                        catch(const std::exception& ex)
                        {
                            std::string what_string(ex.what());
                            if(what_string.find("nsset_contact_map_pkey") != std::string::npos)
                            {
                                create_nsset_exception.add_already_set_technical_contact_handle(*i);
                                ctx.get_conn().exec("ROLLBACK TO SAVEPOINT tech_contact");
                            }
                            else
                                throw;
                        }
                    }//for i
                }//if set tech contacts

                //check exception
                if(create_nsset_exception.throw_me())
                    BOOST_THROW_EXCEPTION(create_nsset_exception);

                //get crdate from object_registry
                {
                    Database::Result crdate_res = ctx.get_conn().exec_params(
                            "SELECT crdate::timestamp AT TIME ZONE 'UTC' AT TIME ZONE $1::text "
                            "  FROM object_registry "
                            " WHERE id = $2::bigint"
                        , Database::query_param_list(returned_timestamp_pg_time_zone_name)(create_object_output.object_id));
                    if (crdate_res.size() != 1)
                    {
                        BOOST_THROW_EXCEPTION(Fred::InternalError("timestamp of the nsset creation was not found"));
                    }
                    timestamp = boost::posix_time::time_from_string(std::string(crdate_res[0][0]));
                }
            }

            //save history
            {
                //nsset_history
                ctx.get_conn().exec_params(
                    "INSERT INTO nsset_history(historyid,id,checklevel) "
                    " SELECT $1::bigint, id, checklevel FROM nsset "
                        " WHERE id = $2::integer"
                        , Database::query_param_list(create_object_output.history_id)(create_object_output.object_id));

                //host_history
                ctx.get_conn().exec_params(
                    "INSERT INTO host_history(historyid, id, nssetid, fqdn) "
                    " SELECT $1::bigint, id, nssetid, fqdn FROM host "
                        " WHERE nssetid = $2::integer"
                    , Database::query_param_list(create_object_output.history_id)(create_object_output.object_id));

                //host_ipaddr_map_history
                ctx.get_conn().exec_params(
                    "INSERT INTO host_ipaddr_map_history(historyid, id, hostid, nssetid, ipaddr) "
                    " SELECT $1::bigint, id, hostid, nssetid, ipaddr FROM host_ipaddr_map "
                        " WHERE nssetid = $2::integer"
                    , Database::query_param_list(create_object_output.history_id)(create_object_output.object_id));

                //nsset_contact_map_history
                ctx.get_conn().exec_params(
                    "INSERT INTO nsset_contact_map_history(historyid, nssetid, contactid) "
                    " SELECT $1::bigint, nssetid, contactid FROM nsset_contact_map "
                        " WHERE nssetid = $2::integer"
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

    std::string CreateNsset::to_string() const
    {
        return Util::format_operation_state("CreateNsset",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("handle",handle_))
        (std::make_pair("registrar",registrar_))
        (std::make_pair("authinfo",authinfo_.print_quoted()))
        (std::make_pair("dns_hosts", Util::format_vector(dns_hosts_)))
        (std::make_pair("tech_contacts",Util::format_vector(tech_contacts_)))
        (std::make_pair("logd_request_id",logd_request_id_.print_quoted()))
        );
    }


}//namespace Fred

