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

const std::string server_name = "test-info-contact";

struct test_contact_fixture : public Test::Fixture::instantiate_db_template
{
    std::string registrar_handle;
    std::string xmark;
    std::string test_contact_handle;

    test_contact_fixture()
    :xmark(RandomDataGenerator().xnumstring(6))
    , test_contact_handle(std::string("TEST-CONTACT-HANDLE")+xmark)
    {
        Fred::OperationContext ctx;
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

        ctx.commit_transaction();//commit fixture
    }
    ~test_contact_fixture()
    {}
};


BOOST_FIXTURE_TEST_SUITE(TestInfoContact, test_contact_fixture)

/**
 * test call InfoContact
*/
BOOST_AUTO_TEST_CASE(info_contact)
{
    Fred::OperationContext ctx;
    Fred::InfoContactOutput contact_info1 = Fred::InfoContactByHandle(test_contact_handle).exec(ctx);
    Fred::InfoContactOutput contact_info2 = Fred::InfoContactByHandle(test_contact_handle).set_lock().exec(ctx);

    std::vector<Fred::InfoContactOutput> contact_history_info1 = Fred::InfoContactHistory(
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

    //impl
    for( int j = 0; j < (1 << 7); ++j)
    {
        Fred::InfoContact i;
        if(j & (1 << 0)) i.set_handle(contact_info1.info_contact_data.handle);
        if(j & (1 << 1)) i.set_roid(contact_info1.info_contact_data.roid);
        if(j & (1 << 2)) i.set_id(contact_info1.info_contact_data.id);
        if(j & (1 << 3)) i.set_historyid(contact_info1.info_contact_data.historyid);
        if(j & (1 << 4)) i.set_lock();
        if(j & (1 << 5)) i.set_history_timestamp(contact_info1.info_contact_data.creation_time);
        if(j & (1 << 6)) i.set_history_query(true);

        std::vector<Fred::InfoContactOutput> output;
        BOOST_MESSAGE(i.explain_analyze(ctx,output));
        if((j & (1 << 0)) || (j & (1 << 1)) || (j & (1 << 2)) || (j & (1 << 3)))//check if selective
        {
            if((contact_info1 != output.at(0)))
            {
                BOOST_MESSAGE(Fred::diff_contact_data(contact_info1.info_contact_data
                        , output.at(0).info_contact_data).to_string());
            }
            BOOST_CHECK(output.at(0) == contact_info1);
        }
    }

}

/**
 * test call InfoContactDiff
*/
BOOST_AUTO_TEST_CASE(info_contact_diff)
{
    Fred::OperationContext ctx;
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
    test_diff.ssntype = std::make_pair(Nullable<std::string>(),Nullable<std::string>("test2"));
    test_diff.ssn = std::make_pair(Nullable<std::string>(),Nullable<std::string>("test2"));
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

    //because of changes to Nullable::operator<<
    BOOST_CHECK(ctx.get_conn().exec_params("select $1::text", Database::query_param_list(Database::QPNull))[0][0].isnull());
    BOOST_CHECK(ctx.get_conn().exec_params("select $1::text", Database::query_param_list(Nullable<std::string>()))[0][0].isnull());
}


BOOST_AUTO_TEST_SUITE_END();//TestInfoContact
