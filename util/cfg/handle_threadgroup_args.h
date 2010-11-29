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
 *  @handle_threadgropu_args.h
 *  thread group configuration
 */

#ifndef HANDLE_THREADGROUP_ARGS_H_
#define HANDLE_THREADGROUP_ARGS_H_

#include <iostream>
#include <exception>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include "faked_args.h"
#include "handle_args.h"


/**
 * \class HandleThreadGroupArgs
 * \brief thread group options handler
 */
class HandleThreadGroupArgs : public HandleArgs
{
public:
    std::size_t thread_number ;//number of threads in test
    std::size_t thread_group_divisor;//

    boost::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        boost::shared_ptr<boost::program_options::options_description> thread_opts(
                new boost::program_options::options_description(
                        std::string("Thread group configuration")));
        thread_opts->add_options()
                ("thread_number", boost::program_options
                            ::value<unsigned int>()->default_value(50)
                             , "number of threads in group")
                ("thread_group_divisor", boost::program_options
                            ::value<unsigned int>()->default_value(10)
                             , "designates fraction of non-synchronized threads");
        return thread_opts;
    }//get_options_description
    void handle( int argc, char* argv[],  FakedArgs &fa)
    {
        boost::program_options::variables_map vm;
        handler_parse_args(get_options_description(), vm, argc, argv, fa);

        thread_number = (vm.count("thread_number") == 0
                ? 50 : vm["thread_number"].as<unsigned>());
        /*
        std::cout << "thread_number: " << thread_number
                << " vm[\"thread_number\"].as<unsigned>(): "
                << vm["thread_number"].as<unsigned>() << std::endl;
        */

        thread_group_divisor = (vm.count("thread_group_divisor") == 0
                ? 10 : vm["thread_group_divisor"].as<unsigned>());
        /*
        std::cout << "thread_group_divisor: " << thread_group_divisor<< ""
                << std::endl;
        */
    }//handle
};

#endif //HANDLE_THREADGROUP_ARGS_H_
