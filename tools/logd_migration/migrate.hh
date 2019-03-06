/*
 * Copyright (C) 2009-2019  CZ.NIC, z. s. p. o.
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
#ifndef MIGRATE_HH_B07B1FEC47E94E6FBABACCB0D12970E9
#define MIGRATE_HH_B07B1FEC47E94E6FBABACCB0D12970E9

#include <iostream>
#include <fstream>
#include <map>


#include "util/db/transaction.hh"

#include "src/deprecated/libfred/requests/request.hh"


#include "util/types/id.hh"

// needed becase of pool_subst
#include "tools/logd_migration/m_epp_parser.hh"

#define ALLOC_STEP 4

using namespace Database;
using namespace LibFred::Logger;

typedef unsigned long long TID;


#endif

