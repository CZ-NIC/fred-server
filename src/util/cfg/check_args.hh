/*
 * Copyright (C) 2011-2019  CZ.NIC, z. s. p. o.
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
 *  @check_args.h
 *  check handlers
 */

#ifndef CHECK_ARGS_HH_C08EF7DC882149AF8D4B0395D718C149
#define CHECK_ARGS_HH_C08EF7DC882149AF8D4B0395D718C149

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/program_options.hpp>
#include <memory>

#include "src/util/cfg/faked_args.hh"
#include "src/util/cfg/handle_args.hh"


/**
 * \class HandleWrongPathGrp
 * \brief wrong path through the grid check handler
 */
class HandleWrongPathGrp : public HandleGrpArgs
{
public:

    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        throw NO_OPTIONS();
    }//get_options_description
    std::size_t handle( int, char*[],  FakedArgs&, std::size_t option_group_index)
    {
        std::cerr << "HandleWrongPathGrp - configuration processing implementation error option_group_index: "
                <<option_group_index << std::endl;
        throw std::runtime_error(
                "HandleWrongPathGrp - configuration processing implementation error");
        return 0;
    }//handle
};//class HandleWrongPathGrp


/**
 * \class HandleCheckArgsRecognitionGrp
 * \brief options recognition check and path reset handler
 */
class HandleCheckArgsRecognitionGrp : public HandleGrpArgs
{
public:

    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        throw NO_OPTIONS();
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        //remove passed config file option from check
        FakedArgs tmp_fa;
        boost::program_options::variables_map vm;
        std::shared_ptr<boost::program_options::options_description> gen_opts(
                   new boost::program_options::options_description(
                           std::string("Passed options configuration")));
               gen_opts->add_options()
                       ("config,C", boost::program_options
                               ::value<std::string>(), "path to configuration file");
        handler_parse_args()(gen_opts, vm, argc, argv, tmp_fa);

        if (tmp_fa.get_argc() > 1)
        {
            std::string err = std::string("found unknown configuration with option_group_index ")
            +boost::lexical_cast<std::string>(option_group_index) + " : ";
            for(int i =1 ; i < tmp_fa.get_argc(); ++i)
            {
                err+=" ";
                err+=tmp_fa.get_argv()[i];
            }
            throw std::runtime_error(err);
        }
        else
            fa = FakedArgs(argc, argv);//don't consume options

        return 0;//reset option_group_index
    }//handle
};//class HandleCheckArgsRecognitionGrp

#endif
