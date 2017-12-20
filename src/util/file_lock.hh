#ifndef FILE_LOCK_H__
#define FILE_LOCK_H__

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

