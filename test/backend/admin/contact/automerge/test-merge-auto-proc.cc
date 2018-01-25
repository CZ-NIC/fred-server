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

#include <iostream>
#include <memory>
#include <string>
#include <algorithm>
#include <functional>
#include <map>
#include <exception>
#include <sys/time.h>
#include <time.h>

#include <boost/regex.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/null.hpp>

#include "src/util/printable.hh"
#include "src/util/db/query_param.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"


//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include "src/util/cfg/config_handler_decl.hh"
#include <boost/test/unit_test.hpp>
#include <utility>
#include "src/util/log/logger.hh"
#include "src/util/corba_wrapper_decl.hh"
#include "src/bin/corba/mailer_manager.hh"
#include "src/backend/admin/contact/merge_contact_auto_procedure.hh"
#include "test/mockup/logger_client_dummy.hh"

#include "src/libfred/registrable_object/contact/create_contact.hh"
#include "src/libfred/object/object_states_info.hh"
#include "src/libfred/object_state/get_object_states.hh"
#include "src/libfred/registrar/create_registrar.hh"
#include "src/libfred/registrar/info_registrar.hh"

#include "test/libfred/contact/util.hh"
#include "test/libfred/contact/fixture.hh"
#include "test/libfred/contact/test_merge_contact_fixture.hh"

namespace Test {

BOOST_AUTO_TEST_SUITE(Backend)
BOOST_AUTO_TEST_SUITE(Admin)
BOOST_AUTO_TEST_SUITE(Contact)
BOOST_AUTO_TEST_SUITE(MergeContactAutoProcedure)

const std::string server_name = "test-merge-contact-auto-proc";


boost::iostreams::stream<boost::iostreams::null_sink> null_stream((boost::iostreams::null_sink()));


/**
 * Setup merge contact test data with states.
 */
struct auto_proc_fixture : Test::LibFred::Contact::MergeContactAutoProc::mergeable_contact_grps_with_linked_objects_and_blocking_states
{
    auto_proc_fixture(const std::string& db_name_suffix = "")
    : Test::LibFred::Contact::MergeContactAutoProc::mergeable_contact_grps_with_linked_objects_and_blocking_states(
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
BOOST_FIXTURE_TEST_CASE(test_auto_proc_dry_run, auto_proc_fixture)
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

    std::shared_ptr<::LibFred::Mailer::Manager> mm( new MailerManager(CorbaContainer::get_instance()->getNS()));
    std::unique_ptr<::LibFred::Logger::LoggerClient> logger_client(
            new ::LibFred::Logger::DummyLoggerCorbaClientImpl());

    std::vector<::LibFred::MergeContactNotificationEmailWithAddr> notification_email;

    notification_email = ::Admin::MergeContactAutoProcedure(
            *(mm.get()),
            *(logger_client.get()),
            registrar_mc_1_handle)
        .set_dry_run(true)
    .exec(null_stream);

    BOOST_CHECK(notification_email.empty());//no notifications

    notification_email = ::Admin::MergeContactAutoProcedure(
            *(mm.get()),
            *(logger_client.get()),
            registrar_mc_1_handle)
        .set_verbose(100)
        .set_dry_run(true)
    .exec(null_stream);

    BOOST_CHECK(notification_email.empty());//no notifications

}

/**
 * check merge with given registrar and no optional parameters
 *  - check that merged objects have selected registrar and objects with other registrar are not changed
 *  - check that update registrar of merged objects is system registrar
 *  - check that source contacts with object states (any of) SERVER_DELETE_PROHIBITED, SERVER_BLOCKED, MOJEID_CONTACT, CONTACT_IN_MANUAL_VERIFICATION, CONTACT_FAILED_MANUAL_VERIFICATION are not changed
 *  - check that destination contacts with object states (any of) SERVER_BLOCKED, CONTACT_IN_MANUAL_VERIFICATION, CONTACT_FAILED_MANUAL_VERIFICATION are not changed
 *  - check that linked objects with object states SERVER_UPDATE_PROHIBITED, SERVER_BLOCKED and SERVER_BLOCKED + SERVER_UPDATE_PROHIBITED are not changed
 *  - check that poll messages exists for deleted source contacts
 *  - check that deleted source contacts are present in notification data
 *  - check that changed objects are present in notification data
 */
BOOST_FIXTURE_TEST_CASE(test_auto_proc, auto_proc_fixture)
{
    ::LibFred::OperationContextCreator ctx;
    //corba config
    FakedArgs fa = CfgArgs::instance()->fa;
    //conf pointers
    HandleCorbaNameServiceArgs* ns_args_ptr=CfgArgs::instance()->
                get_handler_ptr_by_type<HandleCorbaNameServiceArgs>();

    CorbaContainer::set_instance(fa.get_argc(), fa.get_argv()
            , ns_args_ptr->nameservice_host
            , ns_args_ptr->nameservice_port
            , ns_args_ptr->nameservice_context);

    std::shared_ptr<::LibFred::Mailer::Manager> mm( new MailerManager(CorbaContainer::get_instance()->getNS()));
    std::unique_ptr<::LibFred::Logger::LoggerClient> logger_client(
            new ::LibFred::Logger::DummyLoggerCorbaClientImpl());

    std::vector<::LibFred::MergeContactNotificationEmailWithAddr> notification_email;

    notification_email = ::Admin::MergeContactAutoProcedure(
            *(mm.get()),
            *(logger_client.get()),
            registrar_mc_1_handle)
    .exec(null_stream);

    BOOST_CHECK(notification_email.size() == 1);//have just 1 group of mergeable contacts with given registrar

    std::map<std::string, unsigned long long> del_contact_poll_msg = get_del_contact_poll_msg();

    /**
     * selected registrar of objects that should be changed within the same mergeable group (domains are lower case) from fixture setup:
     * REG1
     */
    static const  boost::regex selected_registrar_regex(registrar_mc_1_handle, boost::regex::icase);

    /**
     * the other registrar of objects that shouldn't be changed within the same mergeable group (domains are lower case) from fixture setup:
     * REG2
     */
    static const  boost::regex forbidden_registrar_regex(registrar_mc_2_handle, boost::regex::icase);


    //contact changes
    std::set<std::string> removed_contact_handle;
    std::map<std::string, ::LibFred::InfoContactDiff> changed_contacts = diff_contacts();

    for(std::map<std::string, ::LibFred::InfoContactDiff>::const_iterator ci = changed_contacts.begin(); ci != changed_contacts.end(); ++ci)
    {
        BOOST_TEST_MESSAGE("DEBUG " << ci->first);

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
             * ST4 - SERVER_DELETE_PROHIBITED
             * ST5 - SERVER_BLOCKED
             * ST6 - MOJEID_CONTACT
             * ST7 - CONTACT_IN_MANUAL_VERIFICATION
             * ST8 - CONTACT_FAILED_MANUAL_VERIFICATION
             */
            static const  boost::regex src_contact_forbidden_states_regex("-ST4|-ST5|-ST6|-ST7|-ST8");
            BOOST_CHECK(!boost::regex_search(ci->first, src_contact_forbidden_states_regex));

            //check if poll message exists for deleted contact
            BOOST_CHECK_MESSAGE(del_contact_poll_msg.find(ci->first) != del_contact_poll_msg.end(), ci->first);

            removed_contact_handle.insert(ci->first);
        }
        else
        {//destination contact

            BOOST_CHECK(ci->first == notification_email.at(0).email_data.dst_contact_handle);

            /**
             * forbidden state of destination contact
             * from fixture setup:
             * ST5 - SERVER_BLOCKED
             * ST7 - CONTACT_IN_MANUAL_VERIFICATION
             * ST8 - CONTACT_FAILED_MANUAL_VERIFICATION
             */
            static const  boost::regex dst_contact_forbidden_states_regex("-ST5|-ST7|-ST8");
            BOOST_CHECK(!boost::regex_search(ci->first, dst_contact_forbidden_states_regex));
        }

        BOOST_CHECK_MESSAGE(boost::regex_search(ci->first, selected_registrar_regex)
            , "changed contact  ci->first: " << ci->first << " selected_registrar_regex: " << selected_registrar_regex.str());

        //check that objects of other registrar are not changed
        BOOST_CHECK_MESSAGE(!boost::regex_search(ci->first, forbidden_registrar_regex)
            , "changed contact  ci->first: " << ci->first << " forbidden_registrar_regex: " << forbidden_registrar_regex.str());
    }

    //check all removed contacts are notified
    BOOST_CHECK(std::set<std::string>(notification_email.at(0).email_data.removed_list.begin(), notification_email.at(0).email_data.removed_list.end()) == removed_contact_handle);

    /**
     * forbidden states of linked object (domains are lower case) from fixture setup:
     * -LS1   SERVER_UPDATE_PROHIBITED
     * -LS2 - SERVER_BLOCKED
     * -LS3 - SERVER_BLOCKED + SERVER_UPDATE_PROHIBITED
     */
    static const  boost::regex linked_domain_forbidden_states_regex("-ls1|-ls2|-ls3");
    static const  boost::regex linked_nsset_keyset_forbidden_states_regex("-LS1|-LS3");

    //nsset changes
    std::set<std::string> changed_nsset_handle;
    std::map<std::string, ::LibFred::InfoNssetDiff> changed_nssets = diff_nssets();
    for(std::map<std::string, ::LibFred::InfoNssetDiff>::const_iterator ci = changed_nssets.begin(); ci != changed_nssets.end(); ++ci)
    {
        //check update registrar is system registrar
        if(ci->second.update_registrar_handle.isset())
        {
            BOOST_CHECK(is_system_registrar(ci->second.update_registrar_handle.get_value().second.get_value()));
        }

        changed_nsset_handle.insert(ci->first);

        //check linked object for forbidden state
        BOOST_CHECK_MESSAGE(!boost::regex_search(ci->first, linked_nsset_keyset_forbidden_states_regex)
            , "changed nsset  ci->first: " << ci->first << " linked_nsset_keyset_forbidden_states_regex: " << linked_nsset_keyset_forbidden_states_regex.str());

        BOOST_CHECK_MESSAGE(boost::regex_search(ci->first, selected_registrar_regex)
            , "changed nsset  ci->first: " << ci->first << " selected_registrar_regex: " << selected_registrar_regex.str());

        //check that objects of other registrar are not changed
        BOOST_CHECK_MESSAGE(!boost::regex_search(ci->first, forbidden_registrar_regex)
            , "changed nsset  ci->first: " << ci->first << " forbidden_registrar_regex: " << forbidden_registrar_regex.str());
    }
    //check all updated nssets are notified
    BOOST_CHECK(std::set<std::string>(notification_email.at(0).email_data.nsset_tech_list.begin(), notification_email.at(0).email_data.nsset_tech_list.end()) == changed_nsset_handle);

    //keyset changes
    std::set<std::string> changed_keyset_handle;
    std::map<std::string, ::LibFred::InfoKeysetDiff> changed_keysets = diff_keysets();
    for(std::map<std::string, ::LibFred::InfoKeysetDiff>::const_iterator ci = changed_keysets.begin(); ci != changed_keysets.end(); ++ci)
    {
        //check update registrar is system registrar
        if(ci->second.update_registrar_handle.isset())
        {
            BOOST_CHECK(is_system_registrar(ci->second.update_registrar_handle.get_value().second.get_value()));
        }
        changed_keyset_handle.insert(ci->first);

        //check linked object for forbidden state
        BOOST_CHECK_MESSAGE(!boost::regex_search(ci->first, linked_nsset_keyset_forbidden_states_regex)
            , "changed keyset  ci->first: " << ci->first << " linked_nsset_keyset_forbidden_states_regex: " << linked_nsset_keyset_forbidden_states_regex.str());

        BOOST_CHECK_MESSAGE(boost::regex_search(ci->first, selected_registrar_regex)
            , "changed keyset  ci->first: " << ci->first << " selected_registrar_regex: " << selected_registrar_regex.str());

        //check that objects of other registrar are not changed
        BOOST_CHECK_MESSAGE(!boost::regex_search(ci->first, forbidden_registrar_regex)
        , "changed keyset  ci->first: " << ci->first << " forbidden_registrar_regex: " << forbidden_registrar_regex.str());
    }

    //check all updated keysets are notified
    BOOST_CHECK(std::set<std::string>(notification_email.at(0).email_data.keyset_tech_list.begin(), notification_email.at(0).email_data.keyset_tech_list.end()) == changed_keyset_handle);

    //domain changes
    std::set<std::string> changed_domain_admin_fqdn;
    std::set<std::string> changed_domain_owner_fqdn;
    std::map<std::string, ::LibFred::InfoDomainDiff> changed_domains = diff_domains();
    for(std::map<std::string, ::LibFred::InfoDomainDiff>::const_iterator ci = changed_domains.begin(); ci != changed_domains.end(); ++ci)
    {
        if(ci->second.update_registrar_handle.isset())
        {
            BOOST_CHECK(is_system_registrar(ci->second.update_registrar_handle.get_value().second.get_value()));
        }

        if(ci->second.admin_contacts.isset()) changed_domain_admin_fqdn.insert(ci->first);
        if(ci->second.registrant.isset()) changed_domain_owner_fqdn.insert(ci->first);

        //check linked object for forbidden state
        BOOST_CHECK_MESSAGE(!boost::regex_search(ci->first, linked_domain_forbidden_states_regex)
            , "changed domain  ci->first: " << ci->first << " linked_object_forbidden_states_regex: " << linked_domain_forbidden_states_regex.str());

        BOOST_CHECK_MESSAGE(boost::regex_search(ci->first, selected_registrar_regex)
            , "changed domain  ci->first: " << ci->first << " selected_registrar_regex: " << selected_registrar_regex.str());

        //check that objects of other registrar are not changed
        BOOST_CHECK_MESSAGE(!boost::regex_search(ci->first, forbidden_registrar_regex)
        , "changed domain  ci->first: " << ci->first << " forbidden_registrar_regex: " << forbidden_registrar_regex.str());
    }

    BOOST_CHECK(std::set<std::string>(notification_email.at(0).email_data.domain_admin_list.begin(), notification_email.at(0).email_data.domain_admin_list.end()) == changed_domain_admin_fqdn);
    BOOST_CHECK(std::set<std::string>(notification_email.at(0).email_data.domain_registrant_list.begin(), notification_email.at(0).email_data.domain_registrant_list.end()) == changed_domain_owner_fqdn);
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

    std::shared_ptr<::LibFred::Mailer::Manager> mm( new MailerManager(CorbaContainer::get_instance()->getNS()));
    std::unique_ptr<::LibFred::Logger::LoggerClient> logger_client(
            new ::LibFred::Logger::DummyLoggerCorbaClientImpl());

    //non verbose test data
    std::vector<::LibFred::MergeContactNotificationEmailWithAddr> quiet_notification_emails;
    std::map<std::string, ::LibFred::InfoContactDiff> contact_changes1;
    std::map<std::string, ::LibFred::InfoNssetDiff> nsset_changes1;
    std::map<std::string, ::LibFred::InfoKeysetDiff> keyset_changes1;
    std::map<std::string, ::LibFred::InfoDomainDiff> domain_changes1;

    //verbose test data
    std::vector<::LibFred::MergeContactNotificationEmailWithAddr> verbose_notification_emails;
    std::map<std::string, ::LibFred::InfoContactDiff> contact_changes2;
    std::map<std::string, ::LibFred::InfoNssetDiff> nsset_changes2;
    std::map<std::string, ::LibFred::InfoKeysetDiff> keyset_changes2;
    std::map<std::string, ::LibFred::InfoDomainDiff> domain_changes2;

    /**
     * non-verbose merge
     */
    {
        auto_proc_fixture fixture("_1");

        quiet_notification_emails = ::Admin::MergeContactAutoProcedure(
            *(mm.get()),
            *(logger_client.get()),
            fixture.registrar_mc_1_handle)
        .exec(null_stream);

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

        verbose_notification_emails = ::Admin::MergeContactAutoProcedure(
            *(mm.get()),
            *(logger_client.get()),
            fixture.registrar_mc_1_handle)
        .set_verbose(100)
        .exec(null_stream);

        //data changes
        contact_changes2 = fixture.diff_contacts();
        nsset_changes2 = fixture.diff_nssets();
        keyset_changes2 = fixture.diff_keysets();
        domain_changes2 = fixture.diff_domains();

        BOOST_CHECK(fixture.diff_registrars().empty());
    }

  //comparison
  for(unsigned i = 0; i < quiet_notification_emails.size(); ++i)
  {
      BOOST_CHECK(quiet_notification_emails.at(i).notification_email_addr == verbose_notification_emails.at(i).notification_email_addr);
      BOOST_CHECK(quiet_notification_emails.at(i).email_data.to_string() == verbose_notification_emails.at(i).email_data.to_string());
  }

  for(std::map<std::string, ::LibFred::InfoContactDiff>::const_iterator ci = contact_changes1.begin(); ci != contact_changes1.end(); ++ci)
  {
      ::LibFred::InfoContactDiff contact_diff;
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

  for(std::map<std::string, ::LibFred::InfoNssetDiff>::const_iterator ci = nsset_changes1.begin(); ci != nsset_changes1.end(); ++ci)
  {
      ::LibFred::InfoNssetDiff nsset_diff;
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

  for(std::map<std::string, ::LibFred::InfoKeysetDiff>::const_iterator ci = keyset_changes1.begin(); ci != keyset_changes1.end(); ++ci)
  {
      ::LibFred::InfoKeysetDiff keyset_diff;
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

  for(std::map<std::string, ::LibFred::InfoDomainDiff>::const_iterator ci = domain_changes1.begin(); ci != domain_changes1.end(); ++ci)
  {
      ::LibFred::InfoDomainDiff domain_diff;
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

BOOST_FIXTURE_TEST_CASE(test_keep_contact_states, Test::LibFred::Contact::supply_ctx<Test::LibFred::Contact::HasRegistrarWithContactWithPassedManualVerification>)
{
    Test::LibFred::Contact::Contact contact(ctx, registrar.data.handle); // MCS_FILTER_RECENTLY_CREATED should prioritize this contact against the contact with contactPassedManualVerification status

    //corba config
    FakedArgs fa = CfgArgs::instance()->fa;
    //conf pointers
    HandleCorbaNameServiceArgs* ns_args_ptr =
            CfgArgs::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgs>();

    CorbaContainer::set_instance(
            fa.get_argc(),
            fa.get_argv(),
            ns_args_ptr->nameservice_host,
            ns_args_ptr->nameservice_port,
            ns_args_ptr->nameservice_context);

    const boost::shared_ptr<::LibFred::Mailer::Manager> mm(new MailerManager(CorbaContainer::get_instance()->getNS()));
    const std::unique_ptr<::LibFred::Logger::LoggerClient> logger_client(new ::LibFred::Logger::DummyLoggerCorbaClientImpl());

    std::vector<::LibFred::MergeContactNotificationEmailWithAddr> notification_email;

    commit_transaction();

    notification_email =
            ::Admin::MergeContactAutoProcedure(
                    *(mm.get()),
                    *(logger_client.get()),
                    registrar.data.handle)
                    .exec(null_stream);
                  //.set_verbose(3).exec(std::cout);

    ::LibFred::OperationContextCreator ctx;

    const ::LibFred::ObjectStatesInfo contact_states(::LibFred::GetObjectStates(contact.data.id).exec(ctx));
    BOOST_CHECK(contact_states.presents(::LibFred::Object_State::contact_passed_manual_verification));
}

BOOST_AUTO_TEST_SUITE_END(); // Backend::Admin::Contact::MergeContactAutoProcedure
BOOST_AUTO_TEST_SUITE_END(); // Backend::Admin::Contact
BOOST_AUTO_TEST_SUITE_END(); // Backend::Admin
BOOST_AUTO_TEST_SUITE_END(); // Backend

} // namespace Test