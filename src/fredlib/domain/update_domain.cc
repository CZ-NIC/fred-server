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

        //lock object_registry row for update
        {
            Database::Result lock_res = ctx.get_conn().exec_params(
                "SELECT oreg.id FROM enum_object_type eot"
                " JOIN object_registry oreg ON oreg.type = eot.id "
                " AND oreg.name = LOWER($1::text) AND oreg.erdate IS NULL "
                " WHERE eot.name = 'domain' FOR UPDATE OF oreg"
                , Database::query_param_list(fqdn_));

            if (lock_res.size() != 1)
            {
                std::string errmsg("unable to lock || not found:fqdn: ");
                errmsg += boost::replace_all_copy(fqdn_,"|", "[pipe]");//quote pipes
                errmsg += " |";
                throw UDEX(errmsg.c_str());
            }
        }

        //get domain_id
        unsigned long long domain_id =0;
        {
            Database::Result domain_id_res = ctx.get_conn().exec_params(
                "SELECT oreg.id FROM domain d "
                " JOIN object_registry oreg ON d.id = oreg.id "
                " WHERE oreg.name = LOWER($1::text) AND oreg.erdate IS NULL"
                , Database::query_param_list(fqdn_));

            if (domain_id_res.size() != 1)
            {
                std::string errmsg("|| not found:fqdn: ");
                errmsg += boost::replace_all_copy(fqdn_,"|", "[pipe]");//quote pipes
                errmsg += " |";
                throw UDEX(errmsg.c_str());
            }

            domain_id = domain_id_res[0][0];
        }

        //update object
        Fred::UpdateObject(fqdn_,"domain", registrar_, authinfo_).exec(ctx);

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

                //lock object_registry row for update
                if(new_nsset_value.isnull())
                {
                    //Database::Result lock_res =
                        ctx.get_conn().exec_params(
                        "SELECT noreg.id FROM nsset n "
                        " JOIN object_registry noreg ON noreg.id = n.id AND noreg.erdate IS NULL "
                        " JOIN domain d ON d.nsset = n.id "
                        " JOIN object_registry doreg ON doreg.id = d.id AND doreg.erdate IS NULL "
                        " AND LOWER(doreg.name) = LOWER($1::text) "
                        " FOR UPDATE OF noreg "
                        , Database::query_param_list(fqdn_));
                    //result may be empty if there is no nsset
                    /*
                    if (lock_res.size() != 1)
                    {
                        std::string errmsg("unable to lock current nsset|| not found:fqdn: ");
                        errmsg += boost::replace_all_copy(fqdn_,"|", "[pipe]");//quote pipes
                        errmsg += " |";
                        throw UDEX(errmsg.c_str());
                    }
                    */
                }
                else
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
                        throw UDEX(errmsg.c_str());
                    }
                }



                params.push_back(new_nsset_value);//NULL or value via Nullable operator Database::QueryParam()

                if(new_nsset_value.isnull())
                {//null case query
                    sql << set_separator.get() << " nsset = $"
                        << params.size() << "::integer "; //nsset delete
                }
                else
                {//value case query
                    sql << set_separator.get()
                        << " nsset = raise_exception_ifnull((SELECT oreg.id FROM object_registry oreg "
                        " JOIN nsset n ON oreg.id = n.id "
                        " WHERE oreg.name = UPPER($" << params.size() << "::text) AND oreg.erdate IS NULL) "
                        " ,'|| not found:nsset: '||ex_data($"<< params.size() << "::text)||' |') "; //nsset update
                }
            }//if change nsset

            if(keyset_.isset())//change keyset
            {
                Nullable<std::string> new_keyset_value = keyset_;

                //lock object_registry row for update
                if(new_keyset_value.isnull())
                {
                    Database::Result lock_res = ctx.get_conn().exec_params(
                        "SELECT koreg.id FROM keyset k "
                        " JOIN object_registry koreg ON koreg.id = k.id AND koreg.erdate IS NULL "
                        " JOIN domain d ON d.keyset = k.id "
                        " JOIN object_registry doreg ON doreg.id = d.id AND doreg.erdate IS NULL "
                        " AND doreg.name = LOWER($1::text) "
                        " FOR UPDATE OF koreg "
                        , Database::query_param_list(fqdn_));
                    //result may be empty if there is no keyset
                    /*
                    if (lock_res.size() != 1)
                    {
                        std::string errmsg("unable to lock current keyset|| not found:fqdn: ");
                        errmsg += boost::replace_all_copy(fqdn_, "|", "[pipe]");//quote pipes
                        errmsg += " |";
                        throw UDEX(errmsg.c_str());
                    }
                    */
                }
                else
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
                        throw UDEX(errmsg.c_str());
                    }
                }

                params.push_back(new_keyset_value);//NULL or value via Nullable operator Database::QueryParam()

                if(new_keyset_value.isnull())
                {//null case query
                    sql << set_separator.get() << " keyset = $"
                        << params.size() << "::integer "; //keyset delete
                }
                else
                {//value case query
                    sql << set_separator.get()
                        << " keyset = raise_exception_ifnull("
                        " (SELECT oreg.id FROM object_registry oreg JOIN keyset k ON oreg.id = k.id "
                        " WHERE oreg.name = UPPER($" << params.size() << "::text) AND oreg.erdate IS NULL),"
                        " '|| not found:keyset: '||ex_data($"<< params.size() << "::text)||' |') "; //keyset update
                }
            }//if change keyset

            if(registrant_.isset())//change registrant
            {
                //lock object_registry row for update
                {
                    Database::Result lock_res = ctx.get_conn().exec_params(
                        "SELECT oreg.id FROM enum_object_type eot"
                        " JOIN object_registry oreg ON oreg.type = eot.id "
                        " AND oreg.name = UPPER($1::text) AND oreg.erdate IS NULL "
                        " WHERE eot.name = 'contact' FOR UPDATE OF oreg"
                        , Database::query_param_list(registrant_.get_value()));

                    if (lock_res.size() != 1)
                    {
                        std::string errmsg("unable to lock || not found:registrant: ");
                        errmsg += boost::replace_all_copy(std::string(registrant_.get_value()),"|", "[pipe]");//quote pipes
                        errmsg += " |";
                        throw UDEX(errmsg.c_str());
                    }
                }

                params.push_back(registrant_);
                sql << set_separator.get() << " registrant = raise_exception_ifnull( "
                    " (SELECT oreg.id FROM object_registry oreg JOIN contact c ON oreg.id = c.id "
                    " WHERE oreg.name = UPPER($" << params.size() << "::text) AND oreg.erdate IS NULL) "
                    " ,'|| not found:registrant: '||ex_data($"<< params.size() << "::text)||' |') "; //registrant update
            }//if change registrant

            params.push_back(domain_id);
            sql << " WHERE id = $" << params.size() << "::integer ";
            ctx.get_conn().exec_params(sql.str(), params);
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
                        throw UDEX(errmsg.c_str());
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
                        throw UDEX(errmsg.c_str());
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
                    throw UDEX(errmsg.c_str());
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
                        throw UDEX(errmsg.c_str());
                    }
                }

                Database::QueryParams params_i = params;//query params
                std::stringstream sql_i;
                sql_i << sql.str();

                params_i.push_back(*i);
                sql_i << "contactid = raise_exception_ifnull("
                        " (SELECT oreg.id FROM object_registry oreg JOIN contact c ON oreg.id = c.id "
                        " WHERE oreg.name = UPPER($"<< params_i.size() << "::text) AND oreg.erdate IS NULL) "
                        " ,'|| not found:admin contact: '||ex_data($"<< params.size() << "::text)||' |') "
                        " RETURNING domainid";
                Database::Result domain_del_res = ctx.get_conn().exec_params(sql_i.str(), params_i);
                if (domain_del_res.size() != 1)
                {
                    std::string errmsg("delete admin contact failed || invalid:admin contact: ");
                    errmsg += boost::replace_all_copy(*i,"|", "[pipe]");//quote pipes
                    errmsg += " |";
                    throw UDEX(errmsg.c_str());
                }
            }//for i
        }//if delete admin contacts

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
            ctx.get_conn().exec_params(
                "UPDATE object_registry SET historyid = $1::bigint "
                    " WHERE id = $2::integer"
                    , Database::query_param_list(history_id)(domain_id));

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
        catch(...)//common exception processing
        {
            handleOperationExceptions<UpdateDomainException>(__FILE__, __LINE__, __ASSERT_FUNCTION);
        }
        return history_id;
    }//UpdateDomain::exec

}//namespace Fred

