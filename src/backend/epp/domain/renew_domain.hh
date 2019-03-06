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
#ifndef RENEW_DOMAIN_HH_EA4471E7A0CA4E0A932E089E0A25E5A7
#define RENEW_DOMAIN_HH_EA4471E7A0CA4E0A932E089E0A25E5A7

#include "src/backend/epp/domain/renew_domain_config_data.hh"
#include "src/backend/epp/domain/renew_domain_localized.hh"
#include "src/backend/epp/session_data.hh"
#include "libfred/opcontext.hh"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace Epp {
namespace Domain {

struct RenewDomainResult
{
    const unsigned long long domain_id;
    const unsigned long long domain_history_id;
    const boost::posix_time::ptime curent_time;
    const boost::gregorian::date old_exdate;
    const boost::gregorian::date exdate;
    const unsigned length_of_domain_registration_in_months;


    RenewDomainResult(
            const unsigned long long _domain_id,
            const unsigned long long _domain_history_id,
            const boost::posix_time::ptime& _curent_time,
            const boost::gregorian::date& _old_exdate,
            const boost::gregorian::date& _exdate,
            const unsigned _length_of_domain_registration_in_months)
        : domain_id(_domain_id),
          domain_history_id(_domain_history_id),
          curent_time(_curent_time),
          old_exdate(_old_exdate),
          exdate(_exdate),
          length_of_domain_registration_in_months(_length_of_domain_registration_in_months)
    {
    }


};

RenewDomainResult renew_domain(
        LibFred::OperationContext& _ctx,
        const RenewDomainInputData& _renew_domain_input_data,
        const RenewDomainConfigData& _renew_domain_config_data,
        const SessionData& _session_data);


} // namespace Epp::Domain
} // namespace Epp

#endif
