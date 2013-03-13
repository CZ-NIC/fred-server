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
 *  @file create_nsset.h
 *  create nsset
 */


#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>

#include "fredlib/nsset/create_nsset.h"
#include "fredlib/object/object.h"

#include "fredlib/opcontext.h"
#include "fredlib/db_settings.h"
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
            unsigned long long object_id = CreateObject("nsset", handle_, registrar_, authinfo_).exec(ctx);
            //create nsset
            {
                Database::QueryParams params;//query params
                std::stringstream col_sql, val_sql;
                Util::HeadSeparator col_separator("",", "), val_separator("",", ");

                col_sql <<"INSERT INTO nsset (";
                val_sql << " VALUES (";

                //id
                params.push_back(object_id);
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
                        Database::Result add_host_id_res = ctx.get_conn().exec_params(
                            "INSERT INTO host (nssetid, fqdn) VALUES( "
                            " $1::integer, LOWER($2::text)) RETURNING id"
                            , Database::query_param_list(object_id)(i->get_fqdn()));
                        if(add_host_id_res.size() != 1)
                        {
                            std::string errmsg("set dns hosts || invalid:dns fqdn: ");
                            errmsg += boost::replace_all_copy(i->get_fqdn(),"|", "[pipe]");//quote pipes
                            errmsg += " |";
                            throw CNEX(errmsg.c_str());
                        }

                        unsigned long long add_host_id = add_host_id_res[0][0];

                        std::vector<std::string> dns_host_ip = i->get_inet_addr();

                        for(std::vector<std::string>::iterator j = dns_host_ip.begin(); j != dns_host_ip.end(); ++j)
                        {
                            Database::Result add_host_ipaddr_res = ctx.get_conn().exec_params(
                                "INSERT INTO host_ipaddr_map (hostid, nssetid, ipaddr) "
                                " VALUES($1::integer, $2::integer, $3::inet) RETURNING hostid"
                                , Database::query_param_list(add_host_id)(object_id)(*j));
                            if(add_host_ipaddr_res.size() != 1)
                            {
                                std::string errmsg("set dns hosts || invalid:ipaddr: ");
                                errmsg += boost::replace_all_copy(*j,"|", "[pipe]");//quote pipes
                                errmsg += " |";
                                throw CNEX(errmsg.c_str());
                            }

                        }//for j
                    }//for i
                }//if set dns hosts


                //set tech contacts
                if(!tech_contacts_.empty())
                {
                    Database::QueryParams params;//query params
                    std::stringstream sql;

                    params.push_back(object_id);
                    sql << "INSERT INTO nsset_contact_map(nssetid, contactid) "
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
                                throw CNEX(errmsg.c_str());
                            }
                        }

                        Database::QueryParams params_i = params;//query params
                        std::stringstream sql_i;
                        sql_i << sql.str();

                        params_i.push_back(*i);

                        {//precheck uniqueness
                            Database::Result nsset_res = ctx.get_conn().exec_params(
                            "SELECT nssetid, contactid FROM nsset_contact_map "
                            " WHERE nssetid = $1::bigint "
                            "  AND contactid = raise_exception_ifnull("
                            "    (SELECT oreg.id FROM object_registry oreg "
                            "       JOIN contact c ON oreg.id = c.id "
                            "     WHERE oreg.name = UPPER($2::text) AND oreg.erdate IS NULL) "
                            "     ,'|| not found:tech contact: '||ex_data($2::text)||' |')"
                            , params_i);

                            if (nsset_res.size() == 1)
                            {
                                std::string errmsg("set tech contact precheck uniqueness failed || already set:tech contact: ");
                                errmsg += boost::replace_all_copy(*i,"|", "[pipe]");//quote pipes
                                errmsg += " |";
                                throw CNEX(errmsg.c_str());
                            }
                        }

                        sql_i << " raise_exception_ifnull("
                            " (SELECT oreg.id FROM object_registry oreg JOIN contact c ON oreg.id = c.id "
                            " WHERE oreg.name = UPPER($"<< params_i.size() << "::text) AND oreg.erdate IS NULL) "
                            " ,'|| not found:tech contact: '||ex_data($"<< params.size() << "::text)||' |')) "
                            " RETURNING nssetid";
                        Database::Result nsset_add_check_res = ctx.get_conn().exec_params(sql_i.str(), params_i);
                        if (nsset_add_check_res.size() != 1)
                        {
                            std::string errmsg("set tech contact failed || invalid:handle: ");
                            errmsg += boost::replace_all_copy(handle_,"|", "[pipe]");//quote pipes
                            errmsg += " | invalid:tech contact: ";
                            errmsg += boost::replace_all_copy(*i,"|", "[pipe]");//quote pipes
                            errmsg += " |";
                            throw CNEX(errmsg.c_str());
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
                        throw CNEX(errmsg.c_str());
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
                    "UPDATE object_registry SET historyid = $1::bigint "
                        " WHERE id = $2::integer"
                        , Database::query_param_list(history_id)(object_id));

                //nsset_history
                ctx.get_conn().exec_params(
                    "INSERT INTO nsset_history(historyid,id,checklevel) "
                    " SELECT $1::bigint, id, checklevel FROM nsset "
                        " WHERE id = $2::integer"
                        , Database::query_param_list(history_id)(object_id));

                //host_history
                ctx.get_conn().exec_params(
                    "INSERT INTO host_history(historyid, id, nssetid, fqdn) "
                    " SELECT $1::bigint, id, nssetid, fqdn FROM host "
                        " WHERE nssetid = $2::integer"
                    , Database::query_param_list(history_id)(object_id));

                //host_ipaddr_map_history
                ctx.get_conn().exec_params(
                    "INSERT INTO host_ipaddr_map_history(historyid, id, hostid, nssetid, ipaddr) "
                    " SELECT $1::bigint, id, hostid, nssetid, ipaddr FROM host_ipaddr_map "
                        " WHERE nssetid = $2::integer"
                    , Database::query_param_list(history_id)(object_id));

                //nsset_contact_map_history
                ctx.get_conn().exec_params(
                    "INSERT INTO nsset_contact_map_history(historyid, nssetid, contactid) "
                    " SELECT $1::bigint, nssetid, contactid FROM nsset_contact_map "
                        " WHERE nssetid = $2::integer"
                    , Database::query_param_list(history_id)(object_id));
            }//save history


        }//try
        catch(...)//common exception processing
        {
            handleOperationExceptions<CreateNssetException>(__FILE__, __LINE__, __ASSERT_FUNCTION);
        }

        return timestamp;
    }

}//namespace Fred

