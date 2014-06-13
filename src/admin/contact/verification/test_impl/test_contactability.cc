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

#include "src/admin/contact/verification/test_impl/test_contactability.h"
#include "src/fredlib/contact/verification/enum_test_status.h"

#include <fredlib/contact.h>

#include "util/util.h"

#include <utility>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/algorithm/string/trim.hpp>

namespace Admin {
namespace ContactVerification {

    FACTORY_MODULE_INIT_DEFI(TestContactability_init)

    const std::string                    TestContactability::letter_message_type_("contact_check_notice");
    const unsigned                       TestContactability::letter_file_type_(8); // contact_check_notice
    const std::string                    TestContactability::letter_comm_type_("registered_letter");
    const Fred::Document::GenerationType TestContactability::letter_doc_type_(Fred::Document::GT_ADMIN_CONTACT_VERIFICATION_CONTACT_CHECK_NOTICE);
    const std::string                    TestContactability::email_template_name_("contact_check_notice");
    const boost::gregorian::days         TestContactability::deadline_interval_(25);
    const std::string                    TestContactability::generated_file_name_("contact_check_notice");

    static inline std::string xml_cdata(const std::string& _input) {
        return "<![CDATA[" + _input + "]]>";
    }

    unsigned long long TestContactability::generate_pdf(
        const std::string&                      _contact_handle,
        unsigned long long                      _contact_history_id,
        const std::string&                      _contact_email,
        const Fred::Messages::PostalAddress&    _contact_address
    ) const {
        std::string xmldata;
        boost::posix_time::ptime now(second_clock::local_time());
        boost::posix_time::ptime deadline(now + deadline_interval_);

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
            "       <termination_date>"+   to_iso_extended_string(deadline.date())+" 00:00:00</termination_date>"
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
            generated_file_name_
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

    Test::TestRunResult TestContactability::run(unsigned long long _history_id) const {
        TestDataProvider<TestContactability> data;
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

        std::set<unsigned long long> mail_ids;
        std::set<unsigned long long> message_ids;

        try {
            mail_ids.insert(
                send_email(
                    contact_data.handle,
                    data.email_,
                    generated_pdf_id)
            );
        } catch(...) {
            error = true;
            error_msg += "failed to send email";
        }

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
            return TestRunResult(Fred::ContactTestStatus::ERROR, error_msg, mail_ids, message_ids );
        } else {
            return TestRunResult(Fred::ContactTestStatus::MANUAL, std::string(), mail_ids, message_ids );
        }
    }

    unsigned long long TestContactability::send_email(
        const std::string&          _contact_handle,
        const std::string&          _contact_email,
        unsigned long long          _attached_pdf_id
    ) const {

        std::map<std::string, std::string> param_values;
        param_values["contact_handle"] = _contact_handle;

        Fred::Mailer::Attachments attachements;
        attachements.push_back(_attached_pdf_id);

        return email_manager_
            ->sendEmail(
                "",
                _contact_email,
                "",
                email_template_name_,
                param_values,
                Fred::Mailer::Handles(),
                attachements,
                ""
        );
    }

    unsigned long long TestContactability::send_letter(
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
