/*
 * Copyright (C) 2013  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "src/admin/contact/verification/test_impl/test_interface.h"

namespace Admin {
namespace ContactVerification {
    Test::T_run_result Test::make_result(
        const string&                   _status,
        const Optional<string>&         _error_msg,
        // XXX hopefuly one day related mail and messages will be unified
        const set<unsigned long long>&  _related_mail_archive_ids,
        const set<unsigned long long>&  _related_message_archive_ids
    ) {
        return T_run_result(_status, _error_msg, _related_mail_archive_ids, _related_message_archive_ids);
    }

    Test::~Test() { }

    Fred::InfoContactOutput TestDataProvider_common::get_data(unsigned long long _contact_history_id) {
        Fred::OperationContext ctx;

        return Fred::InfoContactHistoryByHistoryid(_contact_history_id).exec(ctx);
    }

    TestDataProvider_intf& TestDataProvider_common::init_data(unsigned long long _contact_history_id) {
        this->store_data(this->get_data(_contact_history_id));

        return *this;
    }
}
}
