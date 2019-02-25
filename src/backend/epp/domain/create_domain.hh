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
#ifndef CREATE_DOMAIN_HH_09B61A4979AF4972874A0AAFF7875400
#define CREATE_DOMAIN_HH_09B61A4979AF4972874A0AAFF7875400

#include "libfred/opcontext.hh"

#include "src/backend/epp/domain/create_domain_localized.hh"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace Epp {
namespace Domain {

struct CreateDomainResult
{
    const unsigned long long id;
    const unsigned long long create_history_id;
    const boost::posix_time::ptime crtime;
    const boost::gregorian::date old_exdate;
    const boost::gregorian::date exdate;
    const unsigned length_of_domain_registration_in_months;


    CreateDomainResult(
            unsigned long long _domain_id,
            const unsigned long long _create_history_id,
            const boost::posix_time::ptime& _domain_crdate,
            const boost::gregorian::date& _old_exdate,
            const boost::gregorian::date& _exdate,
            const unsigned _length_of_domain_registration_in_months)
        : id(_domain_id),
          create_history_id(_create_history_id),
          crtime(_domain_crdate),
          old_exdate(_old_exdate),
          exdate(_exdate),
          length_of_domain_registration_in_months(_length_of_domain_registration_in_months)
    {
    }


};

CreateDomainResult create_domain(
        LibFred::OperationContext& _ctx,
        const CreateDomainInputData& _data,
        const CreateDomainConfigData& _create_domain_config_data,
        const SessionData& _session_data);


} // namespace Epp::Domain
} // namespace Epp

#endif
