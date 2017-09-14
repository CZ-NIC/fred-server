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

#include "tests/interfaces/epp/contact/fixture.h"
#include "tests/interfaces/epp/util.h"

#include "src/epp/contact/info_contact.h"
#include "src/epp/impl/disclose_policy.h"
#include "src/epp/epp_response_failure.h"
#include "src/epp/epp_result_code.h"
#include "src/epp/session_data.h"
#include "src/epp/contact/info_contact_config_data.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/optional.hpp>
#include <boost/test/unit_test.hpp>

namespace Test {

namespace {

template < ::Epp::Contact::ContactDisclose::Item::Enum ITEM>
bool to_disclose(const ::Epp::Contact::InfoContactOutputData& epp_data)
{
    if (!epp_data.disclose.is_initialized()) {
        return ::Epp::is_the_default_policy_to_disclose();
    }
    return epp_data.disclose->should_be_disclosed<ITEM>(Epp::is_the_default_policy_to_disclose());
}

void check_equal(const ::Epp::Contact::InfoContactOutputData& epp_data, const Fred::InfoContactData& fred_data)
{
    BOOST_CHECK_EQUAL(boost::to_upper_copy(epp_data.handle), fred_data.handle);
    BOOST_CHECK_EQUAL(epp_data.name.get_value_or_default(), fred_data.name.get_value_or_default());
    BOOST_CHECK_EQUAL(epp_data.organization.get_value_or_default(), fred_data.organization.get_value_or_default());
    BOOST_CHECK_EQUAL(epp_data.street1.get_value_or_default(), fred_data.place.get_value_or_default().street1);
    BOOST_CHECK_EQUAL(epp_data.street2.get_value_or_default(), fred_data.place.get_value_or_default().street2.get_value_or_default());
    BOOST_CHECK_EQUAL(epp_data.street3.get_value_or_default(), fred_data.place.get_value_or_default().street3.get_value_or_default());
    BOOST_CHECK_EQUAL(epp_data.city.get_value_or_default(), fred_data.place.get_value_or_default().city);
    BOOST_CHECK_EQUAL(epp_data.postal_code.get_value_or_default(), fred_data.place.get_value_or_default().postalcode);
    BOOST_CHECK_EQUAL(epp_data.state_or_province.get_value_or_default(), fred_data.place.get_value_or_default().stateorprovince.get_value_or_default());
    BOOST_CHECK_EQUAL(epp_data.country_code.get_value_or_default(), fred_data.place.get_value_or_default().country);

    const bool epp_mailing_address_presents = epp_data.mailing_address != boost::none;
    const Fred::ContactAddressList::const_iterator addresses_itr = fred_data.addresses.find(Fred::ContactAddressType::MAILING);
    const bool fred_mailing_address_presents = addresses_itr != fred_data.addresses.end();
    BOOST_CHECK_EQUAL(epp_mailing_address_presents, fred_mailing_address_presents);
    if (epp_mailing_address_presents && fred_mailing_address_presents)
    {
        BOOST_CHECK(!addresses_itr->second.company_name.isset());
        BOOST_CHECK(epp_data.mailing_address->street1 != boost::none);
        if (epp_data.mailing_address->street1 != boost::none)
        {
            BOOST_CHECK_EQUAL(*(epp_data.mailing_address->street1), addresses_itr->second.street1);
        }
        BOOST_CHECK_EQUAL(epp_data.mailing_address->street2 != boost::none, addresses_itr->second.street2.isset());
        if ((epp_data.mailing_address->street2 != boost::none) && (addresses_itr->second.street2.isset()))
        {
            BOOST_CHECK_EQUAL(*(epp_data.mailing_address->street2), addresses_itr->second.street2.get_value());
        }
        BOOST_CHECK_EQUAL(epp_data.mailing_address->street3 != boost::none, addresses_itr->second.street3.isset());
        if ((epp_data.mailing_address->street3 != boost::none) && (addresses_itr->second.street3.isset()))
        {
            BOOST_CHECK_EQUAL(*(epp_data.mailing_address->street3), addresses_itr->second.street3.get_value());
        }
        BOOST_CHECK(epp_data.mailing_address->city != boost::none);
        if (epp_data.mailing_address->city != boost::none)
        {
            BOOST_CHECK_EQUAL(*(epp_data.mailing_address->city), addresses_itr->second.city);
        }
        BOOST_CHECK_EQUAL(epp_data.mailing_address->state_or_province != boost::none, addresses_itr->second.stateorprovince.isset());
        if ((epp_data.mailing_address->state_or_province != boost::none) && (addresses_itr->second.stateorprovince.isset()))
        {
            BOOST_CHECK_EQUAL(*(epp_data.mailing_address->state_or_province), addresses_itr->second.stateorprovince.get_value());
        }
        BOOST_CHECK(epp_data.mailing_address->postal_code != boost::none);
        if (epp_data.mailing_address->postal_code != boost::none)
        {
            BOOST_CHECK_EQUAL(*(epp_data.mailing_address->postal_code), addresses_itr->second.postalcode);
        }
        BOOST_CHECK(epp_data.mailing_address->country_code != boost::none);
        if (epp_data.mailing_address->country_code != boost::none)
        {
            BOOST_CHECK_EQUAL(*(epp_data.mailing_address->country_code), addresses_itr->second.country);
        }
    }

    BOOST_CHECK_EQUAL(epp_data.telephone.get_value_or_default(), fred_data.telephone.get_value_or_default());
    BOOST_CHECK_EQUAL(epp_data.fax.get_value_or_default(), fred_data.fax.get_value_or_default());
    BOOST_CHECK_EQUAL(epp_data.email.get_value_or_default(), fred_data.email.get_value_or_default());
    BOOST_CHECK_EQUAL(epp_data.notify_email.get_value_or_default(), fred_data.notifyemail.get_value_or_default());
    BOOST_CHECK_EQUAL(epp_data.VAT.get_value_or_default(), fred_data.vat.get_value_or_default());
    BOOST_CHECK_EQUAL(epp_data.personal_id.is_initialized() ? epp_data.personal_id->get() : "",
            fred_data.ssn.get_value_or_default());
    BOOST_CHECK_EQUAL(epp_data.personal_id.is_initialized() ? epp_data.personal_id->get_type() : "",
            fred_data.ssntype.get_value_or_default());
    BOOST_REQUIRE(epp_data.authinfopw);
    BOOST_CHECK_EQUAL(epp_data.authinfopw.value(), fred_data.authinfopw);
    BOOST_CHECK_EQUAL(to_disclose< ::Epp::Contact::ContactDisclose::Item::name>(epp_data), fred_data.disclosename);
    BOOST_CHECK_EQUAL(to_disclose< ::Epp::Contact::ContactDisclose::Item::organization>(epp_data), fred_data.discloseorganization);
    BOOST_CHECK_EQUAL(to_disclose< ::Epp::Contact::ContactDisclose::Item::address>(epp_data), fred_data.discloseaddress);
    BOOST_CHECK_EQUAL(to_disclose< ::Epp::Contact::ContactDisclose::Item::telephone>(epp_data), fred_data.disclosetelephone);
    BOOST_CHECK_EQUAL(to_disclose< ::Epp::Contact::ContactDisclose::Item::fax>(epp_data), fred_data.disclosefax);
    BOOST_CHECK_EQUAL(to_disclose< ::Epp::Contact::ContactDisclose::Item::email>(epp_data), fred_data.discloseemail);
    BOOST_CHECK_EQUAL(to_disclose< ::Epp::Contact::ContactDisclose::Item::vat>(epp_data), fred_data.disclosevat);
    BOOST_CHECK_EQUAL(to_disclose< ::Epp::Contact::ContactDisclose::Item::ident>(epp_data), fred_data.discloseident);
    BOOST_CHECK_EQUAL(to_disclose< ::Epp::Contact::ContactDisclose::Item::notify_email>(epp_data), fred_data.disclosenotifyemail);
}

} // namespace Test::{anonymous}

BOOST_AUTO_TEST_SUITE(Backend)
BOOST_AUTO_TEST_SUITE(Epp)
BOOST_AUTO_TEST_SUITE(Contact)
BOOST_AUTO_TEST_SUITE(InfoContact)

bool info_invalid_registrar_id_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::authentication_error_server_closing_connection);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(info_invalid_registrar_id, supply_ctx<HasSessionWithUnauthenticatedRegistrar>)
{
    BOOST_CHECK_EXCEPTION(
        ::Epp::Contact::info_contact(
            ctx,
            ValidHandle().handle,
            DefaultInfoContactConfigData(),
            session_with_unauthenticated_registrar.data
        ),
        ::Epp::EppResponseFailure,
        info_invalid_registrar_id_exception
    );
}

bool info_fail_nonexistent_handle_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_does_not_exist);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(info_fail_nonexistent_handle, supply_ctx<HasRegistrarWithSession>)
{
    BOOST_CHECK_EXCEPTION(
        ::Epp::Contact::info_contact(
            ctx,
            NonexistentHandle().handle,
            DefaultInfoContactConfigData(),
            session.data
        ),
        ::Epp::EppResponseFailure,
        info_fail_nonexistent_handle_exception
    );
}


BOOST_FIXTURE_TEST_CASE(info_ok_full_data, supply_ctx<HasRegistrarWithSessionAndContact>)
{
    check_equal(
        ::Epp::Contact::info_contact(
            ctx,
            contact.data.handle,
            DefaultInfoContactConfigData(),
            session.data
        ),
        contact.data);
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();

} // namespace Test
