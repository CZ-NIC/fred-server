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

bool operator==(const ::LibFred::Zone::EnumZone& _lhs, const ::LibFred::Zone::EnumZone& _rhs)
{
        return (_lhs.id == _rhs.id
            && _lhs.fqdn == _rhs.fqdn
            && _lhs.expiration_period_max_in_months == _rhs.expiration_period_max_in_months
            && _lhs.expiration_period_min_in_months == _rhs.expiration_period_min_in_months
            && _lhs.validation_period_in_months == _rhs.validation_period_in_months
            && _lhs.dots_max == _rhs.dots_max
            && _lhs.sending_warning_letter == _rhs.sending_warning_letter);
}

bool operator==(const ::LibFred::Zone::NonEnumZone& _lhs, const ::LibFred::Zone::NonEnumZone& _rhs)
{
        return (_lhs.id == _rhs.id
            && _lhs.fqdn == _rhs.fqdn
            && _lhs.expiration_period_max_in_months == _rhs.expiration_period_max_in_months
            && _lhs.expiration_period_min_in_months == _rhs.expiration_period_min_in_months
            && _lhs.dots_max == _rhs.dots_max
            && _lhs.sending_warning_letter == _rhs.sending_warning_letter);
}

} // namespace Test

