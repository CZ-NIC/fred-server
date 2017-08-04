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

#ifndef CONTACT_VERIFICATION_TEST_SEND_LETTER_51554788974_
#define CONTACT_VERIFICATION_TEST_SEND_LETTER_51554788974_

#include "src/admin/contact/verification/test_impl/test_interface.h"

#include "src/fredlib/mailer.h"
#include "src/fredlib/messages/messages_impl.h"
#include "src/fredlib/documents.h"

#include <string>
#include <utility>
#include <vector>

#include <memory>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/assign/list_of.hpp>

namespace Admin
{
namespace ContactVerification
{
    FACTORY_MODULE_INIT_DECL(TestSendLetter_init)

    class TestSendLetter
    : public
        Test,
        test_auto_registration<TestSendLetter>
    {
        std::shared_ptr<Fred::Mailer::Manager>    email_manager_;
        std::shared_ptr<Fred::Document::Manager>  document_file_manager_;
        std::shared_ptr<Fred::Messages::Manager>  letter_manager_;

        // snail mail
        const static std::string                    letter_message_type_;
        const static std::string                    letter_comm_type_;
        const static unsigned                       letter_file_type_;
        const static Fred::Document::GenerationType letter_doc_type_;
        const static std::string                    genereted_file_name;

        unsigned long long generate_pdf(
            const std::string&                      _contact_handle,
            unsigned long long                      _contact_history_id,
            const std::string&                      _contact_email,
            const Fred::Messages::PostalAddress&    _contact_address
        ) const;

        unsigned long long send_letter(
            unsigned long long                      _contact_id,
            const std::string&                      _contact_handle,
            unsigned long long                      _contact_history_id,
            const Fred::Messages::PostalAddress&    _contact_address,
            unsigned long long                      _pdf_file_id
        ) const;

        public:
            TestSendLetter& set_document_file_manager(std::shared_ptr<Fred::Document::Manager> _document_file_manager) {
                document_file_manager_ = _document_file_manager;
                return *this;
            }

            TestSendLetter& set_letter_manager(std::shared_ptr<Fred::Messages::Manager> _letter_manager) {
                letter_manager_ = _letter_manager;
                return *this;
            }

            virtual TestRunResult run(unsigned long long _history_id) const;
            static std::string registration_name() { return "send_letter"; }
    };

    template<> struct TestDataProvider<TestSendLetter>
    : TestDataProvider_common,
      _inheritTestRegName<TestSendLetter>
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

        virtual void store_data(const Fred::InfoContactOutput& _data) {
            name_ = boost::algorithm::trim_copy(_data.info_contact_data.name.get_value_or_default());
            email_ = boost::algorithm::trim_copy(_data.info_contact_data.email.get_value_or_default());
            organization_ = boost::algorithm::trim_copy(_data.info_contact_data.organization.get_value_or_default());
            street1_ = boost::algorithm::trim_copy(_data.info_contact_data.place.get_value_or_default().street1);
            street2_ = boost::algorithm::trim_copy(_data.info_contact_data.place.get_value_or_default().street2.get_value_or_default());
            street3_ = boost::algorithm::trim_copy(_data.info_contact_data.place.get_value_or_default().street3.get_value_or_default());
            city_ = boost::algorithm::trim_copy(_data.info_contact_data.place.get_value_or_default().city);
            stateorprovince_ = boost::algorithm::trim_copy(_data.info_contact_data.place.get_value_or_default().stateorprovince.get_value_or_default());
            postalcode_ = boost::algorithm::trim_copy(_data.info_contact_data.place.get_value_or_default().postalcode);
            country_ = boost::algorithm::trim_copy(_data.info_contact_data.place.get_value_or_default().country);
        }

        virtual std::vector<std::string> get_string_data() const {
            return boost::assign::list_of
                (name_)
                (email_)
                (organization_)
                (street1_)(street2_)(street3_)
                (city_)
                (stateorprovince_)
                (postalcode_)
                (country_);
        }
    };
}
}

#endif // #include guard end
