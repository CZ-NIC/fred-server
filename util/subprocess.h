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
#include <vector>

#include <boost/noncopyable.hpp>

/**
 * @class SubProcessOutput
 * @brief command outputs and process exit status
 * 
 * Result of process's run.
 */
struct SubProcessOutput
{
    std::string stdout; /**< contains standard output generated by process */
    std::string stderr; /**< contains error output generated by process */
    int status; /**< exit status of process */

    /**
     * Terminated normally
     * @return true if process terminated normally, that is, by calling exit
     *         or _exit, or by returning from main()
     */
    bool is_exited()const { return WIFEXITED(status); }

    /**
     * Exit status of the normally terminated process
     * @return The exit status of the child. This consists of the least significant
     *         8 bits of the status argument that the child specified in a call to
     *         exit or _exit or as the argument for a return statement in main().
     * @note @ref is_exited() must be true
     */
    int get_exit_status()const { return WEXITSTATUS(status); }

    /**
     * Terminated by a signal
     * @return true if the child process was terminated by a signal
     */
    bool is_signaled()const { return WIFSIGNALED(status); }

    /**
     * Number of the signal
     * @return the number of the signal that caused the child process to terminate
     * @note @ref is_signaled() must be true
     */
    int get_term_sig()const { return WTERMSIG(status); }

#ifdef WCOREDUMP
    /**
     * Produced a core dump
     * @return true if the child process produced a core dump
     */
    bool is_core_dump()const { return this->is_signaled() && (WCOREDUMP(status)); }
#endif
};

/**
 * @class CmdResult
 * @brief wrapper of exec() functions family
 */
class CmdResult:public boost::noncopyable
{
public:
    /**
     * @class Args
     * @brief Command arguments.
     */
    class Args
    {
    public:
        Args() { }
        Args(const Args &_src):items_(_src.items_) { }
        Args(const std::string &_item) { items_.push_back(_item); }
        Args& operator()(const std::string &_item) { items_.push_back(_item); return *this; }
    private:
        typedef std::vector< std::string > Items;
        Items items_;
        friend class CmdResult;
    };

    /**
     * Execute command @arg _cmd using execv/execvp function in forked child process.
     * @param _cmd executable file
     * @param _args command arguments
     * @param _respect_path search executable @arg _cmd considering PATH environment variable
     * @throw std::runtime_error if something wrong happens
     * @note Sets SIGCHLD handler.
     * 
     * Run <em>_cmd [_args...]</em>
     */
    CmdResult(const std::string &_cmd, const Args &_args, bool _respect_path = false);

    /**
     * @note Kills child process if one is already alive.
     */
    ~CmdResult();

    /**
     * @class Seconds
     * @brief Represents time meassured in seconds.
     */
    typedef typeof(::timeval().tv_sec) Seconds;
    enum { INFINITE_TIME = 0 };

    /**
     * Wait as long as command runs.
     * @param _stdin_content data delivered to command via standard input
     * @param _rel_timeout maximal command lifetime in seconds, 0 means infinity.
     * @return Command's standard and error outputs and its exit status.
     * @throw std::runtime_error if something wrong happens
     * @note Restores SIGCHLD handler.
     */
    const SubProcessOutput& wait_until_done(
        const std::string &_stdin_content = std::string(),
        Seconds _rel_timeout = INFINITE_TIME);
private:
    class Pipe;
    class DataChannel
    {
    public:
        DataChannel();
        ~DataChannel();
    private:
        friend class Pipe;
        int fd_[2];
    };

    DataChannel std_in_;
    DataChannel std_out_;
    DataChannel std_err_;

    class ImParent;
    ImParent *parent_;
};//class CmdResult

/**
 * @class ShellCmd
 * @brief shell command wrapper
 * @warning With externally gained data use @ref CmdResult instead!
 *          There is a danger of security incident (shell injection).
 */
class ShellCmd:public boost::noncopyable
{
public:
    /**
     * @class RelativeTimeInSeconds
     * @brief Represents relative (from now) time meassured in seconds.
     */
    typedef CmdResult::Seconds RelativeTimeInSeconds;

    /**
     * Constructor with mandatory parameters.
     * @param _cmd sets command into @ref cmd_ attribute
     */
    ShellCmd(const std::string &_cmd);

    /**
     * Constructor with mandatory parameters.
     * @param _cmd sets command into @ref cmd_ attribute
     * @param _timeout sets maximal command lifetime into @ref timeout_ attribute
     */
    ShellCmd(const std::string &_cmd,
             RelativeTimeInSeconds _timeout
            );

    /**
     * Constructor with all parameters.
     * @param _cmd sets command into @ref cmd_ attribute
     * @param _shell sets shell into @ref shell_ attribute
     * @param _timeout sets maximal command lifetime into @ref timeout_ attribute
     */
    ShellCmd(const std::string &_cmd,
             const std::string &_shell,
             RelativeTimeInSeconds _timeout
            );

    /**
     * @note Kills child process if one is already alive
     */
    ~ShellCmd();

    /**
     * Execute command in shell.
     * @param stdin_str data delivered to @ref cmd_ via standard input
     * @return command outputs and exit status
     * @note Sets and restores SIGCHLD handler
     * 
     * Run <em>echo $stdin_str|$shell_ -c "$cmd_"</em>
     */
    SubProcessOutput execute(const std::string &stdin_str = std::string());

private:
    const std::string cmd_; /**< Command executed by @ref shell_. */
    const std::string shell_; /**< Shell executes command @ref cmd_. */
    const RelativeTimeInSeconds timeout_; /**< Maximal command lifetime in seconds, 0 means infinity. */
};//class ShellCmd

#endif//SUBPROCESS_H_
