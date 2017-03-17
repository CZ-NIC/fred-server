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

#ifndef UPDATE_DOMAIN_INPUT_DATA_UNWRAPPED_H_A5CF26C84F874167A120E06106766722
#define UPDATE_DOMAIN_INPUT_DATA_UNWRAPPED_H_A5CF26C84F874167A120E06106766722

#include "src/corba/EPP.hh"

#include "src/corba/epp/corba_conversions.h"
#include "src/corba/epp/domain/domain_corba_conversions.h"
#include "src/corba/util/corba_conversions_string.h"
#include "src/epp/domain/domain_enum_validation.h"
#include "util/db/nullable.h"
#include "util/optional_value.h"

#include <string>
#include <vector>

namespace Fred {
namespace Corba {
namespace Epp {
namespace Domain {

struct UpdateDomainInputDataUnwrapped
{
    const std::string fqdn;
    const Optional<std::string> registrant_chg;
    const Optional<std::string> authinfopw_chg;
    const Optional<Nullable<std::string> > nsset_chg;
    const Optional<Nullable<std::string> > keyset_chg;
    const std::vector<std::string> admin_contacts_add;
    const std::vector<std::string> admin_contacts_rem;
    const std::vector<std::string> tmpcontacts_rem;
    const std::vector< ::Epp::Domain::EnumValidationExtension> enum_validation_extension_list;


    UpdateDomainInputDataUnwrapped(
            const char* _fqdn,
            const char* _registrant_chg,
            const char* _authinfopw_chg,
            const char* _nsset_chg,
            const char* _keyset_chg,
            const ccReg::AdminContact& _admin_contacts_add,
            const ccReg::AdminContact& _admin_contacts_rem,
            const ccReg::AdminContact& _tmpcontacts_rem,
            const ccReg::ExtensionList& _enum_validation_extension_list)
        : fqdn(Fred::Corba::unwrap_string_from_const_char_ptr(_fqdn)),
          registrant_chg(Fred::Corba::unwrap_string_for_change_to_Optional_string(_registrant_chg)),
          authinfopw_chg(Fred::Corba::unwrap_string_for_change_or_remove_to_Optional_string(_authinfopw_chg)),
          nsset_chg(unwrap_string_for_change_or_remove_to_Optional_Nullable_string(_nsset_chg)),
          keyset_chg(unwrap_string_for_change_or_remove_to_Optional_Nullable_string(_keyset_chg)),
          admin_contacts_add(unwrap_ccreg_admincontacts_to_vector_string(_admin_contacts_add)),
          admin_contacts_rem(unwrap_ccreg_admincontacts_to_vector_string(_admin_contacts_rem)),
          tmpcontacts_rem(unwrap_ccreg_admincontacts_to_vector_string(_tmpcontacts_rem)),
          enum_validation_extension_list(unwrap_enum_validation_extension_list(_enum_validation_extension_list))
    {
    }


};

} // namespace Fred::Corba::Epp::Domain
} // namespace Fred::Corba::Epp
} // namespace Fred::Corba
} // namespace Fred

#endif
