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
 *  @hpmailbatchstate.cc
 *  implementation of hybrid postservice state check interface
 */

#include <cstdio>
#include <string>
#include <memory>
#include <stdexcept>
#include <fstream>
#include <ios>
#include <iomanip>
#include <sstream>
#include <limits>

#include <curl/curl.h>

#include <boost/lexical_cast.hpp>
#include <boost/crc.hpp>
#include <boost/assign.hpp>
#include <utility>

#include "src/util/hp/hp.hh"
#include "src/util/hp/hpmailbatchstate.hh"

//class HPMailBatchState implementation

///static instance init
std::unique_ptr<HPMailBatchState> HPMailBatchState::instance_ptr;

///required HPMailBatchState configuration init with default values
///HPMailBatchState::set is ending values of keys containing "_dir" substring with '/' or '\' for win32
///so don't use key containing "_dir" substring for anything not ended by slash !
HPCfgMap HPMailBatchState::required_config = boost::assign::map_list_of
    ("hp_curlopt_log_dir","./tmpdir/") //log dir for curl stderr logfile
    ("postservice_cert_dir","./cert/") //server certificate dir ended by slash
    ("postservice_cert_file","postsignum_qca_root.pem")//cert file name like "postsignum_qca_root.pem"
    ("hp_statecheck_interface_url","https://online3.postservis.cz/prehledZak.php")//prehledzak form url

    ("hp_statecheck_user", "") //  login name
    ("hp_statecheck_password", "") // password
    ("hp_statecheck_typ", "txt") // txt, csv

    ("hp_statecheck_batchnumber", "") //  batch number, prefered
    ("hp_statecheck_batchdate", "") //  batch date in format yyyymmdd, default empty

    ("hp_curlopt_timeout","120") //orig 1800, maximum time in seconds that you allow the libcurl transfer operation to take
    ("hp_curlopt_connect_timeout","1800") //orig 1800, maximum time in seconds that you allow the connection to the server to take
    ("hp_curlopt_maxconnect","20") //orig 20, maximum amount of simultaneously open connections that libcurl may cache in this easy handle
    ("hp_curlopt_stderr_log","curl_stderr.log")//curl log file name suffix in hp_curlopt_log_dir, if empty redir is not set
    ("hp_curlopt_verbose","1")//verbosity of communication dump 0/1
    ("hp_curlopt_ssl_verifypeer","0") //verify the authenticity of the peer's certificate, 1 - verify, default: 0 - no verify
    ("hp_curlopt_ssl_verifyhost","0") // default: 0 - no verify, 1 , 2 - server certificate must indicate that the server is the server to which you meant to connect, or the connection fails
    ;

///instance set config and return if ok
HPMailBatchState* HPMailBatchState::set(const HPCfgMap& config_changes)
{
    //config
    HPCfgMap config = hp_create_config_map(HPMailBatchState::required_config, config_changes );

    std::unique_ptr<HPMailBatchState> tmp_instance(new HPMailBatchState(config));
    instance_ptr = std::move(tmp_instance);
    if (instance_ptr.get() == 0)
        throw std::runtime_error(
                "HPMailBatchState::set error: instance not set");
    return instance_ptr.get();
}

///instance getter
HPMailBatchState* HPMailBatchState::get()
{
    HPMailBatchState* ret = instance_ptr.get();
    if (ret == 0)
    {
     throw std::runtime_error(
             "HPMailBatchState::get error: instance not set");
    }
    return ret;
}

///postservice interface prehledZak
std::string HPMailBatchState::check(
        const std::string& batch_number //batch number
        , const std::string& date //date in format yyyymmdd
    )
{
    //recycle logfile
    close_curl_log_file(curl_log_file_);//close log
    curl_log_file_name_ = //log file name
            make_curl_log_file_name(config_["hp_curlopt_log_dir"]
                                        ,config_["hp_curlopt_stderr_log"]);
    curl_log_file_ = open_curl_log_file(curl_log_file_name_);//open logfile

    struct curl_httppost *formpost_prehledzak=NULL;
    //release the form going out of scope
    CFormSharedPtr  form_overeni_guard = CurlFormFreePtr(&formpost_prehledzak);

    // Fill in form prehledZak
    hp_form_prehledzak(&formpost_prehledzak //inout parameter
            , config_["hp_statecheck_user"] //loginame
            , config_ ["hp_statecheck_password"] //password
            , config_["hp_statecheck_typ"] //txt or csv
            , batch_number //cislozak
            , date//date in format yyyymmdd
            );

    StringBuffer sb;//response buffer
    StringBuffer debugbuf;//debug buffer
    CURLcode res = hp_form_post(formpost_prehledzak  //linked list ptr
        , config_["hp_statecheck_interface_url"] //form url
        , config_["postservice_cert_dir"] //cert file dir ended by slash
        , config_["postservice_cert_file"] //pem cert file name
        , "" //no cookie
        , "" //no useragent
        , boost::lexical_cast<long>(config_["hp_curlopt_verbose"]) //verbose
        , &sb //response buffer
        , &debugbuf //debug buffer
        , curl_log_file_  //curl logfile
        , 0, 0, 0 //disabled timeouts
        , boost::lexical_cast<long>(config_["hp_curlopt_ssl_verifypeer"]) //ssl config
        , boost::lexical_cast<long>(config_["hp_curlopt_ssl_verifyhost"])
        );
    if (res > 0)
    {
        throw std::runtime_error(
                std::string("prehledzak error: form failed: ")
                    + curl_easy_strerror(res));
    }
    //log result
        std::string form_reply("\n\nreply: \n"
                + sb.copy() + "\n\n" + debugbuf.copy());
        fwrite (form_reply.c_str() , 1, form_reply.size()
                , curl_log_file_ );


        close_curl_log_file(curl_log_file_);//close log
        curl_log_file_ = 0;
        //result
       return sb.copy();
}
