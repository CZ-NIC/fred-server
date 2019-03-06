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
#ifndef CREATE_CONTACT_DATA_FILTER_HH_81C90864A65006DED8E05B59BC583804//date "+%s.%N"|md5sum|tr "[a-f]" "[A-F]"
#define CREATE_CONTACT_DATA_FILTER_HH_81C90864A65006DED8E05B59BC583804

#include "src/backend/epp/contact/create_contact_data_filter.hh"

namespace Epp {
namespace Contact {
namespace Impl {
namespace CzNic {

class CreateContactDataFilter final : public Epp::Contact::CreateContactDataFilter
{
public:
    enum class Data
    {
        show,
        hide,
        publishability_not_specified
    };
    CreateContactDataFilter();
    CreateContactDataFilter(
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

}//namespace Epp::Contact::Impl::CzNic
}//namespace Epp::Contact::Impl
}//namespace Epp::Contact
}//namespace Epp

#endif//CREATE_CONTACT_DATA_FILTER_HH_81C90864A65006DED8E05B59BC583804
