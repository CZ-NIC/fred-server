/*
 * Copyright (C) 2022  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "test/poc/parallel-tests/setup/run_in_background.hh"
#include "test/poc/parallel-tests/setup/arguments.hh"


#include <fcntl.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <utility>

namespace Test {
namespace Util {

namespace {

constexpr int invalid_descriptor = -1;

class Pipe
{
public:
    Pipe()
    {
        constexpr auto success = 0;
        if (::pipe(fd_) != success)
        {
            const auto c_errno = errno;
            throw std::runtime_error{std::strerror(c_errno)};
        }
    }
    ~Pipe()
    {
        if (fd_[0] != invalid_descriptor)
        {
            ::close(fd_[0]);
        }
        if (fd_[1] != invalid_descriptor)
        {
            ::close(fd_[1]);
        }
    }
    int get_nonblocking_read_descriptor() &&
    {
        int read_fd = invalid_descriptor;
        std::swap(fd_[0], read_fd);
        if (fd_[1] != invalid_descriptor)
        {
            ::close(fd_[1]);
            fd_[1] = invalid_descriptor;
        }
        set_nonblocking(read_fd);
        return read_fd;
    }
    void redirect_write_descriptor(int dst_fd) &&
    {
        int write_fd = invalid_descriptor;
        std::swap(fd_[1], write_fd);
        if (fd_[0] != invalid_descriptor)
        {
            ::close(fd_[0]);
            fd_[0] = invalid_descriptor;
        }
        if (write_fd != dst_fd)
        {
            static constexpr int failure = -1;
            if (::dup2(write_fd, dst_fd) == failure)
            {
                const auto c_errno = errno;
                throw std::runtime_error{std::strerror(c_errno)};
            }
            ::close(write_fd);
        }
    }
private:
    static void set_nonblocking(int fd)
    {
        static constexpr auto failure = -1;
        const long current_flags = ::fcntl(fd, F_GETFL);
        if (current_flags == failure)
        {
            const auto c_errno = errno;
            throw std::runtime_error{std::strerror(c_errno)};
        }
        const bool is_nonblocking = (current_flags & O_NONBLOCK) != 0;
        if (!is_nonblocking)
        {
            const long new_flags = current_flags | O_NONBLOCK;
            static constexpr int success = 0;
            if (::fcntl(fd, F_SETFL, new_flags) != success)
            {
                const auto c_errno = errno;
                throw std::runtime_error{std::strerror(c_errno)};
            }
        }
    }
    int fd_[2];
};

}//namespace Test::Util::{anonymous}

class RunInBackground::Process
{
public:
    explicit Process(Arguments& argv, Consumer consumer);
    Process(Process&&);
    Process& operator=(Process&&);
    Process() = delete;
    Process(const Process&) = delete;
    Process& operator=(const Process&) = delete;
    ~Process();
    enum class StreamStatus
    {
        ready,
        closed
    };
    struct Streams
    {
        StreamStatus stdout;
        StreamStatus stderr;
    };
    Streams watch_availability_for_reading(::fd_set& descriptors, int& max_fd) const noexcept;
    Streams read(::fd_set& descriptors) noexcept;
    bool is_running();
    void wait_for_exit();
    ProcessId id() const noexcept;
private:
    ::pid_t child_pid_;
    int stdout_fd_;
    int stderr_fd_;
    Consumer consumer_;
};

RunInBackground::RunInBackground()
    : running_processes_{}
{ }

RunInBackground::~RunInBackground()
{ }

RunInBackground::ProcessId RunInBackground::operator()(
        const char* cmd,
        const char* const* arg_begin,
        const char* const* arg_end,
        Consumer consumer)
{
    Arguments argv{arg_end - arg_begin + 2};
    argv.push_back(cmd);
    std::for_each(arg_begin, arg_end, [&](auto&& argument) { argv.push_back(argument); });
    argv.finish();
    running_processes_.emplace_back(argv, std::move(consumer));
    return running_processes_.back().id();
}

RunInBackground::ProcessId RunInBackground::operator()(
        const char* cmd,
        std::vector<std::string>::const_iterator arg_begin,
        std::vector<std::string>::const_iterator arg_end,
        Consumer consumer)
{
    Arguments argv{arg_end - arg_begin + 2};
    argv.push_back(cmd);
    std::for_each(arg_begin, arg_end, [&](auto&& argument) { argv.push_back(argument); });
    argv.finish();
    running_processes_.emplace_back(argv, std::move(consumer));
    return running_processes_.back().id();
}

RunInBackground& RunInBackground::wait()
{
    const auto does_have_any_stream_open = [&]()
    {
        ::fd_set descriptors;
        FD_ZERO(&descriptors);
        int max_fd = -1;
        std::for_each(begin(running_processes_), end(running_processes_), [&](auto&& process)
        {
            process.watch_availability_for_reading(descriptors, max_fd);
        });
        if (max_fd == -1)
        {
            return false;
        }
        static const auto failed = [](int result) { return result == -1; };
        if (failed(::select(max_fd + 1, &descriptors, nullptr, nullptr, nullptr)))
        {
            return errno != EINTR;
        }
        bool result = false;
        for (auto process_iter = begin(running_processes_); process_iter != end(running_processes_); )
        {
            const auto streams = process_iter->read(descriptors);
            const auto is_closed = (streams.stdout == Process::StreamStatus::closed) &&
                                   (streams.stderr == Process::StreamStatus::closed);
            const auto current_process_iter = process_iter++;
            if (is_closed)
            {
                if (!current_process_iter->is_running())
                {
                    running_processes_.erase(current_process_iter);
                }
            }
            else
            {
                result = true;
            }
        };
        return result;
    };
    while (does_have_any_stream_open())
    { }
    std::for_each(begin(running_processes_), end(running_processes_), [&](auto&& process)
    {
        process.wait_for_exit();
    });
    return *this;
}

RunInBackground::Process::Process(Arguments& argv, Consumer consumer)
    : child_pid_{-1},
      stdout_fd_{invalid_descriptor},
      stderr_fd_{invalid_descriptor},
      consumer_{std::move(consumer)}
{
    Pipe stdout_pipe;
    Pipe stderr_pipe;
    child_pid_ = ::fork();
    static constexpr ::pid_t failure = -1;
    if (child_pid_ == failure)
    {
        const auto c_errno = errno;
        throw std::runtime_error{std::strerror(c_errno)};
    }
    static constexpr ::pid_t im_child = 0;
    if (child_pid_ == im_child)
    {
        try
        {
            std::move(stdout_pipe).redirect_write_descriptor(STDOUT_FILENO);
            std::move(stderr_pipe).redirect_write_descriptor(STDERR_FILENO);
            static constexpr int failure = -1;
            const int retval = ::execvp(argv.data()[0], argv.data());
            if (retval == failure)
            {
                const auto c_errno = errno;
                std::cerr << "execvp failure: " << std::strerror(c_errno) << std::endl;
            }
            else
            {
                std::cerr << "execvp should never return without error" << std::endl;
            }
        }
        catch (...) { }
        ::_exit(EXIT_FAILURE);
    }
    stdout_fd_ = std::move(stdout_pipe).get_nonblocking_read_descriptor();
    stderr_fd_ = std::move(stderr_pipe).get_nonblocking_read_descriptor();
}

RunInBackground::Process::Process(Process&& src)
    : child_pid_{-1},
      stdout_fd_{invalid_descriptor},
      stderr_fd_{invalid_descriptor},
      consumer_{nullptr, nullptr, nullptr}
{
    std::swap(child_pid_, src.child_pid_);
    std::swap(stdout_fd_, src.stdout_fd_);
    std::swap(stderr_fd_, src.stderr_fd_);
    std::swap(consumer_, src.consumer_);
}

RunInBackground::Process& RunInBackground::Process::operator=(Process&& src)
{
    std::swap(child_pid_, src.child_pid_);
    std::swap(stdout_fd_, src.stdout_fd_);
    std::swap(stderr_fd_, src.stderr_fd_);
    std::swap(consumer_, src.consumer_);
    return *this;
}

RunInBackground::Process::~Process()
{
    if ((child_pid_ != -1) &&
        (child_pid_ != 0))
    {
        ::waitpid(child_pid_, nullptr, 0);
    }
    if (stdout_fd_ != invalid_descriptor)
    {
        ::close(stdout_fd_);
    }
    if (stderr_fd_ != invalid_descriptor)
    {
        ::close(stderr_fd_);
    }
}

RunInBackground::Process::Streams RunInBackground::Process::watch_availability_for_reading(::fd_set& descriptors, int& max_fd) const noexcept
{
    Streams status;
    static const auto set = [](int fd, ::fd_set& descriptors, int& max_fd)
    {
        if (fd != invalid_descriptor)
        {
            if (max_fd < fd)
            {
                max_fd = fd;
            }
            FD_SET(fd, &descriptors);
            return StreamStatus::ready;
        }
        return StreamStatus::closed;
    };
    status.stdout = set(stdout_fd_, descriptors, max_fd);
    status.stderr = set(stderr_fd_, descriptors, max_fd);
    return status;
}

RunInBackground::Process::Streams RunInBackground::Process::read(::fd_set& descriptors) noexcept
{
    Streams status;
    static const auto read_from = [](int& fd, ::fd_set& descriptors, const std::function<void(ProcessId, const char*, std::ptrdiff_t)>& consumer, ProcessId pid)
    {
        char buffer[0x40000];//256KB
        static const auto failure = [](::ssize_t bytes) { return bytes == -1; };
        static const auto to_close = [](::ssize_t bytes) { return bytes == 0; };
        static const auto would_block = [](int error)
        {
            return (error == EAGAIN) || (error == EWOULDBLOCK);
        };
        if (fd == invalid_descriptor)
        {
            return StreamStatus::closed;
        }
        if (!FD_ISSET(fd, &descriptors))
        {
            return StreamStatus::ready;
        }
        while (true)
        {
            const auto bytes = ::read(fd, buffer, sizeof(buffer));
            if (failure(bytes))
            {
                const auto c_errno = errno;
                if (c_errno == EINTR)
                {
                    continue;
                }
                if (would_block(c_errno))
                {
                    return StreamStatus::ready;
                }
                ::close(fd);
                fd = invalid_descriptor;
                return StreamStatus::closed;
            }
            if (to_close(bytes))
            {
                ::close(fd);
                fd = invalid_descriptor;
                try
                {
                    consumer(pid, nullptr, 0);
                }
                catch (...) { }
                return StreamStatus::closed;
            }
            try
            {
                consumer(pid, buffer, bytes);
            }
            catch (...) { }
            if (bytes < ::ssize_t{sizeof(buffer)})
            {
                return StreamStatus::ready;
            }
        }
    };
    status.stdout = read_from(stdout_fd_, descriptors, consumer_.from_stdout, this->id());
    status.stderr = read_from(stderr_fd_, descriptors, consumer_.from_stderr, this->id());
    return status;
}

bool RunInBackground::Process::is_running()
{
    if (child_pid_ == -1)
    {
        return false;
    }
    while (true)
    {
        int status = 0;
        const auto waitpid_result = ::waitpid(child_pid_, &status, WNOHANG);
        static const auto is_still_running = [](::pid_t waitpid_result) { return waitpid_result == 0; };
        if (is_still_running(waitpid_result))
        {
            return true;
        }
        static const auto failed = [](::pid_t waitpid_result) { return waitpid_result == -1; };
        if (failed(waitpid_result))
        {
            const auto c_errno = errno;
            if (c_errno == EINTR)
            {
                continue;
            }
            throw std::runtime_error{std::strerror(c_errno)};
        }
        try
        {
            consumer_.exit_status(this->id(), ExitStatus{status});
        }
        catch (...) { }
        child_pid_ = -1;
        return false;
    }
}

void RunInBackground::Process::wait_for_exit()
{
    int status = 0;
    static const auto failed = [](::pid_t waitpid_result) { return waitpid_result == -1; };
    if (failed(::waitpid(child_pid_, &status, 0)))
    {
        const auto c_errno = errno;
        throw std::runtime_error{std::strerror(c_errno)};
    }
    try
    {
        consumer_.exit_status(this->id(), ExitStatus{status});
    }
    catch (...) { }
    child_pid_ = -1;
}

RunInBackground::ProcessId RunInBackground::Process::id() const noexcept
{
    return ProcessId{child_pid_};
}

RunInBackground::ExitStatus::ExitStatus(int status)
    : status_{status}
{ }

bool RunInBackground::ExitStatus::exited() const noexcept
{
    return WIFEXITED(status_);
}

int RunInBackground::ExitStatus::get_exit_status() const
{
    if (this->exited())
    {
        return WEXITSTATUS(status_);
    }
    throw std::runtime_error{"process has not been exited"};
}

bool RunInBackground::ExitStatus::signaled() const noexcept
{
    return WIFSIGNALED(status_);
}

int RunInBackground::ExitStatus::get_term_sig() const
{
    if (this->signaled())
    {
        return WTERMSIG(status_);
    }
    throw std::runtime_error{"process has not been terminated by a signal"};
}

}//namespace Test::Util
}//namespace Test
