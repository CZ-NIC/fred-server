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
#include <boost/assign/list_of.hpp>

#include "tests/interfaces/epp/util.h"
#include "tests/interfaces/epp/contact/fixture.h"

#include "src/epp/contact/contact_update_impl.h"

BOOST_AUTO_TEST_SUITE(TestEpp)
BOOST_AUTO_TEST_SUITE(ContactUpdateImpl)

BOOST_FIXTURE_TEST_CASE(update_invalid_registrar_id, has_contact)
{
    const Epp::ContactUpdateInputData data(
        contact.handle + "*?!",
        "Jan Novak",
        "Firma, a. s.",
        "Vaclavske namesti 1",
        "53. patro",
        "vpravo",
        "Brno",
        "Morava",
        "20000",
        "CZ",
        Optional<std::string>(),
        Optional<std::string>(),
        Optional<std::string>(),
        Optional<std::string>(),
        Optional<std::string>(),
        Optional<std::string>(),
        Nullable<Epp::IdentType::Enum>(),
        Optional<std::string>(),
        Optional<bool>(),
        Optional<bool>(),
        Optional<bool>(),
        Optional<bool>(),
        Optional<bool>(),
        Optional<bool>(),
        Optional<bool>(),
        Optional<bool>(),
        Optional<bool>()
    );

    BOOST_CHECK_THROW(
        Epp::contact_update_impl(
            ctx,
            data,
            0,  /* <== !!! */
            42 /* TODO */
        ),
        Epp::AuthErrorServerClosingConnection
    );
}

BOOST_FIXTURE_TEST_CASE(update_fail_nonexistent_handle, has_contact)
{
    const Epp::ContactUpdateInputData data(
        contact.handle + "abc",
        "Jan Novak",
        "Firma, a. s.",
        "Vaclavske namesti 1",
        "53. patro",
        "vpravo",
        "Brno",
        "Morava",
        "20000",
        "CZ",
        Optional<std::string>(),
        Optional<std::string>(),
        Optional<std::string>(),
        Optional<std::string>(),
        Optional<std::string>(),
        Optional<std::string>(),
        Nullable<Epp::IdentType::Enum>(),
        Optional<std::string>(),
        Optional<bool>(),
        Optional<bool>(),
        Optional<bool>(),
        Optional<bool>(),
        Optional<bool>(),
        Optional<bool>(),
        Optional<bool>(),
        Optional<bool>(),
        Optional<bool>()
    );

    BOOST_CHECK_THROW(
        Epp::contact_update_impl(
            ctx,
            data,
            registrar.id,
            42 /* TODO */
        ),
        Epp::NonexistentHandle
    );
}

BOOST_FIXTURE_TEST_CASE(update_fail_wrong_registrar, has_contact_and_a_different_registrar)
{
    const Epp::ContactUpdateInputData data(
        contact.handle,
        "Jan Novak",
        "Firma, a. s.",
        "Vaclavske namesti 1",
        "53. patro",
        "vpravo",
        "Brno",
        "Morava",
        "20000",
        "CZ",
        Optional<std::string>(),
        Optional<std::string>(),
        Optional<std::string>(),
        Optional<std::string>(),
        Optional<std::string>(),
        Optional<std::string>(),
        Nullable<Epp::IdentType::Enum>(),
        Optional<std::string>(),
        Optional<bool>(),
        Optional<bool>(),
        Optional<bool>(),
        Optional<bool>(),
        Optional<bool>(),
        Optional<bool>(),
        Optional<bool>(),
        Optional<bool>(),
        Optional<bool>()
    );

    BOOST_CHECK_THROW(
        Epp::contact_update_impl(
            ctx,
            data,
            the_different_registrar.id,
            42 /* TODO */
        ),
        Epp::AuthorizationError
    );
}

BOOST_FIXTURE_TEST_CASE(update_fail_prohibiting_status1, has_contact_with_server_update_prohibited)
{
    const Epp::ContactUpdateInputData data(
        contact.handle,
        "Jan Novak",
        "Firma, a. s.",
        "Vaclavske namesti 1",
        "53. patro",
        "vpravo",
        "Brno",
        "Morava",
        "20000",
        "CZ",
        Optional<std::string>(),
        Optional<std::string>(),
        Optional<std::string>(),
        Optional<std::string>(),
        Optional<std::string>(),
        Optional<std::string>(),
        Nullable<Epp::IdentType::Enum>(),
        Optional<std::string>(),
        Optional<bool>(),
        Optional<bool>(),
        Optional<bool>(),
        Optional<bool>(),
        Optional<bool>(),
        Optional<bool>(),
        Optional<bool>(),
        Optional<bool>(),
        Optional<bool>()
    );

    BOOST_CHECK_THROW(
        Epp::contact_update_impl(
            ctx,
            data,
            registrar.id,
            42 /* TODO */

        ),
        Epp::ObjectStatusProhibitingOperation
    );
}

BOOST_FIXTURE_TEST_CASE(update_fail_prohibiting_status2, has_contact_with_delete_candidate)
{
    const Epp::ContactUpdateInputData data(
        contact.handle,
        "Jan Novak",
        "Firma, a. s.",
        "Vaclavske namesti 1",
        "53. patro",
        "vpravo",
        "Brno",
        "Morava",
        "20000",
        "CZ",
        Optional<std::string>(),
        Optional<std::string>(),
        Optional<std::string>(),
        Optional<std::string>(),
        Optional<std::string>(),
        Optional<std::string>(),
        Nullable<Epp::IdentType::Enum>(),
        Optional<std::string>(),
        Optional<bool>(),
        Optional<bool>(),
        Optional<bool>(),
        Optional<bool>(),
        Optional<bool>(),
        Optional<bool>(),
        Optional<bool>(),
        Optional<bool>(),
        Optional<bool>()
    );

    BOOST_CHECK_THROW(
        Epp::contact_update_impl(
            ctx,
            data,
            registrar.id,
            42 /* TODO */

        ),
        Epp::ObjectStatusProhibitingOperation
    );
}

BOOST_FIXTURE_TEST_CASE(update_fail_prohibiting_status_request, has_contact_with_delete_candidate_request)
{
    const Epp::ContactUpdateInputData data(
        contact.handle,
        "Jan Novak",
        "",
        "Vaclavske namesti 1",
        "53. patro",
        "vpravo",
        "Brno",
        "Morava",
        "20000",
        "CZ",
        "+420 123 456 789",
        "+420 987 654 321",
        "jan@novak.novak",
        "jan.notify@novak.novak",
        "MyVATstring",
        "CZ0123456789",
        Epp::IdentType::identity_card,
        "a6tg85jk57yu97",
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true
    );

    BOOST_CHECK_THROW(
        Epp::contact_update_impl(
            ctx,
            data,
            registrar.id,
            42 /* TODO */

        ),
        Epp::ObjectStatusProhibitingOperation
    );

    /* now object has the state deleteCandidate itself */
    {
        std::vector<std::string> object_states_after;
        {
            BOOST_FOREACH(const Fred::ObjectStateData& state, Fred::GetObjectStates(contact.id).exec(ctx) ) {
                object_states_after.push_back(state.state_name);
            }
        }

        BOOST_CHECK(
            std::find( object_states_after.begin(), object_states_after.end(), status )
            !=
            object_states_after.end()
        );
    }
}

BOOST_FIXTURE_TEST_CASE(update_fail_nonexistent_country_code, has_contact)
{
    const Epp::ContactUpdateInputData data(
        contact.handle,
        "Jan Novak",
        "Spol s r. o.",
        "Vaclavske namesti 1",
        "53. patro",
        "vpravo",
        "Brno",
        "Morava",
        "20000",
        "X123Z", /* <- !!! */
        Optional<std::string>(),
        Optional<std::string>(),
        Optional<std::string>(),
        Optional<std::string>(),
        Optional<std::string>(),
        Optional<std::string>(),
        Nullable<Epp::IdentType::Enum>(),
        Optional<std::string>(),
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false
    );

    try {
        try {
            Epp::contact_update_impl(
                ctx,
                data,
                registrar.id,
                42 /* TODO */
            );
        } catch(...) {
            Test::check_correct_aggregated_exception_was_thrown(Epp::Error::of_scalar_parameter(Epp::Param::contact_cc, Epp::Reason::country_notexist));
        }
    } catch (const std::exception& e) {
        BOOST_MESSAGE(e.what());
    }
}

BOOST_FIXTURE_TEST_CASE(update_fail_address_cant_be_undisclosed, has_contact)
{
    const Epp::ContactUpdateInputData data(
        contact.handle,
        "Jan Novak",
        "", /* <- !!! */
        "Vaclavske namesti 1",
        "53. patro",
        "vpravo",
        "Brno",
        "Morava",
        "20000",
        "CZ",
        Optional<std::string>(),
        Optional<std::string>(),
        Optional<std::string>(),
        Optional<std::string>(),
        Optional<std::string>(),
        Optional<std::string>(),
        Nullable<Epp::IdentType::Enum>(),
        Optional<std::string>(),
        false,
        false,
        false,  /* <- !!! */
        false,
        false,
        false,
        false,
        false,
        false
    );

    BOOST_CHECK_THROW(
        Epp::contact_update_impl(
            ctx,
            data,
            registrar.id,
            42 /* TODO */
        ),
        Epp::ObjectStatusProhibitingOperation
    );
}

static void check_equal(
    const Fred::InfoContactData& info_before,
    const Epp::ContactUpdateInputData& update,
    const Fred::InfoContactData& info_after
) {
    BOOST_CHECK_EQUAL( boost::to_upper_copy( info_after.handle ), info_before.handle );
    BOOST_CHECK_EQUAL(
        info_after.name,
        update.name.isset()         ? update.name.get_value()           : info_before.name
    );
    BOOST_CHECK_EQUAL(
        info_after.organization,
        update.organization.isset() ? update.organization.get_value()   : info_before.organization
    );
    BOOST_CHECK_EQUAL(
        info_after.place.get_value_or_default().street1,
        update.street1.isset()
            ? update.street1.get_value()
            : info_before.place.get_value_or_default().street1
    );
    BOOST_CHECK_EQUAL(
        info_after.place.get_value_or_default().street2,
        update.street2.isset()
            ? update.street2.get_value()
            : info_before.place.get_value_or_default().street2
    );
    BOOST_CHECK_EQUAL(
        info_after.place.get_value_or_default().street3,
        update.street3.isset()
            ? update.street3.get_value()
            : info_before.place.get_value_or_default().street3
    );
    BOOST_CHECK_EQUAL(
        info_after.place.get_value_or_default().city,
        update.city.isset()
            ? update.city.get_value()
            : info_before.place.get_value_or_default().city
    );
    BOOST_CHECK_EQUAL(
        info_after.place.get_value_or_default().postalcode,
        update.postal_code.isset()
            ? update.postal_code.get_value()
            : info_before.place.get_value_or_default().postalcode
    );
    BOOST_CHECK_EQUAL(
        info_after.place.get_value_or_default().stateorprovince,
        update.state_or_province.isset()
            ? update.state_or_province.get_value()
            : info_before.place.get_value_or_default().stateorprovince
    );
    BOOST_CHECK_EQUAL(
        info_after.place.get_value_or_default().country,
        update.country_code.isset()
            ? update.country_code.get_value()
            : info_before.place.get_value_or_default().country
    );
    BOOST_CHECK_EQUAL(
        info_after.telephone,
        update.telephone.isset()        ? update.telephone.get_value()      : info_before.telephone
    );
    BOOST_CHECK_EQUAL(
        info_after.fax,
        update.fax.isset()              ? update.fax.get_value()            : info_before.fax
    );
    BOOST_CHECK_EQUAL(
        info_after.email,
        update.email.isset()            ? update.email.get_value()          : info_before.email
    );
    BOOST_CHECK_EQUAL(
        info_after.notifyemail,
        update.notify_email.isset()     ? update.notify_email.get_value()   : info_before.notifyemail
    );
    BOOST_CHECK_EQUAL(
        info_after.vat,
        update.VAT.isset()              ? update.VAT.get_value()            : info_before.vat
    );
    BOOST_CHECK_EQUAL(
        info_after.ssn,
        update.ident.isset()            ? update.ident.get_value()          : info_before.ssn
    );
    BOOST_CHECK_EQUAL(
        info_after.ssntype,
        ! update.identtype.isnull()     ? Epp::to_db_handle( update.identtype.get_value() ) : info_before.ssntype
    );
    BOOST_CHECK_EQUAL(
        info_after.authinfopw,
        update.authinfo.isset()         ? update.authinfo.get_value()       : info_before.authinfopw
    );
    BOOST_CHECK_EQUAL(
        info_after.disclosename,
        update.disclose_name.isset()    ? update.disclose_name.get_value()  : info_before.disclosename
    );
    BOOST_CHECK_EQUAL(
        info_after.discloseorganization,
        update.disclose_organization.isset()? update.disclose_organization.get_value()  : info_before.discloseorganization
    );
    BOOST_CHECK_EQUAL(
        info_after.discloseaddress,
        update.disclose_address.isset()     ? update.disclose_address.get_value()       : info_before.discloseaddress
    );
    BOOST_CHECK_EQUAL(
        info_after.disclosetelephone,
        update.disclose_telephone.isset()   ? update.disclose_telephone.get_value()     : info_before.disclosetelephone
    );
    BOOST_CHECK_EQUAL(
        info_after.disclosefax,
        update.disclose_fax.isset()     ? update.disclose_fax.get_value()               : info_before.disclosefax
    );
    BOOST_CHECK_EQUAL(
        info_after.discloseemail,
        update.disclose_email.isset()   ? update.disclose_email.get_value()             : info_before.discloseemail
    );
    BOOST_CHECK_EQUAL(
        info_after.disclosevat,
        update.disclose_VAT.isset()     ? update.disclose_VAT.get_value()               : info_before.disclosevat
    );
    BOOST_CHECK_EQUAL(
        info_after.discloseident,
        update.disclose_ident.isset()   ? update.disclose_ident.get_value()             : info_before.discloseident
    );
    BOOST_CHECK_EQUAL(
        info_after.disclosenotifyemail,
        update.disclose_notify_email.isset()    ? update.disclose_notify_email.get_value()  : info_before.disclosenotifyemail
    );
}

BOOST_FIXTURE_TEST_CASE(update_ok_full_data, has_contact)
{
    const Epp::ContactUpdateInputData data(
        contact.handle,
        "Jan Novak",
        "",
        "Vaclavske namesti 1",
        "53. patro",
        "vpravo",
        "Brno",
        "Morava",
        "20000",
        "CZ",
        "+420 123 456 789",
        "+420 987 654 321",
        "jan@novak.novak",
        "jan.notify@novak.novak",
        "MyVATstring",
        "CZ0123456789",
        Epp::IdentType::identity_card,
        "a6tg85jk57yu97",
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true
    );

    Epp::contact_update_impl(
        ctx,
        data,
        registrar.id,
        42 /* TODO */
    );

    check_equal(contact, data, Fred::InfoContactByHandle(contact.handle).exec(ctx).info_contact_data);
}

BOOST_FIXTURE_TEST_CASE(update_ok_states_are_upgraded, has_contact_with_server_transfer_prohibited_request)
{
    const Epp::ContactUpdateInputData data(
        contact.handle,
        "Jan Novak",
        "",
        "Vaclavske namesti 1",
        "53. patro",
        "vpravo",
        "Brno",
        "Morava",
        "20000",
        "CZ",
        "+420 123 456 789",
        "+420 987 654 321",
        "jan@novak.novak",
        "jan.notify@novak.novak",
        "MyVATstring",
        "CZ0123456789",
        Epp::IdentType::identity_card,
        "a6tg85jk57yu97",
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true
    );

    Epp::contact_update_impl(
        ctx,
        data,
        registrar.id,
        42 /* TODO */
    );

    check_equal(contact, data, Fred::InfoContactByHandle(contact.handle).exec(ctx).info_contact_data);

    /* now object has the state server_transfer_prohibited itself */
    {
        std::vector<std::string> object_states_after;
        {
            BOOST_FOREACH(const Fred::ObjectStateData& state, Fred::GetObjectStates(contact.id).exec(ctx) ) {
                object_states_after.push_back(state.state_name);
            }
        }

        BOOST_CHECK(
            std::find( object_states_after.begin(), object_states_after.end(), status )
            !=
            object_states_after.end()
        );
    }
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
