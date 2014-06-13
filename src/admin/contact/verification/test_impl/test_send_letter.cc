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

#include "src/admin/contact/verification/test_impl/test_send_letter.h"
#include "src/fredlib/contact/verification/enum_test_status.h"

#include <fredlib/contact.h>

#include "util/util.h"

#include <utility>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/algorithm/string/trim.hpp>

namespace Admin {
namespace ContactVerification {

    FACTORY_MODULE_INIT_DEFI(TestSendLetter_init)

    const std::string                    TestSendLetter::letter_message_type_("contact_check_thank_you");
    const unsigned                       TestSendLetter::letter_file_type_(9); // contact_check_thank_you
    const std::string                    TestSendLetter::letter_comm_type_("registered_letter");   // to enable manual "approval/rejection"
    const Fred::Document::GenerationType TestSendLetter::letter_doc_type_(Fred::Document::GT_ADMIN_CONTACT_VERIFICATION_CONTACT_CHECK_THANK_YOU);
    const std::string                    TestSendLetter::genereted_file_name(TestSendLetter::letter_message_type_);

    static inline std::string xml_cdata(const std::string& _input) {
        return "<![CDATA[" + _input + "]]>";
    }

    unsigned long long TestSendLetter::generate_pdf(
        const std::string&                      _contact_handle,
        unsigned long long                      _contact_history_id,
        const std::string&                      _contact_email,
        const Fred::Messages::PostalAddress&    _contact_address
    ) const {
        std::string xmldata;
        boost::posix_time::ptime now(second_clock::local_time());
        //boost::posix_time::ptime deadline(now + deadline_interval_);

        xmldata +=
            "<?xml version=\"1.0\"?>"
            "<message>"
            "   <holder>"
            "       <name>"+           xml_cdata(_contact_address.name) +"</name>"
            "       <email>"+          xml_cdata(_contact_email) +"</email>"
            "       <organization>"+   xml_cdata(_contact_address.org) +"</organization>"
            "       <street>"+         xml_cdata(
                                        _contact_address.street1 + " " +
                                        _contact_address.street2 + " " +
                                        _contact_address.street3
                                    ) +
            "       </street>"
            "       <city>"+           xml_cdata(_contact_address.city) +"</city>"
            "       <postal_code>"+    xml_cdata(_contact_address.code) +"</postal_code>"
            "       <country>"+        xml_cdata(_contact_address.country) +"</country>"
            "       <actual_date>"+        to_iso_extended_string(now.date())+"</actual_date>"
            //"       <termination_date>"+   to_iso_extended_string(deadline.date())+" 00:00:00</termination_date>"
            "       <handle>"+         xml_cdata(_contact_handle) +"</handle>"
            "       <invalid>"
            "           <item><lang code=\"en\">name and family name</lang><lang code=\"cs\">jméno a příjmení</lang></item>"
            "           <item><lang code=\"en\">postal address</lang><lang code=\"cs\">poštovní adresa</lang></item>"
            "       </invalid>"
            "   </holder>"
            "</message>";

        std::stringstream xmldata_stream;
        xmldata_stream << xmldata;


        std::string filename(
            genereted_file_name
            + boost::lexical_cast<std::string>(_contact_history_id)
            + ".pdf");

        return document_file_manager_
            ->generateDocumentAndSave(
                letter_doc_type_,
                xmldata_stream,
                filename,
                letter_file_type_,
                "");
    }

    Test::TestRunResult TestSendLetter::run(unsigned long long _history_id) const {
        TestDataProvider<TestSendLetter> data;
        data.init_data(_history_id);

        Fred::Messages::PostalAddress address;
        address.name    = data.name_;
        address.org     = data.organization_;
        address.street1 = data.street1_;
        address.street2 = data.street2_;
        address.street3 = data.street3_;
        address.city    = data.city_;
        address.state   = data.stateorprovince_;
        address.code    = data.postalcode_;
        address.country = data.country_;


        unsigned long long generated_pdf_id;

        Fred::OperationContext ctx;
        Fred::InfoContactData contact_data = Fred::InfoContactHistoryByHistoryid(_history_id)
            .exec(ctx)
                .info_contact_data;

        try {
            generated_pdf_id = generate_pdf(
                contact_data.handle,
                contact_data.historyid,
                data.email_,
                address
                );
        } catch(...) {
            return TestRunResult (Fred::ContactTestStatus::ERROR, std::string("failed to generate pdf file") );
        }

        bool error = false;
        std::string error_msg;

        std::set<unsigned long long> message_ids;

        try {
            message_ids.insert(
                send_letter(
                    contact_data.id,
                    contact_data.handle,
                    contact_data.historyid,
                    address,
                    generated_pdf_id)
            );
        } catch(...) {
            error = true;
            error_msg += "failed to send letter";
        }

        if(error) {
            return TestRunResult(Fred::ContactTestStatus::ERROR, error_msg, std::set<unsigned long long>(), message_ids );
        } else {
            return TestRunResult(Fred::ContactTestStatus::MANUAL, std::string(), std::set<unsigned long long>(), message_ids );
        }
    }

    unsigned long long TestSendLetter::send_letter(
        unsigned long long                      _contact_id,
        const std::string&                      _contact_handle,
        unsigned long long                      _contact_history_id,
        const Fred::Messages::PostalAddress&    _contact_address,
        unsigned long long                      _pdf_file_id
    ) const {

        return letter_manager_
            ->save_letter_to_send(
                _contact_handle.c_str(),
                _contact_address,
                _pdf_file_id,
                letter_message_type_.c_str(),
                _contact_id,
                _contact_history_id,
                letter_comm_type_.c_str() );
    }
}
}
