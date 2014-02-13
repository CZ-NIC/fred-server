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
 *  create domain
 */


#include <string>
#include <vector>
#include <set>

#include <boost/algorithm/string.hpp>

#include "src/fredlib/domain/create_domain.h"
#include "src/fredlib/domain/domain_name.h"
#include "src/fredlib/domain/check_domain.h"
#include "src/fredlib/zone/zone.h"
#include "src/fredlib/object/object.h"
#include "src/fredlib/object/object_impl.h"
#include "src/fredlib/registrar/registrar_impl.h"

#include "src/fredlib/opcontext.h"
#include "src/fredlib/db_settings.h"
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
            , const Optional<boost::gregorian::date>& expiration_date
            , const Optional<boost::gregorian::date>& enum_validation_expiration
            , const Optional<bool>& enum_publish_flag
            , const Optional<unsigned long long> logd_request_id
            )
    : fqdn_(fqdn)
    , registrar_(registrar)
    , authinfo_(authinfo)
    , registrant_(registrant)
    , nsset_(nsset)
    , keyset_(keyset)
    , admin_contacts_(admin_contacts)
    , expiration_date_(expiration_date)
    , enum_validation_expiration_(enum_validation_expiration)
    , enum_publish_flag_(enum_publish_flag)
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

    CreateDomain& CreateDomain::set_expiration_date(const Optional<boost::gregorian::date>& expiration_date)
    {
        expiration_date_ = expiration_date;
        return *this;
    }

    CreateDomain& CreateDomain::set_enum_validation_expiration(const boost::gregorian::date& valexdate)
    {
        enum_validation_expiration_ = valexdate;
        return *this;
    }

    CreateDomain& CreateDomain::set_enum_publish_flag(bool enum_publish_flag)
    {
        enum_publish_flag_ = enum_publish_flag;
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
            Registrar::get_registrar_id_by_handle(
                ctx, registrar_, static_cast<Exception*>(0)//set throw
                , &Exception::set_unknown_registrar_handle);

            //check general domain name syntax
            if(!Domain::general_domain_name_syntax_check(fqdn_))
            {
                BOOST_THROW_EXCEPTION(Exception().set_invalid_fqdn_syntax(fqdn_));
            }

            //remove optional root dot from fqdn
            std::string no_root_dot_fqdn = Fred::Zone::rem_trailing_dot(fqdn_);

            //check zone
            if(CheckDomain(fqdn_).is_bad_zone(ctx))
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_zone_fqdn(fqdn_));
            }

            //check domain name
            if(CheckDomain(fqdn_).is_invalid_handle(ctx))
            {
                BOOST_THROW_EXCEPTION(Exception().set_invalid_fqdn_syntax(fqdn_));
            }

            //get zone
            Zone::Data zone;
            try
            {
                zone = Zone::find_zone_in_fqdn(ctx, no_root_dot_fqdn);
            }
            catch(const Zone::Exception& ex)
            {
                if(ex.is_set_unknown_zone_in_fqdn()
                        && (ex.get_unknown_zone_in_fqdn().compare(no_root_dot_fqdn) == 0))
                {
                    BOOST_THROW_EXCEPTION(Exception().set_unknown_zone_fqdn(fqdn_));
                }
                else
                    throw;
            }

            if (zone.is_enum)//check ENUM specific parameters
            {
                if((!enum_validation_expiration_.isset()))
                    BOOST_THROW_EXCEPTION(InternalError("enum_validation_expiration not set for ENUM domain"));
            }
            else
            {
                if(enum_validation_expiration_.isset())
                    BOOST_THROW_EXCEPTION(InternalError("enum_validation_expiration set for non-ENUM domain"));
                if(enum_publish_flag_.isset())
                    BOOST_THROW_EXCEPTION(InternalError("enum_publish_flag set for not-ENUM domain"));
            }

            CreateObjectOutput create_object_output = CreateObject("domain", no_root_dot_fqdn, registrar_, authinfo_, logd_request_id_).exec(ctx);

            //expiration_period
            unsigned expiration_period = zone.ex_period_min;//in months

            //get crdate and exdate and lock row from object_registry
            {
                Database::Result reg_date_res = ctx.get_conn().exec_params(
                    "SELECT crdate::timestamp AT TIME ZONE 'UTC' AT TIME ZONE $1::text "
                    " , (crdate::timestamp AT TIME ZONE 'UTC' AT TIME ZONE $1::text + ( $3::integer * interval '1 month') )::date "
                    "  FROM object_registry "
                    " WHERE id = $2::bigint FOR UPDATE OF object_registry"
                    , Database::query_param_list(returned_timestamp_pg_time_zone_name)(create_object_output.object_id)(expiration_period));

                if (reg_date_res.size() != 1)
                {
                    BOOST_THROW_EXCEPTION(Fred::InternalError("timestamp of the domain creation was not found"));
                }

                timestamp = boost::posix_time::time_from_string(std::string(reg_date_res[0][0]));
                if(!expiration_date_.isset())
                {
                    expiration_date_ = boost::gregorian::from_simple_string(std::string(reg_date_res[0][1]));
                }
            }

            Exception create_domain_exception;

            //lock registrant object_registry row for update and get id
            unsigned long long registrant_id = get_object_id_by_handle_and_type_with_lock(
                    ctx,registrant_,"contact",&create_domain_exception,
                    &Exception::set_unknown_registrant_handle);

            //create domain
            {
                Database::QueryParams params;//query params
                std::stringstream col_sql, val_sql;
                Util::HeadSeparator col_separator("",", "), val_separator("",", ");

                col_sql <<"INSERT INTO domain (";
                val_sql << " VALUES (";

                //id
                params.push_back(create_object_output.object_id);
                col_sql << col_separator.get() << "id";
                val_sql << val_separator.get() << "$" << params.size() <<"::integer";

                //zone id
                params.push_back(zone.id);
                col_sql << col_separator.get() << "zone";
                val_sql << val_separator.get() << "$" << params.size() <<"::integer";

                //expiration_date
                if(expiration_date_.get_value().is_special())
                {
                    create_domain_exception.set_invalid_expiration_date(expiration_date_.get_value());
                }
                else //if expiration_date_ ok
                {
                    params.push_back(expiration_date_.get_value());
                    col_sql << col_separator.get() << "exdate";
                    val_sql << val_separator.get() << "$" << params.size() <<"::date";
                }

                //set registrant
                params.push_back(registrant_id);
                col_sql << col_separator.get() << "registrant";
                val_sql << val_separator.get() << "$" << params.size() <<"::integer";

                //nsset
                if(nsset_.isset())
                {
                    Nullable<std::string> new_nsset_value = nsset_.get_value();
                    col_sql << col_separator.get() << "nsset";

                    if(new_nsset_value.isnull())
                    {//null case query
                        params.push_back(Database::NullQueryParam);//NULL
                        val_sql << val_separator.get() << "$" << params.size() <<"::integer";
                    }
                    else
                    {//value case query, lock nsset object_registry row for update and get id
                        unsigned long long nsset_id = get_object_id_by_handle_and_type_with_lock(
                            ctx,new_nsset_value.get_value(),"nsset",&create_domain_exception,
                            &Exception::set_unknown_nsset_handle);

                        params.push_back(nsset_id);//id
                        val_sql << val_separator.get() << "$" << params.size() <<"::integer";

                    }
                }//if nsset

                //keyset
                if(keyset_.isset())
                {
                    Nullable<std::string> new_keyset_value = keyset_.get_value();
                    col_sql << col_separator.get() << "keyset";

                    if(new_keyset_value.isnull())
                    {//null case query
                        params.push_back(Database::NullQueryParam);//NULL
                        val_sql << val_separator.get() << "$" << params.size() <<"::integer";
                    }
                    else
                    {//value case query, lock keyset object_registry row for update and get id
                        unsigned long long keyset_id = get_object_id_by_handle_and_type_with_lock(
                                ctx,new_keyset_value.get_value(),"keyset",&create_domain_exception,
                                &Exception::set_unknown_keyset_handle);

                        params.push_back(keyset_id);//id
                        val_sql << val_separator.get() << "$" << params.size() <<"::integer";

                    }
                }//if keyset

                col_sql <<")";
                val_sql << ")";

                bool domain_ok = false;//domain params ok
                //insert into domain if ok
                if(!create_domain_exception.throw_me())
                {
                    ctx.get_conn().exec_params(col_sql.str() + val_sql.str(), params);
                    domain_ok = true;
                }

                //set admin contacts
                if(!admin_contacts_.empty())
                {
                    Database::QueryParams params;//query params
                    std::stringstream sql;

                    params.push_back(create_object_output.object_id);
                    sql << "INSERT INTO domain_contact_map(domainid, contactid) "
                            " VALUES ($" << params.size() << "::integer, ";

                    std::set<unsigned long long> admin_contact_id_duplicity_check;//in case we have no domain created

                    for(std::vector<std::string>::iterator i = admin_contacts_.begin(); i != admin_contacts_.end(); ++i)
                    {
                        //lock admin contact object_registry row for update and get id
                        unsigned long long admin_contact_id = get_object_id_by_handle_and_type_with_lock(
                                ctx,*i,"contact",&create_domain_exception,
                                &Exception::add_unknown_admin_contact_handle);
                        if(admin_contact_id == 0) continue;

                        Database::QueryParams params_i = params;//query params
                        std::stringstream sql_i;
                        sql_i << sql.str();

                        params_i.push_back(admin_contact_id);
                        sql_i << " $" << params_i.size() << "::integer)";

                        if(domain_ok)
                        {//assign admin contact to domain
                            try
                            {
                                ctx.get_conn().exec("SAVEPOINT admin_contact");
                                ctx.get_conn().exec_params(sql_i.str(), params_i);
                                ctx.get_conn().exec("RELEASE SAVEPOINT admin_contact");
                            }
                            catch(const std::exception& ex)
                            {
                                std::string what_string(ex.what());
                                if(what_string.find("domain_contact_map_pkey") != std::string::npos)
                                {
                                    create_domain_exception.add_already_set_admin_contact_handle(*i);
                                    ctx.get_conn().exec("ROLLBACK TO SAVEPOINT admin_contact");
                                }
                                else
                                    throw;
                            }
                        }
                        else
                        {//create domain failed - admin_contact duplicity check only
                            if(admin_contact_id_duplicity_check.insert(admin_contact_id).second == false)
                            {
                                create_domain_exception.add_already_set_admin_contact_handle(*i);
                            }
                        }
                    }//for i
                }//if admin contacts
            }

            //check valexdate if set
            if(enum_validation_expiration_.isset() && enum_validation_expiration_.get_value().is_special())
            {
                create_domain_exception.set_invalid_enum_validation_expiration_date(enum_validation_expiration_.get_value());
            }

            //check exception
            if(create_domain_exception.throw_me())
                BOOST_THROW_EXCEPTION(create_domain_exception);

            if(zone.is_enum)//if ENUM domain, insert enumval
            {
                Database::QueryParams params;//query params
                std::stringstream col_sql, val_sql;
                Util::HeadSeparator col_separator("",", "), val_separator("",", ");

                col_sql <<"INSERT INTO enumval (";
                val_sql << " VALUES (";

                //domainid
                params.push_back(create_object_output.object_id);
                col_sql << col_separator.get() << "domainid";
                val_sql << val_separator.get() << "$" << params.size() <<"::integer";

                //ENUM validation expiration date
                params.push_back(enum_validation_expiration_.get_value());
                col_sql << col_separator.get() << "exdate";
                val_sql << val_separator.get() << "$" << params.size() <<"::date";

                //ENUM publish flag
                if(enum_publish_flag_.isset())
                {
                    params.push_back(enum_publish_flag_.get_value());
                    col_sql << col_separator.get() << "publish";
                    val_sql << val_separator.get() << "$" << params.size() <<"::boolean";
                }
                col_sql <<")";
                val_sql << ")";

                //insert into enumval
                ctx.get_conn().exec_params(col_sql.str() + val_sql.str(), params);
            }

            //save history
            {
                //domain_history
                ctx.get_conn().exec_params(
                    "INSERT INTO domain_history(historyid,id,zone, registrant, nsset, exdate, keyset) "
                    " SELECT $1::bigint, id, zone, registrant, nsset, exdate, keyset FROM domain "
                        " WHERE id = $2::integer"
                        , Database::query_param_list(create_object_output.history_id)(create_object_output.object_id));

                //domain_contact_map_history
                ctx.get_conn().exec_params(
                    "INSERT INTO domain_contact_map_history(historyid,domainid,contactid, role) "
                    " SELECT $1::bigint, domainid,contactid, role FROM domain_contact_map "
                        " WHERE domainid = $2::integer"
                        , Database::query_param_list(create_object_output.history_id)(create_object_output.object_id));

                //enumval_history
                ctx.get_conn().exec_params(
                    "INSERT INTO enumval_history(historyid,domainid,exdate, publish) "
                    " SELECT $1::bigint, domainid,exdate, publish FROM enumval "
                        " WHERE domainid = $2::integer"
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

    std::string CreateDomain::to_string() const
    {
        return Util::format_operation_state("CreateDomain",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("fqdn",fqdn_))
        (std::make_pair("registrar",registrar_))
        (std::make_pair("authinfo",authinfo_.print_quoted()))
        (std::make_pair("registrant",registrant_))
        (std::make_pair("nsset",nsset_.isset() ? nsset_.get_value().print_quoted() : nsset_.print_quoted()))
        (std::make_pair("keyset",keyset_.isset() ? keyset_.get_value().print_quoted() : keyset_.print_quoted()))
        (std::make_pair("admin_contacts",Util::format_vector(admin_contacts_)))
        (std::make_pair("expiration_date",expiration_date_.print_quoted()))
        (std::make_pair("enum_validation_expiration",enum_validation_expiration_.print_quoted()))
        (std::make_pair("enum_publish_flag",enum_publish_flag_.print_quoted()))
        (std::make_pair("logd_request_id",logd_request_id_.print_quoted()))
        );
    }


}//namespace Fred

