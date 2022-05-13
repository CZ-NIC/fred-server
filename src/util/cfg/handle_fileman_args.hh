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

#ifndef HANDLE_FILEMAN_ARGS_HH_24731CBEF7D14C1EB861C016D8D388D5
#define HANDLE_FILEMAN_ARGS_HH_24731CBEF7D14C1EB861C016D8D388D5

#include "src/bin/cli/fileman_params.hh"
#include "src/util/cfg/faked_args.hh"
#include "src/util/cfg/handle_args.hh"

#include <boost/program_options.hpp>

#include <cstddef>
#include <string>

struct HandleFilemanArgs : public HandleArgs
{
    FilemanArgs fileman_args;

    std::shared_ptr<boost::program_options::options_description>
    get_options_description() override;
    void handle(int argc, char* argv[], FakedArgs& fa) override;
};

struct HandleFilemanArgsGrp : public HandleGrpArgs,
                              private HandleFilemanArgs
{
public:
    std::shared_ptr<boost::program_options::options_description>
    get_options_description() override;
    std::size_t handle(int argc, char* argv[], FakedArgs& fa, std::size_t option_group_index) override;
    const FilemanArgs& get_args() const;
    const std::string& get_endpoint() const;
};

#endif
