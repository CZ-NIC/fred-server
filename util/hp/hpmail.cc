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
 *  @hpmail.cc
 *  implementation of hybrid postservice simple interface
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

#include "hp.h"
#include "hpmail.h"

//class HPMail implementation

///static instance init
std::auto_ptr<HPMail> HPMail::instance_ptr(0);

///required HPMail configuration init with default values
HPCfgMap HPMail::required_config = boost::assign::map_list_of
    ("mb_proc_tmp_dir","./tmpdir/") //empty temp dir for compressed mail files or nonempty dir with set cleanup option
    ("postservice_cert_dir","./cert/") //server certificate dir ended by slash
    ("postservice_cert_file","postsignum_qca_root.pem")//cert file name like "postsignum_qca_root.pem"
    ("hp_login_job","2010")//set in orig config file like: jobzak="2010"
    ("hp_login_osversion","Linux")//"Linux" or "Windows"
    ("hp_login_clientversion","20100315001")//orig "20100315001"
    ("hp_login_interface_url","https://online3.postservis.cz/Command/over.php")//login form url
    ("hp_upload_interface_url","https://online3.postservis.cz/Command/command.php")//upload form url
    ("hp_ack_interface_url","https://online3.postservis.cz/Command/konec.php")//end form url
    ("hp_cancel_interface_url","https://online3.postservis.cz/Command/prubeh.php")//cancel form url
    ("hp_login_batch_id","hpcb_Jednorazova_zakazka")//some job identification,now login parameter
    ("hp_upload_archiver_filename","7z")//it should be something 7z compatible for now
    ("hp_upload_archiver_command_option","a")//add files to archive
    ("hp_upload_archiver_input_list","@in.lst")//input list of files to compress
    ("hp_upload_archiver_additional_options", "-mx5 -v5m -mmt=on") //5M volumes, multithreaded
    ("hp_upload_archiv_filename_suffix",".7z")//volume number is appended after archiv filename suffix .7z for now
    ("hp_useragent_id","CommandLine klient HP")//useragent id hardcoded in orig client
    ("hp_cleanup_last_arch_volumes","rm -f *.7z*") // delete last archive volumes
    ("hp_cleanup_last_letter_files","rm -f letter_*") // delete last letter files
    ("hp_upload_letter_file_prefix","letter_")//for saved letter file to archive
    ("hp_upload_archiv_filename_body","compressed_mail")//compressed mail file name body
    ("hp_upload_curlopt_timeout","120") //orig 1800, maximum time in seconds that you allow the libcurl transfer operation to take
    ("hp_upload_curlopt_connect_timeout","1800") //orig 1800, maximum time in seconds that you allow the connection to the server to take
    ("hp_upload_curlopt_maxconnect","20") //orig 20, maximum amount of simultaneously open connections that libcurl may cache in this easy handle
    ("hp_upload_curlopt_stderr_log","curl_stderr.log")//if empty free() invalid pointer, curl log file name in mb_proc_tmp_dir, if empty redir is not set
    ("hp_upload_curl_verbose","1")//verbosity of communication dump 0/1
    ("hp_upload_retry","10")//default number of upload retries
    ;

///instance set config and return if ok
HPMail* HPMail::set(const HPCfgMap& config_changes)
{
    //config
    HPCfgMap config;
    for(HPCfgMap::const_iterator default_it = HPMail::required_config.begin()
            ; default_it != HPMail::required_config.end(); ++default_it)
    {
        HPCfgMap::const_iterator change_it
            = config_changes.find(default_it->first);//look for change
        if(change_it != config_changes.end())
            config[change_it->first]= change_it->second;//change
        else//nochange, using required default
            config[default_it->first]=default_it->second;
    }

    std::auto_ptr<HPMail> tmp_instance(new HPMail(config));
    instance_ptr = tmp_instance;
    if (instance_ptr.get() == 0)
        throw std::runtime_error(
                "HPMail::set error: instance not set");
    return instance_ptr.get();
}

///instance getter
HPMail* HPMail::get()
{
    HPMail* ret = instance_ptr.get();
    if (ret == 0)
    {
     throw std::runtime_error(
             "HPMail::get error: instance not set");
    }
    return ret;
}

///postservice interface login, creating batch job and PHP session id
void HPMail::login(const std::string& loginame //postservice account name
    , const std::string& password //postservice account password
    , const std::string& batch_id //batch identificator like: "hpcb_Jednorazova_zakazka"
    , const std::string& batch_note //batch note like: "Testovaci prenos!!!"
    )
{
    if(!phpsessid_.empty())
        throw std::runtime_error("HPMail::login error: already logged in");

    struct curl_httppost *formpost_overeni=NULL;
    //release the form going out of scope
    CFormSharedPtr  form_overeni_guard = CurlFormFreePtr(&formpost_overeni);

    // Fill in form overeni
    hp_form_overeni(&formpost_overeni //inout parameter
            , loginame //loginame
            , password //password
            , batch_id //standzak
            , batch_note //poznamka
            , config_["hp_login_job"] //jobzak may be year, is set in orig config file like "2010"
            , config_["hp_login_osversion"] //verzeOS "Linux"
            , config_["hp_login_clientversion"] //verzeProg orig was "20100315001"
            );

    StringBuffer sb;//response buffer
    StringBuffer debugbuf;//debug buffer
    CURLcode res = hp_form_post(formpost_overeni  //linked list ptr
                , config_["hp_login_interface_url"] //form url
                , config_["postservice_cert_dir"] //cert file dir ended by slash
                , config_["postservice_cert_file"] //pem cert file name
                , "" //no cookie
                , "" //no useragent
                , boost::lexical_cast<long>(config_["hp_upload_curl_verbose"]) //verbose
                , &sb //response buffer
                , &debugbuf //debug buffer
                , curl_log_file_ //curl logfile
                );

    if (res > 0)
    {
        throw std::runtime_error(
                std::string("HPMail::login error: login form failed: ")
                    + curl_easy_strerror(res));
    }

    //log result
        std::string form_reply("\n\nover reply: \n"
                + sb.copy() + "\n\n" + debugbuf.copy());
        fwrite (form_reply.c_str() , 1, form_reply.size()
                , curl_log_file_ );

    //result parsing
    phpsessid_ =  sb.getValueByKey("PHPSESSID=", 32);
    hp_batch_number_ = sb.getValueByKey("cislozakazky", 12) ;

    //detecting errors
    if (hp_batch_number_.empty())
        throw std::runtime_error(std::string("HPMail::login error: empty batch number"));
    if (sb.getValueByKey("overenizak ", 2).compare("KO")==0)
        throw std::runtime_error(std::string("HPMail::login error: batch validation failed"));
    if (sb.getValueByKey("zaladr ", 2).compare("KO") == 0)
        throw std::runtime_error(std::string("HPMail::login error: mkdir failed"));
    if (sb.getValueByKey("Overeni ", 2).compare("OK") != 0)
        throw std::runtime_error(std::string("HPMail::login error: login failed"));
    if (sb.getValueByKey("UDRZBA", 2).compare("on") == 0)
        throw std::runtime_error(std::string("HPMail::login error: server out of order"));
}

/// upload batch of mail files to postservice
/// optionally no args required
/// then use save_file_for_upload and optionally archiver_command before login
void HPMail::upload( const MailBatch& mb)
{
    if(phpsessid_.empty())
        throw std::runtime_error("HPMail::upload error: not logged in");
    save_files_for_upload(mb);//if mb empty may have no effect
    if(!saved_file_for_upload_)
        throw std::runtime_error("HPMail::upload error: "
                "no files saved for upload by this interface");
    if(!compressed_file_for_upload_)
        archiver_command();
    //load archive volumes for upload
    VolumeFileNames upload_filelist = load_compressed_mail_batch_filelist();
    if(upload_filelist.empty())
        throw std::runtime_error(
                "HPMail::upload error: upload_filelist is empty");
    //upload
    upload_of_batch_by_filelist(upload_filelist);
    end_of_batch(upload_filelist);//ack form for postservice
    instance_ptr.reset(0);//end of session if no exception so far
}//HPMail::upload

/// in loop save files for upload to postservice
void HPMail::save_files_for_upload( const MailBatch& mb)
{

    //save letter data to disk like: letter_<number>
    for (MailBatch::const_iterator mail_file = mb.begin()
            ; mail_file != mb.end(); ++mail_file)
                save_file_for_upload( *mail_file);
}//HPMail::save_files_for_upload

/// save one file for upload to postservice
void HPMail::save_file_for_upload( const MailFile& mf)
{
    if(compressed_file_for_upload_)
        throw std::runtime_error("HPMail::save_file_for_upload error: "
                "already compresssed, call upload");
    if(!saved_file_for_upload_)
    {//cleanup for the first time
        std::string command_cleanup("cd " + config_["mb_proc_tmp_dir"]
               + " && "+config_["hp_cleanup_last_arch_volumes"]
               + " && "+config_["hp_cleanup_last_letter_files"]
                );

        int system_command_retcode =
         system(command_cleanup.c_str());//execute

         if(system_command_retcode != 0)
             throw std::runtime_error(
                     "HPMail::save_file_for_upload error: system command: "
                     + command_cleanup
                     + " failed with retcode: "
                     + boost::lexical_cast<std::string>(system_command_retcode));
    }

    //save letter data to disk like: letter_<number>
    std::string letter_file_name(
            config_["hp_upload_letter_file_prefix"]
            +boost::lexical_cast<std::string>(
                    letter_file_number_++)); //updating class file counter
    std::ofstream letter_file;
    letter_file.open ((config_["mb_proc_tmp_dir"]+letter_file_name).c_str()
            , std::ios::out | std::ios::trunc | std::ios::binary);
    if(letter_file.is_open())
    {
        letter_file.write(&mf[0], mf.size());
        letter_file_names_.push_back(letter_file_name);
        saved_file_for_upload_ = true;//we have some file
    }
}//HPMail::save_file_for_upload

///save list of files for archiver
void HPMail::save_list_for_archiver()
{
    std::string file_list_with_prefix(config_["hp_upload_archiver_input_list"]);
    std::string file_list_name( (file_list_with_prefix.substr(1,file_list_with_prefix.size() - 1 )).c_str());
    std::ofstream list_file;
    list_file.open ((config_["mb_proc_tmp_dir"]+file_list_name).c_str(), std::ios::out | std::ios::trunc);
    if(list_file.is_open())
    {
        for(LetterFileNames::iterator i = letter_file_names_.begin()
                ; i != letter_file_names_.end(); ++i )
        {
            list_file << *i << "\n";
        }
    }
    list_file.close();//flush
}//HPMail::save_list_for_archiver

///compress saved letter files for upload to 5m volumes archive
void HPMail::archiver_command()
{
    if(!saved_file_for_upload_)
        throw std::runtime_error("HPMail::archiver_command error: "
                "no files saved for upload by this interface");

    if(compressed_file_for_upload_)
        throw std::runtime_error("HPMail::archiver_command error: "
                "already compresssed, call upload");

    save_list_for_archiver();

    std::string command_for_arch (
            "cd " + config_["mb_proc_tmp_dir"] //cd to set tmpdir
            //compress letters to archive volume files
            //like: <hp_batch_number>.7z.001, ...002 ...
            + " && "+config_["hp_cleanup_last_arch_volumes"]+" "
            + " && "+config_["hp_upload_archiver_filename"]+" "
            + config_["hp_upload_archiver_command_option"] + " "
            + config_["hp_upload_archiv_filename_body"]
            +config_["hp_upload_archiv_filename_suffix"]
            +" " +config_["hp_upload_archiver_input_list"]+" "
            +config_["hp_upload_archiver_additional_options"]);

    int system_command_retcode =
    system(command_for_arch.c_str());//execute

    if(system_command_retcode != 0)
        throw std::runtime_error(
                "HPMail::archiver_command error: system command: "
                + command_for_arch
                + " failed with retcode: "
                + boost::lexical_cast<std::string>(system_command_retcode));

    compressed_file_for_upload_ = true;//ok we have some

}//HPMail::archiver_command

///load compressed mail volume using filename
void HPMail::load_compressed_mail_volume(const std::string& compressed_mail_volume_filename, MailFile& out_mf)
{
    std::ifstream compressed_mail_volume_stream;
    compressed_mail_volume_stream.open (compressed_mail_volume_filename.c_str()
        , std::ios::in | std::ios::binary);

    if(compressed_mail_volume_stream.is_open())
    {//ok file is there
        out_mf.clear();//remove previous content
        // get length of file
        compressed_mail_volume_stream.seekg (0, std::ios::end);
        long long compressed_mail_volume_length =
                compressed_mail_volume_stream.tellg();
        compressed_mail_volume_stream.seekg (0, std::ios::beg);//reset
        //allocate buffer
        out_mf.resize(
		    static_cast<unsigned>(compressed_mail_volume_length),'\0');
        //read whole file into the buffer
        compressed_mail_volume_stream.read( &out_mf[0]
		, static_cast<std::streamsize>(compressed_mail_volume_length) );
    }
    else //no more files
        throw std::runtime_error("HPMail::load_compressed_mail_volume: "
                "error - unable to access file: "
                +  compressed_mail_volume_filename);
}
///load compressed mail batch filelist from disk
VolumeFileNames HPMail::load_compressed_mail_batch_filelist()
{
    VolumeFileNames ret;

	//std::numeric_limits<std::size_t>::max()
	for(std::size_t i = 1; i < static_cast<unsigned>(-1); ++i)
    {
        std::stringstream order_number;
        if(i < 1000)
            order_number << std::setfill('0') << std::setw(3) << i;
        else
            order_number << i;
        std::string compressed_mail_volume_name(
                config_["mb_proc_tmp_dir"]
                +config_["hp_upload_archiv_filename_body"]
                +config_["hp_upload_archiv_filename_suffix"]
                +"."+order_number.str());

        std::ifstream compressed_mail_volume_stream;
        compressed_mail_volume_stream.open (compressed_mail_volume_name.c_str()
            , std::ios::in | std::ios::binary);

        if(compressed_mail_volume_stream.is_open())
        {//ok file is there
            ret.push_back(compressed_mail_volume_name);
        }
        else //no more files
            break;//for i loop
    }//for i

    return ret;
}//HPMail::load_compressed_mail_batch_filelist

///compute crc32 from MailFile&
std::string HPMail::crc32_into_string(MailFile& mf)
{
    //crc32 checksum into string
    boost::crc_32_type  crc32_checksum;
    crc32_checksum.process_bytes( &mf[0]
                                  , mf.size());
    std::stringstream crc32_string;
    crc32_string << std::setw( 8 ) << std::setfill( '0' ) //sometimes crc error when crc not starting with zero
        << std::hex << std::uppercase << crc32_checksum.checksum()
        << std::flush;
    return crc32_string.str();
}//HPMail::crc32_into_string

///upload of compressed mail batch to postservice server
void HPMail::upload_of_batch_by_filelist(VolumeFileNames& compressed_mail_batch_filelist)
{
    //upload to postservice
    for(std::size_t i = compressed_mail_batch_filelist.size(); i > 0; --i)
    {
        //get file buffer
        MailFile mail_archive_volume;

        //load file buffer from file
        load_compressed_mail_volume(compressed_mail_batch_filelist.at(i-1)
                ,mail_archive_volume);

        //archive volume file name
        std::stringstream order_number;
        if(i < 1000)
            order_number << std::setfill('0') << std::setw(3) << i << std::flush;
        else
            order_number << i;

        std::string compressed_mail_volume_name(
                hp_batch_number_+ config_["hp_upload_archiv_filename_suffix"]
                                          +"."+order_number.str());

        unsigned max_retry_count
            = boost::lexical_cast<unsigned>(config_["hp_upload_retry"]);
        for(unsigned retry_count = 0; retry_count < max_retry_count
            ; ++retry_count )
        {
        	bool send_storno_flag = false;//set in case of exception

            try
            {
                std::string crc32_string (//compute crc32
                        crc32_into_string(mail_archive_volume));

                std::cout << "upload: " << compressed_mail_volume_name
                        << " crc32: " << crc32_string
                        << " retry_count: " << retry_count
                        << std::endl;

                //make form for upload
                struct curl_httppost *formpost_command=NULL;
                CFormSharedPtr  form_command_guard = CurlFormFreePtr(&formpost_command);
                ///file upload from buffer with order number and crc32 checksum
                hp_form_command_buffer(&formpost_command //out parameter
                        , boost::lexical_cast<std::string>(i) //decremented number of file
                        , crc32_string  //crc32 checksum
                        , compressed_mail_volume_name //file name
                        , &mail_archive_volume[0] //pointer to file data
                        , mail_archive_volume.size() //size of file data
                        );

                //send form
                StringBuffer sb;//response buffer
                StringBuffer debugbuf;//debug buffer
                CURLcode res = hp_form_post(formpost_command  //linked list ptr
                    , config_["hp_upload_interface_url"]//url
                    , config_["postservice_cert_dir"] //ended by slash
                    , config_["postservice_cert_file"] //pem cert file name
                    , "PHPSESSID="+phpsessid_//PHP session id in cookie
                    , config_["hp_useragent_id"] //useragent id
                    , boost::lexical_cast<long>(config_["hp_upload_curl_verbose"]) //verbose
                    , &sb //response buffer
                    , &debugbuf //debug buffer
                    , curl_log_file_//curl logfile
                    //maximum time in seconds that you allow the libcurl transfer operation to take
                    , boost::lexical_cast<long>(config_["hp_upload_curlopt_timeout"])
                    //maximum time in seconds that you allow the connection to the server to take
                    , boost::lexical_cast<long>(config_["hp_upload_curlopt_connect_timeout"])
                    //maximum amount of simultaneously open connections that libcurl may cache in this easy handle
                    , boost::lexical_cast<long>(config_["hp_upload_curlopt_maxconnect"])
                    );

                if (res > 0)
                {
                    throw std::runtime_error(
                            std::string("HPMail::upload_of_batch error: form post command failed: ")
                                + curl_easy_strerror(res));
                }

                //log result
                std::stringstream formpost_reply;
                    formpost_reply << "\n\nCommand reply: \n"
                        << sb.copy()
                        << "\n\n" << debugbuf.copy()
                        <<  "\n" << "crc32: "
                        << crc32_string
                        << "\nfile number: "
                        << boost::lexical_cast<std::string>(i)
                        << std::endl;
                fwrite (formpost_reply.str().c_str() , 1
                        , formpost_reply.str().size() , curl_log_file_ );

                //result parsing & detecting errors
                if ((sb.getValueByKey("OvereniCrc ", 2)).compare("KO") == 0)
                            throw std::runtime_error(std::string(
                                    "HPMail::upload_of_batch error: crc check failed"));
                if ((sb.getValueByKey("zaladr ", 2)).compare("KO") == 0)
                    throw std::runtime_error(std::string(
                            "HPMail::upload_of_batch error: mkdir failed"));
                //will never happen
                if ((sb.getValueByKey("UDRZBA", 2)).compare("on") == 0)
                    throw std::runtime_error(std::string(
                            "HPMail::upload_of_batch error: server out of order"));

                break;//no check failed - quit for loop
            }//try for one upload volume
            catch(const std::exception& ex)
            {
            	send_storno_flag = true;

                std::stringstream errlogmsg;
                errlogmsg << "HPMail::upload_of_batch: " << ex.what()
                        << "retry_count: " << retry_count << std::flush;
                std::cout << errlogmsg.str() << std::endl;
                fwrite (errlogmsg.str().c_str() , 1
                        , errlogmsg.str().size() , curl_log_file_ );

            }

            if(send_storno_flag)
                send_storno();//attempt to send storno

			if((retry_count + 1) >= max_retry_count)
				throw std::runtime_error("HPMail::upload_of_batch error: retry failed");//retry failed

        }//for retry_count
    }//for compressed_mail_batch

}//HPMail::upload_of_batch_by_filelist

///signal end of batch to postservice server
void HPMail::end_of_batch(VolumeFileNames& compressed_mail_batch_filelist)
{
    //end of batch form
    struct curl_httppost *formpost_konec=NULL;
    CFormSharedPtr  form_konec_guard = CurlFormFreePtr(&formpost_konec);

    //first archive volume file name
    std::string first_compressed_mail_volume_name(
            hp_batch_number_+config_["hp_upload_archiv_filename_suffix"]+".001");

    //fill in konec
    hp_form_konec(&formpost_konec //out parameter
            , first_compressed_mail_volume_name
            , boost::lexical_cast<std::string>(compressed_mail_batch_filelist.size()) //number of files
            , "OK" //status
            );

    StringBuffer sb;//response buffer
    StringBuffer debugbuf;//debug buffer
    CURLcode res = hp_form_post(formpost_konec  //linked list ptr
                , config_["hp_ack_interface_url"]//"konec.php" url
                , config_["postservice_cert_dir"] //ended by slash
                , config_["postservice_cert_file"] //pem cert file name
                , "PHPSESSID="+phpsessid_//PHP session id in cookie
                , config_["hp_useragent_id"] //useragent id
                , boost::lexical_cast<long>(config_["hp_upload_curl_verbose"]) //verbose
                , &sb
                , &debugbuf//debug buffer
                , curl_log_file_//curl logfile
                //maximum time in seconds that you allow the libcurl transfer operation to take
                , boost::lexical_cast<long>(config_["hp_upload_curlopt_timeout"])
                //maximum time in seconds that you allow the connection to the server to take
                , boost::lexical_cast<long>(config_["hp_upload_curlopt_connect_timeout"])
                //maximum amount of simultaneously open connections that libcurl may cache in this easy handle
                , boost::lexical_cast<long>(config_["hp_upload_curlopt_maxconnect"])
                );

    if (res > 0)
    {
        throw std::runtime_error(
                std::string("form post konec failed: ")
                    + curl_easy_strerror(res));
    }

    //result parsing & detecting errors
    if ((sb.getValueByKey("zaladr ", 2)).compare("KO") == 0)
        throw std::runtime_error(std::string("zaladr is KO"));
    //will never happen
    if ((sb.getValueByKey("UDRZBA", 2)).compare("on") == 0)
        throw std::runtime_error(std::string("udrzba on"));


    //log result
    std::stringstream formpost_reply;
        formpost_reply << "\nKonec reply: \n" << sb.copy()
                << + "\n\n" << debugbuf.copy()
                            <<  "\n" << std::endl;
    fwrite (formpost_reply.str().c_str() , 1
            , formpost_reply.str().size() , curl_log_file_ );

}//HPMail::end_of_batch

void HPMail::init_curl()
{
        curl_global_init(CURL_GLOBAL_ALL);//once per process call
}

void HPMail::login()
{
    
        HPMail::set(boost::assign::map_list_of //some custom HPCfgMap config_changes
                ("mb_proc_tmp_dir","./tmpdir/") //empty temp dir for compressed files
                ("postservice_cert_dir","./cert/")); //server certificate dir ended by slash

        HPMail::get()->login("dreplech","dreplech","hpcb_Jednorazova_zakazka","Testovaci prenos!!!");

}

void HPMail::send_storno()
{
    //attempt to send storno
    struct curl_httppost *formpost_prubeh=NULL;
    CFormSharedPtr  form_prubeh_guard = CurlFormFreePtr(&formpost_prubeh);

    if(config_["hp_upload_curl_verbose"].compare("0") == 0)
    {
		//send form
		StringBuffer sb;//response buffer
		StringBuffer debugbuf;//debug buffer
		hp_prubeh_command(&formpost_prubeh //out parameter
				, curl_log_file_name_ //failure errorlog filename
				);

		hp_form_post(formpost_prubeh  //linked list ptr
			, config_["hp_cancel_interface_url"]//url
			, config_["postservice_cert_dir"] //ended by slash
			, config_["postservice_cert_file"] //pem cert file name
			, "PHPSESSID="+phpsessid_//PHP session id in cookie
			, config_["hp_useragent_id"] //useragent id
			, boost::lexical_cast<long>(config_["hp_upload_curl_verbose"]) //verbose
			, &sb
			, &debugbuf //debug buffer
			, curl_log_file_//curl logfile
			//maximum time in seconds that you allow the libcurl transfer operation to take
			, boost::lexical_cast<long>(config_["hp_upload_curlopt_timeout"])
			//maximum time in seconds that you allow the connection to the server to take
			, boost::lexical_cast<long>(config_["hp_upload_curlopt_connect_timeout"])
			//maximum amount of simultaneously open connections that libcurl may cache in this easy handle
			, boost::lexical_cast<long>(config_["hp_upload_curlopt_maxconnect"])
			);

		//log result
		std::stringstream formpost_reply;
			formpost_reply << "\nPrubeh reply: \n" << sb.copy()
					<< "\n\n" << debugbuf.copy()
								<<  "\n" << std::endl;
		fwrite (formpost_reply.str().c_str() , 1
				, formpost_reply.str().size() , curl_log_file_ );
    }
    else
    {
		//log result
		std::stringstream formpost_reply;
			formpost_reply << "\nPrubeh send canceled due to verbose error log\n"
					<< std::endl;
		fwrite (formpost_reply.str().c_str() , 1
				, formpost_reply.str().size() , curl_log_file_ );

    }
}

