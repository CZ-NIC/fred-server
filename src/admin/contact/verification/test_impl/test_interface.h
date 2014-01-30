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
 *  contact verification tests interface
 */

#ifndef CONTACT_VERIFICATION_TEST_INTF_11637813419_
#define CONTACT_VERIFICATION_TEST_INTF_11637813419_

#include <string>
#include <set>
#include <boost/tuple/tuple.hpp>
#include "util/optional_value.h"

namespace Admin
{
    using std::string;
    using std::set;
    using boost::tuple;

    class ContactVerificationTest {
        public:
            typedef tuple<
                string,                 // status
                Optional<string>,       // error message
                // XXX hopefuly one day related mail and messages will be unified
                set<unsigned long long>,// related mail archive ids
                set<unsigned long long> // related message archive ids
            > T_run_result;

            inline static T_run_result make_result(
                const string&                   _status,
                const Optional<string>&         _error_msg = Optional<string>(),
                // XXX hopefuly one day related mail and messages will be unified
                const set<unsigned long long>&  _related_mail_archive_ids = set<unsigned long long>(),
                const set<unsigned long long>&  _related_message_archive_ids = set<unsigned long long>()
            ) {
                return T_run_result(_status, _error_msg, _related_mail_archive_ids, _related_message_archive_ids);
            }

            /**
             * @return final status of the test, optional error message and optional related states and messages
             */
            virtual T_run_result run(long _history_id) const = 0;
            virtual string get_name() const = 0;
            virtual ~ContactVerificationTest() {};
    };
}

#endif // #include guard end
