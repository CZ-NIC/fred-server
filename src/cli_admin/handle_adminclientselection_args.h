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

/**
 *  @handle_adminclientselection_args.h
 *  admin client configuration
 */

#ifndef HANDLE_ADMINCLIENTSELECTION_ARGS_H_
#define HANDLE_ADMINCLIENTSELECTION_ARGS_H_

#include <iostream>
#include <exception>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include "util/cfg/faked_args.h"
#include "util/cfg/handle_args.h"

/**
 * \class HandleAdminClientSelectionArgsGrp
 * \brief admin client selection options handler
 */
class HandleAdminClientSelectionArgsGrp : public HandleGrpArgs
{
public:
    std::string selected_command;

    boost::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        boost::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("Admin client command selection")));
        cfg_opts->add_options()
                ("cmd", boost::program_options
                     ::value<std::string>()
                          , "client command selection is required, possible arg values are: domain_list, keyset_list, contact_list")
                ;
        return cfg_opts;
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args(get_options_description(), vm, argc, argv, fa);

        selected_command = (vm.count("cmd") == 0
                        ? std::string("") : vm["cmd"].as<std::string>());

        //modify option_group_index according to selected_client
        if (selected_command.compare("domain_list") == 0)
            option_group_index = 0;
        else if (selected_command.compare("keyset_list") == 0)
            option_group_index = 1;
        else if (selected_command.compare("contact_list") == 0)
            option_group_index = 2;
        else
            throw std::runtime_error(
                    "HandleAdminClientSelectionArgsGrp: no client command selected, use option: --cmd=arg");
        return option_group_index;
    }//handle
};//class HandleAdminClientSelectionArgsGrp

typedef struct {std::string value; bool is_value_set;} optional_string;
typedef struct {unsigned long long value; bool is_value_set;} optional_id;

/**
 * \class HandleAdminClientDomainListArgsGrp
 * \brief admin client domain_list options handler
 */
class HandleAdminClientDomainListArgsGrp : public HandleGrpArgs
{
public:
    optional_id domain_id;
    optional_string fqdn;
    optional_string domain_handle;
    optional_id nsset_id;
    optional_string nsset_handle;
    bool any_nsset;
    optional_id keyset_id;
    optional_string keyset_handle;
    bool any_keyset;
    optional_id zone_id;
    optional_id registrant_id;
    optional_string registrant_handle;
    optional_string registrant_name;
    optional_id admin_id;
    optional_string admin_handle;
    optional_string admin_name;
    optional_id registrar_id;
    optional_string registrar_handle;
    optional_string registrar_name;
    optional_string crdate;
    optional_string deldate;
    optional_string update;
    optional_string transdate;
    bool full_list;

    boost::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        boost::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("Admin client command domain_list options")));
        cfg_opts->add_options()
            ("id", boost::program_options
                 ::value<unsigned long long>()
                      , "domain id")
            ("fqdn", boost::program_options
               ::value<std::string>()
                ,"fully qualified domain name is domain object handle")
            ("handle", boost::program_options
                 ::value<std::string>()
                  ,"domain object handle is fully qualified domain name (fqdn)")
            ("nsset_id", boost::program_options
             ::value<unsigned long long>()
                  , "nsset id")
            ("nsset_handle", boost::program_options
            ::value<std::string>()
                , "nsset handle")
            ("any_nsset", "any nsset")
            ("keyset_id", boost::program_options
               ::value<unsigned long long>()
                    , "keyset id")
            ("keyset_handle", boost::program_options
            ::value<std::string>()
                , "keyset handle")
            ("any_keyset", "any keyset")
            ("zone_id", boost::program_options
               ::value<unsigned long long>()
                    , "zone id")
            ("registrant_id", boost::program_options
               ::value<unsigned long long>()
                    , "registrant id")
            ("registrant_handle", boost::program_options
                ::value<std::string>()
                , "registrant handle")
            ("registrant_name", boost::program_options
                ::value<std::string>()
                , "registrant name")
            ("admin_id", boost::program_options
               ::value<unsigned long long>()
                    , "admin id")
            ("admin_handle", boost::program_options
                ::value<std::string>()
                , "admin handle")
            ("admin_name", boost::program_options
                ::value<std::string>()
                , "admin name")
            ("registrar_id", boost::program_options
               ::value<unsigned long long>()
                    , "registrar id")
            ("registrar_handle", boost::program_options
                ::value<std::string>()
                , "registrar handle")
            ("registrar_name", boost::program_options
                ::value<std::string>()
                , "registrar name")
            ("crdate", boost::program_options
                ::value<std::string>()
                , "create date, arg format viz --help_dates")
            ("deldate", boost::program_options
                ::value<std::string>()
                , "delete date, arg format viz --help_dates")
            ("transdate", boost::program_options
                ::value<std::string>()
                , "transfer date, arg format viz --help_dates")
            ("full_list", "full list")
            ;
        return cfg_opts;
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args(get_options_description(), vm, argc, argv, fa);

        if((domain_id.is_value_set = vm.count("id")))
            domain_id.value = vm["id"].as<unsigned long long>();
        if((fqdn.is_value_set = vm.count("fqdn")))
            fqdn.value = vm["fqdn"].as<std::string>();
        if((domain_handle.is_value_set = vm.count("handle")))
            domain_handle.value = vm["handle"].as<std::string>();
        if((nsset_id.is_value_set = vm.count("nsset_id")))
            nsset_id.value = vm["nsset_id"].as<unsigned long long>();
        if((nsset_handle.is_value_set = vm.count("nsset_handle")))
            nsset_handle.value = vm["nsset_handle"].as<std::string>();
        any_nsset = vm.count("any_nsset");
        if((keyset_id.is_value_set = vm.count("keyset_id")))
            keyset_id.value = vm["keyset_id"].as<unsigned long long>();
        if((keyset_handle.is_value_set = vm.count("keyset_handle")))
            keyset_handle.value = vm["keyset_handle"].as<std::string>();
        any_keyset = vm.count("any_keyset");
        if((zone_id.is_value_set = vm.count("zone_id")))
            zone_id.value = vm["zone_id"].as<unsigned long long>();
        if((registrant_id.is_value_set = vm.count("registrant_id")))
            registrant_id.value = vm["registrant_id"].as<unsigned long long>();
        if((registrant_handle.is_value_set = vm.count("registrant_handle")))
            registrant_handle.value = vm["registrant_handle"].as<std::string>();
        if((registrant_name.is_value_set = vm.count("registrant_name")))
            registrant_name.value = vm["registrant_name"].as<std::string>();
        if((admin_id.is_value_set = vm.count("admin_id")))
            admin_id.value = vm["admin_id"].as<unsigned long long>();
        if((admin_handle.is_value_set = vm.count("admin_handle")))
            admin_handle.value = vm["admin_handle"].as<std::string>();
        if((admin_name.is_value_set = vm.count("admin_name")))
            admin_name.value = vm["admin_name"].as<std::string>();
        if((registrar_id.is_value_set = vm.count("registrar_id")))
            registrar_id.value = vm["registrar_id"].as<unsigned long long>();
        if((registrar_handle.is_value_set = vm.count("registrar_handle")))
            registrar_handle.value = vm["registrar_handle"].as<std::string>();
        if((registrar_name.is_value_set = vm.count("registrar_name")))
            registrar_name.value = vm["registrar_name"].as<std::string>();
        if((crdate.is_value_set = vm.count("crdate")))
            crdate.value = vm["crdate"].as<std::string>();
        if((deldate.is_value_set = vm.count("deldate")))
            deldate.value = vm["deldate"].as<std::string>();
        if((update.is_value_set = vm.count("update")))
            update.value = vm["update"].as<std::string>();
        if((transdate.is_value_set = vm.count("transdate")))
            transdate.value = vm["transdate"].as<std::string>();
        full_list = vm.count("full_list");

        return option_group_index;
    }//handle
};//class HandleAdminClientDomainListArgsGrp

/**
 * \class HandleAdminClientKeySetListArgsGrp
 * \brief admin client keyset_list options handler
 */
class HandleAdminClientKeySetListArgsGrp : public HandleGrpArgs
{
public:
    optional_id keyset_id;

    boost::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        boost::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("Admin client command keyset_list options")));
        cfg_opts->add_options()
                ("id", boost::program_options
                     ::value<unsigned long long>()
                          , "optional keyset id")
                ;
        return cfg_opts;
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args(get_options_description(), vm, argc, argv, fa);

        if((keyset_id.is_value_set = vm.count("id")))
        keyset_id.value = vm["id"].as<unsigned long long>();

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

    boost::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        boost::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("Admin client date and time format help option")));
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
        handler_parse_args(get_options_description(), vm, argc, argv, fa);

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


#endif //HANDLE_ADMINCLIENTSELECTION_ARGS_H_
