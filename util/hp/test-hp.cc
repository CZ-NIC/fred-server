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

#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

#include <boost/lexical_cast.hpp>
#include <iostream>
#include <string>

//php invented CURLOPT_RETURNTRANSFER replacement callback
 static size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
 {
     size_t buffer_size = 0;

     buffer_size = size * nmemb;//number of bytes in buffer

     std::string data(static_cast<char *>(buffer), buffer_size);
     data.size();

     StringBuffer::get()->append(data);

     std::cout << "\nData: " << data << std::endl;

     return buffer_size; //count bytes taken care of
 }



int main ( int argc, char* argv[])
{
    /* test overeni
    CURL* curl=0;
    CURLcode res;

    struct curl_httppost *formpost=NULL;
    struct curl_httppost *lastptr=NULL;
    struct curl_slist *headerlist=NULL;
    static const char buf[] = "Expect:";

    FILE *headerfile = fopen("./headers_dump", "w");//dump headers to it


    curl_global_init(CURL_GLOBAL_ALL);

    // Fill in overeni
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "cc"
            , CURLFORM_COPYCONTENTS, "us \n", CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "loginame"
            , CURLFORM_COPYCONTENTS, "dreplech", CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "password"
            , CURLFORM_COPYCONTENTS, "dreplech", CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "standzak"
            , CURLFORM_COPYCONTENTS, "hpcb_Jednorazova_zakazka", CURLFORM_END);
    curl_formadd(&formpost, &lastptr,CURLFORM_COPYNAME, "poznamka"
            , CURLFORM_COPYCONTENTS, "Testovaci prenos!!!", CURLFORM_END);
    curl_formadd(&formpost, &lastptr,CURLFORM_COPYNAME, "jobzak"
            , CURLFORM_COPYCONTENTS, "2010", CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "verzeOS"
            , CURLFORM_COPYCONTENTS, "Linux", CURLFORM_END);
    curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "verzeProg"
            , CURLFORM_COPYCONTENTS, "20100315001", CURLFORM_END);

     curl = curl_easy_init();

     // initalize custom header list
     // optionally stating that Expect: 100-continue is not  wanted

     headerlist = curl_slist_append(headerlist, buf);

     if(curl)
     {
         curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);//talk to me
         curl_easy_setopt(curl, CURLOPT_HEADER, 1);//out header
         curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);//use location

         //CURLOPT_RETURNTRANSFER
         StringBuffer::set();//reset recv buffer
         curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);

         //to validate against stored certificate
         // set the file with the certs validating the server
         curl_easy_setopt(curl,CURLOPT_CAINFO,"./cert/postsignum_qca_root.pem");
         curl_easy_setopt(curl,CURLOPT_CAPATH,"./cert/");
         // cert is stored PEM coded in file...
         // since PEM is default, we needn't set it for PEM
         curl_easy_setopt(curl,CURLOPT_SSLCERTTYPE,"PEM");
         // do not verify the authenticity of the peer's certificate
         curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER,0L);
         //server sends a certificate indicating its identity
         //that certificate must indicate that the server is the server
         //to which you meant to connect, or the connection fails.
         curl_easy_setopt(curl,CURLOPT_SSL_VERIFYHOST,2L);
         //what URL that receives this RFC2388 POST
         curl_easy_setopt(curl, CURLOPT_URL, "https://online.postservis.cz/Command/over.php");

         curl_easy_setopt(curl, CURLOPT_WRITEHEADER, headerfile);//dump headers

         //disable 100-continue header if explicitly requested
         //curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
         curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
         res = curl_easy_perform(curl);

         // always cleanup
         curl_easy_cleanup(curl);

         // then cleanup the formpost chain
         curl_formfree(formpost);
         // free slist
         curl_slist_free_all (headerlist);

         //headerfile
         fclose(headerfile);
       }

*/
    StringBuffer::set();//reset recv buffer

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

    std::cout << "PHPSESSID=" << StringBuffer::get()->getValueByKey("PHPSESSID=", 32) << std::endl;
    std::cout << "Overeni " << StringBuffer::get()->getValueByKey("Overeni ", 2) << std::endl;
    std::cout << "overenizak " << StringBuffer::get()->getValueByKey("overenizak ", 2) << std::endl;
    std::cout << "zaladr " << StringBuffer::get()->getValueByKey("zaladr ", 2) << std::endl;




    return 0;
}
