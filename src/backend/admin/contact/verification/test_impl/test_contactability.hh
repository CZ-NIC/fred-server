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

/**
 *  @file
 *  test if contact is reachable (by any means available)
 */

#ifndef TEST_CONTACTABILITY_HH_E987A2533FD741C9B50057E36C3284C8
#define TEST_CONTACTABILITY_HH_E987A2533FD741C9B50057E36C3284C8

#include "src/backend/admin/contact/verification/test_impl/test_interface.hh"

#include "src/deprecated/libfred/documents.hh"
#include "libfred/mailer.hh"
#include "src/deprecated/libfred/messages/messages_impl.hh"

#include <string>
#include <utility>
#include <vector>

#include <boost/algorithm/string/trim.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <memory>

namespace Fred {
namespace Backend {
namespace Admin {
namespace Contact {
namespace Verification {

FACTORY_MODULE_INIT_DECL(TestContactability_init)

class TestContactability
    : public Test,
      test_auto_registration<TestContactability>
{
    std::shared_ptr<LibFred::Mailer::Manager>    email_manager_;
    std::shared_ptr<LibFred::Document::Manager>  document_file_manager_;
    std::shared_ptr<LibFred::Messages::Manager>  letter_manager_;

    // snail mail
    const static std::string letter_message_type_;
    const static std::string letter_comm_type_;
    const static unsigned letter_file_type_;
    const static LibFred::Document::GenerationType letter_doc_type_;
    const static std::string generated_file_name_;

    const static boost::gregorian::days deadline_interval_;

    const static std::string email_template_name_;

    unsigned long long generate_pdf(
            const std::string&                      _contact_handle,
            unsigned long long _contact_history_id,
            const std::string&                      _contact_email,
            const LibFred::Messages::PostalAddress&    _contact_address) const;

    unsigned long long send_email(
            const std::string&          _contact_handle,
            const std::string&          _contact_email,
            unsigned long long _attached_pdf_id) const;

    unsigned long long send_letter(
            unsigned long long _contact_id,
            const std::string&                      _contact_handle,
            unsigned long long _contact_history_id,
            const LibFred::Messages::PostalAddress&    _contact_address,
            unsigned long long _pdf_file_id) const;


public:
    TestContactability& set_email_manager(std::shared_ptr<LibFred::Mailer::Manager> _email_manager)
    {
        email_manager_ = _email_manager;
        return *this;
    }

    TestContactability& set_document_file_manager(
            std::shared_ptr<LibFred::Document::Manager> _document_file_manager)
    {
        document_file_manager_ = _document_file_manager;
        return *this;
    }

    TestContactability& set_letter_manager(std::shared_ptr<LibFred::Messages::Manager> _letter_manager)
    {
        letter_manager_ = _letter_manager;
        return *this;
    }

    virtual TestRunResult run(unsigned long long _history_id) const;

    static std::string registration_name()
    {
        return "contactability";
    }

};

template <>
struct TestDataProvider<TestContactability>
    : TestDataProvider_common,
      _inheritTestRegName<TestContactability>
{
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

    virtual void store_data(const LibFred::InfoContactOutput& _data)
    {
        name_ = boost::algorithm::trim_copy(_data.info_contact_data.name.get_value_or_default());
        email_ = boost::algorithm::trim_copy(_data.info_contact_data.email.get_value_or_default());
        organization_ =
                boost::algorithm::trim_copy(
                        _data.info_contact_data.organization.get_value_or_default());
        street1_ = boost::algorithm::trim_copy(_data.info_contact_data.place.get_value_or_default().street1);
        street2_ =
                boost::algorithm::trim_copy(
                        _data.info_contact_data.place.get_value_or_default().street2.get_value_or_default());
        street3_ =
                boost::algorithm::trim_copy(
                        _data.info_contact_data.place.get_value_or_default().street3.get_value_or_default());
        city_ = boost::algorithm::trim_copy(_data.info_contact_data.place.get_value_or_default().city);
        stateorprovince_ =
                boost::algorithm::trim_copy(
                        _data.info_contact_data.place.get_value_or_default().stateorprovince.get_value_or_default());
        postalcode_ =
                boost::algorithm::trim_copy(
                        _data.info_contact_data.place.get_value_or_default().postalcode);
        country_ = boost::algorithm::trim_copy(_data.info_contact_data.place.get_value_or_default().country);
    }

    virtual std::vector<std::string> get_string_data() const
    {
        return boost::assign::list_of
                // clang-format off
                (name_)
                (email_)
                (organization_)
                (street1_)(street2_)(street3_)
                (city_)
                (stateorprovince_)
                (postalcode_)
                (country_);
                // clang-format on
    }

};

} // namespace Fred::Backend::Admin::Contact::Verification
} // namespace Fred::Backend::Admin::Contact
} // namespace Fred::Backend::Admin
} // namespace Fred::Backend
} // namespace Fred

#endif
