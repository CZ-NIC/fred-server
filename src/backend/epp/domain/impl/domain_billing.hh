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
#ifndef DOMAIN_BILLING_HH_10AD57BE03DA43C183FEF188BEA3D93D
#define DOMAIN_BILLING_HH_10AD57BE03DA43C183FEF188BEA3D93D

#include "libfred/opcontext.hh"

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <string>

namespace Epp {
namespace Domain {

void create_domain_bill_item(
        const std::string& _fqdn,
        const boost::posix_time::ptime& _domain_create_timestamp_utc,
        unsigned long long _sponsoring_registrar_id,
        unsigned long long _created_domain_id,
        LibFred::OperationContext& _ctx);


void renew_domain_bill_item(
        const std::string& _fqdn,
        const boost::posix_time::ptime& _domain_renew_timestamp_utc,
        unsigned long long _sponsoring_registrar_id,
        unsigned long long _renewed_domain_id,
        int _length_of_domain_registration_in_months,
        const boost::gregorian::date& _old_domain_expiration_date_local,
        const boost::gregorian::date& _domain_expiration_date_local,
        LibFred::OperationContext& _ctx);


} // namespace Epp::Domain
} // namespace Epp

#endif
