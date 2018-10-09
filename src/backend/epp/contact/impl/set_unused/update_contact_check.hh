/*
 * Copyright (C) 2018  CZ.NIC, z.s.p.o.
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

#ifndef UPDATE_CONTACT_CHECK_HH_53A04B8AEE936CF7CE7F4BA9ACD27875//date "+%s.%N"|md5sum|tr "[a-f]" "[A-F]"
#define UPDATE_CONTACT_CHECK_HH_53A04B8AEE936CF7CE7F4BA9ACD27875

#include "src/backend/epp/contact/update_operation_check.hh"

namespace Epp {
namespace Contact {
namespace Impl {
namespace SetUnused {

class UpdateContactCheck final : public Epp::Contact::UpdateOperationCheck
{
public:
    enum class Operation
    {
        set_to_show,
        set_to_hide,
        do_not_change
    };
    UpdateContactCheck();
    UpdateContactCheck(
            Operation default_disclose_name,
            Operation default_disclose_organization,
            Operation default_disclose_address,
            Operation default_disclose_telephone,
            Operation default_disclose_fax,
            Operation default_disclose_email,
            Operation default_disclose_vat,
            Operation default_disclose_ident,
            Operation default_disclose_notify_email);
private:
    LibFred::UpdateContactByHandle& operator()(
            LibFred::OperationContext& ctx,
            const LibFred::InfoContactData& old_data,
            const ContactChange& change,
            const SessionData& session_data,
            LibFred::UpdateContactByHandle& update_op)const override;
    const Operation default_disclose_name_;
    const Operation default_disclose_organization_;
    const Operation default_disclose_address_;
    const Operation default_disclose_telephone_;
    const Operation default_disclose_fax_;
    const Operation default_disclose_email_;
    const Operation default_disclose_vat_;
    const Operation default_disclose_ident_;
    const Operation default_disclose_notify_email_;
};

}//namespace Epp::Contact::Impl::SetUnused
}//namespace Epp::Contact::Impl
}//namespace Epp::Contact
}//namespace Epp

#endif//UPDATE_CONTACT_CHECK_HH_53A04B8AEE936CF7CE7F4BA9ACD27875
