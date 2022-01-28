/*
 * Copyright (C) 2021  CZ.NIC, z. s. p. o.
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
#ifndef CONTACT_DATA_SHARE_POLICY_RULES_HH_53E0BB635FD48C9637BBC7A03CD00CDD//date "+%s.%N"|md5sum|tr "[a-f]" "[A-F]"
#define CONTACT_DATA_SHARE_POLICY_RULES_HH_53E0BB635FD48C9637BBC7A03CD00CDD

#include "libfred/opcontext.hh"
#include "libfred/registrable_object/contact/info_contact_data.hh"

#include "src/backend/epp/password.hh"
#include "src/backend/epp/session_data.hh"

#include <string>

namespace Epp {
namespace Contact {

class ContactDataSharePolicyRules
{
public:
    struct InvalidAuthorizationInformation { };
    virtual ~ContactDataSharePolicyRules() { }
    virtual LibFred::InfoContactData& apply(
            LibFred::OperationContext& ctx,
            const Password& contact_authinfopw,
            const SessionData& session_data,
            LibFred::InfoContactData& contact_data) const = 0;
};

}//namespace Epp::Contact
}//namespace Epp

#endif//CONTACT_DATA_SHARE_POLICY_RULES_HH_53E0BB635FD48C9637BBC7A03CD00CDD
