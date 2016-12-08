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
 *  @handle_registry_args.h
 *  basic server config
 */

#ifndef HANDLE_REGISTRY_ARGS_H_C9FB23EA2AC143618F9E646C106DE78B
#define HANDLE_REGISTRY_ARGS_H_C9FB23EA2AC143618F9E646C106DE78B

#include <iostream>
#include <exception>
#include <string>
#include <vector>

#include "src/epp/nsset/impl/nsset_constants.h"

#include <boost/program_options.hpp>
#include <boost/numeric/conversion/cast.hpp>

#include "faked_args.h"
#include "handle_args.h"

namespace po = boost::program_options;

/**
 * \class HandleRegistryArgs
 * \brief server config
 */
class HandleRegistryArgs : public HandleArgs
{
public:
    bool restricted_handles;
    bool disable_epp_notifier;
    bool lock_epp_commands;
    unsigned int nsset_level;
    unsigned int nsset_min_hosts;
    unsigned int nsset_max_hosts;
    std::string docgen_path;
    std::string docgen_template_path;
    unsigned int docgen_domain_count_limit;
    std::string fileclient_path;
    std::string disable_epp_notifier_cltrid_prefix;

    boost::shared_ptr<po::options_description>
    get_options_description()
    {
        boost::shared_ptr<po::options_description> opts_descs(
                new po::options_description(std::string("Registry configuration")));
        opts_descs->add_options()
                ("registry.restricted_handles",
                 po::value<bool>()->default_value(false),
                 "use restricted hadles")
                ("registry.disable_epp_notifier",
                 po::value<bool>()->default_value(false),
                 "disable epp notifier")
                ("registry.lock_epp_commands",
                 po::value<bool>()->default_value(true),
                 "lock epp commands")
                ("registry.nsset_level",
                 po::value<unsigned int>()->default_value(3),
                 "default nsset level")
                ("registry.nsset_min_hosts",
                 po::value<unsigned int>()->default_value(2),
                 "minimal number of hosts in nsset")
                ("registry.nsset_max_hosts",
                 po::value<unsigned int>()->default_value(10),
                 "maximal number of hosts in nsset")
                ("registry.docgen_path",
                 po::value<std::string>(),
                 "")
                ("registry.docgen_template_path",
                 po::value<std::string>(),
                 "")
                ("registry.docgen_domain_count_limit",
                 po::value<unsigned int>()->default_value(100),
                 "")
                ("registry.fileclient_path",
                 po::value<std::string>(),
                 "")
                ("registry.disable_epp_notifier_cltrid_prefix",
                 po::value<std::string>()->default_value("do_not_notify"),
                 "disable epp command notification with specific cltrid prefix"
                 " (only for system registrar)");

        return opts_descs;
    }//get_options_description
    void handle( int argc, char* argv[],  FakedArgs &fa)
    {
        po::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);

        restricted_handles = vm["registry.restricted_handles"].as<bool>();
        disable_epp_notifier = vm["registry.disable_epp_notifier"].as<bool>();
        lock_epp_commands = vm["registry.lock_epp_commands"].as<bool>();
        nsset_level = vm["registry.nsset_level"].as<unsigned int>();

        if(nsset_level > boost::numeric_cast<unsigned int>(Epp::Nsset::max_nsset_tech_check_level))
        {
            throw std::runtime_error("configured default nsset_level out of range");
        }

        nsset_min_hosts = vm["registry.nsset_min_hosts"].as<unsigned int>();
        nsset_max_hosts = vm["registry.nsset_max_hosts"].as<unsigned int>();
        docgen_domain_count_limit = vm["registry.docgen_domain_count_limit"].as<unsigned int>();
        docgen_path = (vm.count("registry.docgen_path") == 0
                ? std::string() : vm["registry.docgen_path"].as<std::string>());
        docgen_template_path = (vm.count("registry.docgen_template_path") == 0
                ? std::string() : vm["registry.docgen_template_path"].as<std::string>());
        fileclient_path = (vm.count("registry.fileclient_path") == 0
                ? std::string() : vm["registry.fileclient_path"].as<std::string>());
        disable_epp_notifier_cltrid_prefix = vm["registry.disable_epp_notifier_cltrid_prefix"].as<std::string>();
    }//handle
};//HandleRegistryArgs

/**
 * \class HandleRegistryArgsGrp
 * \brief server config with option groups
 */

class HandleRegistryArgsGrp : public HandleGrpArgs
                            , private HandleRegistryArgs
{
public:

    boost::shared_ptr<boost::program_options::options_description>
        get_options_description()
    {
        return HandleRegistryArgs::get_options_description();
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        HandleRegistryArgs::handle(argc, argv, fa);
        return option_group_index;
    }//handle

    bool get_restricted_handles()
        {return HandleRegistryArgs::restricted_handles;}
    bool get_disable_epp_notifier()
        {return HandleRegistryArgs::disable_epp_notifier;}
    bool get_lock_epp_commands()
        {return HandleRegistryArgs::lock_epp_commands;}
    unsigned int get_nsset_level()
        {return HandleRegistryArgs::nsset_level;}
    unsigned int get_nsset_min_hosts()
        {return HandleRegistryArgs::nsset_min_hosts;}
    unsigned int get_nsset_max_hosts()
        {return HandleRegistryArgs::nsset_max_hosts;}
    const std::string& get_docgen_path()
        {return HandleRegistryArgs::docgen_path;}
    const std::string& get_docgen_template_path()
        {return HandleRegistryArgs::docgen_template_path;}
    unsigned int get_docgen_domain_count_limit()
        {return HandleRegistryArgs::docgen_domain_count_limit;}
    const std::string& get_fileclient_path()
        {return HandleRegistryArgs::fileclient_path;}
    const std::string& get_disable_epp_notifier_cltrid_prefix()
        {return HandleRegistryArgs::disable_epp_notifier_cltrid_prefix;}
};//class HandleRegistryArgsGrp

#endif
