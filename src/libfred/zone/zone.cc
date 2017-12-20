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
 *  @file zone.cc
 *  zone
 */


#include "src/libfred/zone/zone.hh"


namespace LibFred {
namespace Zone {

    std::string rem_trailing_dot(const std::string& fqdn)
    {
        if(!fqdn.empty() && fqdn.at(fqdn.size()-1) == '.') return fqdn.substr(0,fqdn.size()-1);
        return fqdn;
    }

    Data get_zone(OperationContext& ctx, const std::string& zone_name)
    {
        Database::Result zone_res = ctx.get_conn().exec_params(
            "SELECT id, enum_zone, fqdn, dots_max, ex_period_min, ex_period_max, val_period"
            " FROM zone WHERE fqdn=lower($1::text) FOR SHARE"
            , Database::query_param_list(zone_name));

        if(zone_res.size() == 1)
        {
            return Data(static_cast<unsigned long long>(zone_res[0][0])// zone.id
                , static_cast<bool>(zone_res[0][1])//is_enum_zone
                , static_cast<std::string>(zone_res[0][2])//zone_name
                , static_cast<unsigned>(zone_res[0][3])//dots_max
                , static_cast<unsigned>(zone_res[0][4])//ex_period_min
                , static_cast<unsigned>(zone_res[0][5])//ex_period_max
                , static_cast<unsigned>(zone_res[0][6])//val_period
                );
        }
        throw std::runtime_error("not found");
    }

    ///zone name in db have to be in lower case
    Data find_zone_in_fqdn(OperationContext& ctx, const std::string& no_root_dot_fqdn)
    {
        try
        {
            const std::string label_separator(".");//viz rfc1035

            std::string domain(boost::to_lower_copy(no_root_dot_fqdn));

            Database::Result available_zones_res = ctx.get_conn().exec(
                "SELECT fqdn FROM zone ORDER BY length(fqdn) DESC");

            if (available_zones_res.size() == 0)
            {
                BOOST_THROW_EXCEPTION(InternalError("missing zone configuration"));
            }

            for (Database::Result::size_type i = 0 ; i < available_zones_res.size(); ++i)
            {
                std::string zone  = static_cast<std::string>(available_zones_res[i][0]);
                std::string dot_zone  = label_separator + zone;
                int from = domain.length() - dot_zone.length();
                if(from >= 1)
                {
                    if (domain.find(dot_zone, from) != std::string::npos)
                    {
                        try
                        {
                            return get_zone(ctx,zone);
                        }
                        catch(const std::exception& ex)
                        {
                            BOOST_THROW_EXCEPTION(Exception().set_unknown_zone_in_fqdn(no_root_dot_fqdn));
                        }
                    }
                }
            }//for available_zones_res

            BOOST_THROW_EXCEPTION(Exception().set_unknown_zone_in_fqdn(no_root_dot_fqdn));
        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(std::string("fqdn: ")+no_root_dot_fqdn);
            throw;
        }
    }

} // namespace Zone
} // namespace LibFred

