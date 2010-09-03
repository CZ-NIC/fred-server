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
 *  @handle_server_args.h
 *  server process common configuration
 */

#ifndef HANDLE_SERVER_ARGS_H_
#define HANDLE_SERVER_ARGS_H_

#include <iostream>
#include <exception>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include "faked_args.h"
#include "handle_args.h"


/**
 * \class HandleServerArgs
 * \brief common server process options handler
 */
class HandleServerArgs : public HandleArgs
{
public:
    bool do_daemonize;
    std::string pidfile_name;

    boost::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        boost::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("Common server process configuration")));
        cfg_opts->add_options()
                ("daemonize", "turn foreground process into daemon")
                ("pidfile", boost::program_options
                        ::value<std::string>()
                             , "process id file location");
        return cfg_opts;
    }//get_options_description
    void handle( int argc, char* argv[],  FakedArgs &fa)
    {
        boost::program_options::variables_map vm;
        handler_parse_args(get_options_description(), vm, argc, argv, fa);

        if (vm.count("daemonize"))
            do_daemonize = true;
        else
            do_daemonize = false;

        pidfile_name = (vm.count("pidfile") == 0
                ? std::string("") : vm["pidfile"].as<std::string>());
    }//handle
};

#endif //HANDLE_SERVER_ARGS_H_
