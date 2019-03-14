/*
 * Copyright (C) 2016-2019  CZ.NIC, z. s. p. o.
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
#ifndef LIMITS_HH_F49744AE86B047FEABFC3C79040E4AE3
#define LIMITS_HH_F49744AE86B047FEABFC3C79040E4AE3

namespace Epp {
namespace Keyset {

// allowed values from interval <min, max>
const unsigned min_number_of_tech_contacts =  1;
const unsigned max_number_of_tech_contacts = 10;

// allowed values from interval <min, max>
const unsigned min_number_of_dns_keys =  1;
const unsigned max_number_of_dns_keys = 10;

// allowed values from interval <min, max>
const unsigned min_number_of_ds_records = 0;
const unsigned max_number_of_ds_records = 0;

} // namespace Epp::Keyset
} // namespace Epp

#endif
