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
#include "tools/disclose_flags_updater/thread_safe_output.hh"

#include <iostream>
#include <mutex>

namespace Tools {
namespace DiscloseFlagsUpdater {


std::mutex& get_output_mutex()
{
    static std::mutex m;
    return m;
}


void safe_cout(const std::string& _message)
{
    std::lock_guard<std::mutex> lock(get_output_mutex());
    std::cout << _message;
}


void safe_cout_flush()
{
    std::lock_guard<std::mutex> lock(get_output_mutex());
    std::cout.flush();
}


void safe_cerr(const std::string& _message)
{
    std::lock_guard<std::mutex> lock(get_output_mutex());
    std::cerr << _message;
}


}
}
