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
 *  @file
 *  nsset update
 */

#include <string>
#include <vector>

#include "src/fredlib/nsset/update_nsset.h"
#include "src/fredlib/object/object.h"
#include "src/fredlib/registrar/registrar_impl.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/db_settings.h"
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
            , const Optional<std::string>& sponsoring_registrar
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
    , sponsoring_registrar_(sponsoring_registrar)
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

    UpdateNsset& UpdateNsset::set_sponsoring_registrar(const std::string& sponsoring_registrar)
    {
        sponsoring_registrar_ = sponsoring_registrar;
        return *this;
    }

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
        //check registrar
        Registrar::get_registrar_id_by_handle(
            ctx, registrar_, static_cast<Exception*>(0)//set throw
            , &Exception::set_unknown_registrar_handle);

        //lock row and get nsset_id
        unsigned long long nsset_id = lock_object_by_handle_and_type(
                ctx,handle_,"nsset",static_cast<Exception*>(0),
                &Exception::set_unknown_nsset_handle);

        Exception update_nsset_exception;

        try
        {
            //update object
            history_id = Fred::UpdateObject(handle_,"nsset", registrar_
                , sponsoring_registrar_, authinfo_, logd_request_id_
            ).exec(ctx);
        }
        catch(const Fred::UpdateObject::Exception& ex)
        {
            if(ex.is_set_unknown_object_handle())
            {
                update_nsset_exception.set_unknown_nsset_handle(
                        ex.get_unknown_object_handle());
            }

            if(ex.is_set_unknown_registrar_handle())
            {
                update_nsset_exception.set_unknown_registrar_handle(
                        ex.get_unknown_registrar_handle());
            }

            if(ex.is_set_unknown_sponsoring_registrar_handle())
            {
                update_nsset_exception.set_unknown_sponsoring_registrar_handle(
                        ex.get_unknown_sponsoring_registrar_handle());
            }
        }
        //update nsset tech check level
        if(tech_check_level_.isset() && tech_check_level_.get_value() >= 0)
        {
            Database::Result update_checklevel_res = ctx.get_conn().exec_params(
                "UPDATE nsset SET checklevel = $1::smallint "
                " WHERE id = $2::integer RETURNING id"
                , Database::query_param_list(tech_check_level_.get_value())(nsset_id));
            if(update_checklevel_res.size() != 1)
            {
                BOOST_THROW_EXCEPTION(InternalError("failed to update checklevel"));
            }
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
                unsigned long long tech_contact_id = lock_object_by_handle_and_type(
                        ctx,*i,"contact",&update_nsset_exception,
                        &Exception::add_unknown_technical_contact_handle);
                if(tech_contact_id == 0) continue;

                Database::QueryParams params_i = params;//query params
                std::stringstream sql_i;
                sql_i << sql.str();

                params_i.push_back(tech_contact_id);
                sql_i << " $" << params_i.size() << "::integer) ";

                try
                {
                    ctx.get_conn().exec("SAVEPOINT add_tech_contact");
                    ctx.get_conn().exec_params(sql_i.str(), params_i);
                    ctx.get_conn().exec("RELEASE SAVEPOINT add_tech_contact");
                }
                catch(const std::exception& ex)
                {
                    std::string what_string(ex.what());
                    if(what_string.find("nsset_contact_map_pkey") != std::string::npos)
                    {
                        update_nsset_exception.add_already_set_technical_contact_handle(*i);
                        ctx.get_conn().exec("ROLLBACK TO SAVEPOINT add_tech_contact");
                    }
                    else
                        throw;
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
                unsigned long long tech_contact_id = lock_object_by_handle_and_type(
                        ctx,*i,"contact",&update_nsset_exception,
                        &Exception::add_unknown_technical_contact_handle);
                if(tech_contact_id == 0) continue;

                Database::QueryParams params_i = params;//query params
                std::stringstream sql_i;
                sql_i << sql.str();

                params_i.push_back(tech_contact_id);
                sql_i << "contactid = $" << params_i.size() << "::integer "
                        " RETURNING nssetid";

                Database::Result nsset_del_res = ctx.get_conn().exec_params(sql_i.str(), params_i);
                if (nsset_del_res.size() == 0)
                {
                    update_nsset_exception.add_unassigned_technical_contact_handle(*i);
                    continue;//for rem_tech_contact_
                }
                if (nsset_del_res.size() != 1)
                {
                    BOOST_THROW_EXCEPTION(InternalError("failed to delete technical contact"));
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

                if (rem_host_id_res.size() == 0)
                {
                    update_nsset_exception.add_unassigned_dns_host(*i);
                    continue;//for rem_dns_
                }
                if (rem_host_id_res.size() != 1)
                {
                    BOOST_THROW_EXCEPTION(InternalError("failed to delete DNS host"));
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
                unsigned long long add_host_id = 0;
                try
                {
                    ctx.get_conn().exec("SAVEPOINT add_dns_host");
                    Database::Result add_host_id_res = ctx.get_conn().exec_params(
                        "INSERT INTO host (nssetid, fqdn) VALUES( "
                        " $1::integer, LOWER($2::text)) RETURNING id"
                        , Database::query_param_list(nsset_id)(i->get_fqdn()));
                    ctx.get_conn().exec("RELEASE SAVEPOINT add_dns_host");
                    add_host_id = static_cast<unsigned long long>(add_host_id_res[0][0]);
                }
                catch(const std::exception& ex)
                {
                    std::string what_string(ex.what());
                    if(what_string.find("host_nssetid_fqdn_key") != std::string::npos)
                    {
                        update_nsset_exception.add_already_set_dns_host(i->get_fqdn());
                        ctx.get_conn().exec("ROLLBACK TO SAVEPOINT add_dns_host");
                        continue;//for add_dns_
                    }
                    else
                        throw;
                }

                std::vector<std::string> dns_host_ip = i->get_inet_addr();

                for(std::vector<std::string>::iterator j = dns_host_ip.begin(); j != dns_host_ip.end(); ++j)
                {
                    try
                    {
                        ctx.get_conn().exec("SAVEPOINT add_dns_host_ipaddr");
                        ctx.get_conn().exec_params(
                        "INSERT INTO host_ipaddr_map (hostid, nssetid, ipaddr) "
                        " VALUES($1::integer, $2::integer, $3::inet)"
                        , Database::query_param_list(add_host_id)(nsset_id)(*j));
                        ctx.get_conn().exec("RELEASE SAVEPOINT add_dns_host_ipaddr");
                    }
                    catch(const std::exception& ex)
                    {
                        std::string what_string(ex.what());
                        if(what_string.find("syntax for type inet") != std::string::npos)
                        {
                            update_nsset_exception.add_invalid_dns_host_ipaddr(*j);
                            ctx.get_conn().exec("ROLLBACK TO SAVEPOINT add_dns_host_ipaddr");
                        }
                        else
                            throw;
                    }
                }//for j
            }//for i
        }//if add dns hosts

        //check exception
        if(update_nsset_exception.throw_me())
            BOOST_THROW_EXCEPTION(update_nsset_exception);

        //save history
        {
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
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }

        return history_id;
    }//UpdateNsset::exec

    std::string UpdateNsset::to_string() const
    {
        return Util::format_operation_state("UpdateNsset",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("handle",handle_))
        (std::make_pair("registrar",registrar_))
        (std::make_pair("sponsoring_registrar",sponsoring_registrar_.print_quoted()))
        (std::make_pair("authinfo",authinfo_.print_quoted()))
        (std::make_pair("add_tech_contact",Util::format_vector(add_tech_contact_)))
        (std::make_pair("rem_tech_contact",Util::format_vector(rem_tech_contact_)))
        (std::make_pair("add_dns_host", Util::format_vector(add_dns_)))
        (std::make_pair("rem_dns_host", Util::format_vector(rem_dns_)))
        (std::make_pair("tech_check_level",tech_check_level_.print_quoted()))
        (std::make_pair("logd_request_id",logd_request_id_.print_quoted()))
        );
    }


}//namespace Fred


