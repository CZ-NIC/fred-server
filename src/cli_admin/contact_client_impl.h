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
#include "cfg/config_handler_decl.h"
#include "cfg/handle_database_args.h"
#include "cfg/handle_corbanameservice_args.h"
#include "handle_adminclientselection_args.h"
#include "log/context.h"
#include "contactclient.h"
#include "commonclient.h"
#include "src/fredlib/reminder.h"
#include "src/admin/contact/merge_contact_auto_procedure.h"
#include "src/admin/contact/merge_contact.h"
#include "src/admin/contact/merge_contact_reporting.h"
#include "src/corba/logger_client_impl.h"

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


#endif // CONTACT_CLIENT_IMPL_H_
