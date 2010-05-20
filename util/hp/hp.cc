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

//class PostDataResult
///static instance init
std::auto_ptr<PostDataResult> PostDataResult::instance_ptr(0);

///instance setter
PostDataResult* PostDataResult::set()
{
    std::auto_ptr<PostDataResult>
    tmp_instance(new PostDataResult);
    instance_ptr = tmp_instance;
    if (instance_ptr.get() == 0)
        throw std::runtime_error(
                "PostDataResult::set_instance error: instance not set");
    return instance_ptr.get();
}

///instance  getter
PostDataResult* PostDataResult::get()
{
    PostDataResult* ret = instance_ptr.get();
    if (ret == 0)
    {
        set();
        ret = instance_ptr.get();
        if (ret == 0)  throw std::runtime_error(
                "PostDataResult::get_instance error: instance not set");
    }
    return ret;
}

///buffer append
void PostDataResult::append(const std::string & str)
{
    buffer_.append(str);
}

std::string PostDataResult::getValueByKey(const std::string & str)
{
    std::size_t str_pos = buffer_.find(str);
    if(str_pos == std::string::npos)
        throw std::runtime_error("PostDataResult::getValueByKey error: '"
                + str + "' not found");


    return str;
}


