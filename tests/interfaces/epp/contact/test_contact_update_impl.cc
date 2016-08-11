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
#include "src/epp/contact/contact_update_impl.h"

#include <set>
#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/assign/list_of.hpp>

namespace {

Epp::ContactDisclose get_all_items(bool to_disclose = true)
{
    Epp::ContactDisclose disclose(to_disclose ? Epp::ContactDisclose::Flag::disclose
                                              : Epp::ContactDisclose::Flag::hide);
    disclose.add< Epp::ContactDisclose::Item::name >();
    disclose.add< Epp::ContactDisclose::Item::organization >();
    disclose.add< Epp::ContactDisclose::Item::address >();
    disclose.add< Epp::ContactDisclose::Item::telephone >();
    disclose.add< Epp::ContactDisclose::Item::fax >();
    disclose.add< Epp::ContactDisclose::Item::email >();
    disclose.add< Epp::ContactDisclose::Item::vat >();
    disclose.add< Epp::ContactDisclose::Item::ident >();
    disclose.add< Epp::ContactDisclose::Item::notify_email >();
    return disclose;
}

template < Epp::ContactDisclose::Item::Enum ITEM >
bool updated(const Epp::ContactChange &_update, bool _before)
{
    return _update.disclose.is_initialized()
           ? _update.disclose->should_be_disclosed< ITEM >(Epp::is_the_default_policy_to_disclose())
           : _before;
}

void set_correct_data(Epp::ContactChange &data)
{
    data.name              = "Jan Novak";
    data.organization      = "Firma, a. s.";
    data.streets.clear();
    data.streets.reserve(3);
    data.streets.push_back(Nullable< std::string >("Vaclavske namesti 1"));
    data.streets.push_back(Nullable< std::string >("53. patro"));
    data.streets.push_back(Nullable< std::string >("vpravo"));
    data.city              = "Brno";
    data.state_or_province = "Morava";
    data.postal_code       = "20000";
    data.country_code      = "CZ";
}

void set_correct_data_2(Epp::ContactChange &data)
{
    set_correct_data(data);
    data.organization = "";
    data.telephone    = "+420 123 456 789";
    data.fax          = "+420 987 654 321";
    data.email        = "jan@novak.novak";
    data.notify_email = "jan.notify@novak.novak";
    data.vat          = "MyVATstring";
    data.ident        = "CZ0123456789";
    data.ident_type   = Epp::ContactChange::IdentType::op;
    data.auth_info_pw = "a6tg85jk57yu97";
    data.disclose     = get_all_items();
}

std::string get_new_value(const boost::optional< Nullable< std::string > > &change,
                          const std::string &before)
{
    if (Epp::ContactChange::does_value_mean< Epp::ContactChange::Value::to_set >(change)) {
        return Epp::ContactChange::get_value(change);
    }
    if (Epp::ContactChange::does_value_mean< Epp::ContactChange::Value::to_delete >(change)) {
        return std::string();
    }
    return before;
}

Optional< std::string > get_new_value(const boost::optional< Nullable< std::string > > &change,
                                      const Optional< std::string > &before)
{
    if (Epp::ContactChange::does_value_mean< Epp::ContactChange::Value::to_set >(change)) {
        return Epp::ContactChange::get_value(change);
    }
    if (Epp::ContactChange::does_value_mean< Epp::ContactChange::Value::to_delete >(change)) {
        return Optional< std::string >();
    }
    return before;
}

Nullable< std::string > get_new_value(const boost::optional< Nullable< std::string > > &change,
                                      const Nullable< std::string > &before)
{
    if (Epp::ContactChange::does_value_mean< Epp::ContactChange::Value::to_set >(change)) {
        return Epp::ContactChange::get_value(change);
    }
    if (Epp::ContactChange::does_value_mean< Epp::ContactChange::Value::to_delete >(change)) {
        return Nullable< std::string >();
    }
    return before;
}

std::string ident_type_to_string(Epp::ContactChange::IdentType::Enum type)
{
    switch (type)
    {
        case Epp::ContactChange::IdentType::op:       return Fred::PersonalIdUnion::get_OP("").get_type();
        case Epp::ContactChange::IdentType::pass:     return Fred::PersonalIdUnion::get_PASS("").get_type();
        case Epp::ContactChange::IdentType::ico:      return Fred::PersonalIdUnion::get_ICO("").get_type();
        case Epp::ContactChange::IdentType::mpsv:     return Fred::PersonalIdUnion::get_MPSV("").get_type();
        case Epp::ContactChange::IdentType::birthday: return Fred::PersonalIdUnion::get_BIRTHDAY("").get_type();
    }
    throw std::runtime_error("Invalid Epp::ContactChange::IdentType::Enum value.");
}

}//namespace {anonymous}

BOOST_AUTO_TEST_SUITE(TestEpp)
BOOST_AUTO_TEST_SUITE(ContactUpdateImpl)

BOOST_FIXTURE_TEST_CASE(update_invalid_registrar_id, has_contact)
{
    Epp::ContactChange data;
    set_correct_data(data);

    BOOST_CHECK_THROW(
        Epp::contact_update_impl(
            ctx,
            contact.handle + "*?!",
            data,
            0,  /* <== !!! */
            42 /* TODO */
        ),
        Epp::AuthErrorServerClosingConnection
    );
}

BOOST_FIXTURE_TEST_CASE(update_fail_nonexistent_handle, has_contact)
{
    Epp::ContactChange data;
    set_correct_data(data);

    BOOST_CHECK_THROW(
        Epp::contact_update_impl(
            ctx,
            contact.handle + "abc",
            data,
            registrar.id,
            42 /* TODO */
        ),
        Epp::NonexistentHandle
    );
}

BOOST_FIXTURE_TEST_CASE(update_fail_wrong_registrar, has_contact_and_a_different_registrar)
{
    Epp::ContactChange data;
    set_correct_data(data);

    BOOST_CHECK_THROW(
        Epp::contact_update_impl(
            ctx,
            contact.handle,
            data,
            the_different_registrar.id,
            42 /* TODO */
        ),
        Epp::AuthorizationError
    );
}

BOOST_FIXTURE_TEST_CASE(update_fail_prohibiting_status1, has_contact_with_server_update_prohibited)
{
    Epp::ContactChange data;
    set_correct_data(data);

    BOOST_CHECK_THROW(
        Epp::contact_update_impl(
            ctx,
            contact.handle,
            data,
            registrar.id,
            42 /* TODO */
        ),
        Epp::ObjectStatusProhibitsOperation
    );
}

BOOST_FIXTURE_TEST_CASE(update_fail_prohibiting_status2, has_contact_with_delete_candidate)
{
    Epp::ContactChange data;
    set_correct_data(data);

    BOOST_CHECK_THROW(
        Epp::contact_update_impl(
            ctx,
            contact.handle,
            data,
            registrar.id,
            42 /* TODO */
        ),
        Epp::ObjectStatusProhibitsOperation
    );
}

BOOST_FIXTURE_TEST_CASE(update_fail_prohibiting_status_request, has_contact_with_delete_candidate_request)
{
    Epp::ContactChange data;
    set_correct_data_2(data);

    BOOST_CHECK_THROW(
        Epp::contact_update_impl(
            ctx,
            contact.handle,
            data,
            registrar.id,
            42 /* TODO */
        ),
        Epp::ObjectStatusProhibitsOperation
    );

    /* now object has the state deleteCandidate itself */
    {
        std::set< std::string > object_states_after;
        {
            BOOST_FOREACH(const Fred::ObjectStateData &state, Fred::GetObjectStates(contact.id).exec(ctx))
            {
                object_states_after.insert(state.state_name);
            }
        }

        BOOST_CHECK(object_states_after.find(status) != object_states_after.end());
    }
}

BOOST_FIXTURE_TEST_CASE(update_fail_nonexistent_country_code, has_contact)
{
    Epp::ContactChange data;
    set_correct_data(data);
    data.organization = "Spol s r. o.";
    data.country_code = "X123Z"; /* <- !!! */
    data.disclose     = get_all_items();

    try {
        try {
            Epp::contact_update_impl(
                ctx,
                contact.handle,
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
    Epp::ContactChange data;
    set_correct_data(data);
    data.organization = "";
    data.disclose  = get_all_items(false);  /* address <- !!! */

    BOOST_CHECK_THROW(
        Epp::contact_update_impl(
            ctx,
            contact.handle,
            data,
            registrar.id,
            42 /* TODO */
        ),
        Epp::ObjectStatusProhibitsOperation
    );
}

static void check_equal(
    const Fred::InfoContactData& info_before,
    const Epp::ContactChange& update,
    const Fred::InfoContactData& info_after)
{
    BOOST_CHECK_EQUAL(boost::to_upper_copy(info_after.handle), info_before.handle);
    BOOST_CHECK_EQUAL(info_after.name, get_new_value(update.name, info_before.name));
    BOOST_CHECK_EQUAL(info_after.organization, get_new_value(update.organization, info_before.organization));
    BOOST_CHECK_EQUAL(info_after.place.get_value_or_default().street1,
        0 < update.streets.size() ? get_new_value(update.streets[0], info_before.place.get_value_or_default().street1)
                                  : "");
    BOOST_CHECK_EQUAL(info_after.place.get_value_or_default().street2,
        1 < update.streets.size() ? get_new_value(update.streets[1], info_before.place.get_value_or_default().street2)
                                  : "");
    BOOST_CHECK_EQUAL(info_after.place.get_value_or_default().street3,
        2 < update.streets.size() ? get_new_value(update.streets[2], info_before.place.get_value_or_default().street3)
                                  : "");
    BOOST_CHECK_EQUAL(info_after.place.get_value_or_default().city,
                      get_new_value(update.city, info_before.place.get_value_or_default().city));
    BOOST_CHECK_EQUAL(info_after.place.get_value_or_default().postalcode,
                      get_new_value(update.postal_code, info_before.place.get_value_or_default().postalcode));
    BOOST_CHECK_EQUAL(info_after.place.get_value_or_default().stateorprovince,
                      get_new_value(update.state_or_province, info_before.place.get_value_or_default().stateorprovince));
    BOOST_CHECK_EQUAL(info_after.place.get_value_or_default().country,
                      get_new_value(update.country_code, info_before.place.get_value_or_default().country));
    BOOST_CHECK_EQUAL(info_after.telephone, get_new_value(update.telephone, info_before.telephone));
    BOOST_CHECK_EQUAL(info_after.fax, get_new_value(update.fax, info_before.fax));
    BOOST_CHECK_EQUAL(info_after.email, get_new_value(update.email, info_before.email));
    BOOST_CHECK_EQUAL(info_after.notifyemail, get_new_value(update.notify_email, info_before.notifyemail));
    BOOST_CHECK_EQUAL(info_after.vat, get_new_value(update.vat, info_before.vat));
    BOOST_CHECK_EQUAL(info_after.ssn, get_new_value(update.ident, info_before.ssn));
    BOOST_CHECK_EQUAL(info_after.ssntype,
                      !update.ident_type.isnull() ? ident_type_to_string(update.ident_type.get_value())
                                                  : info_before.ssntype);
    BOOST_CHECK_EQUAL(info_after.authinfopw, get_new_value(update.auth_info_pw, info_before.authinfopw));
    BOOST_CHECK_EQUAL(info_after.disclosename,
                      updated< Epp::ContactDisclose::Item::name         >(update, info_before.disclosename));
    BOOST_CHECK_EQUAL(info_after.discloseorganization,
                      updated< Epp::ContactDisclose::Item::organization >(update, info_before.discloseorganization));
    BOOST_CHECK_EQUAL(info_after.discloseaddress,
                      updated< Epp::ContactDisclose::Item::address      >(update, info_before.discloseaddress));
    BOOST_CHECK_EQUAL(info_after.disclosetelephone,
                      updated< Epp::ContactDisclose::Item::telephone    >(update, info_before.disclosetelephone));
    BOOST_CHECK_EQUAL(info_after.disclosefax,
                      updated< Epp::ContactDisclose::Item::fax          >(update, info_before.disclosefax));
    BOOST_CHECK_EQUAL(info_after.discloseemail,
                      updated< Epp::ContactDisclose::Item::email        >(update, info_before.discloseemail));
    BOOST_CHECK_EQUAL(info_after.disclosevat,
                      updated< Epp::ContactDisclose::Item::vat          >(update, info_before.disclosevat));
    BOOST_CHECK_EQUAL(info_after.discloseident,
                      updated< Epp::ContactDisclose::Item::ident        >(update, info_before.discloseident));
    BOOST_CHECK_EQUAL(info_after.disclosenotifyemail,
                      updated< Epp::ContactDisclose::Item::notify_email >(update, info_before.disclosenotifyemail));
}

BOOST_FIXTURE_TEST_CASE(update_ok_full_data, has_contact)
{
    Epp::ContactChange data;
    set_correct_data_2(data);

    Epp::contact_update_impl(
        ctx,
        contact.handle,
        data,
        registrar.id,
        42 /* TODO */
    );

    check_equal(contact, data, Fred::InfoContactByHandle(contact.handle).exec(ctx).info_contact_data);
}

BOOST_FIXTURE_TEST_CASE(update_ok_states_are_upgraded, has_contact_with_server_transfer_prohibited_request)
{
    Epp::ContactChange data;
    set_correct_data_2(data);

    Epp::contact_update_impl(
        ctx,
        contact.handle,
        data,
        registrar.id,
        42 /* TODO */
    );

    check_equal(contact, data, Fred::InfoContactByHandle(contact.handle).exec(ctx).info_contact_data);

    /* now object has the state server_transfer_prohibited itself */
    {
        std::set< std::string > object_states_after;
        {
            BOOST_FOREACH(const Fred::ObjectStateData &state, Fred::GetObjectStates(contact.id).exec(ctx))
            {
                object_states_after.insert(state.state_name);
            }
        }

        BOOST_CHECK(object_states_after.find(status) != object_states_after.end());
    }
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
