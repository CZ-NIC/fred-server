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

    boost::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        boost::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("Admin client cmd domain_list options:")));
        cfg_opts->add_options()
                ("id", boost::program_options
                     ::value<unsigned long long>()
                          , "optional domain id")
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
                        std::string("Admin client cmd keyset_list options:")));
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

#endif //HANDLE_ADMINCLIENTSELECTION_ARGS_H_
