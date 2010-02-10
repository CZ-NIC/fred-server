#include <stdexcept>
#include <fstream>
#include <vector>

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
    try {
        int chunk_size = 16384;
        typedef std::vector<char> chunk_t;

        _resolve();
        /* contact file manager and get upload object */
        std::string label = get_basename(_name);
        ccReg::FileUpload_var uploader = fmanager_->save(label.c_str(),
                                                         _mime_type.c_str(),
                                                         static_cast<CORBA::Short>(_file_type));

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
            }
            input.close();
            return uploader->finalize_upload();
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


void FileManagerClient::download(const unsigned long long _id,
                                 const std::string &_path)
{
    throw std::runtime_error("not implemented");
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
