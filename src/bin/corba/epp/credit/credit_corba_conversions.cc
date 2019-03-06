/*
 * Copyright (C) 2017-2019  CZ.NIC, z. s. p. o.
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
#include "src/bin/corba/epp/credit/credit_corba_conversions.hh"

#include "src/bin/corba/util/corba_conversions_string.hh"

namespace LibFred {
namespace Corba {

namespace {

void wrap_ZoneCredit(
        const Epp::Credit::ZoneCredit& src,
        ccReg::ZoneCredit_str& dst)
{
    dst.zone_fqdn = wrap_string_to_corba_string(src.get_zone_fqdn());
    if (src.has_credit())
    {
        dst.price = wrap_string_to_corba_string(src.get_credit().get_string());
    }
    else
    {
        const std::string no_credit_information = "0";
        dst.price = wrap_string_to_corba_string(no_credit_information);
    }
}

} // namespace LibFred::Corba::{anonymous}

void wrap_ClientCreditOutputData(
        const Epp::Credit::ClientCreditOutputData& src,
        ccReg::ZoneCredit& dst)
{
    dst.length(0);
    dst.length(src.size());
    unsigned idx = 0;
    for (Epp::Credit::ClientCreditOutputData::const_iterator src_item_ptr = src.begin();
         src_item_ptr != src.end(); ++src_item_ptr, ++idx)
    {
        ccReg::ZoneCredit_str dst_item;
        wrap_ZoneCredit(*src_item_ptr, dst_item);
        dst[idx] = dst_item;
    }
}


} // namespace LibFred::Corba
} // namespace LibFred
