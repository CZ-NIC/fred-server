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
#ifndef CFG_HH_282E3ECE2F882A63460B236E84100B52//date "+%s.%N"|md5sum|tr "[a-f]" "[A-F]"
#define CFG_HH_282E3ECE2F882A63460B236E84100B52

#include "src/util/cfg/handle_args.hh"

#include <functional>
#include <string>

namespace Test {
namespace Cfg {

class DatabaseAdministrator
{
public:
    explicit DatabaseAdministrator(std::string test_database_name, bool setup, std::function<void()> database_setup_procedure);
    DatabaseAdministrator(DatabaseAdministrator&&) = default;
    DatabaseAdministrator(const DatabaseAdministrator&) = delete;
    ~DatabaseAdministrator();
    DatabaseAdministrator& operator=(DatabaseAdministrator&&) = default;
    DatabaseAdministrator& operator=(const DatabaseAdministrator&) = delete;
    std::function<void()> get_restore_test_database_procedure();
private:
    class BackupDatabase
    {
    public:
        BackupDatabase() = default;
        BackupDatabase(BackupDatabase&&) = default;
        BackupDatabase(const BackupDatabase&) = delete;
        ~BackupDatabase();
        BackupDatabase& operator=(BackupDatabase&&) = default;
        BackupDatabase& operator=(const BackupDatabase&) = delete;
        void backup(std::string origin, std::string backup);
        bool has_backup() const noexcept;
        void recover();
        void reset();
    private:
        std::string origin_;
        std::string backup_;
    };
    BackupDatabase origin_;
    BackupDatabase test_;
    std::string test_database_name_;
};

DatabaseAdministrator handle_command_line_args(const HandlerPtrVector& config_handlers, std::function<void()> database_setup_procedure = []() { });

}//namespace Test::Cfg
}//namespace Test

#endif//CFG_HH_282E3ECE2F882A63460B236E84100B52
