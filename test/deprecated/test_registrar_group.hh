/*
 * Copyright (C) 2010-2020  CZ.NIC, z. s. p. o.
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
#ifndef TEST_REGISTRAR_GROUP_HH_540CEF29D8004E06BCB52BE03B6AED6A
#define TEST_REGISTRAR_GROUP_HH_540CEF29D8004E06BCB52BE03B6AED6A

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

#include "libfred/db_settings.hh"
#include "corba/Admin.hh"
#include "src/deprecated/util/dbsql.hh"
#include "src/deprecated/libfred/registrar.hh"
#include "src/util/corba_wrapper_decl.hh"
#include "util/log/logger.hh"
#include "util/log/context.hh"

#include "src/util/concurrent_queue.hh"

#include "src/util/cfg/handle_general_args.hh"
#include "src/util/cfg/handle_database_args.hh"
#include "src/util/cfg/handle_threadgroup_args.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include "src/util/cfg/config_handler_decl.hh"
#include <boost/test/unit_test.hpp>

#endif // TEST_REGISTRAR_GROUP_H_

