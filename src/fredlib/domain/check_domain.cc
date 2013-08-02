/*
 * Copyright (C) 2012  CZ.NIC, z.s.p.o.
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
 *  @file check_domain.cc
 *  domain check
 */

#include <string>

#include "fredlib/domain/check_domain.h"
#include "fredlib/domain/domain_name.h"
#include "fredlib/zone/zone.h"
#include "fredlib/object/object.h"

#include "fredlib/opcontext.h"
#include "fredlib/db_settings.h"
#include "fredlib/object_states.h"

namespace Fred
{
    CheckDomain::CheckDomain(const std::string& fqdn)
    : fqdn_(fqdn)
    {}
    bool CheckDomain::is_invalid_handle(OperationContext& ctx)
    {
        try
        {
            //check general domain name syntax
            if(!Domain::general_domain_name_syntax_check(fqdn_)) return true;

            //remove optional root dot from fqdn
            std::string no_root_dot_fqdn = Fred::Domain::rem_trailing_dot(fqdn_);

            //get zone
            Zone::Data zone;
            try
            {
                zone = Zone::find_zone_in_fqdn(ctx, no_root_dot_fqdn);
            }
            catch(const Zone::Exception& ex)
            {
                if(ex.is_set_unknown_zone_in_fqdn()
                        && (ex.get_unknown_zone_in_fqdn().compare(no_root_dot_fqdn) == 0))
                {
                    return true;//zone not found
                }
                else
                    throw;
            }

            //domain_name_validation
            if(!Fred::Domain::DomainNameValidator()
                .set_checker_names(Fred::Domain::get_domain_name_validation_config_for_zone(ctx,zone.name))
                .set_zone_name(Fred::Domain::DomainName(zone.name))
                .set_ctx(ctx)
                .exec(Fred::Domain::DomainName(fqdn_)
                    , std::count(zone.name.begin(), zone.name.end(),'.')+1)//skip zone labels
            )
            {
                return true;
            }
        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return false;//meaning ok
    }

    bool CheckDomain::is_bad_zone(OperationContext& ctx)
    {
        try
        {
            //remove optional root dot from fqdn
            std::string no_root_dot_fqdn = Fred::Domain::rem_trailing_dot(fqdn_);

            //zone
            Zone::Data zone;
            try
            {
                zone = Zone::find_zone_in_fqdn(ctx, no_root_dot_fqdn);
            }
            catch(const Zone::Exception& ex)
            {
                if(ex.is_set_unknown_zone_in_fqdn()
                        && (ex.get_unknown_zone_in_fqdn().compare(no_root_dot_fqdn) == 0))
                {
                    return true;
                }
                else
                    throw;
            }
        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }

        return false;//meaning ok
    }

    bool CheckDomain::is_bad_length(OperationContext& ctx)
    {
        try
        {
            //remove optional root dot from fqdn
            std::string no_root_dot_fqdn = Fred::Domain::rem_trailing_dot(fqdn_);

            //get zone
            Zone::Data zone;
            try
            {
                zone = Zone::find_zone_in_fqdn(ctx, no_root_dot_fqdn);
            }
            catch(const Zone::Exception& ex)
            {
                if(ex.is_set_unknown_zone_in_fqdn()
                        && (ex.get_unknown_zone_in_fqdn().compare(no_root_dot_fqdn) == 0))
                {
                    return true;//zone not found
                }
                else
                    throw;
            }

            //check number of labels
            if(std::count(no_root_dot_fqdn.begin(), no_root_dot_fqdn.end(),'.')+1//fqdn labels number
                > std::count(zone.name.begin(), zone.name.end(),'.')+1+zone.dots_max)//max labels by zone
            {
                return true;
            }
        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return false;//meaning ok
    }


    std::ostream& operator<<(std::ostream& os, const CheckDomain& i)
    {
        return os << "#CheckDomain fqdn: " << i.fqdn_
                ;
    }
    std::string CheckDomain::to_string()
    {
        std::stringstream ss;
        ss << *this;
        return ss.str();
    }


}//namespace Fred

