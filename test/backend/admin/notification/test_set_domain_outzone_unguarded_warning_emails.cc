/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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
 */

#include "src/backend/admin/notification/notification.hh"
#include "libfred/opcontext.hh"
#include "libfred/opexception.hh"
#include "test/backend/admin/notification/fixtures.hh"
#include "test/setup/fixtures.hh"

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include <map>
#include <set>
#include <string>

namespace Test {

BOOST_AUTO_TEST_SUITE(Admin)
BOOST_AUTO_TEST_SUITE(Notification)
BOOST_AUTO_TEST_SUITE(SetDomainOutzoneUnguardedWarningEmails)

size_t count_domain_emails(const unsigned long long domain_id) {
    ::LibFred::OperationContextCreator ctx;
    const Database::Result db_result = ctx.get_conn().exec_params(
        "SELECT 1 FROM notify_outzone_unguarded_domain_additional_email "
            "WHERE domain_id = $1::bigint",
        Database::query_param_list(domain_id));
    ctx.commit_transaction();
    return db_result.size();
}


BOOST_FIXTURE_TEST_CASE(set_invalid_email, HasInvalidEmail)
{
    BOOST_CHECK_THROW(
        ::Admin::Notification::set_domain_outzone_unguarded_warning_emails(domain_emails_map),
        ::Admin::Notification::DomainEmailValidationError
    );
}

BOOST_FIXTURE_TEST_CASE(set_valid_email, HasValidEmail)
{
    ::Admin::Notification::set_domain_outzone_unguarded_warning_emails(domain_emails_map);
    BOOST_CHECK_EQUAL(count_domain_emails(domain.id), domain_emails_map[domain.id].size());
}

BOOST_FIXTURE_TEST_CASE(set_valid_emails, HasValidEmails)
{
    ::Admin::Notification::set_domain_outzone_unguarded_warning_emails(domain_emails_map);
    BOOST_CHECK_EQUAL(count_domain_emails(domain.id), domain_emails_map[domain.id].size());
}

BOOST_FIXTURE_TEST_CASE(set_same_emails, HasSameEmails)
{
    ::Admin::Notification::set_domain_outzone_unguarded_warning_emails(domain_emails_map);
    BOOST_CHECK_EQUAL(count_domain_emails(domain.id), domain_emails_map[domain.id].size());
}

BOOST_FIXTURE_TEST_CASE(set_empty_email, HasEmptyEmail)
{
    ::Admin::Notification::set_domain_outzone_unguarded_warning_emails(domain_emails_map);
    BOOST_CHECK_EQUAL(count_domain_emails(domain.id), domain_emails_map[domain.id].size() - 1);
}

BOOST_FIXTURE_TEST_CASE(set_no_emails, HasNoEmails)
{
    ::Admin::Notification::set_domain_outzone_unguarded_warning_emails(domain_emails_map);
    BOOST_CHECK_EQUAL(count_domain_emails(domain.id), domain_emails_map[domain.id].size());
    ::Admin::Notification::set_domain_outzone_unguarded_warning_emails(none_domain_emails_map);
    BOOST_CHECK_EQUAL(count_domain_emails(domain.id), none_domain_emails_map[domain.id].size());
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();

} // namespace Test
