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
 *  @file create_domain.h
 *  create domain
 */


#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>

#include "fredlib/domain/create_domain.h"
#include "fredlib/object/object.h"

#include "fredlib/opcontext.h"
#include "fredlib/db_settings.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"

namespace Fred
{

    CreateDomain::CreateDomain(const std::string& fqdn
                , const std::string& registrar
                , const std::string& registrant)
    : fqdn_(fqdn)
    , registrar_(registrar)
    , registrant_(registrant)
    {}

    CreateDomain::CreateDomain(const std::string& fqdn
            , const std::string& registrar
            , const std::string& registrant
            , const Optional<std::string>& authinfo
            , const Optional<Nullable<std::string> >& nsset
            , const Optional<Nullable<std::string> >& keyset
            , const std::vector<std::string>& admin_contacts
            , const Optional<unsigned>& expiration_period
            , const Optional<unsigned long long> logd_request_id
            )
    : fqdn_(fqdn)
    , registrar_(registrar)
    , authinfo_(authinfo)
    , registrant_(registrant)
    , nsset_(nsset)
    , keyset_(keyset)
    , admin_contacts_(admin_contacts)
    , expiration_period_(expiration_period)
    , logd_request_id_(logd_request_id.isset()
            ? Nullable<unsigned long long>(logd_request_id.get_value())
            : Nullable<unsigned long long>())//is NULL if not set
    {}

    CreateDomain& CreateDomain::set_authinfo(const std::string& authinfo)
    {
        authinfo_ = authinfo;
        return *this;
    }

    CreateDomain& CreateDomain::set_nsset(const Nullable<std::string>& nsset)
    {
        nsset_ = nsset;
        return *this;
    }

    CreateDomain& CreateDomain::set_nsset(const std::string& nsset)
    {
        nsset_ = Nullable<std::string>(nsset);
        return *this;
    }

    CreateDomain& CreateDomain::set_keyset(const Nullable<std::string>& keyset)
    {
        keyset_ = keyset;
        return *this;
    }

    CreateDomain& CreateDomain::set_keyset(const std::string& keyset)
    {
        keyset_ = Nullable<std::string>(keyset);
        return *this;
    }

    CreateDomain& CreateDomain::set_admin_contacts(const std::vector<std::string>& admin_contacts)
    {
        admin_contacts_ = admin_contacts;
        return *this;
    }

    CreateDomain& CreateDomain::set_expiration_period(unsigned expiration_period)
    {
        expiration_period_ = expiration_period;
        return *this;
    }

    CreateDomain& CreateDomain::set_logd_request_id(unsigned long long logd_request_id)
    {
        logd_request_id_ = logd_request_id;
        return *this;
    }

    boost::posix_time::ptime CreateDomain::exec(OperationContext& ctx, const std::string& returned_timestamp_pg_time_zone_name)
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
            //zone
            unsigned long long zone_id = 0;
            {
                std::string domain(boost::to_lower_copy(fqdn_));

                Database::Result zone_res = ctx.get_conn().exec(
                    "SELECT fqdn FROM zone ORDER BY length(fqdn) DESC");

                if (zone_res.size() == 0)
                {
                    BOOST_THROW_EXCEPTION(InternalError("unable to create domain without zone configuration"));
                }

                for (Database::Result::size_type i = 0 ; i < zone_res.size(); ++i)
                {
                    std::string zone  = static_cast<std::string>(zone_res[i][0]);
                    int from = domain.length() - zone.length();
                    if(from > 1)
                    {
                        if (domain.find(zone, from) != std::string::npos)
                        {
                            Database::Result zone_id_res = ctx.get_conn().exec_params(
                                "SELECT id FROM zone WHERE lower(fqdn)=lower($1::text) FOR SHARE"
                                , Database::query_param_list(domain.substr(from, std::string::npos)));

                            if(zone_id_res.size() == 1)
                            {
                                zone_id = static_cast<unsigned long long>(zone_id_res[0][0]);
                                break;
                            }
                        }
                    }
                }//for zone_res

                if(zone_id == 0)
                {
                    BOOST_THROW_EXCEPTION(Exception().set_unknown_zone_fqdn(fqdn_));
                }
            }//zone

            //expiration_period
            unsigned expiration_period = 0;//in months
            {
                if(expiration_period_.isset())
                {
                    expiration_period = expiration_period_;
                }
                else
                {
                    //get default
                    Database::Result ex_period_min_res = ctx.get_conn().exec_params(
                        "SELECT ex_period_min FROM zone WHERE id=$1::bigint FOR SHARE"
                        , Database::query_param_list(zone_id));

                    if (ex_period_min_res.size() == 0)
                    {
                        BOOST_THROW_EXCEPTION(InternalError("ex_period_min for zone not found"));
                    }

                    expiration_period = static_cast<unsigned>(ex_period_min_res[0][0]);
                }
            }//expiration_period

            unsigned long long object_id = CreateObject("domain", fqdn_, registrar_, authinfo_).exec(ctx);

            //get crdate and exdate and lock row from object_registry
            boost::gregorian::date expiration_date;
            {
                Database::Result reg_date_res = ctx.get_conn().exec_params(
                    "SELECT crdate::timestamp AT TIME ZONE 'UTC' AT TIME ZONE $1::text "
                    " , (crdate::timestamp AT TIME ZONE 'UTC' AT TIME ZONE $1::text + ( $3::integer * interval '1 month') )::date "
                    "  FROM object_registry "
                    " WHERE id = $2::bigint FOR UPDATE OF object_registry"
                    , Database::query_param_list(returned_timestamp_pg_time_zone_name)(object_id)(expiration_period));

                if (reg_date_res.size() != 1)
                {
                    BOOST_THROW_EXCEPTION(Fred::InternalError("timestamp of the domain creation was not found"));
                }

                timestamp = boost::posix_time::time_from_string(std::string(reg_date_res[0][0]));
                expiration_date = boost::gregorian::from_simple_string(std::string(reg_date_res[0][1]));
            }

            Exception create_domain_exception;

            //lock registrant object_registry row for update and get id
            unsigned long long registrant_id = 0;
            {
                Database::Result registrant_res = ctx.get_conn().exec_params(
                    "SELECT oreg.id FROM enum_object_type eot"
                    " JOIN object_registry oreg ON oreg.type = eot.id "
                    " JOIN contact c ON oreg.id = c.id "
                    " AND oreg.name = UPPER($1::text) AND oreg.erdate IS NULL "
                    " WHERE eot.name = 'contact' FOR UPDATE OF oreg"
                    , Database::query_param_list(registrant_));

                if (registrant_res.size() == 0)
                {
                    create_domain_exception.set_unknown_registrant_handle(registrant_);
                }
                if (registrant_res.size() > 1)
                {
                    BOOST_THROW_EXCEPTION(InternalError("failed to get registrant"));
                }
                if (registrant_res.size() == 1)
                {
                    registrant_id = static_cast<unsigned long long>(registrant_res[0][0]);
                }
            }

            //create domain
            {
                Database::QueryParams params;//query params
                std::stringstream col_sql, val_sql;
                Util::HeadSeparator col_separator("",", "), val_separator("",", ");

                col_sql <<"INSERT INTO domain (";
                val_sql << " VALUES (";

                //id
                params.push_back(object_id);
                col_sql << col_separator.get() << "id";
                val_sql << val_separator.get() << "$" << params.size() <<"::integer";

                //zone id
                params.push_back(zone_id);
                col_sql << col_separator.get() << "zone";
                val_sql << val_separator.get() << "$" << params.size() <<"::integer";

                //expiration_date
                params.push_back(expiration_date);
                col_sql << col_separator.get() << "exdate";
                val_sql << val_separator.get() << "$" << params.size() <<"::date";

                //set registrant
                params.push_back(registrant_id);
                col_sql << col_separator.get() << "registrant";
                val_sql << val_separator.get() << "$" << params.size() <<"::integer";

                //nsset
                if(nsset_.isset())
                {
                    Nullable<std::string> new_nsset_value = nsset_;
                    col_sql << col_separator.get() << "nsset";

                    if(new_nsset_value.isnull())
                    {//null case query
                        params.push_back(Database::NullQueryParam);//NULL
                        val_sql << val_separator.get() << "$" << params.size() <<"::integer";
                    }
                    else
                    {//value case query, lock nsset object_registry row for update and get id
                        unsigned long long nsset_id = 0;
                        {
                            Database::Result lock_nsset_res = ctx.get_conn().exec_params(
                                "SELECT oreg.id FROM enum_object_type eot"
                                " JOIN object_registry oreg ON oreg.type = eot.id "
                                " JOIN nsset n ON oreg.id = n.id "
                                " AND oreg.name = UPPER($1::text) AND oreg.erdate IS NULL "
                                " WHERE eot.name = 'nsset' FOR UPDATE OF oreg"
                                , Database::query_param_list(new_nsset_value));

                            if (lock_nsset_res.size() == 0)
                            {
                                create_domain_exception.set_unknown_nsset_handle(new_nsset_value);
                            }
                            if (lock_nsset_res.size() > 1)
                            {
                                BOOST_THROW_EXCEPTION(InternalError("failed to get nsset"));
                            }
                            if (lock_nsset_res.size() == 1)
                            {
                                nsset_id = static_cast<unsigned long long>(lock_nsset_res[0][0]);
                            }

                            params.push_back(nsset_id);//id
                            val_sql << val_separator.get() << "$" << params.size() <<"::integer";
                        }
                    }
                }//if nsset

                //keyset
                if(keyset_.isset())
                {
                    Nullable<std::string> new_keyset_value = keyset_;
                    col_sql << col_separator.get() << "keyset";

                    if(new_keyset_value.isnull())
                    {//null case query
                        params.push_back(Database::NullQueryParam);//NULL
                        val_sql << val_separator.get() << "$" << params.size() <<"::integer";
                    }
                    else
                    {//value case query, lock keyset object_registry row for update and get id
                        unsigned long long keyset_id = 0;
                        {
                            Database::Result lock_keyset_res = ctx.get_conn().exec_params(
                                "SELECT oreg.id FROM enum_object_type eot"
                                " JOIN object_registry oreg ON oreg.type = eot.id "
                                " JOIN keyset k ON oreg.id = k.id "
                                " AND oreg.name = UPPER($1::text) AND oreg.erdate IS NULL "
                                " WHERE eot.name = 'keyset' FOR UPDATE OF oreg"
                                , Database::query_param_list(new_keyset_value));

                            if (lock_keyset_res.size() == 0)
                            {
                                create_domain_exception.set_unknown_keyset_handle(new_keyset_value);
                            }
                            if (lock_keyset_res.size() > 1)
                            {
                                BOOST_THROW_EXCEPTION(InternalError("failed to get keyset"));
                            }

                            keyset_id = static_cast<unsigned long long>(lock_keyset_res[0][0]);

                            params.push_back(keyset_id);//id
                            val_sql << val_separator.get() << "$" << params.size() <<"::integer";
                        }
                    }
                }//if keyset

                col_sql <<")";
                val_sql << ")";

                //check exception
                if(create_domain_exception.is_set_unknown_registrant_handle()
                || create_domain_exception.is_set_unknown_nsset_handle()
                || create_domain_exception.is_set_unknown_keyset_handle()
                ) BOOST_THROW_EXCEPTION(create_domain_exception);

                //insert into domain
                ctx.get_conn().exec_params(col_sql.str() + val_sql.str(), params);

                //set admin contacts
                if(!admin_contacts_.empty())
                {
                    Database::QueryParams params;//query params
                    std::stringstream sql;

                    params.push_back(object_id);
                    sql << "INSERT INTO domain_contact_map(domainid, contactid) "
                            " VALUES ($" << params.size() << "::integer, ";


                    for(std::vector<std::string>::iterator i = admin_contacts_.begin(); i != admin_contacts_.end(); ++i)
                    {
                        //lock admin contact object_registry row for update and get id
                        unsigned long long admin_contact_id = 0;
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
                                create_domain_exception.add_unknown_admin_contact_handle(*i);
                                continue;
                            }
                            if (lock_res.size() != 1)
                            {
                                BOOST_THROW_EXCEPTION(InternalError("failed to get admin contact"));
                            }
                                admin_contact_id = static_cast<unsigned long long>(lock_res[0][0]);
                        }

                        Database::QueryParams params_i = params;//query params
                        std::stringstream sql_i;
                        sql_i << sql.str();

                        params_i.push_back(admin_contact_id);
                        sql_i << " $" << params_i.size() << "::integer)";

                        try
                        {
                            ctx.get_conn().exec_params(sql_i.str(), params_i);
                        }
                        catch(const std::exception& ex)
                        {
                            std::string what_string(ex.what());
                            if(what_string.find("domain_contact_map_pkey") != std::string::npos)
                                create_domain_exception.add_already_set_admin_contact_handle(*i);
                            else
                                throw;
                        }
                    }//for i
                }//if admin contacts
            }

            //check exception
            if(create_domain_exception.is_set_vector_of_unknown_admin_contact_handle()
            || create_domain_exception.is_set_vector_of_already_set_admin_contact_handle()
            ) BOOST_THROW_EXCEPTION(create_domain_exception);

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
                Database::Result update_historyid_res = ctx.get_conn().exec_params(
                    "UPDATE object_registry SET historyid = $1::bigint, crhistoryid = $1::bigint "
                        " WHERE id = $2::integer RETURNING id"
                        , Database::query_param_list(history_id)(object_id));
                if (update_historyid_res.size() != 1)
                {
                    BOOST_THROW_EXCEPTION(Fred::InternalError("update historyid failed"));
                }

                //domain_history
                ctx.get_conn().exec_params(
                    "INSERT INTO domain_history(historyid,id,zone, registrant, nsset, exdate, keyset) "
                    " SELECT $1::bigint, id, zone, registrant, nsset, exdate, keyset FROM domain "
                        " WHERE id = $2::integer"
                        , Database::query_param_list(history_id)(object_id));

                //domain_contact_map_history
                ctx.get_conn().exec_params(
                    "INSERT INTO domain_contact_map_history(historyid,domainid,contactid, role) "
                    " SELECT $1::bigint, domainid,contactid, role FROM domain_contact_map "
                        " WHERE domainid = $2::integer"
                        , Database::query_param_list(history_id)(object_id));

                //enumval_history
                ctx.get_conn().exec_params(
                    "INSERT INTO enumval_history(historyid,domainid,exdate, publish) "
                    " SELECT $1::bigint, domainid,exdate, publish FROM enumval "
                        " WHERE domainid = $2::integer"
                        , Database::query_param_list(history_id)(object_id));

            }//save history


        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }

        return timestamp;
    }

    std::ostream& operator<<(std::ostream& os, const CreateDomain& i)
    {
        os << "#CreateDomain fqdn: " << i.fqdn_
            << " registrar: " << i.registrar_
            << " authinfo: " << i.authinfo_.print_quoted()
            << " registrant: " << i.registrant_
            << " nsset: " << (i.nsset_.isset() ? i.nsset_.get_value().print_quoted() : i.nsset_.print_quoted())
            << " keyset: " << (i.keyset_.isset() ? i.keyset_.get_value().print_quoted() : i.keyset_.print_quoted())
            ;
        if(!i.admin_contacts_.empty()) os << " admin_contacts: ";
        for(std::vector<std::string>::const_iterator ci = i.admin_contacts_.begin()
                ; ci != i.admin_contacts_.end() ; ++ci ) os << *ci;
        os << " expiration_period: " << i.expiration_period_.print_quoted()
            << " logd_request_id: " << i.logd_request_id_.print_quoted();
        return os;
    }

    std::string CreateDomain::to_string()
    {
        std::stringstream ss;
        ss << *this;
        return ss.str();
    }

}//namespace Fred

