/*
 * Copyright (C) 2018  CZ.NIC, z.s.p.o.
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

#ifndef CORBA_CONVERSIONS_NULLABLEISODATE_HH_B0312DBA49604F05BFC8F4879459192E
#define CORBA_CONVERSIONS_NULLABLEISODATE_HH_B0312DBA49604F05BFC8F4879459192E

#include "src/bin/corba/NullableIsoDate.hh"
#include "util/db/nullable.hh"

#include <boost/date_time/gregorian/gregorian_types.hpp>

namespace CorbaConversion {
namespace Util {

Nullable<boost::gregorian::date>
unwrap_NullableIsoDate_to_Nullable_boost_gregorian_date(
        const Registry::NullableIsoDate* src_ptr);

void
unwrap_NullableIsoDate_to_Nullable_boost_gregorian_date(
        const Registry::NullableIsoDate* src_ptr,
        Nullable<boost::gregorian::date>& dst);

Registry::NullableIsoDate_var
wrap_Nullable_boost_gregorian_date_to_NullableIsoDate(
        const Nullable<boost::gregorian::date>& src);

} // namespace CorbaConversion::Util
} // namespace CorbaConversion

#endif
