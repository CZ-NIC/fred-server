/*
 * Copyright (C) 2014  CZ.NIC, z.s.p.o.
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
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time.hpp>
#include <boost/assign/list_of.hpp>

#include "time_clock.h"
#include "random_data_generator.h"

#include "setup_server_decl.h"

#include "cfg/handle_general_args.h"
#include "cfg/handle_server_args.h"
#include "cfg/handle_logging_args.h"
#include "cfg/handle_database_args.h"
#include "cfg/handle_corbanameservice_args.h"
#include "cfg/handle_registry_args.h"


//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include "cfg/config_handler_decl.h"
#include <boost/test/unit_test.hpp>

#include "src/corba/mailer_manager.h"
#include "src/admin/contact/merge_contact_auto_procedure.h"
#include "src/corba/logger_client_impl.h"

#include "src/fredlib/contact/create_contact.h"
#include "src/fredlib/registrar/create_registrar.h"
#include "src/fredlib/registrar/info_registrar.h"


BOOST_AUTO_TEST_SUITE(TestMergeContactAutoProc)

const std::string server_name = "test-merge-contact-auto-proc";

class auto_proc_fixture
{
    Fred::OperationContext ctx;
public:
    std::string xmark;
    std::string registrar_handle;
    std::string contact_handle;

    auto_proc_fixture()
    try
    : xmark(RandomDataGenerator().xnumstring(8))
    , registrar_handle("REG-AMC_"+xmark+"_")
    , contact_handle("CONTACT-AMC_"+xmark+"_")
    {
        //registrar
        unsigned registrar_count = 1;
        for(unsigned i_registrar = 0; i_registrar < registrar_count; ++i_registrar)
        {
            std::string registrar_handle_i = registrar_handle + boost::lexical_cast<std::string>(i_registrar);
            Fred::CreateRegistrar(registrar_handle_i).set_system(false).exec(ctx);
            BOOST_MESSAGE(registrar_handle_i);

            //contact
            int i_contact = 0;
            do
            {
                std::string contact_handle_i = contact_handle + boost::lexical_cast<std::string>(i_registrar) + "_"+ boost::lexical_cast<std::string>(i_contact);
                Fred::CreateContact(contact_handle_i,registrar_handle_i).exec(ctx);
                BOOST_MESSAGE(contact_handle_i);

                ++i_contact;
            }
            while(i_contact < 3);

            ctx.commit_transaction();
        }


    }
    catch(Fred::OperationException& ex)
    {
        BOOST_MESSAGE(ex.get_exception_stack_info());
    }

};

BOOST_FIXTURE_TEST_CASE( test_auto_proc, auto_proc_fixture )
{
    //corba config
    FakedArgs fa = CfgArgs::instance()->fa;
    //conf pointers
    HandleCorbaNameServiceArgs* ns_args_ptr=CfgArgs::instance()->
                get_handler_ptr_by_type<HandleCorbaNameServiceArgs>();
    CorbaContainer::set_instance(fa.get_argc(), fa.get_argv()
            , ns_args_ptr->nameservice_host
            , ns_args_ptr->nameservice_port
            , ns_args_ptr->nameservice_context);
    boost::shared_ptr<Fred::Mailer::Manager> mm( new MailerManager(CorbaContainer::get_instance()->getNS()));
    std::auto_ptr<Fred::Logger::LoggerClient> logger_client(
            new Fred::Logger::LoggerCorbaClientImpl());

    Admin::MergeContactAutoProcedure(
            *(mm.get()),
            *(logger_client.get()),
            registrar_handle + boost::lexical_cast<std::string>(0))
        .set_limit(Optional<unsigned long long>())
        .set_dry_run(Optional<bool>(false))
        //.set_verbose(Optional<unsigned short>(10))
    .exec();

    //check merge
    Fred::OperationContext ctx;
    BOOST_CHECK(1 == static_cast<int>(ctx.get_conn().exec_params(
        "SELECT count(*) FROM object_registry oreg JOIN contact c ON oreg.id = c.id WHERE oreg.name like $1::text"
        , Database::query_param_list(contact_handle+"%"))[0][0]));

}

BOOST_AUTO_TEST_SUITE_END();//TestContactVerification
