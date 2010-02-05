#ifndef FILE_MANAGER_CLIENT_H_
#define FILE_MANAGER_CLIENT_H_

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

#include "corba/ccReg.hh"
#include "nameservice.h"
#include "register/file_transferer.h"


class FileManagerClient : public Register::File::Transferer
{
public:
    FileManagerClient(NameService *_ns);

    unsigned long long upload(const std::string &_name,
                              const std::string &_mime_type,
                              const unsigned int &_file_type);

    void download(const unsigned long long _id,
                  const std::string &_path);


private:
    NameService            *ns_;
    ccReg::FileManager_var  fmanager_;

    boost::mutex            mutex_;

    void _resolve();
};


#endif /*FILE_MANAGER_CLIENT_H_*/

