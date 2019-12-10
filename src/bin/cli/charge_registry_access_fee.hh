/*
 * Copyright (C) 2019  CZ.NIC, z. s. p. o.
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

#ifndef CHARGE_REGISTRY_ACCESS_FEE_HH_8D913585F8EE478281812C6013EEC041
#define CHARGE_REGISTRY_ACCESS_FEE_HH_8D913585F8EE478281812C6013EEC041

#include <boost/date_time/gregorian/gregorian.hpp>

#include <string>
#include <vector>

namespace Admin {

void chargeRegistryAccessFee(
        bool _all_registrars,
        const std::vector<std::string>& _only_registrars,
        const std::vector<std::string>& _except_registrars,
        const boost::gregorian::date& _date_from,
        const boost::gregorian::date& _date_to,
        const std::string& _zone,
        const std::string& _registry_timezone);

} // namespace Admin

#endif
