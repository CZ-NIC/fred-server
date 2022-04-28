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

#include "src/util/cfg/handle_fileman_args.hh"

#include <string>

std::shared_ptr<boost::program_options::options_description>
HandleFilemanArgs::get_options_description()
{
    std::shared_ptr<boost::program_options::options_description> opts_descs(
            new boost::program_options::options_description(
                    std::string("fileman options")));
    opts_descs->add_options()
         ("fileman", "fileman options")
         ("fileman.endpoint", boost::program_options::value<std::string>()->required(),
             "URI of Fred.Api.Fileman.File service to connect to");
    return opts_descs;
}

void HandleFilemanArgs::handle(int argc, char* argv[], FakedArgs& fa)
{
    boost::program_options::variables_map vm;
    handler_parse_args()(get_options_description(), vm, argc, argv, fa);

    fileman_args.endpoint = vm["fileman.endpoint"].as<std::string>();
}

std::shared_ptr<boost::program_options::options_description>
HandleFilemanArgsGrp::get_options_description()
{
    return HandleFilemanArgs::get_options_description();
}

std::size_t HandleFilemanArgsGrp::handle(int argc, char* argv[], FakedArgs& fa, std::size_t option_group_index)
{
    HandleFilemanArgs::handle(argc, argv, fa);
    return option_group_index;
}

const FilemanArgs& HandleFilemanArgsGrp::get_args() const
{
    return HandleFilemanArgs::fileman_args;
}

const std::string& HandleFilemanArgsGrp::get_endpoint() const
{
    return HandleFilemanArgs::fileman_args.endpoint;
}
