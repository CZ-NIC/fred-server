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

#include "src/util/cfg/handle_secretary_args.hh"

#include <string>

std::shared_ptr<boost::program_options::options_description>
HandleSecretaryArgs::get_options_description()
{
    std::shared_ptr<boost::program_options::options_description> opts_descs(
            new boost::program_options::options_description(
                    std::string("secretary options")));
    opts_descs->add_options()
         ("secretary", "secretary options")
         ("secretary.endpoint", boost::program_options::value<std::string>()->required(),
             "URI of Secretary service to connect to");
    return opts_descs;
}

void HandleSecretaryArgs::handle(int argc, char* argv[], FakedArgs& fa)
{
    boost::program_options::variables_map vm;
    handler_parse_args()(get_options_description(), vm, argc, argv, fa);

    secretary_args.endpoint = vm["secretary.endpoint"].as<std::string>();
}

std::shared_ptr<boost::program_options::options_description>
HandleSecretaryArgsGrp::get_options_description()
{
    return HandleSecretaryArgs::get_options_description();
}

std::size_t HandleSecretaryArgsGrp::handle(int argc, char* argv[], FakedArgs& fa, std::size_t option_group_index)
{
    HandleSecretaryArgs::handle(argc, argv, fa);
    return option_group_index;
}

const SecretaryArgs& HandleSecretaryArgsGrp::get_args() const
{
    return HandleSecretaryArgs::secretary_args;
}

const std::string& HandleSecretaryArgsGrp::get_endpoint() const
{
    return HandleSecretaryArgs::secretary_args.endpoint;
}
