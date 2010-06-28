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
 *  hybrid postservice simple interface header
 */

#ifndef HPMAIL_H_
#define HPMAIL_H_

#include <cstdio>
#include <memory>
#include <vector>
#include <map>
#include <string>
#include <stdexcept>
#include <iostream>
#include <exception>
#include <vector>
#include <errno.h>
#include <string.h>

#include <boost/utility.hpp>

#include "hp.h"

/**
 * \class HPMail
 * \brief interface for uploading to hybrid postservice
 *  one instance per process and non-concurrent use only
 *  , using curl easy interface and 7z executable
 */

typedef std::vector<char> MailFile;//one mail file data
typedef std::vector<MailFile> MailBatch;//all batch files
typedef std::map<std::string, std::string> HPCfgMap;//postservice config map
typedef std::vector<std::string> LetterFileNames; // vector of files to compress
typedef std::vector<std::string> VolumeFileNames; // vector of files to upload

class HPMail : boost::noncopyable
{
    HPCfgMap config_; //runtime configuration
    std::string phpsessid_; //PHP session id
    std::string curl_log_file_name_;//curl log file name
    FILE* curl_log_file_;//curl log file pointer
    std::string hp_batch_number_; // batch number generated by login
    std::size_t letter_file_number_; // file number of letter to compress
    LetterFileNames letter_file_names_; // list of files to compress
    bool saved_file_for_upload_; //have some file for upload
    bool compressed_file_for_upload_; //have some compressed file for upload

    static HPCfgMap required_config;//required postservice config with default values
    static std::auto_ptr<HPMail> instance_ptr;
    friend class std::auto_ptr<HPMail>;
protected:
    ~HPMail()
    {
        fclose(curl_log_file_);
    }//dtor
private:
    //ctor
    HPMail( const HPCfgMap& config)
    : config_ (config) //copy first
    , phpsessid_("") //initially empty, filled by succesful login
    , curl_log_file_name_( config_["hp_upload_curlopt_stderr_log"].empty()
            ? std::string("")
            : (config_["mb_proc_tmp_dir"] + config_["hp_upload_curlopt_stderr_log"])
            )
    , hp_batch_number_("") //initially empty, generated by succesful login
    , letter_file_number_() //initially 0
    , saved_file_for_upload_(false)//initially no files
    , compressed_file_for_upload_(false)//initially no files
    {
        if(curl_log_file_name_.empty()) {
                curl_log_file_ = static_cast<FILE*>(stderr);
        } else {
                curl_log_file_ = fopen(curl_log_file_name_.c_str(),"w");  
                if(curl_log_file_ == NULL) {
                    std::string msg(strerror(errno));
                    throw std::runtime_error(std::string("Error opening log file ") +  curl_log_file_name_ + " - " + msg);
                }
        }

    }
    void save_list_for_archiver();
    VolumeFileNames load_compressed_mail_batch_filelist();
    void load_compressed_mail_volume(
            const std::string& compressed_mail_volume_filename
            , MailFile& out_mf);
    void upload_of_batch_by_filelist(
            VolumeFileNames& compressed_mail_batch_filelist);
    void upload_of_mail_file(std::size_t file_number,const std::string& compressed_mail_volume_name
    		, MailFile& mail_archive_volume);
    void end_of_batch(VolumeFileNames& compressed_mail_batch_filelist);
    void send_storno();
    std::string crc32_into_string(MailFile& mf);
public:
    static HPMail* set(const HPCfgMap& config_changes = HPCfgMap());
    static HPMail* get();

    static void init_session(HPCfgMap &cfg);
    void login();
    void login(const std::string& loginame //postservice account name
        , const std::string& password //postservice account password
        , const std::string& batch_id //batch identificator like: "hpcb_Jednorazova_zakazka"
        , const std::string& batch_note //batch note like: "Testovaci prenos!!!" or anything else you need to tell here
        );
    void upload( const MailBatch& mb = MailBatch());
    void save_files_for_upload( const MailBatch& mb);
    void save_file_for_upload( const MailFile& mf);
    void save_file_for_upload( const std::string& file_name);
    void archiver_command();

};//class HPMail

#endif // HPMAIL_H_
