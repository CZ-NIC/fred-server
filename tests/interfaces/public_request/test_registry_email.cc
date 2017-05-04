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

#include "src/fredlib/object/object_type.h"
#include "src/fredlib/registrar/info_registrar_data.h"
#include "src/fredlib/contact/info_contact_data.h"
#include "src/fredlib/nsset/info_nsset_data.h"
#include "src/fredlib/domain/info_domain_data.h"
#include "src/fredlib/keyset/info_keyset_data.h"
#include "src/fredlib/mailer.h"
#include "util/corba_wrapper_decl.h"
#include "util/cfg/config_handler_decl.h"
#include "src/fredlib/public_request/update_public_request.h"

#include "util/cfg/faked_args.h"
#include "util/util.h"

#include "util/cfg/config_handler_decl.h"

#include "tests/setup/fixtures_utils.h"
#include "tests/setup/fixtures.h"
#include "tests/interfaces/public_request/fixture_common.h"

#define BOOST_TEST_NO_MAIN

#include <boost/shared_ptr.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestPublicRequest)
BOOST_AUTO_TEST_SUITE(RegistryEmail)

class FakeMailer : public Fred::Mailer::Manager
{
public:
    virtual ~FakeMailer() {}

    virtual unsigned long long sendEmail(
            const std::string&,
            const std::string&,
            const std::string&,
            const std::string&,
            const std::map<std::string,std::string>&,
            const std::vector<std::string>&,
            const std::vector<unsigned long long>&,
            const std::string&) throw (Fred::Mailer::NOT_SEND)
    {
        return 0;
    }
    virtual bool checkEmailList(std::string&) const
    {
        return true;
    }
};

struct registry_email_fixture : Test::Fixture::instantiate_db_template
{
    registry_email_fixture()
    {
        Fred::OperationContextCreator ctx;
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

        Registry::PublicRequestImpl pr("public-request-test");
        contact_id = pr.create_authinfo_request_registry_email(
                Registry::PublicRequestImpl::ObjectType::contact,
                contact.handle,
                Optional<unsigned long long>(),
                mailer_manager);
        nsset_id = pr.create_authinfo_request_registry_email(
                Registry::PublicRequestImpl::ObjectType::nsset,
                nsset.handle,
                Optional<unsigned long long>(),
                mailer_manager);
        domain_id = pr.create_authinfo_request_registry_email(
                Registry::PublicRequestImpl::ObjectType::domain,
                domain.fqdn,
                Optional<unsigned long long>(),
                mailer_manager);
        keyset_id = pr.create_authinfo_request_registry_email(
                Registry::PublicRequestImpl::ObjectType::keyset,
                keyset.handle,
                Optional<unsigned long long>(),
                mailer_manager);
    }
    Fred::InfoContactData contact;
    Fred::InfoNssetData nsset;
    Fred::InfoDomainData domain;
    Fred::InfoKeysetData keyset;
    unsigned long long contact_id;
    unsigned long long nsset_id;
    unsigned long long domain_id;
    unsigned long long keyset_id;
};

BOOST_FIXTURE_TEST_CASE(authinfo_request_to_registry_email, registry_email_fixture)
{
    Fred::OperationContextCreator ctx;
    const Database::Result request = get_db_public_request(ctx, contact_id, 1, 0);
    BOOST_CHECK_EQUAL(request.size(), 1);
}

BOOST_FIXTURE_TEST_CASE(no_entity_email, Test::Fixture::instantiate_db_template)
{
    Fred::OperationContextCreator ctx;
    const Fred::InfoContactData contact = Test::contact::make(ctx);
    const Fred::InfoNssetData nsset = Test::nsset::make(ctx);
    const Fred::InfoDomainData domain = Test::domain::make(ctx);
    const Fred::InfoKeysetData keyset = Test::keyset::make(ctx);
    ctx.commit_transaction();

    boost::shared_ptr<Fred::Mailer::Manager> mailer_manager(new FakeMailer());
    try
    {
        Registry::PublicRequestImpl("public-request-test").create_authinfo_request_registry_email(
                Registry::PublicRequestImpl::ObjectType::contact,
                contact.handle,
                Optional<unsigned long long>(),
                mailer_manager);
        BOOST_ERROR("exception of no email awaited");
    }
    catch (const Registry::PublicRequestImpl::NoContactEmail&) { }

    try
    {
        Registry::PublicRequestImpl("public-request-test").create_authinfo_request_registry_email(
                Registry::PublicRequestImpl::ObjectType::nsset,
                nsset.handle,
                Optional<unsigned long long>(),
                mailer_manager);
        BOOST_ERROR("exception of no email awaited");
    }
    catch (const Registry::PublicRequestImpl::NoContactEmail&) { }

    try
    {
        Registry::PublicRequestImpl("public-request-test").create_authinfo_request_registry_email(
                Registry::PublicRequestImpl::ObjectType::domain,
                domain.fqdn,
                Optional<unsigned long long>(),
                mailer_manager);
        BOOST_ERROR("exception of no email awaited");
    }
    catch (const Registry::PublicRequestImpl::NoContactEmail&) { }

    try
    {
        Registry::PublicRequestImpl("public-request-test").create_authinfo_request_registry_email(
                Registry::PublicRequestImpl::ObjectType::keyset,
                keyset.handle,
                Optional<unsigned long long>(),
                mailer_manager);
        BOOST_ERROR("exception of no email awaited");
    }
    catch (const Registry::PublicRequestImpl::NoContactEmail&) { }
}

BOOST_FIXTURE_TEST_CASE(no_object, Test::Fixture::instantiate_db_template)
{
    boost::shared_ptr<Fred::Mailer::Manager> mailer_manager(new FakeMailer());
    BOOST_CHECK_THROW(
            Registry::PublicRequestImpl("public-request-test").create_authinfo_request_registry_email(
                Registry::PublicRequestImpl::ObjectType::contact,
                "test handle",
                Optional<unsigned long long>(),
                mailer_manager),
            Registry::PublicRequestImpl::ObjectNotFound);
}

BOOST_AUTO_TEST_SUITE_END()//TestPublicRequest/RegistryEmail
BOOST_AUTO_TEST_SUITE_END()//TestPublicRequest
