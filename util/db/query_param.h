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



//compile time check - ok condition is true
template <bool B> struct TAssert{};
template <> struct TAssert<true>
{
    static void check() {};
};

//buffer type
typedef std::vector<char> QueryParamData;

/**
 * \class  QueryParam
 * \brief  query parameter text or binary container
 * endian conversion implementation is more easy than perfectly efficient
 * but for database operations it's fast enough
 */

class QueryParam
{
    //text or binary data flag
    bool binary_;
    //text plain or binary in network byte order
    QueryParamData buffer_;

    //save binary type to buffer_ in network byte order
    template <class T> void hton_impl(const T& t, std::size_t size_of_t)
    {
        buffer_ = QueryParamData(size_of_t);
        QueryParamData etest(sizeof(long));
        *(reinterpret_cast<long*>(&etest[0])) = 1l;

        if(etest[0]) //host little endian
        {
            for (std::size_t i = 0; i < size_of_t; ++i)
                buffer_[i] = reinterpret_cast
                    <const char*>(&t)[size_of_t-i-1];
        }
        else //host big endian
        {
            for (std::size_t i = 0; i < size_of_t; ++i)
                buffer_[i] = reinterpret_cast
                    <const char*>(&t)[i];
        }
    }

public:

    //copy
    QueryParam(const QueryParam& param )
    : binary_(param.binary_)
    , buffer_(param.buffer_)
    {}

    QueryParam& operator=(const QueryParam& param)
    {
        if (this != &param)
        {
            binary_=param.binary_;
            buffer_=param.buffer_;
        }
        return *this;
    }//operator=

    //ctor
    explicit QueryParam(const QueryParamData& data )
    : binary_(false)
    , buffer_(data)
    {}
    explicit QueryParam(const char* data_ptr, std::size_t data_size )
    : binary_(false)
    , buffer_(data_ptr, data_ptr + data_size)
    {}

    QueryParam(const char* data_ptr )
    : binary_(false)
    , buffer_(data_ptr, data_ptr + strlen(data_ptr))
    {}

    QueryParam(const std::string& text )
    : binary_(false)
    , buffer_(text.begin(), text.end())
    {}

    template <class T> QueryParam( T t )
    : binary_(true)
    {
        const std::size_t size_of_t = sizeof(t);

        if(size_of_t==1)
        {
            buffer_.push_back(t);
            return;
        }

        //QueryParam usage check: use only for basic types up to 8 bytes
        TAssert<( ((size_of_t%2 == 0) || (size_of_t <= 8)) )>::check();

        //TODO: check valid combinations of basic param types with database types in query
        hton_impl(t, size_of_t);
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
            std::cout << "Text param: " << std::string(buffer_.begin(),buffer_.end()) << std::endl;
        }
    }//print_buffer

    const QueryParamData& get_data() const
    {
        return buffer_;
    }

    bool get_format() const
    {
        return binary_;
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
