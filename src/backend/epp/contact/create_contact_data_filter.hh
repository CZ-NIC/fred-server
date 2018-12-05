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

#ifndef CREATE_CONTACT_DATA_FILTER_HH_6CF3D340D5D780F311B845C8C34088AE//date "+%s.%N"|md5sum|tr "[a-f]" "[A-F]"
#define CREATE_CONTACT_DATA_FILTER_HH_6CF3D340D5D780F311B845C8C34088AE

#include "src/libfred/opcontext.hh"
#include "src/libfred/registrable_object/contact/create_contact.hh"
#include "src/backend/epp/contact/create_contact_input_data.hh"
#include "src/backend/epp/session_data.hh"

namespace Epp {
namespace Contact {

class CreateContactDataFilter
{
public:
    struct DiscloseflagRulesViolation { };
    virtual ~CreateContactDataFilter() { }
    virtual LibFred::CreateContact& operator()(
            LibFred::OperationContext& ctx,
            const CreateContactInputData& contact_data,
            const SessionData& session_data,
            LibFred::CreateContact& create_op)const = 0;
};

}//namespace Epp::Contact
}//namespace Epp

#endif//CREATE_CONTACT_DATA_FILTER_HH_6CF3D340D5D780F311B845C8C34088AE
