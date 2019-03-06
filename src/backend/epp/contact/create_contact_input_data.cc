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
#include "src/backend/epp/contact/create_contact_input_data.hh"

#include "src/backend/epp/contact/contact_data.hh"
#include "src/backend/epp/contact/util.hh"

#include <boost/optional.hpp>

#include <stdexcept>
#include <string>

namespace Epp {
namespace Contact {

namespace {

std::string remove_optionality(const boost::optional<std::string>& src)
{
    return !static_cast<bool>(src) ? std::string() : *src;
}

Hideable<std::string> remove_optionality(const HideableOptional<std::string>& src)
{
    return src.make_with_the_same_privacy(remove_optionality(*src));
}

boost::optional<CreateContactInputData::Address>
convert_to_address(const boost::optional<ContactData::Address>&);

Hideable<CreateContactInputData::Address>
remove_optionality(const HideableOptional<ContactData::Address>& src)
{
    const auto dst = convert_to_address(*src);
    if (static_cast<bool>(dst))
    {
        return src.make_with_the_same_privacy(*dst);
    }
    return src.make_with_the_same_privacy(CreateContactInputData::Address());
}

boost::optional<CreateContactInputData::Address>
convert_to_address(const boost::optional<ContactData::Address>& src)
{
    if (!static_cast<bool>(src))
    {
        return boost::none;
    }
    CreateContactInputData::Address dst;
    static_assert(std::tuple_size<decltype(src->street)>::value == std::tuple_size<decltype(dst.street)>::value,
                  "both streets must have the same size");
    for (std::size_t idx = 0; idx < src->street.size(); ++idx)
    {
        dst.street[idx] = remove_optionality(src->street[idx]);
    }
    dst.city = remove_optionality(src->city);
    dst.state_or_province = remove_optionality(src->state_or_province);
    dst.postal_code = remove_optionality(src->postal_code);
    dst.country_code = remove_optionality(src->country_code);
    return dst;
}

}//namespace Epp::Contact::{anonymous}

CreateContactInputData::CreateContactInputData(const ContactData& src)
    : name(remove_optionality(trim(src.name))),
      organization(remove_optionality(trim(src.organization))),
      address(remove_optionality(trim(src.address))),
      mailing_address(convert_to_address(trim(src.mailing_address))),
      telephone(remove_optionality(trim(src.telephone))),
      fax(remove_optionality(trim(src.fax))),
      email(remove_optionality(trim(src.email))),
      notify_email(remove_optionality(trim(src.notify_email))),
      vat(remove_optionality(trim(src.vat))),
      ident(src.ident),
      authinfopw(src.authinfopw)
{ }

}//namespace Epp::Contact
}//namespace Epp
