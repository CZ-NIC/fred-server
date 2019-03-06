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
 *  @command_selection_args.h
 *  command option group selection
 */

#ifndef COMMAND_SELECTION_ARGS_HH_4752E0F913B04834AFD698979D2A1959
#define COMMAND_SELECTION_ARGS_HH_4752E0F913B04834AFD698979D2A1959

#include <iostream>
#include <exception>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/program_options.hpp>
#include <memory>
#include <boost/function.hpp>

#include "src/util/types/optional.hh"
#include "src/util/cfg/check_args.hh"

/**
 * \class ReturnCode
 * \brief exception to end process returning custom return code
 */

class ReturnCode : public std::runtime_error
{
int ret_code_;
public:
    int get_return_code() const {return ret_code_;}
    ReturnCode(const std::string& _what, int _ret_code)
    : std::runtime_error(_what)
    , ret_code_(_ret_code)
    {}
};



//callback for implementation of the command
typedef boost::function<void ()> ImplCallback;

/**
 * \class command_description
 * \brief command description structure
 */

struct CommandDescription
{
    std::string command_name;
    bool have_arg;
    CommandDescription()
    : command_name("")
    , have_arg(false)
    {}//ctor
    CommandDescription(
            const std::string& _command_name
            , bool _have_arg = false)
    : command_name(_command_name)
    , have_arg(_have_arg)
    {}//ctor
    CommandDescription(CommandDescription const & rhs)
    : command_name(rhs.command_name)
    , have_arg(rhs.have_arg)
    {}//copy
    CommandDescription& operator=(CommandDescription const & rhs)
    {
        if (this != &rhs)
        {
            command_name=rhs.command_name;
            have_arg=rhs.have_arg;
        }
        return *this;
    }//assignment
};//struct command_description

typedef std::vector<CommandDescription> CommandDescriptions;

struct CommandDescriptionWithImplCallback
{
    CommandDescription command_description;
    ImplCallback impl_callback;
    CommandDescriptionWithImplCallback()
    {}//ctor
    CommandDescriptionWithImplCallback(
            const CommandDescription& _command_description
            , const ImplCallback&  _impl_callback )
    : command_description(_command_description)
    , impl_callback(_impl_callback)
    {}//ctor
    CommandDescriptionWithImplCallback(CommandDescriptionWithImplCallback const & rhs)
    : command_description(rhs.command_description)
    , impl_callback(rhs.impl_callback)
    {}//copy
    CommandDescriptionWithImplCallback& operator=(CommandDescriptionWithImplCallback const & rhs)
    {
        if (this != &rhs)
        {
            command_description=rhs.command_description;
            impl_callback=rhs.impl_callback;
        }
        return *this;
    }//assignment
};

typedef std::vector<CommandDescriptionWithImplCallback> CommandDescriptionsWithImplCallback;

/**
 * \class HandleCommandGrpArgs
 * \brief interface for command option group description
 */
class HandleCommandGrpArgs : public HandleGrpArgs
{
public:
    virtual ~HandleCommandGrpArgs() { };
    virtual CommandDescription get_command_option()=0;
};

/**
 * \class HandleCommandSelectionArgsGrp
 * \brief command path selection
 */
class HandleCommandSelectionArgsGrp : public HandleGrpArgs
{
    std::shared_ptr<boost::program_options::options_description> cfg_opts;
public:
    std::size_t selection_;
    CommandDescriptionsWithImplCallback command_descriptions_with_impl_callback_;

    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        return cfg_opts;
    }//get_options_description

    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        //init
        selection_=0;
        fa = FakedArgs(argc, argv);//don't consume args, copy all out
        FakedArgs fa_not_recognized;//to forget unrecognized args

        //command option have to be first on commandline to prevent command option ambiguity with others
        FakedArgs first_fa;
        //copy only first option with possible arguments
        //fred-admin --command arg1 arg2 ...
        int prefix_count=0;//option begin counter
        for (int i = 0; i < fa.get_argc(); ++i)
        {
            std::string arg = fa.get_argv()[i];
            if (*arg.begin() == '-') ++prefix_count;//check option begin
            if (prefix_count < 2)
                first_fa.add_argv(arg);
            else
                break;
        }

        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm
                , first_fa.get_argc(), first_fa.get_argv()
                , fa_not_recognized);

        //return option_group_index and save selection_

        for(std::size_t i = 0; i < command_descriptions_with_impl_callback_.size(); ++i)
        {
            if( vm.count(command_descriptions_with_impl_callback_.at(i).command_description.command_name))
            {
                option_group_index = selection_ = i+1;//index 0 in command_names_ is path 1 of the grid
                break;//exit loop on first match
            }
        }//for i

        if (selection_ == 0)
        {
            std::string unknown_command;
            for (int i = 1; i < first_fa.get_argc(); ++i)
            {
                unknown_command += first_fa.get_argv()[i] + std::string(" ");
            }

            throw std::runtime_error("unknown command: \"" + unknown_command + "\" try --help");
        }
        return option_group_index;
    }//handle
    HandleCommandSelectionArgsGrp(CommandDescriptionsWithImplCallback command_descriptions)//init commands (path index by position + 1)
    : cfg_opts(
        new boost::program_options::options_description(
        std::string("Command list for selection of option group, "
                "command option have to go first on command line to prevent ambiguity with other options, "
        "arg and option descriptions are within respective option group")))
    , selection_(0)
    , command_descriptions_with_impl_callback_(command_descriptions)
    {
        //program options descriptions generated from command names
        for (CommandDescriptionsWithImplCallback::iterator i = command_descriptions_with_impl_callback_.begin()
                ; i != command_descriptions_with_impl_callback_.end(); ++i)
        {
            if(i->command_description.have_arg)
                cfg_opts->add_options()(i->command_description.command_name.c_str()
                    , boost::program_options::value<std::string>(), "");
            else
                cfg_opts->add_options()(i->command_description.command_name.c_str(), "");
        }
    }//init ctor
    ImplCallback get_impl_callback()
    {
        if(selection_ > 0)
        {
            return command_descriptions_with_impl_callback_.at(selection_-1).impl_callback;
        }//if

        //else empty callback
        return ImplCallback();
    }//get_impl_callback
};//class HandleCommandSelectionArgsGrp

//owning container of handlers and implementation callbacks
typedef std::shared_ptr<HandleCommandGrpArgs> HandleCommandArgsPtr;
struct CommandHandlerParam
{
    HandleCommandArgsPtr args_ptr;
    ImplCallback impl_callback;//shall throw in case of failure
    CommandHandlerParam(HandleCommandArgsPtr _args_ptr, ImplCallback _impl_callback)
    : args_ptr(_args_ptr)
    , impl_callback(_impl_callback)
    {}
    CommandHandlerParam()
    {}//ctor
    CommandHandlerParam(CommandHandlerParam const & rhs)
    : args_ptr(rhs.args_ptr)
    , impl_callback(rhs.impl_callback)
    {}//copy
    CommandHandlerParam& operator=(CommandHandlerParam const & rhs)
    {
        if (this != &rhs)
        {
            args_ptr=rhs.args_ptr;
            impl_callback = rhs.impl_callback;
        }
        return *this;
    }//assignment
};//struct CommandHandlerParam
typedef std::vector<CommandHandlerParam> CommandHandlerPtrVector;

/**
 * \class CommandOptionGroups
 * \brief wrapper for command option groups
 */

class CommandOptionGroups
{
    HandlerGrpVector command_selection_gv;
    HandlerGrpVector command_options_gv;
    HandlerGrpVector check_config_gv;

public:
    //getters
    HandlerGrpVector get_command_selection_gv() const {return command_selection_gv;}
    HandlerGrpVector get_command_options_gv() const {return command_options_gv;}
    HandlerGrpVector get_check_config_gv() const {return check_config_gv;}

    //ctor
    CommandOptionGroups(CommandHandlerPtrVector chpv)
    {
        //init command selection group vector
        command_selection_gv.clear();

        //init command options
        HandleGrpArgsPtr wrong_path (new HandleWrongPathGrp());
        command_options_gv.clear();
        //number of command option groups + path 0
        command_options_gv.reserve(chpv.size()+1);
        //path 0 used for error detection
        command_options_gv.push_back(wrong_path);

        //init command names vector
        CommandDescriptionsWithImplCallback command_descriptions;
        command_descriptions.reserve(chpv.size());

        //init check config group vector
        check_config_gv.clear();
        check_config_gv.reserve(chpv.size()+1);
        check_config_gv.push_back(wrong_path);

        //check if all cmdline options were recognized and reset group index to 0
        HandleGrpArgsPtr check_args_ptr (new HandleCheckArgsRecognitionGrp);

        //build option group vectors for commands
        for(CommandHandlerPtrVector::iterator  i = chpv.begin()
                ; i !=  chpv.end(); ++i)
        {
            //copy command name
            command_descriptions.push_back(
                    CommandDescriptionWithImplCallback(
                            i->args_ptr->get_command_option(),i->impl_callback));

            //cast handler to other parent
            HandleGrpArgsPtr hg(
			std::dynamic_pointer_cast<HandleGrpArgs>(i->args_ptr));

            //copy handler
            command_options_gv.push_back(hg);

            //set checking handler
            check_config_gv.push_back(check_args_ptr);
        }//for i

        //set command selection group vector
        command_selection_gv.push_back(
                HandleGrpArgsPtr(//add impl_callback
                        new HandleCommandSelectionArgsGrp (command_descriptions)));

    }//CommandOptionGroups ctor
};//class CommandOptionGroups

//initialization of grid
template <typename CONTAINER_TYPE > struct list_of_gv
    : public CONTAINER_TYPE
{
    typedef typename CONTAINER_TYPE::value_type ELEMENT_TYPE;
    list_of_gv(const ELEMENT_TYPE& t)
    {
        (*this)(t);
    }
    list_of_gv& operator()(const ELEMENT_TYPE& t)
    {
        this->push_back(t);
        return *this;
    }
    list_of_gv& addCommandOptions(const CommandOptionGroups& t)
    {
        this->push_back(t.get_command_selection_gv());
        this->push_back(t.get_command_options_gv());
        this->push_back(t.get_check_config_gv());
        return *this;
    }
};

//boost assign list_of like initialization of grid
typedef list_of_gv<HandlerPtrGrid> gv_list;


#endif
