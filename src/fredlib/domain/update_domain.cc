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
 *  @file update_domain.cc
 *  domain update
 */

#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>

#include "fredlib/domain/update_domain.h"
#include "fredlib/object/object.h"

#include "fredlib/opcontext.h"
#include "fredlib/db_settings.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"

namespace Fred
{
    UpdateDomain::UpdateDomain(const std::string& fqdn
            , const std::string& registrar)
    : fqdn_(fqdn)
    , registrar_(registrar)
    {}

    UpdateDomain::UpdateDomain(const std::string& fqdn
            , const std::string& registrar
            , const Optional<std::string>& registrant
            , const Optional<std::string>& authinfo
            , const Optional<Nullable<std::string> >& nsset
            , const Optional<Nullable<std::string> >& keyset
            , const std::vector<std::string>& add_admin_contact
            , const std::vector<std::string>& rem_admin_contact
            , const Optional<unsigned long long> logd_request_id
            )
    : fqdn_(fqdn)
    , registrar_(registrar)
    , registrant_(registrant)
    , authinfo_(authinfo)
    , nsset_(nsset)
    , keyset_(keyset)
    , add_admin_contact_(add_admin_contact)
    , rem_admin_contact_(rem_admin_contact)
    , logd_request_id_(logd_request_id.isset()
        ? Nullable<unsigned long long>(logd_request_id.get_value())
        : Nullable<unsigned long long>())//is NULL if not set
    {}

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
            //TODO: check registrar access
            {
                Database::Result res = ctx.get_conn().exec_params(
                        "SELECT id FROM registrar WHERE handle = UPPER($1::text) FOR SHARE"
                    , Database::query_param_list(registrar_));

                if (res.size() == 0)
                {
                    BOOST_THROW_EXCEPTION(Exception().set_unknown_registrar_handle(registrar_));
                }
                if (res.size() != 1)
                {
                    BOOST_THROW_EXCEPTION(InternalError("failed to get registrar"));
                }
            }

        //get domain_id and lock object_registry row for update
        unsigned long long domain_id =0;
        {
            Database::Result domain_id_res = ctx.get_conn().exec_params(
                "SELECT oreg.id FROM domain d "
                " JOIN object_registry oreg ON d.id = oreg.id "
                " JOIN enum_object_type eot ON oreg.type = eot.id AND eot.name = 'domain' "
                " WHERE oreg.name = LOWER($1::text) AND oreg.erdate IS NULL "
                " FOR UPDATE OF oreg"
                , Database::query_param_list(fqdn_));

            if (domain_id_res.size() == 0)
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_domain_fqdn(fqdn_));
            }
            if (domain_id_res.size() != 1)
            {
                BOOST_THROW_EXCEPTION(InternalError("failed to get domain"));
            }


            domain_id = domain_id_res[0][0];
        }

        //update object
        Fred::UpdateObject(fqdn_,"domain", registrar_, authinfo_).exec(ctx);

        Exception update_domain_exception;

        //update domain
        if(nsset_.isset() || keyset_.isset() || registrant_.isset())
        {
            Database::QueryParams params;//query params
            std::stringstream sql;
            Util::HeadSeparator set_separator(" SET "," , ");

            sql <<"UPDATE domain ";

            if(nsset_.isset())//change nsset
            {
                Nullable<std::string> new_nsset_value = nsset_;
                if(new_nsset_value.isnull())
                {
                    params.push_back(Database::NullQueryParam);//NULL, no nsset
                }
                else
                {
                    //lock nsset object_registry row for update and get id
                    unsigned long long nsset_id = 0;
                    Database::Result lock_res = ctx.get_conn().exec_params(
                        "SELECT oreg.id FROM enum_object_type eot"
                        " JOIN object_registry oreg ON oreg.type = eot.id "
                        " AND oreg.name = UPPER($1::text) AND oreg.erdate IS NULL "
                        " WHERE eot.name = 'nsset' FOR UPDATE OF oreg"
                        , Database::query_param_list(nsset_.get_value()));
                    if (lock_res.size() == 0)
                    {
                        update_domain_exception.set_unknown_nsset_handle(nsset_.get_value());
                    }
                    if (lock_res.size() > 1)
                    {
                        BOOST_THROW_EXCEPTION(InternalError("failed to get nsset"));
                    }

                    if (lock_res.size() == 1)
                    {
                        nsset_id = static_cast<unsigned long long>(lock_res[0][0]);
                    }
                    params.push_back(nsset_id); //nsset update
                }
                sql << set_separator.get() << " nsset = $"
                    << params.size() << "::integer ";
            }//if change nsset

            if(keyset_.isset())//change keyset
            {
                Nullable<std::string> new_keyset_value = keyset_;
                if(new_keyset_value.isnull())
                {
                    params.push_back(Database::NullQueryParam);//NULL, no nsset
                }
                else
                {
                    //lock keyset object_registry row for update and get id
                    unsigned long long keyset_id = 0;
                    Database::Result lock_res = ctx.get_conn().exec_params(
                        "SELECT oreg.id FROM enum_object_type eot"
                        " JOIN object_registry oreg ON oreg.type = eot.id "
                        " AND oreg.name = UPPER($1::text) AND oreg.erdate IS NULL "
                        " WHERE eot.name = 'keyset' FOR UPDATE OF oreg"
                        , Database::query_param_list(keyset_.get_value()));
                    if (lock_res.size() == 0)
                    {
                        update_domain_exception.set_unknown_keyset_handle(keyset_.get_value());
                    }
                    if (lock_res.size() > 1)
                    {
                        BOOST_THROW_EXCEPTION(InternalError("failed to get keyset"));
                    }
                    if (lock_res.size() == 1)
                    {
                        keyset_id = static_cast<unsigned long long>(lock_res[0][0]);
                    }
                    params.push_back(keyset_id); //keyset update
                }
                sql << set_separator.get() << " keyset = $"
                    << params.size() << "::integer ";
            }//if change keyset

            if(registrant_.isset())//change registrant
            {
                //lock object_registry row for update

                unsigned long long registrant_id = 0;
                Database::Result lock_res = ctx.get_conn().exec_params(
                    "SELECT oreg.id FROM enum_object_type eot"
                    " JOIN object_registry oreg ON oreg.type = eot.id "
                    " AND oreg.name = UPPER($1::text) AND oreg.erdate IS NULL "
                    " WHERE eot.name = 'contact' FOR UPDATE OF oreg"
                    , Database::query_param_list(registrant_.get_value()));
                if (lock_res.size() == 0)
                {
                    update_domain_exception.set_unknown_registrant_handle(registrant_.get_value());
                }
                if (lock_res.size() > 1)
                {
                    BOOST_THROW_EXCEPTION(InternalError("failed to get registrant"));
                }
                if (lock_res.size() == 1)
                {
                    registrant_id = static_cast<unsigned long long>(lock_res[0][0]);
                }
                params.push_back(registrant_id);
                sql << set_separator.get() << " registrant = $"
                    << params.size() << "::integer ";
            }//if change registrant

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

                unsigned long long admin_contact_id = 0;
                Database::Result lock_res = ctx.get_conn().exec_params(
                    "SELECT oreg.id FROM enum_object_type eot"
                    " JOIN object_registry oreg ON oreg.type = eot.id "
                    " AND oreg.name = UPPER($1::text) AND oreg.erdate IS NULL "
                    " WHERE eot.name = 'contact' FOR UPDATE OF oreg"
                    , Database::query_param_list(*i));

                if (lock_res.size() == 0)
                {
                    update_domain_exception.add_unknown_admin_contact_handle(*i);
                    continue;//for add_admin_contact_
                }
                if (lock_res.size() > 1)
                {
                    BOOST_THROW_EXCEPTION(InternalError("failed to get admin contact"));
                }

                admin_contact_id = static_cast<unsigned long long>(lock_res[0][0]);

                Database::QueryParams params_i = params;//query params
                std::stringstream sql_i;
                sql_i << sql.str();

                params_i.push_back(admin_contact_id);

                sql_i << " $" << params_i.size() << "::integer) ";
                try
                {
                    ctx.get_conn().exec("SAVEPOINT admin_contact");
                    ctx.get_conn().exec_params(sql_i.str(), params_i);
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

                unsigned long long admin_contact_id = 0;
                Database::Result lock_res = ctx.get_conn().exec_params(
                    "SELECT oreg.id FROM enum_object_type eot"
                    " JOIN object_registry oreg ON oreg.type = eot.id "
                    " AND oreg.name = UPPER($1::text) AND oreg.erdate IS NULL "
                    " WHERE eot.name = 'contact' FOR UPDATE OF oreg"
                    , Database::query_param_list(*i));

                if (lock_res.size() == 0)
                {
                    update_domain_exception.add_unknown_admin_contact_handle(*i);
                    continue;//for rem_admin_contact_
                }
                if (lock_res.size() > 1)
                {
                    BOOST_THROW_EXCEPTION(InternalError("failed to get admin contact"));
                }

                admin_contact_id = static_cast<unsigned long long>(lock_res[0][0]);

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

        //check exception
        if(update_domain_exception.throw_me())
            BOOST_THROW_EXCEPTION(update_domain_exception);

        //save history
        {
            history_id = Fred::InsertHistory(logd_request_id_).exec(ctx);

            //object_history
            ctx.get_conn().exec_params(
                "INSERT INTO object_history(historyid,id,clid, upid, trdate, update, authinfopw) "
                " SELECT $1::bigint, id,clid, upid, trdate, update, authinfopw FROM object "
                " WHERE id = $2::integer"
                , Database::query_param_list(history_id)(domain_id));

            //object_registry historyid
            Database::Result update_historyid_res = ctx.get_conn().exec_params(
                "UPDATE object_registry SET historyid = $1::bigint "
                    " WHERE id = $2::integer RETURNING id"
                    , Database::query_param_list(history_id)(domain_id));
            if (update_historyid_res.size() != 1)
            {
                BOOST_THROW_EXCEPTION(Fred::InternalError("update historyid failed"));
            }

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

    std::ostream& operator<<(std::ostream& os, const UpdateDomain& i)
    {
        os << "#UpdateDomain fqdn: " << i.fqdn_
            << " registrar: " << i.registrar_
            << " registrant: " << i.registrant_.print_quoted()
            << " authinfo: " << i.authinfo_.print_quoted()
            << " nsset: " << (i.nsset_.isset() ? i.nsset_.get_value().print_quoted() : i.nsset_.print_quoted())
            << " keyset: " << (i.keyset_.isset() ? i.keyset_.get_value().print_quoted() : i.keyset_.print_quoted())
            ;
        if(!i.add_admin_contact_.empty()) os << " add_admin_contact: ";
        for(std::vector<std::string>::const_iterator ci = i.add_admin_contact_.begin()
                ; ci != i.add_admin_contact_.end() ; ++ci ) os << *ci;
        if(!i.rem_admin_contact_.empty()) os << " rem_admin_contact: ";
                for(std::vector<std::string>::const_iterator ci = i.rem_admin_contact_.begin()
                        ; ci != i.rem_admin_contact_.end() ; ++ci ) os << *ci;

        os << " logd_request_id: " << i.logd_request_id_.print_quoted();
        return os;
    }

    std::string UpdateDomain::to_string()
    {
        std::stringstream ss;
        ss << *this;
        return ss.str();
    }

}//namespace Fred

