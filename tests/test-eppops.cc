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

#include <boost/algorithm/string.hpp>
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
#include "fredlib/contact/create_contact.h"
#include "fredlib/nsset/create_nsset.h"
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

///test callback
void print_str(const char* str)
{
    printf("\nstr: %s\n",str);
}

///test callback
void print_3str(const char* str1, const char* str2, const char* str3)
{
    printf("\nstr: %s - %s - %s\n", str1, str2, str3);
}

//check callback input data
class CheckCallback
{
    Fred::ConstArr valid_params_;
    bool param_ok_;

public:
    CheckCallback(const Fred::ConstArr& valid_params)
    : valid_params_(valid_params)
    , param_ok_(true)
    {}

    //checking callback called for all params
    void operator()(const char* param, const char* value)
    {
        bool param_ok = false;

        for(int i =0; i < valid_params_.size();++i)
        {
            if(std::string(param).compare(std::string(valid_params_[i])) ==0) param_ok = true;
        }

        //save
        param_ok_ = param_ok;
    }

    bool params_ok()
    {
        return param_ok_;
    }
};


//get callback - store input data
class GetCallback
{
std::vector<std::string> data_;

public:

    //callback
    void operator()(const char* param, const char* value)
    {
        //BOOST_MESSAGE(std::string("get_cbk: ")+ " : " + param + " : " + value);
        data_.push_back(std::string(param) + " : " + value);
        //BOOST_MESSAGE(data_.size());
    }

    std::vector<std::string> get()
    {
        return data_;
    }
};

BOOST_AUTO_TEST_CASE(fixed_string)
{
    {
        Fred::FixedString<3> fs3;
        fs3.push_front("11");
        fs3.push_front("22");
        BOOST_CHECK(std::string(fs3.data) == "211");
    }

    {
        Fred::FixedString<3> fs3;
        fs3.push_back("11");
        fs3.push_back("22");
        BOOST_CHECK(std::string(fs3.data) == "112");
    }
}

BOOST_AUTO_TEST_CASE(exception_params_callback)
{
    try
    {
        std::string fqdn_("fred.cz");
        std::string registrar_("REG-FRED_A");
        std::string errmsg("test exception ||");
        errmsg += std::string(" not found:fqdn: ")+boost::replace_all_copy(fqdn_,"|", "[pipe]")+" |";
        errmsg += std::string(" not found:registrar: ")+boost::replace_all_copy(registrar_,"|", "[pipe]")+" |";

        throw Fred::UpdateDomainException(__FILE__, __LINE__, __ASSERT_FUNCTION, errmsg.c_str());
    }
    catch(Fred::OperationExceptionBase& ex)
    {
        GetCallback a;
        ex.callback_exception_params(boost::ref(a));
        BOOST_CHECK((a.get().size()) == 2);
    }

}

BOOST_AUTO_TEST_CASE(test_quote_pipe_in_exception)
{
    try
    {
        std::string fqdn_("|fred.cz|");
        std::string errmsg("test exception || not found:fqdn: ");
        errmsg += boost::replace_all_copy(fqdn_,"|", "[pipe]");//quote pipes
        errmsg += " | not found:registrant: test ";
        errmsg += " |";
        throw Fred::UpdateDomainException(__FILE__, __LINE__, __ASSERT_FUNCTION, errmsg.c_str());
    }
    catch(Fred::UpdateDomainException& ex)
    {
        CheckCallback check(ex.get_fail_param());
        ex.callback_exception_params(check);
        BOOST_CHECK(check.params_ok());
    }

    {//check Fred::OperationExceptionBase exception
        std::string fqdn_("|fred.cz|");
        std::string errmsg("test exception || not found:fqdn: ");
        errmsg += boost::replace_all_copy(fqdn_,"|", "[pipe]");//quote pipes
        errmsg += " |";
        BOOST_CHECK_THROW (throw Fred::UpdateDomainException(__FILE__, __LINE__, __ASSERT_FUNCTION, errmsg.c_str())
        , Fred::OperationExceptionBase);
    }

    try
    {
        std::string fqdn_("|fred.cz|");
        std::string errmsg("test exception || not found:fqdn: ");
        errmsg += boost::replace_all_copy(fqdn_,"|", "[pipe]");//quote pipes
        errmsg += " |";
        throw Fred::UpdateDomainException(__FILE__, __LINE__, __ASSERT_FUNCTION, errmsg.c_str());
    }
    catch(Fred::OperationExceptionBase& ex)
    {
        CheckCallback check(ex.get_fail_param());
        ex.callback_exception_params(check);
        BOOST_CHECK(check.params_ok());
    }

    {//invalid exception data
        std::string fqdn_("|fred.cz|");
        std::string errmsg("test error in exception params || not found:fqdn1: ");
        errmsg += boost::replace_all_copy(fqdn_,"|", "[pipe]");//quote pipes
        errmsg += " |";
        BOOST_CHECK_THROW (throw Fred::UpdateDomainException(__FILE__, __LINE__, __ASSERT_FUNCTION, errmsg.c_str())
        , Fred::UpdateDomainException::OperationErrorType);
        BOOST_CHECK_THROW (throw Fred::UpdateDomainException(__FILE__, __LINE__, __ASSERT_FUNCTION, errmsg.c_str())
        , Fred::OperationErrorBase);
    }

}

BOOST_AUTO_TEST_CASE(update_domain_exception)
{
    //no parsable data - ok
    BOOST_CHECK_THROW (throw Fred::UpdateDomainException(__FILE__, __LINE__, __ASSERT_FUNCTION, "test")
    , Fred::OperationExceptionBase);

    //no data - error
    BOOST_CHECK_THROW (throw Fred::UpdateDomainException(__FILE__, __LINE__, __ASSERT_FUNCTION, "test ||  |")
    , Fred::OperationErrorBase);

    //error exception - ok
    BOOST_CHECK_THROW (throw Fred::UpdateDomainException::OperationErrorType("test")
    , Fred::OperationErrorBase);

    //not ended by | - no parsable data - ok
    BOOST_CHECK_THROW (throw Fred::UpdateDomainException(__FILE__, __LINE__, __ASSERT_FUNCTION, "test exception || not found:1111: errtest")
        , Fred::OperationExceptionBase);
}

BOOST_AUTO_TEST_CASE(delete_contact)
{
    std::string registrar_handle = "REG-FRED_A";
    Fred::OperationContext ctx;
    std::string xmark = RandomDataGenerator().xnumstring(6);
    std::string handle = std::string("TEST-DEL-CONTACT-HANDLE")+xmark;

    Fred::CreateContact(handle,registrar_handle).set_name(std::string("TEST-DEL-CONTACT NAME")+xmark)
        .set_name(std::string("TEST-DEL-CONTACT NAME")+xmark)
        .set_disclosename(true)
        .set_street1(std::string("STR1")+xmark)
        .set_city("Praha").set_postalcode("11150").set_country("CZ")
        .set_discloseaddress(true)
        .exec(ctx);
    Fred::DeleteContact(handle).exec(ctx);
    ctx.commit_transaction();

    BOOST_CHECK(static_cast<bool>(ctx.get_conn().exec_params(
        "select erdate is not null from object_registry where name = $1::text"
        ,Database::query_param_list(handle))[0][0]));
}//delete_contact

BOOST_AUTO_TEST_CASE(update_domain)
{
    std::string registrar_handle = "REG-FRED_A";
    Fred::OperationContext ctx;
    std::string xmark = RandomDataGenerator().xnumstring(6);

    std::string admin_contact_handle = std::string("TEST-ADMIN-CONTACT-HANDLE")+xmark;
    Fred::CreateContact(admin_contact_handle,registrar_handle)
        .set_name(std::string("TEST-ADMIN-CONTACT NAME")+xmark)
        .set_disclosename(true)
        .set_street1(std::string("STR1")+xmark)
        .set_city("Praha").set_postalcode("11150").set_country("CZ")
        .set_discloseaddress(true)
        .exec(ctx);

    std::string admin_contact1_handle = std::string("TEST-ADMIN-CONTACT2-HANDLE")+xmark;
    Fred::CreateContact(admin_contact1_handle,registrar_handle)
        .set_name(std::string("TEST-ADMIN-CONTACT2 NAME")+xmark)
        .set_disclosename(true)
        .set_street1(std::string("STR1")+xmark)
        .set_city("Praha").set_postalcode("11150").set_country("CZ")
        .set_discloseaddress(true)
        .exec(ctx);

    std::string registrant_contact_handle = std::string("TEST-REGISTRANT-CONTACT-HANDLE")+xmark;
    Fred::CreateContact(registrant_contact_handle,registrar_handle)
            .set_name(std::string("TEST-REGISTRANT-CONTACT NAME")+xmark)
            .set_disclosename(true)
            .set_street1(std::string("STR1")+xmark)
            .set_city("Praha").set_postalcode("11150").set_country("CZ")
            .set_discloseaddress(true)
            .exec(ctx);

    //call update using big ctor
    Fred::UpdateDomain("fred.cz"//fqdn
            , "REG-FRED_A"//registrar
            , registrant_contact_handle //registrant - owner
            , std::string("testauthinfo1") //authinfo
            , Nullable<std::string>()//unset nsset - set to null
            , Optional<Nullable<std::string> >()//dont change keyset
            , Util::vector_of<std::string> (admin_contact1_handle)(registrant_contact_handle) //add admin contacts
            , Util::vector_of<std::string> ("KONTAKT") //remove admin contacts
            , Optional<unsigned long long>() //request_id not set
            ).exec(ctx);

    //call update using small ctor and set custom params
    Fred::UpdateDomain("fred.cz", "REG-FRED_A")
    .set_authinfo("testauthinfo")
    .set_registrant(registrant_contact_handle)
    .add_admin_contact(admin_contact_handle)
    .rem_admin_contact(registrant_contact_handle)
    .rem_admin_contact(admin_contact1_handle)
    .exec(ctx);

    //call update using small ctor and set one custom param
    Fred::UpdateDomain("fred.cz", "REG-FRED_A").set_authinfo("testauthinfo").exec(ctx);
    Fred::UpdateDomain("fred.cz", "REG-FRED_A").set_registrant(registrant_contact_handle).exec(ctx);
    Fred::UpdateDomain("fred.cz", "REG-FRED_A").add_admin_contact(admin_contact1_handle).exec(ctx);
    Fred::UpdateDomain("fred.cz", "REG-FRED_A").rem_admin_contact(admin_contact_handle).exec(ctx);
    Fred::UpdateDomain("fred.cz", "REG-FRED_A").set_nsset("NSSET-1").exec(ctx);
    Fred::UpdateDomain("fred.cz", "REG-FRED_A").set_nsset(Nullable<std::string>()).exec(ctx);
    Fred::UpdateDomain("fred.cz", "REG-FRED_A").set_nsset("NSSET-1").exec(ctx);
    Fred::UpdateDomain("fred.cz", "REG-FRED_A").unset_nsset().exec(ctx);
    Fred::UpdateDomain("fred.cz", "REG-FRED_A").set_nsset(Nullable<std::string>("NSSET-1")).exec(ctx);
    Fred::UpdateDomain("fred.cz", "REG-FRED_A").set_nsset("NSSET-1").exec(ctx);
    Fred::UpdateDomain("fred.cz", "REG-FRED_A").set_keyset(Nullable<std::string>()).exec(ctx);
    Fred::UpdateDomain("fred.cz", "REG-FRED_A").set_keyset("KEYSID-1").exec(ctx);
    Fred::UpdateDomain("fred.cz", "REG-FRED_A").unset_keyset().exec(ctx);
    Fred::UpdateDomain("fred.cz", "REG-FRED_A").set_keyset(Nullable<std::string>("KEYSID-1")).exec(ctx);
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
    Fred::OperationContext ctx;
    std::string xmark = RandomDataGenerator().xnumstring(6);

    std::string admin_contact2_handle = std::string("TEST-ADMIN-CONTACT2-HANDLE")+xmark;
    Fred::CreateContact(admin_contact2_handle,registrar_handle)
        .set_name(std::string("TEST-ADMIN-CONTACT2 NAME")+xmark)
        .set_disclosename(true)
        .set_street1(std::string("STR1")+xmark)
        .set_city("Praha").set_postalcode("11150").set_country("CZ")
        .set_discloseaddress(true)
        .exec(ctx);

    std::string admin_contact3_handle = std::string("TEST-ADMIN-CONTACT3-HANDLE")+xmark;
    Fred::CreateContact(admin_contact3_handle,registrar_handle)
        .set_name(std::string("TEST-ADMIN-CONTACT3 NAME")+xmark)
        .set_disclosename(true)
        .set_street1(std::string("STR1")+xmark)
        .set_city("Praha").set_postalcode("11150").set_country("CZ")
        .set_discloseaddress(true)
        .exec(ctx);

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
                , Util::vector_of<std::string>(admin_contact3_handle) //std::vector<std::string>() //add_tech_contact
                , Util::vector_of<std::string>(admin_contact3_handle) //std::vector<std::string>() //rem_tech_contact
                , Optional<short>(0) //tech_check_level
                , Optional<unsigned long long>(0) //logd_request_id
                ).exec(ctx);

    Fred::UpdateNsset("NSSET-1", "REG-FRED_A")
        .add_dns(Fred::DnsHost("host2",  Util::vector_of<std::string>("127.0.0.3")("127.1.1.3")))
        .rem_dns("b.ns.nic.cz")
        .add_tech_contact(admin_contact3_handle)
        .rem_tech_contact(admin_contact3_handle)
        .set_authinfo("passw")
        .set_logd_request_id(0)
        .set_tech_check_level(0)
    .exec(ctx);

    Fred::UpdateNsset("NSSET-1", "REG-FRED_A").add_dns(Fred::DnsHost("host3",  Util::vector_of<std::string>("127.0.0.4")("127.1.1.4"))).exec(ctx);
    Fred::UpdateNsset("NSSET-1", "REG-FRED_A").rem_dns("host2").exec(ctx);
    Fred::UpdateNsset("NSSET-1", "REG-FRED_A").add_tech_contact(admin_contact3_handle).exec(ctx);
    Fred::UpdateNsset("NSSET-1", "REG-FRED_A").rem_tech_contact(admin_contact3_handle).exec(ctx);
    Fred::UpdateNsset("NSSET-1", "REG-FRED_A").set_authinfo("passw").exec(ctx);
    Fred::UpdateNsset("NSSET-1", "REG-FRED_A").set_logd_request_id(0).exec(ctx);
    Fred::UpdateNsset("NSSET-1", "REG-FRED_A").set_tech_check_level(0).exec(ctx);

    //ctx.commit_transaction();
}//update_nsset

BOOST_AUTO_TEST_CASE(update_keyset)
{
    std::string registrar_handle = "REG-FRED_A";
    Fred::OperationContext ctx;
    std::string xmark = RandomDataGenerator().xnumstring(6);

    std::string admin_contact4_handle = std::string("TEST-ADMIN-CONTACT4-HANDLE")+xmark;
    Fred::CreateContact(admin_contact4_handle,registrar_handle)
        .set_name(std::string("TEST-ADMIN-CONTACT4 NAME")+xmark)
        .set_disclosename(true)
        .set_street1(std::string("STR1")+xmark)
        .set_city("Praha").set_postalcode("11150").set_country("CZ")
        .set_discloseaddress(true)
        .exec(ctx);

    std::string admin_contact5_handle = std::string("TEST-ADMIN-CONTACT5-HANDLE")+xmark;
    Fred::CreateContact(admin_contact5_handle,registrar_handle)
        .set_name(std::string("TEST-ADMIN-CONTACT5 NAME")+xmark)
        .set_disclosename(true)
        .set_street1(std::string("STR1")+xmark)
        .set_city("Praha").set_postalcode("11150").set_country("CZ")
        .set_discloseaddress(true)
        .exec(ctx);

    Fred::UpdateKeyset("KEYSID-1", "REG-FRED_A").exec(ctx);

    Fred::UpdateKeyset("KEYSID-1"//const std::string& handle
                , "REG-FRED_A"//const std::string& registrar
                , Optional<std::string>("testauthinfo")//const Optional<std::string>& authinfo
                , Util::vector_of<std::string>(admin_contact5_handle) //const std::vector<std::string>& add_tech_contact
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
    Fred::UpdateKeyset("KEYSID-1", "REG-FRED_A").add_tech_contact(admin_contact4_handle).exec(ctx);
    Fred::UpdateKeyset("KEYSID-1", "REG-FRED_A").rem_tech_contact(admin_contact5_handle).exec(ctx);
    Fred::UpdateKeyset("KEYSID-1", "REG-FRED_A").add_dns_key(Fred::DnsKey(257, 3, 5, "key2")).add_dns_key(Fred::DnsKey(257, 3, 5, "key3")).exec(ctx);
    Fred::UpdateKeyset("KEYSID-1", "REG-FRED_A").rem_dns_key(Fred::DnsKey(257, 3, 5, "key")).exec(ctx);
    Fred::UpdateKeyset("KEYSID-1", "REG-FRED_A").set_logd_request_id(0).exec(ctx);

}//update_keyset

BOOST_AUTO_TEST_SUITE_END();//TestEPPops
