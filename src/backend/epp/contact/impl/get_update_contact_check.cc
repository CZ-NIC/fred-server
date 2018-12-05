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
#include "src/backend/epp/contact/impl/set_unused/update_contact_check.hh"
#include "src/backend/epp/contact/impl/set_unused/config.hh"

#include <stdexcept>

namespace Epp {
namespace Contact {
namespace Impl {

namespace {

template <typename T>
CzNic::UpdateContactDataFilter::Operation from_cznic_option_to_operation(const ConfigDataFilter& filter)
{
    const auto option_value = filter.get_value<T>();
    if (option_value == "show")
    {
        return CzNic::UpdateContactDataFilter::Operation::set_to_show;
    }
    if (option_value == "hide")
    {
        return CzNic::UpdateContactDataFilter::Operation::set_to_hide;
    }
    if (option_value.empty())
    {
        return CzNic::UpdateContactDataFilter::Operation::do_not_change;
    }
    throw std::runtime_error("unable convert string to value of CzNic::UpdateContactCheck::Operation type");
}

template <typename T>
SetUnused::UpdateContactDataFilter::Operation from_set_unused_option_to_operation(const ConfigDataFilter& filter)
{
    const auto option_value = filter.get_value<T>();
    if (option_value == "show")
    {
        return SetUnused::UpdateContactDataFilter::Operation::set_to_show;
    }
    if (option_value == "hide")
    {
        return SetUnused::UpdateContactDataFilter::Operation::set_to_hide;
    }
    if (option_value.empty())
    {
        return SetUnused::UpdateContactDataFilter::Operation::do_not_change;
    }
    throw std::runtime_error("unable convert string to value of SetUnused::UpdateContactCheck::Operation type");
}

}//namespace Epp::Contact::Impl::{anonymous}

std::shared_ptr<Epp::Contact::UpdateContactDataFilter> get_update_contact_data_filter(const ConfigDataFilter& filter)
{
    if (filter.is_type_of<CzNic::Specific>())
    {
        return std::make_shared<CzNic::UpdateContactDataFilter>(
                from_cznic_option_to_operation<CzNic::Specific::UpdateContact::Disclose::Name>(filter),
                from_cznic_option_to_operation<CzNic::Specific::UpdateContact::Disclose::Organization>(filter),
                from_cznic_option_to_operation<CzNic::Specific::UpdateContact::Disclose::Address>(filter),
                from_cznic_option_to_operation<CzNic::Specific::UpdateContact::Disclose::Telephone>(filter),
                from_cznic_option_to_operation<CzNic::Specific::UpdateContact::Disclose::Fax>(filter),
                from_cznic_option_to_operation<CzNic::Specific::UpdateContact::Disclose::Email>(filter),
                from_cznic_option_to_operation<CzNic::Specific::UpdateContact::Disclose::Vat>(filter),
                from_cznic_option_to_operation<CzNic::Specific::UpdateContact::Disclose::Ident>(filter),
                from_cznic_option_to_operation<CzNic::Specific::UpdateContact::Disclose::NotifyEmail>(filter));
    }
    if (filter.is_type_of<SetUnused::Config>())
    {
        return std::make_shared<SetUnused::UpdateContactDataFilter>(
                from_set_unused_option_to_operation<SetUnused::Config::UpdateContact::Disclose::Name>(filter),
                from_set_unused_option_to_operation<SetUnused::Config::UpdateContact::Disclose::Organization>(filter),
                from_set_unused_option_to_operation<SetUnused::Config::UpdateContact::Disclose::Address>(filter),
                from_set_unused_option_to_operation<SetUnused::Config::UpdateContact::Disclose::Telephone>(filter),
                from_set_unused_option_to_operation<SetUnused::Config::UpdateContact::Disclose::Fax>(filter),
                from_set_unused_option_to_operation<SetUnused::Config::UpdateContact::Disclose::Email>(filter),
                from_set_unused_option_to_operation<SetUnused::Config::UpdateContact::Disclose::Vat>(filter),
                from_set_unused_option_to_operation<SetUnused::Config::UpdateContact::Disclose::Ident>(filter),
                from_set_unused_option_to_operation<SetUnused::Config::UpdateContact::Disclose::NotifyEmail>(filter));
    }
    if (filter.is_type_of<ConfigDataFilter::Empty>())
    {
        class EmptyUpdateContactDataFilter final : public Epp::Contact::UpdateContactDataFilter
        {
        public:
            EmptyUpdateContactDataFilter() = default;
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
        return std::make_shared<EmptyUpdateContactDataFilter>();
    }
    throw std::runtime_error("unknown update contact data filter name");
}

}//namespace Epp::Contact::Impl
}//namespace Epp::Contact
}//namespace Epp
