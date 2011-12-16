/*
 * Copyright (C) 2010  CZ.NIC, z.s.p.o.
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
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time.hpp>
#include <boost/assign/list_of.hpp>

#include "fredlib/db_settings.h"
#include "corba_wrapper.h"
#include "log/logger.h"
#include "log/context.h"
#include "corba/connection_releaser.h"

#include "cli_admin/domain_client_impl.h"
#include "cli_admin/keyset_client_impl.h"
#include "cli_admin/contact_client_impl.h"
#include "cli_admin/invoice_client_impl.h"
#include "cli_admin/bank_client_impl.h"
#include "cli_admin/poll_client_impl.h"
#include "cli_admin/registrar_client_impl.h"
#include "cli_admin/notify_client_impl.h"
#include "cli_admin/enumparam_client_impl.h"
#include "cli_admin/object_client_impl.h"
#include "cli_admin/file_client_impl.h"
#include "cli_admin/regblock_client.h"
#include "cli_admin/charge_client_impl.h"

#include "cfg/handle_general_args.h"
#include "cfg/handle_logging_args.h"
#include "cfg/handle_database_args.h"
#include "cfg/handle_threadgroup_args.h"
#include "cfg/handle_corbanameservice_args.h"
#include "cli_admin/handle_adminclientselection_args.h"
#include "cfg/handle_registry_args.h"
#include "cfg/handle_sms_args.h"
#include "cfg/check_args.h"
#include "cfg/command_selection_args.h"

#include "cfg/config_handler.h"

using namespace std;

const string prog_name = "fred-admin";

//config args processing

//print help if required
HandlerGrpVector help_gv = boost::assign::list_of
    (HandleGrpArgsPtr(
            new HandleHelpArgGrp("\nUsage: " + prog_name + " <switches>\n")));

//print help on dates if required
HandlerGrpVector help_dates_gv = boost::assign::list_of
    (HandleGrpArgsPtr(
            new HandleHelpDatesArgsGrp()));

CommandHandlerPtrVector chpv = boost::assign::list_of
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientDomainListArgsGrp),domain_list_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientKeySetListArgsGrp),keyset_list_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientContactListArgsGrp),contact_list_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientContactReminderArgsGrp),contact_reminder_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientInvoiceListArgsGrp),invoice_list_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientInvoiceArchiveArgsGrp),invoice_archive_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientInvoiceCreditArgsGrp),invoice_credit_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientInvoiceBillingArgsGrp),invoice_billing_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientInvoiceAddPrefixArgsGrp),invoice_add_prefix_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientCreateInvoicePrefixesArgsGrp),create_invoice_prefixes_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientAddInvoiceNumberPrefixArgsGrp),add_invoice_number_prefix_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientInvoiceCreateArgsGrp),create_invoice_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientBankPaymentListArgsGrp),payment_list_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientBankImportXMLArgsGrp),bank_import_xml_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientBankAddAccountArgsGrp),bank_add_account_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientPollListAllArgsGrp),poll_list_all_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientPollCreateStatechangesArgsGrp),poll_create_statechanges_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientPollCreateRequestFeeMessagesArgsGrp), poll_create_request_fee_messages_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientBlockRegistrarIdArgsGrp), block_registrar_id_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientUnblockRegistrarIdArgsGrp), unblock_registrar_id_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientListBlockedRegsArgsGrp), list_blocked_regs_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientBlockRegistrarsOverLimitArgsGrp), block_registrars_over_limit_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientZoneAddArgsGrp),zone_add_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientRegistrarAddArgsGrp),registrar_add_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientRegistrarAddZoneArgsGrp),registrar_add_zone_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientRegistrarCreateCertificationArgsGrp),registrar_create_certification_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientRegistrarCreateGroupArgsGrp),registrar_create_group_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientRegistrarIntoGroupArgsGrp),registrar_into_group_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientRegistrarListArgsGrp),registrar_list_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientZoneNsAddArgsGrp),zone_ns_add_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientRegistrarAclAddArgsGrp),registrar_acl_add_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientPriceAddArgsGrp),price_add_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientChargeRequestFeeArgsGrp),charge_request_fee_impl() ))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientNotifyStateChangesArgsGrp),notify_state_changes_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientNotifyLettersPostservisSendArgsGrp),notify_letters_postservis_send_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientNotifyRegisteredLettersManualSendArgsGrp),notify_registered_letters_manual_send_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientNotifySmsSendArgsGrp),notify_sms_send_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientEnumParameterChangeArgsGrp),enum_parameter_change_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientObjectNewStateRequestArgsGrp),object_new_state_request_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientObjectNewStateRequestNameArgsGrp),object_new_state_request_name_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientObjectUpdateStatesArgsGrp),object_update_states_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientObjectRegularProcedureArgsGrp),object_regular_procedure_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientObjectDeleteCandidatesArgsGrp),object_delete_candidates_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientFileListArgsGrp),file_list_impl()))
 ;

CommandOptionGroups cog(chpv);

//common config file processing in path 0
HandlerGrpVector config_gv = boost::assign::list_of
    (HandleGrpArgsPtr(
            new HandleConfigFileArgsGrp(CONFIG_FILE))) ;
HandlerGrpVector loging_gv = boost::assign::list_of
    (HandleGrpArgsPtr(
            new HandleLoggingArgsGrp));
HandlerGrpVector database_gv = boost::assign::list_of
    (HandleGrpArgsPtr(
            new HandleDatabaseArgsGrp));
HandlerGrpVector corbans_gv = boost::assign::list_of
    (HandleGrpArgsPtr(
            new HandleCorbaNameServiceArgsGrp));
HandlerGrpVector registry_gv = boost::assign::list_of
        (HandleGrpArgsPtr(
                new HandleRegistryArgsGrp));
HandlerGrpVector sms_gv = boost::assign::list_of
        (HandleGrpArgsPtr(
                new HandleSmsArgsGrp));

HandlerPtrGrid global_hpg = gv_list
    (help_gv)(help_dates_gv)
    (cog)
    (config_gv)(loging_gv)(database_gv)(corbans_gv)(registry_gv)(sms_gv)
    ;


void setup_admin_logging(CfgArgGroups * cfg_instance_ptr)
{
    // setting up logger
    Logging::Log::Type  log_type = static_cast<Logging::Log::Type>(
        cfg_instance_ptr->get_handler_ptr_by_type<HandleLoggingArgsGrp>()
            ->get_log_type());

    boost::any param;
    if (log_type == Logging::Log::LT_FILE) param = cfg_instance_ptr
        ->get_handler_ptr_by_type<HandleLoggingArgsGrp>()->get_log_file();

    if (log_type == Logging::Log::LT_SYSLOG) param = cfg_instance_ptr
        ->get_handler_ptr_by_type<HandleLoggingArgsGrp>()
        ->get_log_syslog_facility();

    Logging::Manager::instance_ref().get(PACKAGE)
        .addHandler(log_type, param);

    Logging::Manager::instance_ref().get(PACKAGE).setLevel(
        static_cast<Logging::Log::Level>(
        cfg_instance_ptr->get_handler_ptr_by_type
            <HandleLoggingArgsGrp>()->get_log_level()));
}


int main(int argc, char* argv[])
{
    Logging::Context ctx("cli_admin");
    Logging::Manager::instance_ref().get(PACKAGE).debug("main start");


    FakedArgs fa; //producing faked args with unrecognized ones
    try
    {
        //config
        fa = CfgArgGroups::instance<HandleHelpArgGrp>(global_hpg)->handle(argc, argv);

        // setting up logger
        setup_admin_logging(CfgArgGroups::instance());

        HandleCommandSelectionArgsGrp* selection_ptr
            = CfgArgGroups::instance()->get_handler_ptr_by_type<
                HandleCommandSelectionArgsGrp>();

        //test callback impl
        ImplCallback impl_callback = selection_ptr->get_impl_callback();
        if(impl_callback)
            impl_callback();//call impl
        else
            throw std::runtime_error("have no implementation for selected command");

    }//try
    catch(CORBA::TRANSIENT&)
    {
        Logging::Manager::instance_ref().get(PACKAGE).error("Caught exception CORBA::TRANSIENT -- unable to contact the server." );
        cerr << "Caught exception CORBA::TRANSIENT -- unable to contact the "
             << "server." << endl;
        return EXIT_FAILURE;
    }
    catch(CORBA::SystemException& ex)
    {
        Logging::Manager::instance_ref().get(PACKAGE).error(string("Caught CORBA::SystemException: ")+ex._name() );
        cerr << "Caught CORBA::SystemException" << ex._name() << endl;
        return EXIT_FAILURE;
    }
    catch(CORBA::Exception& ex)
    {
        Logging::Manager::instance_ref().get(PACKAGE).error(string("Caught CORBA::Exception: ")+ex._name() );
        cerr << "Caught CORBA::Exception: " << ex._name() << endl;
        return EXIT_FAILURE;
    }
    catch(omniORB::fatalException& fe)
    {
        string errmsg = string("Caught omniORB::fatalException: ")
                        + string("  file: ") + string(fe.file())
                        + string("  line: ") + boost::lexical_cast<string>(fe.line())
                        + string("  mesg: ") + string(fe.errmsg());
        Logging::Manager::instance_ref().get(PACKAGE).error(errmsg);
        cerr << errmsg  << endl;
        return EXIT_FAILURE;
    }

    catch(const ReturnCode&rc)
    {
        Logging::Manager::instance_ref().get(PACKAGE).debug(
                string("ReturnCode: ") + boost::lexical_cast<std::string>(rc.get_return_code())
                + string(" ") + rc.what()
                );
        cerr << (string("ReturnCode: ") + boost::lexical_cast<std::string>(rc.get_return_code())
                        + string(" ") + rc.what())
                << endl;
        return rc.get_return_code();
    }

    catch(const ReturnFromMain&)
    {
        return EXIT_SUCCESS;
    }


    catch(exception& ex)
    {
        Logging::Manager::instance_ref().get(PACKAGE).error(string("Error: ")+ex.what());
        cerr << "Error: " << ex.what() << endl;
        return EXIT_FAILURE;
    }
    catch(...)
    {
        Logging::Manager::instance_ref().get(PACKAGE).error("Unknown Error");
        cerr << "Unknown Error" << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

