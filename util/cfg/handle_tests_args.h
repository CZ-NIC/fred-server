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
 *  @file
 *  config file parser for tests
 */

#ifndef HANDLE_TESTS_ARGS_H_13415834121
#define HANDLE_TESTS_ARGS_H_13415834121

#include <string>
#include <vector>

#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

#include "util/cfg/handle_general_args.h"
#include "faked_args.h"
#include "handle_args.h"

/**
 * \class HandleTestsArgs
 * \brief common options and config file handler for tests
 */
class HandleTestsArgs : public HandleArgs
{
    std::string default_config;
    ///options descriptions reference used to print help for all options
    typedef std::vector<boost::shared_ptr<boost::program_options::options_description> > PoDescs;

public:
    PoDescs po_description;

    HandleTestsArgs(const std::string& def_cfg)
        : default_config(def_cfg) {};

    boost::shared_ptr<boost::program_options::options_description>
        get_options_description()
    {
        boost::shared_ptr<boost::program_options::options_description> gen_opts(
                new boost::program_options::options_description(
                        std::string("General configuration")));
        gen_opts->add_options()
                ("help", "print custom help message and boost::test help message for BOOST_VERSION > 103900");

        if(default_config.length() != 0)
        {
            gen_opts->add_options()
                    ("config,C", boost::program_options
                            ::value<std::string>()->default_value(default_config)
                    , "path to configuration file");
        }
        else
        {
            gen_opts->add_options()
                    ("config,C", boost::program_options
                            ::value<std::string>(), "path to configuration file");
        }

        return gen_opts;
    }
    void handle( int argc, char* argv[],  FakedArgs &fa)
    {
        boost::program_options::variables_map vm;
        handler_parse_args(get_options_description(), vm, argc, argv, fa);

        if (vm.count("help"))
        {
            std::cout << std::endl;
            for(PoDescs::iterator it = po_description.begin(); it != po_description.end(); ++it)
            {
                std::cout << **it << std::endl;
            }

            fa.add_argv(std::string("--help")); //pass consumed help option to UTF
            std::cout << "\n\nBoost Test options under Usage:\n" << std::endl;
        }

        //read config file if configured and append content to fa
        if (vm.count("config"))
        {
            std::string fname = vm["config"].as<std::string>();
            std::cout << "HandleTestsArgs::handle config file: " << fname << std::endl;
            if(fname.length())
                parse_config_file_to_faked_args(fname, fa );
        }
    }
};

#endif
