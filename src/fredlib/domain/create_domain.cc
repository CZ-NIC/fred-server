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
            //zone
            unsigned long long zone_id = 0;
            {
                std::string domain(boost::to_lower_copy(fqdn_));

                Database::Result zone_res = ctx.get_conn().exec(
                    "SELECT fqdn FROM zone ORDER BY length(fqdn) DESC");

                if (zone_res.size() == 0)
                {
                    throw CDERR("no zone found");
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
                                "SELECT id FROM zone WHERE lower(fqdn)=lower($1::text)"
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
                    std::string errmsg("|| not found zone:fqdn: ");
                    errmsg += boost::replace_all_copy(fqdn_,"|", "[pipe]");//quote pipes
                    errmsg += " |";
                    throw CDEX(errmsg.c_str());
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
                        "SELECT ex_period_min FROM zone WHERE id=$1::bigint"
                        , Database::query_param_list(zone_id));

                    if (ex_period_min_res.size() == 0)
                    {
                        throw CDERR("ex_period_min for zone not found");
                    }

                    expiration_period = static_cast<unsigned>(ex_period_min_res[0][0]);
                }
            }//expiration_period

            unsigned long long object_id = CreateObject("domain", fqdn_, registrar_, authinfo_).exec(ctx);
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

                //expiration_period
                params.push_back(returned_timestamp_pg_time_zone_name);
                params.push_back(expiration_period);
                params.push_back(object_id);
                params.push_back(fqdn_);
                col_sql << col_separator.get() << "exdate";
                val_sql << val_separator.get() <<
                    " raise_exception_ifnull( "
                    " (SELECT (crdate::timestamp AT TIME ZONE 'UTC' AT TIME ZONE $"<< (params.size() - 3) << "::text "
                    " + ( $"<< (params.size() - 2) << "::integer * interval '1 month'))::date "
                    " FROM object_registry WHERE id = $"<< (params.size() -1) << "::integer) "
                    " ,'|| not found crdate:fqdn: '||ex_data($"<< params.size() << "::text)||' |') ";

                //set registrant
                {
                    //lock object_registry row for update
                    {
                        Database::Result lock_res = ctx.get_conn().exec_params(
                            "SELECT oreg.id FROM enum_object_type eot"
                            " JOIN object_registry oreg ON oreg.type = eot.id "
                            " AND oreg.name = UPPER($1::text) AND oreg.erdate IS NULL "
                            " WHERE eot.name = 'contact' FOR UPDATE OF oreg"
                            , Database::query_param_list(registrant_));

                        if (lock_res.size() != 1)
                        {
                            std::string errmsg("unable to lock || not found:registrant: ");
                            errmsg += boost::replace_all_copy(std::string(registrant_),"|", "[pipe]");//quote pipes
                            errmsg += " |";
                            throw CDEX(errmsg.c_str());
                        }
                    }

                    params.push_back(registrant_);

                    col_sql << col_separator.get() << "registrant";
                    val_sql << val_separator.get() <<
                    " raise_exception_ifnull( "
                    " (SELECT oreg.id FROM object_registry oreg JOIN contact c ON oreg.id = c.id "
                    " WHERE oreg.name = UPPER($" << params.size() << "::text) AND oreg.erdate IS NULL) "
                    " ,'|| not found:registrant: '||ex_data($"<< params.size() << "::text)||' |') "; //registrant update
                }//if set registrant

                //nsset
                if(nsset_.isset())
                {
                    Nullable<std::string> new_nsset_value = nsset_;

                    params.push_back(new_nsset_value);//NULL or value via Nullable operator Database::QueryParam()

                    col_sql << col_separator.get() << "nsset";

                    if(new_nsset_value.isnull())
                    {//null case query
                        val_sql << val_separator.get() << "$" << params.size() <<"::integer";
                    }
                    else
                    {//value case query

                        //lock object_registry row for update
                        {
                            Database::Result lock_res = ctx.get_conn().exec_params(
                                "SELECT oreg.id FROM enum_object_type eot"
                                " JOIN object_registry oreg ON oreg.type = eot.id "
                                " AND oreg.name = UPPER($1::text) AND oreg.erdate IS NULL "
                                " WHERE eot.name = 'nsset' FOR UPDATE OF oreg"
                                , Database::query_param_list(nsset_.get_value()));

                            if (lock_res.size() != 1)
                            {
                                std::string errmsg("unable to lock || not found:nsset: ");
                                errmsg += boost::replace_all_copy(std::string(nsset_.get_value()),"|", "[pipe]");//quote pipes
                                errmsg += " |";
                                throw CDEX(errmsg.c_str());
                            }
                        }

                        val_sql << val_separator.get() <<
                        " raise_exception_ifnull((SELECT oreg.id FROM object_registry oreg "
                        " JOIN nsset n ON oreg.id = n.id "
                        " WHERE oreg.name = UPPER($" << params.size() << "::text) AND oreg.erdate IS NULL) "
                        " ,'|| not found:nsset: '||ex_data($"<< params.size() << "::text)||' |') ";
                    }
                }//if nsset

                //keyset
                if(keyset_.isset())
                {
                    Nullable<std::string> new_keyset_value = keyset_;

                    params.push_back(new_keyset_value);//NULL or value via Nullable operator Database::QueryParam()

                    col_sql << col_separator.get() << "keyset";

                    if(new_keyset_value.isnull())
                    {//null case query
                        val_sql << val_separator.get() << "$" << params.size() <<"::integer";
                    }
                    else
                    {//value case query

                        //lock object_registry row for update
                        {
                            Database::Result lock_res = ctx.get_conn().exec_params(
                                "SELECT oreg.id FROM enum_object_type eot"
                                " JOIN object_registry oreg ON oreg.type = eot.id "
                                " AND oreg.name = UPPER($1::text) AND oreg.erdate IS NULL "
                                " WHERE eot.name = 'keyset' FOR UPDATE OF oreg"
                                , Database::query_param_list(keyset_.get_value()));

                            if (lock_res.size() != 1)
                            {
                                std::string errmsg("unable to lock || not found:keyset: ");
                                errmsg += boost::replace_all_copy(std::string(keyset_.get_value()),"|", "[pipe]");//quote pipes
                                errmsg += " |";
                                throw CDEX(errmsg.c_str());
                            }
                        }

                        val_sql << val_separator.get() <<
                        " raise_exception_ifnull((SELECT oreg.id FROM object_registry oreg "
                        " JOIN keyset k ON oreg.id = k.id "
                        " WHERE oreg.name = UPPER($" << params.size() << "::text) AND oreg.erdate IS NULL) "
                        " ,'|| not found:keyset: '||ex_data($"<< params.size() << "::text)||' |') ";
                    }
                }//if keyset

                col_sql <<")";
                val_sql << ")";
                //insert into contact
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
                                std::string errmsg("unable to lock || not found:admin contact: ");
                                errmsg += boost::replace_all_copy(*i,"|", "[pipe]");//quote pipes
                                errmsg += " |";
                                throw CDEX(errmsg.c_str());
                            }
                        }

                        Database::QueryParams params_i = params;//query params
                        std::stringstream sql_i;
                        sql_i << sql.str();

                        params_i.push_back(*i);

                        {//precheck uniqueness
                            Database::Result domain_add_check_res = ctx.get_conn().exec_params(
                            "SELECT domainid, contactid FROM domain_contact_map "
                            " WHERE domainid = $1::bigint "
                            "  AND contactid = raise_exception_ifnull("
                            "    (SELECT oreg.id FROM object_registry oreg "
                            "       JOIN contact c ON oreg.id = c.id "
                            "     WHERE oreg.name = UPPER($2::text) AND oreg.erdate IS NULL) "
                            "     ,'|| not found:admin contact: '||ex_data($2::text)||' |')"
                            , params_i);

                            if (domain_add_check_res.size() == 1)
                            {
                                std::string errmsg("add admin contact precheck uniqueness failed || already set:admin contact: ");
                                errmsg += boost::replace_all_copy(*i,"|", "[pipe]");//quote pipes
                                errmsg += " |";
                                throw CDEX(errmsg.c_str());
                            }
                        }

                        sql_i << " raise_exception_ifnull("
                            " (SELECT oreg.id FROM object_registry oreg JOIN contact c ON oreg.id = c.id "
                            " WHERE oreg.name = UPPER($"<< params_i.size() << "::text) AND oreg.erdate IS NULL)"
                            " , '|| not found:admin contact: '||ex_data($"<< params.size() << "::text)||' |')) "
                            " RETURNING domainid";
                        Database::Result domain_add_check_res = ctx.get_conn().exec_params(sql_i.str(), params_i);
                        if (domain_add_check_res.size() != 1)
                        {
                            std::string errmsg("add admin contact failed || already set:admin contact: ");
                            errmsg += boost::replace_all_copy(*i,"|", "[pipe]");//quote pipes
                            errmsg += " |";
                            throw CDEX(errmsg.c_str());
                        }
                    }//for i
                }//if admin contacts


                //get crdate from object_registry
                {
                    Database::Result crdate_res = ctx.get_conn().exec_params(
                            "SELECT crdate::timestamp AT TIME ZONE 'UTC' AT TIME ZONE $1::text "
                            "  FROM object_registry "
                            " WHERE id = $2::bigint"
                        , Database::query_param_list(returned_timestamp_pg_time_zone_name)(object_id));

                    if (crdate_res.size() != 1)
                    {
                        std::string errmsg("|| not found crdate:fqdn: ");
                        errmsg += boost::replace_all_copy(fqdn_,"|", "[pipe]");//quote pipes
                        errmsg += " |";
                        throw CDEX(errmsg.c_str());
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
                    "UPDATE object_registry SET historyid = $1::bigint, crhistoryid = $1::bigint "
                        " WHERE id = $2::integer"
                        , Database::query_param_list(history_id)(object_id));

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
        catch(...)//common exception processing
        {
            handleOperationExceptions<CreateDomainException>(__FILE__, __LINE__, __ASSERT_FUNCTION);
        }

        return timestamp;
    }

}//namespace Fred

