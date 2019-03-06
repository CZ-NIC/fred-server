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
#ifndef DOMAIN_EXPIRATION_DATETIME_HH_564FEC20D41643E9B256B3FAF736A818
#define DOMAIN_EXPIRATION_DATETIME_HH_564FEC20D41643E9B256B3FAF736A818

#include "libfred/opcontext.hh"
#include "util/optional_value.hh"
#include <boost/date_time/posix_time/posix_time.hpp>

namespace Fred {
namespace Backend {
namespace Whois {

/**
 * From domain expiration and object state procedure parameters computes
 * datetime when status 'expired' should be set
 *
 * @param   _exdate    domain expiration date
 * @return  domain expiration datetime
 */
boost::posix_time::ptime domain_expiration_datetime_estimate(
        LibFred::OperationContext& _ctx,
        const boost::gregorian::date& _exdate);


/**
 * @param   _domain_id   id of domain
 * @return  domain 'expired' status valid from date time if this status is active
 *          if _domain_id or active status is not found it returns ''not set'' optional value
 */
Optional<boost::posix_time::ptime> domain_expiration_datetime_actual(
        LibFred::OperationContext& _ctx,
        unsigned long long _domain_id);


/**
 * From domain validation expiration and object state procedure parameters computes
 * datetime when status 'not_validated' should be set
 *
 * @param   _exdate    domain validation expiration date
 * @return  domain validation expiration datetime
 */
boost::posix_time::ptime domain_validation_expiration_datetime_estimate(
        LibFred::OperationContext& _ctx,
        const boost::gregorian::date& _exdate);


/**
 * @param   _domain_id   id of domain
 * @return  domain 'not_validated' status valid from date time if this status is active
 *          if _domain_id or active status is not found it returns ''not set'' optional value
 */
Optional<boost::posix_time::ptime> domain_validation_expiration_datetime_actual(
        LibFred::OperationContext& _ctx,
        unsigned long long _domain_id);


} // namespace Fred::Backend::Whois
} // namespace Fred::Backend
} // namespace Fred

#endif
