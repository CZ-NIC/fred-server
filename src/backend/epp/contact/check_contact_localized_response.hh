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
#ifndef CHECK_CONTACT_LOCALIZED_RESPONSE_HH_76B911CCBE23436F97F66117D02910B7
#define CHECK_CONTACT_LOCALIZED_RESPONSE_HH_76B911CCBE23436F97F66117D02910B7

#include "src/backend/epp/contact/contact_handle_registration_obstruction_localized.hh"
#include "src/backend/epp/epp_response_success_localized.hh"

#include <boost/optional.hpp>

#include <map>
#include <string>

namespace Epp {
namespace Contact {

struct CheckContactLocalizedResponse
{
    const EppResponseSuccessLocalized epp_response_success_localized;
    const std::map<std::string,
            boost::optional<ContactHandleRegistrationObstructionLocalized> > contact_statuses;


    CheckContactLocalizedResponse(
            const EppResponseSuccessLocalized& _epp_response_success_localized,
            const std::map<std::string,
                    boost::optional<ContactHandleRegistrationObstructionLocalized> >& _contact_statuses)
        : epp_response_success_localized(_epp_response_success_localized),
          contact_statuses(_contact_statuses)
    {
    }

};

} // namespace Epp::Contact
} // namespace Epp

#endif
