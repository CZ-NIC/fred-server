/*
 * Copyright (C) 2011-2019  CZ.NIC, z. s. p. o.
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
 *  @handle_sms_args.h
 *  sms sender config
 */

#ifndef HANDLE_SMS_ARGS_HH_0DC5647B14DF4EF1AD42BC79CF18FD27
#define HANDLE_SMS_ARGS_HH_0DC5647B14DF4EF1AD42BC79CF18FD27

#include <iostream>
#include <exception>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include "src/util/cfg/faked_args.hh"
#include "src/util/cfg/handle_args.hh"

namespace po = boost::program_options;

/**
 * \class HandleSmsArgs
 * \brief sms config
 */
class HandleSmsArgs : public HandleArgs
{
public:
    std::string command;

    std::shared_ptr<po::options_description>
    get_options_description()
    {
        std::shared_ptr<po::options_description> opts_descs(
                new po::options_description(std::string("Sms sender configuration")));
        opts_descs->add_options()
                ("sms.command",
                 po::value<std::string>()->default_value(""),
                 "send messages shell command")
                 ;

        return opts_descs;
    }//get_options_description
    void handle( int argc, char* argv[],  FakedArgs &fa)
    {
        po::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);

        command = vm["sms.command"].as<std::string>();
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

    std::shared_ptr<boost::program_options::options_description>
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

#endif
