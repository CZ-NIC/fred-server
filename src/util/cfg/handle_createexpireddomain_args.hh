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
#ifndef HANDLE_CREATEEXPIREDDOMAIN_ARGS_HH_E5113DC0BFF84631ABB8E85528E38249
#define HANDLE_CREATEEXPIREDDOMAIN_ARGS_HH_E5113DC0BFF84631ABB8E85528E38249

#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include "src/util/cfg/faked_args.hh"
#include "src/util/cfg/handle_args.hh"

namespace po = boost::program_options;

/**
 * \class HandleCreateExpiredDomainArgs
 * \brief create expired domain backend config
 */
class HandleCreateExpiredDomainArgs : public HandleArgs
{
public:
    std::string registrar_handle;

    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("Create expired domain configuration")));

        cfg_opts->add_options()
                ("create_expired_domain.registrar_handle",
                 po::value<std::string>()->default_value("REG-FRED_C"),
                 "system registrar handle");

        return cfg_opts;
    }//get_options_descriptioni

    void handle( int argc, char* argv[],  FakedArgs &fa)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);

        registrar_handle = vm["create_expired_domain.registrar_handle"].as<std::string>();
    }//handle
};

/**
 * \class HandleCreateExpiredDomainArgsGrp
 * \brief corba nameservice options handler with option groups
 */

class HandleCreateExpiredDomainArgsGrp : public HandleGrpArgs
                            , private HandleCreateExpiredDomainArgs
{
public:
    std::shared_ptr<boost::program_options::options_description>
        get_options_description()
    {
        return HandleCreateExpiredDomainArgs::get_options_description();
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        HandleCreateExpiredDomainArgs::handle(argc, argv, fa);
        return option_group_index;
    }//handle

    const std::string& get_registrar_handle()
        {return HandleCreateExpiredDomainArgs::registrar_handle;}

};//class HandleCreateExpiredDomainArgsGrp
#endif
