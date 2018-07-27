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

#include "src/backend/public_request/create_authinfo_request_registry_email.hh"
#include "src/backend/public_request/exceptions.hh"
#include "src/libfred/mailer.hh"
#include "src/libfred/object/object_type.hh"
#include "src/libfred/public_request/update_public_request.hh"
#include "src/libfred/registrable_object/contact/info_contact_data.hh"
#include "src/libfred/registrable_object/domain/info_domain_data.hh"
#include "src/libfred/registrable_object/keyset/info_keyset_data.hh"
#include "src/libfred/registrable_object/nsset/info_nsset_data.hh"
#include "src/libfred/registrar/info_registrar_data.hh"
#include "src/util/cfg/config_handler_decl.hh"
#include "src/util/cfg/faked_args.hh"
#include "src/util/corba_wrapper_decl.hh"
#include "src/util/util.hh"

#include "test/setup/fixtures_utils.hh"
#include "test/setup/fixtures.hh"
#include "test/backend/public_request/fixture_common.hh"

#define BOOST_TEST_NO_MAIN

#include <memory>
#include <boost/test/unit_test.hpp>
#include <utility>

BOOST_AUTO_TEST_SUITE(TestPublicRequest)
BOOST_AUTO_TEST_SUITE(RegistryEmail)

class FakeMailer : public LibFred::Mailer::Manager
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
            const std::string&)
    {
        return 0;
    }
    virtual bool checkEmailList(std::string&) const
    {
        return true;
    }
};

struct registry_email_fixture : Test::instantiate_db_template
{
    registry_email_fixture()
    {
        LibFred::OperationContextCreator ctx;
        LibFred::InfoRegistrarData registrar = Test::exec(
                Test::CreateX_factory<::LibFred::CreateRegistrar>()
                    .make(),
                ctx);
        contact = Test::exec(
                Test::CreateX_factory<::LibFred::CreateContact>()
                    .make(registrar.handle)
                    .set_email("someemail@nic.cz"),
                ctx);
        nsset = Test::exec(
                Test::CreateX_factory<::LibFred::CreateNsset>()
                    .make(registrar.handle)
                    .set_tech_contacts(Util::vector_of<std::string>(contact.handle)),
                ctx);
        domain = Test::exec(
                Test::CreateX_factory<::LibFred::CreateDomain>()
                    .make(registrar.handle, contact.handle),
                ctx);
        keyset = Test::exec(
                Test::CreateX_factory<::LibFred::CreateKeyset>()
                    .make(registrar.handle)
                    .set_tech_contacts(Util::vector_of<std::string>(contact.handle)),
                ctx);
        ctx.commit_transaction();

        contact_id = Fred::Backend::PublicRequest::create_authinfo_request_registry_email(
                Fred::Backend::PublicRequest::ObjectType::contact,
                contact.handle,
                Optional<unsigned long long>());
        nsset_id = Fred::Backend::PublicRequest::create_authinfo_request_registry_email(
                Fred::Backend::PublicRequest::ObjectType::nsset,
                nsset.handle,
                Optional<unsigned long long>());
        domain_id = Fred::Backend::PublicRequest::create_authinfo_request_registry_email(
                Fred::Backend::PublicRequest::ObjectType::domain,
                domain.fqdn,
                Optional<unsigned long long>());
        keyset_id = Fred::Backend::PublicRequest::create_authinfo_request_registry_email(
                Fred::Backend::PublicRequest::ObjectType::keyset,
                keyset.handle,
                Optional<unsigned long long>());
    }
    LibFred::InfoContactData contact;
    LibFred::InfoNssetData nsset;
    LibFred::InfoDomainData domain;
    LibFred::InfoKeysetData keyset;
    unsigned long long contact_id;
    unsigned long long nsset_id;
    unsigned long long domain_id;
    unsigned long long keyset_id;
};

BOOST_FIXTURE_TEST_CASE(authinfo_request_to_registry_email, registry_email_fixture)
{
    LibFred::OperationContextCreator ctx;
    const Database::Result request = get_db_public_request(ctx, contact_id, 1, 0);
    BOOST_CHECK_EQUAL(request.size(), 1);
}

BOOST_FIXTURE_TEST_CASE(no_entity_email, Test::instantiate_db_template)
{
    LibFred::OperationContextCreator ctx;
    const LibFred::InfoContactData contact = Test::contact::make(ctx);
    const LibFred::InfoNssetData nsset = Test::nsset::make(ctx);
    const LibFred::InfoDomainData domain = Test::domain::make(ctx);
    const LibFred::InfoKeysetData keyset = Test::keyset::make(ctx);
    ctx.commit_transaction();

    try
    {
        Fred::Backend::PublicRequest::create_authinfo_request_registry_email(
                Fred::Backend::PublicRequest::ObjectType::contact,
                contact.handle,
                Optional<unsigned long long>());
        BOOST_ERROR("exception of no email awaited");
    }
    catch (const Fred::Backend::PublicRequest::NoContactEmail&) { }

    try
    {
        Fred::Backend::PublicRequest::create_authinfo_request_registry_email(
                Fred::Backend::PublicRequest::ObjectType::nsset,
                nsset.handle,
                Optional<unsigned long long>());
        BOOST_ERROR("exception of no email awaited");
    }
    catch (const Fred::Backend::PublicRequest::NoContactEmail&) { }

    try
    {
        Fred::Backend::PublicRequest::create_authinfo_request_registry_email(
                Fred::Backend::PublicRequest::ObjectType::domain,
                domain.fqdn,
                Optional<unsigned long long>());
        BOOST_ERROR("exception of no email awaited");
    }
    catch (const Fred::Backend::PublicRequest::NoContactEmail&) { }

    try
    {
        Fred::Backend::PublicRequest::create_authinfo_request_registry_email(
                Fred::Backend::PublicRequest::ObjectType::keyset,
                keyset.handle,
                Optional<unsigned long long>());
        BOOST_ERROR("exception of no email awaited");
    }
    catch (const Fred::Backend::PublicRequest::NoContactEmail&) { }
}

BOOST_FIXTURE_TEST_CASE(no_object, Test::instantiate_db_template)
{
    BOOST_CHECK_THROW(
            Fred::Backend::PublicRequest::create_authinfo_request_registry_email(
                Fred::Backend::PublicRequest::ObjectType::contact,
                "test handle",
                Optional<unsigned long long>()),
            Fred::Backend::PublicRequest::ObjectNotFound);
}

BOOST_AUTO_TEST_SUITE_END()//TestPublicRequest/RegistryEmail
BOOST_AUTO_TEST_SUITE_END()//TestPublicRequest
