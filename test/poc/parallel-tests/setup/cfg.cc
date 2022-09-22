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
#include "config.h"

#include "test/poc/parallel-tests/setup/cfg.hh"
#include "test/poc/parallel-tests/setup/arguments.hh"
#include "test/poc/parallel-tests/setup/run_in_background.hh"
#include "test/poc/parallel-tests/setup/test_tree_list.hh"

#include "src/util/cfg/config_handler_decl.hh"
#include "src/util/cfg/handle_logging_args.hh"
#include "src/util/cfg/handle_tests_args.hh"

#include "util/log/add_log_device.hh"
#include "util/log/logger.hh"

#include "libfred/db_settings.hh"

#include <boost/program_options.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_parameters.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <exception>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <tuple>
#include <vector>


namespace Test {
namespace Cfg {

namespace {

class ScopeExit
{
public:
    ScopeExit(std::function<void()> on_exit)
        : on_exit_{std::move(on_exit)}
    { }
    ~ScopeExit()
    {
        try
        {
            on_exit_();
        }
        catch (...) { }
    }
private:
    std::function<void()> on_exit_;
};

namespace bpo = boost::program_options;

static constexpr unsigned terminal_width = 120;

struct Generic
{
    std::string config_file_name;
    int parallel;
    bool setup;
    class Help : public std::exception
    {
    public:
        explicit Help(std::string msg);
        const char* what() const noexcept override;
    private:
        std::string msg_;
    };
    struct Version : std::exception
    {
        const char* what() const noexcept override;
    };
};

struct Database
{
    std::string host;
    std::string port;
    std::string user;
    std::string pass;
    std::string dbname;
    std::string timeout;
};

struct AdminDatabase
{
    std::string host;
    std::string port;
    std::string user;
    std::string pass;
    std::string timeout;
};

struct Logging
{
    unsigned type;
    unsigned level;
    std::string file;
    unsigned syslog_facility;
    bool config_dump;
};

bpo::options_description make_program_options(Generic& cfg)
{
    bpo::options_description generic_options{"Generic options", terminal_width};
    const auto set_config_file_name = [&](const std::string& file_name) { cfg.config_file_name = file_name; };
    const auto set_parallel = [&](int value)
    {
        if (value < 1)
        {
            class ParallelOutOfRange : public bpo::invalid_option_value
            {
            public:
                explicit ParallelOutOfRange(int value)
                    : bpo::invalid_option_value{std::to_string(value)},
                      msg_{}
                {
                    this->set_option_name("parallel");
                    this->set_prefix(1);
                    msg_ = std::string{this->bpo::invalid_option_value::what()} + ", must be greater then zero";
                }
                const char* what() const noexcept override { return msg_.c_str(); }
            private:
                std::string msg_;
            };
            throw ParallelOutOfRange{value};
        }
        cfg.parallel = value;
    };
    const auto set_setup = [&](bool value) { cfg.setup = value; };
    generic_options.add_options()
        ("help,h", "display help message")
        ("version,v", "display server version information")
        ("parallel,p", bpo::value<int>()->default_value(1)->notifier(set_parallel), "run tests in parallel")
        ("setup,s", bpo::value<bool>()->default_value(true)->notifier(set_setup), "setup database on start")
        ("config,C",
#ifdef CONFIG_FILE
         bpo::value<std::string>()->default_value(std::string{CONFIG_FILE})->notifier(set_config_file_name),
#else
         bpo::value<std::string>()->notifier(set_config_file_name),
#endif
         "name of a file of a configuration.");
    return generic_options;
}

bpo::options_description make_program_options(Database& cfg)
{
    bpo::options_description database_options{"Fred database access options", terminal_width};
    const auto set_host = [&](const std::string& host) { cfg.host = host; };
    const auto set_port = [&](int port) { cfg.port = std::to_string(port); };
    const auto set_user = [&](const std::string& user) { cfg.user = user; };
    const auto set_password = [&](const std::string& password) { cfg.pass = password; };
    const auto set_dbname = [&](const std::string& dbname) { cfg.dbname = dbname; };
    const auto set_connect_timeout = [&](int seconds) { cfg.timeout = std::to_string(seconds); };
    database_options.add_options()
        ("database.host", bpo::value<std::string>()->notifier(set_host), "name of host to connect to")
        ("database.port", bpo::value<int>()->notifier(set_port), "port number to connect to at the server host")
        ("database.user", bpo::value<std::string>()->notifier(set_user), "PostgreSQL user name to connect as")
        ("database.name", bpo::value<std::string>()->notifier(set_dbname), "the database name")
        ("database.password", bpo::value<std::string>()->notifier(set_password), "password used for password authentication")
        ("database.timeout", bpo::value<int>()->notifier(set_connect_timeout),
         "Maximum wait for connection, in seconds. Zero or not specified means wait indefinitely. "
         "It is not recommended to use a timeout of less than 2 seconds.");
    return database_options;
}

bpo::options_description make_program_options(AdminDatabase& cfg)
{
    bpo::options_description database_options{"Fred database admin access options", terminal_width};
    const auto set_host = [&](const std::string& host) { cfg.host = host; };
    const auto set_port = [&](int port) { cfg.port = std::to_string(port); };
    const auto set_user = [&](const std::string& user) { cfg.user = user; };
    const auto set_password = [&](const std::string& password) { cfg.pass = password; };
    const auto set_connect_timeout = [&](int seconds) { cfg.timeout = std::to_string(seconds); };
    database_options.add_options()
        ("admin_database.host", bpo::value<std::string>()->notifier(set_host), "name of host to connect to")
        ("admin_database.port", bpo::value<int>()->notifier(set_port), "port number to connect to at the server host")
        ("admin_database.user", bpo::value<std::string>()->notifier(set_user), "PostgreSQL user name to connect as")
        ("admin_database.password", bpo::value<std::string>()->notifier(set_password), "password used for password authentication")
        ("admin_database.timeout", bpo::value<int>()->notifier(set_connect_timeout),
         "Maximum wait for connection, in seconds. Zero or not specified means wait indefinitely. "
         "It is not recommended to use a timeout of less than 2 seconds.");
    return database_options;
}

bpo::options_description make_program_options(Logging& cfg)
{
    bpo::options_description logging_options{"Logging configuration", terminal_width};
    const auto set_type = [&](unsigned type) { cfg.type = type; };
    const auto set_level = [&](unsigned level) { cfg.level = level; };
    const auto set_file = [&](const std::string& file) { cfg.file = file; };
    const auto set_syslog_facility = [&](unsigned syslog_facility) { cfg.syslog_facility = syslog_facility; };
    const auto set_config_dump = [&](unsigned config_dump) { cfg.config_dump = config_dump; };
    logging_options.add_options()
        ("log.type", bpo::value<unsigned>()->default_value(1)->notifier(set_type), "log type: 0-console, 1-file, 2-syslog")
        ("log.level", bpo::value<unsigned>()->default_value(8)->notifier(set_level), "log severity level")
        ("log.file", bpo::value<std::string>()->default_value("test.log")->notifier(set_file), "log file name for log.type = 1")
        ("log.syslog_facility", bpo::value<unsigned>()->default_value(1)->notifier(set_syslog_facility), "syslog facility for log.type = 2")
        ("log.config_dump", bpo::value<bool>()->default_value(false)->notifier(set_config_dump), "log loaded configuration data with debug severity");
    return logging_options;
}

Generic::Help::Help(std::string msg)
    : msg_{std::move(msg)}
{ }

const char* Generic::Help::what() const noexcept
{
    return msg_.c_str();
}

const char* Generic::Version::what() const noexcept
{
#ifdef VERSION
    return VERSION;
#else
    return "unknown version";
#endif
}

template <typename ...Ts>
auto make_program_options(std::tuple<Ts...>& cfg)
{
    return std::array<bpo::options_description, sizeof...(Ts)>{make_program_options(std::get<Ts>(cfg))...};
}

template <std::size_t n>
bpo::options_description& add_program_options(bpo::options_description& dst, const bpo::options_description* src)
{
    dst.add(*src);
    return add_program_options<n - 1>(dst, src + 1);
}

template <>
bpo::options_description& add_program_options<0>(bpo::options_description& dst, const bpo::options_description*)
{
    return dst;
}

template <std::size_t n>
decltype(auto) add_program_options(
        bpo::options_description& dst,
        const std::array<bpo::options_description, n>& src)
{
    return add_program_options<n>(dst, src.data());
}

std::string* admin_connection_string = nullptr;

using ConfigFileOptions = std::tuple<Database, AdminDatabase, Logging>;

Util::Arguments& push_back(Util::Arguments& args, const Database& cfg);
Util::Arguments& push_back(Util::Arguments& args, const Logging& cfg);

auto parse_arguments(int argc, const char* const* argv, std::function<void(const std::string&)> on_unrecognized)
{
    std::tuple<Generic, ConfigFileOptions> cfg;

    const auto generic_options = make_program_options(std::get<Generic>(cfg));
    const auto config_file_options = make_program_options(std::get<ConfigFileOptions>(cfg));
    bpo::options_description command_line_options_group{argv[0] + std::string{" options"}, terminal_width};
    add_program_options(command_line_options_group.add(generic_options), config_file_options);
    bpo::options_description config_file_options_group;
    add_program_options(config_file_options_group, config_file_options);
    bpo::variables_map variables_map;
    const auto unregistered = [&]()
    {
        try
        {
            auto parsed = bpo::command_line_parser{argc, argv}
                    .options(command_line_options_group)
                    .allow_unregistered()
                    .run();
            bpo::store(parsed, variables_map);
            bpo::notify(variables_map);
            return bpo::collect_unrecognized(parsed.options, bpo::include_positional);
        }
        catch (const bpo::unknown_option& unknown_option)
        {
            std::ostringstream out;
            out << unknown_option.what() << "\n\n" << command_line_options_group;
            throw std::runtime_error{out.str()};
        }
    }();
    std::for_each(begin(unregistered), end(unregistered), [&](auto&& item) { on_unrecognized(item); });
    const auto has_option = [&](const char* option) { return 0 < variables_map.count(option); };
    if (has_option("help"))
    {
        std::ostringstream out;
        out << command_line_options_group;
        throw Generic::Help{std::move(out).str()};
    }
    if (has_option("version"))
    {
        throw Generic::Version{};
    }
    const bool config_file_name_presents = !std::get<Generic>(cfg).config_file_name.empty();
    if (config_file_name_presents)
    {
        std::ifstream config_file{std::get<Generic>(cfg).config_file_name};
        if (!config_file)
        {
            throw std::runtime_error{"can not open config file \"" + std::get<Generic>(cfg).config_file_name + "\""};
        }
        try
        {
            auto parsed = bpo::parse_config_file(
                    config_file,
                    config_file_options_group,
                    true);//allow unregistered
            bpo::store(parsed, variables_map);
            bpo::notify(variables_map);
            const auto unregistered = bpo::collect_unrecognized(parsed.options, bpo::include_positional);
            std::for_each(begin(unregistered), end(unregistered), [&](auto&& item) { on_unrecognized(item); });
        }
        catch (const bpo::unknown_option& unknown_option)
        {
            std::ostringstream out;
            out << unknown_option.what() << "\n\n" << command_line_options_group;
            throw std::runtime_error{out.str()};
        }
        bpo::notify(variables_map);
    }
    return cfg;
}

const std::string& set_admin_connection_string(const AdminDatabase& cfg)
{
    static const auto append = [](std::string& out, const char* name, const std::string& value)
    {
        if (!value.empty())
        {
            if (!out.empty())
            {
                out += " ";
            }
            out += name + value;
        }
    };
    if (admin_connection_string == nullptr)
    {
        admin_connection_string = new std::string{};
        append(*admin_connection_string, "host=", cfg.host);
        append(*admin_connection_string, "port=", cfg.port);
        append(*admin_connection_string, "user=", cfg.user);
        append(*admin_connection_string, "password=", cfg.pass);
        append(*admin_connection_string, "connect_timeout=", cfg.timeout);
    }
    return *admin_connection_string;
}

std::string get_admin_connection_string(const std::string& database_name)
{
    if (!admin_connection_string->empty())
    {
        return (*admin_connection_string) + " dbname=" + database_name;
    }
    return "dbname=" + database_name;
}

std::unique_ptr<::Database::StandaloneConnection> get_admin_connection(const std::string& database_name)
{
    return ::Database::StandaloneManager{get_admin_connection_string(database_name)}.acquire();
}

Util::Arguments& push_back(Util::Arguments& args, const Database& cfg)
{
    return args.push_back_non_empty("--database.name=", cfg.dbname)
               .push_back_non_empty("--database.host=", cfg.host)
               .push_back_non_empty("--database.password=", cfg.pass)
               .push_back_non_empty("--database.port=", cfg.port)
               .push_back_non_empty("--database.timeout=", cfg.timeout)
               .push_back_non_empty("--database.user=", cfg.user);
}

Util::Arguments& push_back(Util::Arguments& args, const Logging& cfg)
{
    args.push_back("--log.type=" + std::to_string(cfg.type))
        .push_back("--log.level=" + std::to_string(cfg.level));
    switch (cfg.type)
    {
        case 1:
            return args.push_back("--log.file=" + cfg.file);
        case 2:
            return args.push_back("--log.syslog_facility=" + std::to_string(cfg.syslog_facility));
    }
    return args;
}

void setup_logging(const Logging& cfg)
{
    ::Logging::Log::Severity min_severity = ::Logging::Log::Severity::trace;
    switch (cfg.level)
    {
        case 0:
            min_severity = ::Logging::Log::Severity::emerg;
            break;
        case 1:
            min_severity = ::Logging::Log::Severity::alert;
            break;
        case 2:
            min_severity = ::Logging::Log::Severity::crit;
            break;
        case 3:
            min_severity = ::Logging::Log::Severity::err;
            break;
        case 4:
            min_severity = ::Logging::Log::Severity::warning;
            break;
        case 5:
            min_severity = ::Logging::Log::Severity::notice;
            break;
        case 6:
            min_severity = ::Logging::Log::Severity::info;
            break;
        case 7:
            min_severity = ::Logging::Log::Severity::debug;
            break;
        case 8:
            min_severity = ::Logging::Log::Severity::trace;
            break;
    }

    switch (cfg.type)
    {
        case 0:
            ::Logging::add_console_device(::LOGGER, min_severity);
            break;
        case 1:
            ::Logging::add_file_device(::LOGGER, cfg.file, min_severity);
            break;
        case 2:
            ::Logging::add_syslog_device(::LOGGER, cfg.syslog_facility, min_severity);
            break;
    }
}

void copy_database(::Database::StandaloneConnection& conn, const std::string& source_db, const std::string& target_db)
{
    conn.exec("CREATE DATABASE \"" + target_db + "\" WITH TEMPLATE \"" + source_db + "\"");
}

void drop_database_if_exists(::Database::StandaloneConnection& conn, const std::string& db)
{
    conn.exec("DROP DATABASE IF EXISTS \"" + db + "\"");
}

std::vector<std::string>& push_back(std::vector<std::string>& args, std::string argv)
{
    args.push_back(std::move(argv));
    return args;
}

std::vector<std::string>& push_back_non_empty(std::vector<std::string>& args, const char* option_name, std::string argv)
{
    if (!argv.empty())
    {
        push_back(args, option_name).push_back(std::move(argv));
    }
    return args;
}

struct ProcessResult
{
    std::string stdout;
    std::string stderr;
    Util::RunInBackground::ExitStatus exit_status;
};

struct PidLess
{
    bool operator()(Util::RunInBackground::ProcessId lhs, Util::RunInBackground::ProcessId rhs) const noexcept { return lhs.value < rhs.value; }
};

using ProcessesData = std::map<Util::RunInBackground::ProcessId, ProcessResult, PidLess>;

void show_process_result(const ProcessesData::value_type& result)
{
    if (result.second.exit_status.exited())
    {
        if (result.second.exit_status.get_exit_status() == EXIT_SUCCESS)
        {
            std::cout << "process " << result.first.value << " exited successfully" << std::endl;
        }
        else
        {
            std::cout << "process " << result.first.value << " failed with status " << result.second.exit_status.get_exit_status() << std::endl;
        }
    }
    else if (result.second.exit_status.signaled())
    {
        std::cout << "process " << result.first.value << " terminated by signal " << result.second.exit_status.get_term_sig() << std::endl;
    }
    else
    {
        std::cout << "process " << result.first.value << " finished under strange circumstances" << std::endl;
    }
    if (!result.second.stdout.empty())
    {
        std::cout << result.second.stdout << std::flush;
    }
    if (!result.second.stderr.empty())
    {
        std::cerr << result.second.stderr;
    }
}

std::string to_string(boost::unit_test::log_level level)
{
    switch (level)
    {
        case boost::unit_test::log_level::log_successful_tests:
            return "success";
        case boost::unit_test::log_level::log_test_units:
            return "unit_scope";
        case boost::unit_test::log_level::log_messages:
            return "message";
        case boost::unit_test::log_level::log_warnings:
            return "warning";
        case boost::unit_test::log_level::log_all_errors:
            return "error";
        case boost::unit_test::log_level::log_cpp_exception_errors:
            return "cpp_exception";
        case boost::unit_test::log_level::log_system_errors:
            return "system_error";
        case boost::unit_test::log_level::log_fatal_errors:
            return "fatal_error";
        case boost::unit_test::log_level::log_nothing:
            return "nothing";
        case boost::unit_test::log_level::invalid_log_level:
            break;
    }
    return "";
}

std::string get_log_level()
{
    if (boost::unit_test::runtime_config::has(boost::unit_test::runtime_config::btrt_log_level))
    {
        return to_string(boost::unit_test::runtime_config::get<boost::unit_test::log_level>(boost::unit_test::runtime_config::btrt_log_level));
    }
    return "";
}

void clone_database(
        std::string src_database,
        std::vector<std::string>::const_iterator dst_database_begin,
        std::vector<std::string>::const_iterator dst_database_end)
{
    auto conn = std::unique_ptr<::Database::StandaloneConnection>{nullptr};
    auto threads = std::list<std::thread>{};
    while (true)
    {
        const auto number_of_databases = dst_database_end - dst_database_begin;
        if (number_of_databases == 0)
        {
            break;
        }
        if (!conn)
        {
            conn = get_admin_connection(src_database);
        }
        copy_database(*conn, src_database, *dst_database_begin);
        if (number_of_databases == 1)
        {
            break;
        }
        auto dst_database_middle = dst_database_begin + 1 + ((number_of_databases - 1) / 2);
        threads.emplace_back(clone_database, *dst_database_begin, dst_database_begin + 1, dst_database_middle);
        dst_database_begin = dst_database_middle;
    }
    std::for_each(begin(threads), end(threads), [](auto&& thread) { thread.join(); });
}

void run_parallel(
        const Generic& generic_cfg,
        const ConfigFileOptions& main_cfg,
        const std::string& test_database_name,
        const Util::TestTreeList& tests)
{
    const auto processes = static_cast<int>(tests.size()) < generic_cfg.parallel
            ? static_cast<int>(tests.size())
            : generic_cfg.parallel;
    std::cout << "run parallel in " << processes << " processes" << std::endl;
    const auto master_source_db = std::get<Database>(main_cfg).dbname;
    const auto make_process_db_name = [&](int process_id)
    {
        return master_source_db + "-" + std::to_string(process_id);
    };
    const auto make_process_log_file = [&](int process_id)
    {
        if (std::get<Logging>(main_cfg).file.empty())
        {
            return std::string{};
        }
        return std::get<Logging>(main_cfg).file + "." + std::to_string(process_id);
    };
    const auto create_databases = [&]()
    {
        auto databases = std::vector<std::string>{};
        databases.reserve(processes);
        for (int process_id = 0; process_id < processes; ++process_id)
        {
            databases.push_back(make_process_db_name(process_id));
        }
        try
        {
            clone_database(test_database_name, begin(databases), end(databases));
        }
        catch (const std::exception& e)
        {
            std::cerr << "clone_database failed: " << e.what() << std::endl;
        }
        catch (...)
        {
            std::cerr << "clone_database failed" << std::endl;
        }
    };
    const auto drop_databases = [&]()
    {
        const auto drop_database = [&](int process_id)
        {
            const auto admin_db_conn = get_admin_connection(test_database_name);
            const auto process_db_name = make_process_db_name(process_id);
            try
            {
                drop_database_if_exists(*admin_db_conn, process_db_name);
            }
            catch (const std::exception& e)
            {
                std::cerr << "DROP DATABASE failed: " << e.what() << std::endl;
            }
            catch (...)
            {
                std::cerr << "DROP DATABASE failed" << std::endl;
            }
        };
        auto threads = std::list<std::thread>{};
        for (int process_id = 1; process_id < processes; ++process_id)
        {
            threads.emplace_back(drop_database, process_id);
        }
        if (0 < processes)
        {
            drop_database(0);
        }
        std::for_each(begin(threads), end(threads), [](auto&& thread) { thread.join(); });
    };
    create_databases();
    ScopeExit do_on_exit{drop_databases};
    Util::RunInBackground running_tests;
    ProcessesData tests_results;
    const auto on_stdout = [&](Util::RunInBackground::ProcessId pid, const char* data, std::ptrdiff_t bytes)
    {
        if (data == nullptr)
        {
            return;
        }
        const auto result_iter = tests_results.find(pid);
        if (result_iter != end(tests_results))
        {
            result_iter->second.stdout.append(data, bytes);
        }
    };
    const auto on_stderr = [&](Util::RunInBackground::ProcessId pid, const char* data, std::ptrdiff_t bytes)
    {
        if (data == nullptr)
        {
            return;
        }
        const auto result_iter = tests_results.find(pid);
        if (result_iter != end(tests_results))
        {
            result_iter->second.stderr.append(data, bytes);
        }
    };
    const auto on_exit = [&](Util::RunInBackground::ProcessId pid, Util::RunInBackground::ExitStatus exit_status)
    {
        const auto result_iter = tests_results.find(pid);
        if (result_iter != end(tests_results))
        {
            result_iter->second.exit_status = exit_status;
        }
    };
    {
        const auto cmd = boost::unit_test::framework::master_test_suite().argv[0];
        Util::RunInBackground::Consumer event_consumer{on_stdout, on_stderr, on_exit};
        ProcessResult process_data{std::string{}, std::string{}, Util::RunInBackground::ExitStatus{-1}};
        for (int process_id = 0; process_id < processes; ++process_id)
        {
            std::vector<std::string> args{};
            const auto get_test_idx = [&](int process_id) { return (tests.size() * process_id) / processes; };
            for (auto idx = get_test_idx(process_id); idx < get_test_idx(process_id + 1); ++idx)
            {
                push_back_non_empty(args, "-t", tests[idx]);
            }
            push_back_non_empty(args, "-l", get_log_level());
            push_back(args, "--");
            push_back_non_empty(args, "-C", generic_cfg.config_file_name);
            push_back_non_empty(args, "-s", "no");
            push_back_non_empty(args, "--database.name", make_process_db_name(process_id));
            push_back_non_empty(args, "--log.file", make_process_log_file(process_id));
            const auto pid = running_tests(cmd, begin(args), end(args), event_consumer);
            tests_results.insert(std::make_pair(pid, process_data));
        }
    }
    running_tests.wait();
    std::for_each(begin(tests_results), end(tests_results), show_process_result);
}

}//namespace Test::Cfg::{anonymous}

DatabaseAdministrator::DatabaseAdministrator(std::string test_database_name, bool setup, std::function<void()> database_setup_procedure)
    : origin_{},
      test_{},
      test_database_name_{std::move(test_database_name)}
{
    if (setup)
    {
        origin_.backup(test_database_name_, test_database_name_ + "-orig");
        ::LOGGER.trace("setup fred database start");
        database_setup_procedure();
        ::LOGGER.trace("setup fred database done");
    }
}

DatabaseAdministrator::~DatabaseAdministrator()
{
}

std::function<void()> DatabaseAdministrator::get_restore_test_database_procedure()
{
    test_.backup(test_database_name_, test_database_name_ + "-bak");
    return [&]() { test_.recover(); };
}

DatabaseAdministrator::BackupDatabase::~BackupDatabase()
{
    if (this->has_backup())
    {
        try
        {
            this->reset();
        }
        catch (const std::exception& e)
        {
            ::LOGGER.error(std::string{"reset failure: "} + e.what());
        }
        catch (...) { }
    }
}

void DatabaseAdministrator::BackupDatabase::backup(std::string origin, std::string backup)
{
    origin_ = std::move(origin);
    backup_ = std::move(backup);
    ::LOGGER.trace("backup " + origin_ + " into " + backup_);
    const auto admin_db_conn = get_admin_connection(origin_);
    drop_database_if_exists(*admin_db_conn, backup_);
    copy_database(*admin_db_conn, origin_, backup_);
}

bool DatabaseAdministrator::BackupDatabase::has_backup() const noexcept
{
    return !origin_.empty() && !backup_.empty();
}

void DatabaseAdministrator::BackupDatabase::recover()
{
    ::LOGGER.trace("recover " + backup_ + " into " + origin_);
    auto admin_db_conn = get_admin_connection(backup_);
    drop_database_if_exists(*admin_db_conn, origin_);
    copy_database(*admin_db_conn, backup_, origin_);
}

void DatabaseAdministrator::BackupDatabase::reset()
{
    auto admin_db_conn = get_admin_connection(backup_);
    ::LOGGER.trace("reset " + backup_ + " into " + origin_);
    drop_database_if_exists(*admin_db_conn, origin_);
    copy_database(*admin_db_conn, backup_, origin_);
    admin_db_conn = get_admin_connection(origin_);
    drop_database_if_exists(*admin_db_conn, backup_);
    origin_.clear();
    backup_.clear();
}

DatabaseAdministrator handle_command_line_args(const HandlerPtrVector& config_handlers, std::function<void()> database_setup_procedure)
{
    try
    {
        auto unrecognized = std::vector<std::string>{};
        auto option_name = std::string{};
        const auto cfg = parse_arguments(
                boost::unit_test::framework::master_test_suite().argc,
                boost::unit_test::framework::master_test_suite().argv,
                [&](const std::string& item)
                {
                    if (option_name.empty())
                    {
                        option_name = item;
                    }
                    else
                    {
                        unrecognized.push_back("--" + option_name + "=" + item);
                        option_name.clear();
                    }
                });
        setup_logging(std::get<Logging>(std::get<ConfigFileOptions>(cfg)));
        {
            Util::Arguments args{boost::unit_test::framework::master_test_suite().argc + 1};
            args.push_back(boost::unit_test::framework::master_test_suite().argv[0]);
            push_back(args, std::get<Database>(std::get<ConfigFileOptions>(cfg)));
            push_back(args, std::get<Logging>(std::get<ConfigFileOptions>(cfg)));
            std::for_each(begin(unrecognized), end(unrecognized), [&](auto&& item) { args.push_back(item); });
            args.finish();
            CfgArgs::init<HandleTestsArgs>(config_handlers)->handle(args.count(), args.data());
        }
        {
            set_admin_connection_string(std::get<AdminDatabase>(std::get<ConfigFileOptions>(cfg)));
            const auto& test_database_name = std::get<Database>(std::get<ConfigFileOptions>(cfg)).dbname;
            DatabaseAdministrator db_administrator{test_database_name, std::get<Generic>(cfg).setup, std::move(database_setup_procedure)};
            if (std::get<Generic>(cfg).parallel <= 1)
            {
                return db_administrator;
            }
            const auto tests = Util::get_test_tree();
            if (tests.size() <= 1)
            {
                return db_administrator;
            }
            run_parallel(std::get<Generic>(cfg), std::get<ConfigFileOptions>(cfg), test_database_name, tests);
        }
        ::_exit(EXIT_SUCCESS);
    }
    catch (const Generic::Help& h)
    {
        std::cout << h.what() << std::endl;
        ::_exit(EXIT_SUCCESS);
    }
    catch (const Generic::Version& v)
    {
        std::cout << v.what() << std::endl;
        ::_exit(EXIT_SUCCESS);
    }
    catch (const std::exception& e)
    {
        std::cerr << "handle_command_line_args failure: " << e.what() << std::endl;
        ::_exit(EXIT_FAILURE);
    }
}

}//namespace Test::Cfg
}//namespace Test
