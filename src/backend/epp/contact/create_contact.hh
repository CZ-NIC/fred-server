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

#ifndef CREATE_CONTACT_HH_6BB52584FE794312AC2D429BA7D07FCB
#define CREATE_CONTACT_HH_6BB52584FE794312AC2D429BA7D07FCB

#include "src/backend/epp/contact/create_contact_input_data.hh"
#include "src/backend/epp/contact/create_contact_config_data.hh"
#include "src/backend/epp/contact/create_contact_localized.hh"
#include "src/backend/epp/session_data.hh"
#include "libfred/opcontext.hh"

#include <boost/date_time/posix_time/posix_time.hpp>

#include <string>

namespace Epp {
namespace Contact {

struct CreateContactResult
{
    CreateContactResult(
            unsigned long long _contact_id,
            unsigned long long _create_history_id,
            const boost::posix_time::ptime& _contact_crdate);
    unsigned long long id;
    unsigned long long create_history_id;
    // TODO guarantee non-special
    boost::posix_time::ptime crdate;
};

CreateContactResult create_contact(
        LibFred::OperationContext& ctx,
        const std::string& contact_handle,
        const CreateContactInputData& data,
        const CreateContactConfigData& create_contact_config_data,
        const SessionData& session_data);

} // namespace Epp::Contact
} // namespace Epp

#endif
