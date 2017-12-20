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

#ifndef CREATE_DOMAIN_LOCALIZED_RESPONSE_H_13DE8F1A56F34C89AAA5567A8696FB50
#define CREATE_DOMAIN_LOCALIZED_RESPONSE_H_13DE8F1A56F34C89AAA5567A8696FB50

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
