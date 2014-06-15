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

/**
 *  @file
 *  contact verification test for e-mail address (syntax only)
 */

#ifndef CONTACT_VERIFICATION_TEST_EMAIL_SYNTAX_11637813419_
#define CONTACT_VERIFICATION_TEST_EMAIL_SYNTAX_11637813419_

#include <boost/regex.hpp>
#include <boost/assign/list_of.hpp>

#include "src/admin/contact/verification/test_impl/test_interface.h"

namespace Admin
{
namespace ContactVerification
{
    FACTORY_MODULE_INIT_DECL(TestEmailSyntax_init)

    class TestEmailSyntax
    : public
        Test,
        test_auto_registration<TestEmailSyntax>
    {
            const boost::regex EMAIL_PATTERN;

        public:
            TestEmailSyntax()
            /* legacy compatibility
             * old e-mail data in register can be multiple e-mail addresses separated by commas ","
             */
            : EMAIL_PATTERN (
                "^"
                    // first - mandatory e-mail
                    "[-!#$%&'*+/=?^_`{}|~0-9A-Za-z]+(\\.[-!#$%&'*+/=?^_`{}|~0-9A-Za-z]+)*"
                    "@(?:[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?\\.)+[A-Za-z]{2,6}\\.?"
                    "(?:" // list of ...
                        "," //< ... comma separated ...
                        // ... other addresses ...
                        "[-!#$%&'*+/=?^_`{}|~0-9A-Za-z]+(\\.[-!#$%&'*+/=?^_`{}|~0-9A-Za-z]+)*"
                        "@(?:[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?\\.)+[A-Za-z]{2,6}\\.?"
                    ")*" //< ... which is only optional
                "$",
                boost::regex::icase
            ) {}

            virtual TestRunResult run(unsigned long long _history_id) const;
            static std::string registration_name() { return "email_syntax"; }
    };

    template<> struct TestDataProvider<TestEmailSyntax>
    : TestDataProvider_common,
      _inheritTestRegName<TestEmailSyntax>
    {
        std::string email_;

        virtual void store_data(const Fred::InfoContactOutput& _data) {
            if( !_data.info_contact_data.email.isnull() ) {
                email_ = _data.info_contact_data.email.get_value_or_default();
            }
        }

        virtual std::vector<std::string> get_string_data() const {
            return boost::assign::list_of(email_);
        }
    };
}
}

#endif // #include guard end
