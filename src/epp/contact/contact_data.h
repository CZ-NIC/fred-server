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

#ifndef CONTACT_DATA_H_ED120B0CE83DFE5832F00845928ECFA0//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define CONTACT_DATA_H_ED120B0CE83DFE5832F00845928ECFA0

#include "src/epp/contact/contact_disclose.h"
#include "src/epp/contact/contact_ident.h"

#include <boost/optional.hpp>

#include <string>
#include <vector>

namespace Epp {
namespace Contact {

struct ContactData
{
    struct Address
    {
        boost::optional<std::string> street1;
        boost::optional<std::string> street2;
        boost::optional<std::string> street3;
        boost::optional<std::string> city;
        boost::optional<std::string> state_or_province;
        boost::optional<std::string> postal_code;
        boost::optional<std::string> country_code;
    };
    boost::optional<std::string> name;
    boost::optional<std::string> organization;
    std::vector<std::string> streets;
    boost::optional<std::string> city;
    boost::optional<std::string> state_or_province;
    boost::optional<std::string> postal_code;
    boost::optional<std::string> country_code;
    boost::optional<Address> mailing_address;
    boost::optional<std::string> telephone;
    boost::optional<std::string> fax;
    boost::optional<std::string> email;
    boost::optional<std::string> notify_email;
    boost::optional<std::string> vat;
    boost::optional<ContactIdent> ident;
    boost::optional<std::string> authinfopw;
    boost::optional<ContactDisclose> disclose;
};

ContactData trim(const ContactData& src);

} // namespace Epp::Contact
} // namespace Epp

#endif//CONTACT_DATA_H_ED120B0CE83DFE5832F00845928ECFA0
