/*
 * Copyright (C) 2018-2019  CZ.NIC, z. s. p. o.
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
#ifndef UPDATE_CONTACT_DATA_FILTER_HH_F26DDFC8A2F6B4F427A86B163A68B1D4//date "+%s.%N"|md5sum|tr "[a-f]" "[A-F]"
#define UPDATE_CONTACT_DATA_FILTER_HH_F26DDFC8A2F6B4F427A86B163A68B1D4

#include "libfred/opcontext.hh"
#include "libfred/registrable_object/contact/info_contact_data.hh"
#include "libfred/registrable_object/contact/update_contact.hh"
#include "src/backend/epp/contact/contact_change.hh"
#include "src/backend/epp/session_data.hh"

namespace Epp {
namespace Contact {

class UpdateContactDataFilter
{
public:
    struct DiscloseflagRulesViolation { };
    virtual ~UpdateContactDataFilter() { }
    virtual LibFred::UpdateContactByHandle& operator()(
            LibFred::OperationContext& ctx,
            const LibFred::InfoContactData& old_data,
            const ContactChange& change,
            const SessionData& session_data,
            LibFred::UpdateContactByHandle& update_op)const = 0;
};

}//namespace Epp::Contact
}//namespace Epp

#endif//UPDATE_CONTACT_DATA_FILTER_HH_F26DDFC8A2F6B4F427A86B163A68B1D4
