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

#ifndef HANDLE_DATABASE_ARGS_HH_932E1219F77B4A3F9E7B1DB0E25CB8E1
#define HANDLE_DATABASE_ARGS_HH_932E1219F77B4A3F9E7B1DB0E25CB8E1

#include "src/util/cfg/faked_args.hh"
#include "src/util/cfg/handle_args.hh"

#include <boost/program_options.hpp>

#include <memory>
#include <string>

/**
 * \class HandleDatabaseArgs
 * \brief database options handler
 */
class HandleDatabaseArgs : public HandleArgs
{
public:
    std::shared_ptr<boost::program_options::options_description>
    get_options_description();
    void handle(int argc, char* argv[], FakedArgs& fa);
    std::string get_conn_info()const;
protected:
    std::string conn_info;
};

/**
 * \class HandleLoggingArgsGrp
 * \brief database options handler with option groups
 */
class HandleDatabaseArgsGrp : public HandleGrpArgs,
                              private HandleDatabaseArgs
{
public:
    std::shared_ptr<boost::program_options::options_description>
    get_options_description();
    std::size_t handle(int argc, char* argv[], FakedArgs& fa, std::size_t option_group_index);
    const std::string& get_conn_info()const;
};

#endif//HANDLE_DATABASE_ARGS_HH_932E1219F77B4A3F9E7B1DB0E25CB8E1
