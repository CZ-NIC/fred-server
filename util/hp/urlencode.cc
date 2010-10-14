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

#include <cstdio>
#include <string>
#include <memory>
#include <stdexcept>
#include <fstream>
#include <ios>
#include <iomanip>
#include <iostream>
#include <sstream>

#include <boost/utility.hpp>

#include <curl/curl.h>
#include "hp.h"

const unsigned buffer_size = 10240;

class UrlEncode : boost::noncopyable
{
    char *encoded_data;
public:
    UrlEncode(CURLSharedPtr curl_easy_guard,const std::string&  data)
    : encoded_data(curl_easy_escape(curl_easy_guard.get(),data.c_str()
            , data.length()))
    {}

    ~UrlEncode()
    {
        curl_free(encoded_data);
    }

    std::string get_encoded_data() const
    {
        return std::string(encoded_data);
    }

};

int main(int argc, char** argv)
{
    try
    {
        curl_global_init(CURL_GLOBAL_ALL);//once per process call
        CURLSharedPtr  curl_easy_guard = CurlEasyCleanupPtr(curl_easy_init());

        std::string data_to_encode("");

        for (int i = 1; i < argc; ++i)
        {
            std::ifstream  ifs(argv[i], std::ios_base::binary);
            if(ifs)
                do
                {
                    char  buffer[buffer_size];
                    ifs.read(buffer, buffer_size);
                    data_to_encode+=std::string(buffer, ifs.gcount());
                } while (ifs);
            else
                std::cerr << "Unable to open file: " << argv[i] << std::endl;
        }//for i

        std::cout << UrlEncode(curl_easy_guard,data_to_encode).get_encoded_data() << std::endl;

        return 0;
    }
    catch (std::exception &ex)
    {
        std::cerr << "Exception: " << ex.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "Unknown exception" << std::endl;
        return 2;
    }
}//main
