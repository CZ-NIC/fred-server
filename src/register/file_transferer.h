/*
 * Copyright (C) 2007  CZ.NIC, z.s.p.o.
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
 *  @file file_transferer.h
 *  Interface for transfering files between client and server
 */

#ifndef FILE_TRANSFER_H_
#define FILE_TRANSFER_H_

#include <memory>
#include <string>

namespace Register {
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
                          const std::string &_path) = 0;
};

typedef std::auto_ptr<Transferer> TransfererPtr;

}
}


#endif /*FILE_TRANSFER_H_*/

