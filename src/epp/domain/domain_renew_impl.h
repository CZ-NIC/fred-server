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

#ifndef EPP_DOMAIN_RENEW_IMPL_H_f2c41138ea534e05b750ee12b6c7ea0c
#define EPP_DOMAIN_RENEW_IMPL_H_f2c41138ea534e05b750ee12b6c7ea0c

#include "src/fredlib/opcontext.h"

#include "src/epp/domain/domain_renew.h"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace Epp {

struct DomainRenewResult {
    const unsigned long long domain_id;
    const unsigned long long domain_history_id;
    const boost::posix_time::ptime curent_time;
    const boost::gregorian::date exdate;
    const unsigned length_of_domain_registration_in_years;

    DomainRenewResult(
        const unsigned long long _domain_id,
        const unsigned long long _domain_history_id,
        const boost::posix_time::ptime& _curent_time,
        const boost::gregorian::date& _exdate,
        const unsigned _length_of_domain_registration_in_years
    ) :
        domain_id(_domain_id),
        domain_history_id(_domain_history_id),
        curent_time(_curent_time),
        exdate(_exdate),
        length_of_domain_registration_in_years(_length_of_domain_registration_in_years)
    { }
};

DomainRenewResult domain_renew_impl(
    Fred::OperationContext& _ctx,
    const DomainRenewInputData& _data,
    unsigned long long _registrar_id,
    const Optional<unsigned long long>& _logd_request_id
);

}

#endif
