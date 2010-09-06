/*
 * Copyright (C) 2010  CZ.NIC, z.s.p.o.
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
 *  @messages_impl.h
 *  header of registry messages
 */

#ifndef MESSAGES_IMPL_H_
#define MESSAGES_IMPL_H_

#include <string>
#include <vector>



namespace Registry
{
namespace MessagesImpl
{


void send_sms_impl(const char* contact_handle
        , const char* phone
        , const char* content
        , const char* message_type
        , unsigned long contact_object_registry_id
        , unsigned long contact_history_historyid
        );

struct PostalAddress
{
    std::string name;
    std::string org;
    std::string street1;
    std::string street2;
    std::string street3;
    std::string city;
    std::string state;
    std::string code;
    std::string county;
};//struct PostalAddress

typedef std::vector<char> ByteBuffer;

void send_letter_impl(const char* contact_handle
        , const PostalAddress& address
        , const ByteBuffer& file_content
        , const char* file_name
        , const char* file_type
        , const char* message_type
        , unsigned long contact_object_registry_id
        , unsigned long contact_history_historyid
        );

}//namespace MessagesImpl
}//namespace Registry
#endif //MESSAGES_IMPL_H_
