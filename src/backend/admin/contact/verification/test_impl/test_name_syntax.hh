/*
 * Copyright (C) 2010-2019  CZ.NIC, z. s. p. o.
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
/**
 *  @file
 *  contact verification test for name (syntax only)
 */

#ifndef TEST_NAME_SYNTAX_HH_D585913B490D498FBDB3F06070B637B7
#define TEST_NAME_SYNTAX_HH_D585913B490D498FBDB3F06070B637B7

#include "src/backend/admin/contact/verification/test_impl/test_interface.hh"

#include <boost/assign/list_of.hpp>

namespace Fred {
namespace Backend {
namespace Admin {
namespace Contact {
namespace Verification {

FACTORY_MODULE_INIT_DECL(TestNameSyntax_init)

class TestNameSyntax
    : public
      Test,
          test_auto_registration<TestNameSyntax>
{

public:
    virtual TestRunResult run(unsigned long long _history_id) const;


    static std::string registration_name()
    {
        return "name_syntax";
    }

};

template <>
struct TestDataProvider<TestNameSyntax>
    : TestDataProvider_common,
      _inheritTestRegName<TestNameSyntax>
{
    std::string name_;

    virtual void store_data(const LibFred::InfoContactOutput& _data)
    {
        if (!_data.info_contact_data.name.isnull())
        {
            name_ = _data.info_contact_data.name.get_value_or_default();
        }
    }

    virtual std::vector<std::string> get_string_data() const
    {
        return boost::assign::list_of(name_);
    }

};

} // namespace Fred::Backend::Admin::Contact::Verification
} // namespace Fred::Backend::Admin::Contact
} // namespace Fred::Backend::Admin
} // namespace Fred::Backend
} // namespace Fred

#endif
