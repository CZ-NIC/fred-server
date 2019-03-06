/*
 * Copyright (C) 2008-2019  CZ.NIC, z. s. p. o.
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
 *  @hp.cc
 *  implementation of communication with hybrid postservice
 */


#include <cstdio>
#include <memory>
#include <string>
#include <ios>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <errno.h>

#include <boost/utility.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <curl/curl.h>

#include "src/util/hp/hp.hh"


//close curl logfile
void close_curl_log_file(FILE* curl_log_file )
{
    if((curl_log_file != stderr) && (curl_log_file != 0))
        fclose(curl_log_file);
}//close_curl_log_file

//open curl logfile or set stderr
FILE* open_curl_log_file(const std::string& curl_log_file_name)
{
    FILE* curl_log_file = 0;

    if(curl_log_file_name.empty())
        curl_log_file = static_cast<FILE*>(stderr);
    else
        curl_log_file = fopen(curl_log_file_name.c_str(),"w");

    if(curl_log_file == 0)
    {
        std::string msg(strerror(errno));
        throw std::runtime_error(std::string("Error opening log file ")
            +  curl_log_file_name + " - " + msg);
    }

    return curl_log_file;
}//open_curl_log_file

//make curl logfile name with timestamp
std::string make_curl_log_file_name(const std::string& dir_name
        , const std::string& file_name_suffix)
{
    return
        (file_name_suffix.empty()
            ? std::string("")
            : (dir_name
                + boost::posix_time::to_iso_string(
                    boost::posix_time::microsec_clock::local_time()
                    )
                +file_name_suffix)
            );
}//make_curl_log_file_name

//common config map processing
//required default is modified with changes
//"_dir" names are ended with slashes
//giving config
HPCfgMap hp_create_config_map(const HPCfgMap& required_config, const HPCfgMap& config_changes)
{
    //config
    HPCfgMap config;
    for(HPCfgMap::const_iterator default_it = required_config.begin()
            ; default_it != required_config.end(); ++default_it)
    {
        HPCfgMap::const_iterator change_it
            = config_changes.find(default_it->first);//look for change
        if(change_it != config_changes.end())
            config[change_it->first]= change_it->second;//change
        else//nochange, using required default
            config[default_it->first]=default_it->second;

        //if it's directory name, check slash at the end
        if(default_it->first.find("_dir") !=  std::string::npos)
        {
            char ending_sl = '/';
#ifdef WIN32
            ending_sl = '\\';
#endif

            if((*((config[default_it->first]).end() - 1)) != ending_sl)
                config[default_it->first]+= ending_sl;//add ending slash

            //std::cout << "\nslashed: " << default_it->first
            //<< " " << config[default_it->first] << std::endl;
        }
    }//for default_it

    return config;
}//hp_create_config_map


//class StringBuffer

///buffer append
void StringBuffer::append(std::string & str)
{
    buffer_.append(str);
}

///buffer append
void StringBuffer::append(const char* str)
{
    std::string tmp_str(str);
    this->append(tmp_str);
}

///buffer copy
std::string StringBuffer::copy()
{
    return buffer_;
}


///get fixed length value by key
std::string StringBuffer::getValueByKey(const std::string & key_str
        , const std::size_t value_len)
{
    std::size_t key_pos = buffer_.find(key_str);
    //if key not found
    if(key_pos == std::string::npos) return "";
    //key found return value
    return buffer_.substr(key_pos + key_str.length(), value_len);
}

///form post callback
 static size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
 {
     size_t buffer_size = size * nmemb;//number of bytes in buffer

     StringBuffer* sb = static_cast<StringBuffer*> (userp);

     //copy data from buffer into std string
     std::string data(static_cast<char *>(buffer), buffer_size);
     sb->append(data);//append to StringBuffer
     return buffer_size; //count bytes taken care of
 }

 static int debug_trace(CURL *handle, curl_infotype type,
              char *data, size_t size,
              void *userp)
 {
   StringBuffer* sb = static_cast<StringBuffer*> (userp);
   std::string strdata(static_cast<char *>(data), size);

   const char *text;
   (void)handle; // prevent compiler warning

   switch (type) {
   case CURLINFO_TEXT:
     text = "\n== Info";
     break;
   default:
     text = "\n=> Unknown header";
     break;
   case CURLINFO_HEADER_OUT:
     text = "\n=> Send header";
     break;
   case CURLINFO_DATA_OUT:
     text = "\n=> Send data";
     break;
   case CURLINFO_SSL_DATA_OUT:
     text = "\n=> Send SSL data";
     break;
   case CURLINFO_HEADER_IN:
     text = "\n<= Recv header";
     break;
   case CURLINFO_DATA_IN:
     text = "\n<= Recv data";
     break;
   case CURLINFO_SSL_DATA_IN:
     text = "\n<= Recv SSL data";
     break;
   }

   std::stringstream result_data;
   result_data << "\n" << text << "\n";// << strdata;
   if((type == CURLINFO_DATA_OUT)
           || (type == CURLINFO_DATA_IN)
           || (type == CURLINFO_SSL_DATA_OUT)
           || (type == CURLINFO_SSL_DATA_IN)
           )
    {//hexdump
       std::size_t format_counter = 0;

       result_data << "\nSize: " << strdata.size();

       for(std::string::iterator i = strdata.begin(); i != strdata.end() ; ++i)
       {

           if (format_counter%64 == 0) result_data << "\n";

           if (format_counter%(64*16) == 0) result_data << "\n";

           if(format_counter%16 == 0) result_data << " ";

           result_data << " ";
           result_data << std::setw( 2 ) << std::setfill( '0' )
               << std::hex << std::uppercase
               << static_cast<unsigned short>(static_cast<unsigned char>(*i));

           ++format_counter;
       }

    }
   else
   {
       result_data << strdata;
   }

   std::string result_string(result_data.str());

   sb->append(result_string);//append to buffer
   return 0;
 }


CURLcode hp_form_post(struct curl_httppost *form  //linked list ptr
        , const std::string& curlopt_url //url
        , const std::string& curlopt_capath //ended by slash
        , const std::string& curlopt_cert_file //cert file name like "postsignum_qca_root.pem"
        , const std::string& curlopt_cookie //cookie NAME=CONTENTS, Set multiple cookies in one string like this: "name1=content1; name2=content2;" PHPSESSID=6d8cbbd1e53b15aa0523f4579612f940;
        , const std::string& curlopt_useragent //header "CommandLine klient HP"
        , long curlopt_verbose // 0 / 1
        , void * write_data_ptr //response data external storage, depends on write_data callback
        , void * debug_data_ptr //debug data external storage, depends on write_data callback
        , FILE* curl_log_file //curl log file
        , long curlopt_timeout //default is not set, maximum time in seconds that you allow the libcurl transfer operation to take
        , long curlopt_connect_timeout //default is not set, maximum time in seconds that you allow the connection to the server to take
        , long curlopt_maxconnect //default is not set, maximum amount of simultaneously open connections that libcurl may cache in this easy handle
        , long curlopt_ssl_verifypeer //verify the authenticity of the peer's certificate, 1 - verify, default: 0 - no verify
        , long curlopt_ssl_verifyhost // 0, 1 , 2 - server certificate must indicate that the server is the server to which you meant to connect, or the connection fails
        )
{
    CURLSharedPtr  curl_easy_guard = CurlEasyCleanupPtr(curl_easy_init());
    CURLcode ret = CURLE_FAILED_INIT;
    CURL* curl = curl_easy_guard.get();

    std::string curlopt_cainfo(curlopt_capath+curlopt_cert_file);
    if(curl)//CURL*
    {
        //debug callback
        if (debug_data_ptr)
        {
            curl_easy_setopt(curl, CURLOPT_DEBUGDATA , debug_data_ptr);
            curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, debug_trace);
        }

        curl_easy_setopt(curl, CURLOPT_VERBOSE, curlopt_verbose);//enable debug callback
        if((curl_log_file)&& (curl_log_file != stderr))//use stderr redirected to logfile if not null
            curl_easy_setopt(curl, CURLOPT_STDERR , curl_log_file);

        if(curlopt_timeout)//curl operation tout
            curl_easy_setopt(curl, CURLOPT_TIMEOUT , curlopt_timeout);
        if(curlopt_connect_timeout)//curl connection tout
            curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT , curlopt_connect_timeout);
        if(curlopt_maxconnect)//curl max connection
            curl_easy_setopt(curl, CURLOPT_MAXCONNECTS , curlopt_maxconnect);

        curl_easy_setopt(curl, CURLOPT_HEADER, 1);//out header
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);//use location
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);//shut off the built-in progress meter completely

        //CURLOPT_RETURNTRANSFER
        curl_easy_setopt(curl, CURLOPT_WRITEDATA , write_data_ptr);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);

        //to validate against stored certificate
        // set the file with the certs validating the server
        curl_easy_setopt(curl,CURLOPT_CAINFO,curlopt_cainfo.c_str());//"./cert/postsignum_qca_root.pem"
        curl_easy_setopt(curl,CURLOPT_CAPATH,curlopt_capath.c_str());//"./cert/"
        // cert is stored PEM coded in file...
        // since PEM is default, we needn't set it for PEM
        curl_easy_setopt(curl,CURLOPT_SSLCERTTYPE,"PEM");
        // do not verify the authenticity of the peer's certificate
        curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER,curlopt_ssl_verifypeer);
        //server sends a certificate indicating its identity
        //that certificate must indicate that the server is the server
        //to which you meant to connect, or the connection fails.
        curl_easy_setopt(curl,CURLOPT_SSL_VERIFYHOST,curlopt_ssl_verifyhost);
        //what URL that receives this RFC2388 POST
        curl_easy_setopt(curl, CURLOPT_URL, curlopt_url.c_str());//https://online.postservis.cz/Command/over.php
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, form);

        if(!curlopt_cookie.empty())//if not empty set cookie
            curl_easy_setopt(curl, CURLOPT_COOKIE, curlopt_cookie.c_str());

        if(!curlopt_useragent.empty())//if not empty set useragent header
            curl_easy_setopt(curl, CURLOPT_USERAGENT, curlopt_useragent.c_str());

        ret = curl_easy_perform(curl);
    }

    return ret;
}

void hp_form_overeni(curl_httppost **formpost_pp //out parameter
        , const std::string& loginame
        , const std::string& password
        , const std::string& standzak //"hpcb_Jednorazova_zakazka"
        , const std::string& poznamka //"Testovaci prenos!!!"
        , const std::string& jobzak //"2010"
        , const std::string& verzeOS //"Linux"
        , const std::string& verzeProg //"20100315001"
        )
{
    struct curl_httppost *lastptr=NULL;
    // Fill in overeni
    curl_formadd(formpost_pp, &lastptr, CURLFORM_COPYNAME, "cc"
            , CURLFORM_COPYCONTENTS, "us \n", CURLFORM_END);
    curl_formadd(formpost_pp, &lastptr, CURLFORM_COPYNAME, "loginame"
            , CURLFORM_COPYCONTENTS, loginame.c_str(), CURLFORM_END);
    curl_formadd(formpost_pp, &lastptr, CURLFORM_COPYNAME, "password"
            , CURLFORM_COPYCONTENTS, password.c_str(), CURLFORM_END);
    curl_formadd(formpost_pp, &lastptr, CURLFORM_COPYNAME, "standzak"
            , CURLFORM_COPYCONTENTS, standzak.c_str(), CURLFORM_END);
    curl_formadd(formpost_pp, &lastptr,CURLFORM_COPYNAME, "poznamka"
            , CURLFORM_COPYCONTENTS, poznamka.c_str(), CURLFORM_END);
    curl_formadd(formpost_pp, &lastptr,CURLFORM_COPYNAME, "jobzak"
            , CURLFORM_COPYCONTENTS, jobzak.c_str(), CURLFORM_END);
    curl_formadd(formpost_pp, &lastptr, CURLFORM_COPYNAME, "verzeOS"
            , CURLFORM_COPYCONTENTS, verzeOS.c_str(), CURLFORM_END);
    curl_formadd(formpost_pp, &lastptr, CURLFORM_COPYNAME, "verzeProg"
            , CURLFORM_COPYCONTENTS, verzeProg.c_str(), CURLFORM_END);
}

///file upload with order number and crc32 checksum
void hp_form_command(curl_httppost **formpost_pp //out parameter
        , const std::string& pocetupl //decremented number of file
        , const std::string& file_crc //crc32 checksum
        , const std::string& filename_to_upload
        )
{
    struct curl_httppost *lastptr=NULL;
    // Fill in command
    curl_formadd(formpost_pp, &lastptr, CURLFORM_COPYNAME, "cc"
            , CURLFORM_COPYCONTENTS, "us \n", CURLFORM_END);
    curl_formadd(formpost_pp, &lastptr, CURLFORM_COPYNAME, "pocetupl"
            , CURLFORM_COPYCONTENTS, (pocetupl + "\n").c_str(), CURLFORM_END);
    curl_formadd(formpost_pp, &lastptr, CURLFORM_COPYNAME, "crc"
            , CURLFORM_COPYCONTENTS, file_crc.c_str(), CURLFORM_END);
    curl_formadd(formpost_pp, &lastptr, CURLFORM_COPYNAME, "upfile"
            , CURLFORM_FILE, filename_to_upload.c_str(), CURLFORM_END);
}

///file upload from buffer with order number and crc32 checksum
void hp_form_command_buffer(curl_httppost **formpost_pp //out parameter
        , const std::string& pocetupl //decremented number of file
        , const std::string& file_crc //crc32 checksum
        , const std::string& file_name //file name
        , const char * file_buffer //pointer to file data in memory
        , const std::size_t file_buffer_length //size of file data
        )
{
    struct curl_httppost *lastptr=NULL;
    // Fill in command
    curl_formadd(formpost_pp, &lastptr, CURLFORM_COPYNAME, "cc"
            , CURLFORM_COPYCONTENTS, "us \n", CURLFORM_END);
    curl_formadd(formpost_pp, &lastptr, CURLFORM_COPYNAME, "pocetupl"
            , CURLFORM_COPYCONTENTS, (pocetupl + "\n").c_str(), CURLFORM_END);
    curl_formadd(formpost_pp, &lastptr, CURLFORM_COPYNAME, "crc"
            , CURLFORM_COPYCONTENTS, file_crc.c_str(), CURLFORM_END);
    curl_formadd(formpost_pp, &lastptr, CURLFORM_COPYNAME, "upfile"
            , CURLFORM_BUFFER, file_name.c_str()
            , CURLFORM_BUFFERPTR, file_buffer
            , CURLFORM_BUFFERLENGTH, file_buffer_length
            , CURLFORM_END);
}


///upload end
void hp_form_konec(curl_httppost **formpost_pp //out parameter
        , const std::string& soubor //first file name
        , const std::string& pocetsouboru //files number
        , const std::string& stav //OK / KO
        )
{
    struct curl_httppost *lastptr=NULL;
    // Fill in konec
    curl_formadd(formpost_pp, &lastptr, CURLFORM_COPYNAME, "cc"
            , CURLFORM_COPYCONTENTS, "us \n", CURLFORM_END);
    curl_formadd(formpost_pp, &lastptr, CURLFORM_COPYNAME, "soubor"
            , CURLFORM_COPYCONTENTS, (soubor + "\n").c_str(), CURLFORM_END);
    curl_formadd(formpost_pp, &lastptr, CURLFORM_COPYNAME, "pocetsouboru"
            , CURLFORM_COPYCONTENTS, (pocetsouboru  + "\n").c_str(), CURLFORM_END);
    curl_formadd(formpost_pp, &lastptr, CURLFORM_COPYNAME, "stav"
            , CURLFORM_COPYCONTENTS, (stav + "\n").c_str(), CURLFORM_END);
}

///failure errorlog upload attempt with storno flag set
void hp_prubeh_command(curl_httppost **formpost_pp //out parameter
        , const std::string& filename_to_upload //failure errorlog filename
        )
{
    struct curl_httppost *lastptr=NULL;
    // Fill in command
    curl_formadd(formpost_pp, &lastptr, CURLFORM_COPYNAME, "cc"
            , CURLFORM_COPYCONTENTS, "us \n", CURLFORM_END);
    curl_formadd(formpost_pp, &lastptr, CURLFORM_COPYNAME, "storno"
            , CURLFORM_COPYCONTENTS, "1\n", CURLFORM_END);
    curl_formadd(formpost_pp, &lastptr, CURLFORM_COPYNAME, "upfile"
            , CURLFORM_FILE, filename_to_upload.c_str(), CURLFORM_END);
}


void hp_form_prehledzak(curl_httppost **formpost_pp //out parameter
        , const std::string& user
        , const std::string& passwd
        , const std::string& typ //txt csv
        , const std::string& cislozak //batch number
        , const std::string& datum //date in format yyyymmdd
        )
{
    if(cislozak.empty() && datum.empty())
        throw std::runtime_error("hp_form_prehledzak error: zadejte cislo zakazky nebo datum yyyymmdd");

    struct curl_httppost *lastptr=NULL;
    // Fill in prehledZak
    curl_formadd(formpost_pp, &lastptr, CURLFORM_COPYNAME, "user"
            , CURLFORM_COPYCONTENTS, user.c_str(), CURLFORM_END);
    curl_formadd(formpost_pp, &lastptr, CURLFORM_COPYNAME, "passwd"
            , CURLFORM_COPYCONTENTS, passwd.c_str(), CURLFORM_END);
    curl_formadd(formpost_pp, &lastptr, CURLFORM_COPYNAME, "typ"
            , CURLFORM_COPYCONTENTS, typ.c_str(), CURLFORM_END);
    //if(!cislozak.empty())
    curl_formadd(formpost_pp, &lastptr,CURLFORM_COPYNAME, "zakazka"
            , CURLFORM_COPYCONTENTS, cislozak.c_str(), CURLFORM_END);
    //if(!datum.empty())
    curl_formadd(formpost_pp, &lastptr, CURLFORM_COPYNAME, "datum"
            , CURLFORM_COPYCONTENTS, datum.c_str(), CURLFORM_END);
}



