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

/**
 *  @file
 */

#include "tests/interfaces/epp/util.h"
#include "tests/interfaces/epp/contact/fixture.h"

#include "src/epp/impl/disclose_policy.h"
#include "src/epp/contact/info_contact.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/optional.hpp>
#include <boost/test/unit_test.hpp>

namespace {

template < Epp::ContactDisclose::Item::Enum ITEM >
bool to_disclose(const Epp::Contact::InfoContactOutputData &epp_data)
{
    if (!epp_data.disclose.is_initialized()) {
        return Epp::is_the_default_policy_to_disclose();
    }
    return epp_data.disclose->should_be_disclosed< ITEM >(Epp::is_the_default_policy_to_disclose());
}

void check_equal(const Epp::Contact::InfoContactOutputData &epp_data, const Fred::InfoContactData &fred_data)
{
    BOOST_CHECK_EQUAL(boost::to_upper_copy(epp_data.handle),        fred_data.handle);
    BOOST_CHECK_EQUAL(epp_data.name.get_value_or_default(),         fred_data.name.get_value_or_default());
    BOOST_CHECK_EQUAL(epp_data.organization.get_value_or_default(), fred_data.organization.get_value_or_default());

    BOOST_CHECK_EQUAL(epp_data.street1.get_value_or_default(),      fred_data.place.get_value_or_default().street1);
    BOOST_CHECK_EQUAL(epp_data.street2.get_value_or_default(),      fred_data.place.get_value_or_default().street2.get_value_or_default());
    BOOST_CHECK_EQUAL(epp_data.street3.get_value_or_default(),      fred_data.place.get_value_or_default().street3.get_value_or_default());
    BOOST_CHECK_EQUAL(epp_data.city.get_value_or_default(),         fred_data.place.get_value_or_default().city);
    BOOST_CHECK_EQUAL(epp_data.postal_code.get_value_or_default(),  fred_data.place.get_value_or_default().postalcode);
    BOOST_CHECK_EQUAL(epp_data.state_or_province.get_value_or_default(), fred_data.place.get_value_or_default().stateorprovince.get_value_or_default());
    BOOST_CHECK_EQUAL(epp_data.country_code.get_value_or_default(), fred_data.place.get_value_or_default().country);
    BOOST_CHECK_EQUAL(epp_data.telephone.get_value_or_default(),    fred_data.telephone.get_value_or_default());
    BOOST_CHECK_EQUAL(epp_data.fax.get_value_or_default(),          fred_data.fax.get_value_or_default());
    BOOST_CHECK_EQUAL(epp_data.email.get_value_or_default(),        fred_data.email.get_value_or_default());
    BOOST_CHECK_EQUAL(epp_data.notify_email.get_value_or_default(), fred_data.notifyemail.get_value_or_default());
    BOOST_CHECK_EQUAL(epp_data.VAT.get_value_or_default(),          fred_data.vat.get_value_or_default());
    BOOST_CHECK_EQUAL(epp_data.personal_id.is_initialized() ? epp_data.personal_id->get() : "",
                      fred_data.ssn.get_value_or_default());

    BOOST_CHECK_EQUAL(epp_data.personal_id.is_initialized() ? epp_data.personal_id->get_type() : "",
                      fred_data.ssntype.get_value_or_default());

    BOOST_REQUIRE(epp_data.auth_info_pw);
    BOOST_CHECK_EQUAL(epp_data.auth_info_pw.value(),                        fred_data.authinfopw);

    BOOST_CHECK_EQUAL(to_disclose< Epp::ContactDisclose::Item::name         >(epp_data), fred_data.disclosename);
    BOOST_CHECK_EQUAL(to_disclose< Epp::ContactDisclose::Item::organization >(epp_data), fred_data.discloseorganization);
    BOOST_CHECK_EQUAL(to_disclose< Epp::ContactDisclose::Item::address      >(epp_data), fred_data.discloseaddress);
    BOOST_CHECK_EQUAL(to_disclose< Epp::ContactDisclose::Item::telephone    >(epp_data), fred_data.disclosetelephone);
    BOOST_CHECK_EQUAL(to_disclose< Epp::ContactDisclose::Item::fax          >(epp_data), fred_data.disclosefax);
    BOOST_CHECK_EQUAL(to_disclose< Epp::ContactDisclose::Item::email        >(epp_data), fred_data.discloseemail);
    BOOST_CHECK_EQUAL(to_disclose< Epp::ContactDisclose::Item::vat          >(epp_data), fred_data.disclosevat);
    BOOST_CHECK_EQUAL(to_disclose< Epp::ContactDisclose::Item::ident        >(epp_data), fred_data.discloseident);
    BOOST_CHECK_EQUAL(to_disclose< Epp::ContactDisclose::Item::notify_email >(epp_data), fred_data.disclosenotifyemail);
}

}//namespace {anonymous}

BOOST_AUTO_TEST_SUITE(TestEpp)
BOOST_AUTO_TEST_SUITE(InfoContact)

BOOST_FIXTURE_TEST_CASE(info_invalid_registrar_id, has_contact)
{
    BOOST_CHECK_THROW(
        Epp::Contact::info_contact(
            ctx,
            contact.handle,
            Epp::SessionLang::en,
            0 /* <== !!! */
        ),
        Epp::AuthErrorServerClosingConnection
    );
}

BOOST_FIXTURE_TEST_CASE(info_fail_nonexistent_handle, has_contact)
{
    BOOST_CHECK_THROW(
        Epp::Contact::info_contact(
            ctx,
            contact.handle + "SOMEobscureSTRING",
            Epp::SessionLang::en,
            42 /* TODO */
        ),
        Epp::NonexistentHandle
    );
}


BOOST_FIXTURE_TEST_CASE(info_ok_full_data, has_contact)
{
    check_equal(
        Epp::Contact::info_contact(ctx, contact.handle, Epp::SessionLang::en, registrar.id),
        contact);
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
