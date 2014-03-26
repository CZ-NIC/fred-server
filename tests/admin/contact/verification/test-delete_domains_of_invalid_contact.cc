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
 *  integration tests for admin/contact/verification/enqueue_check.cc
 */

#include "src/admin/contact/verification/delete_domains_of_invalid_contact.h"

#include "src/admin/contact/verification/resolve_check.h"
#include "src/admin/contact/verification/enqueue_check.h"

#include <fredlib/domain.h>

#include <boost/test/unit_test.hpp>

#include "tests/admin/contact/verification/setup_utils.h"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

BOOST_AUTO_TEST_SUITE(TestContactVerification)
BOOST_FIXTURE_TEST_SUITE(TestDeleteDomainOfInvalidatedContact, autoclean_contact_verification_db)

const std::string server_name = "test-contact_verification_integration-delete_domains_of_invalid_contact";

/**
testing validity of newly created check
@pre existing contact owning a domain
@post domain deleted
*/
BOOST_AUTO_TEST_CASE(test_Deleting_domains)
{
    setup_contact contact;

    Fred::OperationContext ctx;
    Fred::InfoContactOutput contact_info = Fred::InfoContactByHandle(contact.contact_handle).exec(ctx);

    std::vector<std::string> domain_names;

    domain_names.push_back("hokus" + RandomDataGenerator().xnumstring(10) + "1pokus.cz");
    domain_names.push_back("hokus" + RandomDataGenerator().xnumstring(10) + "2pokus.cz");
    domain_names.push_back("hokus" + RandomDataGenerator().xnumstring(10) + "3pokus.cz");

    for(std::vector<std::string>::const_iterator it = domain_names.begin();
        it != domain_names.end();
        ++it
    ) {
        Fred::CreateDomain(
            *it,
            contact_info.info_contact_data.create_registrar_handle,
            contact.contact_handle
        ).exec(ctx);
    }

    std::string check_handle = Admin::enqueue_check(
        ctx,
        contact.contact_id_,
        Fred::TestsuiteHandle::MANUAL);

    Admin::resolve_check(
        uuid::from_string( check_handle ),
        Fred::ContactCheckStatus::FAIL,
        Optional<unsigned long long>()
    ).exec(ctx);

    // end of setup

    Admin::delete_domains_of_invalid_contact(
        ctx,
        uuid::from_string( check_handle) );

    bool domain_deleted = false;

    ctx.get_conn().exec("SAVEPOINT pre_info_savepoint");

    for(std::vector<std::string>::const_iterator it = domain_names.begin();
        it != domain_names.end();
        ++it
    ) {
        domain_deleted = false;

        try {
            Fred::InfoDomainByHandle(*it).exec(ctx);
        } catch(const Fred::InfoDomainByHandle::Exception& e) {
            ctx.get_conn().exec("ROLLBACK TO pre_info_savepoint");

            if(e.is_set_unknown_fqdn()) {
                domain_deleted = true;
            }
        }
        BOOST_CHECK_EQUAL(domain_deleted, true);
    }
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
