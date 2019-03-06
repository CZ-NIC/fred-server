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
#include "src/backend/admin/contact/verification/test_impl/test_name_syntax.hh"

#include "libfred/registrable_object/contact/check_contact.hh"
#include "libfred/registrable_object/contact/copy_contact.hh"
#include "libfred/registrable_object/contact/create_contact.hh"
#include "libfred/registrable_object/contact/delete_contact.hh"
#include "libfred/registrable_object/contact/info_contact.hh"
#include "libfred/registrable_object/contact/info_contact_diff.hh"
#include "libfred/registrable_object/contact/merge_contact.hh"
#include "libfred/registrable_object/contact/update_contact.hh"
#include "libfred/registrable_object/contact/verification/enum_test_status.hh"

#include <boost/algorithm/string/trim.hpp>

namespace Fred {
namespace Backend {
namespace Admin {
namespace Contact {
namespace Verification {

FACTORY_MODULE_INIT_DEFI(TestNameSyntax_init)

Test::TestRunResult TestNameSyntax::run(unsigned long long _history_id) const
{
    TestDataProvider<TestNameSyntax> data;
    data.init_data(_history_id);

    std::string name =  boost::algorithm::trim_copy(static_cast<std::string>(data.name_));

    if (name.find(' ') == std::string::npos)
    {
        return TestRunResult(
                LibFred::ContactTestStatus::FAIL,
                std::string("name has to contain at least two words separated by space"));
    }
    else
    {
        return TestRunResult(LibFred::ContactTestStatus::OK);
    }
}

} // namespace Fred::Backend::Admin::Contact::Verification
} // namespace Fred::Backend::Admin::Contact
} // namespace Fred::Backend::Admin
} // namespace Fred::Backend
} // namespace Fred
