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

#include <boost/function.hpp>
#include <boost/format.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time.hpp>
#include <boost/assign/list_of.hpp>

#include "setup_server_decl.h"
#include "time_clock.h"
#include "fredlib/registrar.h"
#include "fredlib/domain/update_domain.h"
#include "fredlib/nsset/update_nsset.h"
#include "fredlib/keyset/update_keyset.h"
#include "fredlib/contact/delete_contact.h"
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

/*
BOOST_AUTO_TEST_CASE( test_eppops )
{
    //db
    Database::Connection conn = Database::Manager::acquire();
    boost::shared_ptr<Database::Transaction> txp ( new Database::Transaction(conn));
    //Database::Transaction tx(conn);
    Database::Result res = conn.exec("select 1");
    txp->commit();
    BOOST_CHECK_EQUAL(std::string("1").compare(std::string(res[0][0])),0);

    //opctx
    Fred::OperationContext opctx;
    Fred::UpdateDomain("fred.cz", "REG-FRED_A").set_authinfo("testauth").exec(opctx);
    Fred::UpdateNsset("fred.cz", "REG-FRED_A").set_authinfo("testauth").add_dns(Fred::DnsHost("host", Util::vector_of<std::string>("127.0.0.1")("127.1.1.1"))).exec(opctx);
    Fred::UpdateKeyset("fred.cz", "REG-FRED_A").set_authinfo("testauth").add_dns_key(Fred::DnsKey(0,1,2,"key")).exec(opctx);
    Fred::DeleteContact("KONTAKT").exec(opctx);
    opctx.commit_transaction();
}
*/

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
}

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
        //unsigned long long contact_hid =

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

    Fred::Contact::Verification::Contact test_registrant_contact;
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
        test_registrant_contact.handle=std::string("TEST-ADMIN-CONTACT-HANDLE")+xmark;
        test_registrant_contact.name=std::string("TEST-ADMIN-CONTACT NAME")+xmark;
        test_registrant_contact.organization=std::string("TEST-ADMIN-CONTACT-ORG")+xmark;
        test_registrant_contact.street1=std::string("TEST-ADMIN-CONTACT-STR1")+xmark;
        test_registrant_contact.city=std::string("Praha");
        test_registrant_contact.postalcode=std::string("11150");
        test_registrant_contact.country=std::string("CZ");
        test_registrant_contact.telephone=std::string("+420.728")+xmark;
        test_registrant_contact.email=std::string("test")+xmark+"@nic.cz";
        test_registrant_contact.ssn=std::string("1980-01-01");
        test_registrant_contact.ssntype=std::string("BIRTHDAY");
        test_registrant_contact.auth_info=rdg.xnstring(8);
        //unsigned long long contact_hid =

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
    Fred::UpdateDomain("fred.cz", "REG-FRED_A")
    .set_authinfo("testauthinfo")
    .set_registrant(test_registrant_contact.handle)
    .add_admin_contact(test_admin_contact.handle)
    .rem_admin_contact("KONTAKT")
    .exec(ctx);

    Fred::UpdateDomain("fred.cz", "REG-FRED_A").set_nsset(Nullable<std::string>()).exec(ctx);
    Fred::UpdateDomain("fred.cz", "REG-FRED_A").unset_nsset().exec(ctx);
    Fred::UpdateDomain("fred.cz", "REG-FRED_A").set_nsset(Nullable<std::string>("NSSET-1")).exec(ctx);
    Fred::UpdateDomain("fred.cz", "REG-FRED_A").set_nsset("NSSET-1").exec(ctx);

    Fred::UpdateDomain("fred.cz", "REG-FRED_A").set_keyset(Nullable<std::string>()).exec(ctx);
    Fred::UpdateDomain("fred.cz", "REG-FRED_A").unset_keyset().exec(ctx);
    Fred::UpdateDomain("fred.cz", "REG-FRED_A").set_keyset(Nullable<std::string>("KEYSID-1")).exec(ctx);
    Fred::UpdateDomain("fred.cz", "REG-FRED_A").set_keyset("KEYSID-1").exec(ctx);

    Fred::UpdateDomain("fred.cz", "REG-FRED_A").set_logd_request_id(0u).exec(ctx);

    ctx.commit_transaction();

    BOOST_CHECK(static_cast<bool>(ctx.get_conn().exec_params(
        "SELECT o.authinfopw = $1::text "
        //" AND "
        " FROM object_registry oreg "
        " JOIN object o ON o.id = oreg.id "
        " WHERE oreg.name = $2::text"
        ,Database::query_param_list("testauthinfo")("fred.cz"))[0][0]));
}

BOOST_AUTO_TEST_SUITE_END();//TestEPPops
