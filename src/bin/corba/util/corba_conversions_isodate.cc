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
#include "src/bin/corba/util/corba_conversions_isodate.hh"

#include "corba/IsoDate.hh"
#include "src/bin/corba/util/corba_conversions_string.hh"
#include "util/db/nullable.hh"

#include <boost/date_time/gregorian/gregorian.hpp>

namespace CorbaConversion {
namespace Util {

boost::gregorian::date
unwrap_IsoDate_to_boost_gregorian_date(
        const Registry::IsoDate src)
{
    try {
        return boost::gregorian::from_simple_string(src.value.in());
    }
    catch (const boost::bad_lexical_cast& e)
    {
        throw std::runtime_error("date is invalid");
    }
}

void
unwrap_IsoDate_to_boost_gregorian_date(
        const Registry::IsoDate src,
        boost::gregorian::date& dst)
{
    dst = unwrap_IsoDate_to_boost_gregorian_date(src);
}

Registry::IsoDate
wrap_boost_gregorian_date_to_IsoDate(
        const boost::gregorian::date& src)
{
    if (src.is_special()) {
        throw std::runtime_error("date is special");
    }

    Registry::IsoDate iso_date;
    iso_date.value = LibFred::Corba::wrap_string_to_corba_string(boost::gregorian::to_iso_extended_string(src))._retn();
    return iso_date;
}

void
wrap_boost_gregorian_date_to_IsoDate(
        const boost::gregorian::date& src,
        Registry::IsoDate& dst)
{
    if (src.is_special()) {
        throw std::runtime_error("date is special");
    }

    dst = wrap_boost_gregorian_date_to_IsoDate(src);
}

} // namespace CorbaConversion::Util
} // namespace CorbaConversion
