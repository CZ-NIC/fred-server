/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
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

#ifndef FIXTURE_H_97F8653D6277433B888C3C90BB2B858D
#define FIXTURE_H_97F8653D6277433B888C3C90BB2B858D

#include "tests/setup/fixtures.h"
#include "tests/setup/fixtures_utils.h"
#include "src/fredlib/object_state/get_object_states.h"
#include "src/fredlib/documents.h"

#include "util/cfg/config_handler_decl.h"
#include "cfg/handle_general_args.h"
#include "cfg/handle_server_args.h"
#include "cfg/handle_logging_args.h"
#include "cfg/handle_database_args.h"
#include "cfg/handle_registry_args.h"
#include "cfg/handle_corbanameservice_args.h"

#include "src/automatic_keyset_management/automatic_keyset_management.hh"


#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/test/test_tools.hpp>

namespace Test {
namespace AutomaticKeysetManagement {


} // namespace Test::AutomaticKeysetManagement
} // namespace Test

#endif
