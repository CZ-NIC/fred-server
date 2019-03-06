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
 *  hybrid postservice mailbatch state check configuration
 */

#ifndef HANDLE_HPMAILBATCHSTATE_ARGS_HH_3F4A141ECB5A4EB7863F532D9E6A9E2F
#define HANDLE_HPMAILBATCHSTATE_ARGS_HH_3F4A141ECB5A4EB7863F532D9E6A9E2F

#include <iostream>
#include <exception>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include "src/util/cfg/faked_args.hh"
#include "src/util/cfg/handle_args.hh"

/**
 * \class HandleHPMailBatchStateArgs
 * \brief postservice client cmdline options handler
 */

class HandleHPMailBatchStateArgs : public HandleArgs
{
    HPCfgMap hp_config;

public:
	//selected hpmail config options values
   
    const std::string CONFIG_PREFIX;

    HandleHPMailBatchStateArgs()
    : CONFIG_PREFIX("statecheck.")
    {}

    const HPCfgMap get_map() { return hp_config; };

    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> opts_descs(
                new boost::program_options::options_description(
                        std::string("Postservice client state check configuration\n * file_names make no sence here ")
                        , 140 //width of help print in cols
                        ));

        opts_descs->add_options()
                                // following are the options taken from 
                                // hpmailbatchstate.cc source

               // ((CONFIG_PREFIX+"hp_login_batch_id,b").c_str(), boost::program_options
                 //           ::value<std::string>()->default_value(std::string(""))


                ((CONFIG_PREFIX+"hp_statecheck_user").c_str(), boost::program_options
                            ::value<std::string>()
                        , "state check account login name")
				((CONFIG_PREFIX+"hp_statecheck_password").c_str(), boost::program_options
							::value<std::string>()
						, "state check account password")
                ((CONFIG_PREFIX+"hp_statecheck_typ").c_str(), boost::program_options
                            ::value<std::string>()->default_value(std::string("csv"))
                        , "returned status format: csv or txt")

                ((CONFIG_PREFIX+"hp_statecheck_batchnumber").c_str(), boost::program_options
                            ::value<std::string>()
                        , "number of mailbatch returned by upload, prefered before batchdate if both set")
                ((CONFIG_PREFIX+"hp_statecheck_batchdate").c_str(), boost::program_options
                            ::value<std::string>()->default_value(std::string(""))
                        , "date of batch in format: yyyymmdd")

                ((CONFIG_PREFIX+"hp_curlopt_log_dir").c_str(), boost::program_options
                            ::value<std::string>()->default_value(std::string("./logdir/"))
                        , "path for curl stderr logfile")
				((CONFIG_PREFIX+"postservice_cert_dir").c_str(), boost::program_options
							::value<std::string>()->default_value(std::string("./cert/"))
						, "path for PEM certificates")
				((CONFIG_PREFIX+"postservice_cert_file").c_str(), boost::program_options
							::value<std::string>()->default_value(std::string("cert.pem"))
						, "PEM host certificates file name like \"postsignum_qca_root.pem\"")

                ((CONFIG_PREFIX+"hp_curlopt_verbose").c_str(), boost::program_options
                    ::value<std::string>()->default_value(std::string("0"))
                     , "enable additional debug data in stderr logfile")
				((CONFIG_PREFIX+"hp_curlopt_timeout").c_str(), boost::program_options
							::value<std::string>()->default_value("100")
							 , "curl connect timeout and transfer timeout [s]")
                ((CONFIG_PREFIX+"hp_statecheck_interface_url").c_str(), boost::program_options
                        ::value<std::string>()
                         , "optional statecheck form url")

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
};//class HandleHPMailBatchStateArgs

/**
 * \class HandleHPMailBatchStateArgsGrp
 * \brief postservice client cmdline options handler
 */

class HandleHPMailBatchStateArgsGrp : public HandleArgs
{
    HPCfgMap hp_config;

public:
    //selected hpmail config options values

    const std::string CONFIG_PREFIX;

    HandleHPMailBatchStateArgsGrp()
    : CONFIG_PREFIX("statecheck.")
    {}

    const HPCfgMap get_map() { return hp_config; };

    std::shared_ptr<boost::program_options::options_description>
    get_options_description()
    {
        std::shared_ptr<boost::program_options::options_description> opts_descs(
                new boost::program_options::options_description(
                        std::string("Postservice client state check configuration\n * file_names make no sence here ")
                        , 140 //width of help print in cols
                        ));

        opts_descs->add_options()
                                // following are the options taken from
                                // hpmailbatchstate.cc source

               // ((CONFIG_PREFIX+"hp_login_batch_id,b").c_str(), boost::program_options
                 //           ::value<std::string>()->default_value(std::string(""))


                ((CONFIG_PREFIX+"hp_statecheck_user").c_str(), boost::program_options
                            ::value<std::string>()
                        , "state check account login name")
                ((CONFIG_PREFIX+"hp_statecheck_password").c_str(), boost::program_options
                            ::value<std::string>()
                        , "state check account password")
                ((CONFIG_PREFIX+"hp_statecheck_typ").c_str(), boost::program_options
                            ::value<std::string>()->default_value(std::string("csv"))
                        , "returned status format: csv or txt")

                ((CONFIG_PREFIX+"hp_statecheck_batchnumber").c_str(), boost::program_options
                            ::value<std::string>()
                        , "number of mailbatch returned by upload, prefered before batchdate if both set")
                ((CONFIG_PREFIX+"hp_statecheck_batchdate").c_str(), boost::program_options
                            ::value<std::string>()->default_value(std::string(""))
                        , "date of batch in format: yyyymmdd")

                ((CONFIG_PREFIX+"hp_curlopt_log_dir").c_str(), boost::program_options
                            ::value<std::string>()->default_value(std::string("./logdir/"))
                        , "path for curl stderr logfile")
                ((CONFIG_PREFIX+"postservice_cert_dir").c_str(), boost::program_options
                            ::value<std::string>()->default_value(std::string("./cert/"))
                        , "path for PEM certificates")
                ((CONFIG_PREFIX+"postservice_cert_file").c_str(), boost::program_options
                            ::value<std::string>()->default_value(std::string("cert.pem"))
                        , "PEM host certificates file name like \"postsignum_qca_root.pem\"")

                ((CONFIG_PREFIX+"hp_curlopt_verbose").c_str(), boost::program_options
                    ::value<std::string>()->default_value(std::string("0"))
                     , "enable additional debug data in stderr logfile")
                ((CONFIG_PREFIX+"hp_curlopt_timeout").c_str(), boost::program_options
                            ::value<std::string>()->default_value("100")
                             , "curl connect timeout and transfer timeout [s]")
                ((CONFIG_PREFIX+"hp_statecheck_interface_url").c_str(), boost::program_options
                        ::value<std::string>()
                         , "optional statecheck form url")

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

                hp_config [key] = (it->second).as<std::string>();
        }

        return option_group_index;
    }//handle
};//class HandleHPMailBatchStateArgsGrp

#endif
