/*
 * Copyright (C) 2008-2022  CZ.NIC, z. s. p. o.
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
/**
 *  @handle_adminclientselection_args.h
 *  admin client configuration
 */

#ifndef HANDLE_ADMINCLIENTSELECTION_ARGS_HH_809940176CCF407B8E20E1741F12646B
#define HANDLE_ADMINCLIENTSELECTION_ARGS_HH_809940176CCF407B8E20E1741F12646B

#include "src/util/cfg/validate_args.hh"//must be included earlier than the boost/program_options.hpp !!!
#include "src/util/cfg/faked_args.hh"
#include "src/util/cfg/handle_args.hh"
#include "src/util/types/optional.hh"
#include "src/util/types/optional_from_program_options.hh"
#include "src/util/cfg/command_selection_args.hh"

#include "src/bin/cli/domain_params.hh"
#include "src/bin/cli/keyset_params.hh"
#include "src/bin/cli/contact_params.hh"
#include "src/bin/cli/domain_lifecycle_params.hh"
#include "src/bin/cli/invoice_params.hh"
#include "src/bin/cli/bank_params.hh"
#include "src/bin/cli/poll_params.hh"
#include "src/bin/cli/registrar_params.hh"
#include "src/bin/cli/notify_params.hh"
#include "src/bin/cli/enumparam_params.hh"
#include "src/bin/cli/object_params.hh"
#include "src/bin/cli/file_params.hh"
#include "src/bin/cli/regblock_params.hh"
#include "src/bin/cli/charge_params.hh"
#include "src/bin/cli/public_request_params.hh"
#include "src/bin/cli/domain_name_validation_params.hh"
#include "src/bin/cli/charge_registry_access_fee_params.hh"

#include <iostream>
#include <exception>
#include <string>
#include <vector>

#include <boost/asio.hpp>
#include <boost/program_options.hpp>

/**
 * \class HandleAdminClientDomainListArgsGrp
 * \brief admin client domain_list options handler
 */
class HandleAdminClientDomainListArgsGrp : public HandleCommandGrpArgs
{
public:
    DomainListArgs params;
    CommandDescription get_command_option()
    {
        return CommandDescription("domain_list");
    }

    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("domain_list options")));
        cfg_opts->add_options()
            ("domain_list", "list of all domains (via filters)")
            ("login_registrar", boost::program_options
                    ::value<Checked::string>()->notifier(save_optional_string(params.login_registrar))
                ,"login registrar handle")
            ("id", boost::program_options
                 ::value<Checked::id>()->notifier(save_optional_id(params.domain_id))
                      , "domain id")
            ("fqdn", boost::program_options
               ::value<Checked::string>()->notifier(save_optional_string(params.fqdn))
                ,"fully qualified domain name is domain object handle")
            ("handle", boost::program_options
                 ::value<Checked::string>()->notifier(save_optional_string(params.domain_handle))
                  ,"domain object handle is fully qualified domain name (fqdn)")
            ("nsset_id", boost::program_options
             ::value<Checked::id>()->notifier(save_optional_id(params.nsset_id))
                  , "nsset id")
            ("nsset_handle", boost::program_options
            ::value<Checked::string>()->notifier(save_optional_string(params.nsset_handle))
                , "nsset handle")
            ("any_nsset",  boost::program_options
                    ::value<bool>()->zero_tokens()->notifier(save_arg<bool>(params.any_nsset))
                     ,"any nsset")
            ("keyset_id", boost::program_options
               ::value<Checked::id>()->notifier(save_optional_id(params.keyset_id))
                    , "keyset id")
            ("keyset_handle", boost::program_options
            ::value<Checked::string>()->notifier(save_optional_string(params.keyset_handle))
                , "keyset handle")
            ("any_keyset", boost::program_options
                    ::value<bool>()->zero_tokens()->notifier(save_arg<bool>(params.any_keyset)) ,"any keyset")
            ("zone_id", boost::program_options
               ::value<Checked::id>()->notifier(save_optional_id(params.zone_id))
                    , "zone id")
            ("registrant_id", boost::program_options
               ::value<Checked::id>()->notifier(save_optional_id(params.registrant_id))
                    , "registrant id")
            ("registrant_handle", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.registrant_handle))
                , "registrant handle")
            ("registrant_name", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.registrant_name))
                , "registrant name")
            ("admin_id", boost::program_options
               ::value<Checked::id>()->notifier(save_optional_id(params.admin_id))
                    , "admin id")
            ("admin_handle", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.admin_handle))
                , "admin handle")
            ("admin_name", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.admin_name))
                , "admin name")
            ("registrar_id", boost::program_options
               ::value<Checked::id>()->notifier(save_optional_id(params.registrar_id))
                    , "registrar id")
            ("registrar_handle", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.registrar_handle))
                , "registrar handle")
            ("registrar_name", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.registrar_name))
                , "registrar name")
            ("crdate", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.crdate))
                , "create date, arg format viz --help_dates")
            ("deldate", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.deldate))
                , "delete date, arg format viz --help_dates")
            ("update", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.update))
                , "update date, arg format viz --help_dates")
            ("transdate", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.transdate))
                , "transfer date, arg format viz --help_dates")
            ("full_list", boost::program_options
                    ::value<bool>()->zero_tokens()->notifier(save_arg<bool>(params.full_list))
                     , "full list")
            /* invisible options
            ("out_zone_date", boost::program_options
                ::value<Checked::string>()
                , "out zone date, arg format viz --help_dates")
            ("exp_date", boost::program_options
                ::value<Checked::string>()
                , "expiration date, arg format viz --help_dates")
            ("cancel_date", boost::program_options
                ::value<Checked::string>()
                , "cancel date, arg format viz --help_dates")
            invisible options */
            ("limit", boost::program_options
               ::value<Checked::ulonglong>()->notifier(save_optional_ulonglong(params.limit))
                    , "limit")
            ;
        return cfg_opts;
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }//handle
};//class HandleAdminClientDomainListArgsGrp

/**
 * \class HandleAdminClientKeySetListArgsGrp
 * \brief admin client keyset_list options handler
 */
class HandleAdminClientKeySetListArgsGrp : public HandleCommandGrpArgs
{
public:
    KeysetListArgs params;

    CommandDescription get_command_option()
    {
        return CommandDescription("keyset_list");
    }

    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("keyset_list options")));
        cfg_opts->add_options()
            ("keyset_list", boost::program_options
                 ::value<Checked::string>()
                      , "command for list of keysets (via filters)")
            ("login_registrar", boost::program_options
             ::value<Checked::string>()->notifier(save_optional_string(params.login_registrar))
              ,"login registrar handle")
            ("id", boost::program_options
             ::value<Checked::id>()->notifier(save_optional_id(params.keyset_id))
                  , "keyset id")
            ("handle", boost::program_options
            ::value<Checked::string>()->notifier(save_optional_string(params.keyset_handle))
              , "keyset handle")
            ("admin_id", boost::program_options
             ::value<Checked::id>()->notifier(save_optional_id(params.admin_id))
                  , "admin id")
            ("admin_handle", boost::program_options
              ::value<Checked::string>()->notifier(save_optional_string(params.admin_handle))
              , "admin handle")
            ("admin_name", boost::program_options
              ::value<Checked::string>()->notifier(save_optional_string(params.admin_name))
              , "admin name")
            ("registrar_id", boost::program_options
             ::value<Checked::id>()->notifier(save_optional_id(params.registrar_id))
                  , "registrar id")
            ("registrar_handle", boost::program_options
              ::value<Checked::string>()->notifier(save_optional_string(params.registrar_handle))
              , "registrar handle")
            ("registrar_name", boost::program_options
              ::value<Checked::string>()->notifier(save_optional_string(params.registrar_name))
              , "registrar name")
            ("crdate", boost::program_options
              ::value<Checked::string>()->notifier(save_optional_string(params.crdate))
              , "create date, arg format viz --help_dates")
            ("deldate", boost::program_options
              ::value<Checked::string>()->notifier(save_optional_string(params.deldate))
              , "delete date, arg format viz --help_dates")
            ("update", boost::program_options
              ::value<Checked::string>()->notifier(save_optional_string(params.update))
              , "update date, arg format viz --help_dates")
            ("transdate", boost::program_options
              ::value<Checked::string>()->notifier(save_optional_string(params.transdate))
              , "transfer date, arg format viz --help_dates")
            ("full_list",boost::program_options
                    ::value<bool>()->zero_tokens()->notifier(save_arg<bool>(params.full_list))
                     , "full list")
            ("limit", boost::program_options
             ::value<Checked::ulonglong>()->notifier(save_optional_ulonglong(params.limit))
                  , "limit")
                ;
        return cfg_opts;
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }//handle
};//class HandleAdminClientKeySetListArgsGrp

/**
 * \class HandleHelpDatesArgsGrp
 * \brief admin client date and time format help
 */
class HandleHelpDatesArgsGrp : public HandleGrpArgs
{
public:

    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("date and time format help option")));
        cfg_opts->add_options()
                ("help_dates"
                          , "print admin client date and time format help")
                ;
        return cfg_opts;
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);

        //general config actions
        if (vm.count("help_dates"))
        {
            std::cout
                << "Possible dates format: (shown for create date option)\n"
                << " ``--crdate=\"2008-10-16\"''        - one specific day (time is from 00:00:00 to 23:59:59)\n"
                << " ``--crdate=\"2008-10-16;\"''       - interval from '2008-10-16 00:00:00' to the biggest valid date\n"
                << " ``--crdate=\";2008-10-16\"''       - interval from the lowest valid date to '2008-10-16 23:59:59'\n"
                << " ``--crdate=\"2008-10-16;2008-10-20\"'' - interval from '2008-10-16 00:00:00' to '2008-10-20 23:59:59'\n"
                << " ``--crdate=\"last_week;-1\"''      - this mean whole week before this one\n"
                << "\nPossible relative options: last_day, last_week, last_month, last_year, past_hour, past_week, past_month, past_year\n"
                << "Is also possible to input incomplete date e.g: ``--crdate=\"2008-10\"'' means the whole october 2008\n"
                << "\t (same as ``--crdate=\"2008-10-01 00:00:00;2008-10-31 23:59:59\"'')\n"
                << std::endl;

            throw ReturnFromMain("help_dates called");
        }
        return option_group_index;
    }//handle
};//class HandleHelpDatesArgsGrp

/**
 * \class HandleAdminClientContactReminderArgsGrp
 * \brief admin client contact_reminder options handler
 */
class HandleAdminClientContactReminderArgsGrp : public HandleCommandGrpArgs
{
public:
    optional_string date;

    CommandDescription get_command_option()
    {
        return CommandDescription("contact_reminder");
    }

    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("contact_reminder options")));
        cfg_opts->add_options()
            ("contact_reminder", "command for run contact reminder procedure")
            ("date", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(date))
                , "specific date, arg format viz --help_dates")
                ;
        return cfg_opts;
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }//handle
};//class HandleAdminClientContactReminderArgsGrp

/**
 * \class HandleAdminClientContactMergeDuplicateAutoArgsGrp
 * \brief admin client contact_merge_duplicate_auto options handler
 */
class HandleAdminClientContactMergeDuplicateAutoArgsGrp : public HandleCommandGrpArgs
{
public:
    ContactMergeDuplicateAutoArgs params;

    CommandDescription get_command_option()
    {
        return CommandDescription("contact_merge_duplicate_auto");
    }

    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("contact_merge_duplicate_auto options")));
        cfg_opts->add_options()
            ("contact_merge_duplicate_auto",
                "contact merge, if further unspecified for all registrars")
            ("registrar", boost::program_options::value<std::vector<std::string> >()->multitoken()
                ->notifier(save_arg<std::vector<std::string> >(params.registrar)),
                "registrar handles to run merge for, don't use with --except_registrar option")
            ("except_registrar", boost::program_options::value<std::vector<std::string> >()->multitoken()
                ->notifier(save_arg<std::vector<std::string> >(params.except_registrar)),
                "run merge contact for all registrars except of these, don't use with --registrar option")
            ("dry_run", boost::program_options::value<bool>()
                ->default_value(false)->zero_tokens()
                ->notifier(save_arg<bool>(params.dry_run)),
                "just write what could be done; don't actually touch data")
            ("selection_filter_order",
                boost::program_options::value<std::vector<std::string> >()->multitoken()
                    ->notifier(save_arg<std::vector<std::string> >(params.selection_filter_order)),
                "specify custom order of filters for best contact selection; available values are:\n"
                "\tmcs_filter_identified_contact\n"
                "\tmcs_filter_identity_attached\n"
                "\tmcs_filter_conditionally_identified_contact\n"
                "\tmcs_filter_handle_mojeid_syntax\n"
                "\tmcs_filter_max_domains_bound\n"
                "\tmcs_filter_max_objects_bound\n"
                "\tmcs_filter_recently_updated\n"
                "\tmcs_filter_not_regcznic\n"
                "\tmcs_filter_recently_created")
            ("verbose", boost::program_options::value<Checked::ushort>()
                ->notifier(save_optional_ushort(params.verbose)),
                "specify output verbosity level: 0,1,2,3, dry_run is true means 3");
        /* list of contact selection filters could be done by listing keys of ContactSelectionFilterFactory */
        return cfg_opts;
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }//handle
};//class HandleAdminClientContactMergeDuplicateAutoArgsGrp

class HandleAdminClientContactMergeArgsGrp : public HandleCommandGrpArgs
{
public:
    ContactMergeArgs params;

    CommandDescription get_command_option()
    {
        return CommandDescription("contact_merge");
    }

    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("contact_merge options")));
        cfg_opts->add_options()
            ("contact_merge",
                "command to merge two contacts")
            ("src", boost::program_options::value<Checked::string>()
                ->notifier(save_arg<std::string>(params.src)),
                "source contact handle to be merge (this one will be deleted)")
            ("dst", boost::program_options::value<Checked::string>()
                ->notifier(save_arg<std::string>(params.dst)),
                "destination contact handle to be merge onto")
            ("dry_run", boost::program_options::value<bool>()
                ->default_value(false)->zero_tokens()
                ->notifier(save_arg<bool>(params.dry_run)),
                "just write what could be done; don't actually touch data")
            ("verbose", boost::program_options::value<Checked::ushort>()
                ->notifier(save_optional_ushort(params.verbose)),
                "specify output verbosity level");
        return cfg_opts;
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }//handle
};


/**
 * \class HandleAdminClientContactListArgsGrp
 * \brief admin client contact_list options handler
 */
class HandleAdminClientContactListArgsGrp : public HandleCommandGrpArgs
{
public:
    ContactListArgs params;

    CommandDescription get_command_option()
    {
        return CommandDescription("contact_list");
    }

    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("contact_list options")));
        cfg_opts->add_options()
            ("contact_list", "command for list of contacts (via filters)")
            ("login_registrar", boost::program_options
             ::value<Checked::string>()->notifier(save_optional_string(params.login_registrar))
              ,"login registrar handle")
            ("id", boost::program_options
             ::value<Checked::id>()->notifier(save_optional_id(params.contact_id))
                  , "contact id")
            ("handle", boost::program_options
            ::value<Checked::string>()->notifier(save_optional_string(params.contact_handle))
              , "contact handle")
            ("name_name", boost::program_options
            ::value<Checked::string>()->notifier(save_optional_string(params.contact_name))
            , "contact name")
            ("organization", boost::program_options
            ::value<Checked::string>()->notifier(save_optional_string(params.contact_organization))
            , "contact organization")
            ("city", boost::program_options
            ::value<Checked::string>()->notifier(save_optional_string(params.contact_city))
            , "contact city")
            ("email", boost::program_options
            ::value<Checked::string>()->notifier(save_optional_string(params.contact_email))
            , "contact email")
            ("notify_email", boost::program_options
            ::value<Checked::string>()->notifier(save_optional_string(params.contact_notify_email))
            , "contact notify email")
            ("vat", boost::program_options
            ::value<Checked::string>()->notifier(save_optional_string(params.contact_vat))
            , "contact vat")
            ("ssn", boost::program_options
            ::value<Checked::string>()->notifier(save_optional_string(params.contact_ssn))
            , "contact ssn")
            ("registrar_id", boost::program_options
             ::value<Checked::id>()->notifier(save_optional_id(params.registrar_id))
                  , "registrar id")
            ("registrar_handle", boost::program_options
              ::value<Checked::string>()->notifier(save_optional_string(params.registrar_handle))
              , "registrar handle")
            ("registrar_name", boost::program_options
              ::value<Checked::string>()->notifier(save_optional_string(params.registrar_name))
              , "registrar name")
            ("crdate", boost::program_options
              ::value<Checked::string>()->notifier(save_optional_string(params.crdate))
              , "create date, arg format viz --help_dates")
            ("deldate", boost::program_options
              ::value<Checked::string>()->notifier(save_optional_string(params.deldate))
              , "delete date, arg format viz --help_dates")
            ("update", boost::program_options
              ::value<Checked::string>()->notifier(save_optional_string(params.update))
              , "update date, arg format viz --help_dates")
            ("transdate", boost::program_options
              ::value<Checked::string>()->notifier(save_optional_string(params.transdate))
              , "transfer date, arg format viz --help_dates")
            ("full_list",boost::program_options
                    ::value<bool>()->zero_tokens()->notifier(save_arg<bool>(params.full_list))
                     ,"full list")
            ("limit", boost::program_options
             ::value<Checked::ulonglong>()->notifier(save_optional_ulonglong(params.limit))
                  , "limit")
                ;
        return cfg_opts;
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }//handle
};//class HandleAdminClientContactListArgsGrp

/**
 * \class HandleAdminClientInvoiceListArgsGrp
 * \brief admin client invoice_list options handler
 */
class HandleAdminClientInvoiceListArgsGrp : public HandleCommandGrpArgs
{
public:
    InvoiceListArgs params;

    CommandDescription get_command_option()
    {
        return CommandDescription("invoice_list");
    }

    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("invoice_list options")));
        cfg_opts->add_options()
            ("invoice_list", "command for list of invoices (via filters)")
            ("id", boost::program_options
                ::value<Checked::id>()->notifier(save_optional_id(params.invoice_id))
                , "invoice id")
            ("zone_id", boost::program_options
                ::value<Checked::id>()->notifier(save_optional_id(params.zone_id))
                , "zone id")
            ("zone_fqdn", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.zone_fqdn))
                , "zone name")
            ("type", boost::program_options
             ::value<Checked::ulonglong>()->notifier(save_optional_ulonglong(params.type))
              , "invoice type (1=advanced, 2=account)")
            ("number", boost::program_options
              ::value<Checked::string>()->notifier(save_optional_string(params.number))
              , "invoice number")
            ("crdate", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.crdate))
                , "create date, arg format viz --help_dates")
            ("taxdate", boost::program_options
              ::value<Checked::string>()->notifier(save_optional_string(params.taxdate))
              , "tax date, arg format viz --help_dates")
            ("registrar_id", boost::program_options
                ::value<Checked::id>()->notifier(save_optional_id(params.registrar_id))
                , "registrar id")
            ("registrar_handle", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.registrar_handle))
                , "registrar handle")
            ("invoice_file_pdf", boost::program_options
                ::value<Checked::id>()->notifier(save_optional_id(params.invoice_file_pdf))
                , "invoice file pdf")
            ("invoice_file_xml", boost::program_options
                ::value<Checked::id>()->notifier(save_optional_id(params.invoice_file_xml))
                , "invoice file xml")
            ("invoice_file_name", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.invoice_file_name))
                , "invoice file name")
            ("limit", boost::program_options
                ::value<Checked::ulonglong>()->notifier(save_optional_ulonglong(params.limit))
                , "limit")
                ;
        return cfg_opts;
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }//handle
};//class HandleAdminClientInvoiceListArgsGrp

/**
 * \class HandleAdminClientInvoiceArchiveArgsGrp
 * \brief admin client invoice_archive options handler
 */
class HandleAdminClientInvoiceArchiveArgsGrp : public HandleCommandGrpArgs
{
public:

    bool invoice_dont_send;

    CommandDescription get_command_option()
    {
        return CommandDescription("invoice_archive");
    }

    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("invoice_archive options")));
        cfg_opts->add_options()
            ("invoice_archive", "archive unarchived invoices")
            ("invoice_dont_send",boost::program_options
                ::value<bool>()->zero_tokens()->notifier(save_arg<bool>(invoice_dont_send))
                 , "don't send mails with invoices during archivation")
                ;
        return cfg_opts;
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }//handle
};//class HandleAdminClientInvoiceArchiveArgsGrp


/**
 * \class HandleAdminClientInvoiceCreditArgsGrp
 * \brief admin client invoice_credit options handler
 */
class HandleAdminClientInvoiceCreditArgsGrp : public HandleCommandGrpArgs
{
public:

    InvoiceCreditArgs params;

    CommandDescription get_command_option()
    {
        return CommandDescription("invoice_credit");
    }

    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("invoice_credit options")));
        cfg_opts->add_options()
            ("invoice_credit", "create credit invoice for registrar")
            ("zone_id", boost::program_options
                ::value<Checked::id>()->notifier(save_optional_id(params.zone_id))
                , "zone id")
            ("registrar_id", boost::program_options
                ::value<Checked::id>()->notifier(save_optional_id(params.registrar_id))
                , "registrar id")
            ("price", boost::program_options
                ::value<Checked::string_fpnumber>()->notifier(save_arg_using_to_string<optional_string>(params.price))
                , "price")
            ("taxdate", boost::program_options
              ::value<Checked::string>()->notifier(save_optional_string(params.taxdate))
              , "tax date, default in impl is today, arg format viz --help_dates")
                ;
        return cfg_opts;
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }//handle
};//class HandleAdminClientInvoiceCreditArgsGrp

/**
 * \class HandleAdminClientInvoiceBillingArgsGrp
 * \brief admin client invoice_billing options handler
 */
class HandleAdminClientInvoiceBillingArgsGrp : public HandleCommandGrpArgs
{
public:

    InvoiceBillingArgs params;

    CommandDescription get_command_option()
    {
        return CommandDescription("invoice_billing");
    }

    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("invoice_billing options")));
        cfg_opts->add_options()
            ("invoice_billing", "invoice billing")
            ("zone_fqdn", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.zone_fqdn))
                , "zone name")
            ("registrar_handle", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.registrar_handle))
                , "registrar handle")
            ("fromdate", boost::program_options
                    ::value<Checked::string>()->notifier(save_optional_string(params.fromdate))
                , "fromdate, default in impl is day after end of previous account invoice, usually first day of last month,"
                " meaning is start of interval including \"fromdate\" arg format YYYY-MM-DD")
            ("todate", boost::program_options
                    ::value<Checked::string>()->notifier(save_optional_string(params.todate))
                , "todate, default in impl is first day of this month, todate have to be before today except custom interval when fromdate is set"
                ",  meaning is end of interval NOT including \"todate\" arg format YYYY-MM-DD")
            ("taxdate", boost::program_options
              ::value<Checked::string>()->notifier(save_optional_string(params.taxdate))
              , "tax date, default in impl is date of last day of previous month which is last day of accounting interval, arg format YYYY-MM-DD")
            ("invoicedate", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.invoicedate))
                , "invoice date is timestamp of invoice, default in impl is local now(), arg format YYYY-MM-DD hh:mm:ss")
                ;
        return cfg_opts;
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }//handle
};//class HandleAdminClientInvoiceBillingArgsGrp

/**
 * \class HandleAdminClientInvoiceAddPrefixArgsGrp
 * \brief admin client invoice_add_prefix options handler
 */
class HandleAdminClientInvoiceAddPrefixArgsGrp : public HandleCommandGrpArgs
{
public:

    InvoicePrefixArgs params;

    CommandDescription get_command_option()
    {
        return CommandDescription("invoice_add_prefix");
    }

    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("invoice_add_prefix options")));
        cfg_opts->add_options()
            ("invoice_add_prefix", "add row into the ``invoice_prefix'' table")
            ("zone_id", boost::program_options
                ::value<Checked::id>()->notifier(save_optional_id(params.zone_id))
                , "zone id")
            ("zone_fqdn", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.zone_fqdn))
                , "zone name")
            ("type", boost::program_options
                ::value<Checked::ulong>()->notifier(save_optional_ulong(params.type))
                , "type is either 0 (for the deposit invoice) or 1 (for account invoice)")
            ("year", boost::program_options
                    ::value<Checked::ulong>()->notifier(save_optional_ulong(params.year))
                , "year default in impl is the current year")
            ("prefix", boost::program_options
              ::value<Checked::ulonglong>()->notifier(save_optional_ulonglong(params.prefix))
              , "prefix")
                ;
        return cfg_opts;
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }//handle
};//class HandleAdminClientInvoiceAddPrefixArgsGrp

/**
 * \class HandleAdminClientCreateInvoicePrefixesArgsGrp
 * \brief admin client create_invoice_prefixes options handler
 */
class HandleAdminClientCreateInvoicePrefixesArgsGrp : public HandleCommandGrpArgs
{
public:

    CreateInvoicePrefixesArgs params;

    CommandDescription get_command_option()
    {
        return CommandDescription("create_invoice_prefixes");
    }

    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("create_invoice_prefixes options")));
        cfg_opts->add_options()
            ("create_invoice_prefixes"
            , "create next year invoice prefixes for zones and invoice types in invoice_number_prefix if they don't exist")
            ("for_current_year",  boost::program_options
                     ::value<bool>()->zero_tokens()->notifier(save_arg<bool>(params.for_current_year))
                      ,"make prefixes for the current year instead")
            ;
        return cfg_opts;
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }//handle
};//class HandleAdminClientCreateInvoicePrefixesArgsGrp

/**
 * \class HandleAdminClientAddInvoiceNumberPrefixArgsGrp
 * \brief admin client add_invoice_number_prefix options handler
 */
class HandleAdminClientAddInvoiceNumberPrefixArgsGrp : public HandleCommandGrpArgs
{
public:

    AddInvoiceNumberPrefixArgs params;

    CommandDescription get_command_option()
    {
        return CommandDescription("add_invoice_number_prefix");
    }

    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("add_invoice_number_prefix options")));
        cfg_opts->add_options()
            ("add_invoice_number_prefix"
            , "add invoice number prefix for zones and invoice types in invoice_number_prefix if they don't exist")
            ("prefix", boost::program_options
                ::value<Checked::ulong>()->notifier(save_optional_ulong(params.prefix))
                , "two-digit invoice number prefix ")
            ("zone_fqdn", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.zone_fqdn))
                , "zone name")
            ("invoice_type_name", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.invoice_type_name))
                , "name of invoice type (advance, account, ...)")
            ;
        return cfg_opts;
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }//handle
};//class HandleAdminClientAddInvoiceNumberPrefixArgsGrp

/**
 * \class HandleAdminClientBankAddAccountArgsGrp
 * \brief admin client bank_add_account options handler
 */
class HandleAdminClientBankAddAccountArgsGrp : public HandleCommandGrpArgs
{
public:
    AddAccountArgs params;
    CommandDescription get_command_option()
    {
        return CommandDescription("bank_add_account");
    }
    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("bank_add_account options")));
        cfg_opts->add_options()
            ("bank_add_account", "add bank account")
            ("bank_account_number", boost::program_options
                ::value<Checked::string>()->notifier(save_arg<std::string>(params.bank_account_number))
                , "bank_account_number description")
            ("bank_code", boost::program_options
                ::value<Checked::string>()->notifier(save_arg<std::string>(params.bank_code))
                , "bank_code description")
            ("account_name", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.account_name))
                , "account name")
            ("zone_fqdn", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.zone_fqdn))
                , "zone fully qualified domain name")
            ;
        return cfg_opts;
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }//handle
};//class HandleAdminClientBankAddAccountArgsGrp

/**
 * \class HandleAdminClientPollCreateStatechangesArgsGrp
 * \brief admin client poll_create_statechanges options handler
 */
class HandleAdminClientPollCreateStatechangesArgsGrp : public HandleCommandGrpArgs
{
public:
    PollCreateStatechangesArgs params;
    CommandDescription get_command_option()
    {
        return CommandDescription("poll_create_statechanges");
    }
    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("poll_create_statechanges options")));
        cfg_opts->add_options()
            ("poll_create_statechanges", "create messages for state changes")
            ("poll_except_types", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.poll_except_types))
                , "list of poll messages types ignored in creation (only states now)")
            ("poll_limit", boost::program_options
                ::value<Checked::ulong>()->notifier(save_optional_ulong(params.poll_limit))
                , "limit for number of messages generated in one pass (0=no limit)")
            ("poll_debug", boost::program_options
                ::value<bool>()->zero_tokens()->notifier(save_arg<bool>(params.poll_debug))
                , "don't do actually anything, just list xml with values")
            ;
        return cfg_opts;
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }//handle
};//class HandleAdminClientPollCreateStatechangesArgsGrp

/**
 * \class HandleAdminClientPollCreateRequestFeeMessagesArgsGrp
 * \brief
 */
class HandleAdminClientPollCreateRequestFeeMessagesArgsGrp : public HandleCommandGrpArgs
{
public:
    PollCreateRequestFeeMessagesArgs params;
    CommandDescription get_command_option()
    {
        return CommandDescription("poll_create_request_fee_messages");
    }

    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("poll_create_request_fee_messages options")));
        cfg_opts->add_options()
            ("poll_create_request_fee_messages", "create requests fee info messages")
            ("poll_period_to", boost::program_options::value<Checked::string>()->notifier(
                    save_optional_string(params.poll_period_to)),
                    "create request fee poll messages for given date")
            ;
        return cfg_opts;
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }//handle
};//class HandleAdminClientPollCreateRequestFeeMessagesArgsGrp


class HandleAdminClientBlockRegistrarIdArgsGrp : public HandleCommandGrpArgs
{
public:
    RegBlockArgs params;
    CommandDescription get_command_option()
    {
        return CommandDescription("block_registrar_id");
    }

    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("block_registrar_id options")));
        cfg_opts->add_options()
            ("block_registrar_id", boost::program_options
             ::value<Checked::id>()->notifier(save_optional_id(params.block_id))
                  , "Block registrar with given ID")
          ;

        return cfg_opts;
    }
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }//handle
};

class HandleAdminClientUnblockRegistrarIdArgsGrp : public HandleCommandGrpArgs
{
public:
    RegBlockArgs params;
    CommandDescription get_command_option()
    {
        return CommandDescription("unblock_registrar_id");
    }

    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("unblock_registrar_id options")));
        cfg_opts->add_options()
            ("unblock_registrar_id", boost::program_options
             ::value<Checked::id>()->notifier(save_optional_id(params.unblock_id))
                  , "Unblock registrar with given ID")
          ;

        return cfg_opts;

    }
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }//handle
};

class HandleAdminClientListBlockedRegsArgsGrp : public HandleCommandGrpArgs
{
public:
    RegBlockArgs params;
    CommandDescription get_command_option()
    {
        return CommandDescription("list_blocked_registrars");
    }

    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("list_blocked_registrars options")));
        cfg_opts->add_options()
            ("list_blocked_registrars" , boost::program_options
                    ::value<bool>()->zero_tokens()->notifier(save_arg<bool>(params.list_only)),
                     "List registrars which were recently blocked or unblocked")
          ;

        return cfg_opts;

    }
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }//handle
};

class HandleAdminClientBlockRegistrarsOverLimitArgsGrp : public HandleCommandGrpArgs
{
public:
    RegBlockArgs params;
    CommandDescription get_command_option()
    {
        return CommandDescription("block_registrars_over_limit");
    }

    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("block_registrars_over_limit options")));
        cfg_opts->add_options()
            ("block_registrars_over_limit" ,boost::program_options
                    ::value<bool>()->zero_tokens()->notifier(save_arg<bool>(params.over_limit)),
                    "Automatically block registrar which have set and exceeded a limit on number of requests")
            ("email", boost::program_options
                    ::value<Checked::string>()
                     ->notifier(save_arg<std::string>(params.notify_email))
                     , "email address used for notification when registrar is automatically blocked")
            ("shell_cmd_timeout", boost::program_options
                    ::value<Checked::ulong>()->default_value(10)
                     ->notifier(save_arg<unsigned>(params.shell_cmd_timeout))
                     , "set alarm timeout for shell commands (sendmail,..) [s]")
          ;

        return cfg_opts;
    }
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }//handle
};

/**
 * \class HandleAdminClientZoneAddArgsGrp
 * \brief admin client zone_add options handler
 */
class HandleAdminClientZoneAddArgsGrp : public HandleCommandGrpArgs
{
public:
    ZoneAddArgs params;
    CommandDescription get_command_option()
    {
        return CommandDescription("zone_add");
    }
    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("zone_add options")));
        cfg_opts->add_options()
            ("zone_add", "add new zone")
            ("zone_fqdn", boost::program_options
                ::value<Checked::string>()->required()
                ->notifier(save_arg<std::string>(params.zone_fqdn)),
                "zone fully qualified name")
            ("ex_period_min", boost::program_options
                ::value<Checked::ushort>()->default_value(12)
                ->notifier(save_arg<unsigned short>(params.ex_period_min)),
                "minimal prolongation of the period of domains validity (in months)")
            ("ex_period_max", boost::program_options
                ::value<Checked::ushort>()->default_value(120)
                ->notifier(save_arg<unsigned short>(params.ex_period_max)),
                "maximal prolongation of the period of domains validity (in months)")
            ("ttl", boost::program_options
                ::value<Checked::ulong>()->default_value(18000)
                ->notifier(save_arg<unsigned long>(params.ttl)),
                "time to live - the default validity period of resource records in the zone (in seconds)")
            ("hostmaster", boost::program_options
                ::value<Checked::string>()->required()
                ->notifier(save_arg<std::string>(params.hostmaster)),
                "responsible person email (in format: user@domain.tld)")
            ("update_retr", boost::program_options
                ::value<Checked::ulong>()->default_value(300)
                ->notifier(save_arg<unsigned long>(params.update_retr)),
                "retry interval of zone update for secondary nameservers in case of failed zone refresh (in seconds)")
            ("refresh", boost::program_options
                ::value<Checked::ulong>()->default_value(900)
                ->notifier(save_arg<unsigned long>(params.refresh)),
                "secondary nameservers copy of zone refresh interval (in seconds)")
            ("expiry", boost::program_options
                ::value<Checked::ulong>()->default_value(604800)
                ->notifier(save_arg<unsigned long>(params.expiry)),
                "zone expiration period for secondary nameservers (in seconds)")
            ("minimum", boost::program_options
                ::value<Checked::ulong>()->default_value(900)
                ->notifier(save_arg<unsigned long>(params.minimum)),
                "the period for which a result of a non-existent domain should be cached by resolvers (in seconds)")
            ("ns_fqdn", boost::program_options
                ::value<Checked::string>()->required()
                ->notifier(save_arg<std::string>(params.ns_fqdn)),
                "primary nameserver fully qualified name")
            ;
        return cfg_opts;
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }//handle
};//class HandleAdminClientZoneAddArgsGrp


/**
 * \class HandleAdminClientRegistrarAddArgsGrp
 * \brief admin client registrar_add options handler
 */
class HandleAdminClientRegistrarAddArgsGrp : public HandleCommandGrpArgs
{
public:
    RegistrarAddArgs params;
    CommandDescription get_command_option()
    {
        return CommandDescription("registrar_add");
    }
    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("registrar_add options")));
        cfg_opts->add_options()
            ("registrar_add", "add new registrar")
            ("handle", boost::program_options
                ::value<Checked::string>()->required()->notifier(save_arg<std::string>(params.handle)),
                "registrar handle")
            ("country", boost::program_options
                ::value<Checked::string>()->required()->notifier(save_arg<std::string>(params.country)),
                "registrar two letter country code")
            ("ico", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.ico)),
                "organization identifier number")
            ("dic", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.dic)),
                "tax identifier number")
            ("varsymb", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.varsymb)),
                "registrar variable symbol")
            ("reg_name", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.reg_name)),
                "registrar name")
            ("organization", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.organization)),
                "registrar organization")
            ("street1", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.street1)),
                "registrar street #1")
            ("street2", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.street2)),
                "registrar street #2")
            ("street3", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.street3)),
                "registrar street #3")
            ("city", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.city)),
                "registrar city")
            ("stateorprovince", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.stateorprovince)),
                "registrar state or province")
            ("postalcode", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.postalcode)),
                "registrar postal code")
            ("telephone", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.telephone)),
                "registrar telephone")
            ("fax", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.fax)),
                "registrar fax")
            ("email", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.email)),
                "registrar email")
            ("url", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.url)),
                "registrar url")
            ("system", boost::program_options
                ::value<bool>()->zero_tokens()->notifier(save_arg<bool>(params.system)),
                "if registrar is system")
            ("no_vat", boost::program_options
                ::value<bool>()->zero_tokens()->notifier(save_arg<bool>(params.no_vat)),
                "no vat")
            ("internal", boost::program_options
                ::value<bool>()->zero_tokens()->notifier(save_arg<bool>(params.internal)),
                "if registrar is internal")
            ;
        return cfg_opts;
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }//handle
};//class HandleAdminClientRegistrarAddArgsGrp

/**
 * \class HandleAdminClientRegistrarAddZoneArgsGrp
 * \brief admin client registrar_add_zone options handler
 */
class HandleAdminClientRegistrarAddZoneArgsGrp : public HandleCommandGrpArgs
{
public:
    RegistrarAddZoneArgs params;
    CommandDescription get_command_option()
    {
        return CommandDescription("registrar_add_zone");
    }
    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("registrar_add_zone options")));
        cfg_opts->add_options()
            ("registrar_add_zone", "add access right for registrar to zone")
            ("zone_fqdn", boost::program_options
                ::value<Checked::string>()->required()
                ->notifier(save_arg<std::string>(params.zone_fqdn))
                , "zone fqdn")
            ("handle", boost::program_options
                ::value<Checked::string>()->required()
                ->notifier(save_arg<std::string>(params.handle))
                , "registrar handle")
            ("from_date", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.from_date))
                , "from date (default today)")
            ("to_date", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.to_date))
                , "to date (default not filled)")
            ;
        return cfg_opts;
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }//handle
};//class HandleAdminClientRegistrarAddZoneArgsGrp

/**
 * \class HandleAdminClientRegistrarCreateCertificationArgsGrp
 * \brief admin client registrar_create_certification options handler
 */
class HandleAdminClientRegistrarCreateCertificationArgsGrp : public HandleCommandGrpArgs
{
public:
    RegistrarCreateCertificationArgs params;

    CommandDescription get_command_option()
    {
        return CommandDescription("registrar_create_certification");
    }

    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("registrar_create_certification options")));
        cfg_opts->add_options()
            ("registrar_create_certification", "create registrar certification")
            ("certification_evaluation", boost::program_options
                ::value<Checked::string>()->required()
                ->notifier(save_arg<std::string>(params.certification_evaluation)),
                "registrar certification evaluation pdf file")
            ("certification_evaluation_mime_type", boost::program_options
                ::value<Checked::string>()->required()
                ->notifier(save_arg<std::string>(params.certification_evaluation_mime_type)),
                "registrar certification evaluation file MIME type")
            ("certification_score", boost::program_options
                ::value<Checked::ushort>()->required()
                ->notifier(save_arg<ushort>(params.certification_score)),
                "registrar certification score 0 - 5")
            ("handle", boost::program_options
                ::value<Checked::string>()->required()
                ->notifier(save_arg<std::string>(params.handle)),
                "registrar handle")
            ("from_date", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.from_date)),
                "from date (default today)")
            ("to_date", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.to_date)),
                "to date (default not filled)");
        return cfg_opts;
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }//handle
};//class HandleAdminClientRegistrarCreateCertificationArgsGrp

/**
 * \class HandleAdminClientRegistrarCreateGroupArgsGrp
 * \brief admin client registrar_create_group options handler
 */
class HandleAdminClientRegistrarCreateGroupArgsGrp : public HandleCommandGrpArgs
{
public:
    RegistrarCreateGroupArgs params;
    CommandDescription get_command_option()
    {
        return CommandDescription("registrar_create_group");
    }
    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("registrar_create_group options")));
        cfg_opts->add_options()
            ("registrar_create_group", "create registrar group")
            ("registrar_group", boost::program_options
                ::value<Checked::string>()->notifier(save_arg<std::string>(params.registrar_group))
                , "registrar group name")
                ;
        return cfg_opts;
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }//handle
};//class HandleAdminClientRegistrarCreateGroupArgsGrp

/**
 * \class HandleAdminClientRegistrarIntoGroupArgsGrp
 * \brief admin client registrar_into_group options handler
 */
class HandleAdminClientRegistrarIntoGroupArgsGrp : public HandleCommandGrpArgs
{
public:
    RegistrarIntoGroupArgs params;
    CommandDescription get_command_option()
    {
        return CommandDescription("registrar_into_group");
    }
    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("registrar_into_group options")));
        cfg_opts->add_options()
            ("registrar_into_group", "add registrar into group")
            ("handle", boost::program_options
                ::value<Checked::string>()->notifier(save_arg<std::string>(params.handle))
                , "registrar handle")
            ("from_date", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.from_date))
                , "from date (default today)")
            ("to_date", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.to_date))
                , "to date (default not filled)")
            ("registrar_group", boost::program_options
                ::value<Checked::string>()->notifier(save_arg<std::string>(params.registrar_group))
                , "registrar group name")
                ;
        return cfg_opts;
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }//handle
};//class HandleAdminClientRegistrarIntoGroupArgsGrp

/**
 * \class HandleAdminClientRegistrarListArgsGrp
 * \brief admin client registrar_list options handler
 */
class HandleAdminClientRegistrarListArgsGrp : public HandleCommandGrpArgs
{
public:
    RegistrarListArgs params;
    CommandDescription get_command_option()
    {
        return CommandDescription("registrar_list");
    }
    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("registrar_list options")));
        cfg_opts->add_options()
            ("registrar_list", "list all registrars (via filters)")
            ("id", boost::program_options
                ::value<Checked::id>()->notifier(save_optional_id(params.id))
                , "filter records with specific id nubmer")
            ("handle", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.handle))
                , "filter records with specific handle")
            ("name_name", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.name_name))
                , "filter records with specific name")
            ("organization", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.organization))
                , "show only records with specific organization name")
            ("city", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.city))
                , "show only records with specific city")
            ("email", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.email))
                , "show only records with specific email address")
            ("country", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.country))
                , "show only records with specific country")
            ;
        return cfg_opts;
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }//handle
};//class HandleAdminClientRegistrarListArgsGrp

template <>
class save_arg<std::vector<boost::asio::ip::address>>
{
public:
    //ctor taking reference to variable where arg value will be stored
    save_arg(std::vector<boost::asio::ip::address>& val) : val_(val) {}

    void operator()(const std::vector<std::string>& arg)
    {
        val_.clear();
        val_.reserve(arg.size());
        for (const auto& str_addr : arg)
        {
            val_.push_back(boost::asio::ip::address::from_string(str_addr));
        }
    }
private:
    std::vector<boost::asio::ip::address>& val_;
};

/**
 * \class HandleAdminClientZoneNsAddArgsGrp
 * \brief admin client zone_ns_add options handler
 */
class HandleAdminClientZoneNsAddArgsGrp : public HandleCommandGrpArgs
{
public:
    CommandDescription get_command_option()
    {
        return CommandDescription("zone_ns_add");
    }
    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("zone_ns_add options")));
        cfg_opts->add_options()
            ("zone_ns_add", "add new nameserver to the zone")
            ("zone_fqdn", boost::program_options
                ::value<Checked::string>()->required()
                ->notifier(save_arg<std::string>(params.zone_fqdn)),
                "zone fqdn")
            ("ns_fqdn", boost::program_options
                ::value<Checked::string>()->required()
                ->notifier(save_arg<std::string>(params.ns_fqdn)),
                "nameserver fqdn")
            ("addr", boost::program_options
                ::value<Checked::ip_addresses>()->multitoken()
                ->notifier(save_arg<std::vector<boost::asio::ip::address>>(params.addrs)),
                "nameserver addresses");
        return cfg_opts;
    }
    std::size_t handle(int argc, char* argv[], FakedArgs& fa, std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }
    ZoneNsAddArgs params;
};

/**
 * \class HandleAdminClientRegistrarAclAddArgsGrp
 * \brief admin client registrar_acl_add options handler
 */
class HandleAdminClientRegistrarAclAddArgsGrp : public HandleCommandGrpArgs
{
public:
    RegistrarAclAddArgs params;
    CommandDescription get_command_option()
    {
        return CommandDescription("registrar_acl_add");
    }
    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("registrar_acl_add options")));
        cfg_opts->add_options()
            ("registrar_acl_add", "add new certificate for registrar")
            ("handle", boost::program_options
                ::value<Checked::string>()->required()
                ->notifier(save_arg<std::string>(params.handle)),
                "registrar handle")
            ("certificate", boost::program_options
                ::value<Checked::string>()->required()
                ->notifier(save_arg<std::string>(params.certificate)),
                "registrar certificate fingerprint")
            ("password", boost::program_options
                ::value<Checked::string>()->required()
                ->notifier(save_arg<std::string>(params.password)),
                "registrar password")
            ;
        return cfg_opts;
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }//handle
};//class HandleAdminClientRegistrarAclAddArgsGrp

/**
 * \class HandleAdminClientPriceAddArgsGrp
 * \brief admin client price_add options handler
 */
class HandleAdminClientPriceAddArgsGrp : public HandleCommandGrpArgs
{
public:
    PriceAddArgs params;
    CommandDescription get_command_option()
    {
        return CommandDescription("price_add");
    }
    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("price_add options")));
        cfg_opts->add_options()
            ("price_add", "add price")
            ("valid_from", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.valid_from))
                , "price valid from UTC datetime e.g. '2006-09-09 19:15:56'")
            ("valid_to", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.valid_to))
                , "price valid to UTC datetime e.g. '2007-09-29 19:15:56'")
            ("operation_price", boost::program_options
                ::value<Checked::string_fpnumber>()->notifier(save_arg_using_to_string<optional_string>(params.operation_price))
                , "operation price like: 140.00")
            ("period", boost::program_options
                ::value<Checked::ulong>()->notifier(save_optional_ulong(params.period))
                , "period in years")
            ("zone_fqdn", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.zone_fqdn))
                , "zone fqdn")
            ("zone_id", boost::program_options
                ::value<Checked::id>()->notifier(save_optional_id(params.zone_id))
                , "zone id")
            ("operation", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.operation))
                , "charged operation like: CreateDomain, RenewDomain, GeneralEppOperation")
            ("enable_postpaid_operation", boost::program_options
                ::value<bool>()->zero_tokens()->notifier(save_arg<bool>(params.enable_postpaid_operation))
                , "operation charge don't need prepaid credit")
                ;
        return cfg_opts;
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }//handle
};//class HandleAdminClientRegistrarAclAddArgsGrp

/*
 * \class HandleAdminClientChargeRequestFeeArgsGrp
 * \brief charge request fee to a registrar(s)
 */
class HandleAdminClientChargeRequestFeeArgsGrp : public HandleCommandGrpArgs {
public:
    ChargeRequestFeeArgs params;
    CommandDescription get_command_option()
    {
        return CommandDescription("charge_request_fee");
    }
    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                        new boost::program_options::options_description(
                                std::string("charge_request_fee options")));

        cfg_opts->add_options()
                ("charge_request_fee", "Charge fee for requests over limit to a registrar")
                ("only_registrar", boost::program_options
                        ::value<Checked::string>()->notifier(save_optional_string(params.only_registrar)),
                         "Charge requests over limit only to specified registrar handle")
                ("all_except_registrars", boost::program_options
                        ::value<Checked::string>()->notifier(save_optional_string(params.except_registrars)),
                         "Charge requests over limit to all registrars except specified comma separated handles")
                ("poll_msg_period_to", boost::program_options
                        ::value<Checked::string>()->notifier(save_optional_string(params.poll_msg_period_to)),
                         "`Period to' of the poll message on which charging should be based. "
                         "For charging a whole month, It has to be first day of the next month. "
                         "e.g.: 2011-10-01 would call charging for September 2011. "
                         "Default is first day of current month, which means charging for previous month. ")
                         ;

        return cfg_opts;
    }
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }//handle
};

/**
 * \class HandleAdminClientNotifyStateChangesArgsGrp
 * \brief admin client notify_state_changes options handler
 */
class HandleAdminClientNotifyStateChangesArgsGrp : public HandleCommandGrpArgs
{
public:
    NotifyStateChangesArgs params;
    CommandDescription get_command_option()
    {
        return CommandDescription("notify_state_changes");
    }
    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("notify_state_changes options")));
        cfg_opts->add_options()
            ("notify_state_changes", "send emails to contacts about object state changes")
            ("notify_except_types", boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.notify_except_types)),
                "list of notification types ignored in notification")
            ("notify_limit", boost::program_options
                ::value<Checked::ulonglong>()->notifier(save_optional_ulonglong(params.notify_limit)),
                "limit for nubmer of emails generated in one pass (0=no limit)")
            ("notify_debug", boost::program_options
                ::value<bool>()->zero_tokens()->notifier(save_arg<bool>(params.notify_debug)),
                "debug");
        return cfg_opts;
    }//get_options_description
    std::size_t handle(int _argc, char* _argv[], FakedArgs& _fa, std::size_t _option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, _argc, _argv, _fa);
        return _option_group_index;
    }//handle
};//class HandleAdminClientNotifyStateChangesArgsGrp

/**
 * \class HandleAdminClientNotifyLettersPostservisSendArgsGrp
 * \brief admin client notify_letters_postservis_send options handler
 */
class HandleAdminClientNotifyLettersPostservisSendArgsGrp : public HandleCommandGrpArgs
{
public:
    optional_string hpmail_config;
    CommandDescription get_command_option()
    {
        return CommandDescription("notify_letters_postservis_send");
    }
    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("notify_letters_postservis_send options")));
        cfg_opts->add_options()
            ("notify_letters_postservis_send", "send generated PDF notification letters to postservis")

            ("hpmail_config", boost::program_options
                ::value<Checked::string>()->default_value(HPMAIL_CONFIG)
                     ->notifier(save_optional_string(hpmail_config))
                , "configuration file for Postservis client (hpmail)")
                ;
        return cfg_opts;
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }//handle
};//class HandleAdminClientNotifyLettersPostservisSendArgsGrp

/**
 * \class HandleAdminClientNotifyRegisteredLettersManualSendArgsGrp
 * \brief admin client notify_registered_letters_manual_send options handler
 */
class HandleAdminClientNotifyRegisteredLettersManualSendArgsGrp : public HandleCommandGrpArgs
{
public:

    RegisteredLettersManualSendArgs params;

    CommandDescription get_command_option()
    {
        return CommandDescription("notify_registered_letters_manual_send");
    }
    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("notify_registered_letters_manual_send options")));
        cfg_opts->add_options()
            ("notify_registered_letters_manual_send", "manual send of generated registered letters")
            ("working_directory", boost::program_options
                ::value<Checked::string>()->default_value("./")
                     ->notifier(save_optional_string(params.working_directory))
                , "working directory used for letter files")
            ("email", boost::program_options
                ::value<Checked::string>()
                     ->notifier(save_arg<std::string>(params.email))
                , "email address where to send concatenated letter files")
            ("shell_cmd_timeout", boost::program_options
                ::value<Checked::ulong>()->default_value(10)
                     ->notifier(save_arg<unsigned long>(params.shell_cmd_timeout))
                , "set alarm timeout for shell commands [s]")
            ;
        return cfg_opts;
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }//handle
};//class HandleAdminClientNotifyRegisteredLettersManualSendArgsGrp


/**
 * \class HandleAdminClientNotifySmsSendArgsGrp
 * \brief admin client notify_sms_send options handler
 */
class HandleAdminClientNotifySmsSendArgsGrp : public HandleCommandGrpArgs
{
public:
    optional_string cmdline_sms_command;
    CommandDescription get_command_option()
    {
        return CommandDescription("notify_sms_send");
    }
    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("notify_sms_send options")));
        cfg_opts->add_options()
            ("notify_sms_send", "send generated SMS notification messages")

            ("sms_command",boost::program_options
                    ::value<Checked::string>()->notifier(save_optional_string(cmdline_sms_command))
                     ,"shell command to send saved sms messages")
                ;
        return cfg_opts;
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }//handle
};//class HandleAdminClientNotifyLettersPostservisSendArgsGrp

/**
 * \class HandleAdminClientEnumParameterChangeArgsGrp
 * \brief admin client enum_parameter_change options handler
 */
class HandleAdminClientEnumParameterChangeArgsGrp : public HandleCommandGrpArgs
{
public:
    EnumParameterChangeArgs params;
    CommandDescription get_command_option()
    {
        return CommandDescription("enum_parameter_change");
    }
    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("enum_parameter_change options")));
        cfg_opts->add_options()
            ("enum_parameter_change", "change value of enum_parameter by name")
            ("parameter_name",boost::program_options
                    ::value<Checked::string>()->notifier(save_arg<std::string>(params.parameter_name))
                     ,"enum parameter name")
             ("parameter_value",boost::program_options
                     ::value<Checked::string>()->notifier(save_arg<std::string>(params.parameter_value))
                      ,"enum parameter value")
                ;
        return cfg_opts;
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }//handle
};//class HandleAdminClientEnumParameterChangeArgsGrp

class HandleAdminClientDomainLifecycleParametersArgsGrp : public HandleCommandGrpArgs
{
public:
    const DomainLifecycleParams& get_params() const noexcept;
    CommandDescription get_command_option() override;
    std::shared_ptr<boost::program_options::options_description> get_options_description() override;
    std::size_t handle(int argc, char* argv[], FakedArgs& fa, std::size_t option_group_index) override;
private:
    DomainLifecycleParams params_;
};

/**
 * \class HandleAdminClientObjectNewStateRequestNameArgsGrp
 * \brief admin client object_new_state_request_name options handler
 */
class HandleAdminClientObjectNewStateRequestNameArgsGrp : public HandleCommandGrpArgs
{
public:
    ObjectNewStateRequestNameArgs params;
    CommandDescription get_command_option()
    {
        return CommandDescription("object_new_state_request_name");
    }
    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("object_new_state_request_name options")));
        cfg_opts->add_options()
            ("object_new_state_request_name","set request for object state by name")
            ("object_name,n",boost::program_options
                ::value<Checked::string>()->notifier(save_arg<std::string>(params.object_name))
                 ,"object handle")
            ("object_type,o",boost::program_options
                ::value<Checked::ulong>()->notifier(save_arg<unsigned long>(params.object_type))
                  ,"object type number: 1 - contact,  2 - nsset, 3 - domain, 4 - keyset")
            ("object_state_name,s",boost::program_options
                ::value<std::vector<std::string> >()->notifier(insert_arg< std::vector<std::string> >(params.object_state_name))
                   ,"object state name , may appear multiple times like \" -s serverBlocked -s serverOutzoneManual \" , from db table enum_object_states: "
                   "serverRenewProhibited "
                   "serverOutzoneManual "
                   "serverInzoneManual "
                   "serverBlocked "
                   "expirationWarning "
                   "expired "
                   "unguarded "
                   "validationWarning1 "
                   "validationWarning2 "
                   "notValidated "
                   "nssetMissing "
                   "outzone "
                   "serverRegistrantChangeProhibited "
                   "deleteWarning "
                   "outzoneUnguarded "
                   "serverDeleteProhibited "
                   "serverTransferProhibited "
                   "serverUpdateProhibited "
                   "linked "
                   "deleteCandidate "
                   "conditionallyIdentifiedContact "
                   "identifiedContact "
                   "validatedContact "
                   "mojeidContact "
                   )
                   ("valid_from,f",boost::program_options
                       ::value<Checked::string>()->notifier(save_optional_string(params.valid_from))
                        ,"object state request valid from date time , impl default is now, example: 2002-01-31T10:00:01,123456789 ")

                    ("valid_to,t",boost::program_options
                        ::value<Checked::string>()->notifier(save_optional_string(params.valid_to))
                         ,"object state request valid to date time , impl default is empty, example: 2003-01-31T10:00:01,123456789 ")
                     ("with_update,u", boost::program_options
                         ::value<bool>()->zero_tokens()->notifier(save_arg<bool>(params.update_object_state))
                         , "update state of the object")

                ;
        return cfg_opts;
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }//handle
};//class HandleAdminClientObjectNewStateRequestNameArgsGrp

/**
 * \class HandleAdminClientObjectUpdateStatesArgsGrp
 * \brief admin client object_update_states options handler
 */
class HandleAdminClientObjectUpdateStatesArgsGrp : public HandleCommandGrpArgs
{
public:
    ObjectUpdateStatesArgs params;
    CommandDescription get_command_option()
    {
        return CommandDescription("object_update_states");
    }
    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("object_update_states options")));
        cfg_opts->add_options()
            ("object_update_states","globally update all states of all objects")
            ("object_id",boost::program_options
                    ::value<Checked::id>()->notifier(save_optional_id(params.object_id))
                     ,"object id")
                ;
        return cfg_opts;
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }//handle
};//class HandleAdminClientObjectUpdateStatesArgsGrp


/**
 * \class HandleAdminClientObjectDeleteCandidatesArgsGrp
 * \brief admin client object_delete_candidates options handler
 */
struct HandleAdminClientObjectDeleteCandidatesArgsGrp:HandleCommandGrpArgs
{
    CommandDescription get_command_option()override
    {
        return CommandDescription("object_delete_candidates");
    }

    std::shared_ptr<boost::program_options::options_description>
    get_options_description()override
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("object_delete_candidates options")));
        cfg_opts->add_options()
            ("object_delete_candidates", "delete objects of given types with state delete_candidate")
            ("object_delete_types",
             boost::program_options::value<Checked::string>()->notifier(
                     save_optional_string(delete_objects_params.object_delete_types)),
             "only this type of object will be delete during mass delete")
            ("object_delete_limit",
             boost::program_options::value<Checked::ulonglong>()->notifier(
                     save_optional_ulonglong(delete_objects_params.object_delete_limit)),
             "limit for object deleting")
            ("object_delete_parts",
             boost::program_options::value<Checked::ulonglong>()->notifier(
                     save_optional_ulonglong(delete_objects_params.object_delete_parts)),
             "limit for object deleting set to (total count/parts) - just one part of total count will be processed")
             ("object_delete_spread_during_time",
              boost::program_options::value<Checked::ulonglong>()->notifier(
                      save_optional_ulonglong(delete_objects_params.object_delete_spread_during_time)),
              "spread the deletion into time specified in seconds")
            ("object_delete_debug",
             boost::program_options::value<bool>()->zero_tokens()->notifier(
                     save_arg<bool>(delete_objects_params.object_delete_debug)),
             "object delete debug on/off");
        return cfg_opts;
    }

    std::size_t handle(
            int argc,
            char* argv[],
            FakedArgs& fa,
            std::size_t option_group_index)override
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }
    DeleteObjectsArgs delete_objects_params;
};


/**
 * \class HandleAdminClientObjectRegularProcedureArgsGrp
 * \brief admin client object_regular_procedure options handler
 */
struct HandleAdminClientObjectRegularProcedureArgsGrp:HandleCommandGrpArgs
{
    CommandDescription get_command_option()override
    {
        return CommandDescription("object_regular_procedure");
    }

    std::shared_ptr<boost::program_options::options_description>
    get_options_description()override
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("object_regular_procedure options")));
        cfg_opts->add_options()
            ("object_regular_procedure",
             "shortcut for 2x update_object_states, "
             "notify_state changes, "
             "poll_create_statechanges, "
             "object_delete_candidates, "
             "poll_create_low_credit, "
             "notify_letters_create")
            ("poll_except_types",
             boost::program_options::value<Checked::string>()->notifier(
                     save_optional_string(regular_procedure_params.poll_except_types)),
             "list of poll message types ignored in creation (only states now)")
            ("object_delete_types",
             boost::program_options::value<Checked::string>()->notifier(
                     save_optional_string(delete_objects_params.object_delete_types)),
             "only this type of object will be delete during mass delete")
            ("notify_except_types",
             boost::program_options::value<Checked::string>()->notifier(
                     save_optional_string(regular_procedure_params.notify_except_types)),
             "list of notification types ignored in notification")
            ("send_notifications",
             boost::program_options::value<bool>()->zero_tokens()->notifier(
                     save_arg<bool>(regular_procedure_params.send_notifications)),
             "send notifications via old build-in mailer (instead of new independent client)")
            ("object_delete_limit",
             boost::program_options::value<Checked::ulonglong>()->notifier(
                     save_optional_ulonglong(delete_objects_params.object_delete_limit)),
             "limit for object deleting")
            ("object_delete_debug",
             boost::program_options::value<bool>()->zero_tokens()->notifier(
                     save_arg<bool>(delete_objects_params.object_delete_debug)),
             "object delete debug on/off");
        return cfg_opts;
    }

    std::size_t handle(
            int argc,
            char* argv[],
            FakedArgs& fa,
            std::size_t option_group_index)override
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }
    ObjectRegularProcedureArgs regular_procedure_params;
    DeleteObjectsArgs delete_objects_params;
};


/**
 * \class HandleAdminClientFileListArgsGrp
 * \brief admin client file_list options handler
 */
class HandleAdminClientFileListArgsGrp : public HandleCommandGrpArgs
{
public:
    FileListArgs params;

    CommandDescription get_command_option()
    {
        return CommandDescription("file_list");
    }
    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("file_list options")));
        cfg_opts->add_options()
            ("file_list","list all files")
            ("id", boost::program_options
                ::value<Checked::id>()->notifier(save_optional_id(params.id))
                , "file id")
            ("name_name",boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.name_name))
                 ,"file name")
            ("crdate", boost::program_options
                 ::value<Checked::string>()->notifier(save_optional_string(params.crdate))
                 , "create date, arg format viz --help_dates")
            ("path",boost::program_options
             ::value<Checked::string>()->notifier(save_optional_string(params.path))
              ,"file path")
            ("mime",boost::program_options
              ::value<Checked::string>()->notifier(save_optional_string(params.mime))
               ,"file mime type")
            ("size", boost::program_options
               ::value<Checked::ulonglong>()->notifier(save_optional_ulonglong(params.size))
               , "file size")
            ("limit", boost::program_options
              ::value<Checked::ulonglong>()->notifier(save_arg<unsigned long long>(params.limit))
              , "limit")
                ;
        return cfg_opts;
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }//handle
};//class HandleAdminClientFileListArgsGrp

/**
 * \class HandleInitDomainNameValidationArgsGrp
 * \brief admin client init_domain_name_validation options handler
 */
class HandleInitDomainNameValidationCheckersArgsGrp : public HandleCommandGrpArgs
{
public:
    DomainNameValidationCheckersInitArgs params;
    CommandDescription get_command_option()
    {
        return CommandDescription("init_domain_name_validation");
    }
    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("init_domain_name_validation options")));
        cfg_opts->add_options()
            ("init_domain_name_validation","save domain name syntax checker names into database")
            ("auto,a", boost::program_options
                                 ::value<bool>()->zero_tokens()->notifier(save_arg<bool>(params.serch_checkers_by_prefix))
                                 , "get checker names to save from searching selfregistered implementations using checker name prefix")
            ("checker_name_prefix,p",boost::program_options
                ::value<Checked::string>()->default_value("dncheck_")->notifier(save_optional_string(params.checker_name_prefix))
                 ,"prefix of checker implementation name used for searching")
            ("checker_name,c",boost::program_options
                  ::value<std::vector<std::string> >()->notifier(insert_arg< std::vector<std::string> >(params.checker_names))
                   ,"explicit specification of checker name to save, may appear multiple times like \" -c dncheck_single_digit_labels -c dncheck_no_consecutive_hyphens \""
                   "checker names are defined in implementation and are used for selfregistration of the checker into factory")
                ;
        return cfg_opts;
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }//handle
};//class HandleInitDomainNameValidationCheckersArgsGrp

/**
 * \class HandleDomainNameValidationByZoneArgsGrp
 * \brief admin client set_zone_domain_name_validation options handler
 */
class HandleDomainNameValidationByZoneArgsGrp : public HandleCommandGrpArgs
{
public:
    ZoneDomainNameValidationCheckersArgs params;
    CommandDescription get_command_option()
    {
        return CommandDescription("set_zone_domain_name_validation");
    }
    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("set_zone_domain_name_validation options")));
        cfg_opts->add_options()
            ("set_zone_domain_name_validation","set checkers applied to domain names per zone")
            ("zone_name,z",boost::program_options
                ::value<Checked::string>()->notifier(save_optional_string(params.zone_name))
                 ,"name of the zone to configure checkers")
            ("checker_name,c",boost::program_options
                  ::value<std::vector<std::string> >()->notifier(insert_arg< std::vector<std::string> >(params.checker_names))
                   ,"checker names to apply per zone to domain name e.g. \" -c dncheck_rfc1035_preferred_syntax -c dncheck_no_consecutive_hyphens \"")
                ;
        return cfg_opts;
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }//handle
};//class HandleDomainNameValidationByZoneArgsGrp


/**
 * admin client contact_verification_fill_queue_automatic_testsuite options handler
 */
class HandleContactVerificationFillQueueArgsGrp : public HandleCommandGrpArgs {
    private:
        const char* name() const { return "contact_verification_fill_queue"; }
        typedef boost::program_options::options_description options_description;

    public:
        ContactVerificationFillQueueArgs params;

        CommandDescription get_command_option() { return CommandDescription(name()); }

        std::shared_ptr<options_description> get_options_description() {
            std::shared_ptr<options_description> cfg_opts( new options_description(name()) );
            cfg_opts->add_options()(name(), "fill contact checks queue");

            cfg_opts->add_options()("max_active_checks",
                boost::program_options::value<Checked::ulong>()
                    ->default_value(100)
                    ->notifier(save_arg<unsigned>(params.max_queue_length)),
                    "maximum number of all active checks");

            cfg_opts->add_options()("testsuite_handle",
                boost::program_options::value<Checked::string>()
                    ->notifier(save_arg<std::string>(params.testsuite_handle)),
                    "testsuite handle to use for new checks");

            cfg_opts->add_options()("country_code",
                boost::program_options::value<Checked::string>()
                    ->notifier(save_arg<std::string>(params.country_code)),
                    "enqueue only contacts with given country code");

            cfg_opts->add_options()("contact_role",
                boost::program_options::value<std::vector<std::string> >()->multitoken()
                    ->notifier(save_arg<std::vector<std::string> >(params.contact_roles)),
                    "enqueue only contacts with given contact role");

            cfg_opts->add_options()("contact_state",
                boost::program_options::value<std::vector<std::string> >()->multitoken()
                    ->notifier(save_arg<std::vector<std::string> >(params.contact_states)),
                    "enqueue only contacts with given contact state");

            return cfg_opts;
        }

        std::size_t handle( int argc, char* argv[],  FakedArgs &fa, std::size_t option_group_index) {
            boost::program_options::variables_map vm;
            handler_parse_args()(get_options_description(), vm, argc, argv, fa);

            return option_group_index;
        }
};


/**
 * admin client contact_verification_enqueue_check  options handler
 */
class HandleContactVerificationEnqueueCheckArgsGrp : public HandleCommandGrpArgs {
    private:
        const char* name() const { return "contact_verification_enqueue_check";}
        typedef boost::program_options::options_description options_description;

    public:
        ContactVerificationEnqueueCheckArgs params;

        CommandDescription get_command_option() { return CommandDescription(name()); }

        std::shared_ptr<options_description> get_options_description() {
            std::shared_ptr<options_description> cfg_opts( new options_description(name()) );
            cfg_opts->add_options()(name(), "enqueue check for contact specified by id with specified testsuite");
            cfg_opts->add_options()("contact_id",
                boost::program_options::value<Checked::ulonglong>()
                    ->notifier(save_arg<long long>(params.contact_id)),
                    "contact id");
            cfg_opts->add_options()("testsuite_handle",
                boost::program_options::value<Checked::string>()
                    ->notifier(save_arg<std::string>(params.testsuite_handle)),
                    "testsuite handle");

            return cfg_opts;
        }

        std::size_t handle( int argc, char* argv[],  FakedArgs &fa, std::size_t option_group_index) {
            boost::program_options::variables_map vm;
            handler_parse_args()(get_options_description(), vm, argc, argv, fa);

            return option_group_index;
        }
};


/**
 * admin client contact_verification_start_enqueued_checks  options handler
 */
class HandleContactVerificationStartEnqueuedChecksArgsGrp : public HandleCommandGrpArgs {
    private:
        const char* name() const { return "contact_verification_start_enqueued_checks";}
        typedef boost::program_options::options_description options_description;

    public:
        ContactVerificationStartEnqueuedChecksArgs params;

        CommandDescription get_command_option() { return CommandDescription(name()); }

        std::shared_ptr<options_description> get_options_description() {
            std::shared_ptr<options_description> cfg_opts( new options_description(name()) );
            cfg_opts->add_options()(name(), "start enqueued checks");

            cfg_opts->add_options()("cz_address_mvcr_xml_path",
                boost::program_options::value<Checked::string>()
                    ->required()
                    ->notifier(save_arg<std::string>(params.cz_address_mvcr_xml_path)),
                "cz_address_mvcr_xml_path");

            return cfg_opts;
        }

        std::size_t handle( int argc, char* argv[],  FakedArgs &fa, std::size_t option_group_index) {
            boost::program_options::variables_map vm;
            handler_parse_args()(get_options_description(), vm, argc, argv, fa);

            return option_group_index;
        }
};

/**
 * \class HandleAdminClientNotifyLettersOptysSendArgsGrp
 * \brief admin client notify_letters_optys_send options handler
 */
class HandleAdminClientNotifyLettersOptysSendArgsGrp : public HandleCommandGrpArgs
{
public:
    optional_string optys_config;
    CommandDescription get_command_option()
    {
        return CommandDescription("notify_letters_optys_send");
    }
    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("notify_letters_optys_send options")));
        cfg_opts->add_options()
            ("notify_letters_optys_send", "send generated PDF notification letters to optys")

            ("optys_config", boost::program_options
                ::value<Checked::string>()->default_value(OPTYS_CONFIG)
                     ->notifier(save_optional_string(optys_config))
                , "configuration file for Optys client")
                ;
        return cfg_opts;
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }//handle
};//class HandleAdminClientNotifyLettersOptysSendArgsGrp

/**
 * \class HandleAdminClientNotifyLettersOptysGetUndeliveredArgsGrp
 * \brief admin client notify_letters_optys_get_undelivered options handler
 */
class HandleAdminClientNotifyLettersOptysGetUndeliveredArgsGrp : public HandleCommandGrpArgs
{
public:
    optional_string optys_config;
    bool all_local_files_only;

    HandleAdminClientNotifyLettersOptysGetUndeliveredArgsGrp()
    : all_local_files_only(false)
    {}

    CommandDescription get_command_option()
    {
        return CommandDescription("notify_letters_optys_get_undelivered");
    }
    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("notify_letters_optys_get_undelivered options")));
        cfg_opts->add_options()
            ("notify_letters_optys_get_undelivered", "get ids of undelivered letters from optys")
            ("optys_config", boost::program_options
                ::value<Checked::string>()->default_value(OPTYS_CONFIG)
                     ->notifier(save_optional_string(optys_config))
                , "configuration file for Optys client")
            ("local,l", boost::program_options
                ::value<bool>()->zero_tokens()->notifier(save_arg<bool>(all_local_files_only))
                , "do not download new *.csv files and process all *.csv files in local_download_dir (viz Optys "
                    "configuration file for Optys client), otherwise process only currently downloaded new files")
                ;
        return cfg_opts;
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }//handle
};//class HandleAdminClientNotifyLettersOptysGetUndeliveredArgsGrp

/**
 * \class HandleAdminClientCreateExpiredDomainArgsGrp
 * \brief admin client create_expired_domain options handler
 */
class HandleAdminClientCreateExpiredDomainArgsGrp : public HandleCommandGrpArgs
{
    public:
        CreateExpiredDomainArgs params;

        CommandDescription get_command_option()
        {
            return CommandDescription("create_expired_domain");
        }

        std::shared_ptr<boost::program_options::options_description>
        get_options_description()
        {
            std::shared_ptr<boost::program_options::options_description> cfg_opts(
                    new boost::program_options::options_description(
                            std::string("create_expired_domain options")));
            cfg_opts->add_options()
                ("create_expired_domain", "create expired domain")
                ("fqdn", boost::program_options
                    ::value<Checked::string>()->required()
                        ->notifier(save_arg<std::string>(params.fqdn)),
                    "fully qualified domain name is domain object handle")
                ("registrant", boost::program_options
                    ::value<Checked::string>()->required()
                        ->notifier(save_arg<std::string>(params.registrant)),
                    "registrant")
                ("cltrid", boost::program_options
                    ::value<Checked::string>()->required()
                        ->notifier(save_arg<std::string>(params.cltrid)),
                    "cltrid")
                ("delete_existing", boost::program_options
                    ::value<bool>()->zero_tokens()
                        ->notifier(save_arg<bool>(params.delete_existing)),
                    "delete existing domain")
                ;
            return cfg_opts;
        }//get_options_description

        std::size_t handle( int argc, char* argv[], FakedArgs &fa, std::size_t option_group_index)
        {
            boost::program_options::variables_map vm;
            handler_parse_args()(get_options_description(), vm, argc, argv, fa);
            return option_group_index;
        }//handle
};//class HandleAdminClientCreateExpiredDomainArgsGrp

class HandleSendObjectEventNotificationEmailsArgsGrp : public HandleCommandGrpArgs {
    private:
        static const char* get_command_name() { return "notify_email_objects_events"; }
    public:
        CommandDescription get_command_option() {
            return CommandDescription( get_command_name() );
        }

        std::shared_ptr<boost::program_options::options_description> get_options_description() {
            std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description( std::string(get_command_name()) + " options" )
            );

            cfg_opts->add_options()
            (
                get_command_name(),
                boost::program_options::value<bool>()->zero_tokens(),
                "Send e-mails notifying events on objects in registry"
            );

            return cfg_opts;

        }

        std::size_t handle(int argc, char* argv[],  FakedArgs &fa, std::size_t option_group_index) {
            boost::program_options::variables_map vm;
            handler_parse_args()(get_options_description(), vm, argc, argv, fa);
            return option_group_index;
        }
};

struct HandleAdminClientProcessPublicRequestsArgsGrp : HandleCommandGrpArgs
{
    CommandDescription get_command_option() override
    {
        return CommandDescription("process_public_requests");
    }

    std::shared_ptr<boost::program_options::options_description>
    get_options_description() override
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("process_public_requests options")));
        cfg_opts->add_options()
             ("process_public_requests", "process_public_requests options")
             ("types", boost::program_options::value<std::vector<std::string> >()->multitoken()
                 ->notifier(save_arg<std::vector<std::string> >(process_public_requests_params.types)),
                 "list of types of public requests to be processed");
        return cfg_opts;
    }

    std::size_t handle(
            int argc,
            char* argv[],
            FakedArgs& fa,
            std::size_t option_group_index) override
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }
    ProcessPublicRequestsArgs process_public_requests_params;
};

struct HandleChargeRegistryAccessFeeAnnualArgsGrp : HandleCommandGrpArgs
{
    ChargeRegistryAccessFeeAnnualArgs params;

    CommandDescription get_command_option() override
    {
        return CommandDescription("charge_registry_access_fee_annual");
    }

    std::shared_ptr<boost::program_options::options_description>
    get_options_description() override
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("charge_registry_access_fee_annual options")));
        cfg_opts->add_options()
               ("charge_registry_access_fee_annual", "charge_registry_access_fee_annual options")
               ("registrars", boost::program_options::value<std::vector<std::string> >()->multitoken()
                   ->notifier(save_arg<std::vector<std::string>>(params.registrars)),
                   "space separated list of registrars to be processed")
               ("all_registrars_except", boost::program_options::value<std::vector<std::string> >()->multitoken()
                   ->notifier(save_arg<std::vector<std::string>>(params.except_registrars)),
                   "space separated list of registrars not to be processed")
               ("all_registrars",  boost::program_options::value<bool>()->zero_tokens()
                    ->notifier(save_arg<bool>(params.all_registrars)),
                    "process all registrars")
               ("zone", boost::program_options::value<std::string>()->required()
                   ->notifier(save_arg<std::string>(params.zone)),
                   "zone fqdn (for example 'cz')")
               ("year", boost::program_options::value<std::string>()->required()
                   ->notifier(save_arg<std::string>(params.year)),
                   "year [YYYY]")
               ("charge-to-end-of-previous-month", boost::program_options::value<bool>()
                   ->default_value(false)->zero_tokens()
                   ->notifier(save_arg<bool>(params.charge_to_end_of_previous_month)),
                   "timestamp of creation will not be the current time, but the last second of previous month (current month is based on system local date)");
        return cfg_opts;
    }

    std::size_t handle(
            int argc,
            char* argv[],
            FakedArgs& fa,
            std::size_t option_group_index) override
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }
};

struct HandleChargeRegistryAccessFeeMonthlyArgsGrp : HandleCommandGrpArgs
{
    ChargeRegistryAccessFeeMonthlyArgs params;

    CommandDescription get_command_option() override
    {
        return CommandDescription("charge_registry_access_fee_monthly");
    }

    std::shared_ptr<boost::program_options::options_description>
    get_options_description() override
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("charge_registry_access_fee_monthly options")));
        cfg_opts->add_options()
               ("charge_registry_access_fee_monthly", "charge_registry_access_fee_monthly options")
               ("registrars", boost::program_options::value<std::vector<std::string> >()->multitoken()
                   ->notifier(save_arg<std::vector<std::string>>(params.registrars)),
                   "space separated list of registrars to be processed")
               ("all_registrars_except", boost::program_options::value<std::vector<std::string> >()->multitoken()
                   ->notifier(save_arg<std::vector<std::string>>(params.except_registrars)),
                   "space separated list of registrars not to be processed")
               ("all_registrars",  boost::program_options::value<bool>()->zero_tokens()
                    ->notifier(save_arg<bool>(params.all_registrars)),
                    "process all registrars")
               ("zone", boost::program_options::value<std::string>()->required()
                   ->notifier(save_arg<std::string>(params.zone)),
                   "zone fqdn (for example 'cz')")
               ("month", boost::program_options::value<std::string>()->required()
                   ->notifier(save_arg<std::string>(params.year_month)),
                   "month [YYYY-MM]")
               ("charge-to-end-of-previous-month", boost::program_options::value<bool>()
                   ->default_value(false)->zero_tokens()
                   ->notifier(save_arg<bool>(params.charge_to_end_of_previous_month)),
                   "timestamp of creation will not be the current time, but the last second of previous month (current month is based on system local date)");
        return cfg_opts;
    }

    std::size_t handle(
            int argc,
            char* argv[],
            FakedArgs& fa,
            std::size_t option_group_index) override
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);
        return option_group_index;
    }
};

#endif
