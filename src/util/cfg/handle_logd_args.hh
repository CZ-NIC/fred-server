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
 *  @handle_logd_args.h
 *  logger config
 */

#ifndef HANDLE_LOGD_ARGS_HH_72562AC0712641649BE566415B9A3B2F
#define HANDLE_LOGD_ARGS_HH_72562AC0712641649BE566415B9A3B2F

#include <iostream>
#include <exception>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include "src/util/cfg/faked_args.hh"
#include "src/util/cfg/handle_args.hh"

namespace po = boost::program_options;

/**
 * \class HandleLogdArgs
 * \brief logd config
 */
class HandleLogdArgs : public HandleArgs
{
public:
    std::string logd_monitoring_hosts_file;

    std::shared_ptr<po::options_description>
    get_options_description()
    {
        std::shared_ptr<po::options_description> opts_descs(
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
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);

        logd_monitoring_hosts_file = vm["logd.monitoring_hosts_file"].as<std::string>();
    }//handle
};

#endif
