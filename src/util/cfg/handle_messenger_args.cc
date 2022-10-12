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

#include "src/util/cfg/handle_messenger_args.hh"

#include <string>

std::shared_ptr<boost::program_options::options_description>
HandleMessengerArgs::get_options_description()
{
    std::shared_ptr<boost::program_options::options_description> opts_descs(
            new boost::program_options::options_description(
                    std::string("messenger options")));
    opts_descs->add_options()
         ("messenger", "messenger options")
         ("messenger.endpoint", boost::program_options::value<std::string>()->required(),
             "URI of Fred.Api.Messenger.Email service to connect to")
         ("messenger.archive", boost::program_options::value<bool>()->required(),
             "archive the message")
         ("messenger.archive_rendered", boost::program_options::value<bool>()->required(),
             "archive the rendered message");
    return opts_descs;
}

void HandleMessengerArgs::handle(int argc, char* argv[], FakedArgs& fa)
{
    boost::program_options::variables_map vm;
    handler_parse_args()(get_options_description(), vm, argc, argv, fa);

    messenger_args.endpoint = vm["messenger.endpoint"].as<std::string>();
    messenger_args.archive = vm["messenger.archive"].as<bool>();
    messenger_args.archive_rendered = vm["messenger.archive_rendered"].as<bool>();
}

std::shared_ptr<boost::program_options::options_description>
HandleMessengerArgsGrp::get_options_description()
{
    return HandleMessengerArgs::get_options_description();
}

std::size_t HandleMessengerArgsGrp::handle(int argc, char* argv[], FakedArgs& fa, std::size_t option_group_index)
{
    HandleMessengerArgs::handle(argc, argv, fa);
    return option_group_index;
}

const MessengerArgs& HandleMessengerArgsGrp::get_args() const
{
    return HandleMessengerArgs::messenger_args;
}

const std::string& HandleMessengerArgsGrp::get_endpoint() const
{
    return HandleMessengerArgs::messenger_args.endpoint;
}

bool HandleMessengerArgsGrp::get_archive() const
{
    return HandleMessengerArgs::messenger_args.archive;
}

bool HandleMessengerArgsGrp::get_archive_rendered() const
{
    return HandleMessengerArgs::messenger_args.archive_rendered;
}
