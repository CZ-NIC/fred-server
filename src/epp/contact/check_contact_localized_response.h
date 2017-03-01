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

#ifndef CHECK_CONTACT_LOCALIZED_RESPONSE_H_1D58962AC119499B837AB72B6CF42B0B
#define CHECK_CONTACT_LOCALIZED_RESPONSE_H_1D58962AC119499B837AB72B6CF42B0B

#include "src/epp/contact/impl/contact_handle_registration_obstruction_localized.h"
#include "src/epp/epp_response_success_localized.h"

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
