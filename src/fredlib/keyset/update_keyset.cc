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
 *  keyset update
 */

#include <string>
#include <vector>
#include <algorithm>

#include "src/fredlib/keyset/update_keyset.h"
#include "src/fredlib/keyset/copy_history_impl.h"
#include "src/fredlib/object/object.h"
#include "src/fredlib/object/object_impl.h"
#include "src/fredlib/registrar/registrar_impl.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/db_settings.h"
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
            //check registrar
            Registrar::get_registrar_id_by_handle(
                ctx, registrar_, static_cast<Exception*>(0)//set throw
                , &Exception::set_unknown_registrar_handle);

            //lock row and get keyset_id
            unsigned long long keyset_id = get_object_id_by_handle_and_type_with_lock(
                    ctx, true, handle_,"keyset",static_cast<Exception*>(0),
                    &Exception::set_unknown_keyset_handle);

            Exception update_keyset_exception;

            try
            {
                history_id = Fred::UpdateObject(handle_,"keyset", registrar_
                    , authinfo_, logd_request_id_
                ).exec(ctx);
            }
            catch(const Fred::UpdateObject::Exception& ex)
            {
                bool caught_exception_has_been_handled = false;

                if( ex.is_set_unknown_object_handle() ) {
                    update_keyset_exception.set_unknown_keyset_handle( ex.get_unknown_object_handle() );
                    caught_exception_has_been_handled = true;
                }

                if( ex.is_set_unknown_registrar_handle() ) {
                    update_keyset_exception.set_unknown_registrar_handle( ex.get_unknown_registrar_handle() );
                    caught_exception_has_been_handled = true;
                }

                if( ! caught_exception_has_been_handled ) {
                    throw;
                }
            }

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
                    //lock object_registry row for share
                    unsigned long long tech_contact_id = get_object_id_by_handle_and_type_with_lock(
                            ctx, false, *i,"contact",&update_keyset_exception,
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
                        if(what_string.find("keyset_contact_map_pkey") != std::string::npos)
                        {
                            update_keyset_exception.add_already_set_technical_contact_handle(*i);
                            ctx.get_conn().exec("ROLLBACK TO SAVEPOINT add_tech_contact");
                        }
                        else {
                            throw;
                        }
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
                    unsigned long long tech_contact_id = get_object_id_by_handle_and_type_with_lock(
                            ctx, false,*i,"contact",&update_keyset_exception,
                            &Exception::add_unknown_technical_contact_handle);
                    if(tech_contact_id == 0) continue;

                    Database::QueryParams params_i = params;//query params
                    std::stringstream sql_i;
                    sql_i << sql.str();

                    params_i.push_back(tech_contact_id);
                    sql_i << "contactid = $" << params_i.size() << "::integer "
                            " RETURNING keysetid";

                    Database::Result keyset_del_res = ctx.get_conn().exec_params(sql_i.str(), params_i);
                    if (keyset_del_res.size() == 0)
                    {
                        update_keyset_exception.add_unassigned_technical_contact_handle(*i);
                        continue;//for rem_tech_contact_
                    }
                    if (keyset_del_res.size() != 1)
                    {
                        BOOST_THROW_EXCEPTION(InternalError("failed to delete technical contact"));
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
                    if (rem_dns_key_res.size() == 0)
                    {
                        update_keyset_exception.add_unassigned_dns_key(*i);
                        continue;//for rem_dns_key_
                    }
                    if (rem_dns_key_res.size() != 1)
                    {
                        BOOST_THROW_EXCEPTION(InternalError("failed to delete DNS key"));
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
                        ctx.get_conn().exec("SAVEPOINT add_dns_key");
                        ctx.get_conn().exec_params(
                        "INSERT INTO dnskey (keysetid, flags, protocol, alg, key) VALUES($1::integer "
                        ", $2::integer, $3::integer, $4::integer, $5::text)"
                        , Database::query_param_list(keyset_id)(i->get_flags())(i->get_protocol())(i->get_alg())(i->get_key()));
                        ctx.get_conn().exec("RELEASE SAVEPOINT add_dns_key");
                    }
                    catch(const std::exception& ex)
                    {
                        std::string what_string(ex.what());
                        if(what_string.find("dnskey_unique_key") != std::string::npos)
                        {
                            update_keyset_exception.add_already_set_dns_key(*i);
                            ctx.get_conn().exec("ROLLBACK TO SAVEPOINT add_dns_key");
                        }
                        else {
                            throw;
                        }
                    }
                }//for i
            }//if add dns keys

            //check exception
            if(update_keyset_exception.throw_me()) {
                BOOST_THROW_EXCEPTION(update_keyset_exception);
            }

            copy_keyset_data_to_keyset_history_impl(ctx, keyset_id, history_id);

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return history_id;
    }//UpdateKeyset::exec

    std::string UpdateKeyset::to_string() const
    {
        return Util::format_operation_state("UpdateKeyset",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("handle",handle_))
        (std::make_pair("registrar",registrar_))
        (std::make_pair("authinfo",authinfo_.print_quoted()))
        (std::make_pair("add_tech_contact",Util::format_container(add_tech_contact_)))
        (std::make_pair("rem_tech_contact",Util::format_container(rem_tech_contact_)))
        (std::make_pair("add_dns_key", Util::format_container(add_dns_key_)))
        (std::make_pair("rem_dns_key", Util::format_container(rem_dns_key_)))
        (std::make_pair("logd_request_id",logd_request_id_.print_quoted()))
        );
    }
}//namespace Fred
