/*
 * Copyright (C) 2018-2020  CZ.NIC, z. s. p. o.
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
#ifndef CORBA_CONVERSIONS_NULLABLEISODATE_HH_B0312DBA49604F05BFC8F4879459192E
#define CORBA_CONVERSIONS_NULLABLEISODATE_HH_B0312DBA49604F05BFC8F4879459192E

#include "corba/NullableIsoDate.hh"
#include "util/db/nullable.hh"

#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/optional/optional.hpp>

namespace CorbaConversion {
namespace Util {

Nullable<boost::gregorian::date>
unwrap_NullableIsoDate_to_Nullable_boost_gregorian_date(
        const Registry::NullableIsoDate* src_ptr);

void
unwrap_NullableIsoDate_to_Nullable_boost_gregorian_date(
        const Registry::NullableIsoDate* src_ptr,
        Nullable<boost::gregorian::date>& dst);

boost::optional<boost::gregorian::date>
unwrap_NullableIsoDate_to_optional_boost_gregorian_date(
        const Registry::NullableIsoDate* src_ptr);

void
unwrap_NullableIsoDate_to_optional_boost_gregorian_date(
        const Registry::NullableIsoDate* src_ptr,
        boost::optional<boost::gregorian::date>& dst);

Registry::NullableIsoDate_var
wrap_Nullable_boost_gregorian_date_to_NullableIsoDate(
        const Nullable<boost::gregorian::date>& src);

} // namespace CorbaConversion::Util
} // namespace CorbaConversion

#endif
