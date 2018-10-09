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

#ifndef CREATE_CONTACT_CHECK_HH_260B79989EC10A1E11D6CAA14E2744AB//date "+%s.%N"|md5sum|tr "[a-f]" "[A-F]"
#define CREATE_CONTACT_CHECK_HH_260B79989EC10A1E11D6CAA14E2744AB

#include "src/backend/epp/contact/create_operation_check.hh"

namespace Epp {
namespace Contact {
namespace Impl {
namespace SetUnused {

class CreateContactCheck final : public Epp::Contact::CreateOperationCheck
{
public:
    enum class Data
    {
        show,
        hide,
        publishability_not_specified
    };
    CreateContactCheck();
    CreateContactCheck(
            Data default_disclose_name,
            Data default_disclose_organization,
            Data default_disclose_address,
            Data default_disclose_telephone,
            Data default_disclose_fax,
            Data default_disclose_email,
            Data default_disclose_vat,
            Data default_disclose_ident,
            Data default_disclose_notify_email);
private:
    LibFred::CreateContact& operator()(
            LibFred::OperationContext& ctx,
            const CreateContactInputData& contact_data,
            const SessionData& session_data,
            LibFred::CreateContact& create_op)const override;
    const Data default_disclose_name_;
    const Data default_disclose_organization_;
    const Data default_disclose_address_;
    const Data default_disclose_telephone_;
    const Data default_disclose_fax_;
    const Data default_disclose_email_;
    const Data default_disclose_vat_;
    const Data default_disclose_ident_;
    const Data default_disclose_notify_email_;
};

}//namespace Epp::Contact::Impl::SetUnused
}//namespace Epp::Contact::Impl
}//namespace Epp::Contact
}//namespace Epp

#endif//CREATE_CONTACT_CHECK_HH_260B79989EC10A1E11D6CAA14E2744AB
