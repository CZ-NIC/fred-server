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
 *  @handle_rifd_args.h
 *  registrar interface config
 */

#ifndef HANDLE_RIFD_ARGS_H_
#define HANDLE_RIFD_ARGS_H_

#include <iostream>
#include <exception>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include "faked_args.h"
#include "handle_args.h"

namespace po = boost::program_options;

/**
 * \class HandleRifdArgs
 * \brief registrar interface config
 */
class HandleRifdArgs : public HandleArgs
{
public:
    unsigned rifd_session_max;
    unsigned rifd_session_timeout;
    unsigned rifd_session_registrar_max;
    bool rifd_epp_update_domain_keyset_clear;

    boost::shared_ptr<po::options_description>
    get_options_description()
    {
        boost::shared_ptr<po::options_description> opts_descs(
                new po::options_description(std::string("Registrar interface configuration")));
        opts_descs->add_options()
                ("rifd.session_max",
                 po::value<unsigned>()->default_value(200),
                 "RIFD maximum number of sessions")
                ("rifd.session_timeout",
                 po::value<unsigned>()->default_value(300),
                 "RIFD session timeout")
                ("rifd.session_registrar_max",
                 po::value<unsigned>()->default_value(5),
                 "RIFD maximum number active sessions per registrar")
                ("rifd.epp_update_domain_keyset_clear",
                 po::value<bool>()->default_value(false),
                 "EPP command update domain will also clear keyset when changing NSSET");

        return opts_descs;
    }//get_options_description
    void handle( int argc, char* argv[],  FakedArgs &fa)
    {
        po::variables_map vm;
        handler_parse_args(get_options_description(), vm, argc, argv, fa);

        rifd_session_max = vm["rifd.session_max"].as<unsigned>();
        rifd_session_timeout = vm["rifd.session_timeout"].as<unsigned>();
        rifd_session_registrar_max = vm["rifd.session_registrar_max"].as<unsigned>();
        rifd_epp_update_domain_keyset_clear = vm["rifd.epp_update_domain_keyset_clear"].as<bool>();
    }//handle
};

#endif //HANDLE_RIFD_ARGS_H_
