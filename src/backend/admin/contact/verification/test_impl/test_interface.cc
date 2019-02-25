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
#include "src/backend/admin/contact/verification/test_impl/test_interface.hh"

namespace Fred {
namespace Backend {
namespace Admin {
namespace Contact {
namespace Verification {

Test::TestRunResult::TestRunResult(
        const std::string&                   _status,
        const Optional<std::string>&         _error_msg,
        // XXX hopefuly one day related mail and messages will be unified
        const std::set<unsigned long long>&  _related_mail_archive_ids,
        const std::set<unsigned long long>&  _related_message_archive_ids)
    : status(_status),
      error_message(_error_msg),
      related_mail_archive_ids(_related_mail_archive_ids),
      related_message_archive_ids(_related_message_archive_ids)
{
}


Test::~Test()
{
}


LibFred::InfoContactOutput TestDataProvider_common::get_data(unsigned long long _contact_history_id)
{
    LibFred::OperationContextCreator ctx;

    return LibFred::InfoContactHistoryByHistoryid(_contact_history_id).exec(ctx);
}


TestDataProvider_intf& TestDataProvider_common::init_data(unsigned long long _contact_history_id)
{
    this->store_data(this->get_data(_contact_history_id));

    return *this;
}

} // namespace Fred::Backend::Admin::Contact::Verification
} // namespace Fred::Backend::Admin::Contact
} // namespace Fred::Backend::Admin
} // namespace Fred::Backend
} // namespace Fred
