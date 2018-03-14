/*
 *  Copyright (C) 2017  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "src/backend/epp/contact/util.hh"
#include "src/backend/epp/contact/contact_ident.hh"
#include "src/backend/epp/contact/contact_data.hh"
#include "src/backend/epp/contact/contact_change.hh"

#include "src/util/db/nullable.hh"

#include <string>
#include <vector>

#include <boost/algorithm/string/trim.hpp>
#include <boost/optional.hpp>
#include <boost/variant.hpp>

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
    return src ? boost::optional<std::string>(trim(*src))
               : boost::optional<std::string>();
}

template <>
boost::optional< Nullable<std::string> > trim(const boost::optional< Nullable<std::string> >& src)
{
    return src && !src->isnull() ? boost::optional< Nullable<std::string> >(trim(src->get_value()))
                                 : src;
}

template <>
boost::optional< Nullable<ContactChange::Address> > trim(const boost::optional< Nullable<ContactChange::Address> >& src)
{
    if (!src || src->isnull())
    {
        return src;
    }
    ContactChange::Address dst;
    dst.street1 = trim(src->get_value().street1);
    dst.street2 = trim(src->get_value().street2);
    dst.street3 = trim(src->get_value().street3);
    dst.city = trim(src->get_value().city);
    dst.state_or_province = trim(src->get_value().state_or_province);
    dst.postal_code = trim(src->get_value().postal_code);
    dst.country_code = trim(src->get_value().country_code);
    return boost::optional< Nullable<ContactChange::Address> >(dst);
}

template <>
boost::optional<ContactData::Address> trim(const boost::optional<ContactData::Address>& src)
{
    if (!static_cast<bool>(src))
    {
        return src;
    }
    ContactData::Address dst;
    dst.street1 = trim(src->street1);
    dst.street2 = trim(src->street2);
    dst.street3 = trim(src->street3);
    dst.city = trim(src->city);
    dst.state_or_province = trim(src->state_or_province);
    dst.postal_code = trim(src->postal_code);
    dst.country_code = trim(src->country_code);
    return dst;
}

namespace {

struct TrimContactIdent:boost::static_visitor<ContactIdent>
{
    template <typename T>
    ContactIdent operator()(const ContactIdentValueOf<T>& src)const
    {
        return ContactIdentValueOf<T>(boost::trim_copy(src.value));
    }
};

} // namespace Epp::Contact::{anonymous}

template <>
boost::optional<ContactIdent> trim(const boost::optional<ContactIdent>& src)
{
    return static_cast<bool>(src) ? boost::optional<ContactIdent>(boost::apply_visitor(TrimContactIdent(), *src))
                                  : src;
}

template <>
boost::optional< boost::optional<ContactIdent> > trim(const boost::optional< boost::optional<ContactIdent> >& src)
{
    if (!static_cast<bool>(src))
    {
        return src;
    }
    return trim(*src);
}

template <>
std::vector<std::string> trim(const std::vector<std::string>& src)
{
    std::vector<std::string> result;
    result.reserve(src.size());
    for (typename std::vector<std::string>::const_iterator data_ptr = src.begin(); data_ptr != src.end(); ++data_ptr)
    {
        result.push_back(trim(*data_ptr));
    }
    return result;
}

template <>
boost::optional<std::vector<std::string>> trim(const boost::optional<std::vector<std::string>>& src)
{
    if (src == boost::none)
    {
        return src;
    }
    return trim(*src);
}

} // namespace Epp::Contact
} // namespace Epp
