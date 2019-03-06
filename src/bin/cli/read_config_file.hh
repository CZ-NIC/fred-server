/*
 * Copyright (C) 2014-2019  CZ.NIC, z. s. p. o.
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
#ifndef READ_CONFIG_FILE_HH_43328C6DA5694C0EA6620A4529F3DFD2
#define READ_CONFIG_FILE_HH_43328C6DA5694C0EA6620A4529F3DFD2

#include <map>
#include <string>

#include <boost/assign/list_of.hpp>

#include "src/util/cfg/faked_args.hh"
#include "src/util/cfg/handle_args.hh"
#include "util/log/context.hh"

#include "src/util/cfg/config_handler_decl.hh"
#include "src/util/cfg/handle_logging_args.hh"
#include "src/util/cfg/handle_general_args.hh"
#include "util/log/logger.hh"

namespace Admin {

/**
 * read extra config file into key-value config map
 */
template <class HANDLE_ARGS>
std::map<std::string, std::string> read_config_file(
        const std::string& conf_file,
        bool dump_config_into_debug_log)
{
        std::shared_ptr<HANDLE_ARGS> handle_args_ptr(new HANDLE_ARGS);

    HandlerPtrVector hpv = boost::assign::list_of(
            HandleArgsPtr(new HandleConfigFileArgs(conf_file)))(
            HandleArgsPtr(handle_args_ptr));

    // it always needs some argv vector, argc cannot be 0
    FakedArgs fa;
    fa.add_argv(std::string(""));

    //handle
    for (HandlerPtrVector::const_iterator i = hpv.begin(); i != hpv.end();
            ++i)
    {
        FakedArgs fa_out;
        (*i)->handle(fa.get_argc(), fa.get_argv(), fa_out);
        fa = fa_out; //last output to next input
    }

    std::map<std::string, std::string> set_cfg = handle_args_ptr->get_map();

    // config dump
    if (dump_config_into_debug_log)
    {
        for (std::string config_item = AccumulatedConfig::get_instance().pop_front();
             !config_item.empty(); config_item = AccumulatedConfig::get_instance().pop_front())
        {
            Logging::Manager::instance_ref().debug(config_item);
        }
    }

    return set_cfg;
}

} // namespace Admin

#endif
