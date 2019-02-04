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

#ifndef UPDATE_DOMAIN_INPUT_DATA_HH_20735D14D1A54D8293D414DDFE019687
#define UPDATE_DOMAIN_INPUT_DATA_HH_20735D14D1A54D8293D414DDFE019687

#include "src/backend/epp/domain/domain_enum_validation.hh"
#include "util/db/nullable.hh"
#include "util/optional_value.hh"

#include <boost/optional.hpp>

#include <string>

namespace Epp {
namespace Domain {

struct UpdateDomainInputData
{
    const std::string fqdn;
    const Optional<std::string> registrant_chg;
    const Optional<std::string> authinfopw_chg;
    const Optional<Nullable<std::string>> nsset_chg;
    const Optional<Nullable<std::string>> keyset_chg;
    const std::vector<std::string> admin_contacts_add;
    const std::vector<std::string> admin_contacts_rem;
    const std::vector<std::string> tmpcontacts_rem;
    const boost::optional<::Epp::Domain::EnumValidationExtension> enum_validation;

    UpdateDomainInputData(
            const std::string& _fqdn,
            const Optional<std::string>& _registrant_chg,
            const Optional<std::string>& _authinfopw_chg,
            const Optional<Nullable<std::string>>& _nsset_chg,
            const Optional<Nullable<std::string>>& _keyset_chg,
            const std::vector<std::string>& _admin_contacts_add,
            const std::vector<std::string>& _admin_contacts_rem,
            const std::vector<std::string>& _tmpcontacts_rem,
            const boost::optional<::Epp::Domain::EnumValidationExtension>& _enum_validation)
        : fqdn(_fqdn),
          registrant_chg(_registrant_chg),
          authinfopw_chg(_authinfopw_chg),
          nsset_chg(_nsset_chg),
          keyset_chg(_keyset_chg),
          admin_contacts_add(_admin_contacts_add),
          admin_contacts_rem(_admin_contacts_rem),
          tmpcontacts_rem(_tmpcontacts_rem),
          enum_validation(_enum_validation)
    { }
};

} // namespace Epp::Domain
} // namespace Epp

#endif
