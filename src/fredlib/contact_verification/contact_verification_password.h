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

#ifndef CONTACT_VERIFICATION_PASSWORD_H_
#define CONTACT_VERIFICATION_PASSWORD_H_

#include <map>
#include <string>

#include "src/fredlib/public_request/public_request_impl.h"

namespace Fred {
namespace PublicRequest {

class ContactVerificationPassword
{
    PublicRequestAuthImpl* prai_ptr_;
public:
    typedef std::map<std::string, std::string> MessageData;
private:
    const MessageData collectMessageData();
public:
    size_t get_password_chunk_length();
    ContactVerificationPassword(PublicRequestAuthImpl* _prai_ptr);
    void sendEmailPassword(const std::string& mailTemplate //db table mail_type.name
            );
    void sendLetterPassword( const std::string& custom_tag //tag in template xml params: "pin2",  "pin3"
            , Fred::Document::GenerationType doc_type //type for document generator
            , const std::string& message_type //for message_archive: "contact_verification_pin2", "contact_verification_pin3"
            , const std::string& comm_type //for message_archive: "letter"
            );
    void sendSmsPassword(const boost::format& sms_template
            , const std::string& message_type //for message_archive: "contact_verification_pin2"
            );
    std::string generateRandomPassword(const size_t _length);
    std::string generateRandomPassword();
    std::string generateAuthInfoPassword();
};//class ContactVerificationPassword

}}


#endif // CONTACT_VERIFICATION_PASSWORD_H_
