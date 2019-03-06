/*
 * Copyright (C) 2014-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
/**
 *  @file
 *  integration tests for admin/contact/verification/enqueue_check.cc
 */

#include "src/backend/admin/contact/verification/delete_domains_of_invalid_contact.hh"
#include "src/backend/admin/contact/verification/enqueue_check.hh"
#include "src/backend/admin/contact/verification/resolve_check.hh"
#include "libfred/registrable_object/domain/check_domain.hh"
#include "libfred/registrable_object/domain/create_domain.hh"
#include "libfred/registrable_object/domain/delete_domain.hh"
#include "libfred/registrable_object/domain/info_domain.hh"
#include "libfred/registrable_object/domain/info_domain_diff.hh"
#include "libfred/registrable_object/domain/renew_domain.hh"
#include "libfred/registrable_object/domain/update_domain.hh"
#include "test/backend/admin/contact/verification/setup_utils.hh"
#include "test/setup/fixtures.hh"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestContactVerification)
BOOST_FIXTURE_TEST_SUITE(TestDeleteDomainOfInvalidatedContact, Test::instantiate_db_template)

const std::string server_name = "test-contact_verification_integration-delete_domains_of_invalid_contact";

/**
testing validity of newly created check
@pre existing contact owning a domain
@post domain deleted
*/
BOOST_AUTO_TEST_CASE(test_Deleting_domains)
{
    Test::contact contact;

    ::LibFred::OperationContextCreator ctx;
    ::LibFred::InfoContactOutput contact_info = ::LibFred::InfoContactByHandle(contact.info_data.handle).exec(ctx);

    std::vector<std::string> domain_names;

    domain_names.push_back("hokus" + RandomDataGenerator().xnumstring(10) + "1pokus.cz");
    domain_names.push_back("hokus" + RandomDataGenerator().xnumstring(10) + "2pokus.cz");
    domain_names.push_back("hokus" + RandomDataGenerator().xnumstring(10) + "3pokus.cz");

    for(std::vector<std::string>::const_iterator it = domain_names.begin();
        it != domain_names.end();
        ++it
    ) {
        ::LibFred::CreateDomain(
            *it,
            contact_info.info_contact_data.create_registrar_handle,
            contact.info_data.handle
        ).exec(ctx);
    }

    std::string check_handle = Fred::Backend::Admin::Contact::Verification::enqueue_check(
        ctx,
        contact.info_data.id,
        ::LibFred::TestsuiteHandle::MANUAL);

    ::LibFred::UpdateContactCheck(
        uuid::from_string( check_handle ),
        ::LibFred::ContactCheckStatus::AUTO_FAIL,
        Optional<unsigned long long>()
    ).exec(ctx);

    Fred::Backend::Admin::Contact::Verification::resolve_check(
        uuid::from_string( check_handle ),
        ::LibFred::ContactCheckStatus::FAIL,
        Optional<unsigned long long>()
    ).exec(ctx);

    // end of setup

    Fred::Backend::Admin::Contact::Verification::delete_domains_of_invalid_contact(
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
            ::LibFred::InfoDomainByFqdn(*it).exec(ctx);
        } catch(const ::LibFred::InfoDomainByFqdn::Exception& e) {
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
