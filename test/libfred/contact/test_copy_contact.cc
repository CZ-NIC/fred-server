/*
 * Copyright (C) 2013-2019  CZ.NIC, z. s. p. o.
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
#include "libfred/opcontext.hh"
#include "libfred/registrable_object/contact/check_contact.hh"
#include "libfred/registrable_object/contact/copy_contact.hh"
#include "libfred/registrable_object/contact/create_contact.hh"
#include "libfred/registrable_object/contact/delete_contact.hh"
#include "libfred/registrable_object/contact/info_contact.hh"
#include "libfred/registrable_object/contact/info_contact_diff.hh"
#include "libfred/registrable_object/contact/merge_contact.hh"
#include "libfred/registrable_object/contact/update_contact.hh"
#include "util/random/char_set/char_set.hh"
#include "util/random/random.hh"
#include "test/setup/fixtures.hh"

#include <boost/test/unit_test.hpp>

#include <string>

const std::string server_name = "test-copy-contact";

struct copy_contact_fixture : public Test::instantiate_db_template
{
    std::string xmark;
    std::string sys_registrar_handle;
    std::string registrar_handle;
    std::string src_contact_handle;
    std::string dst_contact_handle;

    copy_contact_fixture()
    :   xmark(Random::Generator().get_seq(Random::CharSet::digits(), 6)),
        src_contact_handle(std::string("TEST-COPY-CONTACT-SRC-HANDLE") + xmark),
        dst_contact_handle(std::string("TEST-COPY-CONTACT-DST-HANDLE") + xmark)
    {
        ::LibFred::OperationContextCreator ctx;
        Database::Result registrar_result = ctx.get_conn().exec(
            "SELECT (SELECT handle FROM registrar WHERE system ORDER BY id LIMIT 1),"
                   "(SELECT handle FROM registrar WHERE NOT system ORDER BY id LIMIT 1)");
        sys_registrar_handle = static_cast<std::string>(registrar_result[0][0]);
        registrar_handle = static_cast<std::string>(registrar_result[0][1]);
        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar

        ::LibFred::Contact::PlaceAddress place;
        place.street1 = std::string("STR1") + xmark;
        place.city = "Praha";
        place.postalcode = "11150";
        place.country = "CZ";
        ::LibFred::CreateContact(src_contact_handle, registrar_handle)
            .set_name(std::string("TEST-COPY-CONTACT NAME")+xmark)
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);

        ctx.commit_transaction();
    }
    ~copy_contact_fixture()
    { }
};

BOOST_FIXTURE_TEST_SUITE(TestCopyContact, copy_contact_fixture)

/**
 * test CopyContact
 * ...
 * calls in test shouldn't throw
 */
BOOST_AUTO_TEST_CASE(copy_contact)
{
    ::LibFred::OperationContextCreator ctx;

    const ::LibFred::InfoContactData src_contact_info = ::LibFred::InfoContactByHandle(src_contact_handle).exec(ctx).info_contact_data;
    BOOST_CHECK(src_contact_info.delete_time.isnull());

    ::LibFred::CopyContact(src_contact_handle, dst_contact_handle, sys_registrar_handle, 0).exec(ctx);

    const ::LibFred::InfoContactData dst_contact_info = ::LibFred::InfoContactByHandle(dst_contact_handle).exec(ctx).info_contact_data;
    ctx.commit_transaction();

    BOOST_CHECK(src_contact_info.roid != dst_contact_info.roid);
    BOOST_CHECK(boost::algorithm::to_upper_copy(src_contact_info.handle).compare(boost::algorithm::to_upper_copy(dst_contact_info.handle)) != 0);
    BOOST_CHECK(boost::algorithm::to_upper_copy(src_contact_info.sponsoring_registrar_handle).compare(boost::algorithm::to_upper_copy(dst_contact_info.sponsoring_registrar_handle)) != 0);
    BOOST_CHECK(boost::algorithm::to_upper_copy(src_contact_info.create_registrar_handle).compare(boost::algorithm::to_upper_copy(dst_contact_info.create_registrar_handle)) != 0);
    BOOST_CHECK(src_contact_info.authinfopw == dst_contact_info.authinfopw);
    BOOST_CHECK((src_contact_info.name.isnull() == dst_contact_info.name.isnull()) &&
                (src_contact_info.name.isnull() || (src_contact_info.name.get_value().compare(
                                                    dst_contact_info.name.get_value()) == 0)));
    BOOST_CHECK((src_contact_info.organization.isnull() == dst_contact_info.organization.isnull()) &&
                (src_contact_info.organization.isnull() || (src_contact_info.organization.get_value().compare(
                                                            dst_contact_info.organization.get_value()) == 0)));
    BOOST_CHECK((src_contact_info.place.isnull() == dst_contact_info.place.isnull()) &&
                (src_contact_info.place.isnull() || (src_contact_info.place.get_value() ==
                                                     dst_contact_info.place.get_value())));
    BOOST_CHECK((src_contact_info.telephone.isnull() == dst_contact_info.telephone.isnull()) &&
                (src_contact_info.telephone.isnull() || (src_contact_info.telephone.get_value().compare(
                                                         dst_contact_info.telephone.get_value()) == 0)));
    BOOST_CHECK((src_contact_info.fax.isnull() == dst_contact_info.fax.isnull()) &&
                (src_contact_info.fax.isnull() || (src_contact_info.fax.get_value().compare(
                                                   dst_contact_info.fax.get_value()) == 0)));
    BOOST_CHECK((src_contact_info.email.isnull() == dst_contact_info.email.isnull()) &&
                (src_contact_info.email.isnull() || (src_contact_info.email.get_value().compare(
                                                     dst_contact_info.email.get_value()) == 0)));
    BOOST_CHECK((src_contact_info.notifyemail.isnull() == dst_contact_info.notifyemail.isnull()) &&
                (src_contact_info.notifyemail.isnull() || (src_contact_info.notifyemail.get_value().compare(
                                                           dst_contact_info.notifyemail.get_value()) == 0)));
    BOOST_CHECK((src_contact_info.vat.isnull() == dst_contact_info.vat.isnull()) &&
                (src_contact_info.vat.isnull() || (src_contact_info.vat.get_value().compare(
                                                   dst_contact_info.vat.get_value()) == 0)));
    BOOST_CHECK((src_contact_info.ssntype.isnull() == dst_contact_info.ssntype.isnull()) &&
                (src_contact_info.ssntype.isnull() || (src_contact_info.ssntype.get_value().compare(
                                                       dst_contact_info.ssntype.get_value()) == 0)));
    BOOST_CHECK((src_contact_info.ssn.isnull() == dst_contact_info.ssn.isnull()) &&
                (src_contact_info.ssn.isnull() || (src_contact_info.ssn.get_value().compare(
                                                   dst_contact_info.ssn.get_value()) == 0)));
    BOOST_CHECK(src_contact_info.disclosename == dst_contact_info.disclosename);
    BOOST_CHECK(src_contact_info.discloseorganization == dst_contact_info.discloseorganization);
    BOOST_CHECK(src_contact_info.discloseaddress == dst_contact_info.discloseaddress);
    BOOST_CHECK(src_contact_info.disclosetelephone == dst_contact_info.disclosetelephone);
    BOOST_CHECK(src_contact_info.disclosefax == dst_contact_info.disclosefax);
    BOOST_CHECK(src_contact_info.discloseemail == dst_contact_info.discloseemail);
    BOOST_CHECK(src_contact_info.disclosevat == dst_contact_info.disclosevat);
    BOOST_CHECK(src_contact_info.discloseident == dst_contact_info.discloseident);
    BOOST_CHECK(src_contact_info.disclosenotifyemail == dst_contact_info.disclosenotifyemail);
}

/**
 * test CopyContactBad
 * ...
 * calls in test should throw
 */
BOOST_AUTO_TEST_CASE(copy_contact_bad)
{
    const std::string bad_src_contact_handle = dst_contact_handle;
    try {
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        ::LibFred::CopyContact(bad_src_contact_handle, dst_contact_handle, sys_registrar_handle, 0).exec(ctx);
        ctx.commit_transaction();
        BOOST_CHECK(false);
    }
    catch(const ::LibFred::CopyContact::Exception &ex) {
        BOOST_CHECK(ex.is_set_src_contact_handle_not_found());
        BOOST_CHECK(ex.get_src_contact_handle_not_found().compare(bad_src_contact_handle) == 0);
    }

    const std::string bad_dst_contact_handle = src_contact_handle;
    try {
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        ::LibFred::CopyContact(src_contact_handle, bad_dst_contact_handle, sys_registrar_handle, 0).exec(ctx);
        ctx.commit_transaction();
        BOOST_CHECK(false);
    }
    catch(const ::LibFred::CopyContact::Exception &ex) {
        BOOST_CHECK(ex.is_set_dst_contact_handle_already_exist());
        BOOST_CHECK(ex.get_dst_contact_handle_already_exist().compare(bad_dst_contact_handle) == 0);
    }

    const std::string bad_registrar_handle = std::string("BAD") + sys_registrar_handle + xmark;
    try {
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        ::LibFred::CopyContact(src_contact_handle, dst_contact_handle, bad_registrar_handle, 0).exec(ctx);
        ctx.commit_transaction();
        BOOST_CHECK(false);
    }
    catch(const ::LibFred::CopyContact::Exception &ex) {
        BOOST_CHECK(ex.is_set_create_contact_failed());
    }
}

BOOST_AUTO_TEST_SUITE_END();//TestCopyContact
