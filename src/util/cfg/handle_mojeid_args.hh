/*
 * Copyright (C) 2010-2021  CZ.NIC, z. s. p. o.
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
 *  @handle_mojeid_args.h
 *  mojeid backend config
 */

#ifndef HANDLE_MOJEID_ARGS_HH_D782276C069643D0A93BE79A5E73C985
#define HANDLE_MOJEID_ARGS_HH_D782276C069643D0A93BE79A5E73C985

#include <iostream>
#include <exception>
#include <string>
#include <vector>

#include <boost/program_options.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "src/util/cfg/faked_args.hh"
#include "src/util/cfg/handle_args.hh"

namespace po = boost::program_options;

enum class AutoPin3Sending
{
    always,
    never
};

template <typename CharT, typename Traits>
std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>&, AutoPin3Sending&);

template <typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>&, AutoPin3Sending);

/**
 * \class HandleMojeIdArgs
 * \brief mojeid backend config
 */
class HandleMojeIdArgs : public HandleArgs
{
public:
    std::string registrar_handle;
    std::string hostname;
    bool demo_mode;
    bool notify_commands;
    unsigned letter_limit_count;
    unsigned letter_limit_interval;
    bool auto_sms_generation;
    bool auto_email_generation;
    AutoPin3Sending auto_pin3_sending;

    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> cfg_opts(
                new boost::program_options::options_description(
                        std::string("MojeID server options")));

        cfg_opts->add_options()
                ("mojeid.registrar_handle",
                 po::value<std::string>()->default_value("REG-MOJEID"),
                 "registrar used for mojeid operations")
                ("mojeid.hostname",
                 po::value<std::string>()->default_value("demo.mojeid.cz"),
                 "server hostname")
                ("mojeid.demo_mode",
                 po::value<bool>()->default_value(false),
                 "turn demo mode on/off")
                ("mojeid.notify_commands",
                 po::value<bool>()->default_value(false),
                 "turn command notifier on/off")
                ("mojeid.letter_limit_count",
                 po::value<unsigned>()->default_value(6),
                 "maximum number of letters sent by one contact in a letter_limit_interval days")
                ("mojeid.letter_limit_interval",
                 po::value<unsigned>()->default_value(30),
                 "interval for checking number of sent letters")
                ("mojeid.auto_sms_generation",
                 po::value< bool >()->default_value(true),
                 "turn on/off sms generation after two phase commit")
                ("mojeid.auto_email_generation",
                 po::value< bool >()->default_value(true),
                 "turn on/off email generation after two phase commit")
                ("mojeid.auto_pin3_sending",
                 po::value<AutoPin3Sending>()->default_value(AutoPin3Sending::always),
                 "control automatic PIN3 sending");

        return cfg_opts;
    }//get_options_description

    void handle( int argc, char* argv[],  FakedArgs &fa)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);

        registrar_handle = vm["mojeid.registrar_handle"].as<std::string>();
        hostname = vm["mojeid.hostname"].as<std::string>();
        demo_mode = vm["mojeid.demo_mode"].as<bool>();
        notify_commands = vm["mojeid.notify_commands"].as<bool>();
        letter_limit_count    = vm["mojeid.letter_limit_count"   ].as<unsigned>();
        letter_limit_interval = vm["mojeid.letter_limit_interval"].as<unsigned>();
        auto_sms_generation   = vm["mojeid.auto_sms_generation"].as< bool >();
        auto_email_generation = vm["mojeid.auto_email_generation"].as< bool >();
    }//handle
};

template <typename CharT, typename Traits>
std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& in, AutoPin3Sending& value)
{
    if (in.good())
    {
        std::basic_string<CharT> str;
        in >> str;
        if (str == "always")
        {
            value = AutoPin3Sending::always;
        }
        else if (str == "never")
        {
            value = AutoPin3Sending::never;
        }
        else
        {
            struct InvalidValue : std::exception
            {
                const char* what() const noexcept override
                {
                    return "unable to convert to AutoPin3Sending value";
                }
            };
            throw InvalidValue{};
        }
    }
    return in;
}

template <typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& out, AutoPin3Sending value)
{
    if (out.good())
    {
        switch (value)
        {
            case AutoPin3Sending::always:
                return out << "always";
            case AutoPin3Sending::never:
                return out << "never";
        }
        struct InvalidValue : std::exception
        {
            const char* what() const noexcept override
            {
                return "unexpected AutoPin3Sending value";
            }
        };
        throw InvalidValue{};
    }
    return out;
}

#endif
