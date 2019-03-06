/*
 * Copyright (C) 2017-2019  CZ.NIC, z. s. p. o.
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
#include "src/backend/epp/credit/client_credit_output_data.hh"

#include <stdexcept>

namespace Epp {
namespace Credit {

ZoneCredit::ZoneCredit(const std::string& _zone_fqdn)
    : zone_fqdn_(_zone_fqdn)
{ }

ZoneCredit::ZoneCredit(const std::string& _zone_fqdn, const Decimal& _credit)
    : zone_fqdn_(_zone_fqdn),
      credit_(_credit)
{ }

const std::string& ZoneCredit::get_zone_fqdn()const
{
    return zone_fqdn_;
}

bool ZoneCredit::has_credit()const
{
    return static_cast<bool>(credit_);
}

const Decimal& ZoneCredit::get_credit()const
{
    if (this->has_credit())
    {
        return *credit_;
    }
    throw std::runtime_error("unavailable information about credit for this zone");
}

bool OrderZoneCreditByZoneFqdn::operator()(const ZoneCredit& a, const ZoneCredit& b)const
{
    return a.get_zone_fqdn() < b.get_zone_fqdn();
}

} // namespace Epp::Credit
} // namespace Epp
