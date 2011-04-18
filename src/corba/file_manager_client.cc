#include <stdexcept>
#include <fstream>
#include <vector>

#include <boost/lexical_cast.hpp>

#include "file_manager_client.h"
#include "log/logger.h"


std::string get_basename(const std::string &_path)
{
    std::string::size_type l = _path.length();
    std::string::size_type b = _path.find_last_of('/');
    if (b >= l) {
        return _path;
    }
    else {
        return std::string(_path, b + 1, l - b - 1);
    }
}


FileManagerClient::FileManagerClient(NameService *_ns) : ns_(_ns)
{
    try {
        _resolve();
    }
    catch (...) {
    }
}


unsigned long long FileManagerClient::upload(const std::string &_name,
                                             const std::string &_mime_type,
                                             const unsigned int &_file_type)
{
    Logging::Manager::instance_ref().get(PACKAGE).debug(std::string("FileManagerClient::upload _name: ")
    +_name + std::string(" _mime_type: ") + _mime_type + std::string(" _file_type: ")
     + boost::lexical_cast<std::string>(_file_type));


    try {
        int chunk_size = 16384;
        typedef std::vector<char> chunk_t;

        _resolve();
        /* contact file manager and get upload object */
        std::string label = get_basename(_name);
        ccReg::FileUpload_var uploader = fmanager_->save(label.c_str(),
                                                         _mime_type.c_str(),
                                                         static_cast<CORBA::Short>(_file_type));

        Logging::Manager::instance_ref().get(PACKAGE).debug(std::string("FileManagerClient::upload saved"));

        std::ifstream input(_name.c_str(), std::ios::in);
        if (input.is_open()) {
            while (!input.eof()) {
                chunk_t chunk(chunk_size);
                input.read(&chunk[0], chunk_size);
                std::streamsize read_size = input.gcount();

                CORBA::Octet *buffer = ccReg::BinaryData::allocbuf(read_size);
                for (int i = 0; i < read_size; ++i) {
                    buffer[i] = chunk[i];
                }
                ccReg::BinaryData data(read_size, read_size, buffer, 1);
                uploader->upload(data);

                Logging::Manager::instance_ref().get(PACKAGE).debug(std::string("FileManagerClient::upload data"));
            }
            input.close();

           ::CORBA::Long ret = uploader->finalize_upload();

            Logging::Manager::instance_ref().get(PACKAGE).debug(std::string("FileManagerClient::upload finalize_upload: ")
            +boost::lexical_cast<std::string>(ret));

            return ret;
        }

        throw std::runtime_error(str(boost::format("File upload failed: "
                        "cannot open file '%1%' (mimetype=%2% filetype=%3%)")
                        % _name % _mime_type % _file_type));
    }
    catch (std::exception &ex) {
        throw std::runtime_error(str(boost::format("File upload failed: %1%")
                                     % ex.what()));
    }
    catch (...) {
        throw std::runtime_error("File upload failed: unknown error");
    }
}


void FileManagerClient::download(const unsigned long long _id
        , std::vector<char> &_out_buffer)
{
    try
    {
        _resolve();
        ccReg::FileInfo_var info = fmanager_->info(_id);//get info
        _out_buffer.clear();
        _out_buffer.reserve(info->size);//allocate buffer
        ccReg::FileDownload_var downloader = fmanager_->load(_id);
        ccReg::BinaryData_var file_data(new ccReg::BinaryData);
        file_data = downloader->download(info->size);

        if(file_data->length() !=  info->size)
            throw std::runtime_error(
                    "FileManagerClient::download error: size check failed");

        //there is some server side GC in case of failure
        downloader->finalize_download();
        //copy data
        for(std::size_t i = 0; i < info->size; ++i)
            _out_buffer.push_back(file_data[i]);
    }
    catch (std::exception &ex)
    {
        throw std::runtime_error(str(boost::format("File download failed: %1%")
                                     % ex.what()));
    }
    catch (...)
    {
        throw std::runtime_error("File download failed: unknown error");
    }
}//FileManagerClient::download

unsigned long long FileManagerClient::upload( std::vector<char> &_in_buffer
                                            , const std::string &_name
                                            , const std::string &_mime_type
                                            , const unsigned int &_file_type)
{
    try
    {
        _resolve();
        /* contact file manager and get upload object */
        std::string label = get_basename(_name);
        ccReg::FileUpload_var uploader = fmanager_->save(label.c_str()
                                , _mime_type.c_str()
                                , static_cast<CORBA::Short>(_file_type));

        std::size_t read_size = _in_buffer.size();

        ccReg::BinaryData data(read_size, read_size
            , reinterpret_cast<CORBA::Octet*>( &_in_buffer[0])
            , 0);

        uploader->upload(data);
        return uploader->finalize_upload();
    }
    catch (std::exception &ex)
    {
        throw std::runtime_error(str(boost::format("File upload failed: %1%")
                                     % ex.what()));
    }
    catch (...)
    {
        throw std::runtime_error("File upload failed: unknown error");
    }
}


void FileManagerClient::_resolve() {
    try {
        boost::mutex::scoped_lock scoped_lock(mutex_);

        if (CORBA::is_nil(fmanager_)) {
            CORBA::Object_var object = ns_->resolve("FileManager");
            fmanager_ = ccReg::FileManager::_narrow(object);
            if (CORBA::is_nil(fmanager_)) {
                throw std::runtime_error("cannot resolve file manager object");
            }
        }
    }
    catch (...) {
        throw std::runtime_error("file manager client initialization error");
    }
}
