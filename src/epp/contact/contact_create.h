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

#ifndef EPP_CONTACT_CREATE_H_548537357434
#define EPP_CONTACT_CREATE_H_548537357434

#include "src/epp/contact/ident_type.h"
#include "src/epp/localized_response.h"
#include "src/epp/session_lang.h"
#include "util/db/nullable.h"
#include "util/optional_value.h"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace Epp {

struct ContactCreateInputData {
    std::string handle;
    std::string name;
    std::string organization;
    std::string street1;
    std::string street2;
    std::string street3;
    std::string city;
    std::string state_or_province;
    std::string postal_code;
    std::string country_code;
    std::string telephone;
    std::string fax;
    std::string email;
    std::string notify_email;
    std::string VAT;
    std::string ident;
    Nullable<IdentType::Enum> identtype;
    std::string authinfo;
    bool disclose_name;
    bool disclose_organization;
    bool disclose_address;
    bool disclose_telephone;
    bool disclose_fax;
    bool disclose_email;
    bool disclose_VAT;
    bool disclose_ident;
    bool disclose_notify_email;

    ContactCreateInputData(
        const std::string&  _handle,
        const std::string&  _name,
        const std::string&  _organization,
        const std::string&  _street1,
        const std::string&  _street2,
        const std::string&  _street3,
        const std::string&  _city,
        const std::string&  _state_or_province,
        const std::string&  _postal_code,
        const std::string&  _country_code,
        const std::string&  _telephone,
        const std::string&  _fax,
        const std::string&  _email,
        const std::string&  _notify_email,
        const std::string&  _VAT,
        const std::string&  _ident,
        const Nullable<IdentType::Enum>&   _identtype,
        const std::string&  _authinfo,
        bool                _disclose_name,
        bool                _disclose_organization,
        bool                _disclose_address,
        bool                _disclose_telephone,
        bool                _disclose_fax,
        bool                _disclose_email,
        bool                _disclose_VAT,
        bool                _disclose_ident,
        bool                _disclose_notify_email
    ) :
        handle(_handle),
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
        authinfo(_authinfo),
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

struct LocalizedCreateContactResponse {
    const LocalizedSuccessResponse ok_response;
    const boost::posix_time::ptime crdate;

    LocalizedCreateContactResponse(
        const LocalizedSuccessResponse& _ok_response,
        const boost::posix_time::ptime& _crdate
    ) :
        ok_response(_ok_response),
        crdate(_crdate)
    { }
};

LocalizedCreateContactResponse contact_create(
    const ContactCreateInputData& _data,
    unsigned long long _registrar_id,
    const Optional<unsigned long long>& _logd_request_id,
    SessionLang::Enum _lang,
    const std::string& _server_transaction_handle,
    const std::string& _client_transaction_handle,
    const std::string& _dont_notify_client_transaction_handles_with_this_prefix
);

}

#endif
