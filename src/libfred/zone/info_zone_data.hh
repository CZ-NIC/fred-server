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

#include <boost/variant.hpp>

namespace LibFred {
namespace Zone {

struct Data
{
    std::string fqdn;
    int expiration_period_min_in_months;
    int expiration_period_max_in_months;
    int dots_max;
    bool sending_warning_letter;
};

struct NonEnumZone: Data
{
    bool operator==(const NonEnumZone& _other) const
    {
        return (fqdn == _other.fqdn
                && expiration_period_max_in_months == _other.expiration_period_max_in_months
                && expiration_period_min_in_months == _other.expiration_period_min_in_months
                && dots_max == _other.dots_max
                && sending_warning_letter == _other.sending_warning_letter);
    }
};

struct EnumZone: Data
{
    int validation_period_in_months;

    bool operator==(const EnumZone& _other) const
    {
        return (fqdn == _other.fqdn
                && expiration_period_max_in_months == _other.expiration_period_max_in_months
                && expiration_period_min_in_months == _other.expiration_period_min_in_months
                && validation_period_in_months == _other.validation_period_in_months
                && dots_max == _other.dots_max
                && sending_warning_letter == _other.sending_warning_letter);
    }
};

using InfoZoneData = boost::variant<boost::blank, NonEnumZone, EnumZone>;

} // namespace LibFred::Zone
} // namespace LibFred

#endif
