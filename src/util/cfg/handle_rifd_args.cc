/*
 * Copyright (C) 2018  CZ.NIC, z.s.p.o.
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

#include "src/util/cfg/handle_rifd_args.hh"
#include "src/backend/epp/contact/impl/cznic/specific.hh"
#include "src/backend/epp/contact/impl/set_unused/config.hh"

using CzNicSpecific = Epp::Contact::Impl::CzNic::Specific;
using SetUnusedConfig = Epp::Contact::Impl::SetUnused::Config;

HandleRifdArgs::~HandleRifdArgs()
{
}

std::shared_ptr<boost::program_options::options_description> HandleRifdArgs::get_options_description()
{
    auto opts_descs = std::make_shared<boost::program_options::options_description>(
            std::string("Registrar interface configuration"));
    opts_descs->add_options()
            ("rifd.session_max",
             boost::program_options::value<unsigned>()->default_value(200),
             "RIFD maximum number of sessions")
            ("rifd.session_timeout",
             boost::program_options::value<unsigned>()->default_value(300),
             "RIFD session timeout")
            ("rifd.session_registrar_max",
             boost::program_options::value<unsigned>()->default_value(5),
             "RIFD maximum number active sessions per registrar")
            ("rifd.epp_update_domain_keyset_clear",
             boost::program_options::value<bool>()->default_value(false),
             "EPP command update domain will also clear keyset when changing NSSET")
            ("rifd.epp_operations_charging",
             boost::program_options::value<bool>()->default_value(false),
             "Turns on/off EPP operations credit charging")
            ("rifd.epp_update_contact_enqueue_check",
             boost::program_options::value<bool>()->default_value(false),
             "turn enqueueing of automatic check after contact check via EPP on/off")
            ("rifd.check",
             boost::program_options::value<std::string>()->default_value(std::string()),
             ("type of checks applied in EPP operations; possible values: "
              "<empty>, " +
              CzNicSpecific::get_check_name() + ", " +
              SetUnusedConfig::get_check_name()).c_str());

    Epp::Contact::ConfigCheck::add_options_description<CzNicSpecific>(*opts_descs);
    Epp::Contact::ConfigCheck::add_options_description<SetUnusedConfig>(*opts_descs);
    return opts_descs;
}

void HandleRifdArgs::handle(int argc, char* argv[], FakedArgs &fa)
{
    boost::program_options::variables_map vm;
    handler_parse_args()(get_options_description(), vm, argc, argv, fa);

    rifd_session_max = vm["rifd.session_max"].as<unsigned>();
    rifd_session_timeout = vm["rifd.session_timeout"].as<unsigned>();
    rifd_session_registrar_max = vm["rifd.session_registrar_max"].as<unsigned>();
    rifd_epp_update_domain_keyset_clear = vm["rifd.epp_update_domain_keyset_clear"].as<bool>();
    rifd_epp_operations_charging = vm["rifd.epp_operations_charging"].as<bool>();
    epp_update_contact_enqueue_check = vm["rifd.epp_update_contact_enqueue_check"].as<bool>();
    rifd_check.set_name(vm["rifd.check"].as<std::string>());
    rifd_check.set_all_values<CzNicSpecific>(vm);
    rifd_check.set_all_values<SetUnusedConfig>(vm);
}
