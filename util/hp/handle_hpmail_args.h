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
    HPCfgMap hp_config;

public:
	//selected hpmail config options values
   
    const static std::string CONFIG_PREFIX;

    const HPCfgMap get_map() { return hp_config; };

    boost::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        boost::shared_ptr<boost::program_options::options_description> opts_descs(
                new boost::program_options::options_description(
                        std::string("Postservice client configuration")
                        , 140 //width of help print in cols
                        ));

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
						, "path for letters archiving and errlog temporary "
						"and possibly large data, ended by slash")
                ("main.mb_curl_log_dir,e", boost::program_options
                            ::value<std::string>()->default_value(std::string("./logdir/"))
                        , "path for curl stderr log ended by slash")
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
				("main.hp_upload_curl_verbose,v", boost::program_options
				        ::value<std::string>()->default_value(std::string("0"))
				         , "enable additional debug data in tmpdir error log")
				("main.hp_upload_curlopt_timeout,t", boost::program_options
							::value<std::string>()->default_value("100")
							 , "curl connect timeout and transfer timeout [s]")
                ("main.hp_login_interface_url", boost::program_options
                        ::value<std::string>(), "optional login form url")
                ("main.hp_upload_interface_url", boost::program_options
                        ::value<std::string>(), "optional upload form url")
                ("main.hp_ack_interface_url", boost::program_options
                        ::value<std::string>(), "optional end form url")
                ("main.hp_cancel_interface_url", boost::program_options
                        ::value<std::string>(),  "optional cancel form url")
				;


        return opts_descs;
    }//get_options_description

    void handle( int argc, char* argv[],  FakedArgs &fa)
    {
        boost::program_options::variables_map vm;
        handler_parse_args(get_options_description(), vm, argc, argv, fa);

        // Check required parametres

        if(vm.count("main.hp_login_name") == 0) {
                throw std::runtime_error("Required `hp_login_name' configuration option missing. Cannont continue");
        }
        if(vm.count("main.hp_login_password") == 0) {
                throw std::runtime_error("Required `hp_login_password' configuration option missing. Cannont continue");
        }

        boost::program_options::variables_map::iterator it;
        for(it = vm.begin(); it != vm.end(); it++) {
                std::string key(it->first);
                if (key.compare(0, CONFIG_PREFIX.length(), CONFIG_PREFIX)==0) {
                        key = key.substr(CONFIG_PREFIX.length());
                }

                std::cout << key << " = " << it->second.as<std::string>() << std::endl;

                hp_config [key] = (it->second).as<std::string>();
        }

    }//handle
};//class HandleHPMailArgs

const std::string HandleHPMailArgs::CONFIG_PREFIX("main.");

#endif //HANDLE_HPMAIL_ARGS_H_
