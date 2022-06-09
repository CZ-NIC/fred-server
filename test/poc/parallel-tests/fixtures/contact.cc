/*
 * Copyright (C) 2022  CZ.NIC, z. s. p. o.
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

#include "test/poc/parallel-tests/fixtures/contact.hh"

#include "libfred/registrable_object/contact/info_contact.hh"

#include <stdexcept>


namespace {

decltype(auto) make_contact(LibFred::OperationContext& ctx, LibFred::CreateContact& create)
{
    return LibFred::InfoContactById{create.exec(ctx).create_object_result.object_id}.exec(ctx).info_contact_data;
}

std::string get_version(int index)
{
    if (index < 0)
    {
        throw std::runtime_error{"negative index is not allowed"};
    }
    static constexpr char number_of_letters = 'Z' - 'A';
    if (index <= number_of_letters)
    {
        return std::string(1, 'A' + index);
    }
    return std::to_string(index - number_of_letters);
}

void set_contact(LibFred::CreateContact& op, bool company, int index)
{
    const auto version = get_version(index);
    if (company)
    {
        op.set_name("Default Company " + version)
          .set_organization("Default Company " + version + " Ltd")
          .set_email("default-" + version + "@company.com")
          .set_notifyemail("default-notify-" + version + "@company.com")
          .set_domain_expiration_warning_letter_enabled(true);
    }
    else
    {
        op.set_name("Default Contact " + version)
          .set_email("default-" + version + "@contact.org")
          .set_notifyemail("default-notify-" + version + "@contact.org")
          .set_ssntype("BIRTHDAY").set_ssn("1970-01-01");
    }
    const auto make_place = [&]()
    {
        LibFred::Contact::PlaceAddress place;
        place.street1 = "Street 1 - " + version;
        place.street2 = "Street 2 - " + version;
        place.street3 = "Street 3 - " + version;
        place.city = "City " + version;
        place.stateorprovince = "State Or Province " + version;
        place.postalcode = "143 21";
        place.country = "CZ";
        return place;
    };
    const auto make_addresses = [&]()
    {
        const auto place1 = company ? LibFred::ContactAddress{"Default Company " + version,
                                                                   make_place()}
                                         : LibFred::ContactAddress{Optional<std::string>{},
                                                                   make_place()};
        LibFred::ContactAddress place2 = place1;
        place2.country = "CY";
        LibFred::ContactAddress place3 = place1;
        place3.country = "CH";
        if (company)
        {
            return LibFred::ContactAddressList{
                    {LibFred::ContactAddressType::SHIPPING, place1},
                    {LibFred::ContactAddressType::SHIPPING_2, place2},
                    {LibFred::ContactAddressType::SHIPPING_3, place3}};
        }
        return LibFred::ContactAddressList{
                {LibFred::ContactAddressType::MAILING, place3},
                {LibFred::ContactAddressType::BILLING, place2},
                {LibFred::ContactAddressType::SHIPPING, place1},
                {LibFred::ContactAddressType::SHIPPING_2, place2},
                {LibFred::ContactAddressType::SHIPPING_3, place3}};
    };
    op.set_authinfo("1234" + version)
      .set_place(make_place())
      .set_telephone("+420.441207848")
      .set_fax("+420.361971091")
      .set_vat("1234" + version)
      .set_addresses(make_addresses())
      .set_disclosename(true)
      .set_discloseorganization(true);
}

}//namespace {anonymous}

namespace Test {

Contact::Contact(LibFred::OperationContext& ctx, LibFred::CreateContact create)
    : data{make_contact(ctx, create)}
{ }

}//namespace Test

using namespace Test::Setter;

LibFred::CreateContact Test::Setter::contact(LibFred::CreateContact create, int index)
{
    set_contact(create, false, index);
    return create;
}

LibFred::CreateContact Test::Setter::company(LibFred::CreateContact create, int index)
{
    set_contact(create, true, index);
    return create;
}
