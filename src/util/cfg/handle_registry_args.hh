/*
 * Copyright (C) 2010-2022  CZ.NIC, z. s. p. o.
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
 *  @handle_registry_args.h
 *  basic server config
 */

#ifndef HANDLE_REGISTRY_ARGS_HH_E333CDA4CFDA4E62B878A4F46236938B//date "+%s.%N"|md5sum|tr "[a-f]" "[A-F]"
#define HANDLE_REGISTRY_ARGS_HH_E333CDA4CFDA4E62B878A4F46236938B

#include "src/util/cfg/faked_args.hh"
#include "src/util/cfg/handle_args.hh"

#include "src/backend/epp/nsset/impl/limits.hh"

#include <chrono>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include <boost/program_options.hpp>
#include <boost/numeric/conversion/cast.hpp>

/**
 * \class HandleRegistryArgs
 * \brief server config
 */
class HandleRegistryArgs : public HandleArgs
{
public:
    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> opts_descs(
                new boost::program_options::options_description(std::string("Registry configuration")));
        opts_descs->add_options()
                ("registry.restricted_handles",
                 boost::program_options::value<bool>()->default_value(false),
                 "use restricted hadles")
                ("registry.disable_epp_notifier",
                 boost::program_options::value<bool>()->default_value(false),
                 "disable epp notifier")
                ("registry.lock_epp_commands",
                 boost::program_options::value<bool>()->default_value(true),
                 "lock epp commands")
                ("registry.nsset_level",
                 boost::program_options::value<unsigned int>()->default_value(3),
                 "default nsset level")
                ("registry.nsset_min_hosts",
                 boost::program_options::value<unsigned int>()->default_value(2),
                 "minimal number of hosts in nsset")
                ("registry.nsset_max_hosts",
                 boost::program_options::value<unsigned int>()->default_value(10),
                 "maximal number of hosts in nsset")
                ("registry.docgen_path",
                 boost::program_options::value<std::string>()->required(),
                 "")
                ("registry.docgen_template_path",
                 boost::program_options::value<std::string>()->required(),
                 "")
                ("registry.docgen_domain_count_limit",
                 boost::program_options::value<unsigned int>()->default_value(100),
                 "")
                ("registry.fileclient_path",
                 boost::program_options::value<std::string>()->required(),
                 "")
                ("registry.disable_epp_notifier_cltrid_prefix",
                 boost::program_options::value<std::string>()->default_value("do_not_notify"),
                 "disable epp command notification with specific cltrid prefix"
                 " (only for system registrar)")
                ("registry.registry_timezone",
                 boost::program_options::value<std::string>()->default_value("UTC"),
                 "registry timezone in Olson format, "
                 "must be the same as local system timezone, "
                 "value must be from PostgreSQL pg_timezone_names.name")
                ("registry.system_registrar",
                 boost::program_options::value<std::string>()->required(),
                 "handle of registrar with system privileges for restricted actions")
                ("registry.authinfo_ttl",
                 boost::program_options::value<unsigned long long>()->default_value(14 * 24 * 3600),
                 "TTL of object authinfos");

        return opts_descs;
    }
    void handle(int _argc, char* _argv[], FakedArgs &_fa) override
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, _argc, _argv, _fa);

        restricted_handles = vm["registry.restricted_handles"].as<bool>();
        disable_epp_notifier = vm["registry.disable_epp_notifier"].as<bool>();
        lock_epp_commands = vm["registry.lock_epp_commands"].as<bool>();
        nsset_level = vm["registry.nsset_level"].as<unsigned int>();

        if (nsset_level > boost::numeric_cast<unsigned int>(Epp::Nsset::max_nsset_tech_check_level))
        {
            throw std::runtime_error("configured default nsset_level out of range");
        }

        nsset_min_hosts = vm["registry.nsset_min_hosts"].as<unsigned int>();
        nsset_max_hosts = vm["registry.nsset_max_hosts"].as<unsigned int>();
        docgen_domain_count_limit = vm["registry.docgen_domain_count_limit"].as<unsigned int>();
        docgen_path = vm["registry.docgen_path"].as<std::string>();
        docgen_template_path = vm["registry.docgen_template_path"].as<std::string>();
        fileclient_path = vm["registry.fileclient_path"].as<std::string>();
        disable_epp_notifier_cltrid_prefix = vm["registry.disable_epp_notifier_cltrid_prefix"].as<std::string>();
        registry_timezone = vm["registry.registry_timezone"].as<std::string>();
        system_registrar = vm["registry.system_registrar"].as<std::string>();
        authinfo_ttl = vm["registry.authinfo_ttl"].as<std::chrono::seconds>();
    }
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
    std::string registry_timezone;
    std::string system_registrar;
    std::chrono::seconds authinfo_ttl;
};

/**
 * \class HandleRegistryArgsGrp
 * \brief server config with option groups
 */

class HandleRegistryArgsGrp : public HandleGrpArgs,
                              private HandleRegistryArgs
{
public:
    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        return HandleRegistryArgs::get_options_description();
    }
    std::size_t handle(int _argc, char* _argv[], FakedArgs& _fa, std::size_t _option_group_index)
    {
        HandleRegistryArgs::handle(_argc, _argv, _fa);
        return _option_group_index;
    }
    bool get_restricted_handles()const
    {
        return HandleRegistryArgs::restricted_handles;
    }
    bool get_disable_epp_notifier()const
    {
        return HandleRegistryArgs::disable_epp_notifier;
    }
    bool get_lock_epp_commands()const
    {
        return HandleRegistryArgs::lock_epp_commands;
    }
    unsigned int get_nsset_level()const
    {
        return HandleRegistryArgs::nsset_level;
    }
    unsigned int get_nsset_min_hosts()const
    {
        return HandleRegistryArgs::nsset_min_hosts;
    }
    unsigned int get_nsset_max_hosts()const
    {
        return HandleRegistryArgs::nsset_max_hosts;
    }
    const std::string& get_docgen_path()const
    {
        return HandleRegistryArgs::docgen_path;
    }
    const std::string& get_docgen_template_path()const
    {
        return HandleRegistryArgs::docgen_template_path;
    }
    unsigned int get_docgen_domain_count_limit()const
    {
        return HandleRegistryArgs::docgen_domain_count_limit;
    }
    const std::string& get_fileclient_path()const
    {
        return HandleRegistryArgs::fileclient_path;
    }
    const std::string& get_disable_epp_notifier_cltrid_prefix()const
    {
        return HandleRegistryArgs::disable_epp_notifier_cltrid_prefix;
    }
    const std::string& get_registry_timezone()const
    {
        return HandleRegistryArgs::registry_timezone;
    }
    const std::string& get_system_registrar()const
    {
        return HandleRegistryArgs::system_registrar;
    }
    auto get_authinfo_ttl() const
    {
        return HandleRegistryArgs::authinfo_ttl;
    }
};

#endif//HANDLE_REGISTRY_ARGS_HH_E333CDA4CFDA4E62B878A4F46236938B
