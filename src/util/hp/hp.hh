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
 *  @hp.h
 *  hybrid postservice communication header
 */

#ifndef HP_HH_3A1D3CF0E53D45F485880FE06D9731C8
#define HP_HH_3A1D3CF0E53D45F485880FE06D9731C8


#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <map>

#include <boost/utility.hpp>

#include <curl/curl.h>

//close curl logfile
void close_curl_log_file(FILE* curl_log_file );

//open curl logfile or set stderr
FILE* open_curl_log_file(const std::string& curl_log_file_name);

//make curl logfile name with timestamp
std::string make_curl_log_file_name(const std::string& dir_name //ended with slash
        , const std::string& file_name_suffix);

typedef std::map<std::string, std::string> HPCfgMap;//config map
//common config map processing
//required default is modified with changes
//"_dir" names are ended with slashes
//giving config
HPCfgMap hp_create_config_map(const HPCfgMap& required_config, const HPCfgMap& config_changes);

/**
 * \class StringBuffer
 * \brief string buffer for curl callback
 */
class StringBuffer
{
    std::string buffer_;
public:
    ~StringBuffer(){}
    StringBuffer()
    {
        buffer_.clear();
    }
    void append(std::string & str);
    void append(const char* str);
    std::string copy();
    std::string getValueByKey(const std::string & key_str, const std::size_t value_len);
};//class StringBuffer


///curl easy cleanup
typedef std::shared_ptr<CURL> CURLSharedPtr;
template < typename DELETER >
class CURLPtrT
{
protected:
    CURLSharedPtr m_ptr;
public:
    CURLPtrT(CURL* c) : m_ptr(c,DELETER()) {}
    CURLPtrT() : m_ptr(0,DELETER()) {}

    operator CURLSharedPtr() const
    {
        return m_ptr;
    }
};
///deleter functor for CURL calling curl_easy_cleanup
struct CurlEasyCleanup
{
    void operator()(CURL* c)
    {
        try
        {
            if(c)
            {
                //std::cout << "CURLSharedPtr: functor curl_easy_cleanup: " << c <<  std::endl;
                curl_easy_cleanup(c);
            }
        }
        catch(...){}
    }
};
///CURLSharedPtr factory
typedef CURLPtrT<CurlEasyCleanup> CurlEasyCleanupPtr;
///usage CURLSharedPtr  curl_easy_cleanup_guard = CurlEasyCleanupPtr(curl_easy_init());

///send RFC2388 http post request for postservice
CURLcode hp_form_post(struct curl_httppost *form  //linked list ptr
        , const std::string& curlopt_url //url
        , const std::string& curlopt_capath //ended by slash
        , const std::string& curlopt_cert_file //cert file name like "postsignum_qca_root.pem"
        , const std::string& curlopt_cookie //cookie NAME=CONTENTS, Set multiple cookies in one string like this: "name1=content1; name2=content2;" PHPSESSID=6d8cbbd1e53b15aa0523f4579612f940;
        , const std::string& curlopt_useragent //header "CommandLine klient HP"
        , long curlopt_verbose // 0 / 1
        , void * write_data_ptr //response data external storage, depends on write_data callback
        , void * debug_data_ptr //debug data external storage, depends on write_data callback
        , FILE* curl_log_file = 0 //curl log file
        , long curlopt_timeout = 0 //default is not set, maximum time in seconds that you allow the libcurl transfer operation to take
        , long curlopt_connect_timeout = 0 //default is not set, maximum time in seconds that you allow the connection to the server to take
        , long curlopt_maxconnect = 0 //default is not set, maximum amount of simultaneously open connections that libcurl may cache in this easy handle
        , long curlopt_ssl_verifypeer =0 //verify the authenticity of the peer's certificate, 1 - verify, default: 0 - no verify
        , long curlopt_ssl_verifyhost =2 // 0, 1 , default: 2 - server certificate must indicate that the server is the server to which you meant to connect, or the connection fails
        );

///curl_httppost form free
///actual pointer may change so pointer to pointer usage
typedef std::shared_ptr< curl_httppost* > CFormSharedPtr;
template < typename DELETER >
class CFormPtrT
{
protected:
    CFormSharedPtr m_ptr;
public:
    CFormPtrT(curl_httppost** f) : m_ptr(f,DELETER()) {}
    CFormPtrT() : m_ptr(0,DELETER()) {}

    operator CFormSharedPtr() const
    {
        return m_ptr;
    }
};
///deleter functor for free curl_httppost form
struct CFormFree
{
    void operator()(curl_httppost** f)
    {
        try
        {
            if(f && *f)
            {
                //std::cout << "CFormFree: functor curl_formfree: " << *f <<  std::endl;
                curl_formfree(*f);
            }
        }
        catch(...){}
    }
};
///CFormSharedPtr factory
typedef CFormPtrT<CFormFree> CurlFormFreePtr;
///usage CFormSharedPtr  curl_form_guard = CurlFormFreePtr(formpost);

///login getting sessionid
void hp_form_overeni(curl_httppost **formpost_pp //out parameter
        , const std::string& loginame
        , const std::string& password
        , const std::string& standzak //"hpcb_Jednorazova_zakazka"
        , const std::string& poznamka //"Testovaci prenos!!!"
        , const std::string& jobzak //"2010"
        , const std::string& verzeOS //"Linux"
        , const std::string& verzeProg //"20100315001"
        );

///file upload with order number and crc32 checksum
void hp_form_command(curl_httppost **formpost_pp //out parameter
        , const std::string& pocetupl //decremented number of file
        , const std::string& file_crc //crc32 checksum
        , const std::string& filename_to_upload //filename
        );

///file upload from buffer with order number and crc32 checksum
void hp_form_command_buffer(curl_httppost **formpost_pp //out parameter
        , const std::string& pocetupl //decremented number of file
        , const std::string& file_crc //crc32 checksum
        , const std::string& file_name //file name
        , const char * file_buffer //pointer to file data in memory
        , const std::size_t file_buffer_length //size of file data
        );

///upload end
void hp_form_konec(curl_httppost **formpost_pp //out parameter
        , const std::string& soubor //first file name
        , const std::string& pocetsouboru //files number
        , const std::string& stav //OK / KO
        );

///failure errorlog upload attempt with storno flag set
void hp_prubeh_command(curl_httppost **formpost_pp //out parameter
        , const std::string& filename_to_upload //failure errorlog filename
        );

void hp_form_prehledzak(curl_httppost **formpost_pp //out parameter
        , const std::string& user
        , const std::string& passwd
        , const std::string& typ //txt csv
        //enter cislozak or datum
        //if filled both "cislozak" and "datum" , "cislozak" will be used
        , const std::string& cislozak //batch number
        , const std::string& datum = std::string("") //date in format yyyymmdd, default none
        );

#endif
