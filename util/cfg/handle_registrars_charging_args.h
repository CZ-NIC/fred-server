/*  
 * Copyright (C) 2013  CZ.NIC, z.s.p.o.
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
 *  @file
 *  registrars charging config
 */

#ifndef HANDLE_REGISTRARS_CHARGING_ARGS_H_
#define HANDLE_REGISTRARS_CHARGING_ARGS_H_

#include <iostream>
#include <exception>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include "faked_args.h"
#include "handle_args.h"

namespace po = boost::program_options;

/**
 * @class HandleRegistrarsChargingArgs
 * @brief registrars charging config
 */
class HandleRegistrarsChargingArgs : public HandleArgs
{
public:
    bool charging_of_epp_create_and_renew_domain_disabled;

    boost::shared_ptr<po::options_description>
    get_options_description()
    {
        boost::shared_ptr<po::options_description> opts_descs(
                new po::options_description(std::string("Registrars charging configuration")));
        opts_descs->add_options()
                ("registrars_charging.charging_of_epp_create_and_renew_domain_disabled",
                 po::value<bool>()->default_value(false),
                 "disabled charging for EPP commands create and renew domain");

        return opts_descs;
    }//get_options_description
    void handle( int argc, char* argv[],  FakedArgs &fa)
    {
        po::variables_map vm;
        handler_parse_args(get_options_description(), vm, argc, argv, fa);
        charging_of_epp_create_and_renew_domain_disabled = vm["registrars_charging.charging_of_epp_create_and_renew_domain_disabled"].as<bool>();
    }//handle
};//class HandleRegistrarsChargingArgs

/**
 * @class HandleRegistrarsChargingArgsGrp
 * @brief registrars charging config with option groups
 */

class HandleRegistrarsChargingArgsGrp : public HandleGrpArgs
                            , private HandleRegistrarsChargingArgs
{
public:

    boost::shared_ptr<boost::program_options::options_description>
        get_options_description()
    {
        return HandleRegistrarsChargingArgs::get_options_description();
    }//get_options_description
    std::size_t handle( int argc, char* argv[],  FakedArgs &fa
            , std::size_t option_group_index)
    {
        HandleRegistrarsChargingArgs::handle(argc, argv, fa);
        return option_group_index;
    }//handle

    bool get_charging_of_epp_create_and_renew_domain_disabled()
        {return HandleRegistrarsChargingArgs::charging_of_epp_create_and_renew_domain_disabled;}
};//class HandleRegistrarsChargingArgsGrp

#endif //HANDLE_REGISTRARS_CHARGING_ARGS_H_
