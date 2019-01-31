#ifndef FILE_MANAGER_CLIENT_HH_33AF71E64B704234AC6C8F23FB27C8F7
#define FILE_MANAGER_CLIENT_HH_33AF71E64B704234AC6C8F23FB27C8F7

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

#include "src/bin/corba/FileManager.hh"
#include "src/bin/corba/nameservice.hh"
#include "src/deprecated/libfred/file_transferer.hh"


class FileManagerClient : public LibFred::File::Transferer
{
public:
    FileManagerClient(NameService *_ns);

    unsigned long long upload(const std::string &_name,
                              const std::string &_mime_type,
                              const unsigned int &_file_type);


    void download(const unsigned long long _id
            , std::vector<char> &_out_buffer);

    unsigned long long upload(std::vector<char> &_in_buffer
                              , const std::string &_name
                              , const std::string &_mime_type
                              , const unsigned int &_file_type);

private:
    NameService            *ns_;
    ccReg::FileManager_var  fmanager_;

    boost::mutex            mutex_;

    void _resolve();
};

#endif /*FILE_MANAGER_CLIENT_H_*/

