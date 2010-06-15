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
#include <boost/assign.hpp>

#include <cstdlib>
#include <exception>
#include <fstream>
#include <ios>
#include <iostream>
#include <ostream>
#include <sstream>
#include <vector>

#include "random_data_generator.h"
#include "handle_general_args.h"

HandlerPtrVector global_hpv =
 boost::assign::list_of
 (HandleArgsPtr(new HandleHelpArg))
 ;

int main ( int argc, char* argv[])
{
    try
    {

        //You are strongly advised to not allow this automatic behaviour, by calling curl_global_init(3) yourself properly.
        //viz http://curl.haxx.se/libcurl/c/curl_easy_init.html
        curl_global_init(CURL_GLOBAL_ALL);//once per process call

        RandomDataGenerator rdg;

        //HPMail instance configuration and initialization
        HPMail::set(boost::assign::map_list_of //some custom HPCfgMap config_changes
                ("mb_proc_tmp_dir","/data/img/tmpdir/") //empty temp dir for compressed files
                ("hp_upload_archiver_additional_options", "-mx5 -v20m -mmt=on")//volumes size
                ("hp_upload_curlopt_stderr_log","curl_stderr.log") //no curl log curl_stderr.log
                ("hp_upload_curl_verbose","1")//verbosity of communication dump 0/1
                ("postservice_cert_dir","./cert/")); //server certificate dir ended by slash


        for(unsigned i = 0; i < 2000; ++i)
        {
            std::string tmp_str(rdg.xstring(1024*1024*3));
            MailFile tmp_mf (tmp_str.begin(), tmp_str.end());
            HPMail::get()->save_file_for_upload(tmp_mf);
        }

        //no more data optional call of archiver ahead
        HPMail::get()->archiver_command();

        //mail batch prepared, upload to postservice
        HPMail::get()->login("dreplech","dreplech","hpcb_Jednorazova_zakazka","Testovaci prenos!!!");
        HPMail::get()->upload();

        //large data
        std::string tmp_str(rdg.xstring(1024*512));
        MailFile mf (tmp_str.begin(), tmp_str.end());
        MailBatch mb;
        mb.push_back(mf);

/* small test
        //HPMail instance configuration and initialization
        HPMail::set(boost::assign::map_list_of //some custom HPCfgMap config_changes
                ("mb_proc_tmp_dir","/data/img/tmpdir/") //empty temp dir for compressed files
                ("hp_upload_archiver_additional_options", "-mx5 -v5m -mmt=on")//volumes size
                ("hp_upload_curlopt_stderr_log","curl_stderr.log") //no curl log curl_stderr.log
                ("postservice_cert_dir","./cert/")); //server certificate dir ended by slash

        //mail batch prepared, upload to postservice
        HPMail::get()->login("dreplech","dreplech","hpcb_Jednorazova_zakazka","Testovaci prenos!!!");
        HPMail::get()->upload(mb);
*/

/*
        //1st
        HPMail::set(boost::assign::map_list_of //some custom HPCfgMap config_changes
                ("mb_proc_tmp_dir","./tmpdir0/") //empty temp dir for compressed files
                ("postservice_cert_dir","./cert/")); //server certificate dir ended by slash
        MailBatch mb0;
        mb0.push_back(MailFile(1,49));
        mb0.push_back(MailFile(2,50));
        mb0.push_back(MailFile(3,51));
        //mail batch prepared, upload to postservice
        HPMail::get()->login("dreplech","dreplech","hpcb_Jednorazova_zakazka","Testovaci prenos!!!");
        HPMail::get()->upload(mb0);


        //2nd
        HPMail::set(boost::assign::map_list_of //some custom HPCfgMap config_changes
                ("mb_proc_tmp_dir","./tmpdir1/") //empty temp dir for compressed files
                ("postservice_cert_dir","./cert/")); //server certificate dir ended by slash
        MailBatch mb1;
        mb1.push_back(MailFile(1,49));
        mb1.push_back(MailFile(2,50));
        mb1.push_back(MailFile(3,51));
        //mail batch prepared, upload to postservice
        HPMail::get()->login("dreplech","dreplech","hpcb_Jednorazova_zakazka","Testovaci prenos!!!");
        HPMail::get()->upload(mb1);
*/


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
