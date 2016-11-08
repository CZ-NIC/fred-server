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
// #include "src/fredlib/object_state/perform_object_state_request.h"
// #include "src/fredlib/object/object_type.h"

#include "tests/setup/fixtures_utils.h"
#include "tests/setup/fixtures.h"
#include "tests/interfaces/public_request/fixture_common.h"

#define BOOST_TEST_NO_MAIN

#include <boost/test/unit_test.hpp>

#include <ostream>

BOOST_AUTO_TEST_SUITE(TestPublicRequest)

BOOST_AUTO_TEST_SUITE(TestCreatePdf)

struct create_pdf_fixture : Test::Fixture::instantiate_db_template
{
private:
    Fred::OperationContextCreator ctx;

public:
    Fred::InfoContactData contact;
    Registry::PublicRequestImpl::PublicRequest pr;

    create_pdf_fixture()
    : contact(Test::contact::make(ctx))
    {
        ctx.commit_transaction();
    }
};

class FakeGenerator : public Fred::Document::Generator
{
    std::ostream input;
public:
    FakeGenerator()
    : input(std::cout.rdbuf()) {}

    virtual ~FakeGenerator() {}

    virtual std::ostream& getInput()
    {
        return input;
    }

    virtual unsigned long long closeInput()
    {
        input.flush();
        return 0;
    }
};

class FakeManager : public Fred::Document::Manager
{
    struct UnexpectedCall : std::exception
    {
        virtual const char* what() const throw()
        { return "method must not be called"; }
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
        if (lang != "en" && lang != "cs")
        {
            throw std::invalid_argument("language code not recognized");
        }
        std::auto_ptr<Fred::Document::Generator> generator(new FakeGenerator);
        return generator;
    }

    virtual std::auto_ptr<Fred::Document::Generator> createSavingGenerator(
        Fred::Document::GenerationType type,
        const std::string& filename,
        unsigned filetype,
        const std::string& lang) const
    { throw UnexpectedCall(); }

    virtual void generateDocument(
        Fred::Document::GenerationType type,
        std::istream& input,
        std::ostream& output,
        const std::string& lang) const
    { throw UnexpectedCall(); }

    virtual unsigned long long generateDocumentAndSave(
        Fred::Document::GenerationType type,
        std::istream& input,
        const std::string& name,
        unsigned filetype,
        const std::string& lang) const
    { throw UnexpectedCall(); }
};

BOOST_FIXTURE_TEST_CASE(create_pdf, create_pdf_fixture)
{
    Fred::OperationContextCreator ctx;
    unsigned long long block_transfer_post = pr.create_block_unblock_request(
            Fred::Object_Type::contact,
            contact.handle,
            Optional<unsigned long long>(),
            Registry::PublicRequestImpl::LETTER_WITH_AUTHENTICATED_SIGNATURE,
            Registry::PublicRequestImpl::BLOCK_TRANSFER);
    boost::shared_ptr<Fred::Document::Manager> manager(new FakeManager);
    ctx.commit_transaction();
    const std::string buffer_value = pr.create_public_request_pdf(
            block_transfer_post,
            Registry::PublicRequestImpl::EN,
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
                Registry::PublicRequestImpl::EN,
                manager),
            Registry::PublicRequestImpl::ObjectNotFound);
}

BOOST_FIXTURE_TEST_CASE(not_a_post_type, create_pdf_fixture)
{
    Fred::OperationContextCreator ctx;
    unsigned long long block_transfer_email = pr.create_block_unblock_request(
            Fred::Object_Type::contact,
            contact.handle,
            Optional<unsigned long long>(),
            Registry::PublicRequestImpl::EMAIL_WITH_QUALIFIED_CERTIFICATE,
            Registry::PublicRequestImpl::BLOCK_TRANSFER);
    boost::shared_ptr<Fred::Document::Manager> manager(new FakeManager);
    BOOST_CHECK_THROW(
            pr.create_public_request_pdf(
                block_transfer_email,
                Registry::PublicRequestImpl::EN,
                manager),
            Registry::PublicRequestImpl::InvalidPublicRequestType);
}

BOOST_AUTO_TEST_SUITE_END() // TestCreatePdf

BOOST_AUTO_TEST_SUITE_END() // TestPublicRequest
