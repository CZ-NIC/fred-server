/*
 * Copyright (C) 2013  CZ.NIC, z.s.p.o.
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

#include <boost/test/unit_test.hpp>
#include <string>

#include "src/fredlib/opcontext.h"
#include <fredlib/contact.h>
#include "src/fredlib/contact/info_contact_impl.h"
#include "util/random_data_generator.h"
#include "tests/setup/fixtures.h"

namespace boost { namespace test_tools {
    template<> void print_log_value<Fred::Contact::PlaceAddress>::operator()(std::ostream& _stream, const Fred::Contact::PlaceAddress& _address) {
        _stream << "{"
                << _address.street1 << ", "
                << _address.street2 << ", "
                << _address.street3 << ", "
                << _address.city    << ", "
                << _address.stateorprovince << ", "
                << _address.postalcode << ", "
                << _address.country << " "
                << "}";
    }
}}

BOOST_AUTO_TEST_SUITE(TestInfoContact)

const std::string server_name = "test-info-contact";
//unique global name of the fixture
struct test_contact_fixture_6da88b63b0bc46e29f6d0ce3181fd5d8 : public Test::Fixture::instantiate_db_template
{
    std::string registrar_handle;
    std::string xmark;
    std::string test_contact_handle;
    std::string test_contact_history_handle;

    test_contact_fixture_6da88b63b0bc46e29f6d0ce3181fd5d8()
    :xmark(RandomDataGenerator().xnumstring(6))
    , test_contact_handle(std::string("TEST-CONTACT-HANDLE")+xmark)
    , test_contact_history_handle(std::string("TEST-CONTACT-HISTORY-HANDLE")+xmark)
    {
        Fred::OperationContextCreator ctx;
        registrar_handle = static_cast<std::string>(ctx.get_conn().exec(
                "SELECT handle FROM registrar WHERE system = TRUE ORDER BY id LIMIT 1")[0][0]);
        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar

        Fred::Contact::PlaceAddress place;
        place.street1 = std::string("STR1") + xmark;
        place.city = "Praha";
        place.postalcode = "11150";
        place.country = "CZ";
        Fred::CreateContact(test_contact_handle,registrar_handle).set_name(std::string("TEST-CONTACT NAME")+xmark)
            .set_name(std::string("TEST-CONTACT NAME")+xmark)
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);

        Fred::CreateContact(test_contact_history_handle,registrar_handle)
            .set_name(std::string("TEST-CONTACT-HISTORY NAME")+xmark)
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);

        Fred::UpdateContactByHandle(test_contact_history_handle,registrar_handle)
            .set_name(std::string("TEST-CONTACT-HISTORY NAME1")+xmark)
            .exec(ctx);

        ctx.commit_transaction();//commit fixture
    }
    ~test_contact_fixture_6da88b63b0bc46e29f6d0ce3181fd5d8()
    {}
};


/**
 * test call InfoContact
*/
BOOST_FIXTURE_TEST_CASE(info_contact, test_contact_fixture_6da88b63b0bc46e29f6d0ce3181fd5d8)
{
    Fred::OperationContextCreator ctx;
    Fred::InfoContactOutput contact_info1 = Fred::InfoContactByHandle(test_contact_handle).exec(ctx);
    Fred::InfoContactOutput contact_info2 = Fred::InfoContactByHandle(test_contact_handle).set_lock().exec(ctx);

    std::vector<Fred::InfoContactOutput> contact_history_info1 = Fred::InfoContactHistoryByRoid(
        contact_info1.info_contact_data.roid).exec(ctx);

    BOOST_CHECK(contact_info1 == contact_info2);

    BOOST_MESSAGE(std::string("test contact id: ") + boost::lexical_cast<std::string>(contact_history_info1.at(0).info_contact_data.id));

    //wrappers
    std::vector<Fred::InfoContactOutput> contact_history_info2 = Fred::InfoContactHistoryById(contact_history_info1.at(0).info_contact_data.id).exec(ctx);

    BOOST_CHECK(contact_history_info1.at(0) == contact_history_info2.at(0));

    Fred::InfoContactOutput contact_history_info3 = Fred::InfoContactHistoryByHistoryid(contact_history_info1.at(0).info_contact_data.historyid).exec(ctx);

    BOOST_CHECK(contact_history_info1.at(0) == contact_history_info3);

    Fred::InfoContactOutput contact_history_info4 = Fred::InfoContactById(contact_history_info1.at(0).info_contact_data.id).exec(ctx);

    BOOST_CHECK(contact_history_info1.at(0) == contact_history_info4);

    Fred::InfoContactOutput contact_history_info5 = Fred::InfoContactByHandle(contact_history_info1.at(0).info_contact_data.handle).exec(ctx);

    BOOST_CHECK(contact_history_info1.at(0) == contact_history_info5);

}

BOOST_FIXTURE_TEST_CASE(test_info_contact_output_timestamp, test_contact_fixture_6da88b63b0bc46e29f6d0ce3181fd5d8)
{
    const std::string timezone = "Europe/Prague";
    Fred::OperationContext ctx;
    const Fred::InfoContactOutput contact_output_by_handle              = Fred::InfoContactByHandle(test_contact_handle).exec(ctx, timezone);
    const Fred::InfoContactOutput contact_output_by_id                  = Fred::InfoContactById(contact_output_by_handle.info_contact_data.id).exec(ctx, timezone);
    const Fred::InfoContactOutput contact_output_history_by_historyid   = Fred::InfoContactHistoryByHistoryid(contact_output_by_handle.info_contact_data.historyid).exec(ctx, timezone);
    const Fred::InfoContactOutput contact_output_history_by_id          = Fred::InfoContactHistoryById(contact_output_by_handle.info_contact_data.id).exec(ctx, timezone).at(0);
    const Fred::InfoContactOutput contact_output_history_by_roid        = Fred::InfoContactHistoryByRoid(contact_output_by_handle.info_contact_data.roid).exec(ctx, timezone).at(0);

    /* all are equal amongst themselves ... */
    BOOST_CHECK(
            contact_output_by_handle.utc_timestamp              == contact_output_by_id.utc_timestamp
        &&  contact_output_by_id.utc_timestamp                  == contact_output_history_by_historyid.utc_timestamp
        &&  contact_output_history_by_historyid.utc_timestamp   == contact_output_history_by_id.utc_timestamp
        &&  contact_output_history_by_id.utc_timestamp          == contact_output_history_by_roid.utc_timestamp
    );

    /* ... and one of them is equal to correct constant value */
    BOOST_CHECK_EQUAL(
        contact_output_by_handle.utc_timestamp,
        boost::posix_time::time_from_string( static_cast<std::string>( ctx.get_conn().exec("SELECT now()::timestamp")[0][0] ) )
    );
}

/**
 * test call InfoContactDiff
*/
BOOST_FIXTURE_TEST_CASE(info_contact_diff, test_contact_fixture_6da88b63b0bc46e29f6d0ce3181fd5d8)
{
    Fred::OperationContextCreator ctx;
    Fred::InfoContactOutput contact_info1 = Fred::InfoContactByHandle(test_contact_handle).exec(ctx);
    Fred::InfoContactOutput contact_info2 = Fred::InfoContactByHandle(test_contact_handle).set_lock().exec(ctx);

    Fred::InfoContactDiff test_diff, test_empty_diff;

    //differing data
    test_diff.crhistoryid = std::make_pair(1ull,2ull);
    test_diff.historyid = std::make_pair(1ull,2ull);
    test_diff.id = std::make_pair(1ull,2ull);
    test_diff.delete_time = std::make_pair(Nullable<boost::posix_time::ptime>()
            ,Nullable<boost::posix_time::ptime>(boost::posix_time::second_clock::local_time()));
    test_diff.handle = std::make_pair(std::string("testhandle1"),std::string("testhandle2"));
    test_diff.roid = std::make_pair(std::string("testroid1"),std::string("testroid2"));
    test_diff.sponsoring_registrar_handle = std::make_pair(std::string("testspreg1"),std::string("testspreg2"));
    test_diff.create_registrar_handle = std::make_pair(std::string("testcrreg1"),std::string("testcrreg2"));
    test_diff.update_registrar_handle = std::make_pair(Nullable<std::string>("testcrreg1"),Nullable<std::string>());
    test_diff.creation_time = std::make_pair(boost::posix_time::ptime(),boost::posix_time::second_clock::local_time());
    test_diff.update_time = std::make_pair(Nullable<boost::posix_time::ptime>()
            ,Nullable<boost::posix_time::ptime>(boost::posix_time::second_clock::local_time()));
    test_diff.transfer_time = std::make_pair(Nullable<boost::posix_time::ptime>()
                ,Nullable<boost::posix_time::ptime>(boost::posix_time::second_clock::local_time()));
    test_diff.authinfopw = std::make_pair(std::string("testpass1"),std::string("testpass2"));
    test_diff.name = std::make_pair(Nullable<std::string>(),Nullable<std::string>("testname2"));
    test_diff.organization = std::make_pair(Nullable<std::string>(),Nullable<std::string>("test2"));
    Fred::Contact::PlaceAddress place;
    place.street1 = "test2";
    place.street2 = std::string("test2");
    place.street3 = std::string("test2");
    place.city = "test2";
    place.stateorprovince = std::string("test2");
    place.postalcode = "test2";
    place.country = "test2";
    test_diff.place = std::make_pair(Nullable< Fred::Contact::PlaceAddress >(), Nullable< Fred::Contact::PlaceAddress >(place));
    test_diff.telephone = std::make_pair(Nullable<std::string>(),Nullable<std::string>("test2"));
    test_diff.fax = std::make_pair(Nullable<std::string>(),Nullable<std::string>("test2"));
    test_diff.email = std::make_pair(Nullable<std::string>(),Nullable<std::string>("test2"));
    test_diff.notifyemail = std::make_pair(Nullable<std::string>(),Nullable<std::string>("test2"));
    test_diff.vat = std::make_pair(Nullable<std::string>(),Nullable<std::string>("test2"));
    test_diff.personal_id = std::make_pair(Nullable< Fred::InfoContactDiff::PersonalId >(),
                                           Nullable< Fred::InfoContactDiff::PersonalId >(
                                               Fred::InfoContactDiff::PersonalId("test2", "test2")));
    test_diff.disclosename= std::make_pair(false, true);
    test_diff.discloseorganization= std::make_pair(true, false);
    test_diff.discloseaddress= std::make_pair(false, true);
    test_diff.disclosetelephone= std::make_pair(true, false);
    test_diff.disclosefax= std::make_pair(false, true);
    test_diff.discloseemail= std::make_pair(true, false);
    test_diff.disclosevat= std::make_pair(false, true);
    test_diff.discloseident= std::make_pair(true, false);
    test_diff.disclosenotifyemail= std::make_pair(false, true);

    BOOST_MESSAGE(test_diff.to_string());
    BOOST_MESSAGE(test_empty_diff.to_string());

    BOOST_CHECK(!test_diff.is_empty());
    BOOST_CHECK(test_empty_diff.is_empty());

    BOOST_MESSAGE(Fred::diff_contact_data(contact_info1.info_contact_data,contact_info2.info_contact_data).to_string());
}

/**
 * test InfoContactHistoryByRoid output data sorted by historyid in descending order (current data first, older next)
*/

BOOST_FIXTURE_TEST_CASE(info_contact_history_order, test_contact_fixture_6da88b63b0bc46e29f6d0ce3181fd5d8)
{
    Fred::OperationContextCreator ctx;
    Fred::InfoContactOutput contact_history_info = Fred::InfoContactByHandle(test_contact_history_handle).exec(ctx);

    std::vector<Fred::InfoContactOutput> contact_history_info_by_roid = Fred::InfoContactHistoryByRoid(contact_history_info.info_contact_data.roid).exec(ctx);
    BOOST_CHECK(contact_history_info_by_roid.size() == 2);
    BOOST_CHECK(contact_history_info_by_roid.at(0).info_contact_data.historyid > contact_history_info_by_roid.at(1).info_contact_data.historyid);

    BOOST_CHECK(contact_history_info_by_roid.at(0).info_contact_data.name.get_value() == std::string("TEST-CONTACT-HISTORY NAME1")+xmark);
    BOOST_CHECK(contact_history_info_by_roid.at(1).info_contact_data.name.get_value() == std::string("TEST-CONTACT-HISTORY NAME")+xmark);

    std::vector<Fred::InfoContactOutput> contact_history_info_by_id = Fred::InfoContactHistoryById(contact_history_info.info_contact_data.id).exec(ctx);
    BOOST_CHECK(contact_history_info_by_id.size() == 2);
    BOOST_CHECK(contact_history_info_by_id.at(0).info_contact_data.historyid > contact_history_info_by_id.at(1).info_contact_data.historyid);

    BOOST_CHECK(contact_history_info_by_id.at(0).info_contact_data.name.get_value() == std::string("TEST-CONTACT-HISTORY NAME1")+xmark);
    BOOST_CHECK(contact_history_info_by_id.at(1).info_contact_data.name.get_value() == std::string("TEST-CONTACT-HISTORY NAME")+xmark);
}

BOOST_AUTO_TEST_CASE(test_ContactAddressType) {
    const Fred::ContactAddressType::Value mailing = Fred::ContactAddressType::MAILING;
    const Fred::ContactAddressType::Value shipping = Fred::ContactAddressType::SHIPPING;

    // using built-in comparison operators of enum, std::string...
    {
        // ctor
        const Fred::ContactAddressType type_ctor(mailing);
        BOOST_CHECK_EQUAL(type_ctor.value, mailing);

        // copy ctor
        const Fred::ContactAddressType type_copy(type_ctor);
        BOOST_CHECK_EQUAL(type_copy.value, mailing);

        // copy assignment
        Fred::ContactAddressType type_copy_assign(mailing);
        type_copy_assign = shipping;
        BOOST_CHECK_EQUAL(type_copy_assign.value, shipping);

        // assignment
        Fred::ContactAddressType type_assign(shipping);
        type_assign = type_ctor;
        BOOST_CHECK_EQUAL(type_assign.value, mailing);

        // to_string
        BOOST_CHECK_EQUAL(type_ctor.to_string(), Fred::ContactAddressType::to_string(mailing) );
        BOOST_CHECK_EQUAL(type_ctor.to_string(), "MAILING" );

        // from_string
        Fred::ContactAddressType set_value(mailing);
        set_value.set_value("MAILING");
        BOOST_CHECK_EQUAL(set_value.value, Fred::ContactAddressType::from_string("MAILING"));
        BOOST_CHECK_EQUAL(set_value.value, Fred::ContactAddressType::MAILING);
    }

    // custom comparison operators
    {
        BOOST_CHECK( mailing == mailing );
        BOOST_CHECK( !(mailing == shipping) );

        BOOST_CHECK( mailing != shipping );
        BOOST_CHECK( !( mailing != mailing ) );

        BOOST_CHECK( mailing < shipping );
        BOOST_CHECK( !( mailing < mailing ) );
    }
}


/*
 * also for regression #14102 (bug in PlaceAddress ctor init list)
 */
BOOST_AUTO_TEST_CASE(test_PlaceAddress) {
    const std::string           street1             = "sdfsfsf 345";
    const Optional<std::string> street2             = "Tertq 74374";
    const Optional<std::string> street3             = "jioiutqrej 0128";
    const std::string           city                = "iouronhf";
    const Optional<std::string> stateorprovince     = "hiufaaoh";
    const std::string           postalcode          = "89745";
    const std::string           country             = "CZ";

    const Fred::Contact::PlaceAddress ctor(
        street1,
        street2,
        street3,
        city,
        stateorprovince,
        postalcode,
        country
    );

    BOOST_CHECK_EQUAL(ctor.street1,           street1);
    BOOST_CHECK_EQUAL(ctor.street2,           street2);
    BOOST_CHECK_EQUAL(ctor.street3,           street3);
    BOOST_CHECK_EQUAL(ctor.city,              city);
    BOOST_CHECK_EQUAL(ctor.stateorprovince,   stateorprovince);
    BOOST_CHECK_EQUAL(ctor.postalcode,        postalcode);
    BOOST_CHECK_EQUAL(ctor.country,           country);

    Fred::Contact::PlaceAddress assign;

    assign.street1 =           street1;
    assign.street2 =           street2;
    assign.street3 =           street3;
    assign.city =              city;
    assign.stateorprovince =   stateorprovince;
    assign.postalcode =        postalcode;
    assign.country =           country;

    BOOST_CHECK_EQUAL(assign.street1,           street1);
    BOOST_CHECK_EQUAL(assign.street2,           street2);
    BOOST_CHECK_EQUAL(assign.street3,           street3);
    BOOST_CHECK_EQUAL(assign.city,              city);
    BOOST_CHECK_EQUAL(assign.stateorprovince,   stateorprovince);
    BOOST_CHECK_EQUAL(assign.postalcode,        postalcode);
    BOOST_CHECK_EQUAL(assign.country,           country);

    BOOST_CHECK_EQUAL(ctor, assign);
}

BOOST_AUTO_TEST_CASE(test_ContactAddress) {
    const Optional<std::string> company_name    = "ffsfsgdigofleh";
    const std::string           street1         = "sdfsfsf 345";
    const Optional<std::string> street2         = "Tertq 74374";
    const Optional<std::string> street3         = "jioiutqrej 0128";
    const std::string           city            = "iouronhf";
    const Optional<std::string> stateorprovince = "hiufaaoh";
    const std::string           postalcode      = "89745";
    const std::string           country         = "CZ";

    const Fred::ContactAddress ctor_atomics(
        company_name,
        street1,
        street2,
        street3,
        city,
        stateorprovince,
        postalcode,
        country
    );

    BOOST_CHECK_EQUAL(ctor_atomics.company_name,      company_name);
    BOOST_CHECK_EQUAL(ctor_atomics.street1,           street1);
    BOOST_CHECK_EQUAL(ctor_atomics.street2,           street2);
    BOOST_CHECK_EQUAL(ctor_atomics.street3,           street3);
    BOOST_CHECK_EQUAL(ctor_atomics.city,              city);
    BOOST_CHECK_EQUAL(ctor_atomics.stateorprovince,   stateorprovince);
    BOOST_CHECK_EQUAL(ctor_atomics.postalcode,        postalcode);
    BOOST_CHECK_EQUAL(ctor_atomics.country,           country);


    const Fred::ContactAddress ctor_delegated(
        company_name,
        Fred::Contact::PlaceAddress(
            street1,
            street2,
            street3,
            city,
            stateorprovince,
            postalcode,
            country
        )
    );

    BOOST_CHECK_EQUAL(ctor_delegated.company_name,      company_name);
    BOOST_CHECK_EQUAL(ctor_delegated.street1,           street1);
    BOOST_CHECK_EQUAL(ctor_delegated.street2,           street2);
    BOOST_CHECK_EQUAL(ctor_delegated.street3,           street3);
    BOOST_CHECK_EQUAL(ctor_delegated.city,              city);
    BOOST_CHECK_EQUAL(ctor_delegated.stateorprovince,   stateorprovince);
    BOOST_CHECK_EQUAL(ctor_delegated.postalcode,        postalcode);
    BOOST_CHECK_EQUAL(ctor_delegated.country,           country);

    Fred::ContactAddress assign;

    assign.company_name =      company_name;
    assign.street1 =           street1;
    assign.street2 =           street2;
    assign.street3 =           street3;
    assign.city =              city;
    assign.stateorprovince =   stateorprovince;
    assign.postalcode =        postalcode;
    assign.country =           country;

    BOOST_CHECK_EQUAL(assign.company_name,      company_name);
    BOOST_CHECK_EQUAL(assign.street1,           street1);
    BOOST_CHECK_EQUAL(assign.street2,           street2);
    BOOST_CHECK_EQUAL(assign.street3,           street3);
    BOOST_CHECK_EQUAL(assign.city,              city);
    BOOST_CHECK_EQUAL(assign.stateorprovince,   stateorprovince);
    BOOST_CHECK_EQUAL(assign.postalcode,        postalcode);
    BOOST_CHECK_EQUAL(assign.country,           country);

    BOOST_CHECK_EQUAL(ctor_atomics, assign);
    BOOST_CHECK_EQUAL(assign, ctor_atomics);
    BOOST_CHECK_EQUAL(ctor_delegated, assign);
    BOOST_CHECK_EQUAL(assign, ctor_delegated);
    BOOST_CHECK_EQUAL(ctor_atomics, ctor_delegated);
    BOOST_CHECK_EQUAL(ctor_delegated, ctor_atomics);
}

BOOST_AUTO_TEST_CASE(test_Address) {
    const Optional<std::string> name                = "fas fdjao juhf";
    const Optional<std::string> organization        = "dfadflaisfjlhfi";
    const Optional<std::string> company_name        = "ffsfsgdigofleh";
    const std::string           street1             = "sdfsfsf 345";
    const Optional<std::string> street2             = "Tertq 74374";
    const Optional<std::string> street3             = "jioiutqrej 0128";
    const std::string           city                = "iouronhf";
    const Optional<std::string> stateorprovince     = "hiufaaoh";
    const std::string           postalcode          = "89745";
    const std::string           country             = "CZ";

    const Fred::InfoContactData::Address ctor_atomics(
        name,
        organization,
        company_name,
        street1,
        street2,
        street3,
        city,
        stateorprovince,
        postalcode,
        country
    );

    BOOST_CHECK_EQUAL(ctor_atomics.name,              name);
    BOOST_CHECK_EQUAL(ctor_atomics.organization,      organization);
    BOOST_CHECK_EQUAL(ctor_atomics.company_name,      company_name);
    BOOST_CHECK_EQUAL(ctor_atomics.street1,           street1);
    BOOST_CHECK_EQUAL(ctor_atomics.street2,           street2);
    BOOST_CHECK_EQUAL(ctor_atomics.street3,           street3);
    BOOST_CHECK_EQUAL(ctor_atomics.city,              city);
    BOOST_CHECK_EQUAL(ctor_atomics.stateorprovince,   stateorprovince);
    BOOST_CHECK_EQUAL(ctor_atomics.postalcode,        postalcode);
    BOOST_CHECK_EQUAL(ctor_atomics.country,           country);

    const Fred::InfoContactData::Address ctor_delegated(
        name,
        organization,
        Fred::ContactAddress(
            company_name,
            Fred::Contact::PlaceAddress(
                street1,
                street2,
                street3,
                city,
                stateorprovince,
                postalcode,
                country
            )
        )
    );

    BOOST_CHECK_EQUAL(ctor_delegated.name,              name);
    BOOST_CHECK_EQUAL(ctor_delegated.organization,      organization);
    BOOST_CHECK_EQUAL(ctor_delegated.company_name,      company_name);
    BOOST_CHECK_EQUAL(ctor_delegated.street1,           street1);
    BOOST_CHECK_EQUAL(ctor_delegated.street2,           street2);
    BOOST_CHECK_EQUAL(ctor_delegated.street3,           street3);
    BOOST_CHECK_EQUAL(ctor_delegated.city,              city);
    BOOST_CHECK_EQUAL(ctor_delegated.stateorprovince,   stateorprovince);
    BOOST_CHECK_EQUAL(ctor_delegated.postalcode,        postalcode);
    BOOST_CHECK_EQUAL(ctor_delegated.country,           country);

    Fred::InfoContactData::Address assign;

    assign.name =              name;
    assign.organization =      organization;
    assign.company_name =      company_name;
    assign.street1 =           street1;
    assign.street2 =           street2;
    assign.street3 =           street3;
    assign.city =              city;
    assign.stateorprovince =   stateorprovince;
    assign.postalcode =        postalcode;
    assign.country =           country;

    BOOST_CHECK_EQUAL(assign.name,              name);
    BOOST_CHECK_EQUAL(assign.organization,      organization);
    BOOST_CHECK_EQUAL(assign.company_name,      company_name);
    BOOST_CHECK_EQUAL(assign.street1,           street1);
    BOOST_CHECK_EQUAL(assign.street2,           street2);
    BOOST_CHECK_EQUAL(assign.street3,           street3);
    BOOST_CHECK_EQUAL(assign.city,              city);
    BOOST_CHECK_EQUAL(assign.stateorprovince,   stateorprovince);
    BOOST_CHECK_EQUAL(assign.postalcode,        postalcode);
    BOOST_CHECK_EQUAL(assign.country,           country);

    BOOST_CHECK_EQUAL(ctor_atomics, assign);
    BOOST_CHECK_EQUAL(assign, ctor_atomics);
    BOOST_CHECK_EQUAL(ctor_delegated, assign);
    BOOST_CHECK_EQUAL(assign, ctor_delegated);
    BOOST_CHECK_EQUAL(ctor_atomics, ctor_delegated);
    BOOST_CHECK_EQUAL(ctor_delegated, ctor_atomics);
}

BOOST_AUTO_TEST_SUITE_END();//TestInfoContact
