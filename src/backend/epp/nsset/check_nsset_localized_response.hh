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

#ifndef CHECK_NSSET_LOCALIZED_RESPONSE_H_A69185DF33EA4E5C975C9A0D75875D70
#define CHECK_NSSET_LOCALIZED_RESPONSE_H_A69185DF33EA4E5C975C9A0D75875D70

#include "src/backend/epp/epp_response_success_localized.hh"
#include "src/backend/epp/nsset/nsset_handle_registration_obstruction_localized.hh"

#include <boost/optional.hpp>

#include <map>

namespace Epp {
namespace Nsset {

struct CheckNssetLocalizedResponse
{
    const EppResponseSuccessLocalized epp_response_success_localized;
    const std::map<std::string, boost::optional<NssetHandleRegistrationObstructionLocalized> > nsset_statuses;


    CheckNssetLocalizedResponse(
            const EppResponseSuccessLocalized& _epp_response_success_localized,
            const std::map<std::string,
                    boost::optional<NssetHandleRegistrationObstructionLocalized> >& _nsset_statuses)
        : epp_response_success_localized(_epp_response_success_localized),
          nsset_statuses(_nsset_statuses)
    {
    }


};

} // namespace Epp::Nsset
} // namespace Epp

#endif
