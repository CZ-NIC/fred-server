/*
 * Copyright (C) 2010  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *  (at your option) any later version.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#define BOOST_TEST_MODULE Test db

#include "test-db.h"



//args processing config for custom main
HandlerPtrVector global_hpv =
boost::assign::list_of
(HandleArgsPtr(new HandleGeneralArgs))
(HandleArgsPtr(new HandleDatabaseArgs))
;

#include "test_custom_main.h"


#include <netinet/in.h>
#include <vector>
#include <string>
#include <ios>
#include <iomanip>
#include <sstream>
#include <stdexcept>


//testing prototype
typedef std::vector<char> QueryParamData;
class QueryParam
{
    bool binary_;
    //binary stored in network byte order
    QueryParamData buffer_;

    //save binary type to buffer_ in network byte order
    template <class T> void hton(const T& t)
    {
        buffer_ = QueryParamData(sizeof(t));
        QueryParamData etest(sizeof(long));
        *(reinterpret_cast<long*>(&etest[0])) = 1l;

        if(etest[0]) //host little endian
        {
            for (std::size_t i = 0; i < sizeof(t); ++i)
                buffer_[i] = reinterpret_cast
                    <const char*>(&t)[sizeof(t)-i-1];
        }
        else //host big endian
        {
            for (std::size_t i = 0; i < sizeof(t); ++i)
                buffer_[i] = reinterpret_cast
                    <const char*>(&t)[i];
        }
    }

public:
    //ctor
    explicit QueryParam(const QueryParamData& data )
    : binary_(false)
    , buffer_(data)
    {}
    explicit QueryParam(const char* data_ptr, std::size_t data_size )
    : binary_(false)
    , buffer_(data_ptr, data_ptr + data_size)
    {}
    explicit QueryParam(const char* data_ptr )
    : binary_(false)
    , buffer_(data_ptr, data_ptr + strlen(data_ptr))
    {}
    explicit QueryParam(const std::string& text )
    : binary_(false)
    , buffer_(text.begin(), text.end())
    {}
    explicit QueryParam(const unsigned short uint16 )
    : binary_(true)
    , buffer_(sizeof(uint16))
    {
        *(reinterpret_cast<unsigned short*>(&buffer_[0]))
         =htons(uint16);
    }
    explicit QueryParam(const short int16 )
    : binary_(true)
    , buffer_(sizeof(int16))
    {
        *(reinterpret_cast<short*>(&buffer_[0]))
         =htons(int16);
    }
    explicit QueryParam(const unsigned int _uint )
    : binary_(true)
    , buffer_(sizeof(_uint))
    {
        *(reinterpret_cast<unsigned int*>(&buffer_[0]))
         =htonl(_uint);
    }
    explicit QueryParam(const int _int )
    : binary_(true)
    , buffer_(sizeof(_int))
    {
        *(reinterpret_cast<int*>(&buffer_[0]))
         =htonl(_int);
    }
    explicit QueryParam(const unsigned long _ulong )
    : binary_(true)
    , buffer_(sizeof(_ulong))
    {
        *(reinterpret_cast<unsigned long*>(&buffer_[0]))
         =htonl(_ulong);
    }
    explicit QueryParam(const long _long )
    : binary_(true)
    , buffer_(sizeof(_long))
    {
        *(reinterpret_cast<long*>(&buffer_[0]))
         =htonl(_long);
    }

    template <class T> QueryParam( T t )
    : binary_(true)
    {
        hton(t);
    }

    void print_buffer()
    {
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

        std::cout << std::string(buffer_.begin(),buffer_.end()) << std::endl;
    }//print_buffer
};

BOOST_AUTO_TEST_CASE( test_exec )
{
    QueryParam qp("Kuk");
    qp.print_buffer();

    BOOST_REQUIRE_EQUAL(exec_params_test() , 0);

}


