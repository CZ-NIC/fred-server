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

#include "src/backend/admin/contact/verification/test_impl/test_send_letter.hh"
#include "src/backend/admin/contact/verification/test_impl/util.hh"

#include "libfred/opcontext.hh"
#include "libfred/registrable_object/contact/info_contact.hh"
#include "libfred/registrable_object/contact/verification/enum_test_status.hh"

#include <boost/algorithm/string/trim.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <utility>

namespace Fred {
namespace Backend {
namespace Admin {
namespace Contact {
namespace Verification {

const std::string TestSendLetter::letter_message_type_("contact_check_thank_you");
const unsigned TestSendLetter::letter_file_type_(9); // contact_check_thank_you
const std::string TestSendLetter::letter_comm_type_("registered_letter"); // to enable manual "approval/rejection"
const LibFred::Document::GenerationType TestSendLetter::letter_doc_type_(
        LibFred::Document::GT_ADMIN_CONTACT_VERIFICATION_CONTACT_CHECK_THANK_YOU);
const std::string TestSendLetter::genereted_file_name(TestSendLetter::letter_message_type_);

namespace {

std::string xml_cdata(const std::string& _input)
{
    return "<![CDATA[" + _input + "]]>";
}

}//namespace Fred::Backend::Admin::Contact::Verification::{anonymous}

TestSendLetter& TestSendLetter::set_document_file_manager(
        std::shared_ptr<LibFred::Document::Manager> _document_file_manager)
{
    document_file_manager_ = _document_file_manager;
    return *this;
}

TestSendLetter& TestSendLetter::set_letter_manager(std::shared_ptr<LibFred::Messages::Manager> _letter_manager)
{
    letter_manager_ = _letter_manager;
    return *this;
}

unsigned long long TestSendLetter::generate_pdf(
        const std::string& _contact_handle,
        unsigned long long _contact_history_id,
        const std::string& _contact_email,
        const LibFred::Messages::PostalAddress& _contact_address) const
{
    std::string xmldata;
    boost::posix_time::ptime now(second_clock::local_time());
    // boost::posix_time::ptime deadline(now + deadline_interval_);

    xmldata +=
            // clang-format off
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
            // clang-format on

    std::stringstream xmldata_stream;
    xmldata_stream << xmldata;

    std::string filename(
            genereted_file_name +
            boost::lexical_cast<std::string>(_contact_history_id) +
            ".pdf");

    return document_file_manager_->generateDocumentAndSave(
            letter_doc_type_,
            xmldata_stream,
            filename,
            letter_file_type_,
            "");
}

Test::TestRunResult TestSendLetter::run(unsigned long long _history_id) const
{
    TestDataProvider<TestSendLetter> data;
    data.init_data(_history_id);

    LibFred::Messages::PostalAddress address;
    address.name    = data.name_;
    address.org     = data.organization_;
    address.street1 = data.street1_;
    address.street2 = data.street2_;
    address.street3 = data.street3_;
    address.city    = data.city_;
    address.state   = data.stateorprovince_;
    address.code    = data.postalcode_;

    LibFred::OperationContextCreator ctx;

    try
    {
        address.country = Util::get_country_name(ctx, data.country_);
    }
    catch (...)
    {
        return TestRunResult{
                LibFred::ContactTestStatus::ERROR,
                "failed to get country name"};
    }

    unsigned long long generated_pdf_id;

    LibFred::InfoContactData contact_data = LibFred::InfoContactHistoryByHistoryid(_history_id)
                                            .exec(ctx)
                                            .info_contact_data;

    try
    {
        generated_pdf_id = generate_pdf(
                contact_data.handle,
                contact_data.historyid,
                data.email_,
                address);
    }
    catch (...)
    {
        return TestRunResult{
                LibFred::ContactTestStatus::ERROR,
                "failed to generate pdf file"};
    }

    bool error = false;
    std::string error_msg;

    std::set<unsigned long long> message_ids;

    try
    {
        message_ids.insert(
                send_letter(
                        contact_data.id,
                        contact_data.handle,
                        contact_data.historyid,
                        address,
                        generated_pdf_id));
    }
    catch (...)
    {
        error = true;
        error_msg += "failed to send letter";
    }

    if (error)
    {
        return TestRunResult{
                LibFred::ContactTestStatus::ERROR,
                error_msg,
                std::set<unsigned long long>(),
                message_ids};
    }
    return TestRunResult{
            LibFred::ContactTestStatus::MANUAL,
            std::string(),
            std::set<unsigned long long>(),
            message_ids};
}

unsigned long long TestSendLetter::send_letter(
        unsigned long long _contact_id,
        const std::string& _contact_handle,
        unsigned long long _contact_history_id,
        const LibFred::Messages::PostalAddress& _contact_address,
        unsigned long long _pdf_file_id) const
{
    return letter_manager_->save_letter_to_send(
            _contact_handle.c_str(),
            _contact_address,
            _pdf_file_id,
            letter_message_type_.c_str(),
            _contact_id,
            _contact_history_id,
            letter_comm_type_,
            false);
}

void TestDataProvider<TestSendLetter>::store_data(const LibFred::InfoContactOutput& _data)
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

std::vector<std::string> TestDataProvider<TestSendLetter>::get_string_data() const
{
    return { name_,
             email_,
             organization_,
             street1_,
             street2_,
             street3_,
             city_,
            stateorprovince_,
            postalcode_,
            country_ };
}

template <>
std::string test_name<TestSendLetter>()
{
    return "send_letter";
}

} // namespace Fred::Backend::Admin::Contact::Verification
} // namespace Fred::Backend::Admin::Contact
} // namespace Fred::Backend::Admin
} // namespace Fred::Backend
} // namespace Fred
