/*
 * Copyright (C) 2022  CZ.NIC, z. s. p. o.
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
#ifndef ARGUMENTS_HH_D9E068BE6D41B4827C9DDDB5807A56C9//date "+%s.%N"|md5sum|tr "[a-f]" "[A-F]"
#define ARGUMENTS_HH_D9E068BE6D41B4827C9DDDB5807A56C9

#include <cstddef>
#include <string>
#include <vector>

namespace Test {
namespace Util {

class Arguments
{
public:
    explicit Arguments(std::ptrdiff_t reserved_capacity = 0);
    explicit Arguments(const char* cmd, const char* const* arg_begin, const char* const* arg_end);
    explicit Arguments(
            const char* cmd,
            std::vector<std::string>::const_iterator arg_begin,
            std::vector<std::string>::const_iterator arg_end);
    ~Arguments();
    Arguments& push_back(const char* argument);
    Arguments& push_back(const std::string& argument);
    Arguments& push_back_non_empty(const char* option_name, const std::string& argument);
    void finish();
    std::size_t count() const noexcept;
    char** data() noexcept;
private:
    std::vector<char*> argv_;
};

}//namespace Test::Util
}//namespace Test

#endif//ARGUMENTS_HH_D9E068BE6D41B4827C9DDDB5807A56C9
