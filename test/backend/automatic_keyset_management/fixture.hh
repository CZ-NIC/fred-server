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

#ifndef FIXTURE_HH_2F32CFFD20C14C38B52FC66CEC983B63
#define FIXTURE_HH_2F32CFFD20C14C38B52FC66CEC983B63

#include "test/setup/fixtures.hh"
#include "test/setup/fixtures_utils.hh"
#include "libfred/object_state/get_object_states.hh"
#include "src/deprecated/libfred/documents.hh"

#include "src/util/cfg/config_handler_decl.hh"
#include "src/util/cfg/handle_general_args.hh"
#include "src/util/cfg/handle_server_args.hh"
#include "src/util/cfg/handle_logging_args.hh"
#include "src/util/cfg/handle_database_args.hh"
#include "src/util/cfg/handle_registry_args.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"

#include "src/backend/automatic_keyset_management/automatic_keyset_management.hh"


#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/test/test_tools.hpp>

namespace Test {
namespace AutomaticKeysetManagement {


} // namespace Test::AutomaticKeysetManagement
} // namespace Test

#endif
