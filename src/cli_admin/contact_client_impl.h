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

#include "cfg/config_handler_decl.h"
#include "cfg/handle_database_args.h"
#include "cfg/handle_corbanameservice_args.h"
#include "handle_adminclientselection_args.h"
#include "log/context.h"
#include "cli_admin/contactclient.h"
#include "commonclient.h"
#include "fredlib/reminder.h"
#include "admin/contact/merge_contact_auto_procedure.h"
#include "admin/contact/merge_contact.h"
#include "admin/contact/merge_contact_reporting.h"
#include "corba/logger_client_impl.h"
#include "admin/contact/verification/fill_check_queue.h"
#include "fredlib/contact/verification/create_check.h"
#include "admin/contact/verification/run_all_enqueued_checks.h"
#include "admin/contact/verification/create_test_impl_prototypes.h"


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

        Admin::MergeContactAutoProcedure(
                *(mm.get()),
                *(logger_client.get()), params.registrar,
                params.limit, params.dry_run, params.verbose)
            .set_selection_filter_order(params.selection_filter_order).exec();

        return;
    }
};


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

      typedef boost::tuple<std::string, long long, long long> check_data_type;

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
          Admin::ContactVerificationQueue::fill_check_queue(params.testsuite_name, params.max_queue_lenght)
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
          check_handle = Fred::CreateContactCheck(params.contact_id, params.testsuite_name)
              .exec(ctx);
          ctx.commit_transaction();
      } catch (Fred::CreateContactCheck::ExceptionUnknownContactId& e) {
          throw ReturnCode(
              std::string("given contact id (") + boost::lexical_cast<std::string>(params.contact_id) + ") is unknown",
              1);
      } catch (Fred::CreateContactCheck::ExceptionUnknownTestsuiteName& e) {
          throw ReturnCode(
              std::string("given testsuite name (") + params.testsuite_name + ") is unknown",
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

      std::vector<std::string> started_checks = Admin::run_all_enqueued_checks(Admin::create_test_impl_prototypes());

      if(started_checks.size() > 0) {
          std::cout << "started checks:" << std::endl;

          BOOST_FOREACH(const std::string& handle, started_checks) {
              std::cout << "check handle: " << handle << std::endl;
          }
      } else {
          std::cout << "no checks started" << std::endl;
      }
      return ;
  }
};


#endif // CONTACT_CLIENT_IMPL_H_
