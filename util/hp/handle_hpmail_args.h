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
 *  @handle_hpmail_args.h
 *  hybrid postservice configuration
 */

#ifndef HANDLE_HPMAIL_ARGS_H_
#define HANDLE_HPMAIL_ARGS_H_

#include <iostream>
#include <exception>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include "faked_args.h"
#include "handle_args.h"

/**
 * \class HandleHPMailArgs
 * \brief postservice client cmdline options handler
 */

class HandleHPMailArgs : public HandleArgs
{

private:
    // all hpmail config option values
    HPCfgMap cfg;

public:
	//selected hpmail config options values
    std::string mb_proc_tmp_dir ;
    std::string postservice_cert_dir ;
    std::string postservice_cert_file ;
    std::string hp_login_interface_url ;
    std::string hp_upload_interface_url ;
    std::string hp_ack_interface_url ;
    std::string hp_cancel_interface_url ;
    std::string hp_upload_archiver_filename ;
    std::string hp_upload_archiver_additional_options ;
    std::string hp_upload_curlopt_timeout ;
    std::string hp_upload_curlopt_connect_timeout ;
    std::string hp_upload_curl_verbose ;
    std::string hp_upload_retry ;

    std::string login;
    std::string password;
    std::string hp_login_batch_id ;
    std::string note;

    HPCfgMap 
    getConfig() 
    {
        return cfg;
    }

    boost::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        boost::shared_ptr<boost::program_options::options_description> opts_descs(
                new boost::program_options::options_description(
                        std::string("Postservice client configuration")));

        // TODO add remaining options from hpmail.cc
        opts_descs->add_options()
				("main.send,s", "upload to online.postservice.cz production instance"
						", with right account sending mails"
						"or if not set upload to online3.postservice.cz test instance"
						", data dumped, default option")

                                // following are the options taken from 
                                // hpmail.cc source

                                ("main.hp_login_name,l", boost::program_options
                                            ::value<std::string>()->default_value(std::string(""))
                                        , "upload account login name")
				("main.hp_login_password,p", boost::program_options
							::value<std::string>()->default_value(std::string(""))
						, "upload account password")
				("main.hp_upload_retry,r", boost::program_options
							::value<std::string>()->default_value("10")
							 , "archive volume upload retries number")
				("main.hp_login_batch_id,b", boost::program_options
							::value<std::string>()->default_value(std::string(""))
						, "batch identificator like \"hpcb_Jednorazova_zakazka\"")
				("main.hp_login_note,n", boost::program_options
							::value<std::string>()->default_value(std::string(""))
						, "batch note like \"Testovaci prenos!!!\"")
				("main.mb_proc_tmp_dir,d", boost::program_options
							::value<std::string>()->default_value(std::string("./tmpdir/"))
						, "path for letters archiving and errlog temporary and possibly large data, ended by slash")
				("main.postservice_cert_dir,c", boost::program_options
							::value<std::string>()->default_value(std::string("./cert/"))
						, "path for PEM certificates, ended by slash")
				("main.postservice_cert_file,q", boost::program_options
							::value<std::string>()->default_value(std::string("cert.pem"))
						, "PEM host certificates file name like \"postsignum_qca_root.pem\"")
				("main.hp_upload_archiver_filename,a", boost::program_options
							::value<std::string>()->default_value(std::string("7z"))
						, "7z hp_upload_archiver_filename executable")
				("main.hp_upload_archiver_additional_options,o", boost::program_options
						::value<std::string>()->default_value(std::string("-mx5 -v5m"))
					, "7z hp_upload_archiver_filename additional options")
				("main.hp_upload_curl_verbose,v", "enable additional debug data in tmpdir error log")
				("main.hp_upload_curlopt_timeout,t", boost::program_options
							::value<std::string>()->default_value("1800")
							 , "curl connect timeout and transfer timeout [s]")
				;


        return opts_descs;
    }//get_options_description

    void handle( int argc, char* argv[],  FakedArgs &fa)
    {
        boost::program_options::variables_map vm;
        handler_parse_args(get_options_description(), vm, argc, argv, fa);

        for (boost::program_options::variables_map::iterator it = vm.begin(); it != vm.end(); it++) {
            cfg[it->first] = it->second.as<std::string>(); 

            // TODO - debug output
            std::cout << it->first << " = " << it->second.as<std::string>() << std::endl;        
        }

        // TODO remove duplicated defaults
        mb_proc_tmp_dir = (vm.count("mb_proc_tmp_dir") == 0
                ? std::string("./tmpdir/") : vm["mb_proc_tmp_dir"].as<std::string>());
        postservice_cert_dir  = (vm.count("postservice_cert_dir") == 0
                ? std::string("./cert/") : vm["postservice_cert_dir"].as<std::string>());
        postservice_cert_file = (vm.count("postservice_cert_file") == 0
                ? std::string("") : vm["postservice_cert_file"].as<std::string>());
        if (vm.count("send"))
        {
            hp_login_interface_url = "https://online.postservis.cz/Command/over.php";//login form url
            hp_upload_interface_url = "https://online.postservis.cz/Command/command.php";//upload form url
            hp_ack_interface_url = "https://online.postservis.cz/Command/konec.php";//end form url
            hp_cancel_interface_url = "https://online.postservis.cz/Command/prubeh.php";//cancel form url
        }
        else
        {
            hp_login_interface_url = "https://online3.postservis.cz/Command/over.php";//login form url
            hp_upload_interface_url = "https://online3.postservis.cz/Command/command.php";//upload form url
            hp_ack_interface_url = "https://online3.postservis.cz/Command/konec.php";//end form url
            hp_cancel_interface_url = "https://online3.postservis.cz/Command/prubeh.php";//cancel form url
        }

        hp_login_batch_id = (vm.count("hp_login_batch_id") == 0
                ? std::string("") : vm["hp_login_batch_id"].as<std::string>());
        login = (vm.count("hp_login_name") == 0
                ? std::string("") : vm["hp_login_name"].as<std::string>());
        password = (vm.count("hp_login_password") == 0
                ? std::string("") : vm["hp_login_password"].as<std::string>());
        note = (vm.count("hp_login_note") == 0
                ? std::string("") : vm["hp_login_note"].as<std::string>());
        hp_upload_archiver_filename = (vm.count("hp_upload_archiver_filename") == 0
                ? std::string("7z") : vm["hp_upload_archiver_filename"].as<std::string>());
        hp_upload_archiver_additional_options = (vm.count("hp_upload_archiver_additional_options") == 0
                ? std::string("-mx5 -v5m") : vm["hp_upload_archiver_additional_options"].as<std::string>());
        hp_upload_curlopt_timeout =
		hp_upload_curlopt_connect_timeout = (vm.count("hp_upload_curlopt_timeout") == 0
        		? std::string("1800") : vm["hp_upload_curlopt_timeout"].as<std::string>()) ;
        hp_upload_curl_verbose = (vm.count("hp_upload_curl_verbose") == 0 ? "0" : "1");
        hp_upload_retry= (vm.count("hp_upload_retry") == 0
        		? std::string("10") : vm["hp_upload_retry"].as<std::string>()) ;
    }//handle
};//class HandleHPMailArgs

#endif //HANDLE_HPMAIL_ARGS_H_
