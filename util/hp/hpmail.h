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

class HPMail : boost::noncopyable
{
    HPCfgMap config_; //runtime configuration
    std::string phpsessid_; //PHP session id
    FILESharedPtr curl_log_file_guard_;//curl log file pointer guard
    std::string hp_batch_number_; // batch number generated by login
    std::size_t letter_file_number_; // file number of letter to compress
    LetterFileNames letter_file_names_; // list of files to compress
    bool saved_file_for_upload_; //have some file for upload
    bool compressed_file_for_upload_; //have some compressed file for upload

    static HPCfgMap required_config;//required postservice config with default values
    static std::auto_ptr<HPMail> instance_ptr;
    friend class std::auto_ptr<HPMail>;
protected:
    ~HPMail(){}//dtor
private:
    //ctor
    HPMail( const HPCfgMap& config)
    : config_ (config) //copy first
    , phpsessid_("") //initially empty, filled by succesful login
    , curl_log_file_guard_(fopen((config_["mb_proc_tmp_dir"] + "curl_stderr.log").c_str(),"w"))
    , hp_batch_number_("") //initially empty, generated by succesful login
    , letter_file_number_() //initially 0
    , saved_file_for_upload_(false)//initially no files
    , compressed_file_for_upload_(false)//initially no files
    {}
    void save_list_for_archiver();
    void load_compressed_mail_batch(MailBatch& compressed_mail_batch);
    void save_compressed_mail_batch_for_test(MailBatch& compressed_mail_batch);
    void upload_of_batch(MailBatch& compressed_mail_batch);
    void end_of_batch(MailBatch& compressed_mail_batch);
public:
    static HPMail* set(const HPCfgMap& config_changes);
    static HPMail* get();

    void login(const std::string& loginame //postservice account name
        , const std::string& password //postservice account password
        , const std::string& batch_id //batch identificator like: "hpcb_Jednorazova_zakazka"
        , const std::string& batch_note //batch note like: "Testovaci prenos!!!" or anything else you need to tell here
        );
    void upload( const MailBatch& mb = MailBatch());
    void save_files_for_upload( const MailBatch& mb);
    void save_file_for_upload( const MailFile& mf);
    void archiver_command();

};//class HPMail
#endif // HPMAIL_H_
