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
 *  @handle_general_args.h
 *  config file parser, help print
 */

#ifndef HANDLE_GENERAL_ARGS_H_
#define HANDLE_GENERAL_ARGS_H_

#include <iostream>
#include <fstream>
#include <exception>
#include <string>
#include <vector>

#include <boost/program_options.hpp>
#include <boost/utility.hpp>
#include <boost/algorithm/string.hpp>

#include "faked_args.h"
#include "handle_args.h"

///end process because of cmdline processing by calling return 0; in main()
class ReturnFromMain : public std::runtime_error {
public:
    ReturnFromMain(const std::string& _what) : std::runtime_error(_what) {
  }
};

/**
 * \class HandleGeneralArgs
 * \brief common options and config file handler
 */
class HandleGeneralArgs : public HandleArgs
{
    ///options descriptions reference used to print help for all options
    typedef std::vector<boost::shared_ptr<boost::program_options::options_description> > PoDescs;

    void parse_config_file_to_faked_args(std::string fname, FakedArgs& fa )
    {//options without values are ignored
        std::ifstream cfg_file(fname.c_str());
        if (cfg_file.fail())
          throw std::runtime_error("config file '" + fname + "' not found");

        std::string line, opt_prefix;
        while (std::getline(cfg_file, line))
        {
          boost::algorithm::trim(line);// strip whitespace
          if (!line.empty() && line[0] != '#')// ignore empty line and comments
          {
            if (line[0] == '[' && line[line.size() - 1] == ']')
            {// this is option prefix
              opt_prefix = line;
              boost::algorithm::erase_first(opt_prefix, "[");
              boost::algorithm::erase_last(opt_prefix,  "]");
            }//if [opt_prefix]
            else
            {// this is normal option
              std::string::size_type sep = line.find("=");
              if (sep != std::string::npos)
              {// get name and value couple without any whitespace
                std::string name  = boost::algorithm::trim_copy(line.substr(0, sep));
                std::string value = boost::algorithm::trim_copy(line.substr(sep + 1, line.size() - 1));
                if (!value.empty())
                {// push appropriate commnad-line string
                    fa.add_argv("--" + opt_prefix + "." + name + "=" + value);
                }//if value not empty
              }// if '=' found
            }//else - option
          }//if not empty line
        }//while getline
    }//parse_config_file_to_faked_args

public:
    PoDescs po_description;

    boost::shared_ptr<boost::program_options::options_description>
        get_options_description()
    {
        boost::shared_ptr<boost::program_options::options_description> gen_opts(
                new boost::program_options::options_description(
                        std::string("General configuration")));
        gen_opts->add_options()
                ("help", "print this help message");

        std::string default_config;

#ifdef CONFIG_FILE
        std::cout << "CONFIG_FILE: "<< CONFIG_FILE << std::endl;
        default_config = std::string(CONFIG_FILE);
#else
        default_config = std::string("");
#endif
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
    }//get_options_description
    void handle( int argc, char* argv[],  FakedArgs &fa)
    {
        boost::program_options::variables_map vm;
        handler_parse_args(get_options_description(), vm, argc, argv, fa);

        //general config actions
        if (vm.count("help"))
        {
            std::cout << std::endl;
            for(PoDescs::iterator it = po_description.begin(); it != po_description.end(); ++it)
            {
                std::cout << **it << std::endl;
            }
            throw ReturnFromMain("help called");
        }

        //read config file if configured and append content to fa
        if (vm.count("config"))
        {
            std::string fname = vm["config"].as<std::string>();
            std::cout << "HandleGeneralArgs::handle config file: " << fname << std::endl;
            if(fname.length())
                parse_config_file_to_faked_args(fname, fa );
        }
    }//handle
};//class HandleGeneralArgs


/**
 * \class HandleHelpArg
 * \brief common options and config file handler
 */
class HandleHelpArg : public HandleArgs
{
    ///options descriptions reference used to print help for all options
    typedef std::vector<boost::shared_ptr<boost::program_options::options_description> > PoDescs;


public:
    PoDescs po_description;

    boost::shared_ptr<boost::program_options::options_description>
        get_options_description()
    {
        boost::shared_ptr<boost::program_options::options_description> gen_opts(
                new boost::program_options::options_description(
                        std::string("General configuration")));
        gen_opts->add_options()
                ("help", "print this help message");
        return gen_opts;
    }//get_options_description
    void handle( int argc, char* argv[],  FakedArgs &fa)
    {
        boost::program_options::variables_map vm;
        handler_parse_args(get_options_description(), vm, argc, argv, fa);

        //general config actions
        if (vm.count("help"))
        {
            std::cout << std::endl;
            for(PoDescs::iterator it = po_description.begin(); it != po_description.end(); ++it)
            {
                std::cout << **it << std::endl;
            }
            throw ReturnFromMain("help called");
        }

    }//handle
};//class HandleHelpArg



#endif //HANDLE_GENERAL_ARGS_H_
