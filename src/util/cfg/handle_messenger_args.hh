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

#ifndef HANDLE_MESSENGER_ARGS_HH_A11E1982C90146E1A8EA1AF2C60BF489
#define HANDLE_MESSENGER_ARGS_HH_A11E1982C90146E1A8EA1AF2C60BF489

#include "src/bin/cli/messenger_params.hh"
#include "src/util/cfg/faked_args.hh"
#include "src/util/cfg/handle_args.hh"

#include <boost/program_options.hpp>

#include <cstddef>
#include <string>

struct HandleMessengerArgs : public HandleArgs
{
    MessengerArgs messenger_args;

    std::shared_ptr<boost::program_options::options_description>
    get_options_description() override;
    void handle(int argc, char* argv[], FakedArgs& fa) override;
};

struct HandleMessengerArgsGrp : public HandleGrpArgs,
                                private HandleMessengerArgs
{
public:
    std::shared_ptr<boost::program_options::options_description>
    get_options_description() override;
    std::size_t handle(int argc, char* argv[], FakedArgs& fa, std::size_t option_group_index) override;
    const MessengerArgs& get_args() const;
    const std::string& get_endpoint() const;
    bool get_archive() const;
    bool get_archive_rendered() const;
};

#endif
