/*
 * Copyright (C) 2019  CZ.NIC, z. s. p. o.
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
#include "src/util/cfg/handle_database_args.hh"
#include "libfred/db_settings.hh"
#include "src/deprecated/libfred/db_settings.hh"

#include <exception>
#include <string>

std::shared_ptr<boost::program_options::options_description>
HandleDatabaseArgs::get_options_description()
{
    std::shared_ptr<boost::program_options::options_description> db_opts(
            new boost::program_options::options_description(
                    std::string("Database connection configuration")));
    db_opts->add_options()
            ("database.name", boost::program_options
                        ::value<std::string>()->default_value(std::string("fred"))
                    , "database name")
            ("database.user", boost::program_options
                        ::value<std::string>()->default_value(std::string("fred"))
                    , "database user name")
            ("database.password", boost::program_options
                        ::value<std::string>(), "database password")
            ("database.host", boost::program_options
                        ::value<std::string>()->default_value(std::string("localhost"))
                    , "database hostname")
            ("database.port", boost::program_options
                        ::value<unsigned int>(), "database port number")
            ("database.timeout", boost::program_options
                        ::value<unsigned int>(), "database timeout");
    return db_opts;
}

void HandleDatabaseArgs::handle(int argc, char* argv[], FakedArgs& fa)
{
    boost::program_options::variables_map vm;
    handler_parse_args()(this->get_options_description(), vm, argc, argv, fa);

    /* construct connection string */
    conn_info += "host=";
    conn_info += vm["database.host"].as<std::string>();
    conn_info += " ";

    if (vm.count("database.port") == 1)
    {
        conn_info += "port=";
        conn_info += boost::lexical_cast<std::string>(vm["database.port"].as<unsigned>());
        conn_info += " ";
    }

    conn_info += "dbname=";
    conn_info += vm["database.name"].as<std::string>();
    conn_info += " ";

    conn_info += "user=";
    conn_info += vm["database.user"].as<std::string>();
    conn_info += " ";

    if (vm.count("database.password") == 1)
    {
        conn_info += "password=";
        conn_info += vm["database.password"].as<std::string>();
        conn_info += " ";
    }

    if (vm.count("database.timeout") == 1)
    {
        conn_info += "connect_timeout=";
        conn_info += boost::lexical_cast<std::string>(vm["database.timeout"].as<unsigned>());
        conn_info += " ";
    }

    Database::emplace_default_manager<Database::StandaloneManager>(conn_info);
    Database::Manager::init(new Database::ConnectionFactory(conn_info));
}

std::string HandleDatabaseArgs::get_conn_info()const
{
    if (conn_info.empty())
    {
        throw std::runtime_error("Wrong usage: Connection info not initialized yet");
    }
    return conn_info;
}

std::shared_ptr<boost::program_options::options_description>
HandleDatabaseArgsGrp::get_options_description()
{
    return HandleDatabaseArgs::get_options_description();
}

std::size_t HandleDatabaseArgsGrp::handle(int argc, char* argv[], FakedArgs& fa, std::size_t option_group_index)
{
    HandleDatabaseArgs::handle(argc, argv, fa);
    return option_group_index;
}

const std::string& HandleDatabaseArgsGrp::get_conn_info()const
{
    return HandleDatabaseArgs::conn_info;
}
