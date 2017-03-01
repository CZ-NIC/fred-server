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

#ifndef CREATE_KEYSET_LOCALIZED_RESPONSE_H_859B9F43B50F46A0B71ACE42F26B8302
#define CREATE_KEYSET_LOCALIZED_RESPONSE_H_859B9F43B50F46A0B71ACE42F26B8302

#include "src/epp/epp_response_success_localized.h"

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
