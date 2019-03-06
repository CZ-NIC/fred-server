/*
 * Copyright (C) 2010-2019  CZ.NIC, z. s. p. o.
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
 *  @handle_contactverification_args.h
 *  contact verification backend config
 */

#ifndef HANDLE_CONTACTVERIFICATION_ARGS_HH_D8C183E8CB094D11961B5590CD4A4982
#define HANDLE_CONTACTVERIFICATION_ARGS_HH_D8C183E8CB094D11961B5590CD4A4982

#include <iostream>
#include <exception>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include "src/util/cfg/faked_args.hh"
#include "src/util/cfg/handle_args.hh"

namespace po = boost::program_options;

/**
 * \class HandleContactVerificationArgs
 * \brief contact verification backend config
 */
class HandleContactVerificationArgs : public HandleArgs
{
public:
    std::string hostname;
    bool demo_mode;

    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("Contact verification server options")));

        cfg_opts->add_options()
                ("contact_verification.hostname",
                 po::value<std::string>()->default_value("demo.contactverification.cz"),
                 "server hostname")
                ("contact_verification.demo_mode",
                 po::value<bool>()->default_value(false),
                 "turn demo mode on/off");

        return cfg_opts;
    }//get_options_description
    void handle( int argc, char* argv[],  FakedArgs &fa)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);

        hostname = vm["contact_verification.hostname"].as<std::string>();
        demo_mode = vm["contact_verification.demo_mode"].as<bool>();
    }//handle
};

#endif
