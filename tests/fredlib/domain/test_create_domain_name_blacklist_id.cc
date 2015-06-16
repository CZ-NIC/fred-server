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

#include <boost/test/unit_test.hpp>
#include <string>

#include "src/fredlib/opcontext.h"
#include <fredlib/domain.h>
#include <fredlib/contact.h>
#include "src/fredlib/domain/create_domain_name_blacklist_id.h"

#include "util/random_data_generator.h"
#include "tests/setup/fixtures.h"

const std::string server_name = "test-create-domain-name-blacklist-id";

struct create_domain_name_blacklist_id_fixture : public Test::Fixture::instantiate_db_template
{
    std::string registrar_handle;
    std::string xmark;
    std::string admin_contact2_handle;
    std::string registrant_contact_handle;
    std::string test_domain_fqdn;
    std::string regexp_test_domain_fqdn;
    Fred::ObjectId test_domain_id;

    create_domain_name_blacklist_id_fixture()
    :xmark(RandomDataGenerator().xnumstring(6))
    , admin_contact2_handle(std::string("TEST-CDNB-ADMIN-CONTACT-HANDLE") + xmark)
    , registrant_contact_handle(std::string("TEST-CDNB-REGISTRANT-CONTACT-HANDLE") + xmark)
    , test_domain_fqdn(std::string("fred") + xmark + ".cz")
    , regexp_test_domain_fqdn(std::string("^fred") + xmark + "\\.cz$")
    {
        Fred::OperationContextCreator ctx;
        registrar_handle = static_cast<std::string>(ctx.get_conn().exec(
            "SELECT handle FROM registrar WHERE system ORDER BY id LIMIT 1")[0][0]);
        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar

        Fred::Contact::PlaceAddress place;
        place.street1 = std::string("STR1") + xmark;
        place.city = "Praha";
        place.postalcode = "11150";
        place.country = "CZ";
        Fred::CreateContact(admin_contact2_handle,registrar_handle)
            .set_name(std::string("TEST-CDNB-ADMIN-CONTACT NAME")+xmark)
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);

        Fred::CreateContact(registrant_contact_handle,registrar_handle)
            .set_name(std::string("TEST-REGISTRANT-CONTACT NAME")+xmark)
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);

        Fred::CreateDomain(
                test_domain_fqdn //const std::string& fqdn
                , registrar_handle //const std::string& registrar
                , registrant_contact_handle //registrant
                )
            .set_admin_contacts(Util::vector_of<std::string>(admin_contact2_handle))
            .exec(ctx);

        test_domain_id = static_cast< Fred::ObjectId >(ctx.get_conn().exec_params(
            "SELECT id "
            "FROM object_registry "
            "WHERE name=$1::text AND "
                  "type=3 AND "
                  "erdate IS NULL", Database::query_param_list(test_domain_fqdn))[0][0]);

        ctx.commit_transaction();
    }
    ~create_domain_name_blacklist_id_fixture()
    {}
};

BOOST_FIXTURE_TEST_SUITE(TestCreateDomainNameBlacklistId, create_domain_name_blacklist_id_fixture)

/**
 * test CreateDomainNameBlacklistId
 * ...
 * calls in test shouldn't throw
 */
BOOST_AUTO_TEST_CASE(create_domain_name_blacklist_id)
{
    {
        Fred::OperationContextCreator ctx;
        Fred::CreateDomainNameBlacklistId(test_domain_id, "successfully tested").exec(ctx);
        ctx.commit_transaction();
    }
    Fred::OperationContextCreator ctx;
    Database::Result blacklist_result = ctx.get_conn().exec_params(
        "SELECT COUNT(*) "
        "FROM domain_blacklist "
        "WHERE regexp=$1::text AND "
              "valid_from<CURRENT_TIMESTAMP AND (CURRENT_TIMESTAMP<valid_to OR valid_to IS NULL)",
            Database::query_param_list(regexp_test_domain_fqdn));
    BOOST_CHECK(static_cast< unsigned >(blacklist_result[0][0]) == 1);
}

/**
 * test CreateDomainNameBlacklistIdBad
 * ...
 * calls in test should throw
 */
BOOST_AUTO_TEST_CASE(create_domain_name_blacklist_id_bad)
{
    Fred::ObjectId not_used_id;
    try {
        Fred::OperationContextCreator ctx;//new connection to rollback on error
        not_used_id = static_cast< Fred::ObjectId >(ctx.get_conn().exec("SELECT (MAX(id)+1000)*2 FROM object_registry")[0][0]);
        Fred::CreateDomainNameBlacklistId(not_used_id, "must throw exception").exec(ctx);
        ctx.commit_transaction();
        BOOST_CHECK(false);
    }
    catch(const Fred::CreateDomainNameBlacklistId::Exception &ex) {
        BOOST_CHECK(ex.is_set_object_id_not_found());
        BOOST_CHECK(ex.get_object_id_not_found() == not_used_id);
    }

    try {
        Fred::OperationContextCreator ctx;//new connection to rollback on error
        Fred::CreateDomainNameBlacklistId(test_domain_id, "successfully tested").exec(ctx);
        Fred::CreateDomainNameBlacklistId(test_domain_id, "must throw exception").exec(ctx); // already_blacklisted_domain
        ctx.commit_transaction();
        BOOST_CHECK(false);
    }
    catch(const Fred::CreateDomainNameBlacklistId::Exception &ex) {
        BOOST_CHECK(ex.is_set_already_blacklisted_domain());
        BOOST_CHECK(ex.get_already_blacklisted_domain() == test_domain_id);
    }

    try {
        Fred::OperationContextCreator ctx;//new connection to rollback on error
        Fred::CreateDomainNameBlacklistId(test_domain_id, "must throw exception",
          boost::posix_time::ptime(boost::gregorian::date(2006, 7, 31)),
          boost::posix_time::ptime(boost::gregorian::date(2005, 7, 31))).exec(ctx);
        ctx.commit_transaction();
        BOOST_CHECK(false);
    }
    catch(const Fred::CreateDomainNameBlacklistId::Exception &ex) {
        BOOST_CHECK(ex.is_set_out_of_turn());
    }

    try {
        Fred::OperationContextCreator ctx;//new connection to rollback on error
        Fred::CreateDomainNameBlacklistId blacklist(test_domain_id, "must throw exception");
        blacklist.set_valid_to(boost::posix_time::ptime(boost::gregorian::date(2006, 7, 31)));
        blacklist.exec(ctx);
        ctx.commit_transaction();
        BOOST_CHECK(false);
    }
    catch(const Fred::CreateDomainNameBlacklistId::Exception &ex) {
        BOOST_CHECK(ex.is_set_out_of_turn());
    }
}

BOOST_AUTO_TEST_SUITE_END();//TestCreateDomainNameBlacklistId
