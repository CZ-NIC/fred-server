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
 *  @hpmail.h
 *  hybrid postservice mailbatch state check interface header
 */

#ifndef HPMAILBATCHSTATE_H_
#define HPMAILBATCHSTATE_H_

#include <cstdio>
#include <memory>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <stdexcept>
#include <iostream>
#include <exception>
#include <vector>

#include <string.h>

#include <boost/utility.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>


#include "hp.h"

/**
 * \class HPMailBatchState
 * \brief interface for state of mail batch
 *  one instance per process and non-concurrent use only
 *  , using curl easy interface
 */

class HPMailBatchState : boost::noncopyable
{
    HPCfgMap config_; //runtime configuration
    std::string curl_log_file_name_;//curl log file name
    FILE* curl_log_file_;//curl log file pointer

    static HPCfgMap required_config;//required postservice config with default values
    static std::auto_ptr<HPMailBatchState> instance_ptr;
    friend class std::auto_ptr<HPMailBatchState>;
protected:
    ~HPMailBatchState()
    {
        close_curl_log_file(curl_log_file_);
    }//dtor
private:
    //ctor
    HPMailBatchState( const HPCfgMap& config)
    : config_ (config) //copy first
    , curl_log_file_name_(
            make_curl_log_file_name(config_["hp_curlopt_log_dir"]
                                      ,config_["hp_curlopt_stderr_log"]))
    , curl_log_file_(open_curl_log_file(curl_log_file_name_))//open logfile
    {}//ctor HPMailBatchState
public:
    static HPMailBatchState* set(const HPCfgMap& config_changes = HPCfgMap());
    static HPMailBatchState* get();

    //postservice interface prehledZak
    //may be called repeatedly with same instance
    //each call have its own curl logfile
    std::string check(
            const std::string& batch_number //batch number, if date set, this should be empty ""
            , const std::string& date = "" //date in format yyyymmdd, default empty
        );

};
#endif // HPMAILBATCHSTATE_H_
