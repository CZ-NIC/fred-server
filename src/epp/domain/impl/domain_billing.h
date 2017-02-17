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

#ifndef DOMAIN_BILLING_H_01E6DC08BDFC491D954546D9307693DB
#define DOMAIN_BILLING_H_01E6DC08BDFC491D954546D9307693DB

#include "src/fredlib/opcontext.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include <string>

namespace Epp {
namespace Domain {

void create_domain_bill_item(const std::string& fqdn,
        const boost::posix_time::ptime& domain_create_timestamp_utc,
        unsigned long long sponsoring_registrar_id,
        unsigned long long created_domain_id,
        Fred::OperationContext& ctx);

void renew_domain_bill_item(const std::string& fqdn,
        const boost::posix_time::ptime& domain_renew_timestamp_utc,
        unsigned long long sponsoring_registrar_id,
        unsigned long long renewed_domain_id,
        int length_of_domain_registration_in_months,
        const boost::gregorian::date& old_domain_expiration_date_local,
        const boost::gregorian::date& domain_expiration_date_local,
        Fred::OperationContext& ctx);

} // namespace Epp::Domain
} // namespace Epp

#endif
