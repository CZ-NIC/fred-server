/*
 * Copyright (C) 2017-2019  CZ.NIC, z. s. p. o.
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
#include "src/backend/epp/contact/util.hh"

#include "src/backend/epp/contact/contact_ident.hh"
#include "src/backend/epp/contact/contact_data.hh"
#include "src/backend/epp/contact/contact_change.hh"
#include "src/backend/epp/contact/street_traits.hh"

#include "src/backend/epp/update_operation.hh"

#include "util/db/nullable.hh"

#include <boost/algorithm/string/trim.hpp>
#include <boost/optional.hpp>
#include <boost/variant.hpp>

#include <string>

namespace Epp {
namespace Contact {

template <>
std::string trim(const std::string& src)
{
    return boost::trim_copy(src);
}

template <>
boost::optional<std::string> trim(const boost::optional<std::string>& src)
{
    return src != boost::none ? boost::optional<std::string>(trim(*src))
                              : boost::optional<std::string>();
}

template <>
boost::optional<Nullable<std::string>> trim(const boost::optional<Nullable<std::string>>& src)
{
    return (src != boost::none) && !src->isnull() ? boost::optional<Nullable<std::string>>(trim(src->get_value()))
                                                  : src;
}

template <>
std::array<boost::optional<std::string>, StreetTraits::number_of_rows> trim(
        const std::array<boost::optional<std::string>, StreetTraits::number_of_rows>& src)
{
    std::array<boost::optional<std::string>, StreetTraits::number_of_rows> dst;
    for (std::size_t idx = 0; idx < src.size(); ++idx)
    {
        dst[idx] = trim(src[idx]);
    }
    return dst;
}

template <>
boost::optional<ContactData::Address> trim(const boost::optional<ContactData::Address>& src)
{
    if (!static_cast<bool>(src))
    {
        return src;
    }
    ContactData::Address dst;
    dst.street = trim(src->street);
    dst.city = trim(src->city);
    dst.state_or_province = trim(src->state_or_province);
    dst.postal_code = trim(src->postal_code);
    dst.country_code = trim(src->country_code);
    return dst;
}

template <>
Deletable<std::string> trim(const Deletable<std::string>& src)
{
    if (src == UpdateOperation::Action::set_value)
    {
        return Deletable<std::string>(UpdateOperation::set_value(trim(*src)));
    }
    return src;
}

template <>
Updateable<std::string> trim(const Updateable<std::string>& src)
{
    if (src == UpdateOperation::Action::set_value)
    {
        return Updateable<std::string>(UpdateOperation::set_value(trim(*src)));
    }
    return src;
}

template <typename T>
StreetTraits::Rows<T> trim(const StreetTraits::Rows<T>& src)
{
    StreetTraits::Rows<T> dst;
    for (unsigned idx = 0; idx < src.size(); ++idx)
    {
        dst[idx] = trim(src[idx]);
    }
    return dst;
}

template <>
ContactChange::MainAddress trim(const ContactChange::MainAddress& src)
{
    ContactChange::MainAddress dst;
    dst.street = trim(src.street);
    dst.city = trim(src.city);
    dst.state_or_province = trim(src.state_or_province);
    dst.postal_code = trim(src.postal_code);
    dst.country_code = trim(src.country_code);
    return dst;
}

template <>
ContactChange::Address trim(const ContactChange::Address& src)
{
    ContactChange::Address dst;
    dst.street = trim(src.street);
    dst.city = trim(src.city);
    dst.state_or_province = trim(src.state_or_province);
    dst.postal_code = trim(src.postal_code);
    dst.country_code = trim(src.country_code);
    return dst;
}

template <>
Deletable<ContactChange::Address> trim(const Deletable<ContactChange::Address>& src)
{
    if (src == UpdateOperation::Action::set_value)
    {
        return Deletable<ContactChange::Address>(UpdateOperation::set_value(trim(*src)));
    }
    return src;
}

namespace {

struct TrimContactIdent : boost::static_visitor<ContactIdent>
{
    template <typename T>
    ContactIdent operator()(const ContactIdentValueOf<T>& src)const
    {
        return ContactIdentValueOf<T>(boost::trim_copy(src.value));
    }
};

}//namespace Epp::Contact::{anonymous}

template <>
ContactIdent trim(const ContactIdent& src)
{
    return boost::apply_visitor(TrimContactIdent(), src);
}

template <>
boost::optional<ContactIdent> trim(const boost::optional<ContactIdent>& src)
{
    return static_cast<bool>(src) ? boost::optional<ContactIdent>(trim(*src))
                                  : src;
}

template <>
boost::optional<boost::optional<ContactIdent>> trim(const boost::optional<boost::optional<ContactIdent>>& src)
{
    if (!static_cast<bool>(src))
    {
        return src;
    }
    return trim(*src);
}

template <>
Deletable<ContactIdent> trim(const Deletable<ContactIdent>& src)
{
    if (src == UpdateOperation::operation_not_specified)
    {
        return src;
    }
    if (src == UpdateOperation::Action::set_value)
    {
        return Deletable<ContactIdent>(Epp::UpdateOperation::set_value(trim(*src)));
    }
    return src;
}

}//namespace Epp::Contact
}//namespace Epp
