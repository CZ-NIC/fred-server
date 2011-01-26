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
 *  @file setup_server.h
 *  Implementation of server setup.
 */

#ifndef SETUP_SERVER_H_
#define SETUP_SERVER_H_

#include <string>
#include <boost/any.hpp>
#include "log/logger.h"
#include "corba_wrapper_decl.h"
#include "cfg/faked_args.h"
#include "cfg/config_handler.h"
#include "cfg/handle_logging_args.h"
#include "cfg/handle_server_args.h"
#include "cfg/handle_corbanameservice_args.h"

#include "pidfile.h"
#include "daemonize.h"

void setup_logging(CfgArgs * cfg_instance_ptr)
{
    // setting up logger
    Logging::Log::Type  log_type = static_cast<Logging::Log::Type>(
        cfg_instance_ptr->get_handler_ptr_by_type<HandleLoggingArgs>()
            ->log_type);

    boost::any param;
    if (log_type == Logging::Log::LT_FILE) param = cfg_instance_ptr
        ->get_handler_ptr_by_type<HandleLoggingArgs>()->log_file;

    if (log_type == Logging::Log::LT_SYSLOG) param = cfg_instance_ptr
        ->get_handler_ptr_by_type<HandleLoggingArgs>()
        ->log_syslog_facility;

    Logging::Manager::instance_ref().get(PACKAGE)
        .addHandler(log_type, param);

    Logging::Manager::instance_ref().get(PACKAGE).setLevel(
        static_cast<Logging::Log::Level>(
        cfg_instance_ptr->get_handler_ptr_by_type
            <HandleLoggingArgs>()->log_level));
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

#endif //SETUP_SERVER_H_
