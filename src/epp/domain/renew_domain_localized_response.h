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

#ifndef RENEW_DOMAIN_LOCALIZED_RESPONSE_H_5F1B7CA6A94A46BEA44ECE44C6A641D6
#define RENEW_DOMAIN_LOCALIZED_RESPONSE_H_5F1B7CA6A94A46BEA44ECE44C6A641D6

#include "src/epp/epp_response_success_localized.h"

#include <boost/date_time/gregorian/greg_date.hpp>

namespace Epp {
namespace Domain {

struct RenewDomainLocalizedResponse
{
    const EppResponseSuccessLocalized epp_response_success_localized;
    const boost::gregorian::date expiration_date;


    RenewDomainLocalizedResponse(
            const EppResponseSuccessLocalized& _epp_response_success_localized,
            const boost::gregorian::date& _expiration_date)
        : epp_response_success_localized(_epp_response_success_localized),
          expiration_date(_expiration_date)
    {
    }


};

} // namespace Epp::Domain
} // namespace Epp

#endif
