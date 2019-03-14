/*
 * Copyright (C) 2014-2019  CZ.NIC, z. s. p. o.
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
#ifndef OPTYS_GET_UNDELIVERED_HH_F0E5D0D92ACB4E49BC5CB661EFED8B44
#define OPTYS_GET_UNDELIVERED_HH_F0E5D0D92ACB4E49BC5CB661EFED8B44

#include <string>

namespace Admin {

/**
 * Get undelivered letter IDs from Optys.
 */
void notify_letters_optys_get_undelivered_impl(
        const std::string& optys_config_file,
        bool all_local_files_only,
        bool dump_config_to_log);
} // namespace Admin;

#endif

