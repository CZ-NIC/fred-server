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

#ifndef EPP_CONTACT_UPDATE_H_456451554674
#define EPP_CONTACT_UPDATE_H_456451554674

#include "src/epp/localized_response.h"
#include "src/epp/session_lang.h"
#include "src/epp/contact/ident_type.h"

#include "util/db/nullable.h"
#include "util/optional_value.h"

#include <string>

namespace Epp {
/**
 * members of type Nullable<Optional<std::string> > means:
 * * if Isnull() - change is delete (set to empty string)
 * * if NOT Isnull() and Isset() - change to string content
 * * if NOT Isnull() NOT Isset() - no change at all
 */
// TODO XXX later string members should be more idiot proof
// TODO XXX later explore relations between ident and identtype data
// TODO XXX handle snad neni vymazatelny
// TODO XXX authinfo snad neni vymazatelne
struct ContactUpdateInputData {
    std::string handle;
    Optional<std::string> name;
    Optional<std::string> organization;
    Optional<std::string> street1;
    Optional<std::string> street2;
    Optional<std::string> street3;
    Optional<std::string> city;
    Optional<std::string> state_or_province;
    Optional<std::string> postal_code;
    Optional<std::string> country_code;
    Optional<std::string> telephone;
    Optional<std::string> fax;
    Optional<std::string> email;
    Optional<std::string> notify_email;
    Optional<std::string> VAT;
    Optional<std::string> ident;
    Nullable<IdentType::Enum> identtype;
    Optional<std::string> authinfo;
    Optional<bool> disclose_name;
    Optional<bool> disclose_organization;
    Optional<bool> disclose_address;
    Optional<bool> disclose_telephone;
    Optional<bool> disclose_fax;
    Optional<bool> disclose_email;
    Optional<bool> disclose_VAT;
    Optional<bool> disclose_ident;
    Optional<bool> disclose_notify_email;

    ContactUpdateInputData(
        const std::string&            _handle,
        const Optional<std::string>&  _name,
        const Optional<std::string>&  _organization,
        const Optional<std::string>&  _street1,
        const Optional<std::string>&  _street2,
        const Optional<std::string>&  _street3,
        const Optional<std::string>&  _city,
        const Optional<std::string>&  _state_or_province,
        const Optional<std::string>&  _postal_code,
        const Optional<std::string>&  _country_code,
        const Optional<std::string>&  _telephone,
        const Optional<std::string>&  _fax,
        const Optional<std::string>&  _email,
        const Optional<std::string>&  _notify_email,
        const Optional<std::string>&  _VAT,
        const Optional<std::string>&  _ident,
        const Nullable<IdentType::Enum>&    _identtype,
        const Optional<std::string>&  _authinfo,
        const Optional<bool>&         _disclose_name,
        const Optional<bool>&         _disclose_organization,
        const Optional<bool>&         _disclose_address,
        const Optional<bool>&         _disclose_telephone,
        const Optional<bool>&         _disclose_fax,
        const Optional<bool>&         _disclose_email,
        const Optional<bool>&         _disclose_VAT,
        const Optional<bool>&         _disclose_ident,
        const Optional<bool>&         _disclose_notify_email
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

LocalizedSuccessResponse contact_update(
    const ContactUpdateInputData& _data,
    unsigned long long _registrar_id,
    const Optional<unsigned long long>& _logd_request_id,
    bool _epp_update_contact_enqueue_check,
    SessionLang::Enum _lang,
    const std::string& _server_transaction_handle,
    const std::string& _client_transaction_handle,
    const std::string& _client_transaction_handles_prefix_not_to_nofify
);

}

#endif
