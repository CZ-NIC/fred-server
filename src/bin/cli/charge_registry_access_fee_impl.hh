/*
 * Copyright (C) 2019  CZ.NIC, z. s. p. o.
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

#ifndef CHARGE_REGISTRY_ACCESS_FEE_IMPL_HH_8D913585F8EE478281812C6013EEC041
#define CHARGE_REGISTRY_ACCESS_FEE_IMPL_HH_8D913585F8EE478281812C6013EEC041

#include "src/bin/cli/charge_registry_access_fee.hh"
#include "src/bin/cli/handle_adminclientselection_args.hh"
#include "src/util/cfg/config_handler_decl.hh"
#include "util/log/context.hh"

#include <boost/date_time/gregorian/gregorian.hpp>

#include <vector>

void charge_registry_access_fee_annual_impl()
{
    Logging::Context ctx("charge_registry_access_fee_annual_impl");
    const auto params = CfgArgGroups::instance()->get_handler_ptr_by_type<HandleChargeRegistryAccessFeeAnnualArgsGrp>()->params;
    const auto date_from = boost::gregorian::from_string(params.year + "-01-01");
    const auto date_to = date_from + boost::gregorian::years(1) - boost::gregorian::days(1);

    Admin::chargeRegistryAccessFee(params.all_registrars, params.registrars, params.except_registrars, date_from, date_to);
}

void charge_registry_access_fee_monthly_impl()
{
    Logging::Context ctx("charge_registry_access_fee_monthly_impl");
    const auto params = CfgArgGroups::instance()->get_handler_ptr_by_type<HandleChargeRegistryAccessFeeMonthlyArgsGrp>()->params;
    const auto date_from = boost::gregorian::from_string(params.year_month + "-01");
    const auto date_to = date_from + boost::gregorian::months(1) - boost::gregorian::days(1);

    Admin::chargeRegistryAccessFee(params.all_registrars, params.registrars, params.except_registrars, date_from, date_to);
}

#endif
