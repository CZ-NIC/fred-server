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
 *  @handle_clienthp_args.h
 *  client-hp configuration
 */

#ifndef HANDLE_CLIENTHP_ARGS_H_
#define HANDLE_CLIENTHP_ARGS_H_

#include <iostream>
#include <exception>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include "cfg/faked_args.h"
#include "cfg/handle_args.h"


/**
 * \class HandleClientHPArgs
 * \brief client-hp options handler
 */
class HandleClientHPArgs : public HandleArgs
{
public:
    std::string do_action ;//action to perform

    boost::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        boost::shared_ptr<boost::program_options::options_description> opts(
                new boost::program_options::options_description(
                        std::string("ClientHP configuration")));
        opts->add_options()
                ("do", boost::program_options
                            ::value<std::string>()->default_value(std::string("upload"))
                             , "what to do \"upload\" or \"statecheck\"");
        return opts;
    }//get_options_description
    void handle( int argc, char* argv[],  FakedArgs &fa)
    {
        boost::program_options::variables_map vm;
        handler_parse_args(get_options_description(), vm, argc, argv, fa);

        do_action = (vm.count("do") == 0 ? "" : vm["do"].as<std::string>());

    }//handle
};

/**
 * \class HandleClientHPArgsGrp
 * \brief client-hp options handler
 */
class HandleClientHPArgsGrp : public HandleGrpArgs
{
public:
    std::string do_action ;//action to perform

    boost::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        boost::shared_ptr<boost::program_options::options_description> opts(
                new boost::program_options::options_description(
                        std::string("ClientHP configuration")));
        opts->add_options()
                ("do", boost::program_options
                            ::value<std::string>()->default_value(std::string("upload"))
                             , "what to do \"upload\" or \"statecheck\"");
        return opts;
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa , std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args(get_options_description(), vm, argc, argv, fa);

        do_action = (vm.count("do") == 0 ? "" : vm["do"].as<std::string>());

        //option group selection
        if(do_action.compare("upload") == 0) option_group_index = 0;
        if(do_action.compare("statecheck") == 0) option_group_index = 1;

        return option_group_index;
    }//handle
};


#endif //HANDLE_CLIENTHP_ARGS_H_
