/*
 * Copyright (C) 2011  CZ.NIC, z.s.p.o.
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
 *  @file_params.h
 *  header of file client implementation
 */

#ifndef FILE_PARAMS_H_
#define FILE_PARAMS_H_

#include "util/types/optional.h"

/**
 * \class FileListArgs
 * \brief admin client file_list
 */
struct FileListArgs
{
    optional_id id;
    optional_string name_name;
    optional_string crdate;
    optional_string path;
    optional_string mime;
    optional_ulonglong size;
    optional_ulong file_type;
    unsigned long long limit;

    FileListArgs()
    : limit(0)
    {}//ctor
    FileListArgs(
            const optional_id& _id
            , const optional_string& _name_name
            , const optional_string& _crdate
            , const optional_string& _path
            , const optional_string& _mime
            , const optional_ulonglong& _size
            , const optional_ulong& _file_type
            , const unsigned long long _limit
            )
    : id(_id)
    , name_name(_name_name)
    , crdate(_crdate)
    , path(_path)
    , mime(_mime)
    , size(_size)
    , file_type(_file_type)
    , limit(_limit)
    {}//init ctor
};//struct FileListArgs

#endif // FILE_PARAMS_H_
