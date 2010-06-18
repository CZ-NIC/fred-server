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
 *  @hp.h
 *  hybrid postservice communication header
 */

#ifndef HP_H_
#define HP_H_


#include <cstdio>
#include <iostream>
#include <memory>
#include <string>

#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

#include <curl/curl.h>

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
typedef boost::shared_ptr<CURL> CURLSharedPtr;
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
        );

///curl_httppost form free
///actual pointer may change so pointer to pointer usage
typedef boost::shared_ptr< curl_httppost* > CFormSharedPtr;
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

///second log interface
void hp_form_infolog2(curl_httppost **formpost_pp //out parameter
        , const std::string& text //"Pocet souboru uvedenych v hpcmd.cfg je: "
        , const std::string& cislo //files number
        , const std::string& chyba //0
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

#endif // HP_H_
