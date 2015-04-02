#include "util/subprocess.h"

#include <sys/select.h>
#include <fcntl.h>
#include <unistd.h>

#include <cstdlib>
#include <cerrno>
#include <csignal>
#include <cstring>

#include <iostream>
#include <iomanip>
#include <stdexcept>

#include "config.h"
#ifndef HAVE_LOGGER
#error HAVE_LOGGER is required!
#endif
#include "log/logger.h"

namespace
{

#define DEFAULT_BASH "/bin/sh"
enum { DEFAULT_TIMEOUT_SEC = 10 };

enum {
    FAILURE = -1,
    SUCCESS =  0,
    INVALID_DESCRIPTOR = -1,
    IM_CHILD = 0,
};

}

CmdResult::DataChannel::DataChannel()
{
    const int ret_val = ::pipe(fd_);
    if (ret_val != SUCCESS) {
        const int c_errno = errno;
        std::ostringstream msg;
        msg << __PRETTY_FUNCTION__ << " pipe() call failure: " << std::strerror(c_errno);
        throw std::runtime_error(msg.str());
    }
}

CmdResult::DataChannel::~DataChannel()
{
    for (int *fd_ptr = fd_; fd_ptr < (fd_ + 2); ++fd_ptr) {
        int &fd = *fd_ptr;
        if (fd != INVALID_DESCRIPTOR) {
            ::close(fd);
            fd = INVALID_DESCRIPTOR;
        }
    }
}

class CmdResult::Pipe
{
public:
    Pipe(DataChannel &_data_channel):fd_(_data_channel.fd_) { }

    //public read end of pipe interface, hide write end of pipe
    class ImReader;

    //public write end of pipe interface, hide read end of pipe
    class ImWriter;
private:
    Pipe& close_read_end();
    Pipe& dup_read_end(int _new_fd);

    void set_nonblocking_read_end()const;
    void watch_ready_for_read(::fd_set &_set, int &_max_fd)const;
    bool is_ready_for_read(const ::fd_set &_set)const;
    ::size_t read(void *_buf, ::size_t _buf_size)const;


    Pipe& close_write_end();
    Pipe& dup_write_end(int _new_fd);

    void set_nonblocking_write_end()const;
    void watch_ready_for_write(::fd_set &_set, int &_max_fd)const;
    bool is_ready_for_write(const ::fd_set &_set)const;
    ::size_t write(const void *_data, ::size_t _data_size)const;

    enum Idx {
        READ_END  = 0,
        WRITE_END = 1,
    };

    Pipe& close(Idx _idx);
    Pipe& dup(Idx _idx, int _new_fd);

    bool is_closed(Idx _idx)const;
    bool is_open(Idx _idx)const;
    void watch(Idx _idx, ::fd_set &_set, int &_max_fd)const;
    bool is_set(Idx _idx, const ::fd_set &_set)const;
    void set_nonblocking(Idx _idx)const;

    int *const fd_;

    friend class ImReader;
    friend class ImWriter;
};

CmdResult::Pipe& CmdResult::Pipe::close_read_end()
{
    if (this->is_open(READ_END)) {
        this->close(READ_END);
    }
    return *this;
}

CmdResult::Pipe& CmdResult::Pipe::dup_read_end(int _new_fd)
{
    return this->dup(READ_END, _new_fd);
}

void CmdResult::Pipe::set_nonblocking_read_end()const
{
    this->set_nonblocking(READ_END);
}

void CmdResult::Pipe::watch_ready_for_read(::fd_set &_set, int &_max_fd)const
{
    this->watch(READ_END, _set, _max_fd);
}

bool CmdResult::Pipe::is_ready_for_read(const ::fd_set &_set)const
{
    return this->is_set(READ_END, _set);
}

::size_t CmdResult::Pipe::read(void *_buf, ::size_t _buf_size)const
{
    const ::ssize_t bytes = ::read(fd_[READ_END], _buf, _buf_size);
    if (bytes != FAILURE) {
        return bytes;
    }
    const int c_errno = errno;
    std::ostringstream msg;
    msg << "CmdResult::Pipe::read() read() call failure: " << std::strerror(c_errno);
    throw std::runtime_error(msg.str());
}

CmdResult::Pipe& CmdResult::Pipe::close_write_end()
{
    if (this->is_open(WRITE_END)) {
        this->close(WRITE_END);
    }
    return *this;
}

CmdResult::Pipe& CmdResult::Pipe::dup_write_end(int _new_fd)
{
    return this->dup(WRITE_END, _new_fd);
}

void CmdResult::Pipe::set_nonblocking_write_end()const
{
    this->set_nonblocking(WRITE_END);
}

void CmdResult::Pipe::watch_ready_for_write(::fd_set &_set, int &_max_fd)const
{
    this->watch(WRITE_END, _set, _max_fd);
}

bool CmdResult::Pipe::is_ready_for_write(const ::fd_set &_set)const
{
    return this->is_set(WRITE_END, _set);
}

::size_t CmdResult::Pipe::write(const void *_data, ::size_t _data_size)const
{
    const ::ssize_t bytes = ::write(fd_[WRITE_END], _data, _data_size);
    if (bytes != FAILURE) {
        return bytes;
    }
    const int c_errno = errno;
    std::ostringstream msg;
    msg << "CmdResult::Pipe::write() write() call failure: " << std::strerror(c_errno);
    throw std::runtime_error(msg.str());
}

CmdResult::Pipe& CmdResult::Pipe::close(Idx _idx)
{
    const int ret_val = ::close(fd_[_idx]);
    if (ret_val == SUCCESS) {
        fd_[_idx] = INVALID_DESCRIPTOR;
        return *this;
    }
    const int c_errno = errno;
    std::ostringstream msg;
    msg << "CmdResult::Pipe::close(" << _idx << ") close(" << fd_[_idx] << ") call failure: " << std::strerror(c_errno);
    throw std::runtime_error(msg.str());
}

CmdResult::Pipe& CmdResult::Pipe::dup(Idx _idx, int _new_fd)
{
    const int ret_val = ::dup2(fd_[_idx], _new_fd);
    if (ret_val != FAILURE) {
        this->close(_idx);
        return *this;
    }
    const int c_errno = errno;
    std::ostringstream msg;
    msg << "CmdResult::Pipe::dup(" << _idx << ") dup2(" << fd_[_idx] << ", " << _new_fd << ") call failure: " << std::strerror(c_errno);
    throw std::runtime_error(msg.str());
}

bool CmdResult::Pipe::is_closed(Idx _idx)const
{
    return fd_[_idx] == INVALID_DESCRIPTOR;
}

bool CmdResult::Pipe::is_open(Idx _idx)const
{
    return !this->is_closed(_idx);
}

void CmdResult::Pipe::watch(Idx _idx, ::fd_set &_set, int &_max_fd)const
{
    if (this->is_open(_idx)) {
        FD_SET(fd_[_idx], &_set);
        if (_max_fd < fd_[_idx]) {
            _max_fd = fd_[_idx];
        }
    }
}

bool CmdResult::Pipe::is_set(Idx _idx, const ::fd_set &_set)const
{
    return this->is_open(_idx) && FD_ISSET(fd_[_idx], &_set);
}

void CmdResult::Pipe::set_nonblocking(Idx _idx)const
{
    long flags = ::fcntl(fd_[_idx], F_GETFL);
    if (flags == FAILURE) {
        const int c_errno = errno;
        std::ostringstream msg;
        msg << "CmdResult::Pipe::set_nonblocking(" << _idx << ") fcntl(" << fd_[_idx] << ", F_GETFL) call failure: " << std::strerror(c_errno);
        throw std::runtime_error(msg.str());
    }
    flags |= O_NONBLOCK;
    if (::fcntl(fd_[_idx], F_SETFL, flags) != SUCCESS) {
        const int c_errno = errno;
        std::ostringstream msg;
        msg << "CmdResult::Pipe::set_nonblocking(" << _idx << ") fcntl(" << fd_[_idx] << ", F_SETFL, " << flags << ") call failure: " << std::strerror(c_errno);
        throw std::runtime_error(msg.str());
    }
}

class CmdResult::Pipe::ImReader:public boost::noncopyable
{
public:
    ImReader(DataChannel &_data_channel);
    ~ImReader() { }

    ImReader& dup(int _new_fd);
    ImReader& close();
    ImReader& append(std::string &_buf);

    void set_nonblocking()const;
    void watch(::fd_set &_set, int &_max_fd)const;
    bool is_ready(const ::fd_set &_set)const;
    ::size_t read(void *_buf, ::size_t _buf_size)const;
private:
    Pipe pipe_;
};

CmdResult::Pipe::ImReader::ImReader(DataChannel &_data_channel)
:   pipe_(_data_channel)
{
    pipe_.close_write_end();
}

CmdResult::Pipe::ImReader& CmdResult::Pipe::ImReader::dup(int _new_fd)
{
    pipe_.dup_read_end(_new_fd);
    return *this;
}

CmdResult::Pipe::ImReader& CmdResult::Pipe::ImReader::close()
{
    pipe_.close_read_end();
    return *this;
}

CmdResult::Pipe::ImReader& CmdResult::Pipe::ImReader::append(std::string &_buf)
{
    enum { DATA_CHUNK_SIZE = 1024 };
    char data[DATA_CHUNK_SIZE];
    const ::size_t bytes = this->read(data, DATA_CHUNK_SIZE);
    enum { READ_END_OF_FILE = 0 };
    if (bytes == READ_END_OF_FILE) {
        this->close();
    }
    else {//have data in buf
        _buf.append(data, bytes);
    }
    return *this;
}

void CmdResult::Pipe::ImReader::set_nonblocking()const
{
    pipe_.set_nonblocking_read_end();
}

void CmdResult::Pipe::ImReader::watch(::fd_set &_set, int &_max_fd)const
{
    pipe_.watch_ready_for_read(_set, _max_fd);
}

bool CmdResult::Pipe::ImReader::is_ready(const ::fd_set &_set)const
{
    return pipe_.is_ready_for_read(_set);
}

::size_t CmdResult::Pipe::ImReader::read(void *_buf, ::size_t _buf_size)const
{
    return pipe_.read(_buf, _buf_size);
}

class CmdResult::Pipe::ImWriter:public boost::noncopyable
{
public:
    ImWriter(DataChannel &_data_channel);
    ~ImWriter() { }

    ImWriter& dup(int _new_fd);
    ImWriter& close();

    void set_nonblocking()const;
    void watch(::fd_set &_set, int &_max_fd)const;
    bool is_ready(const ::fd_set &_set)const;
    ::size_t write(const void *_data, ::size_t _data_size)const;
private:
    Pipe pipe_;
};

CmdResult::Pipe::ImWriter::ImWriter(DataChannel &_data_channel)
:   pipe_(_data_channel)
{
    pipe_.close_read_end();
}

CmdResult::Pipe::ImWriter& CmdResult::Pipe::ImWriter::dup(int _new_fd)
{
    pipe_.dup_write_end(_new_fd);
    return *this;
}

CmdResult::Pipe::ImWriter& CmdResult::Pipe::ImWriter::close()
{
    pipe_.close_write_end();
    return *this;
}

void CmdResult::Pipe::ImWriter::set_nonblocking()const
{
    pipe_.set_nonblocking_write_end();
}

void CmdResult::Pipe::ImWriter::watch(::fd_set &_set, int &_max_fd)const
{
    pipe_.watch_ready_for_write(_set, _max_fd);
}

bool CmdResult::Pipe::ImWriter::is_ready(const ::fd_set &_set)const
{
    return pipe_.is_ready_for_write(_set);
}

::size_t CmdResult::Pipe::ImWriter::write(const void *_data, ::size_t _data_size)const
{
    return pipe_.write(_data, _data_size);
}

class CmdResult::ImParent
{
public:
    ImParent(CmdResult &cmd, ::pid_t _child_pid);
    ~ImParent();
    const SubProcessOutput& wait_until_done(const std::string &_stdin_content, Seconds _rel_timeout);
private:
    void kill_child();

    ::pid_t child_pid_;
    Pipe::ImWriter child_std_in_;
    Pipe::ImReader child_std_out_;
    Pipe::ImReader child_std_err_;

    SubProcessOutput out_;

    static void handler(int) { }
    struct ::sigaction old_act_;
    ::sigset_t old_set_;
};

CmdResult::ImParent::ImParent(CmdResult &cmd, ::pid_t _child_pid)
:   child_pid_(_child_pid),
    child_std_in_(cmd.std_in_),
    child_std_out_(cmd.std_out_),
    child_std_err_(cmd.std_err_)
{
    struct ::sigaction new_act;
    new_act.sa_handler = handler;
    ::sigemptyset(&new_act.sa_mask);
    new_act.sa_flags = 0;
    ::sigaction(SIGCHLD, &new_act, &old_act_);

    ::sigset_t new_set;
    ::sigemptyset(&new_set);
    ::sigaddset(&new_set, SIGCHLD);
    ::sigprocmask(SIG_UNBLOCK, &new_set, &old_set_);

    child_std_in_ .set_nonblocking();
    child_std_out_.set_nonblocking();
    child_std_err_.set_nonblocking();
}

CmdResult::ImParent::~ImParent()
{
    ::sigprocmask(SIG_SETMASK, &old_set_, NULL);
    ::sigaction(SIGCHLD, &old_act_, NULL);
}

const SubProcessOutput& CmdResult::ImParent::wait_until_done(
    const std::string &_stdin_content,
    Seconds _rel_timeout)
{
    if (child_pid_ != IM_CHILD) {
        //set select timeout
        struct ::timeval timeout;
        struct ::timeval *timeout_ptr = NULL;
        if (_rel_timeout != INFINITE_TIME) {
            timeout.tv_sec = _rel_timeout;
            timeout.tv_usec = 0;
            timeout_ptr = &timeout;
        }

        //communication with child process
        const char *data = _stdin_content.c_str();
        const char *const data_end = data + _stdin_content.length();
        if (_stdin_content.empty()) { //no input data => close stdin
            child_std_in_.close();
        }

        while (true) {
            //init fd sets for reading and writing
            ::fd_set rfds, wfds;
            FD_ZERO(&rfds);
            FD_ZERO(&wfds);
            int nfds = 0;

            //watch events on stdin, stdout, stderr
            child_std_in_ .watch(wfds, nfds);
            child_std_out_.watch(rfds, nfds);
            child_std_err_.watch(rfds, nfds);

            if (nfds <= 0) {//nothing to do
                break;
            }

            const int events = ::select(nfds + 1, &rfds, &wfds, NULL, timeout_ptr);

            if (events == 0) {//no event => timeout
                child_std_in_ .close();
                child_std_out_.close();
                child_std_err_.close();
                this->kill_child();
                throw std::runtime_error("CmdResult::wait_until_done() timeout reached");
            }

            if (events == FAILURE) { //select broken
                if (errno == EINTR) {//by a signal
                    continue;
                }
                std::string err_msg(std::strerror(errno));
                throw std::runtime_error("CmdResult::wait_until_done() failure in select: " + err_msg);
            }

            //check child stdin
            if (child_std_in_.is_ready(wfds)) {
                if (data < data_end) {
                    const ::size_t wbytes = child_std_in_.write(data, data_end - data);
                    data += wbytes;
                    if (data_end <= data) {
                        child_std_in_.close();
                    }
                }
            }

            //check child stdout
            if (child_std_out_.is_ready(rfds)) {
                child_std_out_.append(out_.stdout);
            }

            //check child stderr
            if (child_std_err_.is_ready(rfds)) {
                child_std_err_.append(out_.stderr);
            }

        }

        //check for child completion
        const ::pid_t dp = ::waitpid(child_pid_, &out_.status, WNOHANG);//death pid

        switch (dp) {
        case FAILURE:
            throw std::runtime_error("CmdResult::wait_until_done() waitpid failure: " + std::string(std::strerror(errno)));
        case 0: //child is still running
            this->kill_child();
            break;
        default:
            if (dp != child_pid_) {
                std::ostringstream msg;
                msg << "CmdResult::wait_until_done() waitpid(" << child_pid_ << ") failure: return unexpected value " << dp;
                throw std::runtime_error(msg.str());
            }
            child_pid_ = IM_CHILD;//child done
            break;
        }
    }
    return out_;
}

void CmdResult::ImParent::kill_child()
{
    if ((child_pid_ == IM_CHILD) ||
        (child_pid_ == FAILURE)) {
        return;
    }

    try {
        Logging::Manager::instance_ref()
        .get(PACKAGE).error("CmdResult::kill_child call");
    }
    catch (...) { }

    if (::kill(child_pid_, SIGKILL) == FAILURE) {
        std::string err_msg(std::strerror(errno));
        try {
            Logging::Manager::instance_ref()
            .get(PACKAGE).error("CmdResult::kill_child kill() failure: " + err_msg);
        }
        catch (...) { }
        return;
    }

    enum { PASS_MAX = 10 };
    int pass = 0;
    while (true) {
        const ::pid_t dp = ::waitpid(child_pid_, &out_.status, WNOHANG); //death pid

        if (dp == child_pid_) {//if killed child done
            try {
                Logging::Manager::instance_ref()
                .get(PACKAGE).debug("CmdResult::ImParent::kill_child success: killed child done");
            }
            catch (...) { }
            child_pid_ = IM_CHILD;
            return;
        }

        if (dp == FAILURE) {
            try {
                std::string err_msg(std::strerror(errno));
                Logging::Manager::instance_ref()
                .get(PACKAGE).error("CmdResult::ImParent::kill_child waitpid() failure: " + err_msg);
            }
            catch (...) { }
            break;
        }

        ++pass;
        //killed child is still running
        if (PASS_MAX <= pass) {
            try {
                Logging::Manager::instance_ref()
                .get(PACKAGE).debug("CmdResult::ImParent::kill_child continue: killed child is still running");
            }
            catch (...) { }
            out_.status = EXIT_FAILURE;
            child_pid_ = IM_CHILD;
            return;
        }
        ::usleep(10 * 1000);//wait for child stop (SIGCHLD), max 10ms
    }
}

CmdResult::CmdResult(const std::string &_cmd, const Args &_args, bool _respecting_path_env)
{
    const ::pid_t child_pid = ::fork();
    if (child_pid == FAILURE) {
        std::string err_msg(std::strerror(errno));
        throw std::runtime_error("CmdResult::CmdResult() fork error: " + err_msg);
    }
    //parent and child now share the pipes
    if (child_pid != IM_CHILD) {//im parent
        parent_.reset(new ImParent(*this, child_pid));//create and store parent process data
        return;
    }
    //in a child
    try {
        //close writable end of stdin
        Pipe::ImReader stdin(std_in_);
        //close readable end of stdout and stderr
        Pipe::ImWriter stdout(std_out_);
        Pipe::ImWriter stderr(std_err_);

        //duplicate readable end of stdin pipe into stdin
        stdin.dup(STDIN_FILENO);
        //duplicate writable end of stdout pipe into stdout
        stdout.dup(STDOUT_FILENO);
        //duplicate writable end of stderr pipe into stderr
        stderr.dup(STDERR_FILENO);

        char *argv[1/*cmd*/ + _args.size() + 1/*NULL*/];
        char **argv_ptr = argv;
        *argv_ptr = const_cast< char* >(_cmd.c_str());
        ++argv_ptr;
        for (Args::const_iterator pArg = _args.begin(); pArg != _args.end(); ++pArg) {
            *argv_ptr = const_cast< char* >(pArg->c_str());
            ++argv_ptr;
        }
        *argv_ptr = NULL;

        //command execute
        const int retval = _respecting_path_env ? ::execvp(argv[0], argv)
                                                : ::execv (argv[0], argv);
        if (retval == FAILURE) {
            //failed to launch command
            std::string err_msg(std::strerror(errno));
            Logging::Manager::instance_ref()
            .get(PACKAGE).error("CmdResult::CmdResult execv failed: " + err_msg);
        }
    }
    catch (...) {
    }
    ::_exit(EXIT_FAILURE);
}

CmdResult::~CmdResult()
{
    try {
        parent_.reset(NULL);
    }
    catch(...) {
    }
}

const SubProcessOutput& CmdResult::wait_until_done(
    const std::string &_stdin_content,
    Seconds _rel_timeout)
{
    if (parent_.get() != NULL) {
        return parent_->wait_until_done(_stdin_content, _rel_timeout);
    }
    throw std::runtime_error("CmdResult::wait_until_done() invalid usage");
}

ShellCmd::ShellCmd(const std::string &_cmd)
:   cmd_(_cmd),
    shell_(DEFAULT_BASH),
    timeout_(DEFAULT_TIMEOUT_SEC)
{
}

ShellCmd::ShellCmd(const std::string &_cmd,
                   RelativeTimeInSeconds _timeout
                  )
:   cmd_(_cmd),
    shell_(DEFAULT_BASH),
    timeout_(_timeout)
{
}

ShellCmd::ShellCmd(const std::string &_cmd,
                   const std::string &_shell,
                   RelativeTimeInSeconds _timeout
                  )
:   cmd_(_cmd),
    shell_(_shell),
    timeout_(_timeout)
{
}

ShellCmd::~ShellCmd()
{
}

SubProcessOutput ShellCmd::execute(std::string stdin_str)
{
    CmdResult::Args args;
    args.push_back("-c");
    args.push_back(cmd_);
    CmdResult cmd(shell_, args, true);
    return cmd.wait_until_done(stdin_str, timeout_);
}
