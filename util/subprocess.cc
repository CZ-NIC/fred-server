#include "util/subprocess.h"

#include <pthread.h>
#include <sys/select.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

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

//public read end of pipe interface, hide write end of pipe
class ImReader;

//public write end of pipe interface, hide read end of pipe
class ImWriter;

class Pipe
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
        DESCRIPTORS_COUNT = 2
    };

    Pipe& close(Idx _idx);
    Pipe& dup(Idx _idx, int _new_fd);

    bool is_closed(Idx _idx)const;
    bool is_opened(Idx _idx)const;
    void watch(Idx _idx, ::fd_set &_set, int &_max_fd)const;
    bool is_set(Idx _idx, const ::fd_set &_set)const;
    void set_nonblocking(Idx _idx)const;

    int fd_[DESCRIPTORS_COUNT];

    friend class ImReader;
    friend class ImWriter;
};

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

SubProcessOutput cmd_run(
    const std::string &_stdin_content,
    const std::string &_cmd,
    bool _search_path,
    const std::vector< std::string > &_args,
    struct ::timeval *_timeout_ptr);

void kill_child(::pid_t _child_pid, int *_status);

}

namespace Cmd
{

Executable::Executable(std::string _cmd, bool _search_path)
:   cmd_(_cmd),
    search_path_(_search_path)
{
}

Executable::Executable(std::string _data, std::string _cmd, bool _search_path)
:   data_(_data),
    cmd_(_cmd),
    search_path_(_search_path)
{
}

Executable& Executable::operator()(std::string _arg)
{
    args_.push_back(_arg);
    return *this;
}

SubProcessOutput Executable::run()
{
    return cmd_run(data_, cmd_, search_path_, args_, NULL);
}

SubProcessOutput Executable::run(Seconds _max_lifetime_sec)
{
    struct ::timeval timeout;
    timeout.tv_sec = _max_lifetime_sec;
    timeout.tv_usec = 0;
    return cmd_run(data_, cmd_, search_path_, args_, &timeout);
}

Data::Data(std::string _data)
:   cmd_(NULL),
    data_(_data)
{
}

Data::~Data()
{
    try { delete cmd_; } catch (...) { } cmd_ = NULL;
}

Executable& Data::into(std::string _cmd, bool _search_path)
{
    delete cmd_;
    cmd_ = new Executable(data_, _cmd, _search_path);
    return *cmd_;
}

}

namespace
{

Pipe::Pipe()
{
    const int ret_val = ::pipe(fd_);
    if (ret_val != SUCCESS) {
        const int c_errno = errno;
        std::ostringstream msg;
        msg << __PRETTY_FUNCTION__ << " pipe() failure: " << std::strerror(c_errno);
        throw std::runtime_error(msg.str());
    }
}

Pipe::~Pipe()
{
    try { this->close_read_end(); } catch (...) { }
    try { this->close_write_end(); } catch (...) { }
}

Pipe& Pipe::close_read_end()
{
    if (this->is_opened(READ_END)) {
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
    msg << "Pipe::read() read() failure: " << std::strerror(c_errno);
    throw std::runtime_error(msg.str());
}

Pipe& Pipe::close_write_end()
{
    if (this->is_opened(WRITE_END)) {
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
    msg << "Pipe::write() write() failure: " << std::strerror(c_errno);
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
    msg << "Pipe::dup(" << _idx << ") dup2(" << fd_[_idx] << ", " << _new_fd << ") failure: " << std::strerror(c_errno);
    throw std::runtime_error(msg.str());
}

bool Pipe::is_closed(Idx _idx)const
{
    return fd_[_idx] == INVALID_DESCRIPTOR;
}

bool Pipe::is_opened(Idx _idx)const
{
    return !this->is_closed(_idx);
}

void Pipe::watch(Idx _idx, ::fd_set &_set, int &_max_fd)const
{
    if (this->is_opened(_idx)) {
        FD_SET(fd_[_idx], &_set);
        if (_max_fd < fd_[_idx]) {
            _max_fd = fd_[_idx];
        }
    }
}

bool Pipe::is_set(Idx _idx, const ::fd_set &_set)const
{
    return this->is_opened(_idx) && FD_ISSET(fd_[_idx], &_set);
}

void Pipe::set_nonblocking(Idx _idx)const
{
    long flags = ::fcntl(fd_[_idx], F_GETFL);
    if (flags == FAILURE) {
        const int c_errno = errno;
        std::ostringstream msg;
        msg << "Pipe::set_nonblocking(" << _idx << ") fcntl(" << fd_[_idx] << ", F_GETFL) failure: " << std::strerror(c_errno);
        throw std::runtime_error(msg.str());
    }
    flags |= O_NONBLOCK;
    if (::fcntl(fd_[_idx], F_SETFL, flags) != SUCCESS) {
        const int c_errno = errno;
        std::ostringstream msg;
        msg << "Pipe::set_nonblocking(" << _idx << ") fcntl(" << fd_[_idx] << ", F_SETFL, " << flags << ") failure: " << std::strerror(c_errno);
        throw std::runtime_error(msg.str());
    }
}

ImReader::ImReader(Pipe &_pipe)
:   pipe_(_pipe)
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

ImWriter::ImWriter(Pipe &_pipe)
:   pipe_(_pipe)
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

class TimedLockGuard
{
public:
    TimedLockGuard(
        ::pthread_mutex_t &_mtx,
        unsigned short _max_delay_sec)
    :   mtx_(_mtx)
    {
        struct ::timespec abs_time;
        ::clock_gettime(CLOCK_REALTIME, &abs_time);
        abs_time.tv_sec += _max_delay_sec;
        const int retval = ::pthread_mutex_timedlock(&mtx_, &abs_time);
        if (retval != SUCCESS) {
            throw std::runtime_error(std::string("pthread_mutex_timedlock() failure: ") +
                                     std::strerror(retval));
        }
    }
    ~TimedLockGuard()
    {
        ::pthread_mutex_unlock(&mtx_);
    }
private:
    ::pthread_mutex_t &mtx_;
};

::pthread_mutex_t cmd_run_mtx = PTHREAD_MUTEX_INITIALIZER;

SubProcessOutput cmd_run(
    const std::string &_stdin_content,
    const std::string &_cmd,
    bool _search_path,
    const std::vector< std::string > &_args,
    struct ::timeval *_timeout_ptr)
{
    enum { LOCK_GUARD_DELAY_MAX = 10 };
    TimedLockGuard lock(cmd_run_mtx, (_timeout_ptr == NULL) ||
                                     (LOCK_GUARD_DELAY_MAX < _timeout_ptr->tv_sec)
                                     ?   LOCK_GUARD_DELAY_MAX
                                     :   _timeout_ptr->tv_sec);
    Pipe std_in;
    Pipe std_out;
    Pipe std_err;

    const ::pid_t child_pid = ::fork();//parent and child now share the pipes

    if (child_pid == FAILURE) {
        std::string err_msg(std::strerror(errno));
        throw std::runtime_error("cmd_run() fork error: " + err_msg);
    }

    if (child_pid == IM_CHILD) {
        //in a child
        try {
            //close writable end of stdin
            ImReader stdin(std_in);
            //close readable end of stdout and stderr
            ImWriter stdout(std_out);
            ImWriter stderr(std_err);

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
            for (std::vector< std::string >::const_iterator item_ptr = _args.begin();
                 item_ptr != _args.end(); ++item_ptr)
            {
                *argv_ptr = const_cast< char* >(item_ptr->c_str());
                ++argv_ptr;
            }
            *argv_ptr = NULL;

            //command execute
            const int retval = _search_path ? ::execvp(argv[0], argv)
                                            : ::execv (argv[0], argv);
            if (retval == FAILURE) { //failed to launch command
                std::string err_msg(std::strerror(errno));
                Logging::Manager::instance_ref()
                .get(PACKAGE).error("cmd_run() execv failed: " + err_msg);
            }
        }
        catch (...) { }
        ::_exit(EXIT_FAILURE);
    }

    class SetSigHandler
    {
    public:
        SetSigHandler()
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
        ~SetSigHandler()
        {
            ::sigprocmask(SIG_SETMASK, &old_set_, NULL);
            ::sigaction(SIGCHLD, &old_act_, NULL);
        }
    private:
        static void handler(int) { }
        struct ::sigaction old_act_;
        ::sigset_t old_set_;
    } set_sig_handler;

    ImWriter child_stdin(std_in);
    child_stdin.set_nonblocking();
    ImReader child_stdout(std_out);
    child_stdout.set_nonblocking();
    ImReader child_stderr(std_err);
    child_stderr.set_nonblocking();

    SubProcessOutput out;

    //communication with child process
    const char *data = _stdin_content.c_str();
    const char *const data_end = data + _stdin_content.length();
    if (_stdin_content.empty()) { //no input data => close stdin
        child_stdin.close();
    }

    while (true) {
        //init fd sets for reading and writing
        ::fd_set rfds, wfds;
        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        int nfds = 0;

        //watch events on stdin, stdout, stderr
        child_stdin .watch(wfds, nfds);
        child_stdout.watch(rfds, nfds);
        child_stderr.watch(rfds, nfds);

        if (nfds <= 0) {//nothing to do
            break;
        }

        const int events = ::select(nfds + 1, &rfds, &wfds, NULL, _timeout_ptr);

        if (events == 0) {//no event => timeout
            child_stdin .close();
            child_stdout.close();
            child_stderr.close();
            kill_child(child_pid, NULL);
            throw std::runtime_error("cmd_run() failure: timeout reached");
        }

        if (events == FAILURE) { //select broken
            if (errno == EINTR) {//by a signal
                continue;
            }
            std::string err_msg(std::strerror(errno));
            throw std::runtime_error("cmd_run() failure in select: " + err_msg);
        }

        //check child's stdin
        if (child_stdin.is_ready(wfds)) {
            if (data < data_end) {
                const ::size_t wbytes = child_stdin.write(data, data_end - data);
                data += wbytes;
                if (data_end <= data) {
                    child_stdin.close();
                }
            }
        }

        //check child's stdout
        if (child_stdout.is_ready(rfds)) {
            child_stdout.append(out.stdout);
        }

        //check child stderr
        if (child_stderr.is_ready(rfds)) {
            child_stderr.append(out.stderr);
        }

    }

    //check for child completion
    const ::pid_t dp = ::waitpid(child_pid, &out.status, WNOHANG);//death pid

    switch (dp) {
    case FAILURE:
        throw std::runtime_error("cmd_run() waitpid failure: " + std::string(std::strerror(errno)));
    case 0: //child is still running
        kill_child(child_pid, &out.status);
        break;
    default:
        if (dp != child_pid) {
            std::ostringstream msg;
            msg << "cmd_run() waitpid(" << child_pid << ") failure: return unexpected value " << dp;
            throw std::runtime_error(msg.str());
        }
        break;
    }
    return out;
}

void kill_child(::pid_t _child_pid, int *_status)
{
    if ((_child_pid == IM_CHILD) ||
        (_child_pid == FAILURE)) {
        return;
    }

    enum { PASS_MAX = 10 };
    int pass = 0;
    bool signal_sent = false;
    while (true) {
        if (::usleep(1000) == FAILURE) {//wait for child stop (SIGCHLD), max 1ms
            const int c_errno = errno;
            if (c_errno != EINTR) {
                try {
                    Logging::Manager::instance_ref()
                    .get(PACKAGE).info("kill_child usleep() failure: " + std::string(std::strerror(c_errno)));
                }
                catch (...) { }
            }
        }
        const ::pid_t dp = ::waitpid(_child_pid, _status, WNOHANG); //death pid

        if (dp == _child_pid) {//if killed child done
            if (signal_sent) {
                try {
                    Logging::Manager::instance_ref()
                    .get(PACKAGE).debug("kill_child success: killed child done");
                }
                catch (...) { }
            }
            return;
        }

        if (dp == FAILURE) {
            try {
                std::string err_msg(std::strerror(errno));
                Logging::Manager::instance_ref()
                .get(PACKAGE).error("kill_child waitpid() failure: " + err_msg);
            }
            catch (...) { }
            break;
        }

        if (!signal_sent) {
            try {
                Logging::Manager::instance_ref()
                .get(PACKAGE).debug("kill_child call");
            }
            catch (...) { }

            if (::kill(_child_pid, SIGKILL) == FAILURE) {
                std::string err_msg(std::strerror(errno));
                try {
                    Logging::Manager::instance_ref()
                    .get(PACKAGE).error("kill_child kill() failure: " + err_msg);
                }
                catch (...) { }
                return;
            }
            signal_sent = true;
        }

        ++pass;
        //killed child is still running
        if (PASS_MAX <= pass) {
            try {
                Logging::Manager::instance_ref()
                .get(PACKAGE).debug("kill_child continue: killed child is still running");
            }
            catch (...) { }
            if (_status != NULL) {
                *_status = EXIT_FAILURE;
            }
            return;
        }
    }
}

}

ShellCmd::ShellCmd(std::string _cmd)
:   cmd_(_cmd),
    shell_(DEFAULT_BASH),
    timeout_(DEFAULT_TIMEOUT_SEC)
{
}

ShellCmd::ShellCmd(std::string _cmd,
                   RelativeTimeInSeconds _timeout
                  )
:   cmd_(_cmd),
    shell_(DEFAULT_BASH),
    timeout_(_timeout)
{
}

ShellCmd::ShellCmd(std::string _cmd,
                   std::string _shell,
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
    return Cmd::Data(stdin_str).into(shell_, false)("-c")(cmd_).run(timeout_);
}
