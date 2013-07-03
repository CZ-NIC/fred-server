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


#include "fredlib/zone/zone.h"


namespace Fred {
namespace Zone {

    Data find_zone_in_fqdn(OperationContext& ctx, const std::string& fqdn)
    {
        try
        {
            std::string domain(boost::to_lower_copy(fqdn));

            Database::Result available_zones_res = ctx.get_conn().exec(
                "SELECT fqdn FROM zone ORDER BY length(fqdn) DESC");

            if (available_zones_res.size() == 0)
            {
                BOOST_THROW_EXCEPTION(InternalError("missing zone configuration"));
            }

            for (Database::Result::size_type i = 0 ; i < available_zones_res.size(); ++i)
            {
                std::string zone  = static_cast<std::string>(available_zones_res[i][0]);
                int from = domain.length() - zone.length();
                if(from > 1)
                {
                    if (domain.find(zone, from) != std::string::npos)
                    {
                        Database::Result zone_res = ctx.get_conn().exec_params(
                            "SELECT id, enum_zone, fqdn  FROM zone WHERE fqdn=lower($1::text) FOR SHARE"
                            , Database::query_param_list(domain.substr(from, std::string::npos)));

                        if(zone_res.size() == 1)
                        {
                            return Data(static_cast<unsigned long long>(zone_res[0][0])// zone.id
                                , static_cast<bool>(zone_res[0][1])//is_enum_zone
                                , static_cast<std::string>(zone_res[0][2])//zone_name
                                );
                        }
                    }
                }
            }//for available_zones_res

            BOOST_THROW_EXCEPTION(Exception().set_unknown_zone_in_fqdn(fqdn));
        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(std::string("fqdn: ")+fqdn);
            throw;
        }
    }

}//namespace Zone
}//namespace Fred

