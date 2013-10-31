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
 *  @handle_administrativeblocking_args.h
 *  administrativeblocking backend config
 */

#ifndef HANDLE_ADMINISTRATIVEBLOCKING_ARGS_H_
#define HANDLE_ADMINISTRATIVEBLOCKING_ARGS_H_

#include <iostream>
#include <exception>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include "faked_args.h"
#include "handle_args.h"

namespace po = boost::program_options;

/**
 * \class HandleAdministrativeBlockingArgs
 * \brief administrativeblocking backend config
 */
class HandleAdministrativeBlockingArgs : public HandleArgs
{
public:
    std::string registrar_handle;
    std::string hostname;
    bool demo_mode;
    bool notify_commands;

    boost::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        boost::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("AdministrativeBlocking server options")));

        cfg_opts->add_options()
                ("administrativeblocking.registrar_handle",
                 po::value<std::string>()->default_value("REG-ADMINISTRATIVEBLOCKING"),
                 "registrar used for administrativeblocking operations")
                ("administrativeblocking.hostname",
                 po::value<std::string>()->default_value("demo.administrativeblocking.cz"),
                 "server hostname")
                ("administrativeblocking.demo_mode",
                 po::value<bool>()->default_value(false),
                 "turn demo mode on/off")
                ("administrativeblocking.notify_commands",
                 po::value<bool>()->default_value(false),
                 "turn command notifier on/off");

        return cfg_opts;
    }//get_options_description
    void handle( int argc, char* argv[],  FakedArgs &fa)
    {
        boost::program_options::variables_map vm;
        handler_parse_args(get_options_description(), vm, argc, argv, fa);

        registrar_handle = vm["administrativeblocking.registrar_handle"].as<std::string>();
        hostname = vm["administrativeblocking.hostname"].as<std::string>();
        demo_mode = vm["administrativeblocking.demo_mode"].as<bool>();
        notify_commands = vm["administrativeblocking.notify_commands"].as<bool>();
    }//handle
};

#endif //HANDLE_ADMINISTRATIVEBLOCKING_ARGS_H_
