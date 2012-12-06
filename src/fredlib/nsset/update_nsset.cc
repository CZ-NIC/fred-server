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

    void UpdateNsset::exec(OperationContext& ctx)
    {
        //lock object_registry row for update
        {
            Database::Result lock_res = ctx.get_conn().exec_params(
                "SELECT id FROM object_registry WHERE UPPER(name) = UPPER($1::text) AND type = 2 FOR UPDATE"
                , Database::query_param_list(handle_));

            if (lock_res.size() != 1)
            {
                throw std::runtime_error("UpdateNsset::exec unable to lock");
            }
        }

        //update object
        Fred::UpdateObject(handle_, registrar_, authinfo_).exec(ctx);

        //update nsset tech check level
        if(tech_check_level_.isset() && tech_check_level_.get_value() >= 0)
        {
            ctx.get_conn().exec_params(
                "UPDATE nsset SET checklevel = $1::smallint "
                " WHERE id = (SELECT oreg.id FROM nsset n "
                    " JOIN object_registry oreg ON n.id = oreg.id "
                    " WHERE UPPER(oreg.name) = UPPER($2::text))"
                , Database::query_param_list(tech_check_level_.get_value())(handle_));
        }//update nsset tech check level

        //add tech contacts
        if(!add_tech_contact_.empty())
        {
            Database::QueryParams params;//query params
            std::stringstream sql;

            params.push_back(handle_);
            sql << "INSERT INTO nsset_contact_map(nssetid, contactid) "
                    " VALUES ((SELECT oreg.id FROM nsset n "
                    " JOIN object_registry oreg ON n.id = oreg.id "
                    " WHERE UPPER(oreg.name) = UPPER($" << params.size() << "::text)), ";

            for(std::vector<std::string>::iterator i = add_tech_contact_.begin(); i != add_tech_contact_.end(); ++i)
            {
                Database::QueryParams params_i = params;//query params
                std::stringstream sql_i(sql.str());

                params_i.push_back(*i);
                sql_i << " (SELECT oreg.id FROM object_registry oreg JOIN contact c ON oreg.id = c.id "
                    " WHERE UPPER(oreg.name) = UPPER($"<< params_i.size() << "::text)));";
                ctx.get_conn().exec_params(sql_i.str(), params_i);
            }//for i
        }//if add tech contacts

        //delete tech contacts
        if(!rem_tech_contact_.empty())
        {
            Database::QueryParams params;//query params
            std::stringstream sql;

            params.push_back(handle_);
            sql << "DELETE FROM nsset_contact_map WHERE nssetid = "
                    " (SELECT oreg.id FROM nsset n "
                    " JOIN object_registry oreg ON n.id = oreg.id "
                    " WHERE UPPER(oreg.name) = UPPER($" << params.size() << "::text)) AND ";

            for(std::vector<std::string>::iterator i = rem_tech_contact_.begin(); i != rem_tech_contact_.end(); ++i)
            {
                Database::QueryParams params_i = params;//query params
                std::stringstream sql_i(sql.str());

                params_i.push_back(*i);
                sql_i << "contactid = (SELECT oreg.id FROM object_registry oreg "
                        " JOIN contact c ON oreg.id = c.id WHERE UPPER(oreg.name) = UPPER($"
                    << params_i.size() << "::text));";
                ctx.get_conn().exec_params(sql_i.str(), params_i);
            }//for i
        }//if delete tech contacts

        //delete dns hosts - before adding new ones
        if(!rem_dns_.empty())
        {
            for(std::vector<std::string>::iterator i = rem_dns_.begin(); i != rem_dns_.end(); ++i)
            {
                Database::Result rem_host_id_res = ctx.get_conn().exec_params(
                    "DELETE FROM host WHERE LOWER(fqdn)=LOWER($1::text) AND"
                    " nssetid = (SELECT oreg.id FROM nsset n JOIN object_registry oreg ON n.id = oreg.id "
                    " WHERE UPPER(oreg.name) = UPPER($2::text)) RETURNING id "
                    , Database::query_param_list(*i)(handle_));

                if (rem_host_id_res.size() != 1)
                {
                    throw std::runtime_error("UpdateNsset::exec unable to get removed host id");
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
                Database::Result add_host_id_res = ctx.get_conn().exec_params(
                    "INSERT INTO host (nssetid, fqdn) VALUES( "
                    " (SELECT oreg.id FROM nsset n JOIN object_registry oreg ON n.id = oreg.id "
                    " WHERE UPPER(oreg.name) = UPPER($1::text)), LOWER($2::text)) RETURNING id"
                    , Database::query_param_list(handle_)(i->get_fqdn()));
                if(add_host_id_res.size() != 1)
                {
                    throw std::runtime_error("UpdateNsset::exec unable to get added host id");
                }

                unsigned long long add_host_id = add_host_id_res[0][0];

                for(std::vector<std::string>::iterator j = i->get_inet_addr().begin(); j != i->get_inet_addr().end(); ++j)
                {
                    ctx.get_conn().exec_params(
                        "INSERT INTO host_ipaddr_map (hostid, nssetid, ipaddr) VALUES($1::integer, "
                        " (SELECT oreg.id FROM nsset n JOIN object_registry oreg ON n.id = oreg.id "
                        " WHERE UPPER(oreg.name) = UPPER($2::text)), $3::inet)"
                        , Database::query_param_list(add_host_id)(handle_)(*j));
                }//for j
            }//for i
        }//if add dns hosts

        //save history
        {
            unsigned long long history_id = Fred::InsertHistory(logd_request_id_).exec(ctx);

            //object_history
            ctx.get_conn().exec_params(
                "INSERT INTO object_history(historyid,id,clid, upid, trdate, update, authinfopw) "
                " SELECT $1::bigint, id,clid, upid, trdate, update, authinfopw FROM object "
                " WHERE id = (SELECT oreg.id FROM nsset n "
                    " JOIN object_registry oreg ON n.id = oreg.id "
                    " WHERE UPPER(oreg.name) = UPPER($2::text))"
                , Database::query_param_list(history_id)(handle_));

            //object_registry historyid
            ctx.get_conn().exec_params(
                "UPDATE object_registry SET historyid = $1::bigint "
                " WHERE id = (SELECT oreg.id FROM nsset n "
                    " JOIN object_registry oreg ON n.id = oreg.id "
                    " WHERE UPPER(oreg.name) = UPPER($2::text))"
                , Database::query_param_list(history_id)(handle_));

            //nsset_history
            ctx.get_conn().exec_params(
                "INSERT INTO nsset_history(historyid,id,checklevel) "
                " SELECT $1::bigint, id, checklevel FROM nsset "
                    " WHERE id = (SELECT oreg.id FROM nsset n "
                    " JOIN object_registry oreg ON n.id = oreg.id "
                    " WHERE UPPER(oreg.name) = UPPER($2::text))"
                , Database::query_param_list(history_id)(handle_));

            //host_history
            ctx.get_conn().exec_params(
                "INSERT INTO host_history(historyid, id, nssetid, fqdn) "
                " SELECT $1::bigint, id, nssetid, fqdn FROM host "
                    " WHERE nssetid = (SELECT oreg.id FROM nsset n "
                    " JOIN object_registry oreg ON n.id = oreg.id "
                    " WHERE UPPER(oreg.name) = UPPER($2::text))"
                , Database::query_param_list(history_id)(handle_));

            //host_ipaddr_map_history
            ctx.get_conn().exec_params(
                "INSERT INTO host_ipaddr_map_history(historyid, id, hostid, nssetid, ipaddr) "
                " SELECT $1::bigint, id, hostid, nssetid, ipaddr FROM host_ipaddr_map "
                    " WHERE nssetid = (SELECT oreg.id FROM nsset n "
                    " JOIN object_registry oreg ON n.id = oreg.id "
                    " WHERE UPPER(oreg.name) = UPPER($2::text))"
                , Database::query_param_list(history_id)(handle_));

            //nsset_contact_map_history
            ctx.get_conn().exec_params(
                "INSERT INTO nsset_contact_map_history(historyid, nssetid, contactid) "
                " SELECT $1::bigint, nssetid, contactid FROM nsset_contact_map "
                    " WHERE nssetid = (SELECT oreg.id FROM nsset n "
                    " JOIN object_registry oreg ON n.id = oreg.id "
                    " WHERE UPPER(oreg.name) = UPPER($2::text))"
                , Database::query_param_list(history_id)(handle_));

        }//save history

    }//UpdateNsset::exec

}//namespace Fred


