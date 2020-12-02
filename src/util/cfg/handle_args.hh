/*
 * Copyright (C) 2010-2020  CZ.NIC, z. s. p. o.
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
 *  @handle_args.h
 *  common config options handling
 */

#ifndef HANDLE_ARGS_HH_F9FF669BA79446E4A11145D780C03F91
#define HANDLE_ARGS_HH_F9FF669BA79446E4A11145D780C03F91

#include "src/util/cfg/faked_args.hh"
#include "src/util/cfg/checked_types.hh"

#include <iostream>
#include <exception>
#include <string>
#include <deque>
#include <vector>

#include <boost/program_options.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <boost/any.hpp>
#include <memory>

/**
 * \class HandleArgs
 * \brief interface for option handlers
 */
class HandleArgs
{
public:
    virtual ~HandleArgs() { };
    virtual std::shared_ptr<boost::program_options::options_description>
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
    virtual std::shared_ptr<boost::program_options::options_description>
        get_options_description()=0;//among other exceptions may throw NO_OPTIONS exception if there are no options

    //handle returning option group index
    virtual std::size_t handle( int argc, char* argv[], FakedArgs &fa , std::size_t option_group_index ) = 0;
};

/**
 * \class VMConfigData
 * \brief internal representation of config data to be accumulated by AccumulatedConfig class
 */
class VMConfigData
{
public:
    VMConfigData(
            const std::string& _key,
            const std::string& _value,
            bool _is_value_default,
            bool _is_value_empty)
        : key_(_key),
          value_(_value),
          is_default_(_is_value_default),
          is_empty_(_is_value_empty)
    {
    }


    std::string get_key() const
    {
        return key_;
    }


    std::string get_value() const
    {
        return value_;
    }


    bool is_value_default() const
    {
        return is_default_;
    }


    bool is_value_empty() const
    {
        return is_empty_;
    }


private:
    std::string key_;
    std::string value_;
    bool is_default_;
    bool is_empty_;
};

/**
 * \class AccumulatedConfig
 * \brief config data accumulator singleton
 */
class AccumulatedConfig
{
public:
    static AccumulatedConfig& get_instance()
    {
        static AccumulatedConfig instance; // lazy init
        return instance;
    }


    /**
     * gets configuration from the variables_map and appends it into its internal data structure
     */
    void append(const boost::program_options::variables_map& vm)
    {
        for (boost::program_options::variables_map::const_iterator it = vm.begin(); it != vm.end(); it++)
        {
            std::string value;
            try
            {
                if (it->second.value().type() == typeid(std::string))
                {
                    value = boost::any_cast<std::string>(it->second.value());
                }
                else if (it->second.value().type() == typeid(unsigned))
                {
                    value = boost::lexical_cast<std::string>(boost::any_cast<unsigned>(it->second.value()));
                }
                else if (it->second.value().type() == typeid(bool))
                {
                    value = boost::any_cast<bool>(it->second.value()) ? "TRUE" : "FALSE";
                }
                else if (it->second.value().type() == typeid(boost::gregorian::date))
                {
                    value = boost::gregorian::to_iso_extended_string(
                            boost::any_cast<boost::gregorian::date>(it->second.value()));
                }
                else if (it->second.value().type() == typeid(boost::posix_time::ptime))
                {
                    value = boost::posix_time::to_iso_extended_string(
                            boost::any_cast<boost::posix_time::ptime>(it->second.value()));
                }
                else if (it->second.value().type() == typeid(Checked::string))
                {
                    value = boost::lexical_cast<std::string>(boost::any_cast<Checked::string>(it->second.value()));
                }
                else if (it->second.value().type() == typeid(Checked::string_fpnumber))
                {
                    value = boost::any_cast<Checked::string_fpnumber>(it->second.value()).to_string();
                }
                else if (it->second.value().type() == typeid(Checked::ulonglong))
                {
                    value = boost::lexical_cast<std::string>(boost::any_cast<Checked::ulonglong>(it->second.value()));
                }
                else if (it->second.value().type() == typeid(Checked::ulong))
                {
                    value = boost::lexical_cast<std::string>(boost::any_cast<Checked::ulong>(it->second.value()));
                }
                else if (it->second.value().type() == typeid(Checked::id))
                {
                    value = boost::lexical_cast<std::string>(boost::any_cast<Checked::id>(it->second.value()));
                }
                else if (it->second.value().type() == typeid(Checked::fpnumber))
                {
                    value = boost::lexical_cast<std::string>(boost::any_cast<Checked::fpnumber>(it->second.value()));
                }
                else if (it->second.value().type() == typeid(Checked::ushort))
                {
                    value = boost::lexical_cast<std::string>(boost::any_cast<Checked::ushort>(it->second.value()));
                }
                else if (it->second.value().type() == typeid(Checked::Date))
                {
                    value = boost::gregorian::to_iso_extended_string(
                            boost::any_cast<Checked::Date>(it->second.value()).date);
                }
                else if (it->second.value().type() == typeid(Checked::ptime))
                {
                    value = boost::posix_time::to_iso_extended_string(
                            boost::any_cast<Checked::ptime>(it->second.value()));
                }
                else if (it->second.value().type() == typeid(std::vector<std::string>))
                {
                    value = boost::algorithm::join(boost::any_cast<std::vector<std::string> >(it->second.value()), ", ");
                }
                else
                {
                    value = "ERROR: unknown type";
                }
            }
            catch (const std::exception& ex)
            {
                value = "ERROR: ";
                value += ex.what();
            }

            data_.push_back(
                    VMConfigData(
                            it->first,
                            value,
                            it->second.defaulted(),
                            it->second.empty()));
        }
    }

    /**
     * returns next config item in text form or empty string in case no further item available
     */
    std::string pop_front()
    {
        std::string text;

        if (!data_.empty())
        {
            text += "config dump: ";
            text += data_.front().get_key();
            text += ": ";
            text += data_.front().get_value();
            text += data_.front().is_value_default() ? " DEFAULT" : "";
            text += data_.front().is_value_empty() ? " EMPTY" : "";

            data_.pop_front();
        }

        return text;
    }

private:
    AccumulatedConfig()
    {
    }

    AccumulatedConfig(AccumulatedConfig const&); //unimplemented
    void operator=(AccumulatedConfig const&); //unimplemented

    std::deque<VMConfigData> data_;

};

///common parsing using program_options
struct handler_parse_args
{
    void operator()(
        std::shared_ptr<boost::program_options::options_description> opts_descs
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

    AccumulatedConfig::get_instance().append(vm);
}
};//handler_parse_args

//owning container of handlers
typedef std::shared_ptr<HandleArgs> HandleArgsPtr;
typedef std::vector<HandleArgsPtr > HandlerPtrVector;

typedef std::shared_ptr<HandleGrpArgs> HandleGrpArgsPtr;//group args ptr
typedef std::vector<HandleGrpArgsPtr > HandlerGrpVector;//vector of arg groups
typedef std::vector<HandlerGrpVector > HandlerPtrGrid;//grid of grouped args

#endif
