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

#ifndef INFO_ZONE_DATA_HH_D239F816DDA648F1B5728C1B1767F185
#define INFO_ZONE_DATA_HH_D239F816DDA648F1B5728C1B1767F185

#include "src/libfred/opcontext.hh"

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/optional.hpp>

namespace LibFred {
namespace Zone {

struct InfoZoneData
{
    std::string fqdn;
    boost::optional<boost::gregorian::months> ex_period_min;
    boost::optional<boost::gregorian::months> ex_period_max;
    boost::optional<boost::gregorian::months> val_period;
    int dots_max;
    bool enum_zone;
    bool warning_letter;

    bool operator==(const InfoZoneData& _other)
    {
        return (fqdn == _other.fqdn
                && ex_period_max == _other.ex_period_max
                && ex_period_min == _other.ex_period_min
                && val_period == _other.val_period
                && dots_max == _other.dots_max
                && enum_zone == _other.enum_zone
                && warning_letter == _other.warning_letter);
    }
};

} // namespace Zone
} // namespace LibFred

#endif
