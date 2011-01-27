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
 *  @handle_adifd_args.h
 *  admin interface config
 */

#ifndef HANDLE_ADIFD_ARGS_H_
#define HANDLE_ADIFD_ARGS_H_

#include <iostream>
#include <exception>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include "faked_args.h"
#include "handle_args.h"

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


    boost::shared_ptr<po::options_description>
    get_options_description()
    {
        boost::shared_ptr<po::options_description> opts_descs(
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
        handler_parse_args(get_options_description(), vm, argc, argv, fa);

        adifd_session_max = vm["adifd.session_max"].as<unsigned>();
        adifd_session_timeout = vm["adifd.session_timeout"].as<unsigned>();
        adifd_session_garbage = vm["adifd.session_garbage"].as<unsigned>();
    }//handle
};

#endif //HANDLE_ADIFD_ARGS_H_
