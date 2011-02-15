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
 *  @handle_logd_args.h
 *  logger config
 */

#ifndef HANDLE_LOGD_ARGS_H_
#define HANDLE_LOGD_ARGS_H_

#include <iostream>
#include <exception>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include "faked_args.h"
#include "handle_args.h"

namespace po = boost::program_options;

/**
 * \class HandleLogdArgs
 * \brief logd config
 */
class HandleLogdArgs : public HandleArgs
{
public:
    std::string logd_monitoring_hosts_file;

    boost::shared_ptr<po::options_description>
    get_options_description()
    {
        boost::shared_ptr<po::options_description> opts_descs(
                new po::options_description(std::string("Logd configuration")));
        opts_descs->add_options()

                ("logd.monitoring_hosts_file",
                 po::value<std::string>()
                 ->default_value("/usr/local/etc/fred/monitoring_hosts.conf")
                 ,"File containing list of monitoring machines")
                 ;

        return opts_descs;
    }//get_options_description
    void handle( int argc, char* argv[],  FakedArgs &fa)
    {
        po::variables_map vm;
        handler_parse_args(get_options_description(), vm, argc, argv, fa);

        logd_monitoring_hosts_file = (vm.count("logd.monitoring_hosts_file") == 0
                ? std::string() : vm["logd.monitoring_hosts_file"].as<std::string>());
    }//handle
};

#endif //HANDLE_LOGD_ARGS_H_
