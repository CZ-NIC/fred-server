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
 *  @test-hp.cc
 *  test connection to postservice
 */

/*
    // Fill in the file
     curl_formadd(&formpost,
                  &lastptr,
                  CURLFORM_COPYNAME, "sendfile",
                  CURLFORM_FILE, "postit2.c",
                  CURLFORM_END);

     // Fill in the string
     curl_formadd(&formpost,
                  &lastptr,
                  CURLFORM_COPYNAME, "filename",
                  CURLFORM_COPYCONTENTS, "postit2.c",
                  CURLFORM_END);
*/

#include "hp.h"

#include <boost/lexical_cast.hpp>
#include <boost/crc.hpp>

#include <cstdlib>    // for EXIT_SUCCESS, EXIT_FAILURE
#include <exception>  // for std::exception
#include <fstream>    // for std::ifstream
#include <ios>        // for std::ios_base, etc.
#include <iostream>   // for std::cerr, std::cout
#include <ostream>    // for std::endl



int main ( int argc, char* argv[])
{
    try
    {
/*
        curl_global_init(CURL_GLOBAL_ALL);//once per process call

        // test overeni
        CURLcode res;

        struct curl_httppost *formpost_overeni=NULL;
        CFormSharedPtr  form_overeni_guard = CurlFormFreePtr(&formpost_overeni);

        // Fill in overeni
        hp_form_overeni(&formpost_overeni //inout parameter
                , "dreplech" //loginame
                , "dreplech" //password
                , "hpcb_Jednorazova_zakazka" //standzak
                , "Testovaci prenos!!!" //poznamka
                , "2010" //jobzak
                , "Linux" //verzeOS
                , "20100315001" //verzeProg
                );

        StringBuffer::set();//reset recv buffer, userp may be better
        res = hp_form_post(formpost_overeni  //linked list ptr
                    , "https://online.postservis.cz/Command/over.php" //url
                    , "./cert/" //ended by slash
                    , "" //no cookie
                    , "" //no useragent
                    , 1); //verbose

        if (res > 0)
        {
            throw std::runtime_error(
                    std::string("form post overeni failed: ")
                        + curl_easy_strerror(res));
        }

        //result parsing
        std::string phpsessid ( StringBuffer::get()->getValueByKey("PHPSESSID=", 32) );
        std::string overeni ( StringBuffer::get()->getValueByKey("Overeni ", 2) );
        std::string zaladr ( StringBuffer::get()->getValueByKey("zaladr ", 2) );
        std::string overenizak ( StringBuffer::get()->getValueByKey("overenizak ", 2) );
        std::string cislozak ( StringBuffer::get()->getValueByKey("cislozak", 12) );
        std::string udrzba ( StringBuffer::get()->getValueByKey("UDRZBA", 6) ) ;//possible error in orig code

        //detecting errors
        if (cislozak.empty())
            throw std::runtime_error(std::string("empty cislozak"));
        if (overenizak.compare("KO") == 0)
            throw std::runtime_error(std::string("overenizak is KO"));
        if (zaladr.compare("KO") == 0)
            throw std::runtime_error(std::string("zaladr is KO"));
        if (overeni.compare("OK") != 0)
            throw std::runtime_error(std::string("login failed"));
        if (udrzba.compare("on") == 0)//will never happen
            throw std::runtime_error(std::string("udrzba on"));


        //print
        std::cout << "PHPSESSID=" << phpsessid << std::endl;
        std::cout << "Overeni " << overeni << std::endl;
        std::cout << "overenizak " << overenizak << std::endl;
        std::cout << "cislozak" << cislozak << std::endl;


        ///////////////////////////////////////

        struct curl_httppost *formpost_infolog2=NULL;
        CFormSharedPtr  form_infolog2_guard = CurlFormFreePtr(&formpost_infolog2);

        StringBuffer::set();//reset recv buffer

        //fill in infolog2
        hp_form_infolog2(&formpost_infolog2 //out parameter
                , "Pocet souboru uvedenych v hpcmd.cfg je: "
                , "3" //number of files
                , "0" //err
                );

        StringBuffer::set();//reset recv buffer, userp may be better
        res = hp_form_post(formpost_infolog2  //linked list ptr
                    , "https://online.postservis.cz/Command/infolog2.php" //url
                    , "./cert/" //ended by slash
                    , "PHPSESSID="+phpsessid //PHP session id in cookie
                    , "CommandLine klient HP" //no useragent
                    , 1); //verbose

        if (res > 0)
        {
            throw std::runtime_error(
                    std::string("form post infolog2 failed: ")
                        + curl_easy_strerror(res));
        }

        //result parsing & detecting errors
        if ((StringBuffer::get()->getValueByKey("zaladr ", 2)).compare("KO") == 0)
            throw std::runtime_error(std::string("zaladr is KO"));
        //will never happen
        if ((StringBuffer::get()->getValueByKey("UDRZBA", 6)).compare("on") == 0)
            throw std::runtime_error(std::string("udrzba on"));

        ////////////////////////
*/

        boost::crc_32_type  result;

        std::streamsize const  buffer_size = 1024;

        std::ifstream  ifs( "./data/test.txt", std::ios_base::binary );

        if ( ifs )
        {
            do
            {
                char  buffer[ buffer_size ];

                ifs.read( buffer, buffer_size );
                result.process_bytes( buffer, ifs.gcount() );
            } while ( ifs );
        }
        else
        {
            std::cerr << "Failed to open file '" << "./data/test.txt" << "'."
             << std::endl;
        }


        std::cout << std::hex << std::uppercase << result.checksum() << std::endl;

    }//try
    catch(std::exception& ex)
    {
        std::cout << "Error: " << ex.what() << std::endl;
        return EXIT_FAILURE;

    }
    catch(...)
    {
        std::cout << "Unknown Error" << std::endl;
        return EXIT_FAILURE;

    }

    return EXIT_SUCCESS;
}


/* Data Sample
    StringBuffer::get()->append("HTTP/1.1 100 Continue\n");
    StringBuffer::get()->append("HTTP/1.1 100 Continue\n");
    StringBuffer::get()->append("\n");
    StringBuffer::get()->append("\n");
    StringBuffer::get()->append("HTTP/1.1 200 OK");
    StringBuffer::get()->append("HTTP/1.1 200 OK");
    StringBuffer::get()->append("Date: Thu, 20 May 2010 12:42:13 GMT\n");
    StringBuffer::get()->append("Date: Thu, 20 May 2010 12:42:13 GMT\n");
    StringBuffer::get()->append("Server: Apache/2.2.13\n");
    StringBuffer::get()->append("Server: Apache/2.2.13\n");
    StringBuffer::get()->append("X-Powered-By: PHP/5.2.11\n");
    StringBuffer::get()->append("X-Powered-By: PHP/5.2.11\n");
    StringBuffer::get()->append("Set-Cookie: PHPSESSID=6d8cbbd1e53b15aa0523f4579612f940; path=/\n");
    StringBuffer::get()->append("Set-Cookie: PHPSESSID=6d8cbbd1e53b15aa0523f4579612f940; path=/\n");
    StringBuffer::get()->append("Expires: Thu, 19 Nov 1981 08:52:00 GMT\n");
    StringBuffer::get()->append("Expires: Thu, 19 Nov 1981 08:52:00 GMT\n");
    StringBuffer::get()->append("Cache-Control: no-store, no-cache, must-revalidate, post-check=0, pre-check=0\n");
    StringBuffer::get()->append("Cache-Control: no-store, no-cache, must-revalidate, post-check=0, pre-check=0\n");
    StringBuffer::get()->append("Pragma: no-cache\n\n");
    StringBuffer::get()->append("Pragma: no-cache\n\n");
    StringBuffer::get()->append("Content-Length: 34\n\n");
    StringBuffer::get()->append("Content-Length: 34\n\n");
    StringBuffer::get()->append("Content-Type: text/html\n\n");
    StringBuffer::get()->append("Content-Type: text/html\n\n");
    StringBuffer::get()->append("Overeni OKcislozakazky201005201216\n");
*/

