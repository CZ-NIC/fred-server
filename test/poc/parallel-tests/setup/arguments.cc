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
#include "test/poc/parallel-tests/setup/arguments.hh"

#include <cstring>

#include <algorithm>

namespace Test {
namespace Util {

Arguments::Arguments(std::ptrdiff_t reserved_capacity)
{
    argv_.reserve(reserved_capacity);
}

Arguments::Arguments(const char* cmd, const char* const* arg_begin, const char* const* arg_end)
    : Arguments{arg_end - arg_begin + 2}
{
    this->push_back(cmd);
    std::for_each(arg_begin, arg_end, [&](auto&& arg) { this->push_back(arg); });
    this->finish();
}

Arguments::Arguments(
        const char* cmd,
        std::vector<std::string>::const_iterator arg_begin,
        std::vector<std::string>::const_iterator arg_end)
    : Arguments{arg_end - arg_begin + 2}
{
    this->push_back(cmd);
    std::for_each(arg_begin, arg_end, [&](auto&& arg) { this->push_back(arg); });
    this->finish();
}

Arguments::~Arguments()
{
    std::for_each(begin(argv_), end(argv_) - 1, [](char*& value) { delete[] value; });
}

Arguments& Arguments::push_back(const char* argument)
{
    const auto argument_size = std::strlen(argument) + 1;
    char* copy = new char[argument_size];
    std::memcpy(copy, argument, argument_size);
    argv_.push_back(copy);
    return *this;
}

Arguments& Arguments::push_back(const std::string& argument)
{
    const auto argument_size = argument.size() + 1;
    char* copy = new char[argument_size];
    std::memcpy(copy, argument.c_str(), argument_size);
    argv_.push_back(copy);
    return *this;
}

Arguments& Arguments::push_back_non_empty(const char* option_name, const std::string& argument)
{
    if (!argument.empty())
    {
        this->push_back(option_name + argument);
    }
    return *this;
}

void Arguments::finish()
{
    argv_.push_back(nullptr);
}

std::size_t Arguments::count() const noexcept
{
    return argv_.size() - 1;
}

char** Arguments::data() noexcept
{
    return argv_.data();
}

}//namespace Test::Util
}//namespace Test
