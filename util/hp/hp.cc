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
 *  @hp.cc
 *  implementation of communication with postservice
 */

#include "hp.h"

//class StringBuffer
///static instance init
std::auto_ptr<StringBuffer> StringBuffer::instance_ptr(0);

///instance setter
StringBuffer* StringBuffer::set()
{
    std::auto_ptr<StringBuffer>
    tmp_instance(new StringBuffer);
    instance_ptr = tmp_instance;
    if (instance_ptr.get() == 0)
        throw std::runtime_error(
                "StringBuffer::set_instance error: instance not set");
    return instance_ptr.get();
}

///instance  getter
StringBuffer* StringBuffer::get()
{
    StringBuffer* ret = instance_ptr.get();
    if (ret == 0)
    {
        set();
        ret = instance_ptr.get();
        if (ret == 0)  throw std::runtime_error(
                "StringBuffer::get_instance error: instance not set");
    }
    return ret;
}

///buffer append
void StringBuffer::append(std::string & str)
{
    for(std::string::iterator i = str.begin(); i != str.end() ; ++i)
        if (*i == '\0') *i = ' ';//replace null chars for spaces before append

    buffer_.append(str);
}

///buffer append
void StringBuffer::append(const char* str)
{
    std::string tmp_str(str);
    this->append(tmp_str);
}

///get fixed length value by key
std::string StringBuffer::getValueByKey(const std::string & key_str
        , const std::size_t value_len)
{
    std::size_t key_pos = buffer_.find(key_str);
    //if key not found
    if(key_pos == std::string::npos) return "";
    //key found return value
    return buffer_.substr(key_pos + key_str.length(), value_len);
}


