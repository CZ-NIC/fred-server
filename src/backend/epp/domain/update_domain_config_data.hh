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

#ifndef UPDATE_DOMAIN_CONFIG_DATA_HH_00C83E0B35DB4CE48F94DA5827CEA0F6
#define UPDATE_DOMAIN_CONFIG_DATA_HH_00C83E0B35DB4CE48F94DA5827CEA0F6

#include <string>

namespace Epp {
namespace Domain {

struct UpdateDomainConfigData
{
    const bool rifd_epp_operations_charging;
    const bool rifd_epp_update_domain_keyset_clear;


    UpdateDomainConfigData(
            const bool _rifd_epp_operations_charging,
            const bool _rifd_epp_update_domain_keyset_clear)
        : rifd_epp_operations_charging(_rifd_epp_operations_charging),
          rifd_epp_update_domain_keyset_clear(_rifd_epp_update_domain_keyset_clear)
    {
    }


};

} // namespace Epp::Domain
} // namespace Epp

#endif
