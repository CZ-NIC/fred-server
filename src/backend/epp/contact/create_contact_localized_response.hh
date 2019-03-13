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
#ifndef CREATE_CONTACT_LOCALIZED_RESPONSE_HH_EA0C7E615FE74E22B5F20F5089B3F15D
#define CREATE_CONTACT_LOCALIZED_RESPONSE_HH_EA0C7E615FE74E22B5F20F5089B3F15D

#include "src/backend/epp/epp_response_success_localized.hh"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace Epp {
namespace Contact {

struct CreateContactLocalizedResponse
{
    const EppResponseSuccessLocalized epp_response_success_localized;
    const boost::posix_time::ptime crdate;


    CreateContactLocalizedResponse(
            const EppResponseSuccessLocalized& _epp_response_success_localized,
            const boost::posix_time::ptime& _crdate)
        : epp_response_success_localized(_epp_response_success_localized),
          crdate(_crdate)
    {
    }

};


} // namespace Epp::Contact
} // namespace Epp

#endif
