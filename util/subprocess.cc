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

class ImReader;
class ImWriter;

#define DEFAULT_BASH "/bin/sh"
enum { DEFAULT_TIMEOUT_SEC = 10 };

enum {
    FAILURE = -1,
    SUCCESS =  0,
    INVALID_DESCRIPTOR = -1,
    IM_CHILD = 0,
};

class Pipe:public boost::noncopyable
{
public:
    Pipe();
    ~Pipe();
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

    friend class ImReader;
    friend class ImWriter;

    int fd_[2];
};

//public read end of pipe interface, hide write end of pipe
class ImReader:public boost::noncopyable
{
public:
    ImReader(Pipe &_pipe);
    ~ImReader() { }

    ImReader& dup(int _new_fd);
    ImReader& close();
    ImReader& append(std::string &_buf);

    void set_nonblocking()const;
    void watch(::fd_set &_set, int &_max_fd)const;
    bool is_ready(const ::fd_set &_set)const;
    ::size_t read(void *_buf, ::size_t _buf_size)const;
private:
    Pipe &pipe_;
};

//public write end of pipe interface, hide read end of pipe
class ImWriter:public boost::noncopyable
{
public:
    ImWriter(Pipe &_pipe);
    ~ImWriter() { }

    ImWriter& dup(int _new_fd);
    ImWriter& close();

    void set_nonblocking()const;
    void watch(::fd_set &_set, int &_max_fd)const;
    bool is_ready(const ::fd_set &_set)const;
    ::size_t write(const void *_data, ::size_t _data_size)const;
private:
    Pipe &pipe_;
};

class SaveAndRestoreSigChldHandler
{
public:
    SaveAndRestoreSigChldHandler();
    ~SaveAndRestoreSigChldHandler();
private:
    static void handler(int) { }
    struct ::sigaction old_act_;
    ::sigset_t old_set_;
};

}

ShellCmd::ShellCmd(const std::string &_cmd)
    : cmd_(_cmd)
    , shell_(DEFAULT_BASH)
    , timeout_(DEFAULT_TIMEOUT_SEC)
    , child_pid_(IM_CHILD)
{
}

ShellCmd::ShellCmd(const std::string &_cmd,
                   RelativeTimeInSeconds _timeout
                  )
    : cmd_(_cmd)
    , shell_(DEFAULT_BASH)
    , timeout_(_timeout)
    , child_pid_(IM_CHILD)
{
}

ShellCmd::ShellCmd(const std::string &_cmd,
                   const std::string &_shell,
                   RelativeTimeInSeconds _timeout
                  )
    : cmd_(_cmd)
    , shell_(_shell)
    , timeout_(_timeout)
    , child_pid_(IM_CHILD)
{
}

ShellCmd::~ShellCmd()
{
    this->kill_child();//if in parent and have child
}

SubProcessOutput ShellCmd::execute(std::string stdin_str)
{
    Pipe pipe_stdin;
    Pipe pipe_stdout;
    Pipe pipe_stderr;

    child_pid_ = ::fork();
    if (child_pid_ == FAILURE) {
        std::string err_msg(std::strerror(errno));
        throw std::runtime_error("ShellCmd::execute fork error: " + err_msg);
    }
    //parent and child now share the pipe's
    if (child_pid_ == IM_CHILD) {
        //in child
        try {
            //close writable end of stdin
            ImReader stdin(pipe_stdin);
            //close readable end of stdout and stderr
            ImWriter stdout(pipe_stdout);
            ImWriter stderr(pipe_stderr);

            //duplicate readable end of stdin pipe into stdin
            stdin.dup(STDIN_FILENO);
            //duplicate writable end of stdout pipe into stdout
            stdout.dup(STDOUT_FILENO);
            //duplicate writable end of stderr pipe into stderr
            stderr.dup(STDERR_FILENO);

            char *shell_argv[] = {
                const_cast< char* >(shell_.c_str()),
                const_cast< char* >("-c"),
                const_cast< char* >(cmd_.c_str()),
                NULL
            };

            //shell exec
            if (::execv(shell_argv[0], shell_argv) == FAILURE) {
                //failed to launch the shell
                std::string err_msg(std::strerror(errno));
                Logging::Manager::instance_ref()
                .get(PACKAGE).error("ShellCmd::execute execv failed: " + err_msg);
            }
        }
        catch (...) {
        }
        ::_exit(EXIT_FAILURE);
    }

    //in parent
    //waitpid need default SIGCHLD handler to work
    SaveAndRestoreSigChldHandler save_and_restore_sig_handler;

    SubProcessOutput ret;
    ImWriter stdin(pipe_stdin);
    ImReader stdout(pipe_stdout);
    ImReader stderr(pipe_stderr);

    stdin.set_nonblocking();
    stdout.set_nonblocking();
    stderr.set_nonblocking();

    //set select timeout
    struct ::timeval timeout;
    timeout.tv_sec = timeout_;
    timeout.tv_usec = 0;

    //communication with child process
    int nfds;
    const char *data = stdin_str.c_str();
    const char *const data_end = data + stdin_str.length();
    if (stdin_str.empty()) { //no input data, close stdin
        stdin.close();
    }

    do {
        //init fd sets for reading and writing
        ::fd_set rfds, wfds;
        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        nfds = 0;

        //watch events on stdin, stdout, stderr
        stdin.watch(wfds, nfds);
        stdout.watch(rfds, nfds);
        stderr.watch(rfds, nfds);

        if (0 < nfds) {//if have fd to wait for

            const int events = ::select(nfds + 1, &rfds, &wfds, NULL, &timeout);

            if (events == 0) {//timeout
                stdin.close();
                stdout.close();
                stderr.close();
                this->kill_child();
                throw std::runtime_error("ShellCmd::check_timeout timeout cmd: " + cmd_);
            }

            if (events == FAILURE) {
                if (errno == EINTR) {
                    continue;
                }
                //error
                std::string err_msg(std::strerror(errno));
                throw std::runtime_error("ShellCmd::execute error in select: " + err_msg);
            }

            //check child stdin
            if (stdin.is_ready(wfds)) {
                if (data < data_end) {
                    const ::size_t wbytes = stdin.write(data, data_end - data);
                    data += wbytes;
                    if (data_end <= data) {
                        stdin_str.clear();
                        stdin.close();
                    }
                }
            }//if stdin ready

            //check child stdout
            if (stdout.is_ready(rfds)) {
                stdout.append(ret.stdout);
            }

            //check child stderr
            if (stderr.is_ready(rfds)) {
                stderr.append(ret.stderr);
            }

        }//if nfds

    }
    while(0 < nfds);  //communication with child process

    //check for child completion
    const ::pid_t dp = ::waitpid(child_pid_, &ret.status, WNOHANG);//death pid

    switch (dp) {
    case FAILURE:
        throw std::runtime_error("ShellCmd::execute error waitpid: " + std::string(std::strerror(errno)));
    case 0: //child is still running
        this->kill_child(&ret.status);
        break;
    default:
        if (dp != child_pid_) {
            std::ostringstream msg;
            msg << "ShellCmd::execute error waitpid(" << child_pid_ << "): return unexpected value " << dp;
            throw std::runtime_error(msg.str());
        }
        child_pid_ = IM_CHILD;//child done
        break;
    }

    return ret;//return outputs
}

void ShellCmd::kill_child(int *_status) throw()
{
    if (child_pid_ <= 0) {
        return;
    }

    ::kill(child_pid_, SIGKILL);

    try {
        Logging::Manager::instance_ref()
        .get(PACKAGE).error("ShellCmd::kill_child command: " + cmd_);
    }
    catch (...) { }

    enum { PASS_MAX = 10 };
    for (int pass = 0; pass < PASS_MAX; ++pass) {
        const ::pid_t dp = ::waitpid(child_pid_, _status, WNOHANG); //death pid

        if (dp == FAILURE) {
            try {
                std::string err_msg(std::strerror(errno));
                Logging::Manager::instance_ref()
                .get(PACKAGE).error("ShellCmd::kill_child error in waitpid: " + err_msg);
            }
            catch (...) { }
            break;
        }

        if (dp == child_pid_) {//if killed child done
            try {
                Logging::Manager::instance_ref()
                .get(PACKAGE).debug("ShellCmd::kill_child, killed child done, command: " + cmd_);
            }
            catch (...) { }
            child_pid_ = IM_CHILD;
            return;
        }

        //killed child is still running
        try {
        }
        catch (...) { }

        if ((pass + 1) < PASS_MAX) {
            ::usleep(10 * 1000);//wait for child stop (SIGCHLD), max 10ms
        }
        else {
            try {
                Logging::Manager::instance_ref()
                .get(PACKAGE).debug("ShellCmd::kill_child, killed child is still running, command: " + cmd_);
            }
            catch (...) { }
        }
    }
    if (_status != NULL) {
        *_status = EXIT_FAILURE;
    }
    child_pid_ = IM_CHILD;
}

namespace
{

Pipe::Pipe()
{
    const int ret_val = ::pipe(fd_);
    if (ret_val != SUCCESS) {
        const int c_errno = errno;
        std::ostringstream msg;
        msg << "Pipe::Pipe() pipe() call failure: " << std::strerror(c_errno);
        throw std::runtime_error(msg.str());
    }
}

Pipe::~Pipe()
{
    try {
        this->close_read_end();
    }
    catch (...) { }

    try {
        this->close_write_end();
    }
    catch (...) { }
}

Pipe& Pipe::close_read_end()
{
    if (this->is_open(READ_END)) {
        this->close(READ_END);
    }
    return *this;
}

Pipe& Pipe::dup_read_end(int _new_fd)
{
    return this->dup(READ_END, _new_fd);
}

void Pipe::set_nonblocking_read_end()const
{
    this->set_nonblocking(READ_END);
}

void Pipe::watch_ready_for_read(::fd_set &_set, int &_max_fd)const
{
    this->watch(READ_END, _set, _max_fd);
}

bool Pipe::is_ready_for_read(const ::fd_set &_set)const
{
    return this->is_set(READ_END, _set);
}

::size_t Pipe::read(void *_buf, ::size_t _buf_size)const
{
    const ::ssize_t bytes = ::read(fd_[READ_END], _buf, _buf_size);
    if (bytes != FAILURE) {
        return bytes;
    }
    const int c_errno = errno;
    std::ostringstream msg;
    msg << "Pipe::read() read() call failure: " << std::strerror(c_errno);
    throw std::runtime_error(msg.str());
}

Pipe& Pipe::close_write_end()
{
    if (this->is_open(WRITE_END)) {
        this->close(WRITE_END);
    }
    return *this;
}

Pipe& Pipe::dup_write_end(int _new_fd)
{
    return this->dup(WRITE_END, _new_fd);
}

void Pipe::set_nonblocking_write_end()const
{
    this->set_nonblocking(WRITE_END);
}

void Pipe::watch_ready_for_write(::fd_set &_set, int &_max_fd)const
{
    this->watch(WRITE_END, _set, _max_fd);
}

bool Pipe::is_ready_for_write(const ::fd_set &_set)const
{
    return this->is_set(WRITE_END, _set);
}

::size_t Pipe::write(const void *_data, ::size_t _data_size)const
{
    const ::ssize_t bytes = ::write(fd_[WRITE_END], _data, _data_size);
    if (bytes != FAILURE) {
        return bytes;
    }
    const int c_errno = errno;
    std::ostringstream msg;
    msg << "Pipe::write() write() call failure: " << std::strerror(c_errno);
    throw std::runtime_error(msg.str());
}

Pipe& Pipe::close(Idx _idx)
{
    const int ret_val = ::close(fd_[_idx]);
    if (ret_val == SUCCESS) {
        fd_[_idx] = INVALID_DESCRIPTOR;
        return *this;
    }
    const int c_errno = errno;
    std::ostringstream msg;
    msg << "Pipe::close(" << _idx << ") close(" << fd_[_idx] << ") call failure: " << std::strerror(c_errno);
    throw std::runtime_error(msg.str());
}

Pipe& Pipe::dup(Idx _idx, int _new_fd)
{
    const int ret_val = ::dup2(fd_[_idx], _new_fd);
    if (ret_val != FAILURE) {
        this->close(_idx);
        return *this;
    }
    const int c_errno = errno;
    std::ostringstream msg;
    msg << "Pipe::dup(" << _idx << ") dup2(" << fd_[_idx] << ", " << _new_fd << ") call failure: " << std::strerror(c_errno);
    throw std::runtime_error(msg.str());
}

bool Pipe::is_closed(Idx _idx)const
{
    return fd_[_idx] == INVALID_DESCRIPTOR;
}

bool Pipe::is_open(Idx _idx)const
{
    return !this->is_closed(_idx);
}

void Pipe::watch(Idx _idx, ::fd_set &_set, int &_max_fd)const
{
    if (this->is_open(_idx)) {
        FD_SET(fd_[_idx], &_set);
        if (_max_fd < fd_[_idx]) {
            _max_fd = fd_[_idx];
        }
    }
}

bool Pipe::is_set(Idx _idx, const ::fd_set &_set)const
{
    return this->is_open(_idx) && FD_ISSET(fd_[_idx], &_set);
}

void Pipe::set_nonblocking(Idx _idx)const
{
    long flags = ::fcntl(fd_[_idx], F_GETFL);
    if (flags == FAILURE) {
        const int c_errno = errno;
        std::ostringstream msg;
        msg << "Pipe::set_nonblocking(" << _idx << ") fcntl(" << fd_[_idx] << ", F_GETFL) call failure: " << std::strerror(c_errno);
        throw std::runtime_error(msg.str());
    }
    flags |= O_NONBLOCK;
    if (::fcntl(fd_[_idx], F_SETFL, flags) != SUCCESS) {
        const int c_errno = errno;
        std::ostringstream msg;
        msg << "Pipe::set_nonblocking(" << _idx << ") fcntl(" << fd_[_idx] << ", F_SETFL, " << flags << ") call failure: " << std::strerror(c_errno);
        throw std::runtime_error(msg.str());
    }
}

ImReader::ImReader(Pipe &_pipe):pipe_(_pipe)
{
    pipe_.close_write_end();
}

ImReader& ImReader::dup(int _new_fd)
{
    pipe_.dup_read_end(_new_fd);
    return *this;
}

ImReader& ImReader::close()
{
    pipe_.close_read_end();
    return *this;
}

ImReader& ImReader::append(std::string &_buf)
{
    char data[1024];
    const ::size_t bytes = this->read(data, sizeof(data));
    if (bytes == 0) {//eof
        this->close();
    }
    else {//have data in buf
        _buf.append(data, bytes);
    }
    return *this;
}

void ImReader::set_nonblocking()const
{
    pipe_.set_nonblocking_read_end();
}

void ImReader::watch(::fd_set &_set, int &_max_fd)const
{
    pipe_.watch_ready_for_read(_set, _max_fd);
}

bool ImReader::is_ready(const ::fd_set &_set)const
{
    return pipe_.is_ready_for_read(_set);
}

::size_t ImReader::read(void *_buf, ::size_t _buf_size)const
{
    return pipe_.read(_buf, _buf_size);
}

ImWriter::ImWriter(Pipe &_pipe):pipe_(_pipe)
{
    pipe_.close_read_end();
}

ImWriter& ImWriter::dup(int _new_fd)
{
    pipe_.dup_write_end(_new_fd);
    return *this;
}

ImWriter& ImWriter::close()
{
    pipe_.close_write_end();
    return *this;
}

void ImWriter::set_nonblocking()const
{
    pipe_.set_nonblocking_write_end();
}

void ImWriter::watch(::fd_set &_set, int &_max_fd)const
{
    pipe_.watch_ready_for_write(_set, _max_fd);
}

bool ImWriter::is_ready(const ::fd_set &_set)const
{
    return pipe_.is_ready_for_write(_set);
}

::size_t ImWriter::write(const void *_data, ::size_t _data_size)const
{
    return pipe_.write(_data, _data_size);
}

SaveAndRestoreSigChldHandler::SaveAndRestoreSigChldHandler()
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

SaveAndRestoreSigChldHandler::~SaveAndRestoreSigChldHandler()
{
    ::sigprocmask(SIG_SETMASK, &old_set_, NULL);
    ::sigaction(SIGCHLD, &old_act_, NULL);
}

}
