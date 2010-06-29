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

    boost::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        boost::shared_ptr<boost::program_options::options_description> opts_descs(
                new boost::program_options::options_description(
                        std::string("Postservice client configuration")));

        // TODO add remaining options from hpmail.cc
        opts_descs->add_options()
                                // following are the options taken from 
                                // hpmail.cc source

                                ("main.hp_login_name,l", boost::program_options
                                            ::value<std::string>()
                                        , "upload account login name")
				("main.hp_login_password,p", boost::program_options
							::value<std::string>()
						, "upload account password")
                                ("main.hp_login_batch_id,b", boost::program_options
							::value<std::string>()->default_value(std::string(""))
						, "batch identificator like \"hpcb_Jednorazova_zakazka\"")
				("main.hp_login_note,n", boost::program_options
							::value<std::string>()->default_value(std::string(""))
						, "batch note like \"Testovaci prenos!!!\"")

                                ("main.hp_upload_retry,r", boost::program_options
							::value<std::string>()->default_value("10")
							 , "archive volume upload retries number")
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
				("main.hp_upload_curl_verbose,v", boost::program_options::value<std::string>()->default_value(std::string("0")), "enable additional debug data in tmpdir error log")
				("main.hp_upload_curlopt_timeout,t", boost::program_options
							::value<std::string>()->default_value("100")
							 , "curl connect timeout and transfer timeout [s]")
                                ("main.hp_login_interface_url", boost::program_options::value<std::string>(), "login form url")
                                ("main.hp_upload_interface_url", boost::program_options::value<std::string>(), "upload form url")
                                ("main.hp_ack_interface_url", boost::program_options::value<std::string>(), "end form url")
                                ("main.hp_cancel_interface_url", boost::program_options::value<std::string>(),  "cancel form url")
				;


        return opts_descs;
    }//get_options_description

    void handle( int argc, char* argv[],  FakedArgs &fa)
    {
        boost::program_options::variables_map vm;
        handler_parse_args(get_options_description(), vm, argc, argv, fa);

        // Check required parametres 
        if(vm.count("main.hp_login_interface_url") == 0) {
                throw std::runtime_error("Required `hp_login_interface_url' configuration option missing. Cannont continue");
        }
        if(vm.count("main.hp_upload_interface_url") == 0) {
                throw std::runtime_error("Required `hp_upload_interface_url' configuration option missing. Cannont continue");
        }
        if(vm.count("main.hp_ack_interface_url") == 0) {
                throw std::runtime_error("Required `hp_ack_interface_url' configuration option missing. Cannont continue");
        }
        if(vm.count("main.hp_cancel_interface_url") == 0) {
                throw std::runtime_error("Required `hp_cancel_interface_url' configuration option missing. Cannont continue");
        }
        if(vm.count("main.hp_login_name") == 0) {
                throw std::runtime_error("Required `hp_login_name' configuration option missing. Cannont continue");
        }
        if(vm.count("main.hp_login_password") == 0) {
                throw std::runtime_error("Required `hp_login_password' configuration option missing. Cannont continue");
        }

        // set variables according to program options map
        hp_login_interface_url = vm["main.hp_login_interface_url"].as<std::string>(); 
        hp_upload_interface_url = vm["main.hp_upload_interface_url"].as<std::string>();
        hp_ack_interface_url = vm["main.hp_ack_interface_url"].as<std::string>();
        hp_cancel_interface_url = vm["main.hp_cancel_interface_url"].as<std::string>();


        hp_login_batch_id = vm["main.hp_login_batch_id"].as<std::string>();
        login = vm["main.hp_login_name"].as<std::string>();
        password = vm["main.hp_login_password"].as<std::string>();
        note = vm["main.hp_login_note"].as<std::string>();
        hp_upload_archiver_filename = vm["main.hp_upload_archiver_filename"].as<std::string>();
        hp_upload_archiver_additional_options = vm["main.hp_upload_archiver_additional_options"].as<std::string>();
        hp_upload_curlopt_timeout = hp_upload_curlopt_connect_timeout = vm["main.hp_upload_curlopt_timeout"].as<std::string>();
        hp_upload_retry= vm["main.hp_upload_retry"].as<std::string>();
        hp_upload_curl_verbose = vm["main.hp_upload_curl_verbose"].as<std::string>();
        mb_proc_tmp_dir = vm["main.mb_proc_tmp_dir"].as<std::string>();
        postservice_cert_dir  = vm["main.postservice_cert_dir"].as<std::string>();
        postservice_cert_file = vm["main.postservice_cert_file"].as<std::string>();


    }//handle
};//class HandleHPMailArgs

#endif //HANDLE_HPMAIL_ARGS_H_
