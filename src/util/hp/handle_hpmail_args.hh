/*
 * Copyright (C) 2010-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
/**
 *  @handle_hpmail_args.h
 *  hybrid postservice upload configuration
 */

#ifndef HANDLE_HPMAIL_ARGS_HH_D481C14FB0C346C8AC381A722CEB060D
#define HANDLE_HPMAIL_ARGS_HH_D481C14FB0C346C8AC381A722CEB060D

#include <iostream>
#include <exception>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include "src/util/cfg/faked_args.hh"
#include "src/util/cfg/handle_args.hh"

/**
 * \class HandleHPMailArgs
 * \brief postservice client cmdline options handler
 */

class HandleHPMailArgs : public HandleArgs
{
    HPCfgMap hp_config;

public:
	//selected hpmail config options values
   
    const std::string CONFIG_PREFIX;

    HandleHPMailArgs()
    : CONFIG_PREFIX("upload.")
    {}

    const HPCfgMap get_map() { return hp_config; };

    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> opts_descs(
                new boost::program_options::options_description(
                        std::string("Postservice client upload configuration")
                        , 140 //width of help print in cols
                        ));

        // TODO add remaining options from hpmail.cc
        opts_descs->add_options()
                                // following are the options taken from 
                                // hpmail.cc source

                ((CONFIG_PREFIX+"hp_login_name,l").c_str(), boost::program_options
                            ::value<std::string>()
                        , "upload account login name")
				((CONFIG_PREFIX+"hp_login_password,p").c_str(), boost::program_options
							::value<std::string>()
						, "upload account password")
                ((CONFIG_PREFIX+"hp_login_batch_id,b").c_str(), boost::program_options
							::value<std::string>()->default_value(std::string("hpcb_Jednorazova_zakazka"))
						, "batch identificator")
                ((CONFIG_PREFIX+"hp_login_registered_letter_batch_id").c_str(), boost::program_options
                            ::value<std::string>()
                        , "registered letter batch identificator")
                ((CONFIG_PREFIX+"hp_login_batch_id_suffix_domestic_letters").c_str(), boost::program_options
                            ::value<std::string>()->default_value(std::string(""))
                        , "batch suffix for domestic letters")
                ((CONFIG_PREFIX+"hp_login_batch_id_suffix_foreign_letters").c_str(), boost::program_options
                            ::value<std::string>()->default_value(std::string(""))
                        , "batch suffix for foreign letters")
				((CONFIG_PREFIX+"hp_login_note,n").c_str(), boost::program_options
							::value<std::string>()->default_value(std::string(""))
						, "note for processing of the batch")
                ((CONFIG_PREFIX+"hp_upload_retry,r").c_str(), boost::program_options
							::value<std::string>()->default_value("10")
							 , "archive volume upload retries number")
				((CONFIG_PREFIX+"mb_proc_tmp_dir,d").c_str(), boost::program_options
							::value<std::string>()->default_value(std::string("./tmpdir/"))
						, "path for letters archiving and errlog temporary "
						"and possibly large data, ended by slash")
                ((CONFIG_PREFIX+"mb_curl_log_dir,e").c_str(), boost::program_options
                            ::value<std::string>()->default_value(std::string("./logdir/"))
                        , "path for curl stderr log ended by slash")
				((CONFIG_PREFIX+"postservice_cert_dir,c").c_str(), boost::program_options
							::value<std::string>()->default_value(std::string("./cert/"))
						, "path for PEM certificates, ended by slash")
				((CONFIG_PREFIX+"postservice_cert_file,q").c_str(), boost::program_options
							::value<std::string>()->default_value(std::string("cert.pem"))
						, "PEM host certificates file name like \"postsignum_qca_root.pem\"")
				((CONFIG_PREFIX+"hp_upload_archiver_filename,a").c_str(), boost::program_options
							::value<std::string>()->default_value(std::string("7z"))
						, "7z hp_upload_archiver_filename executable")
				((CONFIG_PREFIX+"hp_upload_archiver_additional_options,o").c_str(), boost::program_options
						::value<std::string>()->default_value(std::string("-mx5 -v5m"))
					, "7z hp_upload_archiver_filename additional options")
				((CONFIG_PREFIX+"hp_upload_curl_verbose,v").c_str(), boost::program_options
				        ::value<std::string>()->default_value(std::string("0"))
				         , "enable additional debug data in tmpdir error log")
				((CONFIG_PREFIX+"hp_upload_curlopt_timeout,t").c_str(), boost::program_options
							::value<std::string>()->default_value("100")
							 , "curl connect timeout and transfer timeout [s]")
                ((CONFIG_PREFIX+"hp_login_interface_url").c_str(), boost::program_options
                        ::value<std::string>(), "optional login form url")
                ((CONFIG_PREFIX+"hp_upload_interface_url").c_str(), boost::program_options
                        ::value<std::string>(), "optional upload form url")
                ((CONFIG_PREFIX+"hp_ack_interface_url").c_str(), boost::program_options
                        ::value<std::string>(), "optional end form url")
                ((CONFIG_PREFIX+"hp_cancel_interface_url").c_str(), boost::program_options
                        ::value<std::string>(),  "optional cancel form url")

                ((CONFIG_PREFIX+"hp_curlopt_ssl_verifypeer").c_str(), boost::program_options
                   ::value<std::string>()->default_value("0")
                    , "verify the authenticity of the peer's certificate, 1 - verify, default: 0 - no verify http://curl.haxx.se/libcurl/c/curl_easy_setopt.html#CURLOPTSSLVERIFYPEER")
                ((CONFIG_PREFIX+"hp_curlopt_ssl_verifyhost").c_str(), boost::program_options
                    ::value<std::string>()->default_value("0")
                     , "default:0 - no verify, 1 , 2 - verify viz http://curl.haxx.se/libcurl/c/curl_easy_setopt.html#CURLOPTSSLVERIFYHOST")
                 ;


        return opts_descs;
    }//get_options_description

    void handle( int argc, char* argv[],  FakedArgs &fa)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);

        boost::program_options::variables_map::iterator it;
        for(it = vm.begin(); it != vm.end(); it++) {
                std::string key(it->first);
                if (key.compare(0, CONFIG_PREFIX.length(), CONFIG_PREFIX)==0) {
                        key = key.substr(CONFIG_PREFIX.length());
                }

                hp_config [key] = (it->second).as<std::string>();
        }

    }//handle
};//class HandleHPMailArgs


/**
 * \class HandleHPMailArgsGrp
 * \brief postservice client cmdline options handler
 */

class HandleHPMailArgsGrp : public HandleGrpArgs
{
    HPCfgMap hp_config;

public:
    //selected hpmail config options values

    const std::string CONFIG_PREFIX;

    HandleHPMailArgsGrp()
    : CONFIG_PREFIX("upload.")
    {}
    const HPCfgMap get_map() { return hp_config; };

    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> opts_descs(
                new boost::program_options::options_description(
                        std::string("Postservice client upload configuration")
                        , 140 //width of help print in cols
                        ));

        // TODO add remaining options from hpmail.cc
        opts_descs->add_options()
                                // following are the options taken from
                                // hpmail.cc source

                ((CONFIG_PREFIX+"hp_login_name,l").c_str(), boost::program_options
                            ::value<std::string>()
                        , "upload account login name")
                ((CONFIG_PREFIX+"hp_login_password,p").c_str(), boost::program_options
                            ::value<std::string>()
                        , "upload account password")
                ((CONFIG_PREFIX+"hp_login_batch_id,b").c_str(), boost::program_options
                            ::value<std::string>()->default_value(std::string("hpcb_Jednorazova_zakazka"))
                        , "batch identificator")
                ((CONFIG_PREFIX+"hp_login_registered_letter_batch_id").c_str(), boost::program_options
                            ::value<std::string>()
                        , "registered letter batch identificator")
                ((CONFIG_PREFIX+"hp_login_batch_id_suffix_domestic_letters").c_str(), boost::program_options
                            ::value<std::string>()->default_value(std::string(""))
                        , "batch suffix for domestic letters")
                ((CONFIG_PREFIX+"hp_login_batch_id_suffix_foreign_letters").c_str(), boost::program_options
                            ::value<std::string>()->default_value(std::string(""))
                        , "batch suffix for foreign letters")
                ((CONFIG_PREFIX+"hp_login_note,n").c_str(), boost::program_options
                            ::value<std::string>()->default_value(std::string(""))
                        , "note for processing of the batch")
                ((CONFIG_PREFIX+"hp_upload_retry,r").c_str(), boost::program_options
                            ::value<std::string>()->default_value("10")
                             , "archive volume upload retries number")
                ((CONFIG_PREFIX+"mb_proc_tmp_dir,d").c_str(), boost::program_options
                            ::value<std::string>()->default_value(std::string("./tmpdir/"))
                        , "path for letters archiving and errlog temporary "
                        "and possibly large data, ended by slash")
                ((CONFIG_PREFIX+"mb_curl_log_dir,e").c_str(), boost::program_options
                            ::value<std::string>()->default_value(std::string("./logdir/"))
                        , "path for curl stderr log ended by slash")
                ((CONFIG_PREFIX+"postservice_cert_dir,c").c_str(), boost::program_options
                            ::value<std::string>()->default_value(std::string("./cert/"))
                        , "path for PEM certificates, ended by slash")
                ((CONFIG_PREFIX+"postservice_cert_file,q").c_str(), boost::program_options
                            ::value<std::string>()->default_value(std::string("cert.pem"))
                        , "PEM host certificates file name like \"postsignum_qca_root.pem\"")
                ((CONFIG_PREFIX+"hp_upload_archiver_filename,a").c_str(), boost::program_options
                            ::value<std::string>()->default_value(std::string("7z"))
                        , "7z hp_upload_archiver_filename executable")
                ((CONFIG_PREFIX+"hp_upload_archiver_additional_options,o").c_str(), boost::program_options
                        ::value<std::string>()->default_value(std::string("-mx5 -v5m"))
                    , "7z hp_upload_archiver_filename additional options")
                ((CONFIG_PREFIX+"hp_upload_curl_verbose,v").c_str(), boost::program_options
                        ::value<std::string>()->default_value(std::string("0"))
                         , "enable additional debug data in tmpdir error log")
                ((CONFIG_PREFIX+"hp_upload_curlopt_timeout,t").c_str(), boost::program_options
                            ::value<std::string>()->default_value("100")
                             , "curl connect timeout and transfer timeout [s]")
                ((CONFIG_PREFIX+"hp_login_interface_url").c_str(), boost::program_options
                        ::value<std::string>(), "optional login form url")
                ((CONFIG_PREFIX+"hp_upload_interface_url").c_str(), boost::program_options
                        ::value<std::string>(), "optional upload form url")
                ((CONFIG_PREFIX+"hp_ack_interface_url").c_str(), boost::program_options
                        ::value<std::string>(), "optional end form url")
                ((CONFIG_PREFIX+"hp_cancel_interface_url").c_str(), boost::program_options
                        ::value<std::string>(),  "optional cancel form url")

                ((CONFIG_PREFIX+"hp_curlopt_ssl_verifypeer").c_str(), boost::program_options
                   ::value<std::string>()->default_value("0")
                    , "verify the authenticity of the peer's certificate, 1 - verify, default: 0 - no verify http://curl.haxx.se/libcurl/c/curl_easy_setopt.html#CURLOPTSSLVERIFYPEER")
                ((CONFIG_PREFIX+"hp_curlopt_ssl_verifyhost").c_str(), boost::program_options
                    ::value<std::string>()->default_value("0")
                     , "default:0 - no verify, 1 , 2 - verify viz http://curl.haxx.se/libcurl/c/curl_easy_setopt.html#CURLOPTSSLVERIFYHOST")

                ;


        return opts_descs;
    }//get_options_description

    std::size_t handle( int argc, char* argv[],  FakedArgs &fa, std::size_t option_group_index)
    {
        boost::program_options::variables_map vm;
        handler_parse_args()(get_options_description(), vm, argc, argv, fa);

        boost::program_options::variables_map::iterator it;
        for(it = vm.begin(); it != vm.end(); it++) {
                std::string key(it->first);
                if (key.compare(0, CONFIG_PREFIX.length(), CONFIG_PREFIX)==0) {
                        key = key.substr(CONFIG_PREFIX.length());
                }

                std::cout << key << " = " << it->second.as<std::string>() << std::endl;

                hp_config [key] = (it->second).as<std::string>();
        }

        return option_group_index;
    }//handle
};//class HandleHPMailArgsGrp

#endif
