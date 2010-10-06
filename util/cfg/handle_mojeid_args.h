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
 *  @handle_mojeid_args.h
 *  mojeid backend config
 */

#ifndef HANDLE_MOJEID_ARGS_H_
#define HANDLE_MOJEID_ARGS_H_

#include <iostream>
#include <exception>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include "faked_args.h"
#include "handle_args.h"

namespace po = boost::program_options;

/**
 * \class HandleMojeIDArgs
 * \brief mojeid backend config
 */
class HandleMojeIDArgs : public HandleArgs
{
public:
    std::string registrar_handle;
    std::string svtrid_prefix;
    std::string redirect_url_hostname;
    std::string redirect_url;

    boost::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        boost::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("MojeID server options")));

        cfg_opts->add_options()
                ("mojeid.registrar_handle",
                 po::value<std::string>()->default_value("REG-MOJEID"),
                 "registrar used for mojeid operations")
                ("mojeid.svtrid_prefix",
                 po::value<std::string>()->default_value("MojeID"),
                 "prefix for action servertrid (it will looks like svtrid_prefix-0000000001)")
                ("mojeid.redirect_url_hostname",
                 po::value<std::string>()->default_value("demo.mojeid.cz"),
                 "hostname for url redirect")
                ("mojeid.redirect_url",
                 po::value<std::string>()->default_value("https://%1%/identification/%2%/"),
                 "email password redirect url");

        return cfg_opts;
    }//get_options_description
    void handle( int argc, char* argv[],  FakedArgs &fa)
    {
        boost::program_options::variables_map vm;
        handler_parse_args(get_options_description(), vm, argc, argv, fa);

        registrar_handle = vm["mojeid.registrar_handle"].as<std::string>();
        svtrid_prefix = vm["mojeid.svtrid_prefix"].as<std::string>();
        redirect_url_hostname = vm["mojeid.redirect_url_hostname"].as<std::string>();
        redirect_url = vm["mojeid.redirect_url"].as<std::string>();
    }//handle
};

#endif //HANDLE_MOJEID_ARGS_H_
