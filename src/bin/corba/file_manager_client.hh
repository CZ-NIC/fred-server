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

