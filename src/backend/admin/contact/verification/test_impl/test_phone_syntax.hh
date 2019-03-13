/*
 * Copyright (C) 2013-2019  CZ.NIC, z. s. p. o.
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
 *  contact verification test for telephone (syntax only)
 */

#ifndef TEST_PHONE_SYNTAX_HH_EA33EC78FD5A4D5FB4958729106C87BB
#define TEST_PHONE_SYNTAX_HH_EA33EC78FD5A4D5FB4958729106C87BB

#include "src/backend/admin/contact/verification/test_impl/test_interface.hh"

#include <boost/assign/list_of.hpp>
#include <boost/regex.hpp>

namespace Fred {
namespace Backend {
namespace Admin {
namespace Contact {
namespace Verification {

FACTORY_MODULE_INIT_DECL(TestPhoneSyntax_init)

class TestPhoneSyntax
        : public Test,
          test_auto_registration<TestPhoneSyntax>
{
    const boost::regex PHONE_PATTERN;

public:
    TestPhoneSyntax()
    // first draft of pattern - see ticket #9588
        : PHONE_PATTERN("^\\+[0-9]{1,3}\\.[0-9]{1,14}$")
    {
    }

    virtual TestRunResult run(unsigned long long _history_id) const;


    static std::string registration_name()
    {
        return "phone_syntax";
    }

};

template <>
struct TestDataProvider<TestPhoneSyntax>
        : TestDataProvider_common,
          _inheritTestRegName<TestPhoneSyntax>
{
    std::string phone_;

    virtual void store_data(const LibFred::InfoContactOutput& _data)
    {
        if (!_data.info_contact_data.telephone.isnull())
        {
            phone_ = _data.info_contact_data.telephone.get_value_or_default();
        }
    }

    virtual std::vector<std::string> get_string_data() const
    {
        return boost::assign::list_of(phone_);
    }

};

} // namespace Fred::Backend::Admin::Contact::Verification
} // namespace Fred::Backend::Admin::Contact
} // namespace Fred::Backend::Admin
} // namespace Fred::Backend
} // namespace Fred

#endif
