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

#include "src/backend/epp/contact/impl/set_unused/config.hh"

#include "src/backend/epp/contact/config_check.hh"

#include <stdexcept>

namespace Epp {
namespace Contact {
namespace Impl {
namespace SetUnused {

const std::string& Config::get_check_name()
{
    static const std::string singleton = "set_unused_discloseflags";
    return singleton;
}

bool Config::is_name_of_this_check(const std::string& check_name)
{
    return check_name == get_check_name();
}

}//namespace Epp::Contact::Impl::SetUnused
}//namespace Epp::Contact::Impl


using SetUnusedConfig = Impl::SetUnused::Config;
using po_variables_map = boost::program_options::variables_map;

namespace {

template <typename>
const char* get_key_str();

template <>
const char* get_key_str<SetUnusedConfig::CreateContact::Disclose::Name>()
{
    return "rifd::set_unused_discloseflags::create_contact.default_disclosename";
}

template <>
const char* get_key_str<SetUnusedConfig::CreateContact::Disclose::Organization>()
{
    return "rifd::set_unused_discloseflags::create_contact.default_discloseorganization";
}

template <>
const char* get_key_str<SetUnusedConfig::CreateContact::Disclose::Address>()
{
    return "rifd::set_unused_discloseflags::create_contact.default_discloseaddress";
}

template <>
const char* get_key_str<SetUnusedConfig::CreateContact::Disclose::Telephone>()
{
    return "rifd::set_unused_discloseflags::create_contact.default_disclosetelephone";
}

template <>
const char* get_key_str<SetUnusedConfig::CreateContact::Disclose::Fax>()
{
    return "rifd::set_unused_discloseflags::create_contact.default_disclosefax";
}

template <>
const char* get_key_str<SetUnusedConfig::CreateContact::Disclose::Email>()
{
    return "rifd::set_unused_discloseflags::create_contact.default_discloseemail";
}

template <>
const char* get_key_str<SetUnusedConfig::CreateContact::Disclose::Vat>()
{
    return "rifd::set_unused_discloseflags::create_contact.default_disclosevat";
}

template <>
const char* get_key_str<SetUnusedConfig::CreateContact::Disclose::Ident>()
{
    return "rifd::set_unused_discloseflags::create_contact.default_discloseident";
}

template <>
const char* get_key_str<SetUnusedConfig::CreateContact::Disclose::NotifyEmail>()
{
    return "rifd::set_unused_discloseflags::create_contact.default_disclosenotifyemail";
}

template <>
const char* get_key_str<SetUnusedConfig::UpdateContact::Disclose::Name>()
{
    return "rifd::set_unused_discloseflags::update_contact.default_disclosename";
}

template <>
const char* get_key_str<SetUnusedConfig::UpdateContact::Disclose::Organization>()
{
    return "rifd::set_unused_discloseflags::update_contact.default_discloseorganization";
}

template <>
const char* get_key_str<SetUnusedConfig::UpdateContact::Disclose::Address>()
{
    return "rifd::set_unused_discloseflags::update_contact.default_discloseaddress";
}

template <>
const char* get_key_str<SetUnusedConfig::UpdateContact::Disclose::Telephone>()
{
    return "rifd::set_unused_discloseflags::update_contact.default_disclosetelephone";
}

template <>
const char* get_key_str<SetUnusedConfig::UpdateContact::Disclose::Fax>()
{
    return "rifd::set_unused_discloseflags::update_contact.default_disclosefax";
}

template <>
const char* get_key_str<SetUnusedConfig::UpdateContact::Disclose::Email>()
{
    return "rifd::set_unused_discloseflags::update_contact.default_discloseemail";
}

template <>
const char* get_key_str<SetUnusedConfig::UpdateContact::Disclose::Vat>()
{
    return "rifd::set_unused_discloseflags::update_contact.default_disclosevat";
}

template <>
const char* get_key_str<SetUnusedConfig::UpdateContact::Disclose::Ident>()
{
    return "rifd::set_unused_discloseflags::update_contact.default_discloseident";
}

template <>
const char* get_key_str<SetUnusedConfig::UpdateContact::Disclose::NotifyEmail>()
{
    return "rifd::set_unused_discloseflags::update_contact.default_disclosenotifyemail";
}

template <typename T>
const std::string& get_key()
{
    static const std::string singleton = get_key_str<T>();
    return singleton;
}

template <typename T>
void set_value(const po_variables_map& vm, ConfigCheck::KeyValue& options)
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
void ConfigCheck::add_options_description<SetUnusedConfig>(
        boost::program_options::options_description& options_description)
{
    options_description.add_options()
            (get_key_str<SetUnusedConfig::CreateContact::Disclose::Name>(),
             default_value_is_to_show(),
             "default value of disclose name flag not shown in epp xml schemas for a create_contact operation")
            (get_key_str<SetUnusedConfig::CreateContact::Disclose::Organization>(),
             default_value_is_to_show(),
             "default value of disclose organization flag not shown in epp xml schemas for a create_contact operation")
            (get_key_str<SetUnusedConfig::CreateContact::Disclose::Address>(),
             default_value_is_to_show(),
             "default value of disclose address flag not shown in epp xml schemas for a create_contact operation")
            (get_key_str<SetUnusedConfig::CreateContact::Disclose::Telephone>(),
             default_value_is_to_hide(),
             "default value of disclose telephone flag not shown in epp xml schemas for a create_contact operation")
            (get_key_str<SetUnusedConfig::CreateContact::Disclose::Fax>(),
             default_value_is_to_hide(),
             "default value of disclose fax flag not shown in epp xml schemas for a create_contact operation")
            (get_key_str<SetUnusedConfig::CreateContact::Disclose::Email>(),
             default_value_is_to_hide(),
             "default value of disclose email flag not shown in epp xml schemas for a create_contact operation")
            (get_key_str<SetUnusedConfig::CreateContact::Disclose::Vat>(),
             default_value_is_to_hide(),
             "default value of disclose vat flag not shown in epp xml schemas for a create_contact operation")
            (get_key_str<SetUnusedConfig::CreateContact::Disclose::Ident>(),
             default_value_is_to_hide(),
             "default value of disclose ident flag not shown in epp xml schemas for a create_contact operation")
            (get_key_str<SetUnusedConfig::CreateContact::Disclose::NotifyEmail>(),
             default_value_is_to_hide(),
             "default value of disclose notifyemail flag not shown in epp xml schemas for a create_contact operation")
            (get_key_str<SetUnusedConfig::UpdateContact::Disclose::Name>(),
             default_value_is_empty(),
             "default value of disclose name flag not shown in epp xml schemas for a update_contact operation")
            (get_key_str<SetUnusedConfig::UpdateContact::Disclose::Organization>(),
             default_value_is_empty(),
             "default value of disclose organization flag not shown in epp xml schemas for a update_contact operation")
            (get_key_str<SetUnusedConfig::UpdateContact::Disclose::Address>(),
             default_value_is_empty(),
             "default value of disclose address flag not shown in epp xml schemas for a update_contact operation")
            (get_key_str<SetUnusedConfig::UpdateContact::Disclose::Telephone>(),
             default_value_is_empty(),
             "default value of disclose telephone flag not shown in epp xml schemas for a update_contact operation")
            (get_key_str<SetUnusedConfig::UpdateContact::Disclose::Fax>(),
             default_value_is_empty(),
             "default value of disclose fax flag not shown in epp xml schemas for a update_contact operation")
            (get_key_str<SetUnusedConfig::UpdateContact::Disclose::Email>(),
             default_value_is_empty(),
             "default value of disclose email flag not shown in epp xml schemas for a update_contact operation")
            (get_key_str<SetUnusedConfig::UpdateContact::Disclose::Vat>(),
             default_value_is_empty(),
             "default value of disclose vat flag not shown in epp xml schemas for a update_contact operation")
            (get_key_str<SetUnusedConfig::UpdateContact::Disclose::Ident>(),
             default_value_is_empty(),
             "default value of disclose ident flag not shown in epp xml schemas for a update_contact operation")
            (get_key_str<SetUnusedConfig::UpdateContact::Disclose::NotifyEmail>(),
             default_value_is_empty(),
             "default value of disclose notifyemail flag not shown in epp xml schemas for a update_contact operation");
}

template <>
bool ConfigCheck::is_type_of<SetUnusedConfig>()const
{
    return SetUnusedConfig::is_name_of_this_check(name_);
}

template <typename T>
const std::string& ConfigCheck::get_value()const
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
ConfigCheck& ConfigCheck::set_all_values<SetUnusedConfig>(const po_variables_map& vm)
{
    set_value<SetUnusedConfig::CreateContact::Disclose::Name>(vm, options_);
    set_value<SetUnusedConfig::CreateContact::Disclose::Organization>(vm, options_);
    set_value<SetUnusedConfig::CreateContact::Disclose::Address>(vm, options_);
    set_value<SetUnusedConfig::CreateContact::Disclose::Telephone>(vm, options_);
    set_value<SetUnusedConfig::CreateContact::Disclose::Fax>(vm, options_);
    set_value<SetUnusedConfig::CreateContact::Disclose::Email>(vm, options_);
    set_value<SetUnusedConfig::CreateContact::Disclose::Vat>(vm, options_);
    set_value<SetUnusedConfig::CreateContact::Disclose::Ident>(vm, options_);
    set_value<SetUnusedConfig::CreateContact::Disclose::NotifyEmail>(vm, options_);

    set_value<SetUnusedConfig::UpdateContact::Disclose::Name>(vm, options_);
    set_value<SetUnusedConfig::UpdateContact::Disclose::Organization>(vm, options_);
    set_value<SetUnusedConfig::UpdateContact::Disclose::Address>(vm, options_);
    set_value<SetUnusedConfig::UpdateContact::Disclose::Telephone>(vm, options_);
    set_value<SetUnusedConfig::UpdateContact::Disclose::Fax>(vm, options_);
    set_value<SetUnusedConfig::UpdateContact::Disclose::Email>(vm, options_);
    set_value<SetUnusedConfig::UpdateContact::Disclose::Vat>(vm, options_);
    set_value<SetUnusedConfig::UpdateContact::Disclose::Ident>(vm, options_);
    set_value<SetUnusedConfig::UpdateContact::Disclose::NotifyEmail>(vm, options_);
    return *this;
}

template <>
ConfigCheck ConfigCheck::get_default<SetUnusedConfig>()
{
    ConfigCheck check;
    check.set_name(SetUnusedConfig::get_check_name());
    check.options_.emplace(get_key<SetUnusedConfig::CreateContact::Disclose::Name>(), "show");
    check.options_.emplace(get_key<SetUnusedConfig::CreateContact::Disclose::Organization>(), "show");
    check.options_.emplace(get_key<SetUnusedConfig::CreateContact::Disclose::Address>(), "show");
    check.options_.emplace(get_key<SetUnusedConfig::CreateContact::Disclose::Telephone>(), "hide");
    check.options_.emplace(get_key<SetUnusedConfig::CreateContact::Disclose::Fax>(), "hide");
    check.options_.emplace(get_key<SetUnusedConfig::CreateContact::Disclose::Email>(), "hide");
    check.options_.emplace(get_key<SetUnusedConfig::CreateContact::Disclose::Vat>(), "hide");
    check.options_.emplace(get_key<SetUnusedConfig::CreateContact::Disclose::Ident>(), "hide");
    check.options_.emplace(get_key<SetUnusedConfig::CreateContact::Disclose::NotifyEmail>(), "hide");

    check.options_.emplace(get_key<SetUnusedConfig::UpdateContact::Disclose::Name>(), std::string());
    check.options_.emplace(get_key<SetUnusedConfig::UpdateContact::Disclose::Organization>(), std::string());
    check.options_.emplace(get_key<SetUnusedConfig::UpdateContact::Disclose::Address>(), std::string());
    check.options_.emplace(get_key<SetUnusedConfig::UpdateContact::Disclose::Telephone>(), std::string());
    check.options_.emplace(get_key<SetUnusedConfig::UpdateContact::Disclose::Fax>(), std::string());
    check.options_.emplace(get_key<SetUnusedConfig::UpdateContact::Disclose::Email>(), std::string());
    check.options_.emplace(get_key<SetUnusedConfig::UpdateContact::Disclose::Vat>(), std::string());
    check.options_.emplace(get_key<SetUnusedConfig::UpdateContact::Disclose::Ident>(), std::string());
    check.options_.emplace(get_key<SetUnusedConfig::UpdateContact::Disclose::NotifyEmail>(), std::string());
    return check;
}

template const std::string&
ConfigCheck::get_value<SetUnusedConfig::CreateContact::Disclose::Name>()const;

template const std::string&
ConfigCheck::get_value<SetUnusedConfig::CreateContact::Disclose::Organization>()const;

template const std::string&
ConfigCheck::get_value<SetUnusedConfig::CreateContact::Disclose::Address>()const;

template const std::string&
ConfigCheck::get_value<SetUnusedConfig::CreateContact::Disclose::Telephone>()const;

template const std::string&
ConfigCheck::get_value<SetUnusedConfig::CreateContact::Disclose::Fax>()const;

template const std::string&
ConfigCheck::get_value<SetUnusedConfig::CreateContact::Disclose::Email>()const;

template const std::string&
ConfigCheck::get_value<SetUnusedConfig::CreateContact::Disclose::Vat>()const;

template const std::string&
ConfigCheck::get_value<SetUnusedConfig::CreateContact::Disclose::Ident>()const;

template const std::string&
ConfigCheck::get_value<SetUnusedConfig::CreateContact::Disclose::NotifyEmail>()const;

template const std::string&
ConfigCheck::get_value<SetUnusedConfig::UpdateContact::Disclose::Name>()const;

template const std::string&
ConfigCheck::get_value<SetUnusedConfig::UpdateContact::Disclose::Organization>()const;

template const std::string&
ConfigCheck::get_value<SetUnusedConfig::UpdateContact::Disclose::Address>()const;

template const std::string&
ConfigCheck::get_value<SetUnusedConfig::UpdateContact::Disclose::Telephone>()const;

template const std::string&
ConfigCheck::get_value<SetUnusedConfig::UpdateContact::Disclose::Fax>()const;

template const std::string&
ConfigCheck::get_value<SetUnusedConfig::UpdateContact::Disclose::Email>()const;

template const std::string&
ConfigCheck::get_value<SetUnusedConfig::UpdateContact::Disclose::Vat>()const;

template const std::string&
ConfigCheck::get_value<SetUnusedConfig::UpdateContact::Disclose::Ident>()const;

template const std::string&
ConfigCheck::get_value<SetUnusedConfig::UpdateContact::Disclose::NotifyEmail>()const;

}//namespace Epp::Contact
}//namespace Epp
