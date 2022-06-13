/*
 * Copyright (C) 2017-2022  CZ.NIC, z. s. p. o.
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

#ifndef CONTACT_DATA_HH_2ADE4FD677064F6994115E79385BB28D//date "+%s.%N"|md5sum|tr "[a-f]" "[A-F]"
#define CONTACT_DATA_HH_2ADE4FD677064F6994115E79385BB28D

#include "src/backend/epp/contact/hideable.hh"
#include "src/backend/epp/contact/street_traits.hh"
#include "src/backend/epp/contact/contact_ident.hh"

#include <boost/optional.hpp>

#include <string>

namespace Epp {
namespace Contact {

struct ContactData
{
    ContactData();
    struct Address
    {
        StreetTraits::Rows<boost::optional<std::string>> street;
        boost::optional<std::string> city;
        boost::optional<std::string> state_or_province;
        boost::optional<std::string> postal_code;
        boost::optional<std::string> country_code;
    };
    HideableOptional<std::string> name;
    HideableOptional<std::string> organization;
    HideableOptional<Address> address;
    boost::optional<Address> mailing_address;
    HideableOptional<std::string> telephone;
    HideableOptional<std::string> fax;
    HideableOptional<std::string> email;
    HideableOptional<std::string> notify_email;
    HideableOptional<std::string> vat;
    HideableOptional<ContactIdent> ident;
    ContactData get_trimmed_copy()const;
};

}//namespace Epp::Contact
}//namespace Epp

#endif//CONTACT_DATA_HH_2ADE4FD677064F6994115E79385BB28D
