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

//public read end of pipe interface, hide write end of pipe
class CmdResult::ImReader:public boost::noncopyable
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

//public write end of pipe interface, hide read end of pipe
class CmdResult::ImWriter:public boost::noncopyable
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

CmdResult::ImReader::ImReader(DataChannel &_data_channel)
:   pipe_(_data_channel)
{
    pipe_.close_write_end();
}

CmdResult::ImReader& CmdResult::ImReader::dup(int _new_fd)
{
    pipe_.dup_read_end(_new_fd);
    return *this;
}

CmdResult::ImReader& CmdResult::ImReader::close()
{
    pipe_.close_read_end();
    return *this;
}

CmdResult::ImReader& CmdResult::ImReader::append(std::string &_buf)
{
    enum { DATA_CHUNK_SIZE = 1024 };
    char data[DATA_CHUNK_SIZE];
    const ::size_t bytes = this->read(data, DATA_CHUNK_SIZE);
    if (bytes == 0) {//eof
        this->close();
    }
    else {//have data in buf
        _buf.append(data, bytes);
    }
    return *this;
}

void CmdResult::ImReader::set_nonblocking()const
{
    pipe_.set_nonblocking_read_end();
}

void CmdResult::ImReader::watch(::fd_set &_set, int &_max_fd)const
{
    pipe_.watch_ready_for_read(_set, _max_fd);
}

bool CmdResult::ImReader::is_ready(const ::fd_set &_set)const
{
    return pipe_.is_ready_for_read(_set);
}

::size_t CmdResult::ImReader::read(void *_buf, ::size_t _buf_size)const
{
    return pipe_.read(_buf, _buf_size);
}

CmdResult::ImWriter::ImWriter(DataChannel &_data_channel)
:   pipe_(_data_channel)
{
    pipe_.close_read_end();
}

CmdResult::ImWriter& CmdResult::ImWriter::dup(int _new_fd)
{
    pipe_.dup_write_end(_new_fd);
    return *this;
}

CmdResult::ImWriter& CmdResult::ImWriter::close()
{
    pipe_.close_write_end();
    return *this;
}

void CmdResult::ImWriter::set_nonblocking()const
{
    pipe_.set_nonblocking_write_end();
}

void CmdResult::ImWriter::watch(::fd_set &_set, int &_max_fd)const
{
    pipe_.watch_ready_for_write(_set, _max_fd);
}

bool CmdResult::ImWriter::is_ready(const ::fd_set &_set)const
{
    return pipe_.is_ready_for_write(_set);
}

::size_t CmdResult::ImWriter::write(const void *_data, ::size_t _data_size)const
{
    return pipe_.write(_data, _data_size);
}

class CmdResult::SaveAndRestoreSigChldHandler
{
public:
    SaveAndRestoreSigChldHandler();
    ~SaveAndRestoreSigChldHandler();
private:
    static void handler(int) { }
    struct ::sigaction old_act_;
    ::sigset_t old_set_;
};

CmdResult::SaveAndRestoreSigChldHandler::SaveAndRestoreSigChldHandler()
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
}

CmdResult::SaveAndRestoreSigChldHandler::~SaveAndRestoreSigChldHandler()
{
    ::sigprocmask(SIG_SETMASK, &old_set_, NULL);
    ::sigaction(SIGCHLD, &old_act_, NULL);
}

CmdResult::CmdResult(const std::string &_cmd, const Args &_args, bool _respecting_path_env)
:   child_pid_(::fork())
{
    if (child_pid_ == FAILURE) {
        std::string err_msg(std::strerror(errno));
        throw std::runtime_error("CmdResult::CmdResult() fork error: " + err_msg);
    }
    //parent and child now share the pipe's
    if (child_pid_ == IM_CHILD) {
        //in child
        try {

            //close writable end of stdin
            ImReader stdin(std_in_);
            //close readable end of stdout and stderr
            ImWriter stdout(std_out_);
            ImWriter stderr(std_err_);

            //duplicate readable end of stdin pipe into stdin
            stdin.dup(STDIN_FILENO);
            //duplicate writable end of stdout pipe into stdout
            stdout.dup(STDOUT_FILENO);
            //duplicate writable end of stderr pipe into stderr
            stderr.dup(STDERR_FILENO);

            char *argv[1/*cmd*/ + _args.size() + 1/*NULL*/];
            argv[0] = const_cast< char* >(_cmd.c_str());
            char **argv_ptr = argv + 1;
            for (Args::const_iterator pArg = _args.begin(); pArg != _args.end(); ++pArg) {
                *argv_ptr = const_cast< char* >(pArg->c_str());
                ++argv_ptr;
            }
            *argv_ptr = NULL;

            //shell exec
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

    save_and_restore_sig_chld_handler_.reset(new SaveAndRestoreSigChldHandler);

    child_std_in_ .reset(new ImWriter(std_in_));
    child_std_out_.reset(new ImReader(std_out_));
    child_std_err_.reset(new ImReader(std_err_));

    child_std_in_ ->set_nonblocking();
    child_std_out_->set_nonblocking();
    child_std_err_->set_nonblocking();
}

CmdResult::~CmdResult()
{
    try {
        this->kill_child();
    }
    catch(...) {
    }
}

bool CmdResult::is_process_done()const
{
    return (child_pid_ == IM_CHILD) || (child_std_in_.get() == NULL);
}

CmdResult& CmdResult::wait_until_done(const std::string &_stdin_content, Seconds _rel_timeout)
{
    if (this->is_process_done()) {
        throw std::runtime_error("CmdResult::wait_until_done() invalid repetitive usage");
    }
    //set select timeout
    struct ::timeval timeout;
    struct ::timeval *timeout_ptr;
    if (_rel_timeout != INFINITE_TIME) {
        timeout.tv_sec = _rel_timeout;
        timeout.tv_usec = 0;
        timeout_ptr = &timeout;
    }
    else {
        timeout_ptr = NULL;
    }

    //communication with child process
    int nfds;
    const char *data = _stdin_content.c_str();
    const char *const data_end = data + _stdin_content.length();
    if (_stdin_content.empty()) { //no input data, close stdin
        child_std_in_->close();
    }

    do {
        //init fd sets for reading and writing
        ::fd_set rfds, wfds;
        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        nfds = 0;

        //watch events on stdin, stdout, stderr
        child_std_in_ ->watch(wfds, nfds);
        child_std_out_->watch(rfds, nfds);
        child_std_err_->watch(rfds, nfds);

        if (0 < nfds) {//if have fd to wait for

            const int events = ::select(nfds + 1, &rfds, &wfds, NULL, timeout_ptr);

            if (events == 0) {//timeout
                child_std_in_ ->close();
                child_std_out_->close();
                child_std_err_->close();
                this->kill_child();
                throw std::runtime_error("CmdResult::wait_until_done() timeout reached");
            }

            if (events == FAILURE) {
                if (errno == EINTR) {
                    continue;
                }
                //error
                std::string err_msg(std::strerror(errno));
                throw std::runtime_error("CmdResult::wait_until_done() failure in select: " + err_msg);
            }

            //check child stdin
            if (child_std_in_->is_ready(wfds)) {
                if (data < data_end) {
                    const ::size_t wbytes = child_std_in_->write(data, data_end - data);
                    data += wbytes;
                    if (data_end <= data) {
                        child_std_in_->close();
                    }
                }
            }//if stdin ready

            //check child stdout
            if (child_std_out_->is_ready(rfds)) {
                child_std_out_->append(stdout_content_);
            }

            //check child stderr
            if (child_std_err_->is_ready(rfds)) {
                child_std_err_->append(stderr_content_);
            }

        }//if nfds

    }
    while(0 < nfds);  //communication with child process

    //check for child completion
    const ::pid_t dp = ::waitpid(child_pid_, &exit_status_, WNOHANG);//death pid

    switch (dp) {
    case FAILURE:
        throw std::runtime_error("CmdResult::wait_until_done() waitpid failure: " + std::string(std::strerror(errno)));
    case 0: //child is still running
        this->kill_child(&exit_status_);
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
    child_std_err_.reset(NULL);
    child_std_out_.reset(NULL);
    child_std_in_ .reset(NULL);
    return *this;
}

bool CmdResult::is_exited()const
{
    if (this->is_process_done()) {
        return WIFEXITED(exit_status_);
    }
    throw std::runtime_error("CmdResult::is_exited() failure: process is still running");
}

int CmdResult::get_exit_status()const
{
    if (this->is_process_done()) {
        return WEXITSTATUS(exit_status_);
    }
    throw std::runtime_error("CmdResult::get_exit_status() failure: process is still running");
}

bool CmdResult::is_signaled()const
{
    if (this->is_process_done()) {
        return WIFSIGNALED(exit_status_);
    }
    throw std::runtime_error("CmdResult::is_signaled() failure: process is still running");
}

int CmdResult::get_term_sig()const
{
    if (this->is_process_done()) {
        return WTERMSIG(exit_status_);
    }
    throw std::runtime_error("CmdResult::get_term_sig() failure: process is still running");
}

#ifdef WCOREDUMP
bool CmdResult::is_core_dump()const
{
    if (this->is_process_done()) {
        return this->is_signaled() && (WCOREDUMP(exit_status_));
    }
    throw std::runtime_error("CmdResult::is_core_dump() failure: process is still running");
}
#endif

std::string CmdResult::get_stdout()const
{
    if (this->is_process_done()) {
        return stdout_content_;
    }
    throw std::runtime_error("CmdResult::get_stdout() failure: process is still running");
}

std::string CmdResult::get_stderr()const
{
    if (this->is_process_done()) {
        return stderr_content_;
    }
    throw std::runtime_error("CmdResult::get_stderr() failure: process is still running");
}

void CmdResult::kill_child(int *_status) throw()
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
    for (int pass = 0; ; ++pass) {
        const ::pid_t dp = ::waitpid(child_pid_, _status, WNOHANG); //death pid

        if (dp == FAILURE) {
            try {
                std::string err_msg(std::strerror(errno));
                Logging::Manager::instance_ref()
                .get(PACKAGE).error("CmdResult::kill_child error in waitpid: " + err_msg);
            }
            catch (...) { }
            break;
        }

        if (dp == child_pid_) {//if killed child done
            try {
                Logging::Manager::instance_ref()
                .get(PACKAGE).debug("CmdResult::kill_child, killed child done");
            }
            catch (...) { }
            child_pid_ = IM_CHILD;
            return;
        }

        //killed child is still running
        if (PASS_MAX <= (pass + 1)) {
            try {
                Logging::Manager::instance_ref()
                .get(PACKAGE).debug("CmdResult::kill_child, killed child is still running");
            }
            catch (...) { }
            break;
        }
        ::usleep(10 * 1000);//wait for child stop (SIGCHLD), max 10ms
    }
    if (_status != NULL) {
        *_status = EXIT_FAILURE;
    }
    child_pid_ = IM_CHILD;
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
    SubProcessOutput ret;
    CmdResult::Args args;
    args.push_back("-c");
    args.push_back(cmd_);
    CmdResult cmd(shell_, args, true);
    cmd.wait_until_done(stdin_str, timeout_);
    ret.stdout = cmd.get_stdout();
    ret.stderr = cmd.get_stderr();
    ret.status = cmd.get_raw_exit_status();
    return ret;//return outputs
}
