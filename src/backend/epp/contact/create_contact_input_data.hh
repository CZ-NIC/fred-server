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

#ifndef CREATE_CONTACT_INPUT_DATA_HH_616155610E4842C6BAC450787A8269EB//date "+%s.%N"|md5sum|tr "[a-f]" "[A-F]"
#define CREATE_CONTACT_INPUT_DATA_HH_616155610E4842C6BAC450787A8269EB

#include "src/backend/epp/contact/contact_data.hh"
#include "src/backend/epp/contact/street_traits.hh"
#include "src/backend/epp/contact/hideable.hh"

#include <boost/optional.hpp>

#include <string>
#include <vector>

namespace Epp {
namespace Contact {

struct CreateContactInputData
{
    CreateContactInputData() = delete;
    explicit CreateContactInputData(const ContactData& _src);
    struct Address
    {
        StreetTraits::Rows<std::string> street;
        std::string city;
        std::string state_or_province;
        std::string postal_code;
        std::string country_code;
    };
    Hideable<std::string> name;
    Hideable<std::string> organization;
    Hideable<Address> address;
    boost::optional<Address> mailing_address;
    Hideable<std::string> telephone;
    Hideable<std::string> fax;
    Hideable<std::string> email;
    Hideable<std::string> notify_email;
    Hideable<std::string> vat;
    HideableOptional<ContactIdent> ident;
};

}//namespace Epp::Contact
}//namespace Epp

#endif//CREATE_CONTACT_INPUT_DATA_HH_616155610E4842C6BAC450787A8269EB
