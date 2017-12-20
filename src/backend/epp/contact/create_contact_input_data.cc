/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
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

#include "src/backend/epp/contact/create_contact_input_data.hh"

#include "src/backend/epp/contact/contact_data.hh"
#include "src/backend/epp/contact/util.hh"
#include "src/util/db/nullable.hh"
#include "src/util/optional_value.hh"

#include <boost/optional.hpp>

#include <stdexcept>
#include <string>
#include <vector>

namespace Epp {
namespace Contact {

namespace {

std::string convert(const boost::optional<std::string>& src)
{
    return !static_cast<bool>(src) ? std::string() : *src;
}

boost::optional<CreateContactInputData::Address>
convert(const boost::optional<ContactData::Address>& src)
{
    if (!static_cast<bool>(src))
    {
        return boost::none;
    }
    CreateContactInputData::Address dst;
    dst.street1 = convert(src->street1);
    dst.street2 = convert(src->street2);
    dst.street3 = convert(src->street3);
    dst.city = convert(src->city);
    dst.state_or_province = convert(src->state_or_province);
    dst.postal_code = convert(src->postal_code);
    dst.country_code = convert(src->country_code);
    return dst;
}

} // namespace Epp::Contact::{anonymous}

CreateContactInputData::CreateContactInputData(const ContactData& src)
    : name(convert(trim(src.name))),
      organization(convert(trim(src.organization))),
      streets(trim(src.streets)),
      city(convert(trim(src.city))),
      state_or_province(convert(trim(src.state_or_province))),
      postal_code(convert(trim(src.postal_code))),
      country_code(convert(trim(src.country_code))),
      mailing_address(convert(trim(src.mailing_address))),
      telephone(convert(trim(src.telephone))),
      fax(convert(trim(src.fax))),
      email(convert(trim(src.email))),
      notify_email(convert(trim(src.notify_email))),
      vat(convert(trim(src.vat))),
      ident(trim(src.ident)),
      authinfopw(src.authinfopw),
      disclose(src.disclose)
{
    if (static_cast<bool>(disclose))
    {
        disclose->check_validity();
    }
}

} // namespace Epp::Contact
} // namespace Epp
