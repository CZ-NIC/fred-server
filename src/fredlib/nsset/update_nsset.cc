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
 *  @file update_nsset.cc
 *  nsset update
 */

#include <string>
#include <vector>

#include "fredlib/nsset/update_nsset.h"
#include "fredlib/object/object.h"

#include "fredlib/opcontext.h"
#include "fredlib/db_settings.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/log/log.h"

namespace Fred
{
    UpdateNsset::UpdateNsset(const std::string& handle
            , const std::string& registrar)
    : handle_(handle)
    , registrar_(registrar)
    {}

    UpdateNsset::UpdateNsset(const std::string& handle
            , const std::string& registrar
            , const Optional<std::string>& authinfo
            , const std::vector<DnsHost>& add_dns
            , const std::vector<std::string>& rem_dns
            , const std::vector<std::string>& add_tech_contact
            , const std::vector<std::string>& rem_tech_contact
            , const Optional<short>& tech_check_level
            , const Optional<unsigned long long> logd_request_id
            )
    : handle_(handle)
    , registrar_(registrar)
    , authinfo_(authinfo)
    , add_dns_(add_dns)
    , rem_dns_(rem_dns)
    , add_tech_contact_(add_tech_contact)
    , rem_tech_contact_(rem_tech_contact)
    , tech_check_level_(tech_check_level)
    , logd_request_id_(logd_request_id.isset()
        ? Nullable<unsigned long long>(logd_request_id.get_value())
        : Nullable<unsigned long long>())//is NULL if not set
    {}

    UpdateNsset& UpdateNsset::set_authinfo(const std::string& authinfo)
    {
        authinfo_ = authinfo;
        return *this;
    }

    UpdateNsset& UpdateNsset::add_dns(const DnsHost& dns)
    {
        add_dns_.push_back(dns);
        return *this;
    }

    UpdateNsset& UpdateNsset::rem_dns(const std::string& fqdn)
    {
        rem_dns_.push_back(fqdn);
        return *this;
    }

    UpdateNsset& UpdateNsset::add_tech_contact(const std::string& tech_contact)
    {
        add_tech_contact_.push_back(tech_contact);
        return *this;
    }

    UpdateNsset& UpdateNsset::rem_tech_contact(const std::string& tech_contact)
    {
        rem_tech_contact_.push_back(tech_contact);
        return *this;
    }

    UpdateNsset& UpdateNsset::set_logd_request_id(unsigned long long logd_request_id)
    {
        logd_request_id_ = logd_request_id;
        return *this;
    }

    UpdateNsset& UpdateNsset::set_tech_check_level(short tech_check_level)
    {
        tech_check_level_ = tech_check_level;
        return *this;
    }

    unsigned long long UpdateNsset::exec(OperationContext& ctx)
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
                " WHERE eot.name = 'nsset' FOR UPDATE OF oreg"
                , Database::query_param_list(handle_));

            if (lock_res.size() != 1)
            {
                std::string errmsg("unable to lock || not found:handle: ");
                errmsg += boost::replace_all_copy(handle_,"|", "[pipe]");//quote pipes
                errmsg += " |";
                throw UNEX(errmsg.c_str());
            }
        }

        //get nsset_id
        unsigned long long nsset_id =0;
        {
            Database::Result nsset_id_res = ctx.get_conn().exec_params(
                "SELECT oreg.id FROM nsset n "
                " JOIN object_registry oreg ON n.id = oreg.id "
                " WHERE oreg.name = UPPER($1::text) AND oreg.erdate IS NULL"
                , Database::query_param_list(handle_));

            if (nsset_id_res.size() != 1)
            {
                std::string errmsg("|| not found:handle: ");
                errmsg += boost::replace_all_copy(handle_,"|", "[pipe]");//quote pipes
                errmsg += " |";
                throw UNEX(errmsg.c_str());
            }

            nsset_id = nsset_id_res[0][0];
        }


        //update object
        Fred::UpdateObject(handle_,"nsset", registrar_, authinfo_).exec(ctx);

        //update nsset tech check level
        if(tech_check_level_.isset() && tech_check_level_.get_value() >= 0)
        {
            ctx.get_conn().exec_params(
                "UPDATE nsset SET checklevel = $1::smallint "
                " WHERE id = $2::integer"
                , Database::query_param_list(tech_check_level_.get_value())(nsset_id));
        }//update nsset tech check level

        //add tech contacts
        if(!add_tech_contact_.empty())
        {
            Database::QueryParams params;//query params
            std::stringstream sql;

            params.push_back(nsset_id);
            sql << "INSERT INTO nsset_contact_map(nssetid, contactid) "
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
                        throw UNEX(errmsg.c_str());
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
                        std::string errmsg("add tech contact precheck uniqueness failed || already set:tech contact: ");
                        errmsg += boost::replace_all_copy(*i,"|", "[pipe]");//quote pipes
                        errmsg += " |";
                        throw UNEX(errmsg.c_str());
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
                    std::string errmsg("add tech contact failed || invalid:handle: ");
                    errmsg += boost::replace_all_copy(handle_,"|", "[pipe]");//quote pipes
                    errmsg += " | invalid:tech contact: ";
                    errmsg += boost::replace_all_copy(*i,"|", "[pipe]");//quote pipes
                    errmsg += " |";
                    throw UNEX(errmsg.c_str());
                }
            }//for i
        }//if add tech contacts

        //delete tech contacts
        if(!rem_tech_contact_.empty())
        {
            Database::QueryParams params;//query params
            std::stringstream sql;

            params.push_back(nsset_id);
            sql << "DELETE FROM nsset_contact_map WHERE nssetid = $" << params.size() << "::integer AND ";

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
                        throw UNEX(errmsg.c_str());
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
                    " RETURNING nssetid";
                Database::Result nsset_del_res = ctx.get_conn().exec_params(sql_i.str(), params_i);
                if (nsset_del_res.size() != 1)
                {
                    std::string errmsg("delete tech contact failed || invalid:tech contact: ");
                    errmsg += boost::replace_all_copy(*i,"|", "[pipe]");//quote pipes
                    errmsg += " |";
                    throw UNEX(errmsg.c_str());
                }
            }//for i
        }//if delete tech contacts

        //delete dns hosts - before adding new ones
        if(!rem_dns_.empty())
        {
            for(std::vector<std::string>::iterator i = rem_dns_.begin(); i != rem_dns_.end(); ++i)
            {
                Database::Result rem_host_id_res = ctx.get_conn().exec_params(
                    "DELETE FROM host WHERE LOWER(fqdn)=LOWER($1::text) AND"
                    " nssetid = $2::integer RETURNING id "
                    , Database::query_param_list(*i)(nsset_id));

                if (rem_host_id_res.size() != 1)
                {
                    std::string errmsg("delete dns hosts || not found:dns fqdn: ");
                    errmsg += boost::replace_all_copy(*i,"|", "[pipe]");//quote pipes
                    errmsg += " |";
                    throw UNEX(errmsg.c_str());
                }

                unsigned long long rem_host_id = rem_host_id_res[0][0];

                ctx.get_conn().exec_params("DELETE FROM host_ipaddr_map WHERE id = $1::integer"
                    , Database::query_param_list(rem_host_id));
            }//for i
        }//if delete dns hosts

        //add dns hosts
        if(!add_dns_.empty())
        {
            for(std::vector<DnsHost>::iterator i = add_dns_.begin(); i != add_dns_.end(); ++i)
            {
                {//precheck uniqueness
                    Database::Result nsset_res = ctx.get_conn().exec_params(
                    "SELECT nssetid, fqdn FROM host "
                    " WHERE nssetid = $1::bigint "
                    "  AND fqdn = LOWER($2::text) "
                    , Database::query_param_list(nsset_id)(i->get_fqdn()));

                    if (nsset_res.size() == 1)
                    {
                        std::string errmsg("dns host precheck uniqueness failed || invalid:dns fqdn: ");
                        errmsg += boost::replace_all_copy(i->get_fqdn(),"|", "[pipe]");//quote pipes
                        errmsg += " |";
                        throw UNEX(errmsg.c_str());
                    }
                }

                Database::Result add_host_id_res = ctx.get_conn().exec_params(
                    "INSERT INTO host (nssetid, fqdn) VALUES( "
                    " $1::integer, LOWER($2::text)) RETURNING id"
                    , Database::query_param_list(nsset_id)(i->get_fqdn()));
                if(add_host_id_res.size() != 1)
                {
                    std::string errmsg("add dns hosts || invalid:dns fqdn: ");
                    errmsg += boost::replace_all_copy(i->get_fqdn(),"|", "[pipe]");//quote pipes
                    errmsg += " |";
                    throw UNEX(errmsg.c_str());
                }

                unsigned long long add_host_id = add_host_id_res[0][0];

                std::vector<std::string> dns_host_ip = i->get_inet_addr();

                for(std::vector<std::string>::iterator j = dns_host_ip.begin(); j != dns_host_ip.end(); ++j)
                {
                    try
                     {
                         Database::Result add_host_ipaddr_res = ctx.get_conn().exec_params(
                         "INSERT INTO host_ipaddr_map (hostid, nssetid, ipaddr) "
                         " VALUES($1::integer, $2::integer, $3::inet) RETURNING hostid"
                         , Database::query_param_list(add_host_id)(nsset_id)(*j));
                     }
                     catch(Database::ResultFailed& ex)
                     {
                         std::string errmsg = ex.what();
                         errmsg +=" || invalid:ipaddr: ";
                         errmsg += boost::replace_all_copy(*j,"|", "[pipe]");//quote pipes
                         errmsg += " |";
                         throw UNEX(errmsg.c_str());
                     }
                }//for j
            }//for i
        }//if add dns hosts

        //save history
        {
            history_id = Fred::InsertHistory(logd_request_id_).exec(ctx);

            //object_history
            ctx.get_conn().exec_params(
                "INSERT INTO object_history(historyid,id,clid, upid, trdate, update, authinfopw) "
                " SELECT $1::bigint, id,clid, upid, trdate, update, authinfopw FROM object "
                " WHERE id = $2::integer"
                , Database::query_param_list(history_id)(nsset_id));

            //object_registry historyid
            ctx.get_conn().exec_params(
                "UPDATE object_registry SET historyid = $1::bigint "
                    " WHERE id = $2::integer"
                    , Database::query_param_list(history_id)(nsset_id));

            //nsset_history
            ctx.get_conn().exec_params(
                "INSERT INTO nsset_history(historyid,id,checklevel) "
                " SELECT $1::bigint, id, checklevel FROM nsset "
                    " WHERE id = $2::integer"
                    , Database::query_param_list(history_id)(nsset_id));

            //host_history
            ctx.get_conn().exec_params(
                "INSERT INTO host_history(historyid, id, nssetid, fqdn) "
                " SELECT $1::bigint, id, nssetid, fqdn FROM host "
                    " WHERE nssetid = $2::integer"
                , Database::query_param_list(history_id)(nsset_id));

            //host_ipaddr_map_history
            ctx.get_conn().exec_params(
                "INSERT INTO host_ipaddr_map_history(historyid, id, hostid, nssetid, ipaddr) "
                " SELECT $1::bigint, id, hostid, nssetid, ipaddr FROM host_ipaddr_map "
                    " WHERE nssetid = $2::integer"
                , Database::query_param_list(history_id)(nsset_id));

            //nsset_contact_map_history
            ctx.get_conn().exec_params(
                "INSERT INTO nsset_contact_map_history(historyid, nssetid, contactid) "
                " SELECT $1::bigint, nssetid, contactid FROM nsset_contact_map "
                    " WHERE nssetid = $2::integer"
                , Database::query_param_list(history_id)(nsset_id));
        }//save history

        }//try
        catch(...)//common exception processing
        {
            handleOperationExceptions<UpdateNssetException>(__FILE__, __LINE__, __ASSERT_FUNCTION);
        }
        return history_id;
    }//UpdateNsset::exec

}//namespace Fred


