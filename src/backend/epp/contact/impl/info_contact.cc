/*
 * Copyright (C) 2021  CZ.NIC, z. s. p. o.
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

#include "src/backend/epp/contact/impl/info_contact.hh"
#include "src/backend/epp/contact/impl/info_contact_data_filter.hh"

#include "src/backend/epp/contact/config_data_filter.hh"

#include <algorithm>
#include <stdexcept>
#include <vector>

namespace Epp {
namespace Contact {

using po_variables_map = boost::program_options::variables_map;

namespace {

template <typename>
const char* get_key_str();

template <>
const char* get_key_str<Impl::InfoContact::DataSharePolicy>()
{
    return "rifd::info_contact.data_share_policy";
}

template <>
const char* get_key_str<Impl::InfoContact::ShowPrivateDataTo>()
{
    return "rifd::info_contact.show_private_data_to";
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
            KeyIsOccupied() : std::runtime_error{"Key is occupied"} { }
        };
        throw KeyIsOccupied();
    }
    const auto vm_iter = vm.find(get_key<T>());
    const bool key_present = vm_iter != end(vm);
    if (key_present)
    {
        options.emplace(get_key<T>(), vm_iter->second.template as<std::string>());
    }
}

template <>
void set_value<Impl::InfoContact::ShowPrivateDataTo>(const po_variables_map& vm, ConfigDataFilter::KeyValue& options)
{
    static const auto key = []() { return get_key<Impl::InfoContact::ShowPrivateDataTo>(); };
    const auto key_value_itr = options.find(key());
    if (key_value_itr != options.end())
    {
        struct KeyIsOccupied : std::runtime_error
        {
            KeyIsOccupied() : std::runtime_error{"Key is occupied"} { }
        };
        throw KeyIsOccupied();
    }
    const auto vm_iter = vm.find(key());
    const bool key_present = vm_iter != end(vm);
    if (key_present)
    {
        const auto& values = vm[key()].template as<std::vector<std::string>>();
        std::for_each(begin(values), end(values), [&](const std::string& value) { options.emplace(key(), value); });
    }
}

}//namespace {anonymous}

namespace Impl {

void add_info_contact_options_description(boost::program_options::options_description& options_description)
{
    options_description.add_options()
            (get_key_str<Impl::InfoContact::DataSharePolicy>(),
             boost::program_options::value<std::string>()->default_value("show_all"),
             "contact data share policy; possible values are cznic_specific, show_all")
            (get_key_str<Impl::InfoContact::ShowPrivateDataTo>(),
             boost::program_options::value<std::vector<std::string>>()->multitoken(),
             "the relationship between contact and registrar which can see data without restrictions");
}

template <>
bool has_value<InfoContact::DataSharePolicy::cznic_specific>(const std::string& value)
{
    return value == "cznic_specific";
}

template <>
bool has_value<InfoContact::DataSharePolicy::show_all>(const std::string& value)
{
    return value == "show_all";
}

template <>
bool has_value<ContactRegistrarRelationship::AuthorizedRegistrar>(const std::string& value)
{
    return value == "authorized_registrar";
}

template <>
bool has_value<ContactRegistrarRelationship::SponsoringRegistrar>(const std::string& value)
{
    return value == "sponsoring_registrar";
}

template <>
bool has_value<ContactRegistrarRelationship::SponsoringRegistrarOfDomainWhereContactIs::AdminContact>(const std::string& value)
{
    return value == "admin_contact";
}

template <>
bool has_value<ContactRegistrarRelationship::SponsoringRegistrarOfDomainWhereContactIs::DomainHolder>(const std::string& value)
{
    return value == "domain_holder";
}

template <>
bool has_value<ContactRegistrarRelationship::OtherRelationship>(const std::string& value)
{
    return value == "other";
}

template <>
bool has_value<ContactRegistrarRelationship::SystemRegistrar>(const std::string& value)
{
    return value == "system_registrar";
}

}//namespace Epp::Contact::Impl

template <>
const std::string& ConfigDataFilter::get_value<Impl::InfoContact::DataSharePolicy>() const
{
    const auto key_value_itr = options_.find(get_key<Impl::InfoContact::DataSharePolicy>());
    if (key_value_itr != options_.end())
    {
        return key_value_itr->second;
    }
    static const std::string no_value;
    return no_value;
}

template <>
void ConfigDataFilter::iterate_multiple_value<Impl::InfoContact::ShowPrivateDataTo>(std::function<void(const std::string&)> on_value) const
{
    const auto first = options_.find(get_key<Impl::InfoContact::ShowPrivateDataTo>());
    if (first != options_.end())
    {
        auto count = options_.count(get_key<Impl::InfoContact::ShowPrivateDataTo>());
        for (auto iter = first; 0 < count; --count, ++iter)
        {
            on_value(iter->second);
        }
    }
}

template <>
ConfigDataFilter& ConfigDataFilter::set_all_values<Impl::InfoContact>(const po_variables_map& vm)
{
    const bool has_not_defaulted_data_share_policy = 0 < vm.count(get_key<Impl::InfoContact::DataSharePolicy>())
                                                        && !vm[get_key<Impl::InfoContact::DataSharePolicy>()].defaulted();
    const bool has_show_private_data_to = 0 < vm.count(get_key<Impl::InfoContact::ShowPrivateDataTo>());

    if (has_not_defaulted_data_share_policy && has_show_private_data_to)
    {
        struct MutuallyExclusiveOptions : std::runtime_error
        {
            MutuallyExclusiveOptions() : std::runtime_error{"Mutually exclusive options"} { }
        };
        throw MutuallyExclusiveOptions{};
    }
    else if (has_show_private_data_to)
    {
        set_value<Impl::InfoContact::ShowPrivateDataTo>(vm, options_);
    }
    else
    {
        set_value<Impl::InfoContact::DataSharePolicy>(vm, options_);
    }

    return *this;
}

template <>
ConfigDataFilter ConfigDataFilter::get_default<Impl::InfoContact>()
{
    ConfigDataFilter filter;
    filter.options_.emplace(get_key<Impl::InfoContact::DataSharePolicy>(), "show_all");
    return filter;
}

}//namespace Epp::Contact
}//namespace Epp
