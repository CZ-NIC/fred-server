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

#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/assign/list_of.hpp>

#include "tests/interfaces/epp/util.h"
#include "tests/interfaces/epp/contact/fixture.h"

#include "src/epp/contact/contact_info_impl.h"

BOOST_AUTO_TEST_SUITE(TestEpp)
BOOST_AUTO_TEST_SUITE(ContactInfoImpl)

BOOST_FIXTURE_TEST_CASE(info_invalid_registrar_id, has_contact)
{
    BOOST_CHECK_THROW(
        Epp::contact_info_impl(
            ctx,
            contact.handle,
            Epp::SessionLang::en,
            0 /* <== !!! */
        ),
        Epp::AuthErrorServerClosingConnection
    );
}

BOOST_FIXTURE_TEST_CASE(info_fail_handle_format, has_contact)
{
    BOOST_CHECK_THROW(
        Epp::contact_info_impl(
            ctx,
            "SOME_obscure_String*/-+!#*",
            Epp::SessionLang::en,
            42 /* TODO */
        ),
        Epp::InvalidHandle
    );
}

BOOST_FIXTURE_TEST_CASE(info_fail_nonexistent_handle, has_contact)
{
    BOOST_CHECK_THROW(
        Epp::contact_info_impl(
            ctx,
            contact.handle + "SOMEobscureSTRING",
            Epp::SessionLang::en,
            42 /* TODO */
        ),
        Epp::NonexistentHandle
    );
}

static void check_equal(const Epp::ContactInfoOutputData& contact_data, const Fred::InfoContactData& info_data) {
    BOOST_CHECK_EQUAL( boost::to_upper_copy( contact_data.handle ), info_data.handle );
    BOOST_CHECK_EQUAL( contact_data.name.get_value_or_default(),                info_data.name.get_value_or_default() );
    BOOST_CHECK_EQUAL( contact_data.organization.get_value_or_default(),        info_data.organization.get_value_or_default() );

    BOOST_CHECK_EQUAL( contact_data.street1.get_value_or_default(),             info_data.place.get_value_or_default().street1 );
    BOOST_CHECK_EQUAL( contact_data.street2.get_value_or_default(),             info_data.place.get_value_or_default().street2.get_value_or_default() );
    BOOST_CHECK_EQUAL( contact_data.street3.get_value_or_default(),             info_data.place.get_value_or_default().street3.get_value_or_default() );
    BOOST_CHECK_EQUAL( contact_data.city.get_value_or_default(),                info_data.place.get_value_or_default().city );
    BOOST_CHECK_EQUAL( contact_data.postal_code.get_value_or_default(),         info_data.place.get_value_or_default().postalcode );
    BOOST_CHECK_EQUAL( contact_data.state_or_province.get_value_or_default(),   info_data.place.get_value_or_default().stateorprovince.get_value_or_default() );
    BOOST_CHECK_EQUAL( contact_data.country_code.get_value_or_default(),        info_data.place.get_value_or_default().country );
    BOOST_CHECK_EQUAL( contact_data.telephone.get_value_or_default(),           info_data.telephone.get_value_or_default() );
    BOOST_CHECK_EQUAL( contact_data.fax.get_value_or_default(),                 info_data.fax.get_value_or_default() );
    BOOST_CHECK_EQUAL( contact_data.email.get_value_or_default(),               info_data.email.get_value_or_default() );
    BOOST_CHECK_EQUAL( contact_data.notify_email.get_value_or_default(),        info_data.notifyemail.get_value_or_default() );
    BOOST_CHECK_EQUAL( contact_data.VAT.get_value_or_default(),                 info_data.vat.get_value_or_default() );
    BOOST_CHECK_EQUAL( contact_data.ident.get_value_or_default(),               info_data.ssn.get_value_or_default() );

    BOOST_CHECK_EQUAL(
        contact_data.identtype.isnull()
            ?   ""
            :   Epp::to_db_handle( contact_data.identtype.get_value() ),
        info_data.ssntype.get_value_or_default()
    );

    BOOST_CHECK_EQUAL( contact_data.auth_info_pw,           info_data.authinfopw );
    BOOST_CHECK_EQUAL( contact_data.disclose_name,          info_data.disclosename );
    BOOST_CHECK_EQUAL( contact_data.disclose_organization,  info_data.discloseorganization );
    BOOST_CHECK_EQUAL( contact_data.disclose_address,       info_data.discloseaddress );
    BOOST_CHECK_EQUAL( contact_data.disclose_telephone,     info_data.disclosetelephone );
    BOOST_CHECK_EQUAL( contact_data.disclose_fax,           info_data.disclosefax );
    BOOST_CHECK_EQUAL( contact_data.disclose_email,         info_data.discloseemail );
    BOOST_CHECK_EQUAL( contact_data.disclose_VAT,           info_data.disclosevat );
    BOOST_CHECK_EQUAL( contact_data.disclose_ident,         info_data.discloseident );
    BOOST_CHECK_EQUAL( contact_data.disclose_notify_email,  info_data.disclosenotifyemail );
}

BOOST_FIXTURE_TEST_CASE(info_ok_full_data, has_contact)
{
    check_equal(
        Epp::contact_info_impl(
            ctx,
            contact.handle,
            Epp::SessionLang::en,
            registrar.id
        ),
        contact
    );
}

struct has_contact_with_external_and_nonexternal_states : has_contact {
    std::set<std::string> external_states;
    std::set<std::string> nonexternal_states;

    has_contact_with_external_and_nonexternal_states() {
        external_states = boost::assign::list_of("serverDeleteProhibited")("serverUpdateProhibited");
        nonexternal_states = boost::assign::list_of("deleteCandidate");

        ctx.get_conn().exec_params(
            "UPDATE enum_object_states "
                "SET external = 'true'::bool, manual = 'true'::bool "
                "WHERE name = ANY( $1::text[] )",
            Database::query_param_list("{" + boost::algorithm::join(external_states, ", ") + "}")
        );

        ctx.get_conn().exec_params(
            "UPDATE enum_object_states "
                "SET external = 'false'::bool, manual = 'true'::bool "
                "WHERE name = ANY( $1::text[] )",
            Database::query_param_list("{" + boost::algorithm::join(nonexternal_states, ", ") + "}")
        );

        Fred::CreateObjectStateRequestId(contact.id, external_states).exec(ctx);
        Fred::CreateObjectStateRequestId(contact.id, nonexternal_states).exec(ctx);
        Fred::PerformObjectStateRequest(contact.id).exec(ctx);

        ctx.commit_transaction();
    }
};

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
