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
 *  @file setup_server.hh
 *  Implementation of server setup.
 */

#ifndef SETUP_SERVER_HH_B3A8DA8BD3A64FAEBB957E50B12B46BE
#define SETUP_SERVER_HH_B3A8DA8BD3A64FAEBB957E50B12B46BE

#include "util/log/logger.hh"
#include "src/util/corba_wrapper_decl.hh"
#include "src/util/cfg/faked_args.hh"
#include "src/util/cfg/config_handler.hh"
#include "src/util/cfg/handle_logging_args.hh"
#include "src/util/cfg/handle_server_args.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"

#include "src/bin/corba/pidfile.hh"
#include "src/bin/corba/daemonize.hh"

#include <boost/any.hpp>

#include <string>

void setup_logging(CfgArgs* cfg_instance_ptr)
{
    HandleLoggingArgs* const handler_ptr = cfg_instance_ptr->get_handler_ptr_by_type<HandleLoggingArgs>();

    const auto log_type = static_cast<unsigned>(handler_ptr->log_type);
    Logging::Log::EventImportance min_importance = Logging::Log::EventImportance::trace;
    if ((log_type == 0) || (log_type == 1))
    {
        switch (handler_ptr->log_level)
        {
            case 0:
                min_importance = Logging::Log::EventImportance::emerg;
                break;
            case 1:
                min_importance = Logging::Log::EventImportance::alert;
                break;
            case 2:
                min_importance = Logging::Log::EventImportance::crit;
                break;
            case 3:
                min_importance = Logging::Log::EventImportance::err;
                break;
            case 4:
                min_importance = Logging::Log::EventImportance::warning;
                break;
            case 5:
                min_importance = Logging::Log::EventImportance::notice;
                break;
            case 6:
                min_importance = Logging::Log::EventImportance::info;
                break;
            case 7:
                min_importance = Logging::Log::EventImportance::debug;
                break;
            case 8:
                min_importance = Logging::Log::EventImportance::trace;
                break;
        }
    }

    switch (log_type)
    {
        case 0:
            LOGGER.add_handler_of<Logging::Log::Device::console>(min_importance);
            break;
        case 1:
            LOGGER.add_handler_of<Logging::Log::Device::file>(handler_ptr->log_file, min_importance);
            break;
        case 2:
            LOGGER.add_handler_of<Logging::Log::Device::syslog>(handler_ptr->log_syslog_facility);
            break;
    }
}

void run_server(CfgArgs * cfg_instance_ptr , CorbaContainer* corba_instance_ptr )
{
    corba_instance_ptr->poa_persistent->the_POAManager()->activate();

    //run server
    if (cfg_instance_ptr->get_handler_ptr_by_type<HandleServerArgs>()
        ->do_daemonize)
    {
        daemonize();
    }
    std::string pidfile_name = cfg_instance_ptr
        ->get_handler_ptr_by_type<HandleServerArgs>()->pidfile_name;

    if (!pidfile_name.empty())
    {
        PidFileNS::PidFileS::writePid(getpid(), pidfile_name);
    }

    corba_instance_ptr->orb->run();
}

void corba_init()
{
    FakedArgs orb_fa = CfgArgs::instance()->fa;
    HandleCorbaNameServiceArgs* ns_args_ptr=CfgArgs::instance()->
          get_handler_ptr_by_type<HandleCorbaNameServiceArgs>();

    CorbaContainer::set_instance(orb_fa.get_argc(), orb_fa.get_argv()
      , ns_args_ptr->nameservice_host
      , ns_args_ptr->nameservice_port
      , ns_args_ptr->nameservice_context);

}

#endif
