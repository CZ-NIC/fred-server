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
 *  @handle_threadgropu_args.h
 *  thread group configuration
 */

#ifndef HANDLE_THREADGROUP_ARGS_HH_99F3598FFCCC48989BFB569058FAF5C6
#define HANDLE_THREADGROUP_ARGS_HH_99F3598FFCCC48989BFB569058FAF5C6

#include <iostream>
#include <exception>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include "src/util/cfg/faked_args.hh"
#include "src/util/cfg/handle_args.hh"


/**
 * \class HandleThreadGroupArgs
 * \brief thread group options handler
 */
class HandleThreadGroupArgs : public HandleArgs
{
public:
    std::size_t number_of_threads;//number of threads in test
    std::size_t thread_group_divisor;

    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        auto thread_opts = std::make_shared<boost::program_options::options_description>("Thread group configuration");
        thread_opts->add_options()
                ("number_of_threads",
                 boost::program_options::value<unsigned int>()->default_value(50),
                 "number of threads in group")
                ("thread_group_divisor",
                 boost::program_options::value<unsigned int>()->default_value(10),
                 "designates fraction of non-synchronized threads");
        return thread_opts;
    }
    void handle(int argc, char* argv[], FakedArgs &fa)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);

        number_of_threads = vm["number_of_threads"].as<unsigned>();
        thread_group_divisor = vm["thread_group_divisor"].as<unsigned>();
    }
};

#endif
