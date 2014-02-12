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

#ifndef CONTACT_CLIENT_IMPL_H_
#define CONTACT_CLIENT_IMPL_H_

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/make_shared.hpp>

#include "util/cfg/config_handler_decl.h"
#include "util/cfg/handle_database_args.h"
#include "util/cfg/handle_registry_args.h"
#include "util/cfg/handle_corbanameservice_args.h"
#include "src/cli_admin/handle_adminclientselection_args.h"
#include "util/log/context.h"
#include "src/cli_admin/contactclient.h"
#include "src/cli_admin/commonclient.h"
#include "src/fredlib/reminder.h"
#include "src/admin/contact/merge_contact_auto_procedure.h"
#include "src/admin/contact/merge_contact.h"
#include "src/admin/contact/merge_contact_reporting.h"
#include "src/corba/logger_client_impl.h"
#include "src/admin/contact/verification/fill_check_queue.h"
#include "src/admin/contact/verification/enqueue_check.h"
#include "src/admin/contact/verification/run_all_enqueued_checks.h"
#include "src/admin/contact/verification/create_test_impl_prototypes.h"

#include "src/fredlib/mailer.h"
#include "src/fredlib/documents.h"
#include "src/fredlib/messages/messages_impl.h"
#include "src/fredlib/registrar/get_registrar_handles.h"

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
          Fred::run_reminder(
                  &mailer,
                  boost::gregorian::from_string(
                      CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientContactReminderArgsGrp>()->date.get_value()));
      }
      else
      {
          Fred::run_reminder(&mailer);
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

        std::auto_ptr<Fred::Logger::LoggerClient> logger_client(
                new Fred::Logger::LoggerCorbaClientImpl());

        std::auto_ptr<Fred::Mailer::Manager> mm(
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
            registrar_handles = Fred::Registrar::GetRegistrarHandles().set_exclude_registrars(params.except_registrar).exec();
        }

        for(std::vector<std::string>::const_iterator ci = registrar_handles.begin()
            ; ci != registrar_handles.end(); ++ci)
        {
            Admin::MergeContactAutoProcedure(
                    *(mm.get()),
                    *(logger_client.get()))
                .set_registrar(*ci)
                .set_limit(params.limit)
                .set_dry_run(params.dry_run)
                .set_verbose(params.verbose)
                .set_selection_filter_order(params.selection_filter_order)
            .exec();
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

        std::auto_ptr<Fred::Logger::LoggerClient> logger_client(
                new Fred::Logger::LoggerCorbaClientImpl());

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
            Fred::MergeContactOutput merge_data;
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
        catch (Fred::MergeContact::Exception &ex)
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
      Logging::Context log("contact_verification_fill_queue");

      ContactVerificationFillQueueArgs params = CfgArgGroups::instance()
          ->get_handler_ptr_by_type<HandleContactVerificationFillQueueArgsGrp>()->params;

      typedef boost::tuple<std::string, unsigned long long, unsigned long long> check_data_type;

      Admin::ContactVerificationQueue::contact_filter filter;

      if(params.contact_roles.empty() == false) {
          BOOST_FOREACH(const std::string& role, params.contact_roles) {
              if(boost::iequals(role, "domain_owner")) {
                  filter.roles.insert(Admin::ContactVerificationQueue::owner);
              } else if(boost::iequals(role, "admin")) {
                  filter.roles.insert(Admin::ContactVerificationQueue::admin_c);
              } else if(boost::iequals(role, "technical")) {
                  filter.roles.insert(Admin::ContactVerificationQueue::tech_c);
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

      std::vector<check_data_type> enqueued_checks;

      enqueued_checks =
          Admin::ContactVerificationQueue::fill_check_queue(
              boost::to_lower_copy(params.testsuite_handle),
              params.max_queue_lenght
          )
          .set_contact_filter(filter)
          .exec();

      if(enqueued_checks.size() > 0) {
          std::cout << "enqueued check handles:" << std::endl;

          BOOST_FOREACH(const check_data_type& info, enqueued_checks) {
              std::cout
                << "check handle: "         << info.get<0>() << "\t"
                << "contact id: "           << info.get<1>() << "\t"
                << "contact history id: "   << info.get<2>() << std::endl;
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
      Logging::Context log("contact_verification_enqueue_check ");

      ContactVerificationEnqueueCheckArgs params = CfgArgGroups::instance()
        ->get_handler_ptr_by_type<HandleContactVerificationEnqueueCheckArgsGrp>()->params;

      Fred::OperationContext ctx;
      std::string check_handle;
      try {
          check_handle = Admin::enqueue_check(
              ctx,
              params.contact_id,
              params.testsuite_handle);

          ctx.commit_transaction();
      } catch (Fred::ExceptionUnknownContactId& e) {
          throw ReturnCode(
              std::string("given contact id (") + boost::lexical_cast<std::string>(params.contact_id) + ") is unknown",
              1);
      } catch (Fred::ExceptionUnknownTestsuiteHandle& e) {
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
      Logging::Context log("contact_verification_start_enqueued_checks");

      FakedArgs orb_fa = CfgArgGroups::instance()->fa;
      HandleCorbaNameServiceArgsGrp* ns_args_ptr=CfgArgGroups::instance()->
         get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>();

      CorbaContainer::set_instance(
          orb_fa.get_argc(),
          orb_fa.get_argv(),
          ns_args_ptr->get_nameservice_host(),
          ns_args_ptr->get_nameservice_port(),
          ns_args_ptr->get_nameservice_context()
      );

      std::vector<std::string> started_checks = Admin::run_all_enqueued_checks(
          Admin::create_test_impl_prototypes(
              boost::shared_ptr<Fred::Mailer::Manager>(
                  new MailerManager(
                      CorbaContainer::get_instance()->getNS()
                  )
              ),
              boost::shared_ptr<Fred::Document::Manager>(
                  Fred::Document::Manager::create(
                      CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_docgen_path(),
                      CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_docgen_template_path(),
                      CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_fileclient_path(),
                      CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_host_port()
                  ).release()
              ),
              // returns shared_ptr
              Fred::Messages::create_manager(),
              CfgArgGroups::instance()->get_handler_ptr_by_type<HandleContactVerificationStartEnqueuedChecksArgsGrp>()->params.cz_address_mvcr_xml_path
          )
      );

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


#endif // CONTACT_CLIENT_IMPL_H_
