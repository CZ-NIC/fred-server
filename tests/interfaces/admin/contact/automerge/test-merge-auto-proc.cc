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
#include <boost/regex.hpp>

#include "time_clock.h"
#include "random_data_generator.h"

#include "setup_server_decl.h"

#include "util/printable.h"
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
        1//mergeable_contact_group_count
        , Util::set_of<unsigned>(1)(5)(9)(13)////linked_object_cases: nsset, keyset, domain via admin, domain via owner
        , init_set_of_contact_state_combinations()//all contact_state_combinations, stateless states 0, 1
        , init_set_of_linked_object_state_combinations()//all linked_object_state_combinations
        , Util::vector_of<unsigned>(1)//with one linked object per contact
        )
    {}
};

BOOST_FIXTURE_TEST_CASE( test_auto_proc_no_optional_params, auto_proc_fixture )
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

    try
    {
        Admin::MergeContactAutoProcedure(
                *(mm.get()),
                *(logger_client.get()))
        .exec();
    }
    catch(...)
    {
        BOOST_ERROR("got exception from auto procedure");
    }
}

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

    std::vector<Fred::MergeContactNotificationEmailWithAddr> nemail;

    nemail = Admin::MergeContactAutoProcedure(
            *(mm.get()),
            *(logger_client.get()))
        .set_registrar(registrar_mc_1_handle)
        .set_dry_run(true)
    .exec();

    BOOST_CHECK(nemail.size() == 0);//dry run, no notifications

    //no changes
    BOOST_CHECK(diff_contacts().empty());
    BOOST_CHECK(diff_nssets().empty());
    BOOST_CHECK(diff_keysets().empty());
    BOOST_CHECK(diff_domains().empty());
    BOOST_CHECK(diff_registrars().empty());


    nemail = Admin::MergeContactAutoProcedure(
            *(mm.get()),
            *(logger_client.get()))
        .set_registrar(registrar_mc_1_handle)
        .set_verbose(100)
        .set_dry_run(true)
    .exec();

    BOOST_CHECK(nemail.size() == 0);//dry run, no notifications

    //no changes
    BOOST_CHECK(diff_contacts().empty());
    BOOST_CHECK(diff_nssets().empty());
    BOOST_CHECK(diff_keysets().empty());
    BOOST_CHECK(diff_domains().empty());
    BOOST_CHECK(diff_registrars().empty());


    nemail = Admin::MergeContactAutoProcedure(
            *(mm.get()),
            *(logger_client.get()))
        .set_registrar(registrar_mc_1_handle)
    .exec();

    BOOST_CHECK(nemail.size() == 1);//have just 1 group of mergeable contacts with given registrar

    std::map<std::string, unsigned long long> del_contact_poll_msg = get_del_contact_poll_msg();


    //contact changes
    std::set<std::string> removed_contact_handle;
    std::map<std::string, Fred::InfoContactDiff> changed_contacts = diff_contacts();
    for(std::map<std::string, Fred::InfoContactDiff>::const_iterator ci = changed_contacts.begin(); ci != changed_contacts.end(); ++ci)
    {
        //check update registrar is system registrar
        if(ci->second.update_registrar_handle.isset())
        {
            BOOST_CHECK(is_system_registrar(ci->second.update_registrar_handle.get_value().second.get_value()));
        }

        if(ci->second.delete_time.isset())
        {//source contact

            /**
             * forbidden states of source contact
             * from fixture setup:
             * _S4 - SERVER_DELETE_PROHIBITED
             * _S5 - SERVER_BLOCKED
             * _S6 - MOJEID_CONTACT
             */
            static const  boost::regex src_contact_forbidden_states_regex("_S4|_S5|_S6");
            BOOST_CHECK(!boost::regex_match(ci->first, src_contact_forbidden_states_regex));

            //check if poll message exists for deleted contact
            BOOST_CHECK(del_contact_poll_msg.find(ci->first) != del_contact_poll_msg.end());

            removed_contact_handle.insert(ci->first);
        }
        else
        {//destination contact
            BOOST_CHECK(ci->first == nemail.at(0).email_data.dst_contact_handle);

            /**
             * forbidden state of destination contact
             * from fixture setup:
             * _S5 - SERVER_BLOCKED
             */
            static const  boost::regex dst_contact_forbidden_states_regex("_S5");
            BOOST_CHECK(!boost::regex_match(ci->first, dst_contact_forbidden_states_regex));
        }
    }
    //check all removed contacts are notified
    BOOST_CHECK(std::set<std::string>(nemail.at(0).email_data.removed_list.begin(), nemail.at(0).email_data.removed_list.end()) == removed_contact_handle);

    /**
     * forbidden states of linked object (domains are lower case) from fixture setup:
     * _LOS1   SERVER_UPDATE_PROHIBITED
     * _LOS2 - SERVER_BLOCKED
     * _LOS3 - SERVER_BLOCKED + SERVER_UPDATE_PROHIBITED
     */
    static const  boost::regex linked_object_forbidden_states_regex("_LOS1|_LOS2|_LOS3", boost::regex::icase);

    //nsset changes
    std::set<std::string> changed_nsset_handle;
    std::map<std::string, Fred::InfoNssetDiff> changed_nssets = diff_nssets();
    for(std::map<std::string, Fred::InfoNssetDiff>::const_iterator ci = changed_nssets.begin(); ci != changed_nssets.end(); ++ci)
    {
        //check update registrar is system registrar
        if(ci->second.update_registrar_handle.isset())
        {
            BOOST_CHECK(is_system_registrar(ci->second.update_registrar_handle.get_value().second.get_value()));
        }

        changed_nsset_handle.insert(ci->first);

        //check linked object for forbidden state
        BOOST_CHECK(!boost::regex_match(ci->first, linked_object_forbidden_states_regex));
    }
    //check all updated nssets are notified
    BOOST_CHECK(std::set<std::string>(nemail.at(0).email_data.nsset_tech_list.begin(), nemail.at(0).email_data.nsset_tech_list.end()) == changed_nsset_handle);

    //keyset changes
    std::set<std::string> changed_keyset_handle;
    std::map<std::string, Fred::InfoKeysetDiff> changed_keysets = diff_keysets();
    for(std::map<std::string, Fred::InfoKeysetDiff>::const_iterator ci = changed_keysets.begin(); ci != changed_keysets.end(); ++ci)
    {
        //check update registrar is system registrar
        if(ci->second.update_registrar_handle.isset())
        {
            BOOST_CHECK(is_system_registrar(ci->second.update_registrar_handle.get_value().second.get_value()));
        }
        changed_keyset_handle.insert(ci->first);

        //check linked object for forbidden state
        BOOST_CHECK(!boost::regex_match(ci->first, linked_object_forbidden_states_regex));
    }
    //check all updated keysets are notified
    BOOST_CHECK(std::set<std::string>(nemail.at(0).email_data.keyset_tech_list.begin(), nemail.at(0).email_data.keyset_tech_list.end()) == changed_keyset_handle);

    //domain changes
    std::set<std::string> changed_domain_admin_fqdn;
    std::set<std::string> changed_domain_owner_fqdn;
    std::map<std::string, Fred::InfoDomainDiff> changed_domains = diff_domains();
    for(std::map<std::string, Fred::InfoDomainDiff>::const_iterator ci = changed_domains.begin(); ci != changed_domains.end(); ++ci)
    {
        if(ci->second.update_registrar_handle.isset())
        {
            BOOST_CHECK(is_system_registrar(ci->second.update_registrar_handle.get_value().second.get_value()));
        }

        if(ci->second.admin_contacts.isset()) changed_domain_admin_fqdn.insert(ci->first);
        if(ci->second.registrant.isset()) changed_domain_owner_fqdn.insert(ci->first);
        //BOOST_ERROR("changed_domain fqdn: " << ci->first);

        //check linked object for forbidden state
        BOOST_CHECK(!boost::regex_match(ci->first, linked_object_forbidden_states_regex));
    }

    BOOST_CHECK(std::set<std::string>(nemail.at(0).email_data.domain_admin_list.begin(), nemail.at(0).email_data.domain_admin_list.end()) == changed_domain_admin_fqdn);
    BOOST_CHECK(std::set<std::string>(nemail.at(0).email_data.domain_registrant_list.begin(), nemail.at(0).email_data.domain_registrant_list.end()) == changed_domain_owner_fqdn);

}

BOOST_AUTO_TEST_SUITE_END();//TestContactVerification
