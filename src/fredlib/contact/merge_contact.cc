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
 *  @file merge_contact.cc
 *  contact merge
 */

#include <string>

#include "fredlib/contact/merge_contact.h"

#include "fredlib/domain/update_domain.h"
#include "fredlib/nsset/update_nsset.h"
#include "fredlib/keyset/update_keyset.h"
#include "fredlib/contact/delete_contact.h"

#include "fredlib/opcontext.h"
#include "fredlib/db_settings.h"

namespace Fred
{
    MergeContact::MergeContact(const std::string& from_contact_handle, const std::string& to_contact_handle)
    : src_contact_handle_(from_contact_handle)
    , dst_contact_handle_(to_contact_handle)
    {}

    void MergeContact::exec(OperationContext& ctx)
    {
        //lock object_registry row for update
        {
            Database::Result lock_res = ctx.get_conn().exec_params(
                "SELECT id FROM object_registry WHERE UPPER(name) = UPPER($1::text) AND type = 1 FOR UPDATE"
                , Database::query_param_list(src_contact_handle_));

            if (lock_res.size() != 1)
            {
                throw std::runtime_error("MergeContact::exec unable to lock source contact handle");
            }
        }
        {
            Database::Result lock_res = ctx.get_conn().exec_params(
                "SELECT id FROM object_registry WHERE UPPER(name) = UPPER($1::text) AND type = 1 FOR UPDATE"
                , Database::query_param_list(dst_contact_handle_));

            if (lock_res.size() != 1)
            {
                throw std::runtime_error("MergeContact::exec unable to lock destination contact handle");
            }
        }
        //diff contacts
        {
            Database::Result diff_result = ctx.get_conn().exec_params(
            "SELECT "//c1.name, oreg1.name, o1.clid, c2.name, oreg2.name , o2.clid,
            " (COALESCE(c1.name,'') != COALESCE(c2.name,'')) OR "
            " (COALESCE(c1.organization,'') != COALESCE(c2.organization,'')) OR "
            " (COALESCE(c1.street1,'') != COALESCE(c2.street1,'')) OR "
            " (COALESCE(c1.street2,'') != COALESCE(c2.street2,'')) OR "
            " (COALESCE(c1.street3,'') != COALESCE(c2.street3,'')) OR "
            " (COALESCE(c1.city,'') != COALESCE(c2.city,'')) OR "
            " (COALESCE(c1.postalcode,'') != COALESCE(c2.postalcode,'')) OR "
            " (COALESCE(c1.stateorprovince,'') != COALESCE(c2.stateorprovince,'')) OR "
            " (COALESCE(c1.country,'') != COALESCE(c2.country,'')) OR "
            " (COALESCE(c1.telephone,'') != COALESCE(c2.telephone,'')) OR "
            " (COALESCE(c1.fax,'') != COALESCE(c2.fax,'')) OR "
            " (COALESCE(c1.email,'') != COALESCE(c2.email,'')) OR "
            " (COALESCE(c1.notifyemail,'') != COALESCE(c2.notifyemail,'')) OR "
            " (COALESCE(c1.vat,'') != COALESCE(c2.vat,'')) OR "
            " (COALESCE(c1.ssntype,0) != COALESCE(c2.ssntype,0)) OR "
            " (COALESCE(c1.ssn,'') != COALESCE(c2.ssn,'')) OR "
            " (c1.disclosename != c2.disclosename) OR "
            " (c1.discloseorganization != c2.discloseorganization) OR "
            " (c1.discloseaddress != c2.discloseaddress) OR "
            " (c1.disclosetelephone != c2.disclosetelephone) OR "
            " (c1.disclosefax != c2.disclosefax) OR "
            " (c1.discloseemail != c2.discloseemail) OR "
            " (c1.disclosevat != c2.disclosevat) OR "
            " (c1.discloseident != c2.discloseident) OR "
            " (c1.disclosenotifyemail != c2.disclosenotifyemail) OR "
            " o1.clid != o2.clid "// current registrar
            "  as differ "
            " FROM (object_registry oreg1 JOIN object o1 ON oreg1.id=o1.id JOIN contact c1 ON c1.id = oreg1.id AND UPPER(oreg1.name) = UPPER($1::text) ) "
            " JOIN (object_registry oreg2 JOIN object o2 ON oreg2.id=o2.id JOIN contact c2 ON c2.id = oreg2.id AND UPPER(oreg2.name) = UPPER($2::text)) ON TRUE "
              , Database::query_param_list(src_contact_handle_)(dst_contact_handle_));
        if (diff_result.size() != 1)
            throw std::runtime_error("MergeContact::exec unable to get contact difference");
        bool contact_differ = static_cast<bool>(diff_result[0][0]);
        if(contact_differ)
            throw std::runtime_error("MergeContact::exec contacts differ");
        }

        //domain_registrant lock and update
        {
            Database::Result result = ctx.get_conn().exec_params(
                "SELECT oreg.name, r.handle FROM contact src_c JOIN object_registry src_oreg ON src_c.id = src_oreg.id AND UPPER(src_oreg.name) = UPPER($1::text) "
                " JOIN domain d ON d.registrant = src_c.id JOIN object_registry oreg  ON oreg.id = d.id JOIN object o ON oreg.id = o.id JOIN registrar r ON o.clid = r.id FOR UPDATE OF oreg "
            , Database::query_param_list(src_contact_handle_));

            for(Database::Result::size_type i = 0; i < result.size(); ++i)
            {
                UpdateDomain(std::string(result[i][0]) //fqdn
                                , std::string(result[i][1]) //registrar
                ).set_registrant(dst_contact_handle_).exec(ctx);
            }//for
        }

        //domain_admin lock and update
        {
            Database::Result result = ctx.get_conn().exec_params(
                "SELECT oreg.name, r.handle FROM contact src_c JOIN object_registry src_oreg ON src_c.id = src_oreg.id AND UPPER(src_oreg.name) = UPPER($1::text) "
                " JOIN domain_contact_map dcm ON dcm.role = 1 AND dcm.contactid  = src_c.id JOIN domain d ON dcm.domainid = d.id JOIN object_registry oreg ON oreg.id = d.id JOIN object o ON oreg.id = o.id JOIN registrar r ON o.clid = r.id FOR UPDATE OF oreg "
            , Database::query_param_list(src_contact_handle_));

            for(Database::Result::size_type i = 0; i < result.size(); ++i)
            {
                UpdateDomain(std::string(result[i][0]) //fqdn
                            , std::string(result[i][1]) //registrar
                ).rem_admin_contact(src_contact_handle_)
                .add_admin_contact(dst_contact_handle_).exec(ctx);
            }//for
        }

        //nsset_tech lock and update
        {
            Database::Result result = ctx.get_conn().exec_params(
                "SELECT oreg.name, r.handle FROM contact src_c JOIN object_registry src_oreg ON src_c.id = src_oreg.id AND UPPER(src_oreg.name) = UPPER($1::text) "
                " JOIN nsset_contact_map ncm ON ncm.contactid  = src_c.id JOIN nsset n ON ncm.nssetid = n.id JOIN object_registry oreg  ON oreg.id = n.id JOIN object o ON oreg.id = o.id JOIN registrar r ON o.clid = r.id FOR UPDATE OF oreg "
            , Database::query_param_list(src_contact_handle_));

            for(Database::Result::size_type i = 0; i < result.size(); ++i)
            {
                UpdateNsset(std::string(result[i][0]) //handle
                            , std::string(result[i][1]) //registrar
                ).rem_tech_contact(src_contact_handle_)
                .add_tech_contact(dst_contact_handle_).exec(ctx);
            }//for
        }

        //keyset_tech lock and update
        {
            Database::Result result = ctx.get_conn().exec_params(
                "SELECT oreg.name, r.handle FROM contact src_c JOIN object_registry src_oreg ON src_c.id = src_oreg.id AND UPPER(src_oreg.name) = UPPER($1::text) "
                " JOIN keyset_contact_map kcm ON kcm.contactid  = src_c.id JOIN keyset k ON kcm.keysetid = k.id JOIN object_registry oreg  ON oreg.id = k.id JOIN object o ON oreg.id = o.id JOIN registrar r ON o.clid = r.id FOR UPDATE OF oreg "
            , Database::query_param_list(src_contact_handle_));

            for(Database::Result::size_type i = 0; i < result.size(); ++i)
            {
                UpdateKeyset(std::string(result[i][0]) //handle
                            , std::string(result[i][1]) //registrar
                ).rem_tech_contact(src_contact_handle_)
                .add_tech_contact(dst_contact_handle_).exec(ctx);
            }//for
        }

        //delete src contact
        DeleteContact(src_contact_handle_).exec(ctx);
    }//MergeContact::exec

}//namespace Fred

