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
 *  @handle_args.h
 *  common config options handling
 */

#ifndef HANDLE_ARGS_H_
#define HANDLE_ARGS_H_

#include <iostream>
#include <exception>
#include <string>
#include <vector>

#include <boost/program_options.hpp>
#include <boost/shared_ptr.hpp>

#include "faked_args.h"

/**
 * \class HandleArgs
 * \brief interface for option handlers
 */
class HandleArgs
{
public:
    virtual ~HandleArgs() { };
    virtual boost::shared_ptr<boost::program_options::options_description>
        get_options_description()=0;
    virtual void handle( int argc, char* argv[], FakedArgs &fa ) = 0;
};

/**
 * \class NO_OPTIONS
 * \brief exception indicates that handler have no options defined
 */

struct NO_OPTIONS : std::runtime_error
{
    NO_OPTIONS()
            : std::runtime_error("have no options")
    {}
};

/**
 * \class ReturnFromMain
 * \brief exception to end process because of cmdline processing by calling return 0; in main()
 */

class ReturnFromMain : public std::runtime_error
{
public:
    ReturnFromMain(const std::string& _what) : std::runtime_error(_what){}
};

/**
 * \class HandleGrpArgs
 * \brief interface for option group handlers
 */
class HandleGrpArgs
{
public:
    virtual ~HandleGrpArgs() { };
    virtual boost::shared_ptr<boost::program_options::options_description>
        get_options_description()=0;//among other exceptions may throw NO_OPTIONS exception if there are no options

    //handle returning option group index
    virtual std::size_t handle( int argc, char* argv[], FakedArgs &fa , std::size_t option_group_index ) = 0;
};



///common parsing using program_options
static void handler_parse_args(
        boost::shared_ptr<boost::program_options::options_description> opts_descs
        , boost::program_options::variables_map& vm
        , int argc, char* argv[],  FakedArgs &fa)
{
    if(argc < 1)
    {
        throw std::runtime_error(
                "handler_parse_args: input cmdline is empty (argc < 1)"
                " , there should be at least program name");
    }
    using namespace boost::program_options::command_line_style;

    boost::program_options::parsed_options parsed
        = boost::program_options::command_line_parser(argc,argv)
            .options(*opts_descs).allow_unregistered().style(
                    allow_short
                    | short_allow_adjacent
                    | short_allow_next
                    | allow_long
                    | long_allow_adjacent
                    | long_allow_next
                    | allow_sticky
                    | allow_dash_for_short).run();
    boost::program_options::store(parsed, vm);

    typedef std::vector<std::string> string_vector_t;
    string_vector_t to_pass_further;//args

    to_pass_further
        = boost::program_options::collect_unrecognized(parsed.options
                , boost::program_options::include_positional);
    boost::program_options::notify(vm);

    //faked args for unittest framework returned by reference in params
    fa.clear();//to be sure that fa is empty
    fa.prealocate_for_argc(to_pass_further.size() + 1);//new number of args + first program name
    fa.add_argv(argv[0]);//program name copy
    for(string_vector_t::const_iterator i = to_pass_further.begin()
            ; i != to_pass_further.end(); ++i)
    {//copying a new arg vector
        fa.add_argv(*i);//string
    }//for i

}//handler_parse_args

//owning container of handlers
typedef boost::shared_ptr<HandleArgs> HandleArgsPtr;
typedef std::vector<HandleArgsPtr > HandlerPtrVector;

typedef boost::shared_ptr<HandleGrpArgs> HandleGrpArgsPtr;//group args ptr
typedef std::vector<HandleGrpArgsPtr > HandlerGrpVector;//vector of arg groups
typedef std::vector<HandlerGrpVector > HandlerPtrGrid;//grid of grouped args

#endif //HANDLE_ARGS_H_
