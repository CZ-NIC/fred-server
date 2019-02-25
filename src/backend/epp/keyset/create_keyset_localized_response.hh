/*
 * Copyright (C) 2016-2019  CZ.NIC, z. s. p. o.
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
#ifndef CREATE_KEYSET_LOCALIZED_RESPONSE_HH_4D46F0E50B4C4202A1F107F784325B5E
#define CREATE_KEYSET_LOCALIZED_RESPONSE_HH_4D46F0E50B4C4202A1F107F784325B5E

#include "src/backend/epp/epp_response_success_localized.hh"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace Epp {
namespace Keyset {

struct CreateKeysetLocalizedResponse
{
    EppResponseSuccessLocalized epp_response_success_localized;
    boost::posix_time::ptime crdate;


    CreateKeysetLocalizedResponse(
            const EppResponseSuccessLocalized& _response,
            const boost::posix_time::ptime& _crdate)
        : epp_response_success_localized(_response),
          crdate(_crdate)
    {
    }


};

} // namespace Epp::Keyset
} // namespace Epp

#endif
