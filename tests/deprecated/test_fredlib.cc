/*
 * Copyright (C) 2014  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *  (at your option) any later version.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#define BOOST_TEST_MODULE Test Fredlib

#include <memory>
#include <stdexcept>
#include <iostream>
#include <string>
#include <algorithm>
#include <functional>
#include <numeric>
#include <queue>
#include <sys/time.h>
#include <time.h>

#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/lexical_cast.hpp>

#include "src/fredlib/db_settings.h"

#include "log/logger.h"
#include "log/context.h"
#include "random_data_generator.h"
#include "concurrent_queue.h"

#include "cfg/handle_tests_args.h"
#include "cfg/handle_server_args.h"
#include "cfg/handle_logging_args.h"
#include "cfg/handle_database_args.h"

#include "cfg/handle_threadgroup_args.h"
#include "cfg/handle_registry_args.h"
#include "cfg/handle_rifd_args.h"
#include "cfg/handle_contactverification_args.h"
#include "cfg/handle_mojeid_args.h"

#include "config.h" // needed for CONFIG_FILE


//args processing config for custom main
HandlerPtrVector global_hpv =
boost::assign::list_of
(HandleArgsPtr(new HandleTestsArgs(CONFIG_FILE)))
(HandleArgsPtr(new HandleServerArgs))
(HandleArgsPtr(new HandleLoggingArgs))
(HandleArgsPtr(new HandleDatabaseArgs))

(HandleArgsPtr(new HandleThreadGroupArgs))
(HandleArgsPtr(new HandleRegistryArgs))
(HandleArgsPtr(new HandleRifdArgs))
(HandleArgsPtr(new HandleContactVerificationArgs))
(HandleArgsPtr(new HandleMojeIDArgs))
;

#include "cfg/test_custom_main.h"

class ElapsedTimeFixture
{
    ElapsedTime et_;
public:
    ElapsedTimeFixture()
    : et_("elapsed time: ", cout_print())
    {}
};

BOOST_GLOBAL_FIXTURE(ElapsedTimeFixture);


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

class LoggingFixture {
public:
    LoggingFixture() {
        // setting up logger
        setup_logging(CfgArgs::instance());
    }

};

BOOST_GLOBAL_FIXTURE( LoggingFixture );

