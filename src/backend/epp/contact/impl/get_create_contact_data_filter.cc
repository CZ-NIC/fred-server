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
#include "src/backend/epp/contact/impl/get_create_contact_data_filter.hh"
#include "src/backend/epp/contact/impl/cznic/create_contact_data_filter.hh"
#include "src/backend/epp/contact/impl/cznic/specific.hh"
#include "src/backend/epp/contact/impl/set_unused/create_contact_data_filter.hh"
#include "src/backend/epp/contact/impl/set_unused/config.hh"

#include <stdexcept>

namespace Epp {
namespace Contact {
namespace Impl {

namespace {

template <typename T>
CzNic::CreateContactDataFilter::Data from_option_to_cznic_data(const ConfigDataFilter& filter)
{
    const auto option_value = filter.get_value<T>();
    if (option_value == "show")
    {
        return CzNic::CreateContactDataFilter::Data::show;
    }
    if (option_value == "hide")
    {
        return CzNic::CreateContactDataFilter::Data::hide;
    }
    if (option_value.empty())
    {
        return CzNic::CreateContactDataFilter::Data::publishability_not_specified;
    }
    throw std::runtime_error("unable convert string to value of CzNic::CreateContactCheck::Data type");
}

template <typename T>
SetUnused::CreateContactDataFilter::Data from_option_to_set_unused_data(const ConfigDataFilter& filter)
{
    const auto option_value = filter.get_value<T>();
    if (option_value == "show")
    {
        return SetUnused::CreateContactDataFilter::Data::show;
    }
    if (option_value == "hide")
    {
        return SetUnused::CreateContactDataFilter::Data::hide;
    }
    if (option_value.empty())
    {
        return SetUnused::CreateContactDataFilter::Data::publishability_not_specified;
    }
    throw std::runtime_error("unable convert string to value of SetUnused::CreateContactCheck::Data type");
}

}//namespace Epp::Contact::Impl::{anonymous}

std::shared_ptr<Epp::Contact::CreateContactDataFilter> get_create_contact_data_filter(const ConfigDataFilter& filter)
{
    if (filter.is_type_of<CzNic::Specific>())
    {
        return std::make_shared<CzNic::CreateContactDataFilter>(
                from_option_to_cznic_data<CzNic::Specific::CreateContact::Disclose::Name>(filter),
                from_option_to_cznic_data<CzNic::Specific::CreateContact::Disclose::Organization>(filter),
                from_option_to_cznic_data<CzNic::Specific::CreateContact::Disclose::Address>(filter),
                from_option_to_cznic_data<CzNic::Specific::CreateContact::Disclose::Telephone>(filter),
                from_option_to_cznic_data<CzNic::Specific::CreateContact::Disclose::Fax>(filter),
                from_option_to_cznic_data<CzNic::Specific::CreateContact::Disclose::Email>(filter),
                from_option_to_cznic_data<CzNic::Specific::CreateContact::Disclose::Vat>(filter),
                from_option_to_cznic_data<CzNic::Specific::CreateContact::Disclose::Ident>(filter),
                from_option_to_cznic_data<CzNic::Specific::CreateContact::Disclose::NotifyEmail>(filter));
    }
    if (filter.is_type_of<SetUnused::Config>())
    {
        return std::make_shared<SetUnused::CreateContactDataFilter>(
                from_option_to_set_unused_data<SetUnused::Config::CreateContact::Disclose::Name>(filter),
                from_option_to_set_unused_data<SetUnused::Config::CreateContact::Disclose::Organization>(filter),
                from_option_to_set_unused_data<SetUnused::Config::CreateContact::Disclose::Address>(filter),
                from_option_to_set_unused_data<SetUnused::Config::CreateContact::Disclose::Telephone>(filter),
                from_option_to_set_unused_data<SetUnused::Config::CreateContact::Disclose::Fax>(filter),
                from_option_to_set_unused_data<SetUnused::Config::CreateContact::Disclose::Email>(filter),
                from_option_to_set_unused_data<SetUnused::Config::CreateContact::Disclose::Vat>(filter),
                from_option_to_set_unused_data<SetUnused::Config::CreateContact::Disclose::Ident>(filter),
                from_option_to_set_unused_data<SetUnused::Config::CreateContact::Disclose::NotifyEmail>(filter));
    }
    if (filter.is_type_of<ConfigDataFilter::Empty>())
    {
        class EmptyCreateContactDataFilter final : public Epp::Contact::CreateContactDataFilter
        {
        public:
            EmptyCreateContactDataFilter() = default;
        private:
            LibFred::CreateContact& operator()(
                    LibFred::OperationContext&,
                    const CreateContactInputData& input_data,
                    const SessionData&,
                    LibFred::CreateContact& create_op)const override
            {
                if (input_data.name.is_publishability_specified() &&
                    input_data.organization.is_publishability_specified() &&
                    input_data.address.is_publishability_specified() &&
                    input_data.telephone.is_publishability_specified() &&
                    input_data.fax.is_publishability_specified() &&
                    input_data.email.is_publishability_specified() &&
                    input_data.vat.is_publishability_specified() &&
                    input_data.ident.is_publishability_specified() &&
                    input_data.notify_email.is_publishability_specified())
                {
                    return create_op;
                }
                throw std::runtime_error("publishability must be specified");
            }
        };
        return std::make_shared<EmptyCreateContactDataFilter>();
    }
    throw std::runtime_error("unknown create contact data filter name");
}

}//namespace Epp::Contact::Impl
}//namespace Epp::Contact
}//namespace Epp
