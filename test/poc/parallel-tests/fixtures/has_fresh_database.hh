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
#ifndef HAS_FRESH_DATABASE_HH_DAD392B44568C87C28D42596666C1B70//date "+%s.%N"|md5sum|tr "[a-f]" "[A-F]"
#define HAS_FRESH_DATABASE_HH_DAD392B44568C87C28D42596666C1B70

#include <functional>


namespace Test {

struct HasFreshDatabase
{
    HasFreshDatabase();
    static void clear_restore_test_database_procedure();
    static void set_restore_test_database_procedure(std::function<void()> procedure);
    static void commit_done() noexcept;
};

}//namespace Test

#endif//HAS_FRESH_DATABASE_HH_DAD392B44568C87C28D42596666C1B70
