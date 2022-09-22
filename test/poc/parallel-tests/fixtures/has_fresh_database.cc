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
#include "test/poc/parallel-tests/fixtures/has_fresh_database.hh"

#include <stdexcept>
#include <utility>


namespace Test {

namespace {

std::function<void()>* restore_test_database_procedure = nullptr;

bool committed = false;

void restore_test_database()
{
    if (restore_test_database_procedure == nullptr)
    {
        throw std::runtime_error{"restore_test_database_procedure is null"};
    }
    (*restore_test_database_procedure)();
}

}//namespace Test::{anonymous}

HasFreshDatabase::HasFreshDatabase()
{
    if (committed)
    {
        restore_test_database();
        committed = false;
    }
}

void HasFreshDatabase::clear_restore_test_database_procedure()
{
    if (restore_test_database_procedure != nullptr)
    {
        delete restore_test_database_procedure;
        restore_test_database_procedure = nullptr;
    }
}

void HasFreshDatabase::set_restore_test_database_procedure(std::function<void()> procedure)
{
    if (restore_test_database_procedure == nullptr)
    {
        restore_test_database_procedure = new std::function<void()>{std::move(procedure)};
    }
    else
    {
        *restore_test_database_procedure = std::move(procedure);
    }
}

void HasFreshDatabase::commit_done() noexcept
{
    committed = true;
}

}//namespace Test
