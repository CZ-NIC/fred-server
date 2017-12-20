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

#include "src/backend/admin/contact/verification/test_impl/test_phone_syntax.hh"
#include "src/libfred/registrable_object/contact/verification/enum_test_status.hh"

#include <libfred/contact.hh>

namespace Admin {
namespace ContactVerification {

    FACTORY_MODULE_INIT_DEFI(TestPhoneSyntax_init)

    Test::TestRunResult TestPhoneSyntax::run(unsigned long long _history_id) const {
        TestDataProvider<TestPhoneSyntax> data;
        data.init_data(_history_id);

        std::string trimmed_telephone =  boost::algorithm::trim_copy(static_cast<std::string>(data.phone_));

        if(trimmed_telephone.empty()) {
            return TestRunResult(LibFred::ContactTestStatus::SKIPPED, std::string("optional telephone is empty") );
        }

        if ( boost::regex_match(
                // if Nullable is NULL then this casts returns empty string
                trimmed_telephone,
                PHONE_PATTERN )
        ) {
            return TestRunResult(LibFred::ContactTestStatus::OK );
        }

        return TestRunResult(LibFred::ContactTestStatus::FAIL, std::string("invalid phone format") );
    }
}
}
