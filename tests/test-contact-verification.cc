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
#include <sstream>
#include <string>
#include <algorithm>
#include <functional>
#include <numeric>
#include <map>
#include <exception>
#include <queue>
#include <sys/time.h>
#include <time.h>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time.hpp>
#include <boost/assign/list_of.hpp>

#include "setup_server_decl.h"
#include "time_clock.h"
#include "random_data_generator.h"
#include "concurrent_queue.h"


#include "cfg/handle_general_args.h"
#include "cfg/handle_server_args.h"
#include "cfg/handle_logging_args.h"
#include "cfg/handle_database_args.h"
#include "cfg/handle_threadgroup_args.h"
#include "cfg/handle_corbanameservice_args.h"

#include "test-common-threaded.h"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include "cfg/config_handler_decl.h"
#include <boost/test/unit_test.hpp>

#include "mailer_manager.h"
#include "contact_verification/contact_verification_impl.h"

//test-contact-verification.cc

BOOST_AUTO_TEST_SUITE(TestContactVerification)

const std::string server_name = "test-contact-verification";



static bool check_std_exception(std::exception const & ex)
{
    std::string ex_msg(ex.what());
    return (ex_msg.length() != 0);
}


BOOST_AUTO_TEST_CASE( test_createConditionalIdentification_1 )
{
    //CORBA init
    FakedArgs orb_fa = CfgArgs::instance()->fa;
    HandleCorbaNameServiceArgs* ns_args_ptr=CfgArgs::instance()->
            get_handler_ptr_by_type<HandleCorbaNameServiceArgs>();
    CorbaContainer::set_instance(orb_fa.get_argc(), orb_fa.get_argv()
        , ns_args_ptr->nameservice_host
        , ns_args_ptr->nameservice_port
        , ns_args_ptr->nameservice_context);

    const std::auto_ptr<Registry::Contact::Verification::ContactVerificationImpl> cv(
        new Registry::Contact::Verification::ContactVerificationImpl(server_name
            , boost::shared_ptr<Fred::Mailer::Manager>(
                new MailerManager(CorbaContainer::get_instance()
                ->getNS()))));

    unsigned long long request_id =0;

    //get db connection
    Database::Connection conn = Database::Manager::acquire();

    //get registrar id
    std::string registrar_handle = "REG-FRED_A";
    Database::Result res_reg = conn.exec_params(
            "SELECT id FROM registrar WHERE handle=$1::text",
            Database::query_param_list(registrar_handle));
    if(res_reg.size() == 0) {
        throw std::runtime_error("Registrar does not exist");
    }
    unsigned long long registrar_id = res_reg[0][0];

    Fred::Contact::Verification::Contact fcvc;
    RandomDataGenerator rdg;

    //create test contact
    std::string xmark = rdg.xnumstring(6);
    fcvc.handle=std::string("TESTCV-HANDLE")+xmark;
    fcvc.name=std::string("TESTCV-NAME")+xmark;
    fcvc.organization=std::string("TESTCV-ORG")+xmark;
    fcvc.street1=std::string("TESTCV-STR1")+xmark;
    fcvc. city=std::string("Praha");
    fcvc.postalcode=std::string("11150");
    fcvc.country=std::string("CZ");
    fcvc.telephone=std::string("728")+xmark;
    fcvc.email=std::string("test")+xmark+"@nic.cz";
    fcvc.ssn=std::string("1980-01-01");
    fcvc.ssntype=std::string("BIRTHDAY");
    fcvc.auth_info=rdg.xnstring(8);
    //unsigned long long contact_hid =
    Fred::Contact::Verification::contact_create(request_id, registrar_id, fcvc);

    std::string another_request_id;
    cv->createConditionalIdentification(fcvc.handle, registrar_handle
            , request_id, another_request_id);

    cv->processConditionalIdentification(another_request_id
            , fcvc.auth_info, request_id);

    cv->processIdentification(fcvc.handle, fcvc.auth_info, request_id);

    BOOST_CHECK(1==1);

/*
    const std::string& get_server_name();

    unsigned long long createConditionalIdentification(
            const std::string & contact_handle
            , const std::string & registrar_handle
            , const unsigned long long log_id
            , std::string & request_id);

    unsigned long long processConditionalIdentification(
            const std::string & request_id
            , const std::string & password
            , const unsigned long long log_id);

    unsigned long long processIdentification(
            const std::string & contact_handle
            , const std::string & password
            , const unsigned long long log_id);

    std::string getRegistrarName(const std::string & registrar_handle);
*/
}



BOOST_AUTO_TEST_SUITE_END();//TestContactVerification
