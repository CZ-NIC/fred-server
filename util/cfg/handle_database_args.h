/*  
 * Copyright (C) 2010  CZ.NIC, z.s.p.o.
 * 
 * This file is part of FRED.
 * 
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 * 
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 *  @handle_database_args.h
 *  database dependent args handling
 */

#ifndef HANDLE_DATABASE_ARGS_H_
#define HANDLE_DATABASE_ARGS_H_

#include <iostream>
#include <exception>
#include <string>
#include <vector>

#include <boost/program_options.hpp>
#include <boost/format.hpp>

#include "faked_args.h"
#include "handle_args.h"
#include "fredlib/db_settings.h"

/**
 * \class HandleDatabaseArgs
 * \brief database options handler
 */
class HandleDatabaseArgs : public HandleArgs
{
protected:
    std::string conn_info;
public:

    boost::shared_ptr<boost::program_options::options_description>
        get_options_description()
    {
        boost::shared_ptr<boost::program_options::options_description> db_opts(
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
    }//get_options_description

    void handle( int argc, char* argv[],  FakedArgs &fa)
    {
        boost::program_options::variables_map vm;
        handler_parse_args(get_options_description(), vm, argc, argv, fa);

        /* construct connection string */
        std::string dbhost = (vm.count("database.host") == 0 ? ""
                : "host=" + vm["database.host"].as<std::string>() + " ");
        std::string dbpass = (vm.count("database.password") == 0 ? ""
                : "password=" + vm["database.password"].as<std::string>() + " ");
        std::string dbname = (vm.count("database.name") == 0 ? ""
                : "dbname=" + vm["database.name"].as<std::string>() + " ");
        std::string dbuser = (vm.count("database.user") == 0 ? ""
                : "user=" + vm["database.user"].as<std::string>() + " ");
        std::string dbport = (vm.count("database.port") == 0 ? ""
                : "port=" + boost::lexical_cast<std::string>(vm["database.port"].as<unsigned>()) + " ");
        std::string dbtime = (vm.count("database.timeout") == 0 ? ""
                : "connect_timeout=" + boost::lexical_cast<std::string>(vm["database.timeout"].as<unsigned>()) + " ");
        conn_info = str(boost::format("%1% %2% %3% %4% %5% %6%")
                                                  % dbhost
                                                  % dbport
                                                  % dbname
                                                  % dbuser
                                                  % dbpass
                                                  % dbtime);
        // std::cout << "database connection set to: " << conn_info << std::endl;

        Database::Manager::init(new Database::ConnectionFactory(conn_info));

    }//handle

    std::string get_conn_info() {
        if(conn_info.empty()) {
            throw std::runtime_error("Wrong usage: Connection info not initialized yet");
        }
        return conn_info;
    }
};//class HandleDatabaseArgs

/**
 * \class HandleLoggingArgsGrp
 * \brief database options handler with option groups
 */

class HandleDatabaseArgsGrp : public HandleGrpArgs
                            , private HandleDatabaseArgs
{
public:
    boost::shared_ptr<boost::program_options::options_description>
        get_options_description()
    {
        return HandleDatabaseArgs::get_options_description();
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        HandleDatabaseArgs::handle(argc, argv, fa);
        return option_group_index;
    }//handle

    const std::string& get_conn_info(){return HandleDatabaseArgs::conn_info;}
};//class HandleDatabaseArgsGrp


#endif //HANDLE_DATABASE_ARGS_H_
