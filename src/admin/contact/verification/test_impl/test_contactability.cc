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
#include "src/admin/contact/verification/test_impl/test_utils.h"
#include "src/fredlib/contact/verification/enum_test_status.h"

#include "src/fredlib/contact/info_contact_data.h"

#include "util/util.h"

#include <utility>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/algorithm/string.hpp>

namespace Admin {

    typedef ContactVerificationTestContactability Test;

    const std::string                    Test::letter_message_type_("admin_contact_verification_contact_update_call");
    const unsigned                       Test::letter_file_type_(8); // admin_contact_verification_contact_update_call
    const std::string                    Test::letter_comm_type_("registered_letter");
    const Fred::Document::GenerationType Test::letter_doc_type_(Fred::Document::GT_ADMIN_CONTACT_VERIFICATION_CONTACT_UPDATE_CALL);
    const std::string                    Test::email_template_name_("contact_update_call");
    const boost::gregorian::days         Test::deadline_interval_(15);

    static inline string xml_cdata(const string& _input) {
        return "<![CDATA[" + _input + "]]>";
    }

    unsigned long long ContactVerificationTestContactability::generate_pdf(
        const std::string&                      _contact_handle,
        unsigned long                           _contact_history_id,
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
            "admin_contact_verification_contact_update_call"
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

    ContactVerificationTest::T_run_result ContactVerificationTestContactability::run(long _history_id) const {
        /* TODO this is only temporary hack before new version of InfoContactHistory is available
         * see ticket #9544
         */
        Fred::InfoContactData contact_data = Admin::Utils::get_contact_data(_history_id);
        std::string contact_email = boost::algorithm::trim_copy(static_cast<std::string>(contact_data.email));

        Fred::Messages::PostalAddress address;
        address.name    = (contact_data.name.isnull())              ? "" : static_cast<std::string>(contact_data.name);
        address.org     = (contact_data.organization.isnull())      ? "" : static_cast<std::string>(contact_data.organization);
        address.street1 = (contact_data.street1.isnull())           ? "" : static_cast<std::string>(contact_data.street1);
        address.street2 = (contact_data.street2.isnull())           ? "" : static_cast<std::string>(contact_data.street2);
        address.street3 = (contact_data.street3.isnull())           ? "" : static_cast<std::string>(contact_data.street3);
        address.city    = (contact_data.city.isnull())              ? "" : static_cast<std::string>(contact_data.city);
        address.state   = (contact_data.stateorprovince.isnull())   ? "" : static_cast<std::string>(contact_data.stateorprovince);
        address.code    = (contact_data.postalcode.isnull())        ? "" : static_cast<std::string>(contact_data.postalcode);
        address.country = (contact_data.country.isnull())           ? "" : static_cast<std::string>(contact_data.country);


        unsigned long long generated_pdf_id;

        try {
            generated_pdf_id = generate_pdf(
                contact_data.handle,
                contact_data.historyid,
                contact_email,
                address
                );
        } catch(...) {
            return T_run_result (Fred::ContactTestStatus::ERROR, std::string("failed to generate pdf file") );
        }

        bool error = false;
        std::string error_msg;

        try {
            send_email(
                contact_data.handle,
                contact_email,
                generated_pdf_id);
        } catch(...) {
            error = true;
            error_msg += "failed to send email";
        }

        try {
            send_letter(
                /* TODO tady ma bejt contact_id. bude to po zamergovani novych fredlib operaci z mastera */
                10,
                contact_data.handle,
                contact_data.historyid,
                address,
                generated_pdf_id);
        } catch(...) {
            error = true;
            error_msg += "failed to send letter";
        }

        if(error) {
            return T_run_result (Fred::ContactTestStatus::ERROR, error_msg );
        } else {
            return T_run_result (Fred::ContactTestStatus::MANUAL, string() );
        }
    }

    void ContactVerificationTestContactability::send_email(
        const std::string&          _contact_handle,
        const std::string&          _contact_email,
        unsigned long long          _attached_pdf_id
    ) const {

        std::map<std::string, std::string> param_values;
        param_values["contact_handle"] = _contact_handle;

        Fred::Mailer::Attachments attachements;
        attachements.push_back(_attached_pdf_id);

        unsigned long long mid = email_manager_->sendEmail(
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

    void ContactVerificationTestContactability::send_letter(
        unsigned long                           _contact_id,
        const std::string&                      _contact_handle,
        unsigned long                           _contact_history_id,
        const Fred::Messages::PostalAddress&    _contact_address,
        unsigned long long                      _pdf_file_id
    ) const {

        unsigned long long message_id = letter_manager_
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
