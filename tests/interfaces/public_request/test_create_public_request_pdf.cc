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

#include "src/fredlib/public_request/create_public_request.h"
#include "src/fredlib/contact/info_contact_data.h"
#include "src/fredlib/documents.h"

#include "tests/setup/fixtures_utils.h"
#include "tests/setup/fixtures.h"
#include "tests/interfaces/public_request/fixture_common.h"

#define BOOST_TEST_NO_MAIN

#include <boost/test/unit_test.hpp>

#include <sstream>

BOOST_AUTO_TEST_SUITE(TestPublicRequest)
BOOST_AUTO_TEST_SUITE(CreatePdf)

class create_pdf_fixture : public Test::Fixture::instantiate_db_template
{
public:
    create_pdf_fixture()
    : contact(Test::contact::make(ctx))
    {
        ctx.commit_transaction();
    }
private:
    Fred::OperationContextCreator ctx;
public:
    Fred::InfoContactData contact;
    Registry::PublicRequestImpl pr;
};

class FakeGenerator : public Fred::Document::Generator
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

class FakeManager : public Fred::Document::Manager
{
private:
    struct UnexpectedCall : std::exception
    {
        const char* what() const throw() { return "method must not be called"; }
    };
public:
    virtual ~FakeManager() {}

    virtual std::auto_ptr<Fred::Document::Generator> createOutputGenerator(
        Fred::Document::GenerationType type,
        std::ostream& output,
        const std::string& lang) const
    {
        if (type != Fred::Document::GT_PUBLIC_REQUEST_PDF)
        {
            throw std::invalid_argument("only public request pdf may be created from current context");
        }
        if ((lang != "en") && (lang != "cs"))
        {
            throw std::invalid_argument("language code not recognized");
        }
        std::auto_ptr<Fred::Document::Generator> generator(new FakeGenerator);
        return generator;
    }

    virtual std::auto_ptr<Fred::Document::Generator> createSavingGenerator(
        Fred::Document::GenerationType, const std::string&, unsigned, const std::string&)const
    {
        throw UnexpectedCall();
    }

    virtual void generateDocument(
        Fred::Document::GenerationType, std::istream&, std::ostream&, const std::string&)const
    {
        throw UnexpectedCall();
    }

    virtual unsigned long long generateDocumentAndSave(
        Fred::Document::GenerationType, std::istream&, const std::string&, unsigned, const std::string&) const
    {
        throw UnexpectedCall();
    }
};

BOOST_FIXTURE_TEST_CASE(create_pdf, create_pdf_fixture)
{
    Fred::OperationContextCreator ctx;
    unsigned long long block_transfer_post = pr.create_block_unblock_request(
            Registry::PublicRequestImpl::ObjectType::contact,
            contact.handle,
            Optional<unsigned long long>(),
            Registry::PublicRequestImpl::ConfirmationMethod::letter_with_authenticated_signature,
            Registry::PublicRequestImpl::LockRequestType::block_transfer);
    ctx.commit_transaction();
    boost::shared_ptr<Fred::Document::Manager> manager(new FakeManager);
    const std::string buffer_value = pr.create_public_request_pdf(
            block_transfer_post,
            Registry::PublicRequestImpl::Language::en,
            manager).value;
    BOOST_MESSAGE(buffer_value);
}

BOOST_FIXTURE_TEST_CASE(no_public_request, create_pdf_fixture)
{
    Fred::OperationContextCreator ctx;
    boost::shared_ptr<Fred::Document::Manager> manager(new FakeManager);
    BOOST_CHECK_THROW(
            pr.create_public_request_pdf(
                123,
                Registry::PublicRequestImpl::Language::en,
                manager),
            Registry::PublicRequestImpl::ObjectNotFound);
}

BOOST_FIXTURE_TEST_CASE(not_a_post_type, create_pdf_fixture)
{
    Fred::OperationContextCreator ctx;
    unsigned long long block_transfer_email = pr.create_block_unblock_request(
            Registry::PublicRequestImpl::ObjectType::contact,
            contact.handle,
            Optional<unsigned long long>(),
            Registry::PublicRequestImpl::ConfirmationMethod::email_with_qualified_certificate,
            Registry::PublicRequestImpl::LockRequestType::block_transfer);
    boost::shared_ptr<Fred::Document::Manager> manager(new FakeManager);
    BOOST_CHECK_THROW(
            pr.create_public_request_pdf(
                block_transfer_email,
                Registry::PublicRequestImpl::Language::en,
                manager),
            Registry::PublicRequestImpl::InvalidPublicRequestType);
}

BOOST_AUTO_TEST_SUITE_END()//TestPublicRequest/CreatePdf
BOOST_AUTO_TEST_SUITE_END()//TestPublicRequest
