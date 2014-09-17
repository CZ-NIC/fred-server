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
#include "tests/mockup/logger_client_dummy.h"

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
    auto_proc_fixture(const std::string& db_name_suffix = "")
    : MergeContactFixture::mergeable_contact_grps_with_linked_objects_and_blocking_states(
        db_name_suffix
        , 1//mergeable_contact_group_count
        , Util::set_of<unsigned>(1)(5)(9)(13)////linked_object_cases: nsset, keyset, domain via admin, domain via owner
        , init_set_of_contact_state_combinations()//all contact_state_combinations, stateless states 0, 1
        , init_set_of_linked_object_state_combinations()//all linked_object_state_combinations
        , Util::vector_of<unsigned>(1)//with one linked object per contact
        )
    {}
};

/**
 * check that dry run do no changes
 */
BOOST_FIXTURE_TEST_CASE( test_auto_proc_dry_run, auto_proc_fixture )
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
            new Fred::Logger::DummyLoggerCorbaClientImpl());

    std::vector<Fred::MergeContactNotificationEmailWithAddr> nemail;

    nemail = Admin::MergeContactAutoProcedure(
            *(mm.get()),
            *(logger_client.get()),
            registrar_mc_1_handle)
        .set_dry_run(true)
    .exec();

    BOOST_CHECK(nemail.empty());//no notifications

    //no changes
    BOOST_CHECK(diff_contacts().empty());
    BOOST_CHECK(diff_nssets().empty());
    BOOST_CHECK(diff_keysets().empty());
    BOOST_CHECK(diff_domains().empty());
    BOOST_CHECK(diff_registrars().empty());


    nemail = Admin::MergeContactAutoProcedure(
            *(mm.get()),
            *(logger_client.get()),
            registrar_mc_1_handle)
        .set_verbose(100)
        .set_dry_run(true)
    .exec();

    BOOST_CHECK(nemail.empty());//no notifications

    //no changes
    BOOST_CHECK(diff_contacts().empty());
    BOOST_CHECK(diff_nssets().empty());
    BOOST_CHECK(diff_keysets().empty());
    BOOST_CHECK(diff_domains().empty());
    BOOST_CHECK(diff_registrars().empty());

}
/**
 * check merge with given registrar and no optional parameters
 *  - check that merged objects have selected registrar and objects with other registrar are not changed
 *  - check that update registrar of merged objects is system registrar
 *  - check that source contacts with object states SERVER_DELETE_PROHIBITED, SERVER_BLOCKED and MOJEID_CONTACT are not changed
 *  - check that destination contacts with object state SERVER_BLOCKED are not changed
 *  - check that linked objects with object states SERVER_UPDATE_PROHIBITED, SERVER_BLOCKED and SERVER_BLOCKED + SERVER_UPDATE_PROHIBITED are not changed
 *  - check that poll messages exists for deleted source contacts
 *  - check that deleted source contacts are present in notification data
 *  - check that changed objects are present in notification data
 */
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
            new Fred::Logger::DummyLoggerCorbaClientImpl());

    std::vector<Fred::MergeContactNotificationEmailWithAddr> nemail;

    nemail = Admin::MergeContactAutoProcedure(
            *(mm.get()),
            *(logger_client.get()),
            registrar_mc_1_handle)
    .exec();

    BOOST_CHECK(nemail.size() == 1);//have just 1 group of mergeable contacts with given registrar

    std::map<std::string, unsigned long long> del_contact_poll_msg = get_del_contact_poll_msg();

    /**
     * the other registrar of objects that shouldn't be changed within the same mergeable group (domains are lower case) from fixture setup:
     * REG-2
     */
    static const  boost::regex forbidden_registrar_regex("_"+registrar_mc_2_handle, boost::regex::icase);

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

        //check that objects of other registrar are not changed
        BOOST_CHECK(!boost::regex_match(ci->first, forbidden_registrar_regex));
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

        //check that objects of other registrar are not changed
        BOOST_CHECK(!boost::regex_match(ci->first, forbidden_registrar_regex));
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

        //check that objects of other registrar are not changed
        BOOST_CHECK(!boost::regex_match(ci->first, forbidden_registrar_regex));
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

        //check linked object for forbidden state
        BOOST_CHECK(!boost::regex_match(ci->first, linked_object_forbidden_states_regex));

        //check that objects of other registrar are not changed
        BOOST_CHECK(!boost::regex_match(ci->first, forbidden_registrar_regex));
    }

    BOOST_CHECK(std::set<std::string>(nemail.at(0).email_data.domain_admin_list.begin(), nemail.at(0).email_data.domain_admin_list.end()) == changed_domain_admin_fqdn);
    BOOST_CHECK(std::set<std::string>(nemail.at(0).email_data.domain_registrant_list.begin(), nemail.at(0).email_data.domain_registrant_list.end()) == changed_domain_owner_fqdn);
}


/**
 * check that merge with set verbosity have the same effect as merge with no verbosity set
 * except of timestamps of changes and random authinfo changes
 */
BOOST_AUTO_TEST_CASE(test_compare_verbose)
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
            new Fred::Logger::DummyLoggerCorbaClientImpl());

    //non verbose test data
    std::vector<Fred::MergeContactNotificationEmailWithAddr> nemail1;
    std::map<std::string, Fred::InfoContactDiff> contact_changes1;
    std::map<std::string, Fred::InfoNssetDiff> nsset_changes1;
    std::map<std::string, Fred::InfoKeysetDiff> keyset_changes1;
    std::map<std::string, Fred::InfoDomainDiff> domain_changes1;

    //verbose test data
    std::vector<Fred::MergeContactNotificationEmailWithAddr> nemail2;
    std::map<std::string, Fred::InfoContactDiff> contact_changes2;
    std::map<std::string, Fred::InfoNssetDiff> nsset_changes2;
    std::map<std::string, Fred::InfoKeysetDiff> keyset_changes2;
    std::map<std::string, Fred::InfoDomainDiff> domain_changes2;

    /**
     * non-verbose merge
     */
    {
        auto_proc_fixture fixture("_1");

        nemail1 = Admin::MergeContactAutoProcedure(
            *(mm.get()),
            *(logger_client.get()),
            fixture.registrar_mc_1_handle)
        .exec();

        //data changes
        contact_changes1 = fixture.diff_contacts();
        nsset_changes1 = fixture.diff_nssets();
        keyset_changes1 = fixture.diff_keysets();
        domain_changes1 = fixture.diff_domains();

        BOOST_CHECK(fixture.diff_registrars().empty());
    }

    /**
     * verbose merge
     */
    {
        auto_proc_fixture fixture("_2");

        nemail2 = Admin::MergeContactAutoProcedure(
            *(mm.get()),
            *(logger_client.get()),
            fixture.registrar_mc_1_handle)
        .set_verbose(100)
        .exec();

        //data changes
        contact_changes2 = fixture.diff_contacts();
        nsset_changes2 = fixture.diff_nssets();
        keyset_changes2 = fixture.diff_keysets();
        domain_changes2 = fixture.diff_domains();

        BOOST_CHECK(fixture.diff_registrars().empty());
    }

  //comparison
  for(unsigned i = 0 ; i < nemail1.size(); ++i)
  {
      BOOST_CHECK(nemail1.at(i).notification_email_addr == nemail2.at(i).notification_email_addr);
      BOOST_CHECK(nemail1.at(i).email_data.to_string() == nemail2.at(i).email_data.to_string());
  }

  for(std::map<std::string, Fred::InfoContactDiff>::const_iterator ci = contact_changes1.begin(); ci != contact_changes1.end(); ++ci)
  {
      Fred::InfoContactDiff contact_diff;
      try
      {
          contact_diff = map_at(contact_changes2, ci->first);

          //except of timestamp and authinfo
          if(contact_diff.delete_time.isset())
          {
              BOOST_CHECK(contact_diff.delete_time.get_value().first.isnull());
              BOOST_CHECK(!contact_diff.delete_time.get_value().second.isnull());
              BOOST_CHECK(ci->second.delete_time.get_value().first.isnull());
              BOOST_CHECK(!ci->second.delete_time.get_value().second.isnull());
              contact_diff.delete_time = ci->second.delete_time;
          }

          if(contact_diff.update_time.isset())
          {
              BOOST_CHECK(contact_diff.update_time.get_value().first.isnull());
              BOOST_CHECK(!contact_diff.update_time.get_value().second.isnull());
              BOOST_CHECK(ci->second.update_time.get_value().first.isnull());
              BOOST_CHECK(!ci->second.update_time.get_value().second.isnull());
              contact_diff.update_time = ci->second.update_time;
          }

          if(contact_diff.authinfopw.isset())
          {
              contact_diff.authinfopw = ci->second.authinfopw;
          }

          BOOST_CHECK(ci->second.to_string() == contact_diff.to_string());

          if(ci->second.to_string() != contact_diff.to_string())
          {
              BOOST_ERROR(ci->second.to_string());
              BOOST_ERROR(contact_diff.to_string());
          }
      }
      catch(const std::exception& ex)
      {
          BOOST_ERROR(ex.what());
      }
  }

  for(std::map<std::string, Fred::InfoNssetDiff>::const_iterator ci = nsset_changes1.begin(); ci != nsset_changes1.end(); ++ci)
  {
      Fred::InfoNssetDiff nsset_diff;
      try
      {
          nsset_diff = map_at(nsset_changes2, ci->first);
          //except of timestamp
          if(nsset_diff.update_time.isset())
          {
              BOOST_CHECK(nsset_diff.update_time.get_value().first.isnull());
              BOOST_CHECK(!nsset_diff.update_time.get_value().second.isnull());
              BOOST_CHECK(ci->second.update_time.get_value().first.isnull());
              BOOST_CHECK(!ci->second.update_time.get_value().second.isnull());
              nsset_diff.update_time = ci->second.update_time;
          }

          BOOST_CHECK(ci->second.to_string() == nsset_diff.to_string());

          if(ci->second.to_string() != nsset_diff.to_string())
          {
              BOOST_ERROR(ci->second.to_string());
              BOOST_ERROR(nsset_diff.to_string());
          }
      }
      catch(const std::exception& ex)
      {
          BOOST_ERROR(ex.what());
      }
  }

  for(std::map<std::string, Fred::InfoKeysetDiff>::const_iterator ci = keyset_changes1.begin(); ci != keyset_changes1.end(); ++ci)
  {
      Fred::InfoKeysetDiff keyset_diff;
      try
      {
          keyset_diff = map_at(keyset_changes2, ci->first);
          //except of timestamp
          if(keyset_diff.update_time.isset())
          {
              BOOST_CHECK(keyset_diff.update_time.get_value().first.isnull());
              BOOST_CHECK(!keyset_diff.update_time.get_value().second.isnull());
              BOOST_CHECK(ci->second.update_time.get_value().first.isnull());
              BOOST_CHECK(!ci->second.update_time.get_value().second.isnull());
              keyset_diff.update_time = ci->second.update_time;
          }

          BOOST_CHECK(ci->second.to_string() == keyset_diff.to_string());

          if(ci->second.to_string() != keyset_diff.to_string())
          {
              BOOST_ERROR(ci->second.to_string());
              BOOST_ERROR(keyset_diff.to_string());
          }
      }
      catch(const std::exception& ex)
      {
          BOOST_ERROR(ex.what());
      }
  }

  for(std::map<std::string, Fred::InfoDomainDiff>::const_iterator ci = domain_changes1.begin(); ci != domain_changes1.end(); ++ci)
  {
      Fred::InfoDomainDiff domain_diff;
      try
      {
          domain_diff = map_at(domain_changes2, ci->first);
          //except of timestamp
          if(domain_diff.update_time.isset())
          {
              BOOST_CHECK(domain_diff.update_time.get_value().first.isnull());
              BOOST_CHECK(!domain_diff.update_time.get_value().second.isnull());
              BOOST_CHECK(ci->second.update_time.get_value().first.isnull());
              BOOST_CHECK(!ci->second.update_time.get_value().second.isnull());
              domain_diff.update_time = ci->second.update_time;
          }

          BOOST_CHECK(ci->second.to_string() == domain_diff.to_string());

          if(ci->second.to_string() != domain_diff.to_string())
          {
              BOOST_ERROR(ci->second.to_string());
              BOOST_ERROR(domain_diff.to_string());
          }
      }
      catch(const std::exception& ex)
      {
          BOOST_ERROR(ex.what());
      }
  }

}


BOOST_AUTO_TEST_SUITE_END();
