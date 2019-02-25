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
 *  @handle_corbanameservice_args.h
 *  corba nameservice configuration
 */

#ifndef HANDLE_CORBANAMESERVICE_ARGS_HH_AA52E86E46594E788EB970E475A66227
#define HANDLE_CORBANAMESERVICE_ARGS_HH_AA52E86E46594E788EB970E475A66227

#include <iostream>
#include <exception>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include "src/util/cfg/faked_args.hh"
#include "src/util/cfg/handle_args.hh"

/**
 * \class HandleCorbaNameServiceArgs
 * \brief corba nameservice options handler
 */
class HandleCorbaNameServiceArgs : public HandleArgs
{
public:
    std::string nameservice_host ;
    unsigned nameservice_port;
    std::string nameservice_context;

    std::string get_nameservice_host_port()
        {
            return nameservice_host
                +":"+boost::lexical_cast<std::string>(nameservice_port);
        }

    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> opts_descs(
                new boost::program_options::options_description(
                        std::string("CORBA NameService configuration")));
        opts_descs->add_options()
                ("nameservice.host", boost::program_options
                            ::value<std::string>()->default_value(std::string("localhost"))
                        , "nameservice host name")
                ("nameservice.port", boost::program_options
                            ::value<unsigned int>()->default_value(2809)
                             , "nameservice port number")
                ("nameservice.context", boost::program_options
                         ::value<std::string>()->default_value(std::string("fred"))
                     , "freds context in name service");

        return opts_descs;
    }//get_options_description
    void handle( int argc, char* argv[],  FakedArgs &fa)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);

        nameservice_host = vm["nameservice.host"].as<std::string>();
        nameservice_port = vm["nameservice.port"].as<unsigned>();
        nameservice_context = vm["nameservice.context"].as<std::string>();
    }//handle
};//HandleCorbaNameServiceArgs

/**
 * \class HandleCorbaNameServiceArgsGrp
 * \brief corba nameservice options handler with option groups
 */

class HandleCorbaNameServiceArgsGrp : public HandleGrpArgs
                            , private HandleCorbaNameServiceArgs
{
public:
    std::shared_ptr<boost::program_options::options_description>
        get_options_description()
    {
        return HandleCorbaNameServiceArgs::get_options_description();
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        HandleCorbaNameServiceArgs::handle(argc, argv, fa);
        return option_group_index;
    }//handle

    const std::string& get_nameservice_host()
        {return HandleCorbaNameServiceArgs::nameservice_host;}
    unsigned get_nameservice_port()
        {return HandleCorbaNameServiceArgs::nameservice_port;}
    const std::string& get_nameservice_context()
        {return HandleCorbaNameServiceArgs::nameservice_context;}

    std::string get_nameservice_host_port()
        {
            return get_nameservice_host()
                +":"+boost::lexical_cast<std::string>(get_nameservice_port());
        }


};//class HandleCorbaNameServiceArgsGrp

#endif
