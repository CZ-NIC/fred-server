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

#include "src/backend/public_request/public_request.hh"

#include "src/libfred/public_request/create_public_request.hh"
#include "src/libfred/registrable_object/contact/info_contact_data.hh"
#include "src/libfred/documents.hh"

#include "test/setup/fixtures_utils.hh"
#include "test/setup/fixtures.hh"
#include "test/backend/public_request/fixture_common.hh"

#define BOOST_TEST_NO_MAIN

#include <boost/test/unit_test.hpp>

#include <sstream>
#include <utility>

BOOST_AUTO_TEST_SUITE(TestPublicRequest)
BOOST_AUTO_TEST_SUITE(CreatePdf)

class create_pdf_fixture : public Test::instantiate_db_template
{
public:
    create_pdf_fixture()
        : contact(Test::contact::make(ctx)),
          pr("public-request-test")
    {
        ctx.commit_transaction();
    }
private:
    ::LibFred::OperationContextCreator ctx;
public:
    ::LibFred::InfoContactData contact;
    Registry::PublicRequestImpl pr;
};

class FakeGenerator : public ::LibFred::Document::Generator
{
public:
    FakeGenerator() {}

    virtual ~FakeGenerator() {}

    virtual std::ostream& getInput()
    {
        return buffer;
    }

    virtual unsigned long long closeInput()
    {
        buffer.flush();
        return 0;
    }
private:
    std::ostringstream buffer;
};

class FakeManager : public ::LibFred::Document::Manager
{
private:
    struct UnexpectedCall : std::exception
    {
        const char* what() const noexcept { return "method must not be called"; }
    };
public:
    virtual ~FakeManager() {}

    virtual std::unique_ptr<::LibFred::Document::Generator> createOutputGenerator(
        ::LibFred::Document::GenerationType type,
        std::ostream& output,
        const std::string& lang) const
    {
        if (type != ::LibFred::Document::GT_PUBLIC_REQUEST_PDF)
        {
            throw std::invalid_argument("only public request pdf may be created from current context");
        }
        if ((lang != "en") && (lang != "cs"))
        {
            throw std::invalid_argument("language code not recognized");
        }
        std::unique_ptr<::LibFred::Document::Generator> generator(new FakeGenerator);
        return generator;
    }

    virtual std::unique_ptr<::LibFred::Document::Generator> createSavingGenerator(
        ::LibFred::Document::GenerationType, const std::string&, unsigned, const std::string&)const
    {
        throw UnexpectedCall();
    }

    virtual void generateDocument(
        ::LibFred::Document::GenerationType, std::istream&, std::ostream&, const std::string&)const
    {
        throw UnexpectedCall();
    }

    virtual unsigned long long generateDocumentAndSave(
        ::LibFred::Document::GenerationType, std::istream&, const std::string&, unsigned, const std::string&) const
    {
        throw UnexpectedCall();
    }
};

BOOST_FIXTURE_TEST_CASE(create_pdf, create_pdf_fixture)
{
    ::LibFred::OperationContextCreator ctx;
    unsigned long long block_transfer_post = pr.create_block_unblock_request(
            Registry::PublicRequestImpl::ObjectType::contact,
            contact.handle,
            Optional<unsigned long long>(),
            Registry::PublicRequestImpl::ConfirmedBy::letter,
            Registry::PublicRequestImpl::LockRequestType::block_transfer);
    ctx.commit_transaction();
    std::shared_ptr<::LibFred::Document::Manager> manager(new FakeManager);
    const std::string buffer_value = pr.create_public_request_pdf(
            block_transfer_post,
            Registry::PublicRequestImpl::Language::en,
            manager).data;
    BOOST_TEST_MESSAGE(buffer_value);
}

BOOST_FIXTURE_TEST_CASE(no_public_request, create_pdf_fixture)
{
    ::LibFred::OperationContextCreator ctx;
    std::shared_ptr<::LibFred::Document::Manager> manager(new FakeManager);
    BOOST_CHECK_THROW(
            pr.create_public_request_pdf(
                123,
                Registry::PublicRequestImpl::Language::en,
                manager),
            Registry::PublicRequestImpl::ObjectNotFound);
}

BOOST_FIXTURE_TEST_CASE(not_a_post_type, create_pdf_fixture)
{
    ::LibFred::OperationContextCreator ctx;
    unsigned long long block_transfer_email = pr.create_block_unblock_request(
            Registry::PublicRequestImpl::ObjectType::contact,
            contact.handle,
            Optional<unsigned long long>(),
            Registry::PublicRequestImpl::ConfirmedBy::email,
            Registry::PublicRequestImpl::LockRequestType::block_transfer);
    std::shared_ptr<::LibFred::Document::Manager> manager(new FakeManager);
    BOOST_CHECK_THROW(
            pr.create_public_request_pdf(
                block_transfer_email,
                Registry::PublicRequestImpl::Language::en,
                manager),
            Registry::PublicRequestImpl::InvalidPublicRequestType);
}

BOOST_AUTO_TEST_SUITE_END()//TestPublicRequest/CreatePdf
BOOST_AUTO_TEST_SUITE_END()//TestPublicRequest
