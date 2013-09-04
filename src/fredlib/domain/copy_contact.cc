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

#include "fredlib/domain/copy_contact.h"
#include "fredlib/domain/get_blocking_status_desc_list.h"
#include "fredlib/domain/get_object_state_id_map.h"
#include "fredlib/contact/info_contact.h"
#include "fredlib/contact/create_contact.h"
#include "fredlib/opcontext.h"
#include "fredlib/db_settings.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"
#include "fredlib/object.h"

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
            return _n.isnull() ? Optional< T >() : Optional< T >(_n);
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
        Database::Result src_registrar_res = _ctx.get_conn().exec_params(
            "SELECT r.handle "
            "FROM object_registry obr "
            "JOIN registrar r ON r.id=obr.crid "
            "WHERE obr.type=$1::integer AND obr.name=UPPER($2::text) AND obr.erdate IS NULL",
            Database::query_param_list
                (OBJECT_TYPE_ID_CONTACT)(src_contact_handle_));
        if (src_registrar_res.size() != 1) {
            BOOST_THROW_EXCEPTION(Exception().set_src_contact_handle_not_found(src_contact_handle_));
        }
        const std::string src_registrar_handle = src_registrar_res[0][0];
        Fred::InfoContact info_contact(src_contact_handle_, src_registrar_handle);
        Fred::InfoContactOutput old_contact = info_contact.exec(_ctx);
        Fred::CreateContact create_contact(dst_contact_handle_, dst_registrar_handle_,
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
        create_contact.exec(_ctx);
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
