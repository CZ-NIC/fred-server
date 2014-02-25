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

#include <boost/algorithm/string/trim.hpp>

#include "src/admin/contact/verification/test_impl/test_name_syntax.h"
#include "src/fredlib/contact/verification/enum_test_status.h"

#include <fredlib/contact.h>

namespace Admin {
namespace ContactVerification {

    FACTORY_MODULE_INIT_DEFI(TestNameSyntax_init)

    Test::T_run_result TestNameSyntax::run(unsigned long long _history_id) const {
        TestDataProvider<TestNameSyntax> data;
        data.init_data(_history_id);

        string name =  boost::algorithm::trim_copy(static_cast<std::string>(data.name_));

        if( name.find(' ') == std::string::npos ) {
            return make_result(Fred::ContactTestStatus::FAIL, string("name has to contain at least two words separated by space") );
        } else {
            return make_result(Fred::ContactTestStatus::OK );
        }
    }
}
}
