/*
 *  Copyright (C) 2008, 2009  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef READ_CONFIG_FILE_H_9777d979c2ee4b83a0447019af4a7267
#define READ_CONFIG_FILE_H_9777d979c2ee4b83a0447019af4a7267

#include <map>
#include <string>

#include <boost/assign/list_of.hpp>

#include "util/cfg/faked_args.h"
#include "util/cfg/handle_args.h"
#include "util/log/context.h"

#include "cfg/config_handler_decl.h"
#include "cfg/handle_logging_args.h"
#include "util/cfg/handle_general_args.h"
#include "util/log/logger.h"

namespace Admin {

/**
 * read extra config file into key-value config map
 */
template <class HANDLE_ARGS>
std::map<std::string, std::string> readConfigFile(
        const std::string& conf_file)
{
    boost::shared_ptr<HANDLE_ARGS> handle_args_ptr(new HANDLE_ARGS);

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

    //config dump
    if (CfgArgGroups::instance()->get_handler_ptr_by_type<HandleLoggingArgsGrp>()->get_log_config_dump())
    {
        for (std::string config_item =
                        AccumulatedConfig::get_instance().pop_front();
                !config_item.empty();
                config_item =
                        AccumulatedConfig::get_instance().pop_front())
        {
            Logging::Manager::instance_ref().get(PACKAGE).debug(
                    config_item);
        }
    }

    return set_cfg;
}

} // namespace Admin

#endif // READ_CONFIG_FILE_H_9777d979c2ee4b83a0447019af4a7267
