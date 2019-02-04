/*
 * Copyright (C) 2007  CZ.NIC, z.s.p.o.
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

// test-file-manager-client.h

#ifndef TEST_FILE_MANAGER_CLIENT_HH_BE8B1ACDEA434EFDB0D3325CA882E051
#define TEST_FILE_MANAGER_CLIENT_HH_BE8B1ACDEA434EFDB0D3325CA882E051

#include <memory>
#include <iostream>
#include <string>
#include <algorithm>
#include <functional>
#include <numeric>
#include <map>
#include <exception>
#include <queue>
#include <sys/time.h>
#include <time.h>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time.hpp>
#include <boost/assign/list_of.hpp>

#include "src/util/corba_wrapper_decl.hh"
#include "util/log/logger.hh"
#include "util/log/context.hh"

#include "util/random_data_generator.hh"
#include "src/util/concurrent_queue.hh"
#include "util/types/common.hh"

#include "src/util/cfg/handle_general_args.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"
#include "src/bin/corba/file_manager_client.hh"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include "src/util/cfg/config_handler_decl.hh"
#include <boost/test/unit_test.hpp>

#endif
