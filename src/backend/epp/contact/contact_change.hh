/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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

#ifndef CONTACT_CHANGE_HH_2C8C709868134389AD38F302E513C812
#define CONTACT_CHANGE_HH_2C8C709868134389AD38F302E513C812

#include "src/backend/epp/update_operation.hh"

#include "src/backend/epp/contact/contact_ident.hh"
#include "src/backend/epp/contact/hideable.hh"
#include "src/backend/epp/contact/street_traits.hh"

#include <boost/optional.hpp>

#include <string>

namespace Epp {
namespace Contact {

struct ContactChange
{
    struct MainAddress
    {
        StreetTraits::Rows<Deletable<std::string>> street;
        Deletable<std::string> city;
        Deletable<std::string> state_or_province;
        Deletable<std::string> postal_code;
        Updateable<std::string> country_code;
    };
    struct Address
    {
        StreetTraits::Rows<boost::optional<std::string>> street;
        boost::optional<std::string> city;
        boost::optional<std::string> state_or_province;
        boost::optional<std::string> postal_code;
        boost::optional<std::string> country_code;
    };
    struct Publishability
    {
        boost::optional<PrivacyPolicy> name;
        boost::optional<PrivacyPolicy> organization;
        boost::optional<PrivacyPolicy> address;
        boost::optional<PrivacyPolicy> telephone;
        boost::optional<PrivacyPolicy> fax;
        boost::optional<PrivacyPolicy> email;
        boost::optional<PrivacyPolicy> notify_email;
        boost::optional<PrivacyPolicy> vat;
        boost::optional<PrivacyPolicy> ident;
    };
    Deletable<std::string> name;
    Deletable<std::string> organization;
    MainAddress address;
    Deletable<Address> mailing_address;
    Deletable<std::string> telephone;
    Deletable<std::string> fax;
    Deletable<std::string> email;
    Deletable<std::string> notify_email;
    Deletable<std::string> vat;
    Deletable<ContactIdent> ident;
    Deletable<std::string> authinfopw;
    Updateable<Publishability> disclose;
    ContactChange get_trimmed_copy()const;
};

} // namespace Epp::Contact
} // namespace Epp

#endif
