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
 *  @file copy_contact.cc
 *  copy contact
 */

#include "src/fredlib/contact/copy_contact.h"
#include "src/fredlib/object_state/get_blocking_status_desc_list.h"
#include "src/fredlib/object_state/get_object_state_id_map.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/contact/create_contact.h"
#include "src/fredlib/object/object.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/db_settings.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"
#include "src/fredlib/object.h"

#include <boost/algorithm/string.hpp>

namespace Fred
{
    CopyContact::CopyContact(const std::string &_src_contact_handle,
        const std::string &_dst_contact_handle,
        RequestId _request_id)
    :   src_contact_handle_(_src_contact_handle),
        dst_contact_handle_(_dst_contact_handle),
        request_id_(_request_id)
    {}

    CopyContact::CopyContact(const std::string &_src_contact_handle,
        const std::string &_dst_contact_handle,
        const Optional< std::string > &_dst_registrar_handle,
        RequestId _request_id)
    :   src_contact_handle_(_src_contact_handle),
        dst_contact_handle_(_dst_contact_handle),
        dst_registrar_handle_(_dst_registrar_handle),
        request_id_(_request_id)
    {}

    namespace
    {
        template< class T >
        Optional< T > to_optional(const Nullable< T > &_n)
        {
            return _n.isnull() ? Optional< T >() : Optional< T >(_n.get_value());
        }

        Optional< std::string > to_optional(const std::string &_n)
        {
            return _n.empty() ? Optional< std::string >() : Optional< std::string >(_n);
        }

        Optional< unsigned long long > to_optional(unsigned long long _n)
        {
            return _n <= 0 ? Optional< unsigned long long >() : Optional< unsigned long long >(_n);
        }
    }

    ObjectId CopyContact::exec(OperationContext &_ctx)
    {
        Database::Result check_args_res = _ctx.get_conn().exec_params(
            "SELECT "
                   "(SELECT 1 " // src_contact_handle exist ? 1 : NULL
                    "FROM object_registry "
                    "WHERE type=$1::integer AND name=UPPER($2::text) AND erdate IS NULL),"
                   "(SELECT 1 " // dst_contact_handle exist ? 1 : NULL
                    "FROM object_registry "
                    "WHERE type=$1::integer AND name=UPPER($3::text) AND erdate IS NULL),"
                   "(SELECT 1 " // dst_registrar_handle exist ? 1 : NULL
                    "FROM registrar "
                    "WHERE handle=UPPER($4::text))",
            Database::query_param_list
                (OBJECT_TYPE_ID_CONTACT)(src_contact_handle_)(dst_contact_handle_)(dst_registrar_handle_));
        if (check_args_res.size() == 1) {
            Exception ex;
            if (check_args_res[0][0].isnull()) {
                ex.set_src_contact_handle_not_found(src_contact_handle_);
            }
            if (!check_args_res[0][1].isnull()) {
                ex.set_dst_contact_handle_already_exist(dst_contact_handle_);
            }
            if (check_args_res[0][2].isnull()) {
                ex.set_create_contact_failed(std::string("dst_registrar_handle ") + dst_registrar_handle_.get_value() + " doesn't exist");
            }
            if (ex.throw_me()) {
                BOOST_THROW_EXCEPTION(ex);
            }
        }

        Fred::InfoContactByHandle info_contact(src_contact_handle_);
        Fred::InfoContactOutput old_contact = info_contact.exec(_ctx);
        Fred::CreateContact create_contact(dst_contact_handle_, dst_registrar_handle_.get_value(),
          to_optional(old_contact.info_contact_data.authinfopw),
          to_optional(old_contact.info_contact_data.name),
          to_optional(old_contact.info_contact_data.organization),
          to_optional(old_contact.info_contact_data.street1),
          to_optional(old_contact.info_contact_data.street2),
          to_optional(old_contact.info_contact_data.street3),
          to_optional(old_contact.info_contact_data.city),
          to_optional(old_contact.info_contact_data.stateorprovince),
          to_optional(old_contact.info_contact_data.postalcode),
          to_optional(old_contact.info_contact_data.country),
          to_optional(old_contact.info_contact_data.telephone),
          to_optional(old_contact.info_contact_data.fax),
          to_optional(old_contact.info_contact_data.email),
          to_optional(old_contact.info_contact_data.notifyemail),
          to_optional(old_contact.info_contact_data.vat),
          to_optional(old_contact.info_contact_data.ssntype),
          to_optional(old_contact.info_contact_data.ssn),
          to_optional(old_contact.info_contact_data.disclosename),
          to_optional(old_contact.info_contact_data.discloseorganization),
          to_optional(old_contact.info_contact_data.discloseaddress),
          to_optional(old_contact.info_contact_data.disclosetelephone),
          to_optional(old_contact.info_contact_data.disclosefax),
          to_optional(old_contact.info_contact_data.discloseemail),
          to_optional(old_contact.info_contact_data.disclosevat),
          to_optional(old_contact.info_contact_data.discloseident),
          to_optional(old_contact.info_contact_data.disclosenotifyemail),
          to_optional(request_id_));
        try {
            create_contact.exec(_ctx);
        }
        catch (const Fred::CreateObject::Exception &e) {
            BOOST_THROW_EXCEPTION(Exception().set_dst_contact_handle_already_exist(dst_contact_handle_));
        }
        Database::Result dst_contact_id_res = _ctx.get_conn().exec_params(
            "SELECT id "
            "FROM object_registry "
            "WHERE type=$1::integer AND name=UPPER($2::text) AND erdate IS NULL",
            Database::query_param_list
                (OBJECT_TYPE_ID_CONTACT)(dst_contact_handle_));
        if (dst_contact_id_res.size() != 1) {
            BOOST_THROW_EXCEPTION(Exception().set_create_contact_failed("dst_contact " + dst_contact_handle_ + " not found"));
        }
        return dst_contact_id_res[0][0];
    }//CopyContact::exec

}//namespace Fred
