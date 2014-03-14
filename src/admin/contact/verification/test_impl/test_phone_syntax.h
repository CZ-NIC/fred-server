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
 *  contact verification test for telephone (syntax only)
 */

#ifndef CONTACT_VERIFICATION_TEST_PHONE_SYNTAX_976844345676_
#define CONTACT_VERIFICATION_TEST_PHONE_SYNTAX_976844345676_

#include <boost/regex.hpp>

#include "src/admin/contact/verification/test_impl/test_interface.h"

#include <boost/assign/list_of.hpp>

namespace Admin
{
namespace ContactVerification
{
    FACTORY_MODULE_INIT_DECL(TestPhoneSyntax_init)

    class TestPhoneSyntax
    : public
        Test,
        test_auto_registration<TestPhoneSyntax>
    {
            const boost::regex PHONE_PATTERN;

        public:
            TestPhoneSyntax()
                // first draft of pattern - see ticket #9588
                : PHONE_PATTERN ("^\\+[0-9]{1,3}\\.[0-9]{1,14}$") {}

            virtual T_run_result run(unsigned long long _history_id) const;
            static std::string registration_name() { return "phone_syntax"; }
    };

    template<> struct TestDataProvider<TestPhoneSyntax>
    : TestDataProvider_common,
      _inheritTestRegName<TestPhoneSyntax>
    {
        std::string phone_;

        virtual void store_data(const Fred::InfoContactOutput& _data) {
            if(_data.info_contact_data.telephone.isnull() == false) {
                phone_ = _data.info_contact_data.telephone.get_value_or_default();
            }
        }

        virtual vector<string> get_string_data() const {
            return boost::assign::list_of(phone_);
        }
    };
}
}

#endif // #include guard end
