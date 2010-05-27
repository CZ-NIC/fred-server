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
 *  @hp.cc
 *  implementation of communication with hybrid postservice
 */

#include "hp.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/crc.hpp>


//class StringBuffer
///static instance init
std::auto_ptr<StringBuffer> StringBuffer::instance_ptr(0);

///instance setter
StringBuffer* StringBuffer::set()
{
    std::auto_ptr<StringBuffer>
    tmp_instance(new StringBuffer);
    instance_ptr = tmp_instance;
    if (instance_ptr.get() == 0)
        throw std::runtime_error(
                "StringBuffer::set_instance error: instance not set");
    return instance_ptr.get();
}

///instance  getter
StringBuffer* StringBuffer::get()
{
    StringBuffer* ret = instance_ptr.get();
    if (ret == 0)
    {
        set();
        ret = instance_ptr.get();
        if (ret == 0)  throw std::runtime_error(
                "StringBuffer::get_instance error: instance not set");
    }
    return ret;
}

///buffer append
void StringBuffer::append(std::string & str)
{
    for(std::string::iterator i = str.begin(); i != str.end() ; ++i)
        if (*i == '\0') *i = ' ';//replace null chars for spaces before append

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

///php invented curl option CURLOPT_RETURNTRANSFER replacement callback
 static size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
 {
     size_t buffer_size = size * nmemb;//number of bytes in buffer

     //copy data from buffer into std string
     std::string data(static_cast<char *>(buffer), buffer_size);
     StringBuffer::get()->append(data);//append to StringBuffer
     return buffer_size; //count bytes taken care of
 }

CURLcode hp_form_post(struct curl_httppost *form  //linked list ptr
        , const std::string& curlopt_url //url
        , const std::string& curlopt_capath //ended by slash
        , const std::string& curlopt_cookie //cookie NAME=CONTENTS, Set multiple cookies in one string like this: "name1=content1; name2=content2;" PHPSESSID=6d8cbbd1e53b15aa0523f4579612f940;
        , const std::string& curlopt_useragent //header "CommandLine klient HP"
        , long curlopt_verbose // 0 / 1
        , FILE* curl_log_file //curl log file
        )
{
    CURLSharedPtr  curl_easy_guard = CurlEasyCleanupPtr(curl_easy_init());
    CURLcode ret = CURLE_FAILED_INIT;
    CURL* curl = curl_easy_guard.get();

    std::string curlopt_cainfo(curlopt_capath+"postsignum_qca_root.pem");
    if(curl)//CURL*
    {
        curl_easy_setopt(curl, CURLOPT_VERBOSE, curlopt_verbose);//talk to me
        if(curl_log_file)//use stderr redirected to logfile if not null
            curl_easy_setopt(curl, CURLOPT_STDERR , curl_log_file);

        curl_easy_setopt(curl, CURLOPT_HEADER, 1);//out header
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);//use location

        //CURLOPT_RETURNTRANSFER
        StringBuffer::set();//reset recv buffer
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);

        //to validate against stored certificate
        // set the file with the certs validating the server
        curl_easy_setopt(curl,CURLOPT_CAINFO,curlopt_cainfo.c_str());//"./cert/postsignum_qca_root.pem"
        curl_easy_setopt(curl,CURLOPT_CAPATH,curlopt_capath.c_str());//"./cert/"
        // cert is stored PEM coded in file...
        // since PEM is default, we needn't set it for PEM
        curl_easy_setopt(curl,CURLOPT_SSLCERTTYPE,"PEM");
        // do not verify the authenticity of the peer's certificate
        curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER,0L);
        //server sends a certificate indicating its identity
        //that certificate must indicate that the server is the server
        //to which you meant to connect, or the connection fails.
        curl_easy_setopt(curl,CURLOPT_SSL_VERIFYHOST,2L);
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
        , const std::string& loginame //"dreplech"
        , const std::string& password //"dreplech"
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

void hp_form_infolog2(curl_httppost **formpost_pp //out parameter
        , const std::string& text //"Pocet souboru uvedenych v hpcmd.cfg je: "
        , const std::string& cislo //files number
        , const std::string& chyba //0
        )
{
    struct curl_httppost *lastptr=NULL;
    // Fill in overeni
    curl_formadd(formpost_pp, &lastptr, CURLFORM_COPYNAME, "cc"
            , CURLFORM_COPYCONTENTS, "us \n", CURLFORM_END);
    curl_formadd(formpost_pp, &lastptr, CURLFORM_COPYNAME, "text"
            , CURLFORM_COPYCONTENTS, (text + "\n").c_str(), CURLFORM_END);
    curl_formadd(formpost_pp, &lastptr, CURLFORM_COPYNAME, "cislo"
            , CURLFORM_COPYCONTENTS, (cislo  + "\n").c_str(), CURLFORM_END);
    curl_formadd(formpost_pp, &lastptr, CURLFORM_COPYNAME, "chyba"
            , CURLFORM_COPYCONTENTS, (chyba + "\n").c_str(), CURLFORM_END);
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

//class HPMail implementation

///static instance init
std::auto_ptr<HPMail> HPMail::instance_ptr(0);

///instance  setter
HPMail* HPMail::set(const std::string& mb_proc_tmp_dir //tmp dir for 7z files ended by slash
        , const std::string& postservice_cert_dir //postsignum_qca_root.pem server certificate dir ended by slash
        , const std::string& hp_interface_url // ended by slash like: https://online.postservis.cz/Command/
        )
{
    std::auto_ptr<HPMail> tmp_instance(new HPMail(mb_proc_tmp_dir
            , postservice_cert_dir, hp_interface_url ));
    instance_ptr = tmp_instance;
    if (instance_ptr.get() == 0)
        throw std::runtime_error(
                "HPMail::set error: instance not set");
    return instance_ptr.get();
}

///instance  getter
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


/*
/// upload batch of mail files to postservice
void upload_batch( MailBatch mb //mail files data
*/
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
            , "2010" //jobzak may be year, is set in orig config file like "2010"
            , "Linux" //verzeOS
            , "20100315001" //verzeProg orig was "20100315001"
            );

    StringBuffer::set();//reset recv buffer, userp may be better
    CURLcode res = hp_form_post(formpost_overeni  //linked list ptr
                , hp_interface_url_+"over.php" //form url
                , postservice_cert_dir_ //ended by slash
                , "" //no cookie
                , "" //no useragent
                , 1 //verbose
                , curl_log_file_guard_.get() //curl logfile
                );

    if (res > 0)
    {
        throw std::runtime_error(
                std::string("HPMail::login error: login form failed: ")
                    + curl_easy_strerror(res));
    }

    //log result
        std::string form_reply("\n\nover reply: \n"
                + StringBuffer::get()->copy());
        fwrite (form_reply.c_str() , 1, form_reply.size()
                , curl_log_file_guard_.get() );

    //result parsing
    phpsessid_ =  StringBuffer::get()->getValueByKey("PHPSESSID=", 32);
    hp_batch_number_ = StringBuffer::get()->getValueByKey("cislozakazky", 12) ;

    //detecting errors
    if (hp_batch_number_.empty())
        throw std::runtime_error(std::string("HPMail::login error: empty batch number"));
    if (StringBuffer::get()->getValueByKey("overenizak ", 2).compare("KO")==0)
        throw std::runtime_error(std::string("HPMail::login error: batch validation failed"));
    if (StringBuffer::get()->getValueByKey("zaladr ", 2).compare("KO") == 0)
        throw std::runtime_error(std::string("HPMail::login error: mkdir failed"));
    if (StringBuffer::get()->getValueByKey("Overeni ", 2).compare("OK") != 0)
        throw std::runtime_error(std::string("HPMail::login error: login failed"));
    if (StringBuffer::get()->getValueByKey("UDRZBA", 2).compare("on") == 0)
        throw std::runtime_error(std::string("HPMail::login error: server out of order"));
}

/// upload batch of mail files to postservice
void HPMail::upload( const MailBatch& mb)
{
    if(phpsessid_.empty())
        throw std::runtime_error("HPMail::upload error: not logged in");

    typedef std::vector<std::string> LetterFileNames;
    LetterFileNames letter_file_names;//list file for 7z

    //save letter data to disk like: letter_<number>
    for (unsigned i=0; i < mb.size(); ++i)
    {
        std::string letter_file_name(
                "letter_"
                +boost::lexical_cast<std::string>(i));
        std::ofstream letter_file;
        letter_file.open ((mb_proc_tmp_dir_+letter_file_name).c_str()
                , std::ios::out | std::ios::binary);
        if(letter_file.is_open())
            {
                letter_file.write(&mb[i][0], mb[i].size());
                letter_file_names.push_back(letter_file_name);
            }
    }//for mb files

    //save list of letter files
    std::string file_list_name("in.lst");
    std::ofstream list_file;
    list_file.open ((mb_proc_tmp_dir_+file_list_name).c_str(), std::ios::out );
    if(list_file.is_open())
    {
        for(LetterFileNames::iterator i = letter_file_names.begin()
                ; i != letter_file_names.end(); ++i )
        {
            list_file << *i << "\n";
        }
    }
    list_file.close();//flush

    //7z command
    std::string command_for_7z (
            "cd " + mb_proc_tmp_dir_ //cd to set tmpdir
            //compress letters to archive volume files
            //like: <hp_batch_number>.7z.001, ...002 ...
            + " && 7z a "+hp_batch_number_+".7z @in.lst -mx5 -v5m -mmt=on");
    int system_command_retcode =
    system(command_for_7z.c_str());//execute

    if(system_command_retcode != 0)
        throw std::runtime_error(
                "HPMail::upload error: system command: "
                + command_for_7z
                + "failed with retcode: "
                + boost::lexical_cast<std::string>(system_command_retcode));

    //load 7z compressed mail batch
    MailBatch compressed_mail_batch;

    for(unsigned i = 1; i < std::numeric_limits<unsigned>::max(); ++i)
    {
        std::stringstream order_number;
        if(i < 1000)
            order_number << std::setfill('0') << std::setw(3) << i;
        else
            order_number << i; //TODO: check behaviour over 1000 volumes
        std::string compressed_mail_volume_name(
                mb_proc_tmp_dir_+hp_batch_number_+".7z."+order_number.str());

        std::ifstream compressed_mail_volume_stream;
        compressed_mail_volume_stream.open (compressed_mail_volume_name.c_str()
            , std::ios::in | std::ios::binary);

        if(compressed_mail_volume_stream.is_open())
        {//ok file is there
            compressed_mail_batch.push_back(MailFile());//add mail arch volume
            // get length of file
            compressed_mail_volume_stream.seekg (0, std::ios::end);
            long long compressed_mail_volume_length =
                    compressed_mail_volume_stream.tellg();
            compressed_mail_volume_stream.seekg (0, std::ios::beg);//reset
            //get new MailFile at the end of batch
            MailFile& mail_archive_volume = *(--(compressed_mail_batch.end()));
            //allocate buffer
            mail_archive_volume.resize(compressed_mail_volume_length,'\0');
            //read whole file into the buffer
            compressed_mail_volume_stream.read( &mail_archive_volume[0]
                , compressed_mail_volume_length );
        }
        else //no more files
            break;//for i loop
    }//for i

    if(compressed_mail_batch.size() < 1)
        throw std::runtime_error(
                "HPMail::upload error: compressed_mail_batch.size() < 1");

    //upload to postservice
    for(std::size_t i = compressed_mail_batch.size(); i > 0; --i)
    {
        //get file buffer
        MailFile& mail_archive_volume = compressed_mail_batch[i-1];

        //crc32 checksum into string
        boost::crc_32_type  crc32_checksum;
        crc32_checksum.process_bytes( &mail_archive_volume[0]
                                      , mail_archive_volume.size());
        std::stringstream crc32_string;
        crc32_string << std::hex << std::uppercase
                << crc32_checksum.checksum() << std::flush;

        //archive volume file name
        std::stringstream order_number;
        if(i < 1000)
            order_number << std::setfill('0') << std::setw(3) << i << std::flush;
        else
            order_number << i; //TODO: check behaviour over 1000 volumes
        std::string compressed_mail_volume_name(
                hp_batch_number_+".7z."+order_number.str());

        //make form for upload
        struct curl_httppost *formpost_command=NULL;
        CFormSharedPtr  form_command_guard = CurlFormFreePtr(&formpost_command);
        ///file upload from buffer with order number and crc32 checksum
        hp_form_command_buffer(&formpost_command //out parameter
                , boost::lexical_cast<std::string>(i) //decremented number of file
                , crc32_string.str()  //crc32 checksum
                , compressed_mail_volume_name //file name
                , &mail_archive_volume[0] //pointer to file data
                , mail_archive_volume.size() //size of file data
                );

        //send form
        StringBuffer::set();//reset recv buffer, userp may be better
        CURLcode res = hp_form_post(formpost_command  //linked list ptr
                    , hp_interface_url_+"command.php" //url
                    , postservice_cert_dir_ //ended by slash
                    , "PHPSESSID="+phpsessid_//PHP session id in cookie
                    , "CommandLine klient HP" //useragent id
                    , 1 //verbose
                    , curl_log_file_guard_.get()//curl logfile
                    );

        if (res > 0)
        {
            throw std::runtime_error(
                    std::string("HPMail::upload error: form post command failed: ")
                        + curl_easy_strerror(res));
        }

        //log result
        std::stringstream formpost_reply;
            formpost_reply << "\n\nCommand reply: \n"
                << StringBuffer::get()->copy()
                <<  "\n" << "crc32: "
                << crc32_string.str()
                << "\nfile number: "
                << boost::lexical_cast<std::string>(i)
                << std::endl;
        fwrite (formpost_reply.str().c_str() , 1
                , formpost_reply.str().size() , curl_log_file_guard_.get() );

        //result parsing & detecting errors
        if ((StringBuffer::get()->getValueByKey("OvereniCrc ", 2)).compare("KO") == 0)
                    throw std::runtime_error(std::string("HPMail::upload error: crc check failed"));
        if ((StringBuffer::get()->getValueByKey("zaladr ", 2)).compare("KO") == 0)
            throw std::runtime_error(std::string("HPMail::upload error: mkdir failed"));
        //will never happen
        if ((StringBuffer::get()->getValueByKey("UDRZBA", 2)).compare("on") == 0)
            throw std::runtime_error(std::string("HPMail::upload error: server out of order"));

    }//for compressed_mail_batch


    //end of batch form
    struct curl_httppost *formpost_konec=NULL;
    CFormSharedPtr  form_konec_guard = CurlFormFreePtr(&formpost_konec);

    //first archive volume file name
    std::string first_compressed_mail_volume_name(
            hp_batch_number_+".7z.001");

    //fill in konec
    hp_form_konec(&formpost_konec //out parameter
            , first_compressed_mail_volume_name
            , boost::lexical_cast<std::string>(compressed_mail_batch.size()) //number of files
            , "OK" //status
            );

    StringBuffer::set();//reset recv buffer, userp may be better
    CURLcode res = hp_form_post(formpost_konec  //linked list ptr
                , hp_interface_url_+"konec.php" //url
                , postservice_cert_dir_ //ended by slash
                , "PHPSESSID="+phpsessid_//PHP session id in cookie
                , "CommandLine klient HP" //useragent id
                , 1 //verbose
                , curl_log_file_guard_.get()//curl logfile
                );

    if (res > 0)
    {
        throw std::runtime_error(
                std::string("form post konec failed: ")
                    + curl_easy_strerror(res));
    }

    //result parsing & detecting errors
    if ((StringBuffer::get()->getValueByKey("zaladr ", 2)).compare("KO") == 0)
        throw std::runtime_error(std::string("zaladr is KO"));
    //will never happen
    if ((StringBuffer::get()->getValueByKey("UDRZBA", 6)).compare("on") == 0)
        throw std::runtime_error(std::string("udrzba on"));


    //log result
    std::stringstream formpost_reply;
        formpost_reply << "\nKonec reply: \n" << StringBuffer::get()->copy()
                            <<  "\n" << std::endl;
    fwrite (formpost_reply.str().c_str() , 1
            , formpost_reply.str().size() , curl_log_file_guard_.get() );

    instance_ptr.reset(0);//end of session

}//HPMail::upload



