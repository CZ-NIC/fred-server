/*
 * Copyright (C) 2017-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef INFO_CONTACT_LOCALIZED_OUTPUT_DATA_HH_2F08A74A2E1F4EA09D32D78E0D3FDF36
#define INFO_CONTACT_LOCALIZED_OUTPUT_DATA_HH_2F08A74A2E1F4EA09D32D78E0D3FDF36

#include "src/backend/epp/contact/contact_data.hh"
#include "src/backend/epp/contact/status_value.hh"
#include "src/backend/epp/contact/contact_ident.hh"
#include "src/backend/epp/contact/hideable.hh"
#include "src/backend/epp/contact/street_traits.hh"
#include "src/backend/epp/object_states_localized.hh"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional.hpp>

#include <map>
#include <string>

namespace Epp {
namespace Contact {

struct InfoContactLocalizedOutputData
{
    struct Address
    {
        StreetTraits::Rows<boost::optional<std::string>> street;
        boost::optional<std::string> city;
        boost::optional<std::string> state_or_province;
        boost::optional<std::string> postal_code;
        boost::optional<std::string> country_code;
    };

    InfoContactLocalizedOutputData() = default;

    InfoContactLocalizedOutputData(
            const std::string& _handle,
            const std::string& _roid,
            const std::string& _sponsoring_registrar_handle,
            const std::string& _creating_registrar_handle,
            const boost::optional<std::string>& _last_update_registrar_handle,
            const ObjectStatesLocalized<StatusValue>& _localized_external_states,
            const boost::posix_time::ptime& _crdate,
            const boost::optional<boost::posix_time::ptime>& _last_update,
            const boost::optional<boost::posix_time::ptime>& _last_transfer,
            const HideableOptional<std::string>& _name,
            const HideableOptional<std::string>& _organization,
            const Hideable<Address>& _address,
            const boost::optional<ContactData::Address>& _mailing_address,
            const HideableOptional<std::string>& _telephone,
            const HideableOptional<std::string>& _fax,
            const HideableOptional<std::string>& _email,
            const HideableOptional<std::string>& _notify_email,
            const HideableOptional<std::string>& _VAT,
            const HideableOptional<ContactIdent>& _ident,
            const boost::optional<std::string>& _authinfopw)
        : handle(_handle),
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
          address(_address),
          mailing_address(_mailing_address),
          telephone(_telephone),
          fax(_fax),
          email(_email),
          notify_email(_notify_email),
          VAT(_VAT),
          ident(_ident),
          authinfopw(_authinfopw)
    { }

    std::string handle;
    std::string roid;
    std::string sponsoring_registrar_handle;
    std::string creating_registrar_handle;
    boost::optional<std::string> last_update_registrar_handle;
    ObjectStatesLocalized<StatusValue> localized_external_states;
    boost::posix_time::ptime crdate;
    boost::optional<boost::posix_time::ptime> last_update;
    boost::optional<boost::posix_time::ptime> last_transfer;
    HideableOptional<std::string> name;
    HideableOptional<std::string> organization;
    Hideable<Address> address;
    boost::optional<ContactData::Address> mailing_address;
    HideableOptional<std::string> telephone;
    HideableOptional<std::string> fax;
    HideableOptional<std::string> email;
    HideableOptional<std::string> notify_email;
    HideableOptional<std::string> VAT;
    HideableOptional<ContactIdent> ident;
    boost::optional<std::string> authinfopw;
};

} // namespace Epp::Contact
} // namespace Epp

#endif
