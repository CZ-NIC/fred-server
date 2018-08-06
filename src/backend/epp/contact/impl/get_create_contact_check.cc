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

#include "src/backend/epp/contact/impl/get_create_contact_check.hh"
#include "src/backend/epp/contact/impl/cznic/create_contact_check.hh"
#include "src/backend/epp/contact/impl/cznic/specific.hh"

#include <stdexcept>

namespace Epp {
namespace Contact {
namespace Impl {

namespace {

template <typename T>
CzNic::CreateContactCheck::Data from_option_to_data(const HandleRifdArgs::Check& check)
{
    const auto option_value = check.get_value<T>();
    if (option_value == "show")
    {
        return CzNic::CreateContactCheck::Data::show;
    }
    if (option_value == "hide")
    {
        return CzNic::CreateContactCheck::Data::hide;
    }
    if (option_value.empty())
    {
        return CzNic::CreateContactCheck::Data::publishability_not_specified;
    }
    throw std::runtime_error("unable convert string to value of CzNic::CreateContactCheck::Data type");
}

}//namespace Epp::Contact::Impl::{anonymous}

std::shared_ptr<Epp::Contact::CreateOperationCheck> get_create_contact_check(const HandleRifdArgs::Check& check)
{
    if (check.is_type_of<CzNic::Specific>())
    {
        return std::make_shared<CzNic::CreateContactCheck>(
                from_option_to_data<CzNic::Specific::CreateContact::Disclose::Name>(check),
                from_option_to_data<CzNic::Specific::CreateContact::Disclose::Organization>(check),
                from_option_to_data<CzNic::Specific::CreateContact::Disclose::Address>(check),
                from_option_to_data<CzNic::Specific::CreateContact::Disclose::Telephone>(check),
                from_option_to_data<CzNic::Specific::CreateContact::Disclose::Fax>(check),
                from_option_to_data<CzNic::Specific::CreateContact::Disclose::Email>(check),
                from_option_to_data<CzNic::Specific::CreateContact::Disclose::Vat>(check),
                from_option_to_data<CzNic::Specific::CreateContact::Disclose::Ident>(check),
                from_option_to_data<CzNic::Specific::CreateContact::Disclose::NotifyEmail>(check));
    }
    if (check.is_type_of<HandleRifdArgs::Check::Empty>())
    {
        class DummyCreateContactCheck final:public Epp::Contact::CreateOperationCheck
        {
        public:
            DummyCreateContactCheck() = default;
        private:
            LibFred::CreateContact& operator()(
                    LibFred::OperationContext&,
                    const CreateContactInputData&,
                    const SessionData&,
                    LibFred::CreateContact& create_op)const override
            {
                return create_op;
            }
        };
        return std::make_shared<DummyCreateContactCheck>();
    }
    throw std::runtime_error("unknown create contact check name");
}

}//namespace Epp::Contact::Impl
}//namespace Epp::Contact
}//namespace Epp
