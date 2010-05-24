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

#include "hp.h"

#include <boost/lexical_cast.hpp>
#include <boost/crc.hpp>

#include <cstdlib>    // for EXIT_SUCCESS, EXIT_FAILURE
#include <exception>  // for std::exception
#include <fstream>    // for std::ifstream
#include <ios>        // for std::ios_base, etc.
#include <iostream>   // for std::cerr, std::cout
#include <ostream>    // for std::endl
#include <sstream>
#include <vector>




int main ( int argc, char* argv[])
{
    try
    {

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


        std::cout << "\nInfolog2 reply: \n" << StringBuffer::get()->copy()
                        <<  "\n" << std::endl;

        ////////////////////////

        unsigned file_number=1;

        std::string filename_to_upload("./data/file1.pdf");//file to send
        //std::string filename_to_upload("./data/test.txt");//file to send

        boost::crc_32_type  result;
        std::streamsize const  buffer_size = 1024;
        std::vector<char> char_vector;

        std::ifstream  ifs( filename_to_upload.c_str(), std::ios_base::binary );

        if ( ifs )
        {
            do
            {
                char  buffer[ buffer_size ];
                ifs.read( buffer, buffer_size );
                std::streamsize read_bytes = ifs.gcount();

                //alloc
                if ( static_cast<std::streamsize>(char_vector.capacity()
                        - char_vector.size()) < read_bytes)
                {
                    char_vector.reserve(char_vector.size() + read_bytes);
                }

                for (std::streamsize i = 0; i < read_bytes; ++i)
                {
                    char_vector.push_back(buffer[i]);
                }


            } while ( ifs );
        }
        else
        {
            throw std::runtime_error( "Failed to open file '" +filename_to_upload + "'.");
        }

        result.process_bytes( &char_vector[0],  char_vector.size());

        std::stringstream crc32_string;
        crc32_string << std::hex << std::uppercase << result.checksum() << std::flush;

        struct curl_httppost *formpost_command=NULL;
        CFormSharedPtr  form_command_guard = CurlFormFreePtr(&formpost_command);

        StringBuffer::set();//reset recv buffer
/*
        //fill in command
        hp_form_command(&formpost_command //out parameter
                , boost::lexical_cast<std::string>(file_number) //number of file
                , crc32_string.str() //crc32
                , filename_to_upload //filename
                );
*/

        ///file upload from buffer with order number and crc32 checksum
        hp_form_command_buffer(&formpost_command //out parameter
                , boost::lexical_cast<std::string>(file_number) //decremented number of file
                , crc32_string.str()  //crc32 checksum
                , filename_to_upload //file name
                , &char_vector[0] //pointer to file data in memory
                , char_vector.size() //size of file data
                );


        StringBuffer::set();//reset recv buffer, userp may be better
        res = hp_form_post(formpost_command  //linked list ptr
                    , "https://online.postservis.cz/Command/command.php" //url
                    , "./cert/" //ended by slash
                    , "PHPSESSID="+phpsessid //PHP session id in cookie
                    , "CommandLine klient HP" //no useragent
                    , 1); //verbose

        if (res > 0)
        {
            throw std::runtime_error(
                    std::string("form post command failed: ")
                        + curl_easy_strerror(res));
        }

        //result parsing & detecting errors
        if ((StringBuffer::get()->getValueByKey("OvereniCrc ", 2)).compare("KO") == 0)
                    throw std::runtime_error(std::string("crc is KO"));
        if ((StringBuffer::get()->getValueByKey("zaladr ", 2)).compare("KO") == 0)
            throw std::runtime_error(std::string("zaladr is KO"));
        //will never happen
        if ((StringBuffer::get()->getValueByKey("UDRZBA", 6)).compare("on") == 0)
            throw std::runtime_error(std::string("udrzba on"));

        std::cout << "\n\nCommand reply: \n" << StringBuffer::get()->copy()
                <<  "\n" << "crc32: " << crc32_string.str()
                << "\nnumber: " << boost::lexical_cast<std::string>(file_number)
                << std::endl;
        ////////////////////////


        struct curl_httppost *formpost_konec=NULL;
        CFormSharedPtr  form_konec_guard = CurlFormFreePtr(&formpost_konec);

        StringBuffer::set();//reset recv buffer

        //fill in konec
        hp_form_konec(&formpost_konec //out parameter
                , filename_to_upload
                , "1" //number of files
                , "OK" //status
                );

        StringBuffer::set();//reset recv buffer, userp may be better
        res = hp_form_post(formpost_konec  //linked list ptr
                    , "https://online.postservis.cz/Command/konec.php" //url
                    , "./cert/" //ended by slash
                    , "PHPSESSID="+phpsessid //PHP session id in cookie
                    , "CommandLine klient HP" //useragent
                    , 1); //verbose

        if (res > 0)
        {
            throw std::runtime_error(
                    std::string("form post konec failed: ")
                        + curl_easy_strerror(res));
        }

        //result parsing & detecting errors
        if ((StringBuffer::get()->getValueByKey("zaladr ", 2)).compare("KO") == 0)
            throw std::runtime_error(std::string("zaladr is KO"));
        //will never happen
        if ((StringBuffer::get()->getValueByKey("UDRZBA", 6)).compare("on") == 0)
            throw std::runtime_error(std::string("udrzba on"));


        std::cout << "\nKonec reply: \n" << StringBuffer::get()->copy()
                        <<  "\n" << std::endl;



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
