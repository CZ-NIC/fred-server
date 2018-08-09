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

#include "src/backend/epp/contact/impl/get_update_contact_check.hh"
#include "src/backend/epp/contact/impl/cznic/update_contact_check.hh"
#include "src/backend/epp/contact/impl/cznic/specific.hh"

#include <stdexcept>

namespace Epp {
namespace Contact {
namespace Impl {

namespace {

template <typename T>
CzNic::UpdateContactCheck::Operation from_option_to_operation(const HandleRifdArgs::Check& check)
{
    const auto option_value = check.get_value<T>();
    if (option_value == "show")
    {
        return CzNic::UpdateContactCheck::Operation::set_to_show;
    }
    if (option_value == "hide")
    {
        return CzNic::UpdateContactCheck::Operation::set_to_hide;
    }
    if (option_value.empty())
    {
        return CzNic::UpdateContactCheck::Operation::do_not_change;
    }
    throw std::runtime_error("unable convert string to value of CzNic::UpdateContactCheck::Operation type");
}

}//namespace Epp::Contact::Impl::{anonymous}

std::shared_ptr<Epp::Contact::UpdateOperationCheck> get_update_contact_check(const HandleRifdArgs::Check& check)
{
    if (check.is_type_of<CzNic::Specific>())
    {
        return std::make_shared<CzNic::UpdateContactCheck>(
                from_option_to_operation<CzNic::Specific::UpdateContact::Disclose::Name>(check),
                from_option_to_operation<CzNic::Specific::UpdateContact::Disclose::Organization>(check),
                from_option_to_operation<CzNic::Specific::UpdateContact::Disclose::Address>(check),
                from_option_to_operation<CzNic::Specific::UpdateContact::Disclose::Telephone>(check),
                from_option_to_operation<CzNic::Specific::UpdateContact::Disclose::Fax>(check),
                from_option_to_operation<CzNic::Specific::UpdateContact::Disclose::Email>(check),
                from_option_to_operation<CzNic::Specific::UpdateContact::Disclose::Vat>(check),
                from_option_to_operation<CzNic::Specific::UpdateContact::Disclose::Ident>(check),
                from_option_to_operation<CzNic::Specific::UpdateContact::Disclose::NotifyEmail>(check));
    }
    if (check.is_type_of<HandleRifdArgs::Check::Empty>())
    {
        class DummyUpdateContactCheck final : public Epp::Contact::UpdateOperationCheck
        {
        public:
            DummyUpdateContactCheck() = default;
        private:
            LibFred::UpdateContactByHandle& operator()(
                    LibFred::OperationContext&,
                    const LibFred::InfoContactData&,
                    const ContactChange&,
                    const SessionData&,
                    LibFred::UpdateContactByHandle& update_op)const override
            {
                return update_op;
            }
        };
        return std::make_shared<DummyUpdateContactCheck>();
    }
    throw std::runtime_error("unknown update contact check name");
}

}//namespace Epp::Contact::Impl
}//namespace Epp::Contact
}//namespace Epp
