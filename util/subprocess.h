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


#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <csignal>
#include <unistd.h>
#include <cstring>
#include <string>
#include <iostream>
#include <stdexcept>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cassert>
#include <iterator>

#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>

#include "config.h"
#ifndef HAVE_LOGGER
#error HAVE_LOGGER is required!
#endif
#include "log/logger.h"


///FILE* close shared pointer
typedef boost::shared_ptr<FILE> FileSharedPtr;
template < typename DELETER >
class FilePtrT
{
protected:
    FileSharedPtr m_ptr;
public:
    FilePtrT(FILE* f) : m_ptr(f,DELETER()) {}
    FilePtrT() : m_ptr(0,DELETER()) {}

    operator FileSharedPtr() const
    {
        return m_ptr;
    }
};
///deleter functor for file calling fclose
struct FileClose
{
    void operator()(FILE* f)
    {
        try
        {
            if(f)
            {
                fclose(f);
            }
        }
        catch(...){}
    }
};
///FileSharedPtr factory
typedef FilePtrT<FileClose> FileClosePtr;
///usage FileSharedPtr  file_close_guard = FileClosePtr((FILE*)0);


//SIGALARM handler user in ShellCmd for waitpid timeout
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
    const std::string cmd_;
    const std::string shell_;
    unsigned long timeout_;

    int p[num_of_pipes][num_of_pipe_ends];//pipes fds

    void close_pipes()
    {
        for(int i = 0; i < num_of_pipes; ++i)
            for(int j = 0; j < num_of_pipe_ends; ++j)
            {
                if(p[i][j] != -1)
                {
                    if(close(p[i][j]) != 0)
                    {
		    /* this may fail
                        std::string err_msg(strerror(errno));
                        Logging::Manager::instance_ref()
                            .get(PACKAGE).error(
                                    std::string("ShellCmd::close_pipes() error closing pipe: ")
                                    + boost::lexical_cast<std::string>(i)
                                    + (" msg: ")+err_msg );
		     */
                    }//check pipe fds close

                    p[i][j] = -1;//set pipe fds invalid
                }//if fds is valid
            }//for i j
    }//close_pipes

    void create_pipes()
    {
        // create the pipes
        for(int i = 0; i < num_of_pipes; ++i)
        {//if the pipe is created succesfully, then readable end is p[0], and p[1] is the writable end
            if(pipe(p[i]) != 0)
            {
                std::string pipe_error (strerror(errno));
                Logging::Manager::instance_ref()
                    .get(PACKAGE).error(
                            std::string("ShellCmd::create_pipes() error creating pipe: ")
                            + boost::lexical_cast<std::string>(i)
                            + (" msg: ")+pipe_error );
                throw std::runtime_error(std::string("create pipe error: ")+pipe_error);
            }
        }//for i is num_of_pipes
    }//create_pipes

public:
    ShellCmd(const std::string& cmd)
    :cmd_(cmd)
    , shell_("/bin/sh")
    , timeout_(10)
    {
        //init pipe fds
        for(int i = 0; i < num_of_pipes; ++i)
            for(int j = 0; j < num_of_pipe_ends; ++j)
                p[i][j] = -1;
    }

    ShellCmd(const std::string& cmd
            , const unsigned long timeout
            )
    :cmd_(cmd)
    , shell_("/bin/sh")
    , timeout_(timeout)
    {
        //init pipe fds
        for(int i = 0; i < num_of_pipes; ++i)
            for(int j = 0; j < num_of_pipe_ends; ++j)
                p[i][j] = -1;
    }


    ShellCmd(const std::string& cmd
            , const std::string& shell //shell is filename, not command line
            , const unsigned long timeout //in seconds used for alarm
            )
    :cmd_(cmd)
    , shell_(shell)
    , timeout_(timeout)
    {
        //init pipe fds
        for(int i = 0; i < num_of_pipes; ++i)
            for(int j = 0; j < num_of_pipe_ends; ++j)
                p[i][j] = -1;
    }


    ~ShellCmd()
    {
        try
        {
            close_pipes();
        }//try
        catch(const std::exception& ex)
        {
            Logging::Manager::instance_ref()
                .get(PACKAGE).error(std::string("~ShellCmd exception: ") + ex.what());
        }
        catch(...)
        {
            Logging::Manager::instance_ref()
                .get(PACKAGE).error("~ShellCmd unknown exception");
        }
    }

    SubProcessOutput execute(std::string stdin_str = std::string())
    {
        close_pipes();
        create_pipes();

        pid_t pid;//child pid
        pid = fork();
        if(-1 == pid)
        {
          std::string err_msg(strerror(errno));
          Logging::Manager::instance_ref()
              .get(PACKAGE).error(
                      std::string("ShellCmd::operator() fork error: ")
                      + err_msg );
          throw std::runtime_error(std::string("ShellCmd::operator() fork error: ")+err_msg);
        }
        //parent and child now share the pipe's file descriptors
        //readable end is p[0], and p[1] is the writable end
        if(pid)
        {
            //in parent
            //close readable end of stdin
            if(close(p[STDIN_FILENO][0]) != 0)
            {
                std::string err_msg(strerror(errno));
                std::string msg("ShellCmd::operator() error in parent closing pipe 0 0: ");
                Logging::Manager::instance_ref()
                    .get(PACKAGE).error(msg+err_msg);
                throw std::runtime_error(msg+err_msg);
            }//check pipe fds close
            p[STDIN_FILENO][0] = -1;

            //close writable end of stdout
            if(close(p[STDOUT_FILENO][1]) != 0 )
            {
                std::string err_msg(strerror(errno));
                std::string msg("ShellCmd::operator() error in parent closing pipe 1 1: ");
                Logging::Manager::instance_ref()
                    .get(PACKAGE).error(msg+err_msg);
                throw std::runtime_error(msg+err_msg);
            }//check pipe fds close
            p[STDOUT_FILENO][1] = -1;

            //close writable end of stderr
            if(close(p[STDERR_FILENO][1]) != 0)
            {
                std::string err_msg(strerror(errno));
                std::string msg("ShellCmd::operator() error in parent closing pipe 2 1: ");
                Logging::Manager::instance_ref()
                    .get(PACKAGE).error(msg+err_msg);
                throw std::runtime_error(msg+err_msg);
            }//check pipe fds close
            p[STDERR_FILENO][1] = -1;

            //open file streams
            FILE* child_stdin = fdopen(p[STDIN_FILENO][1], "w") ;
            if(child_stdin == NULL)
            {
                std::string err_msg(strerror(errno));
                std::string msg("ShellCmd::operator() error fdopen child_stdin: ");
                Logging::Manager::instance_ref()
                    .get(PACKAGE).error(msg+err_msg);
                throw std::runtime_error(msg+err_msg);
            }
            FileSharedPtr  child_stdin_close_guard = FileClosePtr(child_stdin);

            FILE* child_stdout = fdopen(p[STDOUT_FILENO][0], "r" );
            if(child_stdout == NULL)
            {
                std::string err_msg(strerror(errno));
                std::string msg("ShellCmd::operator() error fdopen child_stdout: ");
                Logging::Manager::instance_ref()
                    .get(PACKAGE).error(msg+err_msg);
                throw std::runtime_error(msg+err_msg);
            }
            FileSharedPtr  child_stdout_close_guard = FileClosePtr(child_stdout);

            FILE* child_stderr = fdopen(p[STDERR_FILENO][0], "r");
            if(child_stderr == NULL)
            {
                std::string err_msg(strerror(errno));
                std::string msg("ShellCmd::operator() error fdopen child_stderr: ");
                Logging::Manager::instance_ref()
                    .get(PACKAGE).error(msg+err_msg);
                throw std::runtime_error(msg+err_msg);
            }
            FileSharedPtr  child_stderr_close_guard = FileClosePtr(child_stderr);

            //child input
            fprintf(child_stdin_close_guard.get(), "%s\n", cmd_.c_str());
            if(!stdin_str.empty()) fprintf(child_stdin_close_guard.get(), "%s\n", stdin_str.c_str());

            child_stdin_close_guard.reset((FILE*)(0));//flush and close input
            //failed close is better than leaked file descriptor
            //p[STDIN_FILENO][1]= -1;//fclose inFileSharedPtr deleter is closing also file descriptor

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

            SubProcessOutput ret;
            //child output
            while(true)
            {
                char buf[100]={0};//init buffer
                if(fgets(buf, sizeof(buf)-1,child_stdout_close_guard.get()) == NULL)//try read
                {
                    if ( feof(child_stdout_close_guard.get()) ) break;    // check for EOF
                    //error
                    std::string err_msg(strerror(errno));
                    std::string msg("ShellCmd::operator() error read by fgets: ");
                    Logging::Manager::instance_ref()
                        .get(PACKAGE).error(msg+err_msg);
                    throw std::runtime_error(msg+err_msg);
                }//if read data
                ret.stdout+=buf;
            }//while(true)
            child_stdout_close_guard.reset((FILE*)(0));//flush and close output
            //failed close is better than leaked file descriptor
            //p[STDOUT_FILENO][0]= -1;//fclose inFileSharedPtr deleter is closing also file descriptor

            //child erroutput
            while(true)
            {
                char buf[100]={0};//init buffer
                if(fgets(buf, sizeof(buf)-1,child_stderr_close_guard.get()) == NULL)//try read
                {
                    if ( feof(child_stderr_close_guard.get()) ) break;    // check for EOF
                    //error
                    std::string err_msg(strerror(errno));
                    std::string msg("ShellCmd::operator() error read by fgets: ");
                    Logging::Manager::instance_ref()
                        .get(PACKAGE).error(msg+err_msg);
                    throw std::runtime_error(msg+err_msg);
                }//if read data
                ret.stderr+=buf;
            }//while(true)
            child_stderr_close_guard.reset((FILE*)(0));//flush and close output
            //failed close is better than leaked file descriptor
            //p[STDERR_FILENO][0]= -1;//fclose inFileSharedPtr deleter is closing also file descriptor

            signal(SIGCHLD, sig_chld_h);//restore saved SIGCHLD handler

            return ret;//return outputs
        }
            else
        {
            //in child redirect stdin, stdout, stderr via pipes
            //duplicate readable end of pipe 0 into stdin
            if(dup2(p[STDIN_FILENO][0],STDIN_FILENO) == -1)
            {
                std::string err_msg(strerror(errno));
                std::string msg("ShellCmd::operator() error in child duplicating stdin pipe 0 0: ");
                Logging::Manager::instance_ref()
                    .get(PACKAGE).error(msg+err_msg);
                throw std::runtime_error(msg+err_msg);
            }
            //close writable end of stdin
            if(close(p[STDIN_FILENO][1]) != 0)
            {
                std::string err_msg(strerror(errno));
                std::string msg("ShellCmd::operator() error in child closing stdin pipe 0 1: ");
                Logging::Manager::instance_ref()
                    .get(PACKAGE).error(msg+err_msg);
                throw std::runtime_error(msg+err_msg);
            }
            //duplicate writable end of pipe 1 into stdout
            if(dup2(p[STDOUT_FILENO][1],STDOUT_FILENO) == -1)
            {
                std::string err_msg(strerror(errno));
                std::string msg("ShellCmd::operator() error in child duplicating stdout pipe 1 1: ");
                Logging::Manager::instance_ref()
                    .get(PACKAGE).error(msg+err_msg);
                throw std::runtime_error(msg+err_msg);
            }
            //close readable end of stdout
            if(close(p[STDOUT_FILENO][0]) != 0)
            {
                std::string err_msg(strerror(errno));
                std::string msg("ShellCmd::operator() error in child closing stdout pipe 1 0: ");
                Logging::Manager::instance_ref()
                    .get(PACKAGE).error(msg+err_msg);
                throw std::runtime_error(msg+err_msg);
            }
            //duplicate writable end of pipe 2 into stderr
            if(dup2(p[STDERR_FILENO][1],STDERR_FILENO) == -1)
            {
                std::string err_msg(strerror(errno));
                std::string msg("ShellCmd::operator() error in child duplicating stderr pipe 2 1: ");
                Logging::Manager::instance_ref()
                    .get(PACKAGE).error(msg+err_msg);
                throw std::runtime_error(msg+err_msg);
            }
            //close readable end of stderr
            if(close(p[STDERR_FILENO][0]) != 0)
            {
                std::string err_msg(strerror(errno));
                std::string msg("ShellCmd::operator() error in child closing stderr pipe 2 0: ");
                Logging::Manager::instance_ref()
                    .get(PACKAGE).error(msg+err_msg);
                throw std::runtime_error(msg+err_msg);
            }

            char *shell_argv[2];
            shell_argv[0] = (char*)shell_.c_str();
            shell_argv[1] = NULL;


            //std::cout << "\n\nshell: " << shell_ << " cmd: " << cmd_ << std::endl;

            //shell exec
            execvp(shell_argv[0],shell_argv);

            //failed to launch the shell
            std::string err_msg(strerror(errno));
            std::string msg("ShellCmd::operator() failed to launch shell: ");
            Logging::Manager::instance_ref()
                .get(PACKAGE).error(msg+ err_msg);
            throw std::runtime_error(msg+ err_msg);

        }//child else
        //close_pipes();
        //return SubProcessOutput();
    }
};//class ShellCmd

#endif //SUBPROCESS_H_
