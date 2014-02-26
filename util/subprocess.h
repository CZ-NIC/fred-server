/*
 * Copyright (C) 2014  CZ.NIC, z.s.p.o.
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

#include <string>

#include <boost/noncopyable.hpp>

//shell output
struct SubProcessOutput
{
    std::string stdout;
    std::string stderr;
    int status;//man 2 waitpid

    bool is_exited()const { return WIFEXITED(status); }
    int get_exit_status()const { return WEXITSTATUS(status); }
    bool is_signaled()const { return WIFSIGNALED(status); }
    int get_term_sig()const { return WTERMSIG(status); }
#ifdef WCOREDUMP
    bool is_core_dump()const { return this->is_signaled() && (WCOREDUMP(status)); }
#endif
};

/**
 * \class ShellCmd
 * \brief shell command wrapper
 */

class ShellCmd:public boost::noncopyable
{
public:
    typedef typeof(::timeval().tv_sec) RelativeTimeInSeconds;

    ShellCmd(const std::string &_cmd);

    ShellCmd(const std::string &_cmd,
             RelativeTimeInSeconds _timeout
            );

    ShellCmd(const std::string &_cmd,
             const std::string &_shell, // _shell -c _cmd; _shell = "/bin/bash"
             RelativeTimeInSeconds _timeout
            );

    ~ShellCmd();

    SubProcessOutput execute(std::string stdin_str = std::string());

private:
    const std::string cmd_;
    const std::string shell_;
    const RelativeTimeInSeconds timeout_;
    ::pid_t child_pid_;

    void kill_child(int *_status = NULL) throw();
};//class ShellCmd

#endif//SUBPROCESS_H_
