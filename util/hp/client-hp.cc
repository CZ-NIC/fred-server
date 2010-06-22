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

#include "config_handler.h"
#include "handle_general_args.h"
#include "handle_hpmail_args.h"

HandlerPtrVector global_hpv =
	boost::assign::list_of
	(HandleArgsPtr(new HandleHelpArg("\nUsage: client-hp <switches> [<file_names>...]\n")))
	(HandleArgsPtr(new HandleHPMailArgs))
	;


int main ( int argc, char* argv[])
{
	//producing faked args with unrecognized ones
	FakedArgs fa;
	try
	{
		fa = CfgArgs::instance<HandleHelpArg>(global_hpv)->handle(argc, argv);
	}
	catch(const ReturnFromMain&)
	{
		return 0;
	}

    try
    {
    	HandleHPMailArgs* hpm_cfg
			= CfgArgs::instance()->get_handler_ptr_by_type<HandleHPMailArgs>();

    	HPCfgMap set_cfg = boost::assign::map_list_of
    	("mb_proc_tmp_dir",hpm_cfg->mb_proc_tmp_dir)
    	("postservice_cert_dir",hpm_cfg->postservice_cert_dir)
    	("postservice_cert_file", hpm_cfg->postservice_cert_file)
    	("hp_login_interface_url",hpm_cfg->hp_login_interface_url)
		("hp_upload_interface_url",hpm_cfg->hp_upload_interface_url)
		("hp_ack_interface_url",hpm_cfg->hp_ack_interface_url)
		("hp_cancel_interface_url",hpm_cfg->hp_cancel_interface_url)
		("hp_upload_archiver_filename",hpm_cfg->hp_upload_archiver_filename)
		("hp_upload_archiver_additional_options"
				,hpm_cfg->hp_upload_archiver_additional_options)
		("hp_upload_curlopt_timeout",hpm_cfg->hp_upload_curlopt_timeout )
		("hp_upload_curlopt_connect_timeout"
				,hpm_cfg->hp_upload_curlopt_connect_timeout)
		("hp_upload_curl_verbose",hpm_cfg->hp_upload_curl_verbose )
		("hp_upload_retry",hpm_cfg->hp_upload_retry )
#ifdef WIN32
        ("hp_login_osversion","Windows")//"Linux" or "Windows"
        ("hp_cleanup_last_arch_volumes","del /F *.7z*") // delete last archive volumes
        ("hp_cleanup_last_letter_files","del /F letter_*") // delete last letter files
#endif
		;

    	std::cout << "\nConfig print\n" << std::endl;
    	for (HPCfgMap::const_iterator i = set_cfg.begin()
    			; i !=  set_cfg.end(); ++i)
    		std::cout	<< i->first << ": " << i->second << std::endl;

    	std::cout
    	<<"\nlogin: " << hpm_cfg->login
    	<<" password: " << hpm_cfg->password
    	<<" batchid: " << hpm_cfg->hp_login_batch_id
    	<<" note: " << hpm_cfg->note
    	<< std::endl;

    	std::cout << "\nFile names\n" << std::endl;
    	for (int i = 1; i < fa.get_argc(); ++i)
    		std::cout << fa.get_argv()[i] << std::endl;

        //You are strongly advised to not allow this automatic behaviour,
    	// by calling curl_global_init(3) yourself properly.
        //viz http://curl.haxx.se/libcurl/c/curl_easy_init.html
        curl_global_init(CURL_GLOBAL_ALL);//once per process call

        //HPMail instance configuration and initialization
        HPMail::set(set_cfg);

        for (int i = 1; i < fa.get_argc(); ++i)
        {
        	MailFile tmp_mf;
        	std::ifstream letter_file;
        	std::string letter_file_name(fa.get_argv()[i]);
        	HPMail::get()->save_file_for_upload(letter_file_name);
        }

        //no more data optional call of archiver ahead
        HPMail::get()->archiver_command();

        //mail batch prepared, upload to postservice
        HPMail::get()->login(hpm_cfg->login,hpm_cfg->password
        		,hpm_cfg->hp_login_batch_id,hpm_cfg->note);
        HPMail::get()->upload();

        std::cout << "The End" << std::endl;

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


