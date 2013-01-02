/*
 * Copyright (C) 2012  CZ.NIC, z.s.p.o.
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

#include <memory>
#include <iostream>
#include <string>
#include <algorithm>
#include <functional>
#include <numeric>
#include <map>
#include <exception>
#include <queue>
#include <sys/time.h>
#include <time.h>
#include <string.h>

#include <boost/function.hpp>
#include <boost/format.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time.hpp>
#include <boost/assign/list_of.hpp>

//#include <omniORB4/fixed.h>

#include "setup_server_decl.h"
#include "time_clock.h"
#include "fredlib/registrar.h"
#include "fredlib/domain/update_domain.h"
#include "fredlib/nsset/update_nsset.h"
#include "fredlib/keyset/update_keyset.h"
#include "fredlib/contact/delete_contact.h"
#include "fredlib/opexception.h"
#include "util/util.h"

#include "fredlib/contact_verification/contact.h"
#include "fredlib/object_states.h"
#include "contact_verification/contact_verification_impl.h"
#include "random_data_generator.h"
#include "concurrent_queue.h"


#include "fredlib/db_settings.h"

#include "cfg/handle_general_args.h"
#include "cfg/handle_server_args.h"
#include "cfg/handle_logging_args.h"
#include "cfg/handle_database_args.h"
#include "cfg/handle_threadgroup_args.h"
#include "cfg/handle_corbanameservice_args.h"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include "cfg/config_handler_decl.h"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestEPPops)

const std::string server_name = "test-eppops";



struct TestFailParam
{
    ConstArr get_fail_param() throw()
    {
        static const char* list[]={"param1", "param2", "param3", "param4", "param5"};
        return ConstArr(list,sizeof(list)/sizeof(char*));
    }
protected:
    ~TestFailParam() throw(){}
};

struct TestFailReason
{
    ConstArr get_fail_reason() throw()
    {
        static const char* list[]={"reason1", "reason2", "reason3", "reason4", "reason5"};
        return ConstArr(list,sizeof(list)/sizeof(char*));
    }
protected:
    ~TestFailReason()throw(){}
};

///test operation exception interface
struct TestOperationException
: virtual public std::exception  //common base
{
    virtual const char* what() const throw() = 0;
    virtual ~TestOperationException() throw() {};
};


typedef OperationException<2048,TestOperationException,TestFailParam,TestFailReason> TestOpEx;

#define OPEX(DATA) TestOpEx(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))

///test callback
void print_str(TestOpEx::FixedStringType str)
{
    printf("\nstr: %s\n",str.data);
}

///test callback
void print_3str(TestOpEx::FixedStringType str1, TestOpEx::FixedStringType str2, TestOpEx::FixedStringType str3)
{
    printf("\nstr: %s - %s - %s\n", str1.data, str2.data, str3.data);
}




BOOST_AUTO_TEST_CASE(operation_exception)
{
    //test op
    try
    {
        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction trans(conn);
        Database::Result res = conn.exec_params(
            "SELECT $1::text, raise_exception_ifnull((select null::text) "
            " ,'operation_exception test || reason1:param1: '||ex_data('value1')"
            " ||' | reason2:param2: '||ex_data('value2')"
            " ||' | reason3:param3: '||ex_data('value3')||' |')"
            , Database::query_param_list("test"));
        if(res.size() == 0)
        {
            throw std::runtime_error("failed");
        }
    }
    catch(std::exception& ex)
    {
        TestOpEx::FixedStringFunc func = print_str;
        TestOpEx testexp = OPEX(ex.what()) ;
        BOOST_MESSAGE(ex.what());
        BOOST_MESSAGE(testexp.what());

        BOOST_CHECK(testexp.get_fail_param().size == 5);
        BOOST_CHECK(strlen(testexp.get_fail_param().arr[0]) == 6);

        testexp.for_params(func);

        SearchCallback<TestOpEx> ("",print_3str,testexp).run();//exec for all
        SearchCallback<TestOpEx> ("param2",print_3str,testexp).run();//exec for param2
        SearchCallback<TestOpEx> ("param2",print_3str,testexp).run();//exec for param2
        SearchCallback<TestOpEx> ("reason3",print_3str,testexp).run();//exec for reason3
        SearchCallback<TestOpEx> ("param3",print_3str,testexp).run();//exec for param3

        testexp.callback_exception_params(print_3str,"");
        testexp.callback_exception_params(print_3str,"param1");
        testexp.callback_exception_params(print_3str);

        //TestFailReason* bad_ptr = dynamic_cast<TestFailReason*>(&testexp);
        //delete(bad_ptr);//prohibited by protected dtor
        //TestFailReason tfr;//prohibited by protected dtor
    }
    {//length of the list check
        const char* list[]={"reason1", "reason2", "reason3", "reason4", "reason5"};
        BOOST_CHECK(sizeof(list)/sizeof(char*) == 5);

        //printf("ConstArr(list).size: %d",ConstArr(list, sizeof(list)/sizeof(char*)).size);
        BOOST_CHECK(ConstArr(list, sizeof(list)/sizeof(char*)).size == 5);
    }
}

BOOST_AUTO_TEST_CASE(delete_contact)
{
    std::string registrar_handle = "REG-FRED_A";
    Fred::Contact::Verification::Contact fcvc;
    unsigned long long request_id =0;
    std::string another_request_id;
    {
        //might get replaced by CreateContact when we have one
        //get db connection
        Database::Connection conn = Database::Manager::acquire();

        //Database::Transaction trans(conn);

        //get registrar id
        Database::Result res_reg = conn.exec_params(
                "SELECT id FROM registrar WHERE handle=$1::text",
                Database::query_param_list(registrar_handle));
        if(res_reg.size() == 0) {
            throw std::runtime_error("Registrar does not exist");
        }
        unsigned long long registrar_id = res_reg[0][0];

        RandomDataGenerator rdg;

        //create test contact
        std::string xmark = rdg.xnumstring(6);
        fcvc.handle=std::string("TEST-DEL-CONTACT-HANDLE")+xmark;
        fcvc.name=std::string("TEST-DEL-CONTACT NAME")+xmark;
        fcvc.organization=std::string("TEST-DEL-CONTACT-ORG")+xmark;
        fcvc.street1=std::string("TEST-DEL-CONTACT-STR1")+xmark;
        fcvc.city=std::string("Praha");
        fcvc.postalcode=std::string("11150");
        fcvc.country=std::string("CZ");
        fcvc.telephone=std::string("+420.728")+xmark;
        fcvc.email=std::string("test")+xmark+"@nic.cz";
        fcvc.ssn=std::string("1980-01-01");
        fcvc.ssntype=std::string("BIRTHDAY");
        fcvc.auth_info=rdg.xnstring(8);
        //unsigned long long contact_hid =

        fcvc.disclosename = true;
        fcvc.discloseorganization = true;
        fcvc.discloseaddress = true;
        fcvc.disclosetelephone = true;
        fcvc.disclosefax = true;
        fcvc.discloseemail = true;
        fcvc.disclosevat = true;
        fcvc.discloseident = true;
        fcvc.disclosenotifyemail = true;

        Fred::Contact::Verification::contact_create(request_id, registrar_id, fcvc);
    }

    Fred::OperationContext ctx;
    Fred::DeleteContact(fcvc.handle).exec(ctx);
    ctx.commit_transaction();

    BOOST_CHECK(static_cast<bool>(ctx.get_conn().exec_params(
        "select erdate is not null from object_registry where name = $1::text"
        ,Database::query_param_list(fcvc.handle))[0][0]));
}//delete_contact

BOOST_AUTO_TEST_CASE(update_domain)
{
    std::string registrar_handle = "REG-FRED_A";
    unsigned long long request_id =0;
    std::string another_request_id;

    Fred::Contact::Verification::Contact test_admin_contact;
    {
        //might get replaced by CreateContact when we have one
        //get db connection
        Database::Connection conn = Database::Manager::acquire();

        //get registrar id
        Database::Result res_reg = conn.exec_params(
                "SELECT id FROM registrar WHERE handle=$1::text",
                Database::query_param_list(registrar_handle));
        if(res_reg.size() == 0) {
            throw std::runtime_error("Registrar does not exist");
        }
        unsigned long long registrar_id = res_reg[0][0];

        RandomDataGenerator rdg;

        //create test contact
        std::string xmark = rdg.xnumstring(6);
        test_admin_contact.handle=std::string("TEST-ADMIN-CONTACT-HANDLE")+xmark;
        test_admin_contact.name=std::string("TEST-ADMIN-CONTACT NAME")+xmark;
        test_admin_contact.organization=std::string("TEST-ADMIN-CONTACT-ORG")+xmark;
        test_admin_contact.street1=std::string("TEST-ADMIN-CONTACT-STR1")+xmark;
        test_admin_contact.city=std::string("Praha");
        test_admin_contact.postalcode=std::string("11150");
        test_admin_contact.country=std::string("CZ");
        test_admin_contact.telephone=std::string("+420.728")+xmark;
        test_admin_contact.email=std::string("test")+xmark+"@nic.cz";
        test_admin_contact.ssn=std::string("1980-01-01");
        test_admin_contact.ssntype=std::string("BIRTHDAY");
        test_admin_contact.auth_info=rdg.xnstring(8);

        test_admin_contact.disclosename = true;
        test_admin_contact.discloseorganization = true;
        test_admin_contact.discloseaddress = true;
        test_admin_contact.disclosetelephone = true;
        test_admin_contact.disclosefax = true;
        test_admin_contact.discloseemail = true;
        test_admin_contact.disclosevat = true;
        test_admin_contact.discloseident = true;
        test_admin_contact.disclosenotifyemail = true;

        Fred::Contact::Verification::contact_create(request_id, registrar_id, test_admin_contact);
    }

    Fred::Contact::Verification::Contact test_admin_contact1;
    {
        //might get replaced by CreateContact when we have one
        //get db connection
        Database::Connection conn = Database::Manager::acquire();

        //get registrar id
        Database::Result res_reg = conn.exec_params(
                "SELECT id FROM registrar WHERE handle=$1::text",
                Database::query_param_list(registrar_handle));
        if(res_reg.size() == 0) {
            throw std::runtime_error("Registrar does not exist");
        }
        unsigned long long registrar_id = res_reg[0][0];

        RandomDataGenerator rdg;

        //create test contact
        std::string xmark = rdg.xnumstring(6);
        test_admin_contact1.handle=std::string("TEST-ADMIN-CONTACT2-HANDLE")+xmark;
        test_admin_contact1.name=std::string("TEST-ADMIN-CONTACT2 NAME")+xmark;
        test_admin_contact1.organization=std::string("TEST-ADMIN-CONTACT2-ORG")+xmark;
        test_admin_contact1.street1=std::string("TEST-ADMIN-CONTACT2-STR1")+xmark;
        test_admin_contact1.city=std::string("Praha");
        test_admin_contact1.postalcode=std::string("11150");
        test_admin_contact1.country=std::string("CZ");
        test_admin_contact1.telephone=std::string("+420.728")+xmark;
        test_admin_contact1.email=std::string("test")+xmark+"@nic.cz";
        test_admin_contact1.ssn=std::string("1980-01-01");
        test_admin_contact1.ssntype=std::string("BIRTHDAY");
        test_admin_contact1.auth_info=rdg.xnstring(8);

        test_admin_contact1.disclosename = true;
        test_admin_contact1.discloseorganization = true;
        test_admin_contact1.discloseaddress = true;
        test_admin_contact1.disclosetelephone = true;
        test_admin_contact1.disclosefax = true;
        test_admin_contact1.discloseemail = true;
        test_admin_contact1.disclosevat = true;
        test_admin_contact1.discloseident = true;
        test_admin_contact1.disclosenotifyemail = true;

        Fred::Contact::Verification::contact_create(request_id, registrar_id, test_admin_contact1);
    }


    Fred::Contact::Verification::Contact test_registrant_contact;
    {
        //might get replaced by CreateContact when we have one
        //get db connection
        Database::Connection conn = Database::Manager::acquire();

        //get registrar id
        Database::Result res_reg = conn.exec_params(
                "SELECT id FROM registrar WHERE handle=$1::text",
                Database::query_param_list(registrar_handle));
        if(res_reg.size() == 0) {
            throw std::runtime_error("Registrar does not exist");
        }
        unsigned long long registrar_id = res_reg[0][0];

        RandomDataGenerator rdg;

        //create test contact
        std::string xmark = rdg.xnumstring(6);
        test_registrant_contact.handle=std::string("TEST-REGISTRANT-CONTACT-HANDLE")+xmark;
        test_registrant_contact.name=std::string("TEST-REGISTRANT-CONTACT NAME")+xmark;
        test_registrant_contact.organization=std::string("TEST-REGISTRANT-CONTACT-ORG")+xmark;
        test_registrant_contact.street1=std::string("TEST-REGISTRANT-CONTACT-STR1")+xmark;
        test_registrant_contact.city=std::string("Praha");
        test_registrant_contact.postalcode=std::string("11150");
        test_registrant_contact.country=std::string("CZ");
        test_registrant_contact.telephone=std::string("+420.728")+xmark;
        test_registrant_contact.email=std::string("test")+xmark+"@nic.cz";
        test_registrant_contact.ssn=std::string("1980-01-01");
        test_registrant_contact.ssntype=std::string("BIRTHDAY");
        test_registrant_contact.auth_info=rdg.xnstring(8);

        test_registrant_contact.disclosename = true;
        test_registrant_contact.discloseorganization = true;
        test_registrant_contact.discloseaddress = true;
        test_registrant_contact.disclosetelephone = true;
        test_registrant_contact.disclosefax = true;
        test_registrant_contact.discloseemail = true;
        test_registrant_contact.disclosevat = true;
        test_registrant_contact.discloseident = true;
        test_registrant_contact.disclosenotifyemail = true;

        Fred::Contact::Verification::contact_create(request_id, registrar_id, test_registrant_contact);
    }

    Fred::OperationContext ctx;

    //call update using big ctor
    Fred::UpdateDomain("fred.cz"//fqdn
            , "REG-FRED_A"//registrar
            , test_registrant_contact.handle //registrant - owner
            , std::string("testauthinfo1") //authinfo
            , Nullable<std::string>()//unset nsset - set to null
            , Optional<Nullable<std::string> >()//dont change keyset
            , Util::vector_of<std::string> (test_admin_contact1.handle)(test_registrant_contact.handle) //add admin contacts
            , Util::vector_of<std::string> ("KONTAKT") //remove admin contacts
            , Optional<unsigned long long>() //request_id not set
            ).exec(ctx);

    //call update using small ctor and set custom params
    Fred::UpdateDomain("fred.cz", "REG-FRED_A")
    .set_authinfo("testauthinfo")
    .set_registrant(test_registrant_contact.handle)
    .add_admin_contact(test_admin_contact.handle)
    .rem_admin_contact(test_registrant_contact.handle)
    .rem_admin_contact(test_admin_contact1.handle)
    .exec(ctx);

    //call update using small ctor and set one custom param
    Fred::UpdateDomain("fred.cz", "REG-FRED_A").set_authinfo("testauthinfo").exec(ctx);
    Fred::UpdateDomain("fred.cz", "REG-FRED_A").set_registrant(test_registrant_contact.handle).exec(ctx);
    Fred::UpdateDomain("fred.cz", "REG-FRED_A").add_admin_contact(test_admin_contact1.handle).exec(ctx);
    Fred::UpdateDomain("fred.cz", "REG-FRED_A").rem_admin_contact(test_admin_contact.handle).exec(ctx);
    Fred::UpdateDomain("fred.cz", "REG-FRED_A").set_nsset(Nullable<std::string>()).exec(ctx);
    Fred::UpdateDomain("fred.cz", "REG-FRED_A").unset_nsset().exec(ctx);
    Fred::UpdateDomain("fred.cz", "REG-FRED_A").set_nsset(Nullable<std::string>("NSSET-1")).exec(ctx);
    Fred::UpdateDomain("fred.cz", "REG-FRED_A").set_nsset("NSSET-1").exec(ctx);
    Fred::UpdateDomain("fred.cz", "REG-FRED_A").set_keyset(Nullable<std::string>()).exec(ctx);
    Fred::UpdateDomain("fred.cz", "REG-FRED_A").unset_keyset().exec(ctx);
    Fred::UpdateDomain("fred.cz", "REG-FRED_A").set_keyset(Nullable<std::string>("KEYSID-1")).exec(ctx);
    Fred::UpdateDomain("fred.cz", "REG-FRED_A").set_keyset("KEYSID-1").exec(ctx);
    Fred::UpdateDomain("fred.cz", "REG-FRED_A").set_logd_request_id(0u).exec(ctx);

    //commit db transaction
    ctx.commit_transaction();

    //TODO check result of updates
    BOOST_CHECK(static_cast<bool>(ctx.get_conn().exec_params(
        "SELECT o.authinfopw = $1::text "
        //" AND "
        " FROM object_registry oreg "
        " JOIN object o ON o.id = oreg.id "
        " WHERE oreg.name = $2::text"
        ,Database::query_param_list("testauthinfo")("fred.cz"))[0][0]));
}//update_domain

BOOST_AUTO_TEST_CASE(update_nsset)
{
    std::string registrar_handle = "REG-FRED_A";
    unsigned long long request_id =0;
    std::string another_request_id;

    Fred::Contact::Verification::Contact test_admin_contact2;
    {
        //might get replaced by CreateContact when we have one
        //get db connection
        Database::Connection conn = Database::Manager::acquire();

        //get registrar id
        Database::Result res_reg = conn.exec_params(
                "SELECT id FROM registrar WHERE handle=$1::text",
                Database::query_param_list(registrar_handle));
        if(res_reg.size() == 0) {
            throw std::runtime_error("Registrar does not exist");
        }
        unsigned long long registrar_id = res_reg[0][0];

        RandomDataGenerator rdg;

        //create test contact
        std::string xmark = rdg.xnumstring(6);
        test_admin_contact2.handle=std::string("TEST-ADMIN-CONTACT2-HANDLE")+xmark;
        test_admin_contact2.name=std::string("TEST-ADMIN-CONTACT2 NAME")+xmark;
        test_admin_contact2.organization=std::string("TEST-ADMIN-CONTACT2-ORG")+xmark;
        test_admin_contact2.street1=std::string("TEST-ADMIN-CONTACT2-STR1")+xmark;
        test_admin_contact2.city=std::string("Praha");
        test_admin_contact2.postalcode=std::string("11150");
        test_admin_contact2.country=std::string("CZ");
        test_admin_contact2.telephone=std::string("+420.728")+xmark;
        test_admin_contact2.email=std::string("test")+xmark+"@nic.cz";
        test_admin_contact2.ssn=std::string("1980-01-01");
        test_admin_contact2.ssntype=std::string("BIRTHDAY");
        test_admin_contact2.auth_info=rdg.xnstring(8);

        test_admin_contact2.disclosename = true;
        test_admin_contact2.discloseorganization = true;
        test_admin_contact2.discloseaddress = true;
        test_admin_contact2.disclosetelephone = true;
        test_admin_contact2.disclosefax = true;
        test_admin_contact2.discloseemail = true;
        test_admin_contact2.disclosevat = true;
        test_admin_contact2.discloseident = true;
        test_admin_contact2.disclosenotifyemail = true;

        Fred::Contact::Verification::contact_create(request_id, registrar_id, test_admin_contact2);
    }

    Fred::Contact::Verification::Contact test_admin_contact3;
    {
        //might get replaced by CreateContact when we have one
        //get db connection
        Database::Connection conn = Database::Manager::acquire();

        //get registrar id
        Database::Result res_reg = conn.exec_params(
                "SELECT id FROM registrar WHERE handle=$1::text",
                Database::query_param_list(registrar_handle));
        if(res_reg.size() == 0) {
            throw std::runtime_error("Registrar does not exist");
        }
        unsigned long long registrar_id = res_reg[0][0];

        RandomDataGenerator rdg;

        //create test contact
        std::string xmark = rdg.xnumstring(6);
        test_admin_contact3.handle=std::string("TEST-ADMIN-CONTACT3-HANDLE")+xmark;
        test_admin_contact3.name=std::string("TEST-ADMIN-CONTACT3 NAME")+xmark;
        test_admin_contact3.organization=std::string("TEST-ADMIN-CONTACT3-ORG")+xmark;
        test_admin_contact3.street1=std::string("TEST-ADMIN-CONTACT3-STR1")+xmark;
        test_admin_contact3.city=std::string("Praha");
        test_admin_contact3.postalcode=std::string("11150");
        test_admin_contact3.country=std::string("CZ");
        test_admin_contact3.telephone=std::string("+420.728")+xmark;
        test_admin_contact3.email=std::string("test")+xmark+"@nic.cz";
        test_admin_contact3.ssn=std::string("1980-01-01");
        test_admin_contact3.ssntype=std::string("BIRTHDAY");
        test_admin_contact3.auth_info=rdg.xnstring(8);

        test_admin_contact3.disclosename = true;
        test_admin_contact3.discloseorganization = true;
        test_admin_contact3.discloseaddress = true;
        test_admin_contact3.disclosetelephone = true;
        test_admin_contact3.disclosefax = true;
        test_admin_contact3.discloseemail = true;
        test_admin_contact3.disclosevat = true;
        test_admin_contact3.discloseident = true;
        test_admin_contact3.disclosenotifyemail = true;

        Fred::Contact::Verification::contact_create(request_id, registrar_id, test_admin_contact3);
    }


    Fred::OperationContext ctx;
    Fred::UpdateNsset("NSSET-1", "REG-FRED_A").exec(ctx);

    Fred::UpdateNsset("NSSET-1"//handle
            , "REG-FRED_A"//registrar
            , Optional<std::string>()//authinfo
            , std::vector<Fred::DnsHost>() //add_dns
            , std::vector<std::string>() //rem_dns
            , std::vector<std::string>() //add_tech_contact
            , std::vector<std::string>() //rem_tech_contact
            , Optional<short>() //tech_check_level
            , Optional<unsigned long long>() //logd_request_id
            ).exec(ctx);

    Fred::UpdateNsset("NSSET-1"//handle
            , "REG-FRED_A"//registrar
                , Optional<std::string>("passwd")//authinfo
                , Util::vector_of<Fred::DnsHost>
                    (Fred::DnsHost("host",  Util::vector_of<std::string>("127.0.0.1")("127.1.1.1"))) //add_dns
                    (Fred::DnsHost("host1", Util::vector_of<std::string>("127.0.0.2")("127.1.1.2"))) //add_dns
                , Util::vector_of<std::string>("a.ns.nic.cz") //rem_dns
                , Util::vector_of<std::string>(test_admin_contact3.handle) //std::vector<std::string>() //add_tech_contact
                , Util::vector_of<std::string>(test_admin_contact3.handle) //std::vector<std::string>() //rem_tech_contact
                , Optional<short>(0) //tech_check_level
                , Optional<unsigned long long>(0) //logd_request_id
                ).exec(ctx);

    Fred::UpdateNsset("NSSET-1", "REG-FRED_A")
        .add_dns(Fred::DnsHost("host2",  Util::vector_of<std::string>("127.0.0.3")("127.1.1.3")))
        .rem_dns("b.ns.nic.cz")
        .add_tech_contact(test_admin_contact3.handle)
        .rem_tech_contact(test_admin_contact3.handle)
        .set_authinfo("passw")
        .set_logd_request_id(0)
        .set_tech_check_level(0)
    .exec(ctx);

    Fred::UpdateNsset("NSSET-1", "REG-FRED_A").add_dns(Fred::DnsHost("host3",  Util::vector_of<std::string>("127.0.0.4")("127.1.1.4"))).exec(ctx);
    Fred::UpdateNsset("NSSET-1", "REG-FRED_A").rem_dns("host2").exec(ctx);
    Fred::UpdateNsset("NSSET-1", "REG-FRED_A").add_tech_contact(test_admin_contact3.handle).exec(ctx);
    Fred::UpdateNsset("NSSET-1", "REG-FRED_A").rem_tech_contact(test_admin_contact3.handle).exec(ctx);
    Fred::UpdateNsset("NSSET-1", "REG-FRED_A").set_authinfo("passw").exec(ctx);
    Fred::UpdateNsset("NSSET-1", "REG-FRED_A").set_logd_request_id(0).exec(ctx);
    Fred::UpdateNsset("NSSET-1", "REG-FRED_A").set_tech_check_level(0).exec(ctx);

    //ctx.commit_transaction();
}//update_nsset

BOOST_AUTO_TEST_CASE(update_keyset)
{
    std::string registrar_handle = "REG-FRED_A";
    unsigned long long request_id =0;
    std::string another_request_id;

    Fred::Contact::Verification::Contact test_admin_contact4;
    {
        //might get replaced by CreateContact when we have one
        //get db connection
        Database::Connection conn = Database::Manager::acquire();

        //get registrar id
        Database::Result res_reg = conn.exec_params(
                "SELECT id FROM registrar WHERE handle=$1::text",
                Database::query_param_list(registrar_handle));
        if(res_reg.size() == 0) {
            throw std::runtime_error("Registrar does not exist");
        }
        unsigned long long registrar_id = res_reg[0][0];

        RandomDataGenerator rdg;

        //create test contact
        std::string xmark = rdg.xnumstring(6);
        test_admin_contact4.handle=std::string("TEST-ADMIN-CONTACT4-HANDLE")+xmark;
        test_admin_contact4.name=std::string("TEST-ADMIN-CONTACT4 NAME")+xmark;
        test_admin_contact4.organization=std::string("TEST-ADMIN-CONTACT4-ORG")+xmark;
        test_admin_contact4.street1=std::string("TEST-ADMIN-CONTACT4-STR1")+xmark;
        test_admin_contact4.city=std::string("Praha");
        test_admin_contact4.postalcode=std::string("11150");
        test_admin_contact4.country=std::string("CZ");
        test_admin_contact4.telephone=std::string("+420.728")+xmark;
        test_admin_contact4.email=std::string("test")+xmark+"@nic.cz";
        test_admin_contact4.ssn=std::string("1980-01-01");
        test_admin_contact4.ssntype=std::string("BIRTHDAY");
        test_admin_contact4.auth_info=rdg.xnstring(8);

        test_admin_contact4.disclosename = true;
        test_admin_contact4.discloseorganization = true;
        test_admin_contact4.discloseaddress = true;
        test_admin_contact4.disclosetelephone = true;
        test_admin_contact4.disclosefax = true;
        test_admin_contact4.discloseemail = true;
        test_admin_contact4.disclosevat = true;
        test_admin_contact4.discloseident = true;
        test_admin_contact4.disclosenotifyemail = true;

        Fred::Contact::Verification::contact_create(request_id, registrar_id, test_admin_contact4);
    }

    Fred::Contact::Verification::Contact test_admin_contact5;
    {
        //might get replaced by CreateContact when we have one
        //get db connection
        Database::Connection conn = Database::Manager::acquire();

        //get registrar id
        Database::Result res_reg = conn.exec_params(
                "SELECT id FROM registrar WHERE handle=$1::text",
                Database::query_param_list(registrar_handle));
        if(res_reg.size() == 0) {
            throw std::runtime_error("Registrar does not exist");
        }
        unsigned long long registrar_id = res_reg[0][0];

        RandomDataGenerator rdg;

        //create test contact
        std::string xmark = rdg.xnumstring(6);
        test_admin_contact5.handle=std::string("TEST-ADMIN-CONTACT5-HANDLE")+xmark;
        test_admin_contact5.name=std::string("TEST-ADMIN-CONTACT5 NAME")+xmark;
        test_admin_contact5.organization=std::string("TEST-ADMIN-CONTACT5-ORG")+xmark;
        test_admin_contact5.street1=std::string("TEST-ADMIN-CONTACT5-STR1")+xmark;
        test_admin_contact5.city=std::string("Praha");
        test_admin_contact5.postalcode=std::string("11150");
        test_admin_contact5.country=std::string("CZ");
        test_admin_contact5.telephone=std::string("+420.728")+xmark;
        test_admin_contact5.email=std::string("test")+xmark+"@nic.cz";
        test_admin_contact5.ssn=std::string("1980-01-01");
        test_admin_contact5.ssntype=std::string("BIRTHDAY");
        test_admin_contact5.auth_info=rdg.xnstring(8);

        test_admin_contact5.disclosename = true;
        test_admin_contact5.discloseorganization = true;
        test_admin_contact5.discloseaddress = true;
        test_admin_contact5.disclosetelephone = true;
        test_admin_contact5.disclosefax = true;
        test_admin_contact5.discloseemail = true;
        test_admin_contact5.disclosevat = true;
        test_admin_contact5.discloseident = true;
        test_admin_contact5.disclosenotifyemail = true;

        Fred::Contact::Verification::contact_create(request_id, registrar_id, test_admin_contact5);
    }


    Fred::OperationContext ctx;

    Fred::UpdateKeyset("KEYSID-1", "REG-FRED_A").exec(ctx);

    Fred::UpdateKeyset("KEYSID-1"//const std::string& handle
                , "REG-FRED_A"//const std::string& registrar
                , Optional<std::string>("testauthinfo")//const Optional<std::string>& authinfo
                , Util::vector_of<std::string>(test_admin_contact5.handle) //const std::vector<std::string>& add_tech_contact
                , Util::vector_of<std::string>("KONTAKT")//const std::vector<std::string>& rem_tech_contact
                , Util::vector_of<Fred::DnsKey> (Fred::DnsKey(257, 3, 5, "key"))//const std::vector<DnsKey>& add_dns_key
                , Util::vector_of<Fred::DnsKey> (Fred::DnsKey(257, 3, 5, "AwEAAddt2AkLfYGKgiEZB5SmIF8EvrjxNMH6HtxWEA4RJ9Ao6LCWheg8"))//const std::vector<DnsKey>& rem_dns_key
                , Optional<unsigned long long>(0)//const Optional<unsigned long long> logd_request_id
                ).exec(ctx);

    Fred::UpdateKeyset("KEYSID-1"//const std::string& handle
                , "REG-FRED_A"//const std::string& registrar
                , Optional<std::string>()//const Optional<std::string>& authinfo
                , std::vector<std::string>() //const std::vector<std::string>& add_tech_contact
                , std::vector<std::string>()//const std::vector<std::string>& rem_tech_contact
                , std::vector<Fred::DnsKey>()//const std::vector<DnsKey>& add_dns_key
                , std::vector<Fred::DnsKey>()//const std::vector<DnsKey>& rem_dns_key
                , Optional<unsigned long long>()//const Optional<unsigned long long> logd_request_id
                ).exec(ctx);

    Fred::UpdateKeyset("KEYSID-1", "REG-FRED_A").set_authinfo("kukauthinfo").exec(ctx);
    Fred::UpdateKeyset("KEYSID-1", "REG-FRED_A").add_tech_contact(test_admin_contact4.handle).exec(ctx);
    Fred::UpdateKeyset("KEYSID-1", "REG-FRED_A").rem_tech_contact(test_admin_contact5.handle).exec(ctx);
    Fred::UpdateKeyset("KEYSID-1", "REG-FRED_A").add_dns_key(Fred::DnsKey(257, 3, 5, "key2")).add_dns_key(Fred::DnsKey(257, 3, 5, "key3")).exec(ctx);
    Fred::UpdateKeyset("KEYSID-1", "REG-FRED_A").rem_dns_key(Fred::DnsKey(257, 3, 5, "key")).exec(ctx);
    Fred::UpdateKeyset("KEYSID-1", "REG-FRED_A").set_logd_request_id(0).exec(ctx);


}//update_keyset

BOOST_AUTO_TEST_SUITE_END();//TestEPPops
