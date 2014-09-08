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
#include "util/log/logger.h"
#include "util/corba_wrapper_decl.h"
#include "src/corba/mailer_manager.h"
#include "src/admin/contact/merge_contact_auto_procedure.h"
#include "src/corba/logger_client_impl.h"

#include "src/fredlib/contact/create_contact.h"
#include "src/fredlib/registrar/create_registrar.h"
#include "src/fredlib/registrar/info_registrar.h"

#include "tests/fredlib/contact/test_merge_contact_fixture.h"

BOOST_AUTO_TEST_SUITE(TestMergeContactAutoProc)

const std::string server_name = "test-merge-contact-auto-proc";


/**
 * Setup merge contact test data with states.
 */
struct auto_proc_fixture : MergeContactFixture::mergeable_contact_grps_with_linked_objects_and_blocking_states
{
    auto_proc_fixture()
    : MergeContactFixture::mergeable_contact_grps_with_linked_objects_and_blocking_states(
        1u//mergeable_contact_group_count
        , Util::set_of<unsigned>(0)(15)(16)(17)(18)(19)(20)//init_linked_object_combinations()//linked_object_cases
        , init_set_of_contact_state_combinations()//contact_state_combinations//stateless states 0, 1
        , init_set_of_linked_object_state_combinations()//linked_object_state_combinations
        , Util::vector_of<unsigned>(0)(1)(2)//linked_object_quantities
        )
    {}
};


BOOST_FIXTURE_TEST_CASE( test_auto_proc_given_registrar, auto_proc_fixture )
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
            *(logger_client.get()))
        .set_registrar(registrar_mc_1_handle)
        //.set_limit(Optional<unsigned long long>())
        //.set_dry_run(Optional<bool>(false))
        //.set_verbose(Optional<unsigned short>(10))
    .exec();

    //contact changes
    std::map<std::string, Fred::InfoContactDiff> changed_contacts = diff_contacts();
    for(std::map<std::string, Fred::InfoContactDiff>::const_iterator ci = changed_contacts.begin(); ci != changed_contacts.end(); ++ci)
    {
        //check update registrar is system registrar
        if(ci->second.update_registrar_handle.isset())
        {
            BOOST_CHECK(is_system_registrar(ci->second.update_registrar_handle.get_value().second.get_value()));
        }
    }

    //nsset changes
    std::map<std::string, Fred::InfoNssetDiff> changed_nssets = diff_nssets();
    for(std::map<std::string, Fred::InfoNssetDiff>::const_iterator ci = changed_nssets.begin(); ci != changed_nssets.end(); ++ci)
    {
        //check update registrar is system registrar
        if(ci->second.update_registrar_handle.isset())
        {
            BOOST_CHECK(is_system_registrar(ci->second.update_registrar_handle.get_value().second.get_value()));
        }
    }

    //keyset changes
    std::map<std::string, Fred::InfoKeysetDiff> changed_keysets = diff_keysets();
    for(std::map<std::string, Fred::InfoKeysetDiff>::const_iterator ci = changed_keysets.begin(); ci != changed_keysets.end(); ++ci)
    {
        //check update registrar is system registrar
        if(ci->second.update_registrar_handle.isset())
        {
            BOOST_CHECK(is_system_registrar(ci->second.update_registrar_handle.get_value().second.get_value()));
        }
    }

    //domain changes
    std::map<std::string, Fred::InfoDomainDiff> changed_domains = diff_domains();
    for(std::map<std::string, Fred::InfoDomainDiff>::const_iterator ci = changed_domains.begin(); ci != changed_domains.end(); ++ci)
    {
        if(ci->second.update_registrar_handle.isset())
        {
            BOOST_CHECK(is_system_registrar(ci->second.update_registrar_handle.get_value().second.get_value()));
        }
        //BOOST_ERROR("changed_domain fqdn: " << ci->first);
    }


    /*
    //check merge
    Fred::OperationContext ctx;
    BOOST_CHECK(1 == static_cast<int>(ctx.get_conn().exec_params(
        "SELECT count(*) FROM object_registry oreg JOIN contact c ON oreg.id = c.id WHERE oreg.name like $1::text"
        , Database::query_param_list(contact_handle+"%"))[0][0]));
    */

}

BOOST_AUTO_TEST_SUITE_END();//TestContactVerification
