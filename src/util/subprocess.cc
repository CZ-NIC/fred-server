#include "src/util/subprocess.hh"

#include "config.h"
#ifndef HAVE_LOGGER
#error HAVE_LOGGER is required!
#endif
#include "util/log/logger.hh"

#include <sys/select.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <csignal>

#include <iostream>
#include <iomanip>
#include <stdexcept>

namespace {

const char *const default_shell = "/bin/sh";
const ShellCmd::RelativeTimeInSeconds default_timeout_sec = 10;

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
    struct Direction
    {
        enum Enum
        {
            read,
            write
        };
    };
    template < unsigned idx >
    class Descriptor
    {
    public:
        static int get(const Pipe &_p)
        {
            return _p.fd_[idx];
        }
        static void invalidate(Pipe &_p)
        {
            _p.fd_[idx] = invalid_;
        }
        static bool is_closed(const Pipe &_p)
        {
            return get(_p) == invalid_;
        }
        static bool is_opened(const Pipe &_p)
        {
            return !is_closed(_p);
        }
    private:
        static const int invalid_ = -1;
    };
    template < Direction::Enum, bool = false >
    struct DescriptorFor;
    template < bool x >
    struct DescriptorFor< Direction::read, x >:Descriptor< 0 > { };
    template < bool x >
    struct DescriptorFor< Direction::write, x >:Descriptor< 1 > { };

    template < Direction::Enum >
    Pipe& close();
    template < Direction::Enum >
    Pipe& dup(int _new_fd);
    template < Direction::Enum >
    void set_nonblocking()const;

    template < Direction::Enum >
    bool is_closed()const;
    template < Direction::Enum >
    void watch_ready_for(::fd_set &_set, int &_max_fd)const;
    template < Direction::Enum >
    bool is_ready_for(const ::fd_set &_set)const;
    ::size_t read(void *_buf, ::size_t _buf_size)const;
    ::size_t write(const void *_data, ::size_t _data_size)const;

    static const unsigned number_of_descriptors_ = 2;
    int fd_[number_of_descriptors_];

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
    bool is_closed()const;
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
    const ::timespec *_timeout_ptr);

void kill_child(::pid_t _child_pid);

} // namespace {anonymous}

namespace Cmd
{

Executable::Executable(std::string _cmd)
:   cmd_(_cmd)
{
}

Executable::Executable(std::string _data, std::string _cmd)
:   data_(_data),
    cmd_(_cmd)
{
}

Executable& Executable::operator()(std::string _arg)
{
    args_.push_back(_arg);
    return *this;
}

SubProcessOutput Executable::run()
{
    static const bool with_path = false;
    return cmd_run(data_, cmd_, with_path, args_, NULL);
}

SubProcessOutput Executable::run(Seconds _max_lifetime_sec)
{
    static const bool with_path = false;
    ::timespec timeout;
    timeout.tv_sec = _max_lifetime_sec;
    timeout.tv_nsec = 0;
    return cmd_run(data_, cmd_, with_path, args_, &timeout);
}

SubProcessOutput Executable::run_with_path()
{
    static const bool with_path = true;
    return cmd_run(data_, cmd_, with_path, args_, NULL);
}

SubProcessOutput Executable::run_with_path(Seconds _max_lifetime_sec)
{
    static const bool with_path = true;
    ::timespec timeout;
    timeout.tv_sec = _max_lifetime_sec;
    timeout.tv_nsec = 0;
    return cmd_run(data_, cmd_, with_path, args_, &timeout);
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

Executable& Data::into(std::string _cmd)
{
    delete cmd_;
    cmd_ = new Executable(data_, _cmd);
    return *cmd_;
}

} // namespace Cmd

namespace {

Pipe::Pipe()
{
    static const int success = 0;
    const int retval = ::pipe(fd_);
    if (retval != success)
    {
        throw std::runtime_error("Pipe() pipe() call failure: " + std::string(std::strerror(errno)));
    }
}

Pipe::~Pipe()
{
    try { this->close< Direction::read >(); } catch (...) { }
    try { this->close< Direction::write >(); } catch (...) { }
}

template < Pipe::Direction::Enum direction >
Pipe& Pipe::close()
{
    typedef DescriptorFor< direction > MyDescriptor;
    if (MyDescriptor::is_opened(*this))
    {
        static const int success = 0;
        const int retval = ::close(MyDescriptor::get(*this));
        if (retval == success)
        {
            MyDescriptor::invalidate(*this);
            return *this;
        }
        const int c_errno = errno;
        std::ostringstream msg;
        msg << "Pipe::close< " << direction << " >() close(" << MyDescriptor::get(*this) << ") call failure: "
            << std::strerror(c_errno);
        throw std::runtime_error(msg.str());
    }
    return *this;
}

template < Pipe::Direction::Enum direction >
Pipe& Pipe::dup(int _new_fd)
{
    typedef DescriptorFor< direction > MyDescriptor;
    static const int failure = -1;
    const int retval = ::dup2(MyDescriptor::get(*this), _new_fd);
    if (retval != failure)
    {
        return this->close< direction >();
    }
    const int c_errno = errno;
    std::ostringstream msg;
    msg << "Pipe::dup< " << direction << " >() dup2(" << MyDescriptor::get(*this) << ", " << _new_fd << ") "
           "call failure: " << std::strerror(c_errno);
    throw std::runtime_error(msg.str());
}

template < Pipe::Direction::Enum direction >
void Pipe::set_nonblocking()const
{
    typedef DescriptorFor< direction > MyDescriptor;
    static const long failure = -1;
    const long current_flags = ::fcntl(MyDescriptor::get(*this), F_GETFL);
    if (current_flags == failure)
    {
        const int c_errno = errno;
        std::ostringstream msg;
        msg << "Pipe::set_nonblocking< " << direction << " >() fcntl(" << MyDescriptor::get(*this) << ", F_GETFL) "
               "call failure: " << std::strerror(c_errno);
        throw std::runtime_error(msg.str());
    }
    const bool is_nonblocking = (current_flags & O_NONBLOCK) != 0;
    if (is_nonblocking)
    {
        return;
    }
    const long new_flags = current_flags | O_NONBLOCK;
    static const int success = 0;
    if (::fcntl(MyDescriptor::get(*this), F_SETFL, new_flags) != success)
    {
        const int c_errno = errno;
        std::ostringstream msg;
        msg << "Pipe::set_nonblocking< " << direction << " >() "
               "fcntl(" << MyDescriptor::get(*this) << ", F_SETFL, " << new_flags << ") "
               "call failure: " << std::strerror(c_errno);
        throw std::runtime_error(msg.str());
    }
}

template < Pipe::Direction::Enum direction >
bool Pipe::is_closed()const
{
    typedef DescriptorFor< direction > MyDescriptor;
    return MyDescriptor::is_closed(*this);
}

template < Pipe::Direction::Enum direction >
void Pipe::watch_ready_for(::fd_set &_set, int &_max_fd)const
{
    typedef DescriptorFor< direction > MyDescriptor;
    if (MyDescriptor::is_opened(*this))
    {
        FD_SET(MyDescriptor::get(*this), &_set);
        if (_max_fd < MyDescriptor::get(*this))
        {
            _max_fd = MyDescriptor::get(*this);
        }
    }
}

template < Pipe::Direction::Enum direction >
bool Pipe::is_ready_for(const ::fd_set &_set)const
{
    typedef DescriptorFor< direction > MyDescriptor;
    return MyDescriptor::is_opened(*this) && FD_ISSET(MyDescriptor::get(*this), &_set);
}

::size_t Pipe::read(void *_buf, ::size_t _buf_size)const
{
    static const ::ssize_t failure = -1;
    const ::ssize_t bytes = ::read(DescriptorFor< Direction::read >::get(*this), _buf, _buf_size);
    if (bytes == failure)
    {
        throw std::runtime_error("Pipe::read() read() failure: " + std::string(std::strerror(errno)));
    }
    return bytes;
}

::size_t Pipe::write(const void *_data, ::size_t _data_size)const
{
    static const ::ssize_t failure = -1;
    const ::ssize_t bytes = ::write(DescriptorFor< Direction::write >::get(*this), _data, _data_size);
    if (bytes == failure)
    {
        throw std::runtime_error("Pipe::write() write() failure: " + std::string(std::strerror(errno)));
    }
    return bytes;
}

ImReader::ImReader(Pipe &_pipe)
:   pipe_(_pipe)
{
    pipe_.close< Pipe::Direction::write >();
}

ImReader& ImReader::dup(int _new_fd)
{
    pipe_.dup< Pipe::Direction::read >(_new_fd);
    return *this;
}

ImReader& ImReader::close()
{
    pipe_.close< Pipe::Direction::read >();
    return *this;
}

ImReader& ImReader::append(std::string &_buf)
{
    static const unsigned data_chunk_size = 1024;
    char data[data_chunk_size];
    static const ::size_t end_of_file = 0;
    const ::size_t bytes = this->read(data, sizeof(data));
    if (bytes == end_of_file)
    {
        this->close();
    }
    else
    {
        _buf.append(data, bytes);
    }
    return *this;
}

void ImReader::set_nonblocking()const
{
    pipe_.set_nonblocking< Pipe::Direction::read >();
}

void ImReader::watch(::fd_set &_set, int &_max_fd)const
{
    pipe_.watch_ready_for< Pipe::Direction::read >(_set, _max_fd);
}

bool ImReader::is_closed()const
{
    return pipe_.is_closed< Pipe::Direction::read >();
}

bool ImReader::is_ready(const ::fd_set &_set)const
{
    return pipe_.is_ready_for< Pipe::Direction::read >(_set);
}

::size_t ImReader::read(void *_buf, ::size_t _buf_size)const
{
    return pipe_.read(_buf, _buf_size);
}

ImWriter::ImWriter(Pipe &_pipe)
:   pipe_(_pipe)
{
    pipe_.close< Pipe::Direction::read >();
}

ImWriter& ImWriter::dup(int _new_fd)
{
    pipe_.dup< Pipe::Direction::write >(_new_fd);
    return *this;
}

ImWriter& ImWriter::close()
{
    pipe_.close< Pipe::Direction::write >();
    return *this;
}

void ImWriter::set_nonblocking()const
{
    pipe_.set_nonblocking< Pipe::Direction::write >();
}

void ImWriter::watch(::fd_set &_set, int &_max_fd)const
{
    pipe_.watch_ready_for< Pipe::Direction::write >(_set, _max_fd);
}

bool ImWriter::is_ready(const ::fd_set &_set)const
{
    return pipe_.is_ready_for< Pipe::Direction::write >(_set);
}

::size_t ImWriter::write(const void *_data, ::size_t _data_size)const
{
    return pipe_.write(_data, _data_size);
}

class BlockSigchld
{
public:
    BlockSigchld()
    {
        const int retval = ::pthread_sigmask(SIG_SETMASK, NULL, &mask_);
        static const int success = 0;
        if (retval != success)
        {
            throw std::runtime_error("pthread_sigmask() failure: " + std::string(std::strerror(retval)));
        }
        static const int failure = -1;
        static const int sigchld_is_unblocked = 0;
        static const int sigchld_is_blocked   = 1;
        const int sigchld_blocking = ::sigismember(&mask_, SIGCHLD);
        switch (sigchld_blocking)
        {
        case failure:
            throw std::runtime_error("sigismember() failure: " + std::string(std::strerror(errno)));
        case sigchld_is_unblocked:
            add_blocking();
            break;
        case sigchld_is_blocked:
            remove_sigchld(mask_);
            break;
        default:
            throw std::runtime_error("sigismember() failure: unexpected return value");
        }
    }
    ~BlockSigchld() { }
    const ::sigset_t* get_unblocked_sigchild_mask()const
    {
        return &mask_;
    }
private:
    static void add_blocking()
    {
        ::sigset_t mask;
        static const int success = 0;
        if (::sigemptyset(&mask) != success)
        {
            throw std::runtime_error("sigemptyset() failure: " + std::string(std::strerror(errno)));
        }
        if (::sigaddset(&mask, SIGCHLD) != success)
        {
            throw std::runtime_error("sigaddset() failure: " + std::string(std::strerror(errno)));
        }
        const int retval = ::pthread_sigmask(SIG_BLOCK, &mask, NULL);
        if (retval != success)
        {
            throw std::runtime_error("pthread_sigmask() failure: " + std::string(std::strerror(retval)));
        }
    }
    static void remove_sigchld(::sigset_t &_mask)
    {
        static const int success = 0;
        if (::sigdelset(&_mask, SIGCHLD) != success)
        {
            throw std::runtime_error("sigdelset() failure: " + std::string(std::strerror(errno)));
        }
    }
    ::sigset_t mask_;
};

void check_waitpid_functionality();

class SetMySigchldHandler
{
public:
    SetMySigchldHandler()
    :   block_sigchld_()
    {
        const bool is_first_instance_created = (total_number_of_instances_ == 0);
        if (is_first_instance_created)
        {
            do_sigaction(NULL, &sigaction_to_restore_);
            has_to_be_sigaction_restored_ = !is_sigaction_correct(sigaction_to_restore_);
            if (has_to_be_sigaction_restored_)
            {
                this->set_correct_sigaction();
            }
            total_number_of_instances_ = 1;
            return;
        }
        struct ::sigaction current_sigaction;
        do_sigaction(NULL, &current_sigaction);
        if (!is_sigaction_correct(current_sigaction))
        {
            this->set_correct_sigaction();
        }
        ++total_number_of_instances_;
    }
    ~SetMySigchldHandler()
    {
        try
        {
            --total_number_of_instances_;
            if (has_to_be_sigaction_restored_ && (total_number_of_instances_ == 0))
            {
                do_sigaction(&sigaction_to_restore_, NULL);
            }
        }
        catch (...)
        {
        }
    }
    const ::sigset_t* get_unblocked_sigchild_mask()const
    {
        return block_sigchld_.get_unblocked_sigchild_mask();
    }
private:
    static void do_sigaction(const struct ::sigaction *_from_now_action, struct ::sigaction *_up_to_now_action)
    {
        static const int failure = -1;
        if (::sigaction(SIGCHLD, _from_now_action, _up_to_now_action) == failure)
        {
            throw std::runtime_error("sigaction() failure: " + std::string(std::strerror(errno)));
        }
    }
    static bool is_sigaction_correct(const struct ::sigaction &_sigaction)
    {
        return ((_sigaction.sa_flags & SA_SIGINFO) == 0) && (_sigaction.sa_handler == my_handler);
    }
    void set_correct_sigaction()const
    {
        struct ::sigaction correct_sigaction;
        correct_sigaction.sa_flags = 0;
        correct_sigaction.sa_handler = my_handler;
        static const int failure = -1;
        if (::sigemptyset(&correct_sigaction.sa_mask) == failure)
        {
            throw std::runtime_error("sigemptyset() failure: " + std::string(std::strerror(errno)));
        }
        do_sigaction(&correct_sigaction, NULL);
    }
    BlockSigchld block_sigchld_;
    static void my_handler(int) { }
    static struct ::sigaction sigaction_to_restore_;
    static bool has_to_be_sigaction_restored_;
    static unsigned total_number_of_instances_;
    friend void check_waitpid_functionality();
};

struct ::sigaction SetMySigchldHandler::sigaction_to_restore_;
bool SetMySigchldHandler::has_to_be_sigaction_restored_ = false;
unsigned SetMySigchldHandler::total_number_of_instances_ = 0;

void check_waitpid_functionality()
{
    struct ::sigaction sigchld_action;
    SetMySigchldHandler::do_sigaction(NULL, &sigchld_action);
    const bool wrong_sigchld_handler = ((sigchld_action.sa_flags & SA_SIGINFO) == 0) && (sigchld_action.sa_handler == SIG_IGN);
    if (wrong_sigchld_handler)
    {
        throw std::runtime_error("sigchld handler is SIG_IGN => waitpid will not work");
    }
    const bool wrong_sigchld_flag = ((sigchld_action.sa_flags & SA_NOCLDWAIT) != 0);
    if (wrong_sigchld_flag)
    {
        throw std::runtime_error("sigchld action flag is SA_NOCLDWAIT => waitpid will not work");
    }
}

bool is_child_running(::pid_t _child_pid, int *_status)
{
    static const ::pid_t child_is_still_running = 0;
    const ::pid_t exited_child_pid = ::waitpid(_child_pid, _status, WNOHANG);
    if (exited_child_pid == child_is_still_running)
    {
        return true;
    }
    if (exited_child_pid == _child_pid)
    {
        return false;
    }
    const std::string err_msg = std::string("is_child_running() waitpid failure: ") + std::strerror(errno);
    try {
        Logging::Manager::instance_ref().error(err_msg);
    }
    catch (...) { }
    throw std::runtime_error(err_msg);
}

class PselectTimeout
{
public:
    PselectTimeout(const ::timespec *_timeout_ptr)
    :   max_time_(add_to_now(_timeout_ptr))
    {
    }
    ~PselectTimeout() { }
    const ::timespec* time_to_limit()
    {
        if ((max_time_.tv_sec == no_timeout_.tv_sec) && (max_time_.tv_nsec == no_timeout_.tv_nsec))
        {
            return NULL;
        }
        const ::timespec current_time = now();
        if ((max_time_.tv_sec < current_time.tv_sec) ||
            ((max_time_.tv_sec == current_time.tv_sec) && (max_time_.tv_nsec <= current_time.tv_nsec)))
        {
            rest_to_timeout_.tv_sec  = 0;
            rest_to_timeout_.tv_nsec = 0;
        }
        else
        {
            const long nsec = max_time_.tv_nsec - current_time.tv_nsec;
            if (0 <= nsec)
            {
                rest_to_timeout_.tv_sec = max_time_.tv_sec - current_time.tv_sec;
                rest_to_timeout_.tv_nsec = nsec;
            }
            else
            {
                rest_to_timeout_.tv_sec = max_time_.tv_sec - current_time.tv_sec - 1;
                rest_to_timeout_.tv_nsec = nsec + nsec_per_sec;

            }
        }
        return &rest_to_timeout_;
    }
private:
    static ::timespec now()
    {
        ::timespec current_time;
        static const int success = 0;
        static const int error = -1;
        switch (::clock_gettime(CLOCK_MONOTONIC, &current_time))
        {
        case success:
            return current_time;
        case error:
            throw std::runtime_error("clock_gettime() failure: " + std::string(std::strerror(errno)));
        default:
            throw std::runtime_error("clock_gettime() failure: unexpected return value");
        }
    }
    static ::timespec add_to_now(const ::timespec *b_ptr)
    {
        if (b_ptr == NULL)
        {
            return no_timeout_;
        }
        const ::timespec a = now();
        ::timespec sum;
        const ::time_t nsec = a.tv_nsec + b_ptr->tv_nsec;
        sum.tv_sec = a.tv_sec + b_ptr->tv_sec + (nsec / nsec_per_sec);
        sum.tv_nsec = nsec % nsec_per_sec;
        return sum;
    }
    const ::timespec max_time_;
    ::timespec rest_to_timeout_;
    static const ::timespec no_timeout_;
    static const long nsec_per_sec = 1000000000l;
};

const ::timespec PselectTimeout::no_timeout_ = { 0, 0 };

void cmd_process(
        const std::string &_cmd,
        bool _search_path,
        const std::vector< std::string > &_args,
        ImReader &_cmd_stdin_from_child,
        ImWriter &_cmd_stdout_to_parent,
        ImWriter &_cmd_stderr_to_parent)
{
    try
    {
        //duplicate readable end of stdin pipe into stdin
        _cmd_stdin_from_child.dup(STDIN_FILENO);
        //duplicate writable end of stdout pipe into stdout
        _cmd_stdout_to_parent.dup(STDOUT_FILENO);
        //duplicate writable end of stderr pipe into stderr
        _cmd_stderr_to_parent.dup(STDERR_FILENO);

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
        static const int failure = -1;
        const int retval = _search_path ? ::execvp(argv[0], argv)
                                        : ::execv (argv[0], argv);
        if (retval == failure)
        {
            const std::string err_msg(std::strerror(errno));
            Logging::Manager::instance_ref()
            .error("cmd_process() execv failed: " + err_msg);
        }
        else
        {
            Logging::Manager::instance_ref()
            .error("cmd_process() execv should never return without error");
        }
    }
    catch (...) { }
    ::_exit(EXIT_FAILURE);
}

int single_threaded_child(
        const std::string &_stdin_content,
        const std::string &_cmd,
        bool _search_path,
        const std::vector< std::string > &_args,
        const ::timespec *_timeout_ptr,
        ImWriter &_cmd_stdout_to_parent,
        ImWriter &_cmd_stderr_to_parent,
        ImWriter &_cmd_status_to_parent)
{
    PselectTimeout timeout_tools(_timeout_ptr);

    check_waitpid_functionality();

    Pipe cmd_stdin;

    SetMySigchldHandler sigchld_tools;

    static const ::pid_t fork_failure = -1;
    static const ::pid_t im_child     =  0;

    const ::pid_t child_pid = ::fork();

    if (child_pid == fork_failure)
    {
        throw std::runtime_error("cmd_run() fork error: " + std::string(std::strerror(errno)));
    }

    if (child_pid == im_child)
    {
        try
        {
            ImReader cmd_stdin_from_child(cmd_stdin);
            cmd_process(_cmd,
                        _search_path,
                        _args,
                        cmd_stdin_from_child,
                        _cmd_stdout_to_parent,
                        _cmd_stderr_to_parent);
        }
        catch (...) { }
        ::_exit(EXIT_FAILURE);
    }

    ImWriter cmd_stdin_from_me(cmd_stdin);
    cmd_stdin_from_me.set_nonblocking();

    const bool no_data_for_cmd = _stdin_content.empty();
    const char *data = no_data_for_cmd ? NULL : _stdin_content.c_str();
    const char *const data_end = no_data_for_cmd ? NULL : data + _stdin_content.length();
    if (no_data_for_cmd)
    {
        cmd_stdin_from_me.close();
    }

    const ::sigset_t *const unblocked_sigchld = sigchld_tools.get_unblocked_sigchild_mask();
    bool cmd_is_done = false;
    int status;

    while (!cmd_is_done)
    {
        ::fd_set write_event;
        FD_ZERO(&write_event);
        int max_fd = -1;

        cmd_stdin_from_me.watch(write_event, max_fd);
        const int size_of_set = (max_fd < 0) ? 0 : max_fd + 1;

        static const int failure = -1;
        const int number_of_events = ::pselect(size_of_set, NULL, &write_event, NULL,
                                               timeout_tools.time_to_limit(), unblocked_sigchld);

        if (number_of_events == failure)//pselect broken
        {
            if (errno == EINTR)         //by a signal
            {
                if (!cmd_is_done)
                {
                    cmd_is_done = !is_child_running(child_pid, &status);
                }
                continue;
            }
            return EXIT_FAILURE;
        }

        const bool time_is_up = (number_of_events == 0);
        if (time_is_up)
        {
            if (!cmd_is_done)
            {
                kill_child(child_pid);
            }
            return EXIT_SUCCESS;//timeout
        }

        if (cmd_stdin_from_me.is_ready(write_event))
        {
            if (data < data_end)
            {
                const ::size_t wbytes = cmd_stdin_from_me.write(data, data_end - data);
                data += wbytes;
                if (data_end <= data)
                {
                    cmd_stdin_from_me.close();
                }
            }
        }
    }

    {
        const char *data_ptr = reinterpret_cast< const char* >(&status);
        const char *const data_end = data_ptr + sizeof(status);
        while (data_ptr < data_end)
        {
            const ::size_t bytes = _cmd_status_to_parent.write(data_ptr, data_end - data_ptr);
            data_ptr += bytes;
        }
        _cmd_status_to_parent.close();
    }

    return EXIT_SUCCESS;
}

SubProcessOutput cmd_run(
    const std::string &_stdin_content,
    const std::string &_cmd,
    bool _search_path,
    const std::vector< std::string > &_args,
    const ::timespec *_timeout_ptr)
{
    check_waitpid_functionality();

    Pipe child_stdout;
    Pipe child_stderr;
    Pipe cmd_status;

    static const ::pid_t fork_failure = -1;
    static const ::pid_t im_child     =  0;

    //multi-threaded parent -> single-threaded child -> cmd
    const ::pid_t child_pid = ::fork();

    if (child_pid == fork_failure)
    {
        throw std::runtime_error("cmd_run() fork error: " + std::string(std::strerror(errno)));
    }

    if (child_pid == im_child)
    {
        try
        {
            ImWriter cmd_stdout_to_parent(child_stdout);
            ImWriter cmd_stderr_to_parent(child_stderr);
            ImWriter cmd_status_to_parent(cmd_status);
            const int exit_status = single_threaded_child(_stdin_content,
                                                          _cmd,
                                                          _search_path,
                                                          _args,
                                                          _timeout_ptr,
                                                          cmd_stdout_to_parent,
                                                          cmd_stderr_to_parent,
                                                          cmd_status_to_parent);
            cmd_stdout_to_parent.close();
            cmd_stderr_to_parent.close();
            cmd_status_to_parent.close();
            ::_exit(exit_status);
        }
        catch (...) { }
        ::_exit(EXIT_FAILURE);
    }

    ImReader child_stdout_for_me(child_stdout);
    child_stdout_for_me.set_nonblocking();
    ImReader child_stderr_for_me(child_stderr);
    child_stderr_for_me.set_nonblocking();
    ImReader cmd_status_for_me(cmd_status);
    cmd_status_for_me.set_nonblocking();

    SubProcessOutput out;
    std::string cmd_status_buf;
    cmd_status_buf.reserve(sizeof(out.status));
    bool child_is_running = true;
    int child_status;

    while (true)
    {
        ::fd_set read_event;
        FD_ZERO(&read_event);
        int max_fd = -1;

        //watch events on stdout, stderr, cmdstatus
        child_stdout_for_me.watch(read_event, max_fd);
        child_stderr_for_me.watch(read_event, max_fd);
        cmd_status_for_me.watch(read_event, max_fd);
        const bool all_pipes_closed = (max_fd < 0);
        if (all_pipes_closed)
        {
            break;
        }
        const int size_of_set = max_fd + 1;

        static const int failure = -1;
        const int number_of_events = ::select(size_of_set, &read_event, NULL, NULL, NULL);

        if (number_of_events == failure) //select broken
        {
            if (errno == EINTR)//by a signal
            {
                if (child_is_running)
                {
                    child_is_running = is_child_running(child_pid, &child_status);
                }
                continue;
            }
            throw std::runtime_error("cmd_run() failure in select: " + std::string(std::strerror(errno)));
        }

        const bool time_is_up = (number_of_events == 0);
        if (time_is_up)
        {
            if (child_is_running)
            {
                kill_child(child_pid);
            }
            throw std::runtime_error("cmd_run() select failure: unexpected timeout reached");
        }

        if (child_stdout_for_me.is_ready(read_event))
        {
            child_stdout_for_me.append(out.stdout);
        }

        if (child_stderr_for_me.is_ready(read_event))
        {
            child_stderr_for_me.append(out.stderr);
        }

        if (cmd_status_for_me.is_ready(read_event))
        {
            cmd_status_for_me.append(cmd_status_buf);
        }
    }

    if (child_is_running)
    {
        while (true)
        {
            static const ::pid_t failure = -1;
            const ::pid_t exited_child = ::waitpid(child_pid, &child_status, 0);
            if (exited_child == child_pid)
            {
                break;
            }
            if (exited_child == failure)
            {
                if (errno == EINTR)
                {
                    continue;
                }
                throw std::runtime_error("cmd_run() waitpid failure: " + std::string(std::strerror(errno)));
            }
            throw std::runtime_error("cmd_run() waitpid failure: unexpected return value");
        }
    }
    const bool child_exited_successfully = WIFEXITED(child_status) && (WEXITSTATUS(child_status) == EXIT_SUCCESS);
    if (!child_exited_successfully)
    {
        throw std::runtime_error("cmd_run() failure: child process failure");
    }
    const bool cmd_runtime_expired = cmd_status_buf.empty();
    if (cmd_runtime_expired)
    {
        throw std::runtime_error("cmd_run() failure: command runtime expired");
    }
    const bool status_received = (cmd_status_buf.length() == sizeof(out.status));
    if (!status_received)
    {
        throw std::runtime_error("cmd_run() failure: command process exit status receiving failure");
    }
    std::memcpy(&out.status, cmd_status_buf.c_str(), sizeof(out.status));
    return out;
}

void kill_child(::pid_t _child_pid)
{
    const bool pid_means_one_process = (0 < _child_pid);
    if (!pid_means_one_process)
    {
        return;
    }

    static const ::pid_t waitpid_failure = -1;
    const ::pid_t exited_child_pid = ::waitpid(_child_pid, NULL, WNOHANG);
    const bool child_exited = (exited_child_pid == _child_pid);
    if (child_exited)
    {
        return;
    }
    const bool not_my_child = (exited_child_pid == waitpid_failure) && (errno == ECHILD);
    if (not_my_child)
    {
        return;
    }

    try { Logging::Manager::instance_ref().debug("kill_child call"); } catch (...) { }

    static const int kill_failure = -1;
    if (::kill(_child_pid, SIGKILL) == kill_failure)
    {
        const bool process_does_not_exist = (errno == ESRCH);
        if (process_does_not_exist)
        {
            const ::pid_t exited_child_pid = ::waitpid(_child_pid, NULL, WNOHANG);
            const bool child_exited = (exited_child_pid == _child_pid);
            if (child_exited)
            {
                return;
            }
        }
        const std::string err_msg = std::string("kill_child kill() failure: ") + std::strerror(errno);
        try
        {
            Logging::Manager::instance_ref().error(err_msg);
        }
        catch (...) { }
        throw std::runtime_error(err_msg);
    }

    while (true)
    {
        const ::pid_t exited_child_pid = ::waitpid(_child_pid, NULL, 0);
        const bool child_exited = (exited_child_pid == _child_pid);
        if (child_exited)
        {
            break;
        }
        const bool not_my_child = (exited_child_pid == waitpid_failure) && (errno == ECHILD);
        if (not_my_child)
        {
            break;
        }
        const bool interrupted_by_signal = (exited_child_pid == waitpid_failure) && (errno == EINTR);
        if (!interrupted_by_signal)
        {
            const std::string err_msg("kill_child waitpid() failure: " + std::string(std::strerror(errno)));
            try
            {
                Logging::Manager::instance_ref().error(err_msg);
            }
            catch (...) { }
            throw std::runtime_error(err_msg);
        }
    }
    try
    {
        Logging::Manager::instance_ref()
        .debug("kill_child success: killed child done");
    }
    catch (...) { }
}

} // namespace {anonymous}

ShellCmd::ShellCmd(std::string _cmd)
:   cmd_(_cmd),
    shell_(default_shell),
    timeout_(default_timeout_sec)
{
}

ShellCmd::ShellCmd(std::string _cmd,
                   RelativeTimeInSeconds _timeout
                  )
:   cmd_(_cmd),
    shell_(default_shell),
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
    return Cmd::Data(stdin_str).into(shell_)("-c")(cmd_).run(timeout_);
}
