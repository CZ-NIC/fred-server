/*
 * Copyright (C) 2014-2022  CZ.NIC, z. s. p. o.
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

#ifndef TEST_SEND_LETTER_HH_8ECCCA2C52214DB2B5F9C4A317B06B9D
#define TEST_SEND_LETTER_HH_8ECCCA2C52214DB2B5F9C4A317B06B9D

#include "src/backend/admin/contact/verification/test_impl/test_interface.hh"

#include "src/deprecated/libfred/documents.hh"
#include "src/deprecated/libfred/messages/messages_impl.hh"

#include "libfred/mailer.hh"

#include <boost/algorithm/string/trim.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>


namespace Fred {
namespace Backend {
namespace Admin {
namespace Contact {
namespace Verification {

class TestSendLetter : public Test
{
public:
    TestSendLetter& set_document_file_manager(
            std::shared_ptr<LibFred::Document::Manager> _document_file_manager);
    TestSendLetter& set_letter_manager(std::shared_ptr<LibFred::Messages::Manager> _letter_manager);
    TestRunResult run(unsigned long long _history_id) const override;
private:
    unsigned long long generate_pdf(
            const std::string& _contact_handle,
            unsigned long long _contact_history_id,
            const std::string& _contact_email,
            const LibFred::Messages::PostalAddress& _contact_address) const;

    unsigned long long send_letter(
            unsigned long long _contact_id,
            const std::string& _contact_handle,
            unsigned long long _contact_history_id,
            const LibFred::Messages::PostalAddress& _contact_address,
            unsigned long long _pdf_file_id) const;

    std::shared_ptr<LibFred::Mailer::Manager> email_manager_;
    std::shared_ptr<LibFred::Document::Manager> document_file_manager_;
    std::shared_ptr<LibFred::Messages::Manager> letter_manager_;

    // snail mail
    static const std::string letter_message_type_;
    static const std::string letter_comm_type_;
    static const unsigned letter_file_type_;
    static const LibFred::Document::GenerationType letter_doc_type_;
    static const std::string genereted_file_name;
};

template <>
std::string test_name<TestSendLetter>();

template <>
struct TestDataProvider<TestSendLetter> : TestDataProvider_common
{
    void store_data(const LibFred::InfoContactOutput& _data) override;
    virtual std::vector<std::string> get_string_data() const override;

    std::string name_;
    std::string email_;
    std::string organization_;
    std::string street1_;
    std::string street2_;
    std::string street3_;
    std::string city_;
    std::string stateorprovince_;
    std::string postalcode_;
    std::string country_;
};

} // namespace Fred::Backend::Admin::Contact::Verification
} // namespace Fred::Backend::Admin::Contact
} // namespace Fred::Backend::Admin
} // namespace Fred::Backend
} // namespace Fred

#endif//TEST_SEND_LETTER_HH_8ECCCA2C52214DB2B5F9C4A317B06B9D
