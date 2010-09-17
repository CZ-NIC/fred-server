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
#include <memory>
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>


namespace Register
{
namespace Messages
{

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
    std::string country;
};//struct PostalAddress

unsigned long long get_filetype_id(std::string file_type);

class Manager : boost::noncopyable
{
public:

    //save message for later send
    unsigned long long send_sms(const char* contact_handle
            , const char* phone
            , const char* content
            , const char* message_type
            , unsigned long contact_object_registry_id
            , unsigned long contact_history_historyid
            );

    //save message for later send
    unsigned long long send_letter(const char* contact_handle
            , const PostalAddress& address
            , unsigned long long file_id
            , const char* message_type
            , unsigned long contact_object_registry_id
            , unsigned long contact_history_historyid
            );

    //send saved letters
    void processLetters(std::size_t batch_size_limit);

    //send saved sms
    void processSMS(std::size_t batch_size_limit);

};

typedef boost::shared_ptr<Manager> ManagerPtr;
ManagerPtr create_manager();


}//namespace Messages
}//namespace Register
#endif //MESSAGES_IMPL_H_
