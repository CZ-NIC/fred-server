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
#include "test/poc/parallel-tests/setup/test_tree_list.hh"

#include <boost/test/unit_test.hpp>
#include <boost/test/tree/traverse.hpp>
#include <boost/test/tree/visitor.hpp>

#include <functional>

namespace Test {
namespace Util {

namespace {

class Visitor : public boost::unit_test::test_tree_visitor
{
public:
    Visitor(std::function<void(const std::string&)> on_test)
        : path_{},
          on_test_{on_test}
    { }
    void visit(const boost::unit_test::test_case& test) override
    {
        if (test.is_enabled())
        {
            on_test_(path_ + test.p_name.get());
        }
    }
    bool test_suite_start(const boost::unit_test::test_suite& suite) override
    {
        if (!is_module(suite))
        {
            path_ += suite.p_name.get() + "/";
        }
        return true;
    }
    void test_suite_finish(const boost::unit_test::test_suite& suite) override
    {
        if (!is_module(suite))
        {
            path_.resize(path_.size() - suite.p_name.get().size() - 1);
        }
    }
private:
    static bool is_module(const boost::unit_test::test_suite& suite) noexcept
    {
        return suite.p_type_name == "module";
    }
    std::string path_;
    std::function<void(const std::string&)> on_test_;
};

}//namespace Test::Util::{anonymous}

TestTreeList get_test_tree()
{
    TestTreeList result;
    Visitor visitor{[&](const std::string& test) { result.push_back(test); }};
    boost::unit_test::traverse_test_tree(boost::unit_test::framework::master_test_suite(), visitor);
    return result;
}

}//namespace Test::Util
}//namespace Test
