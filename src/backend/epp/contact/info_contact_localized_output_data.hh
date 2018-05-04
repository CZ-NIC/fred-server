/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
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

#ifndef INFO_CONTACT_LOCALIZED_OUTPUT_DATA_HH_2F08A74A2E1F4EA09D32D78E0D3FDF36
#define INFO_CONTACT_LOCALIZED_OUTPUT_DATA_HH_2F08A74A2E1F4EA09D32D78E0D3FDF36

#include "src/backend/epp/contact/contact_disclose.hh"
#include "src/backend/epp/contact/contact_data.hh"
#include "src/backend/epp/contact/status_value.hh"
#include "src/backend/epp/contact/contact_ident.hh"
#include "src/backend/epp/object_states_localized.hh"
#include "src/util/db/nullable.hh"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional.hpp>

#include <map>
#include <string>

namespace Epp {
namespace Contact {

struct InfoContactLocalizedOutputData
{
    std::string handle;
    std::string roid;
    std::string sponsoring_registrar_handle;
    std::string creating_registrar_handle;
    Nullable<std::string> last_update_registrar_handle;
    ObjectStatesLocalized<StatusValue> localized_external_states;
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
    boost::optional<ContactData::Address> mailing_address;
    Nullable<std::string> telephone;
    Nullable<std::string> fax;
    Nullable<std::string> email;
    Nullable<std::string> notify_email;
    Nullable<std::string> VAT;
    boost::optional<ContactIdent> ident;
    boost::optional<std::string> authinfopw;
    boost::optional<ContactDisclose> disclose;


    explicit InfoContactLocalizedOutputData(const boost::optional<ContactDisclose>& _disclose)
        : disclose(_disclose)
    {
    }

    InfoContactLocalizedOutputData(
            const std::string& _handle,
            const std::string& _roid,
            const std::string& _sponsoring_registrar_handle,
            const std::string& _creating_registrar_handle,
            const Nullable<std::string>& _last_update_registrar_handle,
            const ObjectStatesLocalized<StatusValue>& _localized_external_states,
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
            const boost::optional<ContactData::Address>& _mailing_address,
            const Nullable<std::string>& _telephone,
            const Nullable<std::string>& _fax,
            const Nullable<std::string>& _email,
            const Nullable<std::string>& _notify_email,
            const Nullable<std::string>& _VAT,
            const boost::optional<ContactIdent>& _ident,
            const boost::optional<std::string>& _authinfopw,
            const boost::optional<ContactDisclose>& _disclose)
    :
        handle(_handle),
        roid(_roid),
        sponsoring_registrar_handle(_sponsoring_registrar_handle),
        creating_registrar_handle(_creating_registrar_handle),
        last_update_registrar_handle(_last_update_registrar_handle),
        localized_external_states(_localized_external_states),
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
        mailing_address(_mailing_address),
        telephone(_telephone),
        fax(_fax),
        email(_email),
        notify_email(_notify_email),
        VAT(_VAT),
        ident(_ident),
        authinfopw(_authinfopw),
        disclose(_disclose)
    {
    }

};

} // namespace Epp::Contact
} // namespace Epp

#endif
