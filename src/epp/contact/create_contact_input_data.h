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

#ifndef CREATE_CONTACT_INPUT_DATA_H_D431534126A348DFA7E2C485EB27B2A7
#define CREATE_CONTACT_INPUT_DATA_H_D431534126A348DFA7E2C485EB27B2A7

#include "src/epp/contact/contact_data.h"

#include <string>
#include <vector>

#include <boost/optional.hpp>

namespace Epp {
namespace Contact {

struct CreateContactInputData
{
    explicit CreateContactInputData(const ContactData& _src);
    struct Address
    {
        std::string street1;
        std::string street2;
        std::string street3;
        std::string city;
        std::string state_or_province;
        std::string postal_code;
        std::string country_code;
    };
    std::string name;
    std::string organization;
    std::vector<std::string> streets;
    std::string city;
    std::string state_or_province;
    std::string postal_code;
    std::string country_code;
    boost::optional<Address> mailing_address;
    std::string telephone;
    std::string fax;
    std::string email;
    std::string notify_email;
    std::string vat;
    boost::optional<ContactIdent> ident;
    boost::optional<std::string> authinfopw;
    boost::optional<ContactDisclose> disclose;
};

} // namespace Epp::Contact
} // namespace Epp

#endif//CREATE_CONTACT_INPUT_DATA_H_D431534126A348DFA7E2C485EB27B2A7
