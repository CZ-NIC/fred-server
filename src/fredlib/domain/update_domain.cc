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

    UpdateDomain& UpdateDomain::set_keyset(const Nullable<std::string>& keyset)
    {
        keyset_ = keyset;
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

    void UpdateDomain::exec(OperationContext& ctx)
    {
        //update object
        Fred::UpdateObject(fqdn_, registrar_, authinfo_).exec(ctx);

        //update domain
        {
            Database::QueryParams params;//query params
            std::stringstream sql;
            Util::HeadSeparator set_separator(" SET "," , ");
            params.push_back(registrar_);
            sql <<"UPDATE domain ";

            if(nsset_.isset())//change nsset
            {
                Nullable<std::string> new_nsset_value = nsset_;
                params.push_back(new_nsset_value);//NULL or value via Nullable operator Database::QueryParam()

                if(new_nsset_value.isnull())
                {//null case query
                    sql << set_separator.get() << " nsset = $"
                        << params.size() << "::integer "; //nsset delete
                }
                else
                {//value case query
                    sql << set_separator.get()
                        << " nsset = (SELECT oreg.id FROM object_registry oreg "
                        " JOIN nsset n ON oreg.id = n.id WHERE UPPER(oreg.name) = UPPER($"
                        << params.size() << "::text)) "; //nsset update
                }
            }//if change nsset

            if(keyset_.isset())//change keyset
            {
                Nullable<std::string> new_keyset_value = keyset_;
                params.push_back(new_keyset_value);//NULL or value via Nullable operator Database::QueryParam()

                if(new_keyset_value.isnull())
                {//null case query
                    sql << set_separator.get() << " keyset = $"
                        << params.size() << "::integer "; //keyset delete
                }
                else
                {//value case query
                    sql << set_separator.get()
                        << " keyset = (SELECT oreg.id FROM object_registry oreg "
                        " JOIN keyset k ON oreg.id = k.id WHERE UPPER(oreg.name) = UPPER($"
                        << params.size() << "::text)) "; //keyset update
                }
            }//if change keyset

            if(registrant_.isset())//change registrant
            {
                params.push_back(registrant_);
                sql << set_separator.get() << " registrant = (SELECT oreg.id "
                    " FROM object_registry oreg JOIN contact c ON oreg.id = c.id "
                    " WHERE UPPER(oreg.name) = UPPER($" << params.size() << "::text)) "; //registrant update
            }//if change registrant

            params.push_back(fqdn_);
            sql << " WHERE id = (SELECT oreg.id FROM domain d "
                    " JOIN object_registry oreg ON d.id = oreg.id "
                    " WHERE UPPER(oreg.name) = UPPER($" << params.size() << "::text)); ";//update object_id by handle
            ctx.get_conn().exec_params(sql.str(), params);
        }//update domain

        //add admin contacts
        if(!add_admin_contact_.empty())
        {
            Database::QueryParams params;//query params
            std::stringstream sql;

            params.push_back(fqdn_);
            sql << "INSERT INTO domain_contact_map(domainid, contactid) "
                    " VALUES ((SELECT oreg.id FROM domain d "
                    " JOIN object_registry oreg ON d.id = oreg.id "
                    " WHERE UPPER(oreg.name) = UPPER($" << params.size() << "::text)), ";

            for(std::vector<std::string>::iterator i = add_admin_contact_.begin(); i != add_admin_contact_.end(); ++i)
            {
                Database::QueryParams params_i = params;//query params
                std::stringstream sql_i(sql.str());

                params_i.push_back(*i);
                sql_i << " (SELECT oreg.id FROM object_registry oreg JOIN contact c ON oreg.id = c.id "
                    " WHERE UPPER(oreg.name) = UPPER($"<< params_i.size() << "::text)));";
                ctx.get_conn().exec_params(sql_i.str(), params_i);
            }//for i
        }//if add admin contacts

        //delete admin contacts
        if(!rem_admin_contact_.empty())
        {
            Database::QueryParams params;//query params
            std::stringstream sql;

            params.push_back(fqdn_);
            sql << "DELETE FROM domain_contact_map WHERE domainid = "
                    " (SELECT oreg.id FROM domain d "
                    " JOIN object_registry oreg ON d.id = oreg.id "
                    " WHERE UPPER(oreg.name) = UPPER($" << params.size() << "::text)) AND ";

            for(std::vector<std::string>::iterator i = rem_admin_contact_.begin(); i != rem_admin_contact_.end(); ++i)
            {
                Database::QueryParams params_i = params;//query params
                std::stringstream sql_i(sql.str());

                params_i.push_back(*i);
                sql_i << "contactid = (SELECT oreg.id FROM object_registry oreg "
                        " JOIN contact c ON oreg.id = c.id WHERE UPPER(oreg.name) = UPPER($"
                    << params_i.size() << "::text));";
                ctx.get_conn().exec_params(sql_i.str(), params_i);
            }//for i
        }//if delete admin contacts

        //save history
        {
            unsigned long long history_id = Fred::InsertHistory(logd_request_id_).exec(ctx);

            //object_history
            ctx.get_conn().exec_params(
                "INSERT INTO object_history(historyid,id,clid, upid, trdate, update, authinfopw) "
                " SELECT $1::bigint, id,clid, upid, trdate, update, authinfopw FROM object "
                " WHERE id = (SELECT oreg.id FROM domain d "
                    " JOIN object_registry oreg ON d.id = oreg.id "
                    " WHERE UPPER(oreg.name) = UPPER($2::text))"
                , Database::query_param_list(history_id)(fqdn_));

            //object_registry historyid
            ctx.get_conn().exec_params(
                "UPDATE object_registry SET historyid = $1::bigint "
                " WHERE id = (SELECT oreg.id FROM domain d "
                    " JOIN object_registry oreg ON d.id = oreg.id "
                    " WHERE UPPER(oreg.name) = UPPER($2::text))"
                , Database::query_param_list(history_id)(fqdn_));

            //domain_history
            ctx.get_conn().exec_params(
                "INSERT INTO domain_history(historyid,id,zone, registrant, nsset, exdate, keyset) "
                " SELECT $1::bigint, id, zone, registrant, nsset, exdate, keyset FROM domain "
                " WHERE id = (SELECT oreg.id FROM domain d "
                    " JOIN object_registry oreg ON d.id = oreg.id "
                    " WHERE UPPER(oreg.name) = UPPER($2::text))"
                , Database::query_param_list(history_id)(fqdn_));

            //domain_contact_map_history
            ctx.get_conn().exec_params(
                "INSERT INTO domain_contact_map_history(historyid,domainid,contactid, role) "
                " SELECT $1::bigint, domainid,contactid, role FROM domain_contact_map "
                " WHERE domainid = (SELECT oreg.id FROM domain d "
                    " JOIN object_registry oreg ON d.id = oreg.id "
                    " WHERE UPPER(oreg.name) = UPPER($2::text))"
                , Database::query_param_list(history_id)(fqdn_));

            //enumval_history
            ctx.get_conn().exec_params(
                "INSERT INTO enumval_history(historyid,domainid,exdate, publish) "
                " SELECT $1::bigint, domainid,exdate, publish FROM enumval "
                " WHERE domainid = (SELECT oreg.id FROM domain d "
                    " JOIN object_registry oreg ON d.id = oreg.id "
                    " WHERE UPPER(oreg.name) = UPPER($2::text))"
                , Database::query_param_list(history_id)(fqdn_));
        }//save history
    }//UpdateDomain::exec

}//namespace Fred

