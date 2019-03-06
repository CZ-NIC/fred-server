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
/**
 *  @messages_filemanager.cc
 *  corba client imementation for registry messages
 */

#include <vector>
#include <string>

#include "src/bin/corba/messages/messages_filemanager.hh"
#include "src/util/corba_wrapper_decl.hh"

#include "src/bin/corba/FileManager.hh"
#include "src/bin/corba/file_manager_client.hh"

unsigned long long save_file(std::vector<char>& file_buffer
        , const std::string file_name, const std::string file_mime_type
        , const unsigned int file_type )
{
    FileManagerClient fm_client(CorbaContainer::get_instance()->getNS());

    unsigned long long file_id
            = fm_client.upload(file_buffer,file_name,file_mime_type,file_type);

    return file_id;
}
