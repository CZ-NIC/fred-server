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

#include "src/public_request/public_request.h"
#include "src/fredlib/object/get_present_object_id.h"

#include "src/fredlib/object/object_type.h"
#include "src/fredlib/contact/info_contact_data.h"
#include "src/fredlib/nsset/info_nsset_data.h"
#include "src/fredlib/registrar/info_registrar_data.h"
#include "src/fredlib/domain/info_domain_data.h"
#include "src/fredlib/keyset/info_keyset_data.h"
#include "src/fredlib/mailer.h"
#include "util/corba_wrapper_decl.h"
#include "util/cfg/config_handler_decl.h"

#include "util/cfg/faked_args.h"
#include "util/util.h"

#include "util/cfg/config_handler_decl.h"

#include "tests/setup/fixtures_utils.h"
#include "tests/setup/fixtures.h"

#define BOOST_TEST_NO_MAIN

#include <boost/shared_ptr.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestPublicRequest)

BOOST_AUTO_TEST_SUITE(TestRegistryEmail) //TODO check if makefile has all 3 files

class FakeMailer : public Fred::Mailer::Manager
{
public:
    virtual ~FakeMailer() {}

    virtual unsigned long long sendEmail(
            const std::string& from,
            const std::string& to,
            const std::string& subject,
            const std::string& mailTemplate,
            const std::map<std::string,std::string>& params,
            const std::vector<std::string>& handles,
            const std::vector<unsigned long long>& attach,
            const std::string& reply_to) throw (Fred::Mailer::NOT_SEND)
    {
        return 0;
    }
    virtual bool checkEmailList(std::string &_email_list) const
    {
        return true;
    }
};

struct registry_email_fixture : Test::Fixture::instantiate_db_template
{
private:
    Fred::OperationContextCreator ctx;

public:
    Fred::InfoContactData contact;
    Fred::InfoNssetData nsset;
    Fred::InfoDomainData domain;
    Fred::InfoKeysetData keyset;
    const std::string reason;
    unsigned long long contact_id;
    unsigned long long nsset_id;
    unsigned long long domain_id;
    unsigned long long keyset_id;

    registry_email_fixture()
    : ctx(),
      reason("some reason")
    {
        Fred::InfoRegistrarData registrar = Test::exec(
                Test::CreateX_factory<Fred::CreateRegistrar>()
                    .make(),
                ctx);
        contact = Test::exec(
                Test::CreateX_factory<Fred::CreateContact>()
                    .make(registrar.handle)
                    .set_email("someemail@nic.cz"),
                ctx);
        nsset = Test::exec(
                Test::CreateX_factory<Fred::CreateNsset>()
                    .make(registrar.handle)
                    .set_tech_contacts(Util::vector_of<std::string>(contact.handle)),
                ctx);
        domain = Test::exec(
                Test::CreateX_factory<Fred::CreateDomain>()
                    .make(registrar.handle, contact.handle),
                ctx);
        keyset = Test::exec(
                Test::CreateX_factory<Fred::CreateKeyset>()
                    .make(registrar.handle)
                    .set_tech_contacts(Util::vector_of<std::string>(contact.handle)),
                ctx);
        ctx.commit_transaction();

        boost::shared_ptr<Fred::Mailer::Manager> mailer_manager(
                new FakeMailer());

        Registry::PublicRequestImpl::PublicRequest pr;
        contact_id = pr.create_authinfo_request_registry_email(
                Fred::Object_Type::contact,
                contact.handle,
                reason,
                Optional<unsigned long long>(),
                mailer_manager);
        nsset_id = pr.create_authinfo_request_registry_email(
                Fred::Object_Type::nsset,
                nsset.handle,
                reason,
                Optional<unsigned long long>(),
                mailer_manager);
        domain_id = pr.create_authinfo_request_registry_email(
                Fred::Object_Type::domain,
                domain.fqdn,
                reason,
                Optional<unsigned long long>(),
                mailer_manager);
        keyset_id = pr.create_authinfo_request_registry_email(
                Fred::Object_Type::keyset,
                keyset.handle,
                reason,
                Optional<unsigned long long>(),
                mailer_manager);
    }
};
// TODO no email entities?
BOOST_FIXTURE_TEST_CASE(authinfo_request_to_registry_email, registry_email_fixture)
{
    Fred::OperationContextCreator ctx;
    Database::Result request = ctx.get_conn().exec_params(
            "SELECT create_request_id, resolve_request_id "
            "FROM public_request "
            "WHERE id=$1::bigint "
              "AND request_type=$2::text "
              "AND status=$3::smallint "
              "AND reason=$4::text "
              "AND email_to_answer IS NULL "
              "AND registrar_id IS NULL ",
            Database::query_param_list(id)(1)(0)(reason));
    BOOST_CHECK(request.size() == 1);
}

BOOST_AUTO_TEST_CASE(no_object)
{
    BOOST_CHECK_THROW(
            Fred::PublicRequest().create_authinfo_request_registry_email(
                Fred::Object_Type::contact,
                "test handle",
                reason,
                Optional<unsigned long long>()),
            Fred::UnknownObject);
    BOOST_CHECK_THROW(
            Fred::PublicRequest().create_authinfo_request_registry_email(
                Fred::Object_Type::nsset,
                "test handle",
                reason,
                Optional<unsigned long long>()),
            Fred::UnknownObject);
    BOOST_CHECK_THROW(
            Fred::PublicRequest().create_authinfo_request_registry_email(
                Fred::Object_Type::domain,
                "test handle",
                reason,
                Optional<unsigned long long>()),
            Fred::UnknownObject);
    BOOST_CHECK_THROW(
            Fred::PublicRequest().create_authinfo_request_registry_email(
                Fred::Object_Type::keyset,
                "test handle",
                reason,
                Optional<unsigned long long>()),
            Fred::UnknownObject);
}

BOOST_AUTO_TEST_SUITE_END() // TestPublicRequest
