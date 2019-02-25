/*
 * Copyright (C) 2009-2019  CZ.NIC, z. s. p. o.
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
 *  @file file_transferer.h
 *  Interface for transfering files between client and server
 */

#ifndef FILE_TRANSFERER_HH_A90C3634F4A44F07A17A3582C2F42B9C
#define FILE_TRANSFERER_HH_A90C3634F4A44F07A17A3582C2F42B9C

#include <memory>
#include <string>

namespace LibFred {
namespace File {

class Transferer {
public:
    virtual ~Transferer()
    {
    }

    virtual unsigned long long upload(const std::string &_name,
                                      const std::string &_mime_type,
                                      const unsigned int &_file_type) = 0;

    virtual void download(const unsigned long long _id,
                  std::vector<char> &_out_buffer) = 0;

    virtual unsigned long long upload(std::vector<char> &_in_buffer
                              , const std::string &_name
                              , const std::string &_mime_type
                              , const unsigned int &_file_type) = 0;

};

typedef std::unique_ptr<Transferer> TransfererPtr;

}
}


#endif /*FILE_TRANSFER_H_*/

