/*
 * Copyright (C) 2015  CZ.NIC, z.s.p.o.
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
 */

#ifndef EPP_CONTACT_INFO_IMPL_976543419473
#define EPP_CONTACT_INFO_IMPL_976543419473

#include "src/epp/contact/ident_type.h"
#include "src/epp/contact/disclose.h"
#include "src/epp/session_lang.h"
#include "src/fredlib/opcontext.h"
#include "util/db/nullable.h"

#include <string>
#include <set>
#include <vector>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace Epp {

struct ContactInfoOutputData
{
    std::string handle;
    std::string roid;
    std::string sponsoring_registrar_handle;
    std::string creating_registrar_handle;
    Nullable< std::string > last_update_registrar_handle;
    struct State
    {
        State(const std::string &_name, bool _is_external):name(_name), is_external(_is_external) { }
        bool operator<(const State &dst)const { return name < dst.name; }
        const std::string name;
        const bool is_external;
    };
    std::set< State > states;
    boost::posix_time::ptime crdate;
    Nullable< boost::posix_time::ptime > last_update;
    Nullable< boost::posix_time::ptime > last_transfer;
    Nullable< std::string > name;
    Nullable< std::string > organization;
    Nullable< std::string > street1;
    Nullable< std::string > street2;
    Nullable< std::string > street3;
    Nullable< std::string > city;
    Nullable< std::string > state_or_province;
    Nullable< std::string > postal_code;
    Nullable< std::string > country_code;
    Nullable< std::string > telephone;
    Nullable< std::string > fax;
    Nullable< std::string > email;
    Nullable< std::string > notify_email;
    Nullable< std::string > VAT;
    Nullable< std::string > ident;
    Nullable< IdentType::Enum > identtype;
    std::string auth_info_pw;
    std::set< ContactDisclose::Enum > to_hide;
    std::set< ContactDisclose::Enum > to_disclose;
};

/**
 * @throws ExceptionAuthErrorServerClosingConnection
 * @throws ExceptionNonexistentHandle
 */
ContactInfoOutputData contact_info_impl(
    Fred::OperationContext &_ctx,
    const std::string &_handle,
    SessionLang::Enum _object_state_description_lang,
    unsigned long long _session_registrar_id);

}

#endif
