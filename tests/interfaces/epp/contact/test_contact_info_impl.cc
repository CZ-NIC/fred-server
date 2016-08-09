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

#include "src/epp/disclose_policy.h"
#include "src/epp/contact/contact_info_impl.h"

#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/assign/list_of.hpp>

namespace {

template < Epp::ContactDisclose::Enum ITEM >
bool disclose(const Epp::ContactInfoOutputData &src)
{
        const bool some_entry_to_hide = !src.to_hide.empty();
        const bool some_entry_to_disclose = !src.to_disclose.empty();
        if (some_entry_to_hide && some_entry_to_disclose) {
            throw std::runtime_error("Only hide or disclose can be set, not both.");
        }
        if (!some_entry_to_hide && !some_entry_to_disclose) {
            return Epp::is_the_default_policy_to_disclose();
        }
        if (some_entry_to_hide && !some_entry_to_disclose) {
            return src.to_hide.find(ITEM) == src.to_hide.end();
        }
        return src.to_disclose.find(ITEM) != src.to_disclose.end();
}

void check_equal(const Epp::ContactInfoOutputData &epp_data, const Fred::InfoContactData &fred_data)
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
    BOOST_CHECK_EQUAL(epp_data.ident.get_value_or_default(),        fred_data.ssn.get_value_or_default());

    BOOST_CHECK_EQUAL(epp_data.identtype.isnull() ? "" : Epp::to_db_handle(epp_data.identtype.get_value()),
                      fred_data.ssntype.get_value_or_default());

    BOOST_CHECK_EQUAL(epp_data.auth_info_pw,                        fred_data.authinfopw);
    BOOST_CHECK_EQUAL(disclose< Epp::ContactDisclose::name         >(epp_data), fred_data.disclosename);
    BOOST_CHECK_EQUAL(disclose< Epp::ContactDisclose::organization >(epp_data), fred_data.discloseorganization );
    BOOST_CHECK_EQUAL(disclose< Epp::ContactDisclose::address      >(epp_data), fred_data.discloseaddress );
    BOOST_CHECK_EQUAL(disclose< Epp::ContactDisclose::telephone    >(epp_data), fred_data.disclosetelephone );
    BOOST_CHECK_EQUAL(disclose< Epp::ContactDisclose::fax          >(epp_data), fred_data.disclosefax );
    BOOST_CHECK_EQUAL(disclose< Epp::ContactDisclose::email        >(epp_data), fred_data.discloseemail );
    BOOST_CHECK_EQUAL(disclose< Epp::ContactDisclose::vat          >(epp_data), fred_data.disclosevat );
    BOOST_CHECK_EQUAL(disclose< Epp::ContactDisclose::ident        >(epp_data), fred_data.discloseident );
    BOOST_CHECK_EQUAL(disclose< Epp::ContactDisclose::notify_email >(epp_data), fred_data.disclosenotifyemail );
}

}//namespace {anonymous}

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


BOOST_FIXTURE_TEST_CASE(info_ok_full_data, has_contact)
{
    check_equal(
        Epp::contact_info_impl(ctx, contact.handle, Epp::SessionLang::en, registrar.id),
        contact);
}

#if 0
struct has_contact_with_external_and_nonexternal_states : has_contact
{
    has_contact_with_external_and_nonexternal_states()
    {
        external_states = boost::assign::list_of("serverDeleteProhibited")("serverUpdateProhibited");
        nonexternal_states = boost::assign::list_of("deleteCandidate");

        ctx.get_conn().exec_params(
            "UPDATE enum_object_states "
            "SET external='t'::BOOL,"
                "manual='t'::BOOL "
            "WHERE name=ANY($1::TEXT[])",
            Database::query_param_list("{" + boost::algorithm::join(external_states, ",") + "}"));

        ctx.get_conn().exec_params(
            "UPDATE enum_object_states "
            "SET external='f'::BOOL,"
                "manual='t'::BOOL "
            "WHERE name=ANY($1::TEXT[])",
            Database::query_param_list("{" + boost::algorithm::join(nonexternal_states, ",") + "}"));

        Fred::CreateObjectStateRequestId(contact.id, external_states).exec(ctx);
        Fred::CreateObjectStateRequestId(contact.id, nonexternal_states).exec(ctx);
        Fred::PerformObjectStateRequest(contact.id).exec(ctx);

        ctx.commit_transaction();
    }

    std::set< std::string > external_states;
    std::set< std::string > nonexternal_states;
};
#endif

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
