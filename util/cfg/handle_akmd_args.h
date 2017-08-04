/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
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
 *  @handle_akmd_args.h
 *  automatic keyset management interface config
 */

#ifndef HANDLE_AKMD_ARGS_H_1DB8D08DB999477AA07B980E2C929C76
#define HANDLE_AKMD_ARGS_H_1DB8D08DB999477AA07B980E2C929C76

#include "util/cfg/faked_args.h"
#include "util/cfg/handle_args.h"

#include <boost/program_options.hpp>

#include <exception>
#include <iostream>
#include <string>
#include <vector>

/**
 * \struct HandleAkmdArgs
 * \brief registrar interface config
 */
struct HandleAkmdArgs : public HandleArgs
{
    std::string automatically_managed_keyset_prefix;
    std::string automatically_managed_keyset_registrar;
    std::string automatically_managed_keyset_tech_contact;
    std::set<std::string> automatically_managed_keyset_zones;
    bool disable_notifier;
    bool enable_request_logger;

    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> opts_descs(
                new boost::program_options::options_description(std::string("AKMD interface configuration")));
        opts_descs->add_options()
                ("akmd.automatically_managed_keyset_prefix",
                 boost::program_options::value<std::string>()->default_value("AUTO-"),
                 "AKMD prefix of automatically managed keyset");
        opts_descs->add_options()
                ("akmd.automatically_managed_keyset_registrar",
                 boost::program_options::value<std::string>()->required(),
                 "AKMD registrar of automatically managed keyset");
        opts_descs->add_options()
                ("akmd.automatically_managed_keyset_tech_contact",
                 boost::program_options::value<std::string>()->required(),
                 "AKMD technical contact of automatically managed keyset");
        opts_descs->add_options()
                ("akmd.automatically_managed_keyset_zones",
                 boost::program_options::value<std::vector<std::string> >()->multitoken()->composing()->required(),
                 "AKMD domain zones permitted for automatic keyset management");
        opts_descs->add_options()
                ("akmd.disable_notifier",
                 boost::program_options::value<bool>()->default_value(false),
                 "disable notifications");
        opts_descs->add_options()
                ("akmd.enable_request_logger",
                 boost::program_options::value<bool>()->default_value(true),
                 "enable logging to request logger component");

        return opts_descs;
    }

    void handle(int argc, char* argv[], FakedArgs& fa)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);

        automatically_managed_keyset_prefix = vm["akmd.automatically_managed_keyset_prefix"].as<std::string>();
        automatically_managed_keyset_registrar = vm["akmd.automatically_managed_keyset_registrar"].as<std::string>();
        automatically_managed_keyset_tech_contact = vm["akmd.automatically_managed_keyset_tech_contact"].as<std::string>();
        std::vector<std::string> tmp = vm["akmd.automatically_managed_keyset_zones"].as<std::vector<std::string> >();
        automatically_managed_keyset_zones = std::set<std::string>(tmp.begin(), tmp.end());
        disable_notifier = vm["akmd.disable_notifier"].as<bool>();
        enable_request_logger = vm["akmd.enable_request_logger"].as<bool>();
    }

};

#endif
