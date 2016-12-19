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

/**
 *  @file domain_update_impl.h
 *  <++>
 */

#ifndef DOMAIN_UPDATE_IMPL_H_B80377A0BC624CA296665A7136E08421
#define DOMAIN_UPDATE_IMPL_H_B80377A0BC624CA296665A7136E08421

#include "src/epp/contact/update_contact.h"
#include "src/epp/domain/domain_enum_validation.h"
#include "src/fredlib/opcontext.h"
#include "util/db/nullable.h"
#include "util/optional_value.h"

#include <string>

namespace Epp {

namespace Domain {

unsigned long long domain_update_impl(
    Fred::OperationContext& _ctx,
    const std::string& _domain_fqdn,
    const Optional<std::string>& _registrant_chg,
    const Optional<std::string>& _auth_info_pw_chg,
    const Optional<Nullable<std::string> >& _nsset_chg,
    const Optional<Nullable<std::string> >& _keyset_chg,
    const std::vector<std::string>& _admin_contacts_add,
    const std::vector<std::string>& _admin_contacts_rem,
    const std::vector<std::string>& _tmpcontacts_rem,
    const std::vector<Epp::ENUMValidationExtension>& _enum_validation_list,
    unsigned long long _registrar_id,
    const Optional<unsigned long long>& _logd_request_id,
    bool _rifd_epp_update_domain_keyset_clear);

}

}

#endif
