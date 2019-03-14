/*
 * Copyright (C) 2014-2019  CZ.NIC, z. s. p. o.
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
 *  contact verification test for Czech postal address (defined in official set)
 */

#ifndef TEST_EMAIL_EXISTS_HH_0A7A8925A770410AB0036AFDC9A11442
#define TEST_EMAIL_EXISTS_HH_0A7A8925A770410AB0036AFDC9A11442

#include "src/backend/admin/contact/verification/test_impl/test_interface.hh"

#include <boost/assign/list_of.hpp>

namespace Fred {
namespace Backend {
namespace Admin {
namespace Contact {
namespace Verification {

FACTORY_MODULE_INIT_DECL(TestEmailExists_init)

class TestEmailExists
        : public Test,
          test_auto_registration<TestEmailExists>
{
public:
    virtual TestRunResult run(unsigned long long _history_id) const;

    static std::string registration_name()
    {
        return "email_host_existence";
    }

};

template <>
struct TestDataProvider<TestEmailExists>
        : TestDataProvider_common,
          _inheritTestRegName<TestEmailExists>
{
    std::string email_;

    virtual void store_data(const LibFred::InfoContactOutput& _data)
    {
        if (!_data.info_contact_data.email.isnull())
        {
            email_ = _data.info_contact_data.email.get_value_or_default();
        }
    }

    virtual std::vector<std::string> get_string_data() const
    {
        return boost::assign::list_of(email_);
    }

};

} // namespace Fred::Backend::Admin::Contact::Verification
} // namespace Fred::Backend::Admin::Contact
} // namespace Fred::Backend::Admin
} // namespace Fred::Backend
} // namespace Fred

#endif
