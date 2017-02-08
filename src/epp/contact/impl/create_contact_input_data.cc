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

#include "src/epp/contact/impl/create_contact_input_data.h"

#include "src/epp/contact/contact_change.h"
#include "src/epp/contact/impl/util.h"
#include "util/db/nullable.h"
#include "util/optional_value.h"

#include <stdexcept>
#include <string>
#include <vector>

namespace Epp {
namespace Contact {

namespace {

std::string convert(const boost::optional<std::string>& src)
{
    return ContactChange::does_value_mean<ContactChange::Value::to_set>(src)
           ? ContactChange::get_value(src)
           : std::string();
}

std::string convert(const boost::optional<Nullable<std::string> >& src)
{
    return ContactChange::does_value_mean<ContactChange::Value::to_set>(src)
           ? ContactChange::get_value(src)
           : std::string();
}

std::vector<std::string> convert(const std::vector<boost::optional<Nullable<std::string> > >& src)
{
    std::vector<std::string> result;
    result.reserve(src.size());
    typedef std::vector<boost::optional<Nullable<std::string> > > VectorOfChangeData;
    for (VectorOfChangeData::const_iterator data_ptr = src.begin(); data_ptr != src.end(); ++data_ptr) {
        result.push_back(convert(*data_ptr));
    }
    return result;
}

} // namespace Epp::Contact::{anonymous}

CreateContactInputData::CreateContactInputData(const ContactChange& src)
    : name(convert(trim(src.name))),
      organization(convert(trim(src.organization))),
      streets(convert(trim(src.streets))),
      city(convert(trim(src.city))),
      state_or_province(convert(trim(src.state_or_province))),
      postal_code(convert(trim(src.postal_code))),
      country_code(convert(trim(src.country_code))),
      telephone(convert(trim(src.telephone))),
      fax(convert(trim(src.fax))),
      email(convert(trim(src.email))),
      notify_email(convert(trim(src.notify_email))),
      VAT(convert(trim(src.vat))),
      ident(convert(trim(src.ident))),
      identtype(src.ident_type),
      authinfopw((src.authinfopw && (*src.authinfopw).isnull())
                          ? boost::optional<std::string>()
                          : boost::optional<std::string>(convert(src.authinfopw))),
      disclose(src.disclose)
{
    if (disclose.is_initialized()) {
        disclose->check_validity();
    }
}

} // namespace Epp::Contact
} // namespace Epp
