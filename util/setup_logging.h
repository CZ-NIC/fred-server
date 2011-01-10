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
 *  @file setup_logging.h
 *  Implementation of server logging setup.
 */

#ifndef SETUP_LOGGING_H_
#define SETUP_LOGGING_H_

#include <boost/any.hpp>
#include "log/logger.h"
#include "cfg/config_handler.h"
#include "cfg/handle_logging_args.h"

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

#endif //SETUP_LOGGING_H_
