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
#include "src/epp/session_lang.h"
#include "src/fredlib/opcontext.h"
#include "util/db/nullable.h"

#include <string>
#include <vector>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace Epp {

struct ContactInfoOutputData {
    std::string handle;
    std::string roid;
    std::string sponsoring_registrar_handle;
    std::string creating_registrar_handle;
    Nullable<std::string> last_update_registrar_handle;
    std::set<std::string> states;
    boost::posix_time::ptime crdate;
    Nullable<boost::posix_time::ptime> last_update;
    Nullable<boost::posix_time::ptime> last_transfer;
    Nullable<std::string> name;
    Nullable<std::string> organization;
    Nullable<std::string> street1;
    Nullable<std::string> street2;
    Nullable<std::string> street3;
    Nullable<std::string> city;
    Nullable<std::string> state_or_province;
    Nullable<std::string> postal_code;
    Nullable<std::string> country_code;
    Nullable<std::string> telephone;
    Nullable<std::string> fax;
    Nullable<std::string> email;
    Nullable<std::string> notify_email;
    Nullable<std::string> VAT;
    Nullable<std::string> ident;
    Nullable<IdentType::Enum> identtype;
    std::string auth_info_pw;
    bool disclose_name;
    bool disclose_organization;
    bool disclose_address;
    bool disclose_telephone;
    bool disclose_fax;
    bool disclose_email;
    bool disclose_VAT;
    bool disclose_ident;
    bool disclose_notify_email;

    ContactInfoOutputData(
        const std::string& _handle,
        const std::string& _roid,
        const std::string& _sponsoring_registrar_handle,
        const std::string& _creating_registrar_handle,
        const Nullable<std::string>& _last_update_registrar_handle,
        const std::set<std::string>& _states,
        const boost::posix_time::ptime& _crdate,
        const Nullable<boost::posix_time::ptime>& _last_update,
        const Nullable<boost::posix_time::ptime>& _last_transfer,
        const Nullable<std::string>& _name,
        const Nullable<std::string>& _organization,
        const Nullable<std::string>& _street1,
        const Nullable<std::string>& _street2,
        const Nullable<std::string>& _street3,
        const Nullable<std::string>& _city,
        const Nullable<std::string>& _state_or_province,
        const Nullable<std::string>& _postal_code,
        const Nullable<std::string>& _country_code,
        const Nullable<std::string>& _telephone,
        const Nullable<std::string>& _fax,
        const Nullable<std::string>& _email,
        const Nullable<std::string>& _notify_email,
        const Nullable<std::string>& _VAT,
        const Nullable<std::string>& _ident,
        const Nullable<IdentType::Enum>& _identtype,
        const std::string& _auth_info_pw,
        bool _disclose_name,
        bool _disclose_organization,
        bool _disclose_address,
        bool _disclose_telephone,
        bool _disclose_fax,
        bool _disclose_email,
        bool _disclose_VAT,
        bool _disclose_ident,
        bool _disclose_notify_email
    ) :
        handle(_handle),
        roid(_roid),
        sponsoring_registrar_handle(_sponsoring_registrar_handle),
        creating_registrar_handle(_creating_registrar_handle),
        last_update_registrar_handle(_last_update_registrar_handle),
        states(_states),
        crdate(_crdate),
        last_update(_last_update),
        last_transfer(_last_transfer),
        name(_name),
        organization(_organization),
        street1(_street1),
        street2(_street2),
        street3(_street3),
        city(_city),
        state_or_province(_state_or_province),
        postal_code(_postal_code),
        country_code(_country_code),
        telephone(_telephone),
        fax(_fax),
        email(_email),
        notify_email(_notify_email),
        VAT(_VAT),
        ident(_ident),
        identtype(_identtype),
        auth_info_pw(_auth_info_pw),
        disclose_name(_disclose_name),
        disclose_organization(_disclose_organization),
        disclose_address(_disclose_address),
        disclose_telephone(_disclose_telephone),
        disclose_fax(_disclose_fax),
        disclose_email(_disclose_email),
        disclose_VAT(_disclose_VAT),
        disclose_ident(_disclose_ident),
        disclose_notify_email(_disclose_notify_email)
    { }
};

/**
 * @throws ExceptionAuthErrorServerClosingConnection
 * @throws ExceptionInvalidHandle
 * @throws ExceptionNonexistentHandle
 */
ContactInfoOutputData contact_info_impl(
    Fred::OperationContext& _ctx,
    const std::string& _handle,
    SessionLang::Enum _object_state_description_lang,
    unsigned long long _session_registrar_id
);

}

#endif
