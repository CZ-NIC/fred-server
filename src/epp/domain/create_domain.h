/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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

#ifndef CREATE_DOMAIN_H_7C2F051F1B4049FCBD49A2067CB12083
#define CREATE_DOMAIN_H_7C2F051F1B4049FCBD49A2067CB12083

#include "src/fredlib/opcontext.h"

#include "src/epp/domain/create_domain_localized.h"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace Epp {
namespace Domain {

struct CreateDomainResult {
    const unsigned long long id;
    const unsigned long long create_history_id;
    const boost::posix_time::ptime crtime;
    const boost::gregorian::date old_exdate;
    const boost::gregorian::date exdate;
    const unsigned length_of_domain_registration_in_years;

    CreateDomainResult(
        unsigned long long _domain_id,
        unsigned long long _create_history_id,
        const boost::posix_time::ptime& _domain_crdate,
        const boost::gregorian::date& _old_exdate,
        const boost::gregorian::date& _exdate,
        const unsigned _length_of_domain_registration_in_years
    ) :
        id(_domain_id),
        create_history_id(_create_history_id),
        crtime(_domain_crdate),
        old_exdate(_old_exdate),
        exdate(_exdate),
        length_of_domain_registration_in_years(_length_of_domain_registration_in_years)
    { }
};

/**
 * @throws AuthErrorServerClosingConnection
 * @throws ObjectExists
 * @throws AggregatedParamErrors
 */
CreateDomainResult create_domain(
    Fred::OperationContext& _ctx,
    const CreateDomainInputData& _data,
    unsigned long long _registrar_id,
    const Optional<unsigned long long>& _logd_request_id
);

} // namespace Epp::Domain
} // namespace Epp

#endif
