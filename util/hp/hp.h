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

#include <cstdlib>
#include <exception>
#include <string>
#include <memory>
#include <stdexcept>
#include <fstream>
#include <ios>
#include <iostream>
#include <sstream>
#include <vector>


#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>


/**
 * \class StringBuffer
 * \brief global string buffer for post result returned by post write callback
 */
class StringBuffer : boost::noncopyable
{
    std::string buffer_;
    static std::auto_ptr<StringBuffer> instance_ptr;
    friend class std::auto_ptr<StringBuffer>;
protected:
    ~StringBuffer(){}
private:
    StringBuffer()
    {
        buffer_.clear();
    }
public:
    static StringBuffer* set();
    static StringBuffer* get();

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
            if(c) curl_easy_cleanup(c);
        }
        catch(...){}
    }
};
///CURLSharedPtr factory
typedef CURLPtrT<CurlEasyCleanup> CurlEasyCleanupPtr;
///usage CURLSharedPtr  curl_easy_cleanup_guard = CurlEasyCleanupPtr(curl_easy_init());


///FILE easy cleanup
typedef boost::shared_ptr<FILE> FILESharedPtr;
template < typename DELETER >
class FILEPtrT
{
protected:
    FILESharedPtr m_ptr;
public:
    FILEPtrT(FILE* f) : m_ptr(f,DELETER()) {}
    FILEPtrT() : m_ptr(0,DELETER()) {}

    operator FILESharedPtr() const
    {
        return m_ptr;
    }
};
///deleter functor for FILE calling fclose
struct FileClose
{
    void operator()(FILE* f)
    {
        try
        {
            if(f) fclose(f);
        }
        catch(...){}
    }
};
///FILESharedPtr factory
typedef FILEPtrT<FileClose> FileClosePtr;
///usage FILESharedPtr  file_close_guard = FileClosePtr(fopen());



///send RFC2388 http post request for postservice
CURLcode hp_form_post(struct curl_httppost *form  //linked list ptr
        , const std::string& curlopt_url //url
        , const std::string& curlopt_capath //ended by slash
        , const std::string& curlopt_cookie //cookie NAME=CONTENTS, Set multiple cookies in one string like this: "name1=content1; name2=content2;" PHPSESSID=6d8cbbd1e53b15aa0523f4579612f940;
        , const std::string& curlopt_useragent //header "CommandLine klient HP"
        , long curlopt_verbose // 0 / 1
        , FILE* curl_log_file = 0 //curl log file
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
            if(f && *f) curl_formfree(*f);
        }
        catch(...){}
    }
};
///CFormSharedPtr factory
typedef CFormPtrT<CFormFree> CurlFormFreePtr;
///usage CFormSharedPtr  curl_form_guard = CurlFormFreePtr(formpost);

///login getting sessionid
void hp_form_overeni(curl_httppost **formpost_pp //out parameter
        , const std::string& loginame //"dreplech"
        , const std::string& password //"dreplech"
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



/**
 * \class HPMail
 * \brief interface for uploading to hybrid postservice
 *  singleton not usable in multiple threads, using curl easy interface
 */

typedef std::vector<char> MailFile;//one "letter" file data
typedef std::vector<MailFile> MailBatch;//all batch files

class HPMail : boost::noncopyable
{
    std::string mb_proc_tmp_dir_; //tmp dir for 7z files ended by slash
    std::string postservice_cert_dir_; //postsignum_qca_root.pem server certificate dir ended by slash
    std::string phpsessid_; //PHP session id
    FILESharedPtr curl_log_file_guard_;//curl log file
    std::string hp_interface_url_;// ended by slash like: https://online.postservis.cz/Command/
    std::string hp_batch_number_; // batch number generated by login

    static std::auto_ptr<HPMail> instance_ptr;
    friend class std::auto_ptr<HPMail>;
protected:
    ~HPMail(){}//dtor
private:
    //ctor
    HPMail(const std::string& mb_proc_tmp_dir //tmp dir for 7z files ended by slash
            , const std::string& postservice_cert_dir //postsignum_qca_root.pem server certificate dir ended by slash
            , const std::string& hp_interface_url // ended by slash like: https://online.postservis.cz/Command/
            )
    : mb_proc_tmp_dir_(mb_proc_tmp_dir)
    , postservice_cert_dir_(postservice_cert_dir)
    , phpsessid_("") //initially empty, filled by succesful login
    , curl_log_file_guard_(fopen((mb_proc_tmp_dir+"curl_stderr.log").c_str(),"w"))
    , hp_interface_url_(hp_interface_url)
    , hp_batch_number_("") //initially empty, generated by succesful login
    {}
public:
    static HPMail* set(const std::string& mb_proc_tmp_dir //tmp dir for 7z files ended by slash
            , const std::string& postservice_cert_dir //postsignum_qca_root.pem server certificate dir ended by slash
            , const std::string& hp_interface_url // ended by slash like: https://online.postservis.cz/Command/
            );
    static HPMail* get();

    void login(const std::string& loginame //postservice account name
        , const std::string& password //postservice account password
        , const std::string& batch_id //batch identificator like: "hpcb_Jednorazova_zakazka"
        , const std::string& batch_note //batch note like: "Testovaci prenos!!!" or anything else you need to tell here
        );

    void upload( const MailBatch& mb);


};//class HPMail
#endif // HP_H_
