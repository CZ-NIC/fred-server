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

struct letter_proc {
    unsigned long long file_id;
    unsigned long long letter_id;
    unsigned attempt;
    std::string fname;
};//info for letter processing

typedef std::vector<letter_proc> LetterProcInfo;

struct sms_proc {
    unsigned long long sms_id;
    unsigned attempt;
    std::string phone_number;
    std::string content;
    int new_status;
};//info for sms processing

typedef std::vector<sms_proc> SmsProcInfo;


class Manager : boost::noncopyable
{
public:

    //save message for later send
    unsigned long long save_sms_to_send(const char* contact_handle
            , const char* phone
            , const char* content
            , const char* message_type
            , unsigned long contact_object_registry_id
            , unsigned long contact_history_historyid
            );

    //save message for later send
    unsigned long long save_letter_to_send(const char* contact_handle
            , const PostalAddress& address
            , unsigned long long file_id
            , const char* message_type
            , unsigned long contact_object_registry_id
            , unsigned long contact_history_historyid
            );

    //load saved letters
    LetterProcInfo load_letters_to_send(std::size_t batch_size_limit);

    //load saved sms
    SmsProcInfo load_sms_to_send(std::size_t batch_size_limit);

    //set send result into letter status
    void set_letter_status(const LetterProcInfo& letters,long new_status, const std::string& batch_id);

    //set send result into sms status
    void set_sms_status(const SmsProcInfo& messages);

};

typedef boost::shared_ptr<Manager> ManagerPtr;
ManagerPtr create_manager();


}//namespace Messages
}//namespace Register
#endif //MESSAGES_IMPL_H_
