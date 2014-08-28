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
#include <fredlib/domain.h>
#include <fredlib/contact.h>
#include <fredlib/nsset.h>
#include <fredlib/keyset.h>
#include "src/fredlib/domain/info_domain_impl.h"

#include "util/random_data_generator.h"
#include "tests/setup/fixtures.h"

const std::string server_name = "test-info-domain";

struct test_domain_fixture : public Test::Fixture::instantiate_db_template
{
    std::string registrar_handle;
    std::string xmark;
    std::string admin_contact_handle;
    std::string admin_contact1_handle;
    std::string admin_contact2_handle;
    std::string registrant_contact_handle;
    std::string test_nsset_handle;
    std::string test_keyset_handle;
    std::string test_fqdn;

    test_domain_fixture()
    :xmark(RandomDataGenerator().xnumstring(6))
    , admin_contact_handle(std::string("TEST-ADMIN-CONTACT-HANDLE")+xmark)
    , admin_contact1_handle(std::string("TEST-ADMIN-CONTACT2-HANDLE")+xmark)
    , admin_contact2_handle(std::string("TEST-ADMIN-CONTACT3-HANDLE")+xmark)
    , registrant_contact_handle(std::string("TEST-REGISTRANT-CONTACT-HANDLE")+xmark)
    , test_nsset_handle(std::string("TEST-NSSET-HANDLE")+xmark)
    , test_keyset_handle(std::string("TEST-KEYSET-HANDLE")+xmark)
    , test_fqdn(std::string("fredinfo")+xmark+".cz")
    {
        namespace ip = boost::asio::ip;
        Fred::OperationContext ctx;
        registrar_handle = static_cast<std::string>(ctx.get_conn().exec(
                "SELECT handle FROM registrar WHERE system = TRUE ORDER BY id LIMIT 1")[0][0]);
        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar

        Fred::CreateContact(admin_contact_handle,registrar_handle)
            .set_name(std::string("TEST-ADMIN-CONTACT NAME")+xmark)
            .set_disclosename(true)
            .set_street1(std::string("STR1")+xmark)
            .set_city("Praha").set_postalcode("11150").set_country("CZ")
            .set_discloseaddress(true)
            .exec(ctx);

        Fred::CreateContact(admin_contact1_handle,registrar_handle)
            .set_name(std::string("TEST-ADMIN-CONTACT2 NAME")+xmark)
            .set_disclosename(true)
            .set_street1(std::string("STR1")+xmark)
            .set_city("Praha").set_postalcode("11150").set_country("CZ")
            .set_discloseaddress(true)
            .exec(ctx);

        Fred::CreateContact(admin_contact2_handle,registrar_handle)
            .set_name(std::string("TEST-ADMIN-CONTACT3 NAME")+xmark)
            .set_disclosename(true)
            .set_street1(std::string("STR1")+xmark)
            .set_city("Praha").set_postalcode("11150").set_country("CZ")
            .set_discloseaddress(true)
            .exec(ctx);

        Fred::CreateContact(registrant_contact_handle,registrar_handle)
            .set_name(std::string("TEST-REGISTRANT-CONTACT NAME")+xmark)
            .set_disclosename(true)
            .set_street1(std::string("STR1")+xmark)
            .set_city("Praha").set_postalcode("11150").set_country("CZ")
            .set_discloseaddress(true)
            .exec(ctx);

        Fred::CreateNsset(test_nsset_handle, registrar_handle)
            .set_tech_contacts(Util::vector_of<std::string>(admin_contact_handle))
            .set_dns_hosts(Util::vector_of<Fred::DnsHost>
            (Fred::DnsHost("a.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("127.0.0.3"))(ip::address::from_string("127.1.1.3")))) //add_dns
            (Fred::DnsHost("b.ns.nic.cz",  Util::vector_of<ip::address>(ip::address::from_string("127.0.0.4"))(ip::address::from_string("127.1.1.4")))) //add_dns
            ).exec(ctx);

        Fred::CreateKeyset(test_keyset_handle, registrar_handle)
        .set_tech_contacts(Util::vector_of<std::string>(admin_contact_handle))
                .exec(ctx);

        Fred::CreateDomain(test_fqdn//const std::string& fqdn
                    , registrar_handle//const std::string& registrar
                    , registrant_contact_handle//const std::string& registrant
                    , Optional<std::string>("testpasswd")//const Optional<std::string>& authinfo
                    , Nullable<std::string>(test_nsset_handle)//const Optional<Nullable<std::string> >& nsset
                    , Nullable<std::string>(test_keyset_handle)//const Optional<Nullable<std::string> >& keyset
                    , Util::vector_of<std::string>(admin_contact2_handle)//const std::vector<std::string>& admin_contacts
                    , boost::gregorian::day_clock::local_day()+boost::gregorian::months(12)//const Optional<boost::gregorian::date>& expiration_date
                    , Optional<boost::gregorian::date>()
                    , Optional<bool>()
                    , 0//const Optional<unsigned long long> logd_request_id
                    ).exec(ctx);

        ctx.commit_transaction();//commit fixture
        {
            Fred::OperationContext ctx1;
            //call update using big ctor
            Fred::UpdateDomain(test_fqdn//fqdn
                    , registrar_handle//registrar
                    , Optional<std::string>()//sponsoring registrar
                    , registrant_contact_handle //registrant - owner
                    , std::string("testauthinfo1") //authinfo
                    , Nullable<std::string>()//unset nsset - set to null
                    , Optional<Nullable<std::string> >()//dont change keyset
                    , Util::vector_of<std::string> (admin_contact1_handle)(registrant_contact_handle) //add admin contacts
                    , Util::vector_of<std::string> (admin_contact2_handle) //remove admin contacts
                    , Optional<boost::gregorian::date>()//exdate
                    , Optional<boost::gregorian::date>()//valexdate
                    , Optional<bool>()
                    , Optional<unsigned long long>() //request_id not set
                    ).exec(ctx1);


            ctx1.commit_transaction();
        }
    }
    ~test_domain_fixture()
    {}
};


BOOST_FIXTURE_TEST_SUITE(TestInfoDomain, test_domain_fixture   )

/**
 * test InfoDomain
 */
BOOST_AUTO_TEST_CASE(info_domain)
{
    Fred::OperationContext ctx;

    Fred::InfoDomainOutput info_data_1 = Fred::InfoDomainByHandle(test_fqdn).exec(ctx);
    Fred::InfoDomainOutput info_data_2 = Fred::InfoDomainByHandle(test_fqdn).exec(ctx);
    BOOST_CHECK(info_data_1 == info_data_2);
    Fred::InfoDomainOutput info_data_3 = Fred::InfoDomainById(info_data_1.info_domain_data.id).exec(ctx);
    BOOST_CHECK(info_data_1 == info_data_3);
    Fred::InfoDomainOutput info_data_4 = Fred::InfoDomainHistory(info_data_1.info_domain_data.roid).exec(ctx).at(0);
    BOOST_CHECK(info_data_1 == info_data_4);
    Fred::InfoDomainOutput info_data_5 = Fred::InfoDomainHistoryById(info_data_1.info_domain_data.id).exec(ctx).at(0);
    BOOST_CHECK(info_data_1 == info_data_5);
    Fred::InfoDomainOutput info_data_6 = Fred::InfoDomainHistoryByHistoryid(info_data_1.info_domain_data.historyid).exec(ctx);
    BOOST_CHECK(info_data_1 == info_data_6);

    //impl
    for( int j = 0; j < (1 << 7); ++j)
    {
        Fred::InfoDomain i;
        if(j & (1 << 0)) i.set_fqdn(info_data_1.info_domain_data.fqdn);
        if(j & (1 << 1)) i.set_roid(info_data_1.info_domain_data.roid);
        if(j & (1 << 2)) i.set_id(info_data_1.info_domain_data.id);
        if(j & (1 << 3)) i.set_historyid(info_data_1.info_domain_data.historyid);
        if(j & (1 << 4)) i.set_lock();
        if(j & (1 << 5)) i.set_history_timestamp(info_data_1.info_domain_data.update_time.get_value());
        if(j & (1 << 6)) i.set_history_query(true);

        std::vector<Fred::InfoDomainOutput> output;
        BOOST_MESSAGE(i.explain_analyze(ctx,output));
        if((j & (1 << 0)) || (j & (1 << 1)) || (j & (1 << 2)) || (j & (1 << 3)))//check if selective
        {
            if((info_data_1 != output.at(0)))
            {
                BOOST_MESSAGE(Fred::diff_domain_data(info_data_1.info_domain_data
                        , output.at(0).info_domain_data).to_string());
            }
            BOOST_CHECK(output.at(0) == info_data_1);
        }
    }

}//info_domain

/**
 * test call InfoDomainDiff
*/
BOOST_AUTO_TEST_CASE(info_domain_diff)
{
    Fred::OperationContext ctx;
    Fred::InfoDomainOutput domain_info1 = Fred::InfoDomainByHandle(test_fqdn).exec(ctx);
    Fred::InfoDomainOutput domain_info2 = Fred::InfoDomainByHandle(test_fqdn).set_lock().exec(ctx);

    Fred::InfoDomainDiff test_diff, test_empty_diff;

    //differing data
    test_diff.crhistoryid = std::make_pair(1ull,2ull);
    test_diff.historyid = std::make_pair(1ull,2ull);
    test_diff.id = std::make_pair(1ull,2ull);
    test_diff.delete_time = std::make_pair(Nullable<boost::posix_time::ptime>()
            ,Nullable<boost::posix_time::ptime>(boost::posix_time::second_clock::local_time()));
    test_diff.fqdn = std::make_pair(std::string("testfqdn1"),std::string("testfqdn2"));
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

    BOOST_MESSAGE(test_diff.to_string());
    BOOST_MESSAGE(test_empty_diff.to_string());

    BOOST_CHECK(!test_diff.is_empty());
    BOOST_CHECK(test_empty_diff.is_empty());

    BOOST_MESSAGE(Fred::diff_domain_data(domain_info1.info_domain_data,domain_info2.info_domain_data).to_string());

    //because of changes to Nullable::operator<<
    BOOST_CHECK(ctx.get_conn().exec_params("select $1::text", Database::query_param_list(Database::QPNull))[0][0].isnull());
    BOOST_CHECK(ctx.get_conn().exec_params("select $1::text", Database::query_param_list(Nullable<std::string>()))[0][0].isnull());

}


BOOST_AUTO_TEST_SUITE_END();//TestInfoDomain
