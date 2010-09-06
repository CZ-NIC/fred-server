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
 *  @messages_filemanager.h
 *  corba client header for registry messages
 */



#ifndef MESSAGES_CORBA_IMPL_H_
#define MESSAGES_CORBA_IMPL_H_

#include <vector>
#include <string>

unsigned long long save_file(std::vector<char> & file_buffer
        , const std::string file_name, const std::string file_mime_type
        , const unsigned int file_type );

#endif // MESSAGES_CORBA_IMPL_H_
