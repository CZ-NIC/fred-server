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

#include  <sys/types.h>
#include  <sys/wait.h>
#include <string.h>

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
};

///SIGALARM handler user in ShellCmd for waitpid timeout
static void handleSIGALARM (int sig)
{
     signal(SIGALRM, SIG_IGN);//uninstall handler

     std::string msg("timeout");
     Logging::Manager::instance_ref()
         .get(PACKAGE).error(msg);
     throw std::runtime_error(msg);
}


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
            , const unsigned long timeout //in seconds used for alarm
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
                    Logging::Manager::instance_ref()
                        .get(PACKAGE).error(
                                std::string("ShellCmd::create_pipes() error creating pipe: ")
                                + boost::lexical_cast<std::string>(i)
                                + (" msg: ")+pipe_error );
                    throw std::runtime_error(std::string("create pipe error: ")+pipe_error);
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
          Logging::Manager::instance_ref()
              .get(PACKAGE).error(
                      std::string("ShellCmd::operator() fork error: ")
                      + err_msg );
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
                Logging::Manager::instance_ref()
                    .get(PACKAGE).error(msg+err_msg);
                throw std::runtime_error(msg+err_msg);
            }
            //close readable end of stdout
            if(pfd[STDOUT_FILENO][0].closefd() == -1)
            {
                std::string err_msg(strerror(errno));
                std::string msg("ShellCmd::operator() error in child closing stdout pipe 1 0: ");
                Logging::Manager::instance_ref()
                    .get(PACKAGE).error(msg+err_msg);
                throw std::runtime_error(msg+err_msg);
            }
            //close readable end of stderr
            if(pfd[STDERR_FILENO][0].closefd() == -1)
            {
                std::string err_msg(strerror(errno));
                std::string msg("ShellCmd::operator() error in child closing stderr pipe 2 0: ");
                Logging::Manager::instance_ref()
                    .get(PACKAGE).error(msg+err_msg);
                throw std::runtime_error(msg+err_msg);
            }

            //duplicate readable end of pipe 0 into stdin
            if(pfd[STDIN_FILENO][0].get() != STDIN_FILENO)
            {
                if(dup2(pfd[STDIN_FILENO][0].get(),STDIN_FILENO) != STDIN_FILENO)
                {
                    std::string err_msg(strerror(errno));
                    std::string msg("ShellCmd::operator() error in child duplicating stdin pipe 0 0: ");
                    Logging::Manager::instance_ref()
                        .get(PACKAGE).error(msg+err_msg);
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
                    Logging::Manager::instance_ref()
                        .get(PACKAGE).error(msg+err_msg);
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
                    Logging::Manager::instance_ref()
                        .get(PACKAGE).error(msg+err_msg);
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
                Logging::Manager::instance_ref()
                    .get(PACKAGE).error(msg+ err_msg);
                throw std::runtime_error(msg+ err_msg);
            }

        }//if in child
        else if(pid > 0)
        {
            //in parent
            //close readable end of stdin
            if(pfd[STDIN_FILENO][0].closefd() == -1)
            {
                std::string err_msg(strerror(errno));
                std::string msg("ShellCmd::operator() error in parent closing pipe 0 0: ");
                Logging::Manager::instance_ref()
                    .get(PACKAGE).error(msg+err_msg);
                throw std::runtime_error(msg+err_msg);
            }//check pipe fds close

            //close writable end of stdout
            if(pfd[STDOUT_FILENO][1].closefd() == -1 )
            {
                std::string err_msg(strerror(errno));
                std::string msg("ShellCmd::operator() error in parent closing pipe 1 1: ");
                Logging::Manager::instance_ref()
                    .get(PACKAGE).error(msg+err_msg);
                throw std::runtime_error(msg+err_msg);
            }//check pipe fds close

            //close writable end of stderr
            if(pfd[STDERR_FILENO][1].closefd() == -1)
            {
                std::string err_msg(strerror(errno));
                std::string msg("ShellCmd::operator() error in parent closing pipe 2 1: ");
                Logging::Manager::instance_ref()
                    .get(PACKAGE).error(msg+err_msg);
                throw std::runtime_error(msg+err_msg);
            }//check pipe fds close

            //child input
            if(!stdin_str.empty())
            {
                if(write(pfd[STDIN_FILENO][1].get(),stdin_str.c_str(),stdin_str.size()) != static_cast<int>(stdin_str.size()))
                {
                    std::string err_msg(strerror(errno));
                    std::string msg("ShellCmd::operator() stdin pipe write error: ");
                    Logging::Manager::instance_ref()
                        .get(PACKAGE).error(msg+err_msg);
                    throw std::runtime_error(msg+err_msg);
                }
            }

            //close writable end of stdin for reader
            if(pfd[STDIN_FILENO][1].closefd() == -1)
            {
                std::string err_msg(strerror(errno));
                std::string msg("ShellCmd::operator() error in parent closing pipe 0 1: ");
                Logging::Manager::instance_ref()
                    .get(PACKAGE).error(msg+err_msg);
                throw std::runtime_error(msg+err_msg);
            }//check pipe fds close

            //waitpid need default SIGCHLD handler to work
            sighandler_t sig_chld_h = signal(SIGCHLD, SIG_DFL);
            signal(SIGALRM, handleSIGALARM);     //install the handler

            // wait for child completion
            //set child timeout
            if (timeout_ != 0) alarm(timeout_);
            int    status;//shell exit status
            do
            {
                pid_t dp;//death pid
                dp = waitpid(pid, &status,0);
                if (timeout_ != 0) alarm(0);//cancel timeout

                if(dp == -1)
                {
                    std::string err_msg(strerror(errno));
                    std::string msg("ShellCmd::operator() error waitpid: ");
                    Logging::Manager::instance_ref()
                        .get(PACKAGE).error(msg+err_msg);
                    throw std::runtime_error(msg+err_msg);
                }//if waitpid error

                if (WIFSIGNALED(status))
                {
                    std::string err_msg(strerror(errno));
                    std::string msg("ShellCmd::operator() waitpid - child killed by signal: ");
                    Logging::Manager::instance_ref()
                        .get(PACKAGE).error(msg+boost::lexical_cast<std::string>(WTERMSIG(status))
                            + std::string(" ") + err_msg);
                    throw std::runtime_error(msg+boost::lexical_cast<std::string>(WTERMSIG(status))
                            + std::string(" ") + err_msg);
                }
            }
            while (!WIFEXITED(status) && !WIFSIGNALED(status));

            //child output
            while(true)
            {
                char buf[1024]={0};//init buffer
                int read_result = read(pfd[STDOUT_FILENO][0].get(),buf,sizeof(buf)-1);
                if (read_result < 0)
                {//error
                    std::string err_msg(strerror(errno));
                    std::string msg("ShellCmd::operator() read error: ");
                    Logging::Manager::instance_ref()
                        .get(PACKAGE).error(msg+err_msg);
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
            while(true)
            {
                char buf[1024]={0};//init buffer
                int read_result = read(pfd[STDERR_FILENO][0].get(),buf,sizeof(buf)-1);
                if (read_result < 0)
                {//error
                    std::string err_msg(strerror(errno));
                    std::string msg("ShellCmd::operator() read error: ");
                    Logging::Manager::instance_ref()
                        .get(PACKAGE).error(msg+err_msg);
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
                Logging::Manager::instance_ref()
                    .get(PACKAGE).error(msg+err_msg);
                throw std::runtime_error(msg+err_msg);
            }//check pipe fds close

            //close readable end of stderr
            if(pfd[STDERR_FILENO][0].closefd() == -1)
            {
                std::string err_msg(strerror(errno));
                std::string msg("ShellCmd::operator() error in parent closing pipe 2 1: ");
                Logging::Manager::instance_ref()
                    .get(PACKAGE).error(msg+err_msg);
                throw std::runtime_error(msg+err_msg);
            }//check pipe fds close

            signal(SIGCHLD, sig_chld_h);//restore saved SIGCHLD handler
/*
            printf("\nbefore close\n");
            printf("\npipe: %d fd0: %d fd1: %d\n", 0, pfd[0][0].get(), pfd[0][1].get());
            printf("\npipe: %d fd0: %d fd1: %d\n", 1, pfd[1][0].get(), pfd[1][1].get());
            printf("\npipe: %d fd0: %d fd1: %d\n", 2, pfd[2][0].get(), pfd[2][1].get());
*/

        }//if in parent

        return ret;//return outputs
    }


};//class ShellCmd

#endif //SUBPROCESS_H_
