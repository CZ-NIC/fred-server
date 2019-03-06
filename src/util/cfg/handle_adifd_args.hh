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
 *  @handle_adifd_args.h
 *  admin interface config
 */

#ifndef HANDLE_ADIFD_ARGS_HH_554360F06B964373B1F5A82419BE7708
#define HANDLE_ADIFD_ARGS_HH_554360F06B964373B1F5A82419BE7708

#include <iostream>
#include <exception>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include "src/util/cfg/faked_args.hh"
#include "src/util/cfg/handle_args.hh"

namespace po = boost::program_options;

/**
 * \class HandleAdifdArgs
 * \brief admin interface config
 */
class HandleAdifdArgs : public HandleArgs
{
public:
    unsigned adifd_session_max;
    unsigned adifd_session_timeout;
    unsigned adifd_session_garbage;


    std::shared_ptr<po::options_description>
    get_options_description()
    {
        std::shared_ptr<po::options_description> opts_descs(
                new po::options_description(std::string("Admin interface configuration")));
        opts_descs->add_options()
                ("adifd.session_max",
                 po::value<unsigned>()->default_value(0),
                 "ADIFD maximum number of sessions (0 means not limited)")
                ("adifd.session_timeout",
                 po::value<unsigned>()->default_value(3600),
                 "ADIFD session timeout")
                ("adifd.session_garbage",
                 po::value<unsigned>()->default_value(150),
                 "ADIFD session garbage interval")
                ;

        return opts_descs;
    }//get_options_description
    void handle( int argc, char* argv[],  FakedArgs &fa)
    {
        po::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);

        adifd_session_max = vm["adifd.session_max"].as<unsigned>();
        adifd_session_timeout = vm["adifd.session_timeout"].as<unsigned>();
        adifd_session_garbage = vm["adifd.session_garbage"].as<unsigned>();
    }//handle
};

#endif
