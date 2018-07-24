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

#ifndef CORBA_CONVERSIONS_ISODATE_HH_2BF5BCD875144165A64AD4C1BBC18508
#define CORBA_CONVERSIONS_ISODATE_HH_2BF5BCD875144165A64AD4C1BBC18508

#include "src/bin/corba/IsoDate.hh"

#include <boost/date_time/gregorian/gregorian_types.hpp>

namespace CorbaConversion {
namespace Util {

boost::gregorian::date
unwrap_IsoDate_to_boost_gregorian_date(
        const Registry::IsoDate src);

void
unwrap_IsoDate_to_boost_gregorian_date(
        const Registry::IsoDate src,
        boost::gregorian::date& dst);

Registry::IsoDate
wrap_boost_gregorian_date_to_IsoDate(
        const boost::gregorian::date& src);

void
wrap_boost_gregorian_date_to_IsoDate(
        const boost::gregorian::date& src,
        Registry::IsoDate& dst);

} // namespace CorbaConversion::Util
} // namespace CorbaConversion

#endif
