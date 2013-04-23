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
 *  @file subprocess.h
 *  sub-process related utils
 */


#ifndef SUBPROCESS_H_
#define SUBPROCESS_H_

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/select.h>
#include <string.h>
#include <fcntl.h>

#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <csignal>
#include <unistd.h>
#include <cstring>
#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cassert>
#include <iterator>

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time.hpp>


#include "config.h"
#ifndef HAVE_LOGGER
#error HAVE_LOGGER is required!
#endif
#include "log/logger.h"


///fd reference close guard
class FdCloseGuard
: public boost::noncopyable
{
    int fd_;
public:
    FdCloseGuard()
        : fd_(-1)
        {}
    explicit FdCloseGuard(int fd_ref)
    : fd_(fd_ref)
    {}
    ~FdCloseGuard()
    {
        closefd();
    }

    int closefd()
    {
        int ret = -2;
        if(fd_ > -1)
        {
            ret = close(fd_);
            fd_ = -1;
        }
        return ret;
    }

    int get() const
    {
        return fd_;
    }

    void set(int fd)
    {
        fd_=fd;
    }

    bool is_closed()
    {
        return (fd_ == -1);
    }

    int set_blocking()
    {
        int flags = fcntl(fd_, F_GETFL, 0);
        if (flags == -1) return flags;
        flags &= ~O_NONBLOCK;
        return fcntl(fd_, F_SETFL, flags);
    }

    int set_nonblocking()
    {
        int flags = fcntl(fd_, F_GETFL, 0);
        if (flags == -1) return flags;
        flags |= O_NONBLOCK;
        return fcntl(fd_, F_SETFL, flags);
    }

};

//shell output
struct SubProcessOutput
{
  std::string stdout;
  std::string stderr;
};

/**
 * \class ShellCmd
 * \brief shell command wrapper, after command is entered "\n" is added
 */

class ShellCmd
{
    static const int num_of_pipes = 3;
    static const int num_of_pipe_ends = 2;
    std::string cmd_;
    std::string shell_;
    unsigned long timeout_;

    void check_timeout_kill_child(pid_t pid, boost::posix_time::ptime timeout_time_utc)
    {
        if(timeout_time_utc < boost::posix_time::microsec_clock::universal_time())//child timeout, kill child
        {
            kill(pid, SIGKILL);
            Logging::Manager::instance_ref()
                .get(PACKAGE).error(std::string("ShellCmd::operator() child timeout, kill child, command: ")+cmd_);
            pid_t dp = waitpid(pid, NULL, WNOHANG); //death pid

            if(dp == -1)
            {
                std::string err_msg(strerror(errno));
                std::string msg("ShellCmd::operator() error waitpid in timeout: ");
                throw std::runtime_error(msg+err_msg);
            }//if waitpid error

            throw std::runtime_error(std::string("ShellCmd::operator() timeout cmd: "+cmd_ ));
        }
    }

public:
    ShellCmd(const std::string& cmd)
    :cmd_(cmd)
    , shell_("/bin/sh")
    , timeout_(10)
    {}

    ShellCmd(const std::string& cmd
            , const unsigned long timeout
            )
    :cmd_(cmd)
    , shell_("/bin/sh")
    , timeout_(timeout)
    {}


    ShellCmd(const std::string& cmd
            , const std::string& shell //shell is filename, not command line
            , const unsigned long timeout //in seconds
            )
    :cmd_(cmd)
    , shell_(shell)
    , timeout_(timeout)
    {}


    ~ShellCmd(){}

    SubProcessOutput execute(std::string stdin_str = std::string())
    {
        SubProcessOutput ret;
        FdCloseGuard pfd[num_of_pipes][num_of_pipe_ends];//pipes fds

        {//init pfd
            int p[num_of_pipes][num_of_pipe_ends];//pipes fds

            //init pipe fds
            for(int i = 0; i < num_of_pipes; ++i)
                for(int j = 0; j < num_of_pipe_ends; ++j)
                    p[i][j] = -1;

            // create the pipes
            for(int i = 0; i < num_of_pipes; ++i)
            {//if the pipe is created succesfully, then readable end is p[0], and p[1] is the writable end
                if(pipe(p[i]) < 0)
                {
                    std::string pipe_error (strerror(errno));
                    throw std::runtime_error(std::string("ShellCmd::create_pipes() error creating pipe: ")
                        + boost::lexical_cast<std::string>(i)+ (" msg: ")+pipe_error);
                }
                //set fds
                pfd[i][0].set(p[i][0]);
                pfd[i][1].set(p[i][1]);
            }//for i is num_of_pipes
        }

        pid_t pid;//child pid
        pid = fork();
        if(pid == -1)
        {
          std::string err_msg(strerror(errno));
          throw std::runtime_error(std::string("ShellCmd::operator() fork error: ")+err_msg);
        }
        //parent and child now share the pipe's file descriptors
        //readable end is p[x][0], and p[x][1] is the writable end
        else if (pid == 0)
        {
            //in child
            //close writable end of stdin
            if(pfd[STDIN_FILENO][1].closefd() == -1)
            {
                std::string err_msg(strerror(errno));
                std::string msg("ShellCmd::operator() error in child closing stdin pipe 0 1: ");
                throw std::runtime_error(msg+err_msg);
            }
            //close readable end of stdout
            if(pfd[STDOUT_FILENO][0].closefd() == -1)
            {
                std::string err_msg(strerror(errno));
                std::string msg("ShellCmd::operator() error in child closing stdout pipe 1 0: ");
                throw std::runtime_error(msg+err_msg);
            }
            //close readable end of stderr
            if(pfd[STDERR_FILENO][0].closefd() == -1)
            {
                std::string err_msg(strerror(errno));
                std::string msg("ShellCmd::operator() error in child closing stderr pipe 2 0: ");
                throw std::runtime_error(msg+err_msg);
            }

            //duplicate readable end of pipe 0 into stdin
            if(pfd[STDIN_FILENO][0].get() != STDIN_FILENO)
            {
                if(dup2(pfd[STDIN_FILENO][0].get(),STDIN_FILENO) != STDIN_FILENO)
                {
                    std::string err_msg(strerror(errno));
                    std::string msg("ShellCmd::operator() error in child duplicating stdin pipe 0 0: ");
                    throw std::runtime_error(msg+err_msg);
                }
                pfd[STDIN_FILENO][0].closefd();
            }
            //duplicate writable end of pipe 1 into stdout
            if(pfd[STDOUT_FILENO][1].get() != STDOUT_FILENO)
            {
                if(dup2(pfd[STDOUT_FILENO][1].get(),STDOUT_FILENO) != STDOUT_FILENO)
                {
                    std::string err_msg(strerror(errno));
                    std::string msg("ShellCmd::operator() error in child duplicating stdout pipe 1 1: ");
                    throw std::runtime_error(msg+err_msg);
                }
                pfd[STDOUT_FILENO][1].closefd();
            }
            //duplicate writable end of pipe 2 into stderr
            if(pfd[STDERR_FILENO][1].get() != STDERR_FILENO)
            {
                if(dup2(pfd[STDERR_FILENO][1].get(),STDERR_FILENO) != STDERR_FILENO)
                {
                    std::string err_msg(strerror(errno));
                    std::string msg("ShellCmd::operator() error in child duplicating stdout pipe 2 1: ");
                    throw std::runtime_error(msg+err_msg);
                }
                pfd[STDERR_FILENO][1].closefd();
            }

            char *shell_argv[4];
            shell_argv[0] = (char*)shell_.c_str();
            shell_argv[1] = (char*)(std::string("-c").c_str());
            shell_argv[2] = (char*)cmd_.c_str();
            shell_argv[3] = NULL;


            //std::cout << "\n\nshell: " << shell_ << " cmd: " << cmd_ << std::endl;

            //shell exec
            if(execvp(shell_argv[0],shell_argv) == -1)
            {
                //failed to launch the shell
                std::string err_msg(strerror(errno));
                std::string msg("ShellCmd::operator() failed to launch shell: ");
                throw std::runtime_error(msg+ err_msg);
            }

        }//if in child
        else if(pid > 0)
        {
            //in parent
            //waitpid need default SIGCHLD handler to work
            sighandler_t sig_chld_h = signal(SIGCHLD, SIG_DFL);

            //close readable end of stdin
            if(pfd[STDIN_FILENO][0].closefd() == -1)
            {
                std::string err_msg(strerror(errno));
                std::string msg("ShellCmd::operator() error in parent closing pipe 0 0: ");
                throw std::runtime_error(msg+err_msg);
            }//check pipe fds close

            //close writable end of stdout
            if(pfd[STDOUT_FILENO][1].closefd() == -1 )
            {
                std::string err_msg(strerror(errno));
                std::string msg("ShellCmd::operator() error in parent closing pipe 1 1: ");
                throw std::runtime_error(msg+err_msg);
            }//check pipe fds close

            //close writable end of stderr
            if(pfd[STDERR_FILENO][1].closefd() == -1)
            {
                std::string err_msg(strerror(errno));
                std::string msg("ShellCmd::operator() error in parent closing pipe 2 1: ");
                throw std::runtime_error(msg+err_msg);
            }//check pipe fds close

            //set nonblocking fd
            if(pfd[STDIN_FILENO][1].set_nonblocking() == -1)
            {
                std::string err_msg(strerror(errno));
                std::string msg("ShellCmd::operator() error in parent setting stdin nonblocking: ");
                throw std::runtime_error(msg+err_msg);
            }

            if(pfd[STDOUT_FILENO][0].set_nonblocking() == -1)
            {
                std::string err_msg(strerror(errno));
                std::string msg("ShellCmd::operator() error in parent setting stdout nonblocking: ");
                throw std::runtime_error(msg+err_msg);
            }

            if(pfd[STDERR_FILENO][0].set_nonblocking() == -1)
            {
                std::string err_msg(strerror(errno));
                std::string msg("ShellCmd::operator() error in parent setting stderr nonblocking: ");
                throw std::runtime_error(msg+err_msg);
            }

            //set select timeout
            timeval tv; tv.tv_sec = 0; tv.tv_usec = 0;
            if (timeout_ > 0) tv.tv_sec = timeout_;

            boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::universal_time();
            boost::posix_time::ptime timeout_time = start_time + boost::posix_time::seconds(timeout_);

            //communication with child process
            int nfds = 0;
            std::size_t stdin_str_size_to_be_written = stdin_str.size();
            do
            {
                check_timeout_kill_child(pid, timeout_time);
                //init fd sets for reading and writing
                fd_set rfds, wfds;
                FD_ZERO(&rfds);
                FD_ZERO(&wfds);
                nfds = 0;

                //set wait for readable end of stdout
                if(!pfd[STDOUT_FILENO][0].is_closed())
                {
                    FD_SET(pfd[STDOUT_FILENO][0].get(), &rfds);
                    if(nfds < pfd[STDOUT_FILENO][0].get()) nfds = pfd[STDOUT_FILENO][0].get();
                }

                //set wait for readable end of stderr
                if(!pfd[STDERR_FILENO][0].is_closed())
                {
                    FD_SET(pfd[STDERR_FILENO][0].get(), &rfds);
                    if(nfds < pfd[STDERR_FILENO][0].get()) nfds = pfd[STDERR_FILENO][0].get();
                }

                //set wait for writable end of stdin
                if(!pfd[STDIN_FILENO][1].is_closed())
                {
                    FD_SET(pfd[STDIN_FILENO][1].get(), &wfds);
                    if(nfds < pfd[STDIN_FILENO][1].get()) nfds = pfd[STDIN_FILENO][1].get();
                }

                if(nfds > 0)//if have fd to wait for
                {
                    int select_result = select(nfds+1, &rfds, &wfds, NULL, &tv);
                    if (select_result == -1 && errno != EINTR)
                    {
                        //error
                        std::string err_msg(strerror(errno));
                        std::string msg("ShellCmd::operator() error in select: ");
                        throw std::runtime_error(msg+err_msg);
                    }

                    //check writable end of child stdin
                    if(!(pfd[STDIN_FILENO][1].is_closed()) && FD_ISSET(pfd[STDIN_FILENO][1].get(), &wfds))
                    {
                        //child input
                        if(stdin_str_size_to_be_written)
                        {
                            int wbytes = write(pfd[STDIN_FILENO][1].get()
                                , stdin_str.c_str() + (stdin_str.size() - stdin_str_size_to_be_written)
                                , stdin_str_size_to_be_written );

                            stdin_str_size_to_be_written -= wbytes;

                            //std::cout << "wbytes: " << wbytes << std::endl;

                            if(stdin_str_size_to_be_written == 0) stdin_str.clear();

                            if(wbytes == -1)
                            {
                                std::string err_msg(strerror(errno));
                                std::string msg("ShellCmd::operator() stdin pipe write error: ");
                                throw std::runtime_error(msg+err_msg);
                            }
                        }
                    }//if stdin fd ready

                    if(stdin_str.empty()) //no input data, close stdin
                    if(pfd[STDIN_FILENO][1].closefd() == -1)
                    {
                        std::string err_msg(strerror(errno));
                        std::string msg("ShellCmd::operator() error in parent closing pipe 0 1: ");
                        throw std::runtime_error(msg+err_msg);
                    }//check pipe fds close

                    //check readable end of child stdout
                    if(!pfd[STDOUT_FILENO][0].is_closed() && FD_ISSET(pfd[STDOUT_FILENO][0].get(), &rfds))
                    {
                        char buf[1024]={0};//init buffer
                        int read_result = read(pfd[STDOUT_FILENO][0].get(),buf,sizeof(buf)-1);

                        if (read_result < 0)
                        {//error
                            std::string err_msg(strerror(errno));
                            std::string msg("ShellCmd::operator() read stdout error: ");
                            throw std::runtime_error(msg+err_msg);
                        }
                        else if (read_result == 0)
                        {//eof
                            //close readable end of stdout
                            if(pfd[STDOUT_FILENO][0].closefd() == -1)
                            {
                                std::string err_msg(strerror(errno));
                                std::string msg("ShellCmd::operator() error in parent closing pipe 1 1: ");
                                throw std::runtime_error(msg+err_msg);
                            }//check pipe fds close
                        }
                        else if (read_result > 0)
                        {//have data in buf
                            ret.stdout+=buf;
                        }
                    }//if stdout fd ready

                    //check readable end of child stderr
                    if(!pfd[STDERR_FILENO][0].is_closed() && FD_ISSET(pfd[STDERR_FILENO][0].get(), &rfds))
                    {
                        char buf[1024]={0};//init buffer
                        int read_result = read(pfd[STDERR_FILENO][0].get(),buf,sizeof(buf)-1);

                        if (read_result < 0)
                        {//error
                            std::string err_msg(strerror(errno));
                            std::string msg("ShellCmd::operator() read stderr error: ");
                            throw std::runtime_error(msg+err_msg);
                        }
                        else if (read_result == 0)
                        {//eof
                            //close readable end of stdout
                            if(pfd[STDERR_FILENO][0].closefd() == -1)
                            {
                                std::string err_msg(strerror(errno));
                                std::string msg("ShellCmd::operator() error in parent closing pipe 1 1: ");
                                throw std::runtime_error(msg+err_msg);
                            }//check pipe fds close
                        }
                        else if (read_result > 0)
                        {//have data in buf
                            ret.stderr+=buf;
                        }
                    }//if stderr fd ready

                }//if nfds

            } while(nfds > 0);//communication with child process

            //close writable end of stdin for reader
            if(pfd[STDIN_FILENO][1].closefd() == -1)
            {
                std::string err_msg(strerror(errno));
                std::string msg("ShellCmd::operator() error in parent closing pipe 0 1: ");
                throw std::runtime_error(msg+err_msg);
            }//check pipe fds close

            //child output
            while(!pfd[STDOUT_FILENO][0].is_closed())
            {
                char buf[1024]={0};//init buffer
                int read_result = read(pfd[STDOUT_FILENO][0].get(),buf,sizeof(buf)-1);
                if (read_result < 0)
                {//error
                    std::string err_msg(strerror(errno));
                    std::string msg("ShellCmd::operator() read stdout error: ");
                    throw std::runtime_error(msg+err_msg);
                }
                else if (read_result == 0)
                {//eof
                    break;
                }
                else if (read_result > 0)
                {//have data in buf
                    ret.stdout+=buf;
                }

            }//while(true) stdout

            //child error output
            while(!pfd[STDERR_FILENO][0].is_closed())
            {
                char buf[1024]={0};//init buffer
                int read_result = read(pfd[STDERR_FILENO][0].get(),buf,sizeof(buf)-1);
                if (read_result < 0)
                {//error
                    std::string err_msg(strerror(errno));
                    std::string msg("ShellCmd::operator() read stderr error: ");
                    throw std::runtime_error(msg+err_msg);
                }
                else if (read_result == 0)
                {//eof
                    break;
                }
                else if (read_result > 0)
                {//have data in buf
                    ret.stderr+=buf;
                }
            }//while(true) stdout

            //close readable end of stdout
            if(pfd[STDOUT_FILENO][0].closefd() == -1)
            {
                std::string err_msg(strerror(errno));
                std::string msg("ShellCmd::operator() error in parent closing pipe 1 1: ");
                throw std::runtime_error(msg+err_msg);
            }//check pipe fds close

            //close readable end of stderr
            if(pfd[STDERR_FILENO][0].closefd() == -1)
            {
                std::string err_msg(strerror(errno));
                std::string msg("ShellCmd::operator() error in parent closing pipe 2 1: ");
                throw std::runtime_error(msg+err_msg);
            }//check pipe fds close

            //check for child completion
            do
            {
                pid_t dp = waitpid(pid, NULL, WNOHANG);//death pid
                if(dp == pid) break; //child waited - ok

                if(dp == -1) //error
                {
                    std::string err_msg(strerror(errno));
                    std::string msg("ShellCmd::operator() error waitpid: ");
                    throw std::runtime_error(msg+err_msg);
                }//if waitpid error

                if(dp == 0) check_timeout_kill_child(pid, timeout_time);

            } while (true);//timeout waiting

            signal(SIGCHLD, sig_chld_h);//restore saved SIGCHLD handler

        }//if in parent

        return ret;//return outputs
    }


};//class ShellCmd

#endif //SUBPROCESS_H_
