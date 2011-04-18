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
 *  @handle_sms_args.h
 *  sms sender config
 */

#ifndef HANDLE_SMS_ARGS_H_
#define HANDLE_SMS_ARGS_H_

#include <iostream>
#include <exception>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include "faked_args.h"
#include "handle_args.h"

namespace po = boost::program_options;

/**
 * \class HandleSmsArgs
 * \brief sms config
 */
class HandleSmsArgs : public HandleArgs
{
public:
    std::string command;

    boost::shared_ptr<po::options_description>
    get_options_description()
    {
        boost::shared_ptr<po::options_description> opts_descs(
                new po::options_description(std::string("Sms sender configuration")));
        opts_descs->add_options()
                ("sms.command",
                 po::value<std::string>(),
                 "send messages shell command")
                 ;

        return opts_descs;
    }//get_options_description
    void handle( int argc, char* argv[],  FakedArgs &fa)
    {
        po::variables_map vm;
        handler_parse_args(get_options_description(), vm, argc, argv, fa);

        command = (vm.count("sms.command") == 0
                ? std::string() : vm["sms.command"].as<std::string>());
    }//handle
};//HandleSmsArgs

/**
 * \class HandleSmsArgsGrp
 * \brief sms config with option groups
 */

class HandleSmsArgsGrp : public HandleGrpArgs
                            , private HandleSmsArgs
{
public:

    boost::shared_ptr<boost::program_options::options_description>
        get_options_description()
    {
        return HandleSmsArgs::get_options_description();
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        HandleSmsArgs::handle(argc, argv, fa);
        return option_group_index;
    }//handle

    const std::string& get_sms_command()
        {return HandleSmsArgs::command;}
};//class HandleSmsArgsGrp

#endif //HANDLE_SMS_ARGS_H_
