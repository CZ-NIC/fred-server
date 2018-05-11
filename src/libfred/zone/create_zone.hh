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

#ifndef CREATE_ZONE_HH_3D464D2FBF714314991D5DF2D86C15F6
#define CREATE_ZONE_HH_3D464D2FBF714314991D5DF2D86C15F6

#include "src/libfred/opcontext.hh"

#include <boost/optional.hpp>


namespace LibFred {
namespace Zone {

class CreateZone
{
public:
    CreateZone(
            const std::string& _fqdn,
            int _expiration_period_min_in_months,
            int _expiration_period_max_in_months)
        : fqdn_(_fqdn),
          expiration_period_min_in_months_(_expiration_period_min_in_months),
          expiration_period_max_in_months_(_expiration_period_max_in_months),
          sending_warning_letter_(false)
    {
    }

    CreateZone& set_enum_validation_period(int _enum_validation_period_in_months);
    CreateZone& set_sending_warning_letter(bool _sending_warning_letter);
    unsigned long long exec(OperationContext& _ctx) const;

private:
    std::string fqdn_;
    int expiration_period_min_in_months_;
    int expiration_period_max_in_months_;
    bool sending_warning_letter_;
    boost::optional<int> enum_validation_period_in_months_;
};

} // namespace LibFred::Zone
} // namespace LibFred

#endif
