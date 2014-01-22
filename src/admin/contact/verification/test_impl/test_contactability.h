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

#ifndef CONTACT_VERIFICATION_TEST_CONTACTABILITY_42567645497_
#define CONTACT_VERIFICATION_TEST_CONTACTABILITY_42567645497_

#include "src/admin/contact/verification/test_impl/test_interface.h"

#include "src/fredlib/mailer.h"
#include "src/fredlib/messages/messages_impl.h"
#include "src/fredlib/documents.h"

#include <string>
#include <utility>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

namespace Admin
{
    class ContactVerificationTestContactability: public ContactVerificationTest {
        boost::shared_ptr<Fred::Mailer::Manager>    email_manager_;
        boost::shared_ptr<Fred::Document::Manager>  document_file_manager_;
        boost::shared_ptr<Fred::Messages::Manager>  letter_manager_;

        // snail mail
        const static std::string                    letter_message_type_;
        const static std::string                    letter_comm_type_;
        const static unsigned                       letter_file_type_;
        const static Fred::Document::GenerationType letter_doc_type_;

        const static boost::gregorian::days         deadline_interval_;

        const static std::string                    email_template_name_;

        unsigned long long generate_pdf(
            const std::string&                      _contact_handle,
            unsigned long                           _contact_history_id,
            const std::string&                      _contact_email,
            const Fred::Messages::PostalAddress&    _contact_address
        ) const;

        void send_email(
            const std::string&          _contact_handle,
            const std::string&          _contact_email,
            unsigned long long          _attached_pdf_id
        ) const;

        void send_letter(
            unsigned long                           _contact_id,
            const std::string&                      _contact_handle,
            unsigned long                           _contact_history_id,
            const Fred::Messages::PostalAddress&    _contact_address,
            unsigned long long                      _pdf_file_id
        ) const;

        public:
            ContactVerificationTestContactability(
                boost::shared_ptr<Fred::Mailer::Manager>    _email_manager,
                boost::shared_ptr<Fred::Document::Manager>  _document_file_manager,
                boost::shared_ptr<Fred::Messages::Manager>  _letter_manager
            ) :
                email_manager_(_email_manager),
                document_file_manager_(_document_file_manager),
                letter_manager_(_letter_manager)
            { }

            virtual ContactVerificationTest::T_run_result run(long _history_id) const;
            virtual std::string get_name() const { return "contactability"; }
    };
}


#endif // #include guard end
