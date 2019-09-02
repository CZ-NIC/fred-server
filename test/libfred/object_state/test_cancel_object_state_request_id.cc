/*
 * Copyright (C) 2013-2019  CZ.NIC, z. s. p. o.
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
#include "libfred/object_state/cancel_object_state_request_id.hh"
#include "libfred/object_state/create_object_state_request_id.hh"
#include "libfred/object_state/perform_object_state_request.hh"
#include "libfred/opcontext.hh"
#include "libfred/registrable_object/contact/check_contact.hh"
#include "libfred/registrable_object/contact/copy_contact.hh"
#include "libfred/registrable_object/contact/create_contact.hh"
#include "libfred/registrable_object/contact/delete_contact.hh"
#include "libfred/registrable_object/contact/info_contact.hh"
#include "libfred/registrable_object/contact/info_contact_diff.hh"
#include "libfred/registrable_object/contact/merge_contact.hh"
#include "libfred/registrable_object/contact/update_contact.hh"
#include "libfred/registrable_object/domain/check_domain.hh"
#include "libfred/registrable_object/domain/create_domain.hh"
#include "libfred/registrable_object/domain/delete_domain.hh"
#include "libfred/registrable_object/domain/info_domain.hh"
#include "libfred/registrable_object/domain/info_domain_diff.hh"
#include "libfred/registrable_object/domain/renew_domain.hh"
#include "libfred/registrable_object/domain/update_domain.hh"
#include "util/random/char_set/char_set.hh"
#include "util/random/random.hh"
#include "test/setup/fixtures.hh"

#include <boost/test/unit_test.hpp>

#include <string>

const std::string server_name = "test-cancel-object-state-request-id";

struct cancel_object_state_request_id_fixture : public Test::instantiate_db_template
{
    std::string registrar_handle;
    std::string xmark;
    std::string admin_contact2_handle;
    std::string registrant_contact_handle;
    std::string test_domain_fqdn;
    ::LibFred::ObjectId test_domain_id;
    ::LibFred::StatusList status_list;

    cancel_object_state_request_id_fixture()
    :xmark(Random::Generator().get_seq(Random::CharSet::digits(), 6))
    , admin_contact2_handle(std::string("TEST-COSR-ADMIN-CONTACT-HANDLE") + xmark)
    , registrant_contact_handle(std::string("TEST-COSR-REGISTRANT-CONTACT-HANDLE") + xmark)
    , test_domain_fqdn ( std::string("fred")+xmark+".cz")
    {
        ::LibFred::OperationContextCreator ctx;
        registrar_handle = static_cast<std::string>(ctx.get_conn().exec(
            "SELECT handle FROM registrar WHERE system ORDER BY id LIMIT 1")[0][0]);
        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar

        ::LibFred::Contact::PlaceAddress place;
        place.street1 = std::string("STR1") + xmark;
        place.city = "Praha";
        place.postalcode = "11150";
        place.country = "CZ";
        ::LibFred::CreateContact(admin_contact2_handle,registrar_handle)
            .set_name(std::string("TEST-COSR-ADMIN-CONTACT NAME")+xmark)
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);

        ::LibFred::CreateContact(registrant_contact_handle,registrar_handle)
            .set_name(std::string("TEST-REGISTRANT-CONTACT NAME")+xmark)
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);

        ::LibFred::CreateDomain(
                test_domain_fqdn //const std::string& fqdn
                , registrar_handle //const std::string& registrar
                , registrant_contact_handle //registrant
                )
            .set_admin_contacts(Util::vector_of<std::string>(admin_contact2_handle))
            .exec(ctx);

        Database::Result status_result = ctx.get_conn().exec("SELECT name FROM enum_object_states WHERE manual AND 3=ANY(types)");
        for (::size_t idx = 0; idx < status_result.size(); ++idx) {
            status_list.insert(status_result[idx][0]);
        }
        test_domain_id = static_cast< ::LibFred::ObjectId >(ctx.get_conn().exec_params(
            "SELECT id "
            "FROM object_registry "
            "WHERE name=$1::text AND "
                  "type=3 AND "
                  "erdate IS NULL", Database::query_param_list(test_domain_fqdn))[0][0]);
        ::LibFred::CreateObjectStateRequestId(test_domain_id, status_list).exec(ctx);
        ::LibFred::PerformObjectStateRequest(test_domain_id).exec(ctx);
        ctx.commit_transaction();
    }
    ~cancel_object_state_request_id_fixture()
    {}
};

BOOST_FIXTURE_TEST_SUITE(TestCancelObjectStateRequestId, cancel_object_state_request_id_fixture  )

/**
 * test CancelObjectStateRequestId
 * ...
 * calls in test shouldn't throw
 */
BOOST_AUTO_TEST_CASE(cancel_object_state_request_id)
{
    {
        ::LibFred::OperationContextCreator ctx;
        ::LibFred::CancelObjectStateRequestId(test_domain_id, status_list).exec(ctx);
        ctx.commit_transaction();
    }
    ::LibFred::OperationContextCreator ctx;
    std::ostringstream query;
    Database::query_param_list param(test_domain_id);
    ::LibFred::StatusList::const_iterator pStatus = status_list.begin();
    param(*pStatus);
    ++pStatus;
    query << "SELECT eos.name "
             "FROM object_state_request osr "
             "JOIN object_registry obr ON obr.id=osr.object_id "
             "JOIN enum_object_states eos ON (eos.id=osr.state_id AND obr.type=ANY(eos.types)) "
             "WHERE osr.object_id=$1::bigint AND "
                   "osr.valid_from<=CURRENT_TIMESTAMP AND "
                   "(osr.valid_to IS NULL OR CURRENT_TIMESTAMP<valid_to) AND "
                   "obr.erdate IS NULL AND "
                   "osr.canceled IS NULL AND "
                   "eos.manual AND "
                   "eos.name IN ($" << param.size() << "::text";
    while (pStatus != status_list.end()) {
        param(*pStatus);
        ++pStatus;
        query << ",$" << param.size() << "::text";
    }
    query << ")";
    Database::Result status_result = ctx.get_conn().exec_params(query.str(), param);
    BOOST_CHECK(status_result.size() == 0);
    ::LibFred::PerformObjectStateRequest(test_domain_id).exec(ctx);
    ctx.commit_transaction();
}

/**
 * test CancelObjectStateRequestIdBad
 * ...
 * calls in test should throw
 */
BOOST_AUTO_TEST_CASE(cancel_object_state_request_id_bad)
{
    ::LibFred::ObjectId not_used_id;
    try {
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        not_used_id = static_cast< ::LibFred::ObjectId >(ctx.get_conn().exec("SELECT (MAX(id)+1000)*2 FROM object_registry")[0][0]);
        ::LibFred::CancelObjectStateRequestId(not_used_id, status_list).exec(ctx);
        ctx.commit_transaction();
        BOOST_CHECK(false);
    }
    catch(const ::LibFred::CancelObjectStateRequestId::Exception &ex) {
        BOOST_CHECK(ex.is_set_object_id_not_found());
        BOOST_CHECK(ex.get_object_id_not_found() == not_used_id);
    }

    ::LibFred::StatusList bad_status_list = status_list;
    try {
        ::LibFred::OperationContextCreator ctx;//new connection to rollback on error
        Database::Result status_result = ctx.get_conn().exec("SELECT name FROM enum_object_states WHERE NOT (manual AND 3=ANY(types))");
        for (::size_t idx = 0; idx < status_result.size(); ++idx) {
            bad_status_list.insert(status_result[idx][0]);
        }
        bad_status_list.insert(std::string("BadStatus") + xmark);
        ::LibFred::CancelObjectStateRequestId(test_domain_id, bad_status_list).exec(ctx);
        ctx.commit_transaction();
        BOOST_CHECK(false);
    }
    catch(const ::LibFred::CancelObjectStateRequestId::Exception &ex) {
        BOOST_CHECK(ex.is_set_state_not_found());
    }
}

BOOST_AUTO_TEST_SUITE_END();//TestCancelObjectStateRequestId
