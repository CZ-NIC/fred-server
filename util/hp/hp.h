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
 *  @hp.h
 *  postservice communication header
 */

#ifndef HP_H_
#define HP_H_

#include <string>
#include <memory>
#include <stdexcept>

#include <boost/utility.hpp>

/**
 * \class PostDataResult
 * \brief global string buffer for post result returned by post write callback
 */
class PostDataResult : boost::noncopyable
{
    std::string buffer_;
    static std::auto_ptr<PostDataResult> instance_ptr;
    friend class std::auto_ptr<PostDataResult>;
protected:
    ~PostDataResult(){}
private:
    PostDataResult()
    {
        buffer_.clear();
    }
public:
    static PostDataResult* set();
    static PostDataResult* get();

    void append(const std::string & str);
    std::string getValueByKey(const std::string & str);




};//class PostDataResult

#endif // HP_H_
