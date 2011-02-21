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
 *  @handle_logging_args.h
 *  logging configuration
 */

#ifndef HANDLE_LOGGING_ARGS_H_
#define HANDLE_LOGGING_ARGS_H_

#include <iostream>
#include <exception>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include "faked_args.h"
#include "handle_args.h"


/**
 * \class HandleLoggingArgs
 * \brief logging options handler
 */
class HandleLoggingArgs : public HandleArgs
{
public:

    unsigned log_type;
    unsigned log_level;
    std::string log_file;
    unsigned log_syslog_facility;

    boost::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        boost::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("Logging configuration")));
        cfg_opts->add_options()
                ("log.type", boost::program_options
                            ::value<unsigned int>()->default_value(1)
                             , "log type: 0- console, 1- file, 2-syslog")
                ("log.level", boost::program_options
                         ::value<unsigned int>()->default_value(8)
                          , "log severity level")
                ("log.file", boost::program_options
                     ::value<std::string>()->default_value("fred.log")
                          , "log file name for log.type = 1")
                ("log.syslog_facility", boost::program_options
                   ::value<unsigned int>()->default_value(1)
                        , "syslog facility for log.type = 2")
                ;
        return cfg_opts;
    }//get_options_description
    void handle( int argc, char* argv[],  FakedArgs &fa)
    {
        boost::program_options::variables_map vm;
        handler_parse_args(get_options_description(), vm, argc, argv, fa);

        log_type = (vm.count("log.type") == 0)
                ? 0 : vm["log.type"].as<unsigned>();

        log_level = (vm.count("log.level") == 0)
                ? 0 : vm["log.level"].as<unsigned>();

        log_file = (vm.count("log.file") == 0
                        ? std::string("") : vm["log.file"].as<std::string>());

        log_syslog_facility = (vm.count("log.syslog_facility") == 0)
                ? 0 : vm["log.syslog_facility"].as<unsigned>();
    }//handle
};

/**
 * \class HandleLoggingArgsGrp
 * \brief logging options handler with option groups
 */

class HandleLoggingArgsGrp : public HandleGrpArgs
                            , private HandleLoggingArgs
{
public:

    boost::shared_ptr<boost::program_options::options_description>
        get_options_description()
    {
        return HandleLoggingArgs::get_options_description();
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        HandleLoggingArgs::handle(argc, argv, fa);
        return option_group_index;
    }//handle

    unsigned get_log_type()
        {return HandleLoggingArgs::log_type;}
    unsigned get_log_level()
        {return HandleLoggingArgs::log_level;}
    unsigned get_log_syslog_facility()
        {return HandleLoggingArgs::log_syslog_facility;}
    const std::string& get_log_file()
        {return HandleLoggingArgs::log_file;}
};//class HandleLoggingArgsGrp

#endif //HANDLE_LOGGING_ARGS_H_
