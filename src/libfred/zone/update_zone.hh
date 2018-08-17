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

#ifndef UPDATE_ZONE_HH_766AB0C28E7C44F6AEC45570BC6E00D7
#define UPDATE_ZONE_HH_766AB0C28E7C44F6AEC45570BC6E00D7

#include "src/libfred/opcontext.hh"

#include <boost/optional.hpp>
#include <string>

namespace LibFred {
namespace Zone {

class UpdateZone
{
public:
    explicit UpdateZone(const std::string& _fqdn);

    UpdateZone& set_expiration_period_min_in_months(boost::optional<int> _expiration_period_min_in_months);

    UpdateZone& set_expiration_period_max_in_months(boost::optional<int> _expiration_period_max_in_months);

    UpdateZone& set_enum_validation_period_in_months(boost::optional<int> _enum_validation_period_in_months);

    UpdateZone& set_sending_warning_letter(boost::optional<bool> _sending_warning_letter);

    unsigned long long exec(OperationContext& _ctx) const;

private:
    std::string fqdn_;
    boost::optional<int> expiration_period_min_in_months_;
    boost::optional<int> expiration_period_max_in_months_;
    boost::optional<int> enum_validation_period_in_months_;
    boost::optional<bool> sending_warning_letter_;
};

} // namespace LibFred::Zone
} // namespace LibFred

#endif
