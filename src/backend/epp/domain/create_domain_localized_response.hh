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
#ifndef CREATE_DOMAIN_LOCALIZED_RESPONSE_HH_90F34943E80E4308992A27922A4EB397
#define CREATE_DOMAIN_LOCALIZED_RESPONSE_HH_90F34943E80E4308992A27922A4EB397

#include "src/backend/epp/epp_response_success_localized.hh"

#include <boost/date_time/gregorian/greg_date.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <string>

namespace Epp {
namespace Domain {

struct CreateDomainLocalizedResponse
{
    const EppResponseSuccessLocalized epp_response_success_localized;
    const boost::posix_time::ptime crtime;
    const boost::gregorian::date expiration_date;


    CreateDomainLocalizedResponse(
            const EppResponseSuccessLocalized& _epp_response_success_localized,
            const boost::posix_time::ptime& _crtime,
            const boost::gregorian::date& _expiration_date)
        : epp_response_success_localized(_epp_response_success_localized),
          crtime(_crtime),
          expiration_date(_expiration_date)
    {
    }

};

} // namespace Epp::Domain
} // namespace Epp

#endif
