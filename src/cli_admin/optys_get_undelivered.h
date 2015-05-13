/*
 *  Copyright (C) 2014  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef OPTYS_GET_UNDELIVERED_H_
#define OPTYS_GET_UNDELIVERED_H_

#include <string>

namespace Admin {

/**
 * Get undelivered letter IDs from Optys.
 */
void notify_letters_optys_get_undelivered_impl(const std::string& optys_config_file, bool all_local_files_only);
} // namespace Admin;

#endif
