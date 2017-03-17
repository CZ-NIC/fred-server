/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
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

#ifndef CHECK_KEYSET_CONFIG_DATA_H_D725D1AC22374EACBE6E6DB7AC615A1E
#define CHECK_KEYSET_CONFIG_DATA_H_D725D1AC22374EACBE6E6DB7AC615A1E

namespace Epp {
namespace Keyset {

struct CheckKeysetConfigData
{
    const bool rifd_epp_operations_charging;


    CheckKeysetConfigData(
            const bool _rifd_epp_operations_charging)
        : rifd_epp_operations_charging(_rifd_epp_operations_charging)
    {
    }


};

} // namespace Epp::Keyset
} // namespace Epp

#endif
