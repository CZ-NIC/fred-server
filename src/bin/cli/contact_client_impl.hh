/*
 * Copyright (C) 2011  CZ.NIC, z.s.p.o.
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

/**
 *  @contact_client_impl.h
 *  contact client implementation header
 */

#ifndef CONTACT_CLIENT_IMPL_HH_24044019110A40949F068BFB640C2404
#define CONTACT_CLIENT_IMPL_HH_24044019110A40949F068BFB640C2404

#include "src/backend/admin/contact/merge_contact.hh"
#include "src/backend/admin/contact/merge_contact_auto_procedure.hh"
#include "src/backend/admin/contact/merge_contact_reporting.hh"
#include "src/backend/admin/contact/verification/create_test_impl_prototypes.hh"
#include "src/backend/admin/contact/verification/delete_domains_of_invalid_contact.hh"
#include "src/backend/admin/contact/verification/enqueue_check.hh"
#include "src/backend/admin/contact/verification/exceptions.hh"
#include "src/backend/admin/contact/verification/fill_check_queue.hh"
#include "src/backend/admin/contact/verification/list_active_checks.hh"
#include "src/backend/admin/contact/verification/related_records.hh"
#include "src/backend/admin/contact/verification/resolve_check.hh"
#include "src/backend/admin/contact/verification/run_all_enqueued_checks.hh"
#include "src/backend/admin/contact/verification/update_tests.hh"
#include "src/bin/cli/commonclient.hh"
#include "src/bin/cli/contactclient.hh"
#include "src/bin/cli/handle_adminclientselection_args.hh"
#include "src/bin/corba/logger_client_impl.hh"
#include "src/deprecated/libfred/documents.hh"
#include "libfred/mailer.hh"
#include "src/deprecated/libfred/messages/messages_impl.hh"
#include "libfred/registrar/get_registrar_handles.hh"
#include "src/deprecated/libfred/reminder.hh"
#include "src/util/cfg/config_handler_decl.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"
#include "src/util/cfg/handle_database_args.hh"
#include "src/util/cfg/handle_registry_args.hh"
#include "util/log/context.hh"

#include <boost/algorithm/string.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/foreach.hpp>
/**
 * \class contact_list_impl
 * \brief admin client implementation of contact_list
 */
struct contact_list_impl
{
  void operator()() const
  {
      Logging::Context ctx("contact_list_impl");
      Admin::ContactClient contact_client(
              CfgArgGroups::instance()->get_handler_ptr_by_type<HandleDatabaseArgsGrp>()->get_conn_info()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_host_port()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_context()
              , true //contact_list
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientContactListArgsGrp>()->params
              );
      contact_client.runMethod();
      return ;
  }
};

/**
 * \class contact_list_impl
 * \brief admin client implementation of contact_list
 */
struct contact_reminder_impl
{
  void operator()() const
  {
      Logging::Context ctx("contact_reminder_impl");

      CorbaClient cc(
              0, // argc
              0, // argv
              CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_host_port(),
              CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_context());
      MailerManager mailer(cc.getNS());

      if (CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientContactReminderArgsGrp>()->date.is_value_set())
      {
          LibFred::run_reminder(
                  &mailer,
                  boost::gregorian::from_string(
                      CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientContactReminderArgsGrp>()->date.get_value()));
      }
      else
      {
          LibFred::run_reminder(&mailer);
      }

      return;
  }
};

/**
 * \class contact_merge_duplicate_auto_impl
 * \brief functor to run automatic contact duplicates merge procedure
 */
struct contact_merge_duplicate_auto_impl
{
    void operator()() const
    {
        Logging::Context ctx("contact_merge_duplicate_auto_impl");


        FakedArgs orb_fa = CfgArgGroups::instance()->fa;

        /* prepare logger corba client */
        HandleCorbaNameServiceArgsGrp* ns_args_ptr=CfgArgGroups::instance()->
                   get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>();

        CorbaContainer::set_instance(orb_fa.get_argc(), orb_fa.get_argv()
               , ns_args_ptr->get_nameservice_host()
               , ns_args_ptr->get_nameservice_port()
               , ns_args_ptr->get_nameservice_context());

        std::unique_ptr<LibFred::Logger::LoggerClient> logger_client(
                new LibFred::Logger::LoggerCorbaClientImpl());

        std::unique_ptr<LibFred::Mailer::Manager> mm(
                new MailerManager(CorbaContainer::get_instance()->getNS()));

        if (!logger_client.get()) {
            throw std::runtime_error("unable to get request logger reference");
        }

        ContactMergeDuplicateAutoArgs params = CfgArgGroups::instance()->
            get_handler_ptr_by_type<HandleAdminClientContactMergeDuplicateAutoArgsGrp>()->params;

        if((!params.registrar.empty()) && (!params.except_registrar.empty()))
        {
            throw std::runtime_error("unable to use --registrar option with --except_registrar option");
        }

        std::vector<std::string> registrar_handles;

        if(!params.registrar.empty())
        {
            registrar_handles=params.registrar;
        }
        else
        {
            registrar_handles = LibFred::Registrar::GetRegistrarHandles().set_exclude_registrars(params.except_registrar).exec();
        }

        for(std::vector<std::string>::const_iterator ci = registrar_handles.begin()
            ; ci != registrar_handles.end(); ++ci)
        {
            Admin::MergeContactAutoProcedure(
                    *(mm.get()),
                    *(logger_client.get()),
                    *ci)
                .set_dry_run(params.dry_run)
                .set_verbose(params.verbose.is_value_set() ? Optional<unsigned short>(params.verbose.get_value()) : Optional<unsigned short>())
                .set_selection_filter_order(params.selection_filter_order)
            .exec(std::cout);
        }
        return;
    }
};//struct contact_merge_duplicate_auto_impl


/**
 * \class contact_merge_impl
 * \brief functor to run manual two contact merge command
 */
struct contact_merge_impl
{
    void operator()() const
    {
        Logging::Context ctx("contact_merge_impl");

        FakedArgs orb_fa = CfgArgGroups::instance()->fa;

        /* prepare logger corba client */
        HandleCorbaNameServiceArgsGrp* ns_args_ptr=CfgArgGroups::instance()->
                   get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>();

        CorbaContainer::set_instance(orb_fa.get_argc(), orb_fa.get_argv()
               , ns_args_ptr->get_nameservice_host()
               , ns_args_ptr->get_nameservice_port()
               , ns_args_ptr->get_nameservice_context());

        std::unique_ptr<LibFred::Logger::LoggerClient> logger_client(
                new LibFred::Logger::LoggerCorbaClientImpl());

        ContactMergeArgs params = CfgArgGroups::instance()
            ->get_handler_ptr_by_type<HandleAdminClientContactMergeArgsGrp>()->params;

        if (params.src.empty()) {
            throw ReturnCode("the option '--src' is required but missing", 1);
        }
        if (params.dst.empty()) {
            throw ReturnCode("the option '--dst' is required but missing", 1);
        }

        unsigned short verbose_level = 0;
        if (params.verbose.is_value_set()) {
            verbose_level = params.verbose.get_value();
        }
        if (params.dry_run) {
            verbose_level = 1;
        }


        try
        {
            LibFred::MergeContactOutput merge_data;
            Admin::MergeContactOperationSummary merge_operation_info;
            Admin::MergeContactSummaryInfo merge_summary_info;
            Admin::MergeContact merge_op = Admin::MergeContact(params.src, params.dst);
            if (params.dry_run)
            {
                merge_data = merge_op.exec_dry_run();
            }
            else
            {
                merge_data = merge_op.exec(*(logger_client.get()));
            }
            merge_operation_info.add_merge_output(merge_data);
            merge_summary_info.inc_merge_set();
            merge_summary_info.inc_merge_operation();

            if (verbose_level > 0) {
                OutputIndenter indenter(2, 0, ' ');
                std::cout << Admin::format_merge_contact_output(merge_data, params.src, params.dst, merge_summary_info, indenter);
                std::cout << merge_operation_info.format(indenter.dive());
            }
        }
        catch (LibFred::MergeContact::Exception &ex)
        {
            if (ex.is_set_unknown_source_contact_handle()) {
                throw ReturnCode(std::string("source contact '") + params.src + std::string("' not found"), 1);
            }
            if (ex.is_set_unknown_destination_contact_handle()) {
                throw ReturnCode(std::string("destination contact '") + params.dst + std::string("' not found"), 1);
            }
            if (ex.is_set_contacts_differ()) {
                throw ReturnCode(std::string("contact differs - cannot merge"), 1);
            }
            if (ex.is_set_identical_contacts_handle() || ex.is_set_identical_contacts_roid()) {
                throw ReturnCode(std::string("identical contacts passed as source and destination"), 1);
            }
        }

        return;
    }
};



/**
 * admin client implementation of contact verification check queue filling
 */
struct contact_verification_fill_queue_impl
{
  void operator()() const
  {
      ContactVerificationFillQueueArgs params = CfgArgGroups::instance()
          ->get_handler_ptr_by_type<HandleContactVerificationFillQueueArgsGrp>()->params;

      Fred::Backend::Admin::Contact::Verification::Queue::contact_filter filter;

      if(params.contact_roles.empty() == false) {
          BOOST_FOREACH(const std::string& role, params.contact_roles) {
              if(boost::iequals(role, "domain_owner")) {
                  filter.roles.insert(Fred::Backend::Admin::Contact::Verification::Queue::owner);
              } else if(boost::iequals(role, "admin")) {
                  filter.roles.insert(Fred::Backend::Admin::Contact::Verification::Queue::admin_c);
              } else if(boost::iequals(role, "technical")) {
                  filter.roles.insert(Fred::Backend::Admin::Contact::Verification::Queue::tech_c);
              } else {
                  throw ReturnCode(
                      "unknown role: \"" + role + "\"\n" +
                      "valid roles are: domain_owner, admin, techical", 1);
              }
          }
      }

      if(params.contact_states.empty() == false) {
          BOOST_FOREACH(const std::string& state, params.contact_states) {
              filter.states.insert(state);
          }
      }

      if(params.country_code.length() > 0) {
          filter.country_code = params.country_code;
      }

      std::vector<Fred::Backend::Admin::Contact::Verification::Queue::enqueued_check> enqueued_checks;

      enqueued_checks =
          Fred::Backend::Admin::Contact::Verification::Queue::fill_check_queue(
              boost::to_lower_copy(params.testsuite_handle),
              params.max_queue_length
          )
          .set_contact_filter(filter)
          .exec();

      if(enqueued_checks.size() > 0) {
          std::cout << "enqueued check handles:" << std::endl;

          BOOST_FOREACH(const Fred::Backend::Admin::Contact::Verification::Queue::enqueued_check& info, enqueued_checks) {
              std::cout
                << "check handle: "         << info.handle << "\t"
                << "contact id: "           << info.contact_id << "\t"
                << "contact history id: "   << info.contact_history_id << std::endl;
          }
      } else {
          std::cout << "no checks enqueued" << std::endl;
      }

      return ;
  }
};



/**
 * admin client implementation of contact verification check enqueueing
 */
struct contact_verification_enqueue_check_impl
{
  void operator()() const
  {
      ContactVerificationEnqueueCheckArgs params = CfgArgGroups::instance()
        ->get_handler_ptr_by_type<HandleContactVerificationEnqueueCheckArgsGrp>()->params;

      LibFred::OperationContextCreator ctx;
      std::string check_handle;
      try {
          check_handle = Fred::Backend::Admin::Contact::Verification::enqueue_check(
              ctx,
              params.contact_id,
              params.testsuite_handle);

          ctx.commit_transaction();
      } catch (LibFred::ExceptionUnknownContactId& e) {
          throw ReturnCode(
              std::string("given contact id (") + boost::lexical_cast<std::string>(params.contact_id) + ") is unknown",
              1);
      } catch (LibFred::ExceptionUnknownTestsuiteHandle& e) {
          throw ReturnCode(
              std::string("given testsuite handle (") + params.testsuite_handle + ") is unknown",
              1);
      }

      // if no exception was translated to throw (and the check was really created)...
      std::cout << "enqueued check with handle: " << check_handle << std::endl;

      return ;
  }
};


/**
 * admin client implementation of enqueued contact verification check starting
 */
struct contact_verification_start_enqueued_checks_impl
{
  void operator()() const
  {
      FakedArgs orb_fa = CfgArgGroups::instance()->fa;
      HandleCorbaNameServiceArgsGrp* ns_args_ptr=CfgArgGroups::instance()->
         get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>();

      orb_fa.add_argv(std::string("-ORBnativeCharCodeSet"));
      orb_fa.add_argv(std::string("UTF-8"));

      CorbaContainer::set_instance(
          orb_fa.get_argc(),
          orb_fa.get_argv(),
          ns_args_ptr->get_nameservice_host(),
          ns_args_ptr->get_nameservice_port(),
          ns_args_ptr->get_nameservice_context()
      );

      std::vector<std::string> started_checks;

      try {
          started_checks = Fred::Backend::Admin::Contact::Verification::run_all_enqueued_checks(
              Fred::Backend::Admin::Contact::Verification::create_test_impl_prototypes(
                  std::shared_ptr<LibFred::Mailer::Manager>(
                      new MailerManager(
                          CorbaContainer::get_instance()->getNS()
                      )
                  ),
                  std::shared_ptr<LibFred::Document::Manager>(
                      LibFred::Document::Manager::create(
                          CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_docgen_path(),
                          CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_docgen_template_path(),
                          CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_fileclient_path(),
                          CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_host_port()
                      ).release()
                  ),
                  // returns shared_ptr
                  LibFred::Messages::create_manager(),
                  CfgArgGroups::instance()->get_handler_ptr_by_type<HandleContactVerificationStartEnqueuedChecksArgsGrp>()->params.cz_address_mvcr_xml_path
              )
          );
      } catch (const Fred::Backend::Admin::Contact::Verification::ExceptionTestImplementationError& ) {
          throw ReturnCode("error in test implementation or prototype handling", 1);
      }

      if(started_checks.size() > 0) {
          std::cout << "started checks:" << std::endl;

          BOOST_FOREACH(const std::string& handle, started_checks) {
              std::cout << "check handle: " << handle << std::endl;
          }
      } else {
          std::cout << "no checks started" << std::endl;
      }
  }
};


#endif
