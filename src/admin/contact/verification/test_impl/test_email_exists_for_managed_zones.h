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
 *  contact verification test for Czech postal address (defined in official set)
 */

#ifndef CONTACT_VERIFICATION_TEST_EMAIL_EXISTS_FOR_MANAGED_ZONES_23545756763574_
#define CONTACT_VERIFICATION_TEST_EMAIL_EXISTS_FOR_MANAGED_ZONES_23545756763574_

#include "src/admin/contact/verification/test_impl/test_interface.h"

#include <boost/assign/list_of.hpp>

namespace Admin
{
namespace ContactVerification
{
    FACTORY_MODULE_INIT_DECL(TestEmailExistsForManagedZones_init)

    class TestEmailExistsForManagedZones
    : public
        Test,
        test_auto_registration<TestEmailExistsForManagedZones>
    {
        public:
            virtual TestRunResult run(unsigned long long _history_id) const;
            static std::string registration_name() { return "email_existence_in_managed_zones"; }
    };

    template<> struct TestDataProvider<TestEmailExistsForManagedZones>
    : TestDataProvider_common,
      _inheritTestRegName<TestEmailExistsForManagedZones>
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
