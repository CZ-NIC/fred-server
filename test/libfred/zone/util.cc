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
#include "test/libfred/zone/util.hh"

namespace Test {

bool operator==(const ::LibFred::Zone::EnumZone& _first, const ::LibFred::Zone::EnumZone& _second)
{
        return (_first.fqdn == _second.fqdn
            && _first.expiration_period_max_in_months == _second.expiration_period_max_in_months
            && _first.expiration_period_min_in_months == _second.expiration_period_min_in_months
            && _first.validation_period_in_months == _second.validation_period_in_months
            && _first.dots_max == _second.dots_max
            && _first.sending_warning_letter == _second.sending_warning_letter);
}

bool operator==(const ::LibFred::Zone::NonEnumZone& _first, const ::LibFred::Zone::NonEnumZone& _second)
{
        return (_first.fqdn == _second.fqdn
            && _first.expiration_period_max_in_months == _second.expiration_period_max_in_months
            && _first.expiration_period_min_in_months == _second.expiration_period_min_in_months
            && _first.dots_max == _second.dots_max
            && _first.sending_warning_letter == _second.sending_warning_letter);
}

} // namespace Test

