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
 *  @notify_client_impl.h
 *  notify client implementation header
 */

#ifndef NOTIFY_CLIENT_IMPL_H_
#define NOTIFY_CLIENT_IMPL_H_
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include "cfg/config_handler_decl.h"
#include "cfg/handle_database_args.h"
#include "cfg/handle_corbanameservice_args.h"
#include "cfg/handle_sms_args.h"
#include "handle_adminclientselection_args.h"
#include "log/context.h"
#include "notifyclient.h"
#include "commonclient.h"
#include "src/corba/file_manager_client.h"
#include "optys_get_undelivered.h"




/**
 * \class notify_state_changes_impl
 * \brief admin client implementation of notify_state_changes
 */
struct notify_state_changes_impl
{
  void operator()() const
  {
      Logging::Context ctx("notify_state_changes_impl");
      Admin::NotifyClient notify_client(
              CfgArgGroups::instance()->get_handler_ptr_by_type<HandleDatabaseArgsGrp>()->get_conn_info(),
              CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_host_port(),
              CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_context(),
              optional_string(CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_docgen_path()),
              optional_string(CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_docgen_template_path()),
              optional_string(CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_fileclient_path()),
              CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_restricted_handles(),
              true, //bool _notify_state_changes
              NotifyStateChangesArgs(
                      CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientNotifyStateChangesArgsGrp>()->params.notify_except_types,
                      CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientNotifyStateChangesArgsGrp>()->params.notify_limit,
                      CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientNotifyStateChangesArgsGrp>()->params.notify_debug),
              false, //bool _notify_letters_postservis_send
              optional_string(),//const optional_string& _hpmail_config
              false,//bool _notify_sms_send
              optional_string(),// cmdline_sms_command
              optional_string());// configfile_sms_command
      notify_client.runMethod();
  }
};

/**
 * \class notify_letters_postservis_send_impl
 * \brief admin client implementation of notify_letters_postservis_send
 */
struct notify_letters_postservis_send_impl
{
  void operator()() const
  {
      Logging::Context ctx("notify_letters_postservis_send_impl");
      Admin::NotifyClient notify_client(
              CfgArgGroups::instance()->get_handler_ptr_by_type<HandleDatabaseArgsGrp>()->get_conn_info()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_host_port()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_context()
              , optional_string(CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_docgen_path())
              , optional_string(CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_docgen_template_path())
              , optional_string(CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_fileclient_path())
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_restricted_handles()
              , false //bool _notify_state_changes
              , NotifyStateChangesArgs()
              , true //bool _notify_letters_postservis_send
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientNotifyLettersPostservisSendArgsGrp>()->hpmail_config//optional_string()//const optional_string& _hpmail_config
              , false//bool _notify_sms_send
              , optional_string()// cmdline_sms_command
              , optional_string()// configfile_sms_command
              );
      notify_client.runMethod();
      return ;
  }
};

/**
 * \class notify_registered_letters_manual_send_impl
 * \brief admin client implementation of notify_registered_letters_manual_send
 */
struct notify_registered_letters_manual_send_impl
{
  void operator()() const
  {
    Logging::Context ctx("notify_registered_letters_manual_send_impl");

    Admin::notify_registered_letters_manual_send_impl(
        CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_host_port()
        , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_context()
        , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientNotifyRegisteredLettersManualSendArgsGrp>()->params
    );
    return;
  }
};


/**
 * \class notify_sms_send_impl
 * \brief admin client implementation of notify_sms_send
 */
struct notify_sms_send_impl
{
  void operator()() const
  {
      Logging::Context ctx("notify_sms_send_impl");
      Admin::NotifyClient notify_client(
              CfgArgGroups::instance()->get_handler_ptr_by_type<HandleDatabaseArgsGrp>()->get_conn_info()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_host_port()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_context()
              , optional_string(CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_docgen_path())
              , optional_string(CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_docgen_template_path())
              , optional_string(CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_fileclient_path())
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleRegistryArgsGrp>()->get_restricted_handles()
              , false //bool _notify_state_changes
              , NotifyStateChangesArgs()
              , false //bool _notify_letters_postservis_send
              , optional_string()//const optional_string& _hpmail_config
              , true//bool _notify_sms_send
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientNotifySmsSendArgsGrp>()->cmdline_sms_command//optional_string()// cmdline_sms_command
              , optional_string(CfgArgGroups::instance()->get_handler_ptr_by_type<HandleSmsArgsGrp>()->get_sms_command())//optional_string()// configfile_sms_command
              );
      notify_client.runMethod();
      return ;
  }
};

/**
 * \class notify_letters_optys_send_impl
 * \brief admin client implementation of notify_letters_optys_send
 */
struct notify_letters_optys_send_impl
{
  void operator()() const
  {
      Logging::Context ctx("notify_letters_optys_send_impl");
      Admin::notify_letters_optys_send_impl(
              CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_host_port()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_context()
              , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientNotifyLettersOptysSendArgsGrp>()->optys_config
              );
  }
};

/**
 * \class notify_letters_optys_get_undelivered_impl
 * \brief admin client implementation of notify_letters_optys_get_undelivered
 */
struct notify_letters_optys_get_undelivered_impl
{
  void operator()() const
  {
      Logging::Context ctx("notify_letters_optys_get_undelivered_impl");
      HandleAdminClientNotifyLettersOptysGetUndeliveredArgsGrp* cfg_ptr = CfgArgGroups::instance()
          ->get_handler_ptr_by_type<HandleAdminClientNotifyLettersOptysGetUndeliveredArgsGrp>();
      Admin::notify_letters_optys_get_undelivered_impl(cfg_ptr->optys_config, cfg_ptr->all_local_files_only);
  }
};

struct send_object_event_notification_emails_impl {

    void operator()() const {

        Logging::Context ctx("send_object_event_notification_emails_impl");

        FakedArgs orb_fa = CfgArgGroups::instance()->fa;

        orb_fa.add_argv(std::string("-ORBnativeCharCodeSet"));
        orb_fa.add_argv(std::string("UTF-8"));

        HandleCorbaNameServiceArgsGrp* ns_args_ptr = CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>();

        CorbaContainer::set_instance(
            orb_fa.get_argc(), orb_fa.get_argv(),
            ns_args_ptr->get_nameservice_host(), ns_args_ptr->get_nameservice_port(), ns_args_ptr->get_nameservice_context()
        );

        boost::shared_ptr<Fred::Mailer::Manager> mailer_manager( new MailerManager(CorbaContainer::get_instance()->getNS()) );

        Admin::send_object_event_notification_emails_impl(mailer_manager);
    }
};

#endif // NOTIFY_CLIENT_IMPL_H_
