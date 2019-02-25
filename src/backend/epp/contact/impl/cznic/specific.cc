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
#include "src/backend/epp/contact/impl/cznic/specific.hh"

#include "src/backend/epp/contact/config_data_filter.hh"

#include <stdexcept>

namespace Epp {
namespace Contact {
namespace Impl {
namespace CzNic {

const std::string& Specific::get_contact_data_filter_name()
{
    static const std::string singleton = "cznic_specific";
    return singleton;
}

bool Specific::is_name_of_this_contact_data_filter(const std::string& filter_name)
{
    return filter_name == get_contact_data_filter_name();
}

}//namespace Epp::Contact::Impl::CzNic
}//namespace Epp::Contact::Impl


using CzNicSpecific = Impl::CzNic::Specific;
using po_variables_map = boost::program_options::variables_map;

namespace {

template <typename>
const char* get_key_str();

template <>
const char* get_key_str<CzNicSpecific::CreateContact::Disclose::Name>()
{
    return "rifd::cznic_specific::create_contact.default_disclosename";
}

template <>
const char* get_key_str<CzNicSpecific::CreateContact::Disclose::Organization>()
{
    return "rifd::cznic_specific::create_contact.default_discloseorganization";
}

template <>
const char* get_key_str<CzNicSpecific::CreateContact::Disclose::Address>()
{
    return "rifd::cznic_specific::create_contact.default_discloseaddress";
}

template <>
const char* get_key_str<CzNicSpecific::CreateContact::Disclose::Telephone>()
{
    return "rifd::cznic_specific::create_contact.default_disclosetelephone";
}

template <>
const char* get_key_str<CzNicSpecific::CreateContact::Disclose::Fax>()
{
    return "rifd::cznic_specific::create_contact.default_disclosefax";
}

template <>
const char* get_key_str<CzNicSpecific::CreateContact::Disclose::Email>()
{
    return "rifd::cznic_specific::create_contact.default_discloseemail";
}

template <>
const char* get_key_str<CzNicSpecific::CreateContact::Disclose::Vat>()
{
    return "rifd::cznic_specific::create_contact.default_disclosevat";
}

template <>
const char* get_key_str<CzNicSpecific::CreateContact::Disclose::Ident>()
{
    return "rifd::cznic_specific::create_contact.default_discloseident";
}

template <>
const char* get_key_str<CzNicSpecific::CreateContact::Disclose::NotifyEmail>()
{
    return "rifd::cznic_specific::create_contact.default_disclosenotifyemail";
}

template <>
const char* get_key_str<CzNicSpecific::UpdateContact::Disclose::Name>()
{
    return "rifd::cznic_specific::update_contact.default_disclosename";
}

template <>
const char* get_key_str<CzNicSpecific::UpdateContact::Disclose::Organization>()
{
    return "rifd::cznic_specific::update_contact.default_discloseorganization";
}

template <>
const char* get_key_str<CzNicSpecific::UpdateContact::Disclose::Address>()
{
    return "rifd::cznic_specific::update_contact.default_discloseaddress";
}

template <>
const char* get_key_str<CzNicSpecific::UpdateContact::Disclose::Telephone>()
{
    return "rifd::cznic_specific::update_contact.default_disclosetelephone";
}

template <>
const char* get_key_str<CzNicSpecific::UpdateContact::Disclose::Fax>()
{
    return "rifd::cznic_specific::update_contact.default_disclosefax";
}

template <>
const char* get_key_str<CzNicSpecific::UpdateContact::Disclose::Email>()
{
    return "rifd::cznic_specific::update_contact.default_discloseemail";
}

template <>
const char* get_key_str<CzNicSpecific::UpdateContact::Disclose::Vat>()
{
    return "rifd::cznic_specific::update_contact.default_disclosevat";
}

template <>
const char* get_key_str<CzNicSpecific::UpdateContact::Disclose::Ident>()
{
    return "rifd::cznic_specific::update_contact.default_discloseident";
}

template <>
const char* get_key_str<CzNicSpecific::UpdateContact::Disclose::NotifyEmail>()
{
    return "rifd::cznic_specific::update_contact.default_disclosenotifyemail";
}

template <typename T>
const std::string& get_key()
{
    static const std::string singleton = get_key_str<T>();
    return singleton;
}

template <typename T>
void set_value(const po_variables_map& vm, ConfigDataFilter::KeyValue& options)
{
    const auto key_value_itr = options.find(get_key<T>());
    if (key_value_itr != options.end())
    {
        struct KeyIsOccupied : std::runtime_error
        {
            KeyIsOccupied() : std::runtime_error("Key is occupied") { }
        };
        throw KeyIsOccupied();
    }
    options.emplace(get_key<T>(), vm[get_key<T>()].template as<std::string>());
}

auto default_value_is_to_show() { return boost::program_options::value<std::string>()->default_value("show"); }
auto default_value_is_to_hide() { return boost::program_options::value<std::string>()->default_value("hide"); }
auto default_value_is_empty() { return boost::program_options::value<std::string>()->default_value(std::string()); }

}//namespace {anonymous}

template <>
void ConfigDataFilter::add_options_description<CzNicSpecific>(
        boost::program_options::options_description& options_description)
{
    options_description.add_options()
            (get_key_str<CzNicSpecific::CreateContact::Disclose::Name>(),
             default_value_is_to_show(),
             "default value of disclose name flag not shown in epp xml schemas for a create_contact operation")
            (get_key_str<CzNicSpecific::CreateContact::Disclose::Organization>(),
             default_value_is_to_show(),
             "default value of disclose organization flag not shown in epp xml schemas for a create_contact operation")
            (get_key_str<CzNicSpecific::CreateContact::Disclose::Address>(),
             default_value_is_to_show(),
             "default value of disclose address flag not shown in epp xml schemas for a create_contact operation")
            (get_key_str<CzNicSpecific::CreateContact::Disclose::Telephone>(),
             default_value_is_to_hide(),
             "default value of disclose telephone flag not shown in epp xml schemas for a create_contact operation")
            (get_key_str<CzNicSpecific::CreateContact::Disclose::Fax>(),
             default_value_is_to_hide(),
             "default value of disclose fax flag not shown in epp xml schemas for a create_contact operation")
            (get_key_str<CzNicSpecific::CreateContact::Disclose::Email>(),
             default_value_is_to_hide(),
             "default value of disclose email flag not shown in epp xml schemas for a create_contact operation")
            (get_key_str<CzNicSpecific::CreateContact::Disclose::Vat>(),
             default_value_is_to_hide(),
             "default value of disclose vat flag not shown in epp xml schemas for a create_contact operation")
            (get_key_str<CzNicSpecific::CreateContact::Disclose::Ident>(),
             default_value_is_to_hide(),
             "default value of disclose ident flag not shown in epp xml schemas for a create_contact operation")
            (get_key_str<CzNicSpecific::CreateContact::Disclose::NotifyEmail>(),
             default_value_is_to_hide(),
             "default value of disclose notifyemail flag not shown in epp xml schemas for a create_contact operation")
            (get_key_str<CzNicSpecific::UpdateContact::Disclose::Name>(),
             default_value_is_empty(),
             "default value of disclose name flag not shown in epp xml schemas for a update_contact operation")
            (get_key_str<CzNicSpecific::UpdateContact::Disclose::Organization>(),
             default_value_is_empty(),
             "default value of disclose organization flag not shown in epp xml schemas for a update_contact operation")
            (get_key_str<CzNicSpecific::UpdateContact::Disclose::Address>(),
             default_value_is_empty(),
             "default value of disclose address flag not shown in epp xml schemas for a update_contact operation")
            (get_key_str<CzNicSpecific::UpdateContact::Disclose::Telephone>(),
             default_value_is_empty(),
             "default value of disclose telephone flag not shown in epp xml schemas for a update_contact operation")
            (get_key_str<CzNicSpecific::UpdateContact::Disclose::Fax>(),
             default_value_is_empty(),
             "default value of disclose fax flag not shown in epp xml schemas for a update_contact operation")
            (get_key_str<CzNicSpecific::UpdateContact::Disclose::Email>(),
             default_value_is_empty(),
             "default value of disclose email flag not shown in epp xml schemas for a update_contact operation")
            (get_key_str<CzNicSpecific::UpdateContact::Disclose::Vat>(),
             default_value_is_empty(),
             "default value of disclose vat flag not shown in epp xml schemas for a update_contact operation")
            (get_key_str<CzNicSpecific::UpdateContact::Disclose::Ident>(),
             default_value_is_empty(),
             "default value of disclose ident flag not shown in epp xml schemas for a update_contact operation")
            (get_key_str<CzNicSpecific::UpdateContact::Disclose::NotifyEmail>(),
             default_value_is_empty(),
             "default value of disclose notifyemail flag not shown in epp xml schemas for a update_contact operation");
}

template <>
bool ConfigDataFilter::is_type_of<CzNicSpecific>()const
{
    return CzNicSpecific::is_name_of_this_contact_data_filter(name_);
}

template <typename T>
const std::string& ConfigDataFilter::get_value()const
{
    const auto key_value_itr = options_.find(get_key<T>());
    if (key_value_itr != options_.end())
    {
        return key_value_itr->second;
    }
    static const std::string no_value;
    return no_value;
}

template <>
ConfigDataFilter& ConfigDataFilter::set_all_values<CzNicSpecific>(const po_variables_map& vm)
{
    set_value<CzNicSpecific::CreateContact::Disclose::Name>(vm, options_);
    set_value<CzNicSpecific::CreateContact::Disclose::Organization>(vm, options_);
    set_value<CzNicSpecific::CreateContact::Disclose::Address>(vm, options_);
    set_value<CzNicSpecific::CreateContact::Disclose::Telephone>(vm, options_);
    set_value<CzNicSpecific::CreateContact::Disclose::Fax>(vm, options_);
    set_value<CzNicSpecific::CreateContact::Disclose::Email>(vm, options_);
    set_value<CzNicSpecific::CreateContact::Disclose::Vat>(vm, options_);
    set_value<CzNicSpecific::CreateContact::Disclose::Ident>(vm, options_);
    set_value<CzNicSpecific::CreateContact::Disclose::NotifyEmail>(vm, options_);

    set_value<CzNicSpecific::UpdateContact::Disclose::Name>(vm, options_);
    set_value<CzNicSpecific::UpdateContact::Disclose::Organization>(vm, options_);
    set_value<CzNicSpecific::UpdateContact::Disclose::Address>(vm, options_);
    set_value<CzNicSpecific::UpdateContact::Disclose::Telephone>(vm, options_);
    set_value<CzNicSpecific::UpdateContact::Disclose::Fax>(vm, options_);
    set_value<CzNicSpecific::UpdateContact::Disclose::Email>(vm, options_);
    set_value<CzNicSpecific::UpdateContact::Disclose::Vat>(vm, options_);
    set_value<CzNicSpecific::UpdateContact::Disclose::Ident>(vm, options_);
    set_value<CzNicSpecific::UpdateContact::Disclose::NotifyEmail>(vm, options_);
    return *this;
}

template <>
ConfigDataFilter ConfigDataFilter::get_default<CzNicSpecific>()
{
    ConfigDataFilter filter;
    filter.set_name(CzNicSpecific::get_contact_data_filter_name());
    filter.options_.emplace(get_key<CzNicSpecific::CreateContact::Disclose::Name>(), "show");
    filter.options_.emplace(get_key<CzNicSpecific::CreateContact::Disclose::Organization>(), "show");
    filter.options_.emplace(get_key<CzNicSpecific::CreateContact::Disclose::Address>(), "show");
    filter.options_.emplace(get_key<CzNicSpecific::CreateContact::Disclose::Telephone>(), "hide");
    filter.options_.emplace(get_key<CzNicSpecific::CreateContact::Disclose::Fax>(), "hide");
    filter.options_.emplace(get_key<CzNicSpecific::CreateContact::Disclose::Email>(), "hide");
    filter.options_.emplace(get_key<CzNicSpecific::CreateContact::Disclose::Vat>(), "hide");
    filter.options_.emplace(get_key<CzNicSpecific::CreateContact::Disclose::Ident>(), "hide");
    filter.options_.emplace(get_key<CzNicSpecific::CreateContact::Disclose::NotifyEmail>(), "hide");

    filter.options_.emplace(get_key<CzNicSpecific::UpdateContact::Disclose::Name>(), std::string());
    filter.options_.emplace(get_key<CzNicSpecific::UpdateContact::Disclose::Organization>(), std::string());
    filter.options_.emplace(get_key<CzNicSpecific::UpdateContact::Disclose::Address>(), std::string());
    filter.options_.emplace(get_key<CzNicSpecific::UpdateContact::Disclose::Telephone>(), std::string());
    filter.options_.emplace(get_key<CzNicSpecific::UpdateContact::Disclose::Fax>(), std::string());
    filter.options_.emplace(get_key<CzNicSpecific::UpdateContact::Disclose::Email>(), std::string());
    filter.options_.emplace(get_key<CzNicSpecific::UpdateContact::Disclose::Vat>(), std::string());
    filter.options_.emplace(get_key<CzNicSpecific::UpdateContact::Disclose::Ident>(), std::string());
    filter.options_.emplace(get_key<CzNicSpecific::UpdateContact::Disclose::NotifyEmail>(), std::string());
    return filter;
}

template const std::string&
ConfigDataFilter::get_value<CzNicSpecific::CreateContact::Disclose::Name>()const;

template const std::string&
ConfigDataFilter::get_value<CzNicSpecific::CreateContact::Disclose::Organization>()const;

template const std::string&
ConfigDataFilter::get_value<CzNicSpecific::CreateContact::Disclose::Address>()const;

template const std::string&
ConfigDataFilter::get_value<CzNicSpecific::CreateContact::Disclose::Telephone>()const;

template const std::string&
ConfigDataFilter::get_value<CzNicSpecific::CreateContact::Disclose::Fax>()const;

template const std::string&
ConfigDataFilter::get_value<CzNicSpecific::CreateContact::Disclose::Email>()const;

template const std::string&
ConfigDataFilter::get_value<CzNicSpecific::CreateContact::Disclose::Vat>()const;

template const std::string&
ConfigDataFilter::get_value<CzNicSpecific::CreateContact::Disclose::Ident>()const;

template const std::string&
ConfigDataFilter::get_value<CzNicSpecific::CreateContact::Disclose::NotifyEmail>()const;

template const std::string&
ConfigDataFilter::get_value<CzNicSpecific::UpdateContact::Disclose::Name>()const;

template const std::string&
ConfigDataFilter::get_value<CzNicSpecific::UpdateContact::Disclose::Organization>()const;

template const std::string&
ConfigDataFilter::get_value<CzNicSpecific::UpdateContact::Disclose::Address>()const;

template const std::string&
ConfigDataFilter::get_value<CzNicSpecific::UpdateContact::Disclose::Telephone>()const;

template const std::string&
ConfigDataFilter::get_value<CzNicSpecific::UpdateContact::Disclose::Fax>()const;

template const std::string&
ConfigDataFilter::get_value<CzNicSpecific::UpdateContact::Disclose::Email>()const;

template const std::string&
ConfigDataFilter::get_value<CzNicSpecific::UpdateContact::Disclose::Vat>()const;

template const std::string&
ConfigDataFilter::get_value<CzNicSpecific::UpdateContact::Disclose::Ident>()const;

template const std::string&
ConfigDataFilter::get_value<CzNicSpecific::UpdateContact::Disclose::NotifyEmail>()const;

}//namespace Epp::Contact
}//namespace Epp
