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
#ifndef TEST_TREE_LIST_HH_78BC0DB8525725853AA7FB1C02EC558D//date "+%s.%N"|md5sum|tr "[a-f]" "[A-F]"
#define TEST_TREE_LIST_HH_78BC0DB8525725853AA7FB1C02EC558D

#include <string>
#include <vector>

namespace Test {
namespace Util {

using TestTreeList = std::vector<std::string>;

TestTreeList get_test_tree();

}//namespace Test::Util
}//namespace Test

#endif//TEST_TREE_LIST_HH_78BC0DB8525725853AA7FB1C02EC558D
