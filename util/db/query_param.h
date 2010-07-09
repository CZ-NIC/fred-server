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
 *  @file query_param.h
 *  Implementation of param object.
 */

#ifndef QUERYPARAM_H_
#define QUERYPARAM_H_

#include <vector>
#include <string>
#include <ios>
#include <iomanip>
#include <sstream>
#include <stdexcept>

#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>



//compile time check - OK condition is true
template <bool B> struct TAssert{};
template <> struct TAssert<true>
{
    static void check() {};
};

//buffer type
typedef std::string QueryParamData;

//db null
typedef int NullQueryParamType;
const NullQueryParamType QPNull =0;
const NullQueryParamType NullQueryParam =0;

/**
 * \class  QueryParam
 * \brief  query parameter text , binary or null container
 */

class QueryParam
{
    //text or binary data flag
    bool binary_;
    //isnull flag
    bool null_;

    //text or binary
    QueryParamData buffer_;

public:

    //copy
    QueryParam(const QueryParam& param )
    : binary_(param.binary_)
    , null_(false)
    , buffer_(param.buffer_)
    {}

    QueryParam& operator=(const QueryParam& param)
    {
        if (this != &param)
        {
            binary_=param.binary_;
            null_=param.null_;
            buffer_=param.buffer_;
        }
        return *this;
    }//operator=

    //ctor
    explicit QueryParam(const NullQueryParamType )
    : binary_(false)
    , null_ (true)
    {}

    QueryParam(const bool binary, const QueryParamData& data  )
    : binary_(binary)
    , null_(false)
    , buffer_(data)
    {}

    QueryParam(const char* data_ptr )
    : binary_(false)
    , null_(false)
    , buffer_(data_ptr)
    {}

    QueryParam(const std::string& text )
    : binary_(false)
    , null_(false)
    , buffer_(text)
    {}

    QueryParam( const std::vector<unsigned char>& blob )
    : binary_(true)
    , null_(false)
    {
        buffer_.resize(blob.size());
        memcpy(const_cast<char *>(buffer_.data()), &(*blob.begin()), blob.size());
    }

    QueryParam(const boost::posix_time::ptime& value )
    : binary_(false)
    , null_(false)
    {
        buffer_ = boost::posix_time::to_iso_extended_string(value);
        buffer_[buffer_.find('T')] = ' ';
    }

    //QueryParam(const boost::posix_time::ptime::date_type& value )
    QueryParam(const boost::gregorian::date& value )
    : binary_(false)
    , null_(false)
    , buffer_( boost::gregorian::to_iso_extended_string(value))
    {}

    template <class T> QueryParam( T t )
    : binary_(false)
    , null_(false)
    {
        const std::size_t size_of_t = sizeof(t);

        //QueryParam usage check: use only for basic types up to 8 bytes
        TAssert<( ((size_of_t==1) || (size_of_t%2 == 0) || (size_of_t <= 8)) )>::check();

        //TODO: check valid combinations of basic param types with database types in query
        buffer_ = boost::lexical_cast<std::string>(t);
    }

    void print_buffer()
    {
        if(binary_)
        {
            std::cout << "Binary param: ";
             for (QueryParamData::const_iterator i = buffer_.begin()
                     ; i != buffer_.end(); ++i)
             {
                 std::stringstream hexdump;
                 hexdump <<  std::setw( 2 ) << std::setfill( '0' )
                     << std::hex << std::uppercase
                     << static_cast<unsigned short>(static_cast<unsigned char>(*i));
                 std::cout << " " << hexdump.str();
             }
             std::cout << std::endl;
        }
        else
        {
            std::cout << "Text param: " <<  buffer_ //std::string(buffer_.begin() ,buffer_.end())
                     << std::endl;
        }
    }//print_buffer

    const QueryParamData& get_data() const
    {
        return buffer_;
    }

    bool is_binary() const
    {
        return binary_;
    }

    bool is_null() const
    {
        return null_;
    }
};//class QueryParam

typedef std::vector<QueryParam> QueryParams;

//simple functor template for container initialization
template <typename CONTAINER_TYPE > struct list_of_params
    : public CONTAINER_TYPE
{
    typedef typename CONTAINER_TYPE::value_type ELEMENT_TYPE;
    list_of_params(const ELEMENT_TYPE& t)
    {
        (*this)(t);
    }
    list_of_params& operator()(const ELEMENT_TYPE& t)
    {
        this->push_back(t);
        return *this;
    }
};

typedef list_of_params<QueryParams> query_param_list;

#endif //QUERYPARAM_H_
