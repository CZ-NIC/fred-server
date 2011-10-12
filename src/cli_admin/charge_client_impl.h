/*
 * Copyright (C) 2011  CZ.NIC, z.s.p.o.
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
 *  @charge_client_impl.h
 *  Operation charging implementation header
 */

#ifndef _CHARGE_CLIENT_IMPL_H_
#define _CHARGE_CLIENT_IMPL_H_

#include "cfg/config_handler_decl.h"
// #include "cfg/handle_database_args.h"
#include "cli_admin/handle_adminclientselection_args.h"
#include "log/context.h"
#include "cli_admin/chargeclient.h"


/**
 * \class price_add_impl
 * \brief admin client implementation of price_add
 */
struct charge_request_fee_impl
{
    void operator() ()
    {
        Logging::Context ctx("charge_request_fee_impl");
        Admin::ChargeClient charge_client (
            CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientChargeRequestFeeArgsGrp>()->params
        );
        charge_client.runMethod();
    }
};

#endif /*_CHARGE_CLIENT_IMPL_H_*/
