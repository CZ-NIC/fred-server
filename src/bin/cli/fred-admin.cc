/*
 * Copyright (C) 2011-2022  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "libfred/db_settings.hh"
#include "src/util/corba_wrapper.hh"
#include "util/log/add_log_device.hh"
#include "util/log/logger.hh"
#include "util/log/context.hh"
#include "src/bin/corba/connection_releaser.hh"

#include "src/bin/cli/charge_registry_access_fee_impl.hh"
#include "src/bin/cli/domain_client_impl.hh"
#include "src/bin/cli/keyset_client_impl.hh"
#include "src/bin/cli/contact_client_impl.hh"
#include "src/bin/cli/invoice_client_impl.hh"
#include "src/bin/cli/bank_client_impl.hh"
#include "src/bin/cli/poll_client_impl.hh"
#include "src/bin/cli/registrar_client_impl.hh"
#include "src/bin/cli/notify_client_impl.hh"
#include "src/bin/cli/enumparam_client_impl.hh"
#include "src/bin/cli/manage_domain_lifecycle_parameters.hh"
#include "src/bin/cli/object_client_impl.hh"
#include "src/bin/cli/file_client_impl.hh"
#include "src/bin/cli/regblock_client.hh"
#include "src/bin/cli/charge_client_impl.hh"
#include "src/bin/cli/domain_name_validation_init.hh"
#include "src/bin/cli/public_request_impl.hh"
#include "src/bin/cli/invoice_export_impl.hh"

#include "src/util/cfg/handle_general_args.hh"
#include "src/util/cfg/handle_logging_args.hh"
#include "src/util/cfg/handle_database_args.hh"
#include "src/util/cfg/handle_threadgroup_args.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"
#include "src/util/cfg/handle_createexpireddomain_args.hh"
#include "src/bin/cli/handle_adminclientselection_args.hh"
#include "src/util/cfg/handle_registry_args.hh"
#include "src/util/cfg/handle_sms_args.hh"
#include "src/util/cfg/handle_messenger_args.hh"
#include "src/util/cfg/handle_fileman_args.hh"
#include "src/util/cfg/handle_secretary_args.hh"
#include "src/util/cfg/check_args.hh"
#include "src/util/cfg/command_selection_args.hh"

#include "src/util/cfg/config_handler.hh"

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time.hpp>
#include <boost/assign/list_of.hpp>

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
#include <utility>

const std::string prog_name = "fred-admin";

//config args processing

//print help if required
HandlerGrpVector help_gv = boost::assign::list_of
    (HandleGrpArgsPtr(
            new HandleHelpGrpArg("\nUsage: " + prog_name + " <switches>\n")));

//print help on dates if required
HandlerGrpVector help_dates_gv = boost::assign::list_of
    (HandleGrpArgsPtr(
            new HandleHelpDatesArgsGrp()));

CommandHandlerPtrVector chpv = boost::assign::list_of
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientDomainListArgsGrp),domain_list_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientKeySetListArgsGrp),keyset_list_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientContactListArgsGrp),contact_list_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientContactReminderArgsGrp),contact_reminder_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientContactMergeDuplicateAutoArgsGrp), contact_merge_duplicate_auto_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientContactMergeArgsGrp), contact_merge_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientInvoiceListArgsGrp),invoice_list_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientInvoiceArchiveArgsGrp),invoice_archive_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientInvoiceCreditArgsGrp),invoice_credit_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientInvoiceBillingArgsGrp),invoice_billing_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientInvoiceAddPrefixArgsGrp),invoice_add_prefix_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientCreateInvoicePrefixesArgsGrp),create_invoice_prefixes_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientAddInvoiceNumberPrefixArgsGrp),add_invoice_number_prefix_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientBankAddAccountArgsGrp),bank_add_account_impl()))
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
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientNotifyLettersOptysSendArgsGrp),notify_letters_optys_send_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientNotifyLettersOptysGetUndeliveredArgsGrp),notify_letters_optys_get_undelivered_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientEnumParameterChangeArgsGrp),enum_parameter_change_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientDomainLifecycleParametersArgsGrp), manage_domain_lifecycle_parameters))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientObjectNewStateRequestNameArgsGrp),object_new_state_request_name_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientObjectUpdateStatesArgsGrp),object_update_states_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientObjectRegularProcedureArgsGrp),object_regular_procedure_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientObjectDeleteCandidatesArgsGrp),object_delete_candidates_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientFileListArgsGrp),file_list_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleInitDomainNameValidationCheckersArgsGrp),init_domain_name_validation_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleDomainNameValidationByZoneArgsGrp),set_zone_domain_name_validation_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleContactVerificationFillQueueArgsGrp), contact_verification_fill_queue_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleContactVerificationEnqueueCheckArgsGrp), contact_verification_enqueue_check_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleContactVerificationStartEnqueuedChecksArgsGrp), contact_verification_start_enqueued_checks_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleSendObjectEventNotificationEmailsArgsGrp), send_object_event_notification_emails_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientCreateExpiredDomainArgsGrp), create_expired_domain_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientProcessPublicRequestsArgsGrp), process_public_requests_impl()))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleChargeRegistryAccessFeeAnnualArgsGrp), charge_registry_access_fee_annual_impl))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleChargeRegistryAccessFeeMonthlyArgsGrp), charge_registry_access_fee_monthly_impl))
    (CommandHandlerParam(HandleCommandArgsPtr(new HandleAdminClientInvoiceExportArgsGrp), invoice_export_impl));

CommandOptionGroups cog(chpv);

//common config file processing in path 0
HandlerGrpVector config_gv = boost::assign::list_of
    (HandleGrpArgsPtr(
            new HandleConfigFileGrpArgs(CONFIG_FILE))) ;
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
HandlerGrpVector create_expired_domain_gv = boost::assign::list_of
        (HandleGrpArgsPtr(
                new HandleCreateExpiredDomainArgsGrp));
HandlerGrpVector messenger_gv = boost::assign::list_of
    (HandleGrpArgsPtr(
            new HandleMessengerArgsGrp));
HandlerGrpVector fileman_gv = boost::assign::list_of
    (HandleGrpArgsPtr(
            new HandleFilemanArgsGrp));
HandlerGrpVector secretary_gv = boost::assign::list_of
    (HandleGrpArgsPtr(
            new HandleSecretaryArgsGrp));

HandlerPtrGrid global_hpg = gv_list
    (help_gv)(help_dates_gv)
    .addCommandOptions(cog)
    (config_gv)(loging_gv)(database_gv)(corbans_gv)(registry_gv)(sms_gv)(create_expired_domain_gv)(messenger_gv)(fileman_gv)(secretary_gv);

void setup_admin_logging(CfgArgGroups* cfg_instance_ptr)
{
    HandleLoggingArgsGrp* const handler_ptr = cfg_instance_ptr->get_handler_ptr_by_type<HandleLoggingArgsGrp>();

    const auto log_type = static_cast<unsigned>(handler_ptr->get_log_type());
    Logging::Log::Severity min_severity = Logging::Log::Severity::trace;
    switch (handler_ptr->get_log_level())
    {
        case 0:
            min_severity = Logging::Log::Severity::emerg;
            break;
        case 1:
            min_severity = Logging::Log::Severity::alert;
            break;
        case 2:
            min_severity = Logging::Log::Severity::crit;
            break;
        case 3:
            min_severity = Logging::Log::Severity::err;
            break;
        case 4:
            min_severity = Logging::Log::Severity::warning;
            break;
        case 5:
            min_severity = Logging::Log::Severity::notice;
            break;
        case 6:
            min_severity = Logging::Log::Severity::info;
            break;
        case 7:
            min_severity = Logging::Log::Severity::debug;
            break;
        case 8:
            min_severity = Logging::Log::Severity::trace;
            break;
    }

    switch (log_type)
    {
        case 0:
            Logging::add_console_device(LOGGER, min_severity);
            break;
        case 1:
            Logging::add_file_device(LOGGER, handler_ptr->get_log_file(), min_severity);
            break;
        case 2:
            Logging::add_syslog_device(LOGGER, handler_ptr->get_log_syslog_facility(), min_severity);
            break;
    }
}

int main(int argc, char* argv[])
{
    Logging::Context ctx("cli_admin");
    Logging::Manager::instance_ref().debug("main start");

    FakedArgs fa; //producing faked args with unrecognized ones
    try
    {
        //config
        fa = CfgArgGroups::init<HandleHelpGrpArg>(global_hpg)->handle(argc, argv);

        // setting up logger
        setup_admin_logging(CfgArgGroups::instance());

        // config dump
        if(CfgArgGroups::instance()->get_handler_ptr_by_type<HandleLoggingArgsGrp>()->get_log_config_dump())
        {
            for (std::string config_item = AccumulatedConfig::get_instance().pop_front();
                !config_item.empty(); config_item = AccumulatedConfig::get_instance().pop_front())
            {
                Logging::Manager::instance_ref().debug(config_item);
            }
        }

        HandleCommandSelectionArgsGrp* const selection_ptr =
                CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCommandSelectionArgsGrp>();

        //test callback impl
        ImplCallback impl_callback = selection_ptr->get_impl_callback();
        if (impl_callback)
        {
            impl_callback();//call impl
            return EXIT_SUCCESS;
        }
        throw std::runtime_error("have no implementation for selected command");
    }
    catch (const CORBA::TRANSIENT&)
    {
        Logging::Manager::instance_ref().error("Caught exception CORBA::TRANSIENT -- unable to contact the server.");
        std::cerr << "Caught exception CORBA::TRANSIENT -- unable to contact the "
             << "server." << std::endl;
        return EXIT_FAILURE;
    }
    catch (const CORBA::SystemException& e)
    {
        Logging::Manager::instance_ref().error(std::string("Caught CORBA::SystemException: ") + e._name());
        std::cerr << "Caught CORBA::SystemException" << e._name() << std::endl;
        return EXIT_FAILURE;
    }
    catch (const CORBA::Exception& e)
    {
        Logging::Manager::instance_ref().error(std::string("Caught CORBA::Exception: ") + e._name());
        std::cerr << "Caught CORBA::Exception: " << e._name() << std::endl;
        return EXIT_FAILURE;
    }
    catch (const omniORB::fatalException& e)
    {
        const auto errmsg = std::string("Caught omniORB::fatalException: ") +
                "  file: " + e.file() +
                "  line: " + boost::lexical_cast<std::string>(e.line()) +
                "  mesg: " + e.errmsg();
        Logging::Manager::instance_ref().error(errmsg);
        std::cerr << errmsg  << std::endl;
        return EXIT_FAILURE;
    }
    catch (const ReturnCode& e)
    {
        Logging::Manager::instance_ref().debug(
                "ReturnCode: " + boost::lexical_cast<std::string>(e.get_return_code()) + " " + e.what());
        if (e.what()[0] != '\0')
        {
            std::cerr << "error: " << e.what() << std::endl;
        }

        return e.get_return_code();
    }
    catch (const ReturnFromMain&)
    {
        return EXIT_SUCCESS;
    }
    catch (const std::exception& e)
    {
        Logging::Manager::instance_ref().error(std::string("Error: ") + e.what());
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        Logging::Manager::instance_ref().error("Unknown Error");
        std::cerr << "Unknown Error" << std::endl;
        return EXIT_FAILURE;
    }
}
