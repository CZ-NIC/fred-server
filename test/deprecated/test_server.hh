/*
 * Copyright (C) 2010-2019  CZ.NIC, z. s. p. o.
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
#ifndef TEST_SERVER_HH_A22A698B89124E618E6E6F0F07FA4AA5
#define TEST_SERVER_HH_A22A698B89124E618E6E6F0F07FA4AA5

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

#include "libfred/db_settings.hh"
#include "src/util/corba_wrapper.hh"
#include "util/log/logger.hh"
#include "util/log/context.hh"
#include "src/util/concurrent_queue.hh"

#include "src/util/setup_server.hh"

#include "src/util/cfg/handle_tests_args.hh"
#include "src/util/cfg/handle_server_args.hh"
#include "src/util/cfg/handle_logging_args.hh"
#include "src/util/cfg/handle_database_args.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"
#include "src/util/cfg/handle_threadgroup_args.hh"
#include "src/util/cfg/handle_registry_args.hh"
#include "src/util/cfg/handle_rifd_args.hh"
#include "src/util/cfg/handle_contactverification_args.hh"
#include "src/util/cfg/handle_mojeid_args.hh"
#include "src/util/cfg/handle_domainbrowser_args.hh"


#endif
