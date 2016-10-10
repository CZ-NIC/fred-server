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

/**
 *  @file
 */

#ifndef EPP_DOMAIN_CREATE_IMPL_H_68343a5af3724c88b97ea37df550d021
#define EPP_DOMAIN_CREATE_IMPL_H_68343a5af3724c88b97ea37df550d021

#include "src/fredlib/opcontext.h"

#include "src/epp/domain/domain_create.h"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace Epp {

struct DomainCreateResult {
    const unsigned long long id;
    const unsigned long long create_history_id;
    const boost::posix_time::ptime crtime;
    const boost::gregorian::date exdate;
    const unsigned length_of_domain_registration_in_years;

    DomainCreateResult(
        unsigned long long _domain_id,
        unsigned long long _create_history_id,
        const boost::posix_time::ptime& _domain_crdate,
        const boost::gregorian::date& _exdate,
        const unsigned _length_of_domain_registration_in_years
    ) :
        id(_domain_id),
        create_history_id(_create_history_id),
        crtime(_domain_crdate),
        exdate(_exdate),
        length_of_domain_registration_in_years(_length_of_domain_registration_in_years)
    { }
};

/**
 * @throws AuthErrorServerClosingConnection
 * @throws ObjectExists
 * @throws AggregatedParamErrors
 */
DomainCreateResult domain_create_impl(
    Fred::OperationContext& _ctx,
    const DomainCreateInputData& _data,
    unsigned long long _registrar_id,
    const Optional<unsigned long long>& _logd_request_id
);

}

#endif
