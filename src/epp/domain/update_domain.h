/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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

#ifndef UPDATE_DOMAIN_H_2D1D225B719341E0B0F4384B04E36D74
#define UPDATE_DOMAIN_H_2D1D225B719341E0B0F4384B04E36D74

#include "src/epp/contact/update_contact.h"
#include "src/epp/domain/impl/domain_enum_validation.h"
#include "src/fredlib/opcontext.h"
#include "util/db/nullable.h"
#include "util/optional_value.h"

#include <string>

namespace Epp {
namespace Domain {

unsigned long long update_domain(
        Fred::OperationContext& _ctx,
        const std::string& _domain_fqdn,
        const Optional<std::string>& _registrant_chg,
        const Optional<std::string>& _auth_info_pw_chg,
        const Optional<Nullable<std::string> >& _nsset_chg,
        const Optional<Nullable<std::string> >& _keyset_chg,
        const std::vector<std::string>& _admin_contacts_add,
        const std::vector<std::string>& _admin_contacts_rem,
        const std::vector<std::string>& _tmpcontacts_rem,
        const std::vector<EnumValidationExtension>& _enum_validation_list,
        unsigned long long _registrar_id,
        const Optional<unsigned long long>& _logd_request_id,
        bool _rifd_epp_update_domain_keyset_clear);

} // namespace Epp::Domain
} // namespace Epp

#endif
