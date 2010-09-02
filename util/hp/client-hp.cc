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

#include "config.h"
#include "hpmail.h"
#include "hpmailbatchstate.h"

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

#include "cfg/config_handler.h"
#include "cfg/handle_general_args.h"
#include "handle_clienthp_args.h"
#include "handle_hpmail_args.h"
#include "handle_hpmailbatchstate_args.h"

HandlerPtrVector global_hpv =
	boost::assign::list_of
	(HandleArgsPtr(new HandleHelpArg("\nUsage: client-hp <switches> [<file_names>...]\n")))
	(HandleArgsPtr(new HandleConfigFileArgs(HPMAIL_CONFIG) ))
	(HandleArgsPtr(new HandleClientHPArgs))
	(HandleArgsPtr(new HandleHPMailArgs))
	(HandleArgsPtr(new HandleHPMailBatchStateArgs))
	;


void config_print(HPCfgMap& set_cfg)
{
    std::cout << "\nConfig print\n" << std::endl;
    for (HPCfgMap::const_iterator i = set_cfg.begin()
            ; i !=  set_cfg.end(); ++i)
        std::cout   << i->first << ": " << i->second << std::endl;
}//config_print

void do_statecheck(FakedArgs& fa)
{
    HPCfgMap set_cfg = CfgArgs::instance()
        ->get_handler_ptr_by_type<HandleHPMailBatchStateArgs>()->get_map();
    config_print(set_cfg);//print current config
    curl_global_init(CURL_GLOBAL_ALL);//once per process call
    HPMailBatchState::set(set_cfg);
    std::cout << HPMailBatchState::get()->check( set_cfg["hp_statecheck_batchnumber"]
                                        , set_cfg["hp_statecheck_batchdate"] )
                                        << std::endl;
}//do_statecheck

void do_upload( FakedArgs& fa)
{
    HandleHPMailArgs* hpm_cfg
        = CfgArgs::instance()->get_handler_ptr_by_type<HandleHPMailArgs>();

    HPCfgMap set_cfg = hpm_cfg->get_map();

#ifdef WIN32
    HPCfgMap ins = boost::assign::map_list_of
    ("hp_login_osversion","Windows")//"Linux" or "Windows"
    ("hp_cleanup_last_arch_volumes","del /F *.7z*") // delete last archive volumes
    ("hp_cleanup_last_letter_files","del /F *.pdf"); // delete last letter files
    set_cfg.insert(ins.begin(), ins.end());
#endif

    config_print(set_cfg);

    std::cout << "\nFile names\n" << std::endl;
    for (int i = 1; i < fa.get_argc(); ++i)
        std::cout << fa.get_argv()[i] << std::endl;

    curl_global_init(CURL_GLOBAL_ALL);//once per process call

    //HPMail instance configuration and initialization
    HPMail::set(set_cfg);

    for (int i = 1; i < fa.get_argc(); ++i) //save files for upload
    {
        MailFile tmp_mf;
        std::ifstream letter_file;
        std::string letter_file_name(fa.get_argv()[i]);
        HPMail::get()->save_file_for_upload(letter_file_name);
    }

    HPMail::get()->archiver_command(); //optional call of archiver ahead
    //mail batch prepared, upload to postservice
    HPMail::get()->login();
    HPMail::get()->upload();

    std::cout << "The End" << std::endl;
}//do upload

void do_something( FakedArgs& fa)
{
    //what to do
    std::string do_action = CfgArgs::instance()
        ->get_handler_ptr_by_type<HandleClientHPArgs>()->do_action;

    if(do_action.compare("upload") == 0) do_upload(fa);
    if(do_action.compare("statecheck") == 0) do_statecheck(fa);

}//do_something


int main ( int argc, char* argv[])
{
	//producing faked args with unrecognized ones
	FakedArgs fa;
    try
    {
        fa = CfgArgs::instance<HandleHelpArg>(global_hpv)->handle(argc, argv);
        do_something(fa);
    }
    catch(const ReturnFromMain&)
    {
        return 0;
    }
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
}//main


