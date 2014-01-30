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

#include <boost/regex.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/trim.hpp>

#include "src/admin/contact/verification/test_impl/test_phone_syntax.h"
#include "src/fredlib/contact/verification/enum_test_status.h"

#include <fredlib/contact.h>

namespace Admin {
    ContactVerificationTest::T_run_result ContactVerificationTestPhoneSyntax::run(long _history_id) const {
        Fred::OperationContext ctx;
        const Fred::InfoContactData& contact_data = Fred::HistoryInfoContactByHistoryid(_history_id).exec(ctx).info_contact_data;

        std::string trimmed_telephone(static_cast<std::string>(contact_data.telephone));
        boost::algorithm::trim(trimmed_telephone);

        if(trimmed_telephone.empty()) {
            return make_result(Fred::ContactTestStatus::SKIPPED, string("optional telephone is empty") );
        }

        if ( boost::regex_match(
                // if Nullable is NULL then this casts returns empty string
                trimmed_telephone,
                PHONE_PATTERN )
        ) {
            return make_result(Fred::ContactTestStatus::OK );
        }

        return make_result(Fred::ContactTestStatus::FAIL, string("invalid phone format") );
    }
}
