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

#ifndef INFO_CONTACT_HH_FDF0BE2833874CB0A8E8CD911C1D2572
#define INFO_CONTACT_HH_FDF0BE2833874CB0A8E8CD911C1D2572

#include "src/backend/epp/contact/contact_data.hh"
#include "src/backend/epp/contact/contact_disclose.hh"
#include "src/backend/epp/contact/info_contact_config_data.hh"
#include "src/backend/epp/contact/status_value.hh"
#include "src/backend/epp/session_data.hh"
#include "src/backend/epp/session_lang.hh"
#include "src/libfred/registrable_object/contact/info_contact_data.hh"
#include "src/libfred/opcontext.hh"
#include "src/util/db/nullable.hh"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional.hpp>

#include <set>
#include <string>
#include <vector>

namespace Epp {
namespace Contact {

struct InfoContactOutputData
{
    explicit InfoContactOutputData(const boost::optional<ContactDisclose>& _disclose);
    std::string handle;
    std::string roid;
    std::string sponsoring_registrar_handle;
    std::string creating_registrar_handle;
    Nullable<std::string> last_update_registrar_handle;
    std::set<StatusValue::Enum> states;
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
    boost::optional<LibFred::PersonalIdUnion> personal_id;
    boost::optional<std::string> authinfopw;
    boost::optional<ContactDisclose> disclose;
};

/**
 * @throws ExceptionAuthErrorServerClosingConnection
 * @throws ExceptionNonexistentHandle
 */
InfoContactOutputData info_contact(
        LibFred::OperationContext& _ctx,
        const std::string& _contact_handle,
        const InfoContactConfigData& _info_contact_config_data,
        const SessionData& _session_data);

} // namespace Epp::Contact
} // namespace Epp

#endif