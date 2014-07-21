/*
 * Copyright (C) 2014  CZ.NIC, z.s.p.o.
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
 *  @file
 *  optys mail upload configuration
 */

#ifndef HANDLE_OPTYS_MAIL_ARGS_H_b41ac6cd859a4393b867be7898b2b474
#define HANDLE_OPTYS_MAIL_ARGS_H_b41ac6cd859a4393b867be7898b2b474

#include <iostream>
#include <exception>
#include <string>
#include <vector>
#include <map>

#include <boost/program_options.hpp>

#include "cfg/faked_args.h"
#include "cfg/handle_args.h"

/**
 * HandleOptysMailArgs
 * Optys client config options handler
 */

class HandleOptysMailArgs : public HandleArgs
{
    std::map<std::string, std::string> optys_config;

public:
    //selected optys config options values
   
    const std::string CONFIG_PREFIX;

    HandleOptysMailArgs()
    : CONFIG_PREFIX("optys_upload.")
    {}

    const std::map<std::string, std::string> get_map() { return optys_config; };

    boost::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        boost::shared_ptr<boost::program_options::options_description> opts_descs(
                new boost::program_options::options_description(
                        std::string("Optys client upload configuration")
                        , 140 //width of help print in cols
                        ));
        opts_descs->add_options()
                ((CONFIG_PREFIX+"host,h").c_str(), boost::program_options
                            ::value<std::string>()
                        , "optys mail upload host")
                ((CONFIG_PREFIX+"user,u").c_str(), boost::program_options
                            ::value<std::string>()
                        , "optys mail upload account login name")
                ((CONFIG_PREFIX+"password,p").c_str(), boost::program_options
                            ::value<std::string>()
                        , "optys mail upload account password")
                ((CONFIG_PREFIX+"port,r").c_str(), boost::program_options
                            ::value<std::string>()
                        , "optys mail upload port")
                ((CONFIG_PREFIX+"zip_tmp_dir,t").c_str(), boost::program_options
                            ::value<std::string>()
                        , "optys zip temp dir path")
                ((CONFIG_PREFIX+"cleanup_zip_tmp_dir,c").c_str(), boost::program_options
                            ::value<std::string>()
                        , "remove *.zip files in optys zip temp dir before new zip file is created (true/false)")
                 ;
        return opts_descs;
    }//get_options_description

    void handle( int argc, char* argv[],  FakedArgs &fa)
    {
        boost::program_options::variables_map vm;
        handler_parse_args(get_options_description(), vm, argc, argv, fa);

        boost::program_options::variables_map::iterator it;
        for(it = vm.begin(); it != vm.end(); it++) {
                std::string key(it->first);
                if (key.compare(0, CONFIG_PREFIX.length(), CONFIG_PREFIX)==0) {
                        key = key.substr(CONFIG_PREFIX.length());
                }

                //std::cout << key << " = " << it->second.as<std::string>() << std::endl;

                optys_config [key] = (it->second).as<std::string>();
        }

    }//handle
};//class HandleOptysMailArgs

#endif //OPTYS_MAIL
