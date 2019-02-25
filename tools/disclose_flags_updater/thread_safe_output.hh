/*
 * Copyright (C) 2018-2019  CZ.NIC, z. s. p. o.
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
#ifndef THREAD_SAFE_OUTPUT_HH_78DCBB04004B79212589DA174F0D99A6//date "+%s" | md5sum | cut -f1 -d" " | tr "[a-f]" "[A-F]" | tr -d "\n"
#define THREAD_SAFE_OUTPUT_HH_78DCBB04004B79212589DA174F0D99A6

#include <mutex>


namespace Tools {
namespace DiscloseFlagsUpdater {


void safe_cout(const std::string& _message);

void safe_cout_flush();

void safe_cerr(const std::string& _message);


}
}

#endif//THREAD_SAFE_OUTPUT_HH_78DCBB04004B79212589DA174F0D99A6
