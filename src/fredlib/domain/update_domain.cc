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
 *  domain update
 */

#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "src/fredlib/domain/update_domain.h"
#include "src/fredlib/domain/domain_name.h"
#include "src/fredlib/zone/zone.h"
#include "src/fredlib/object/object.h"
#include "src/fredlib/registrar/registrar_impl.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/db_settings.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"
#include "util/printable.h"

namespace Fred
{
    UpdateDomain::UpdateDomain(const std::string& fqdn
            , const std::string& registrar)
    : fqdn_(fqdn)
    , registrar_(registrar)
    {}

    UpdateDomain::UpdateDomain(const std::string& fqdn
            , const std::string& registrar
            , const Optional<std::string>& sponsoring_registrar
            , const Optional<std::string>& registrant
            , const Optional<std::string>& authinfo
            , const Optional<Nullable<std::string> >& nsset
            , const Optional<Nullable<std::string> >& keyset
            , const std::vector<std::string>& add_admin_contact
            , const std::vector<std::string>& rem_admin_contact
            , const Optional<boost::gregorian::date>& expiration_date
            , const Optional<boost::gregorian::date>& enum_validation_expiration
            , const Optional<bool>& enum_publish_flag
            , const Optional<unsigned long long> logd_request_id
            )
    : fqdn_(fqdn)
    , registrar_(registrar)
    , sponsoring_registrar_(sponsoring_registrar)
    , registrant_(registrant)
    , authinfo_(authinfo)
    , nsset_(nsset)
    , keyset_(keyset)
    , add_admin_contact_(add_admin_contact)
    , rem_admin_contact_(rem_admin_contact)
    , expiration_date_(expiration_date)
    , enum_validation_expiration_(enum_validation_expiration)
    , enum_publish_flag_(enum_publish_flag)
    , logd_request_id_(logd_request_id.isset()
        ? Nullable<unsigned long long>(logd_request_id.get_value())
        : Nullable<unsigned long long>())//is NULL if not set
    {}

    UpdateDomain& UpdateDomain::set_sponsoring_registrar(const std::string& sponsoring_registrar)
    {
        sponsoring_registrar_ = sponsoring_registrar;
        return *this;
    }

    UpdateDomain& UpdateDomain::set_registrant(const std::string& registrant)
    {
        registrant_ = registrant;
        return *this;
    }

    UpdateDomain& UpdateDomain::set_authinfo(const std::string& authinfo)
    {
        authinfo_ = authinfo;
        return *this;
    }

    UpdateDomain& UpdateDomain::set_nsset(const Nullable<std::string>& nsset)
    {
        nsset_ = nsset;
        return *this;
    }

    UpdateDomain& UpdateDomain::set_nsset(const std::string& nsset)
    {
        nsset_ = Nullable<std::string>(nsset);
        return *this;
    }

    UpdateDomain& UpdateDomain::unset_nsset()
    {
        nsset_ = Nullable<std::string>();
        return *this;
    }

    UpdateDomain& UpdateDomain::set_keyset(const Nullable<std::string>& keyset)
    {
        keyset_ = keyset;
        return *this;
    }

    UpdateDomain& UpdateDomain::set_keyset(const std::string& keyset)
    {
        keyset_ = Nullable<std::string>(keyset);
        return *this;
    }

    UpdateDomain& UpdateDomain::unset_keyset()
    {
        keyset_ = Nullable<std::string>();
        return *this;
    }

    UpdateDomain& UpdateDomain::add_admin_contact(const std::string& admin_contact)
    {
        add_admin_contact_.push_back(admin_contact);
        return *this;
    }

    UpdateDomain& UpdateDomain::rem_admin_contact(const std::string& admin_contact)
    {
        rem_admin_contact_.push_back(admin_contact);
        return *this;
    }

    UpdateDomain& UpdateDomain::set_domain_expiration(const boost::gregorian::date& exdate)
    {
        expiration_date_ = exdate;
        return *this;
    }

    UpdateDomain& UpdateDomain::set_enum_validation_expiration(const boost::gregorian::date& valexdate)
    {
        enum_validation_expiration_ = valexdate;
        return *this;
    }

    UpdateDomain& UpdateDomain::set_enum_publish_flag(bool enum_publish_flag)
    {
        enum_publish_flag_ = enum_publish_flag;
        return *this;
    }

    UpdateDomain& UpdateDomain::set_logd_request_id(unsigned long long logd_request_id)
    {
        logd_request_id_ = logd_request_id;
        return *this;
    }

    unsigned long long UpdateDomain::exec(OperationContext& ctx)
    {
        unsigned long long history_id=0;//return
        try
        {
            //check registrar exists
            Registrar::get_registrar_id_by_handle(
                ctx, registrar_, static_cast<Exception*>(0)//set throw
                , &Exception::set_unknown_registrar_handle);

            //remove optional root dot from fqdn
            std::string no_root_dot_fqdn = Fred::Zone::rem_trailing_dot(fqdn_);

        //get domain_id, ENUM flag and lock object_registry row for update
        unsigned long long domain_id =0;
        bool is_enum_zone = false;
        {
            Database::Result domain_res = ctx.get_conn().exec_params(
                "SELECT oreg.id, z.enum_zone FROM domain d "
                " JOIN zone z ON z.id = d.zone "
                " JOIN object_registry oreg ON d.id = oreg.id "
                " JOIN enum_object_type eot ON oreg.type = eot.id AND eot.name = 'domain' "
                " WHERE oreg.name = LOWER($1::text) AND oreg.erdate IS NULL "
                " FOR UPDATE OF oreg"
                , Database::query_param_list(no_root_dot_fqdn));

            if (domain_res.size() == 0)
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_domain_fqdn(fqdn_));
            }
            if (domain_res.size() != 1)
            {
                BOOST_THROW_EXCEPTION(InternalError("failed to get domain"));
            }

            domain_id = static_cast<unsigned long long>(domain_res[0][0]);
            is_enum_zone = static_cast<bool>(domain_res[0][1]);
        }

        if (!is_enum_zone)//check ENUM specific parameters
        {
            if(enum_validation_expiration_.isset())
                BOOST_THROW_EXCEPTION(InternalError("enum_validation_expiration set for non-ENUM domain"));
            if(enum_publish_flag_.isset())
                BOOST_THROW_EXCEPTION(InternalError("enum_publish_flag set for non-ENUM domain"));
        }

        Exception update_domain_exception;

        try
        {
            //update object
            history_id = Fred::UpdateObject(no_root_dot_fqdn,"domain", registrar_
                , sponsoring_registrar_, authinfo_, logd_request_id_
                ).exec(ctx);
        }
        catch(const Fred::UpdateObject::Exception& ex)
        {
            if(ex.is_set_unknown_object_handle())
            {
                update_domain_exception.set_unknown_domain_fqdn(
                        ex.get_unknown_object_handle());
            }

            if(ex.is_set_unknown_registrar_handle())
            {
                update_domain_exception.set_unknown_registrar_handle(
                        ex.get_unknown_registrar_handle());
            }

            if(ex.is_set_unknown_sponsoring_registrar_handle())
            {
                update_domain_exception.set_unknown_sponsoring_registrar_handle(
                        ex.get_unknown_sponsoring_registrar_handle());
            }
        }
        //update domain
        if(nsset_.isset() || keyset_.isset() || registrant_.isset() || expiration_date_.isset())
        {
            Database::QueryParams params;//query params
            std::stringstream sql;
            Util::HeadSeparator set_separator(" SET "," , ");

            sql <<"UPDATE domain ";

            if(nsset_.isset())//change nsset
            {
                Nullable<std::string> new_nsset_value = nsset_.get_value();
                if(new_nsset_value.isnull())
                {
                    params.push_back(Database::NullQueryParam);//NULL, no nsset
                }
                else
                {
                    //lock nsset object_registry row for update and get id
                    unsigned long long nsset_id = get_object_id_by_handle_and_type_with_lock(
                            ctx,new_nsset_value.get_value(),"nsset",&update_domain_exception,
                            &Exception::set_unknown_nsset_handle);

                    params.push_back(nsset_id); //nsset update
                }
                sql << set_separator.get() << " nsset = $"
                    << params.size() << "::integer ";
            }//if change nsset

            if(keyset_.isset())//change keyset
            {
                Nullable<std::string> new_keyset_value = keyset_.get_value();
                if(new_keyset_value.isnull())
                {
                    params.push_back(Database::NullQueryParam);//NULL, no nsset
                }
                else
                {
                    //lock keyset object_registry row for update and get id
                    unsigned long long keyset_id = get_object_id_by_handle_and_type_with_lock(
                            ctx,new_keyset_value.get_value(),"keyset",&update_domain_exception,
                            &Exception::set_unknown_keyset_handle);

                    params.push_back(keyset_id); //keyset update
                }
                sql << set_separator.get() << " keyset = $"
                    << params.size() << "::integer ";
            }//if change keyset

            if(registrant_.isset())//change registrant
            {
                //lock object_registry row for update
                unsigned long long registrant_id = get_object_id_by_handle_and_type_with_lock(
                        ctx,registrant_.get_value(),"contact",&update_domain_exception,
                        &Exception::set_unknown_registrant_handle);

                params.push_back(registrant_id);
                sql << set_separator.get() << " registrant = $"
                    << params.size() << "::integer ";
            }//if change registrant

            if(expiration_date_.isset())
            {
                if(expiration_date_.get_value().is_special())
                {
                    update_domain_exception.set_invalid_expiration_date(expiration_date_.get_value());
                }

                params.push_back(expiration_date_.get_value());
                sql << set_separator.get() << " exdate = $"
                    << params.size() << "::date ";
            }//if change exdate

            //check exception
            if(update_domain_exception.throw_me())
                BOOST_THROW_EXCEPTION(update_domain_exception);

            params.push_back(domain_id);
            sql << " WHERE id = $" << params.size() << "::integer RETURNING id";
            Database::Result update_domain_res = ctx.get_conn().exec_params(sql.str(), params);
            if (update_domain_res.size() != 1)
            {
                BOOST_THROW_EXCEPTION(InternalError("failed to update domain"));
            }

        }//if update domain

        //add admin contacts
        if(!add_admin_contact_.empty())
        {
            Database::QueryParams params;//query params
            std::stringstream sql;

            params.push_back(domain_id);
            sql << "INSERT INTO domain_contact_map(domainid, contactid) "
                    " VALUES ($" << params.size() << "::integer, ";

            for(std::vector<std::string>::iterator i = add_admin_contact_.begin(); i != add_admin_contact_.end(); ++i)
            {
                //lock object_registry row for update and get id

                unsigned long long admin_contact_id = get_object_id_by_handle_and_type_with_lock(
                        ctx,*i,"contact",&update_domain_exception,
                        &Exception::add_unknown_admin_contact_handle);
                if(admin_contact_id == 0) continue;

                Database::QueryParams params_i = params;//query params
                std::stringstream sql_i;
                sql_i << sql.str();

                params_i.push_back(admin_contact_id);

                sql_i << " $" << params_i.size() << "::integer) ";
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
                        update_domain_exception.add_already_set_admin_contact_handle(*i);
                        ctx.get_conn().exec("ROLLBACK TO SAVEPOINT admin_contact");
                    }
                    else
                        throw;
                }
            }//for i
        }//if add admin contacts

        //delete admin contacts
        if(!rem_admin_contact_.empty())
        {
            Database::QueryParams params;//query params
            std::stringstream sql;

            params.push_back(domain_id);
            sql << "DELETE FROM domain_contact_map WHERE domainid = $" << params.size() << "::integer AND ";

            for(std::vector<std::string>::iterator i = rem_admin_contact_.begin(); i != rem_admin_contact_.end(); ++i)
            {
                //lock object_registry row for update and get id

                unsigned long long admin_contact_id = get_object_id_by_handle_and_type_with_lock(
                        ctx,*i,"contact",&update_domain_exception,
                        &Exception::add_unknown_admin_contact_handle);
                if(admin_contact_id == 0) continue;

                Database::QueryParams params_i = params;//query params
                std::stringstream sql_i;
                sql_i << sql.str();

                params_i.push_back(admin_contact_id);
                sql_i << "contactid = $" << params_i.size() << "::integer "
                        " RETURNING domainid";
                Database::Result domain_del_res = ctx.get_conn().exec_params(sql_i.str(), params_i);
                if (domain_del_res.size() == 0)
                {
                    update_domain_exception.add_unassigned_admin_contact_handle(*i);
                    continue;//for rem_admin_contact_
                }
                if (domain_del_res.size() > 1)
                {
                    BOOST_THROW_EXCEPTION(InternalError("failed to unassign admin contact"));
                }

            }//for i
        }//if delete admin contacts

        //check valexdate if set
        if(enum_validation_expiration_.isset() && enum_validation_expiration_.get_value().is_special())
        {
            update_domain_exception.set_invalid_enum_validation_expiration_date(enum_validation_expiration_.get_value());
        }

        //check exception
        if(update_domain_exception.throw_me())
            BOOST_THROW_EXCEPTION(update_domain_exception);


        //update enumval
        if(enum_validation_expiration_.isset() || enum_publish_flag_.isset())
        {
            Database::QueryParams params;//query params
            std::stringstream sql;
            Util::HeadSeparator set_separator(" SET "," , ");

            sql <<"UPDATE enumval ";

            if(enum_validation_expiration_.isset())
            {
                params.push_back(enum_validation_expiration_.get_value());
                sql << set_separator.get() << " exdate = $"
                    << params.size() << "::date ";
            }

            if(enum_publish_flag_.isset())
            {
                params.push_back(enum_publish_flag_.get_value());
                sql << set_separator.get() << " publish = $"
                    << params.size() << "::boolean ";
            }

            params.push_back(domain_id);
            sql << " WHERE domainid = $" << params.size() << "::integer RETURNING domainid";

            Database::Result update_enumval_res = ctx.get_conn().exec_params(sql.str(), params);
            if (update_enumval_res.size() != 1)
            {
                BOOST_THROW_EXCEPTION(InternalError("failed to update enumval"));
            }
        }

        //save history
        {
            //domain_history
            ctx.get_conn().exec_params(
                "INSERT INTO domain_history(historyid,id,zone, registrant, nsset, exdate, keyset) "
                " SELECT $1::bigint, id, zone, registrant, nsset, exdate, keyset FROM domain "
                    " WHERE id = $2::integer"
                    , Database::query_param_list(history_id)(domain_id));

            //domain_contact_map_history
            ctx.get_conn().exec_params(
                "INSERT INTO domain_contact_map_history(historyid,domainid,contactid, role) "
                " SELECT $1::bigint, domainid,contactid, role FROM domain_contact_map "
                    " WHERE domainid = $2::integer"
                    , Database::query_param_list(history_id)(domain_id));

            //enumval_history
            ctx.get_conn().exec_params(
                "INSERT INTO enumval_history(historyid,domainid,exdate, publish) "
                " SELECT $1::bigint, domainid,exdate, publish FROM enumval "
                    " WHERE domainid = $2::integer"
                    , Database::query_param_list(history_id)(domain_id));
        }//save history

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }

        return history_id;
    }//UpdateDomain::exec

    std::string UpdateDomain::to_string() const
    {
        return Util::format_operation_state("UpdateDomain",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("fqdn",fqdn_))
        (std::make_pair("registrar",registrar_))
        (std::make_pair("sponsoring_registrar",sponsoring_registrar_.print_quoted()))
        (std::make_pair("registrant",registrant_.print_quoted()))
        (std::make_pair("authinfo",authinfo_.print_quoted()))
        (std::make_pair("nsset",nsset_.isset() ? nsset_.get_value().print_quoted() : nsset_.print_quoted()))
        (std::make_pair("keyset",keyset_.isset() ? keyset_.get_value().print_quoted() : keyset_.print_quoted()))
        (std::make_pair("add_admin_contact", Util::format_vector(add_admin_contact_)))
        (std::make_pair("rem_admin_contact", Util::format_vector(rem_admin_contact_)))
        (std::make_pair("expiration_date",expiration_date_.print_quoted()))
        (std::make_pair("enum_validation_expiration",enum_validation_expiration_.print_quoted()))
        (std::make_pair("enum_publish_flag",enum_publish_flag_.print_quoted()))
        (std::make_pair("logd_request_id",logd_request_id_.print_quoted()))
        );
    }

}//namespace Fred

