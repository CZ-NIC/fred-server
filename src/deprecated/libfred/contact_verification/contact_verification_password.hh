/*
 * Copyright (C) 2012  CZ.NIC, z.s.p.o.
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
 *  @contact_verification_password.h
 *  header of passwords implementation
 */

#ifndef CONTACT_VERIFICATION_PASSWORD_HH_C663C8EF11554AC2B587074DA95FFCAD
#define CONTACT_VERIFICATION_PASSWORD_HH_C663C8EF11554AC2B587074DA95FFCAD

#include <map>
#include <string>

#include "src/deprecated/libfred/public_request/public_request_impl.hh"

namespace LibFred {
namespace PublicRequest {

class ContactVerificationPassword
{
public:
    typedef std::map<std::string, std::string> MessageData;
    size_t get_password_chunk_length()const;
    ContactVerificationPassword(PublicRequestAuthImpl* _prai_ptr);
    void sendEmailPassword(const std::string& mailTemplate //db table mail_type.name
            );
    void sendLetterPassword( const std::string& custom_tag //tag in template xml params: "pin2",  "pin3"
            , LibFred::Document::GenerationType doc_type //type for document generator
            , const std::string& message_type //for message_archive: "contact_verification_pin2", "contact_verification_pin3"
            , const std::string& comm_type //for message_archive: "letter"
            );
    void sendSmsPassword(const boost::format& sms_template
            , const std::string& message_type //for message_archive: "contact_verification_pin2"
            );
    std::string generateRandomPassword(const size_t _length);
    std::string generateRandomPassword();
    std::string generateAuthInfoPassword();
private:
    MessageData collectMessageData()const;
    PublicRequestAuthImpl* prai_ptr_;
};//class ContactVerificationPassword

//TODO: move this function into more convenient place
ContactVerificationPassword::MessageData& collect_message_data(
    unsigned long long _contact_id,
    Database::Connection &_conn,
    ContactVerificationPassword::MessageData &_data);
}//PublicRequest
}//Fred


#endif
