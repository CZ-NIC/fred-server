/*
 * Copyright (C) 2011-2019  CZ.NIC, z. s. p. o.
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
#ifndef FILE_LOCK_HH_0CACDBF88C5E454A82B847BDAD863064
#define FILE_LOCK_HH_0CACDBF88C5E454A82B847BDAD863064

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdexcept>



struct FileLockError : public std::runtime_error
{
public:
    FileLockError(const std::string &_lockname, const std::string &_errmsg) :
        std::runtime_error("cannot acquire file lock "
                + _lockname + " reason (" + _errmsg + ")")
    {
    }
};


class FileLock
{
public:
    FileLock(const std::string &_lockpath)
    {
        fl.l_type = F_WRLCK;
        fl.l_whence = SEEK_SET;
        fl.l_start = 0;
        fl.l_len = 1;
        fl.l_pid = 0;

        if ((fdlock = open(_lockpath.c_str(), O_WRONLY|O_CREAT, 0666)) == -1) {
            throw FileLockError(_lockpath, "open call");
        }

        if (fcntl(fdlock, F_SETLK, &fl) == -1) {
            throw FileLockError(_lockpath, "fcntl call");
        }
    }


private:
    int fdlock;
    flock fl;
};



#endif /*FILE_LOCK_H__*/

