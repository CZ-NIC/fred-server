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
 *  @file handle_rifd_args.hh
 *  registrar interface config
 */

#ifndef HANDLE_RIFD_ARGS_HH_1F2430A2C0A949A7AB56A73A8B39CC9D
#define HANDLE_RIFD_ARGS_HH_1F2430A2C0A949A7AB56A73A8B39CC9D

#include "src/util/cfg/faked_args.hh"
#include "src/util/cfg/handle_args.hh"

#include "src/backend/epp/contact/config_check.hh"

#include <boost/program_options.hpp>

/**
 * \class HandleRifdArgs
 * \brief registrar interface config
 */
class HandleRifdArgs : public HandleArgs
{
public:
    ~HandleRifdArgs();
    std::shared_ptr<boost::program_options::options_description> get_options_description()override;
    void handle(int argc, char* argv[], FakedArgs &fa)override;

    unsigned rifd_session_max;
    unsigned rifd_session_timeout;
    unsigned rifd_session_registrar_max;
    bool rifd_epp_update_domain_keyset_clear;
    bool rifd_epp_operations_charging;
    bool epp_update_contact_enqueue_check;

    Epp::Contact::ConfigDataFilter rifd_contact_data_filter;
};

#endif
