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
#ifndef RUN_IN_BACKGROUND_HH_660543BA33DDF3CECF1A6FCFE9892702//date "+%s.%N"|md5sum|tr "[a-f]" "[A-F]"
#define RUN_IN_BACKGROUND_HH_660543BA33DDF3CECF1A6FCFE9892702

#include <cstddef>
#include <functional>
#include <list>
#include <string>
#include <vector>

namespace Test {
namespace Util {

class RunInBackground
{
public:
    RunInBackground();
    ~RunInBackground();
    RunInBackground(const RunInBackground&) = delete;
    RunInBackground(RunInBackground&) = default;
    RunInBackground& operator=(const RunInBackground&) = delete;
    RunInBackground& operator=(RunInBackground&) = default;
    struct ProcessId
    {
        int value;
    };
    class ExitStatus
    {
    public:
        explicit ExitStatus(int status);
        bool exited() const noexcept;
        int get_exit_status() const;
        bool signaled() const noexcept;
        int get_term_sig() const;
    private:
        int status_;
    };
    struct Consumer
    {
        std::function<void(ProcessId, const char*, std::ptrdiff_t)> from_stdout;
        std::function<void(ProcessId, const char*, std::ptrdiff_t)> from_stderr;
        std::function<void(ProcessId, ExitStatus)> exit_status;
    };
    ProcessId operator()(
            const char* cmd,
            const char* const* arg_begin,
            const char* const* arg_end,
            Consumer consumer);
    ProcessId operator()(
            const char* cmd,
            std::vector<std::string>::const_iterator arg_begin,
            std::vector<std::string>::const_iterator arg_end,
            Consumer consumer);
    RunInBackground& wait();
private:
    class Process;
    std::list<Process> running_processes_;
};

}//namespace Test::Util
}//namespace Test

#endif//RUN_IN_BACKGROUND_HH_660543BA33DDF3CECF1A6FCFE9892702
