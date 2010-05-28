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

#include "hpmail.h"

#include <boost/lexical_cast.hpp>
#include <boost/crc.hpp>

#include <cstdlib>
#include <exception>
#include <fstream>
#include <ios>
#include <iostream>
#include <ostream>
#include <sstream>
#include <vector>




int main ( int argc, char* argv[])
{
    try
    {

        curl_global_init(CURL_GLOBAL_ALL);//once per process call



        HPMail::set("./tmpdir/","./cert/", "https://online.postservis.cz/Command/");
        HPMail::get()->login("dreplech","dreplech","hpcb_Jednorazova_zakazka", "Testovaci prenos!!!");

        MailBatch mb;
        mb.push_back(MailFile(1,49));
        mb.push_back(MailFile(2,50));
        mb.push_back(MailFile(3,51));
        HPMail::get()->upload(mb);


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
