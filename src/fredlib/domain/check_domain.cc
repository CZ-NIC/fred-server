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
 *  @file check_domain.cc
 *  domain check
 */

#include <string>

#include "src/fredlib/domain/check_domain.h"
#include "src/fredlib/domain/domain_name.h"
#include "src/fredlib/zone/zone.h"
#include "src/fredlib/object/object.h"

#include "src/fredlib/opcontext.h"
#include "src/fredlib/db_settings.h"
#include "src/fredlib/object_states.h"

namespace Fred
{
    CheckDomain::CheckDomain(const std::string& fqdn, const bool _is_system_registrar)
    : fqdn_(fqdn), is_system_registrar_(_is_system_registrar)
    {}

    bool CheckDomain::is_invalid_syntax() const
    {
        if(Domain::general_domain_name_syntax_check(fqdn_))
        {
            return false;
        }

        return true;
    }

    bool CheckDomain::is_invalid_handle(OperationContext& ctx) const
    {
        try
        {
            if(!Domain::is_rfc1123_compliant_host_name(fqdn_)) {
                return true;
            }

            //remove optional root dot from fqdn
            std::string no_root_dot_fqdn = Fred::Zone::rem_trailing_dot(fqdn_);

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
                else {
                    throw;
                }
            }

            //domain_name_validation
            if(!Fred::Domain::DomainNameValidator(is_system_registrar_)
                .set_checker_names(Fred::Domain::get_domain_name_validation_config_for_zone(ctx, zone.name))
                .set_zone_name(Fred::Domain::DomainName(zone.name))
                .set_ctx(ctx)
                .exec(Fred::Domain::DomainName(fqdn_), std::count(zone.name.begin(), zone.name.end(), '.') + 1) // skip zone labels
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

    bool CheckDomain::is_bad_zone(OperationContext& ctx) const
    {
        try
        {
            //remove optional root dot from fqdn
            std::string no_root_dot_fqdn = Fred::Zone::rem_trailing_dot(fqdn_);

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
                else {
                    throw;
                }
            }
        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }

        return false;//meaning ok
    }

    bool CheckDomain::is_bad_length(OperationContext& ctx) const
    {
        try
        {
            //remove optional root dot from fqdn
            std::string no_root_dot_fqdn = Fred::Zone::rem_trailing_dot(fqdn_);

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
                else {
                    throw;
                }
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

    bool CheckDomain::is_blacklisted(OperationContext& ctx) const
    {
        try
        {
            //remove optional root dot from fqdn
            std::string no_root_dot_fqdn = Fred::Zone::rem_trailing_dot(fqdn_);

            //check blacklist regexp for match with fqdn
            Database::Result bl_res  = ctx.get_conn().exec_params(
                "SELECT id FROM domain_blacklist b "
                "WHERE $1::text ~ b.regexp AND NOW()>=b.valid_from "
                "AND (b.valid_to ISNULL OR NOW()<b.valid_to) "
            , Database::query_param_list(no_root_dot_fqdn));
            if(bl_res.size() > 0)//positively blacklisted
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

    bool CheckDomain::is_registered(OperationContext& ctx, std::string& conflicting_fqdn_out) const
    {
        try
        {
            //remove optional root dot from fqdn
            std::string no_root_dot_fqdn = Fred::Zone::rem_trailing_dot(fqdn_);

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
                    return false;//zone not found
                }
                else {
                    throw;
                }
            }

            if(zone.is_enum)
            {
                Database::Result conflicting_fqdn_res  = ctx.get_conn().exec_params(
                    "SELECT o.name, o.id FROM object_registry o WHERE o.type=get_object_type_id('domain'::text) "
                    "AND o.erdate ISNULL ""AND (($1::text LIKE '%.'|| o.name) "
                    "OR (o.name LIKE '%.'||$1::text) OR o.name=LOWER($1::text)) LIMIT 1"
                , Database::query_param_list(no_root_dot_fqdn));
                if(conflicting_fqdn_res.size() > 0)//have conflicting_fqdn
                {
                    conflicting_fqdn_out = static_cast<std::string>(conflicting_fqdn_res[0][0]);
                    return true;
                }
            }
            else
            {//is not ENUM
                Database::Result conflicting_fqdn_res  = ctx.get_conn().exec_params(
                    "SELECT o.name, o.id FROM object_registry o WHERE o.type=get_object_type_id('domain'::text) "
                    "AND o.erdate ISNULL AND o.name=LOWER($1::text) LIMIT 1"
                , Database::query_param_list(no_root_dot_fqdn));
                if(conflicting_fqdn_res.size() > 0)//have conflicting_fqdn
                {
                    conflicting_fqdn_out = static_cast<std::string>(conflicting_fqdn_res[0][0]);
                    return true;
                }

            }
        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return false;//meaning ok
    }

    bool CheckDomain::is_registered(OperationContext& ctx) const
    {
        std::string conflicting_fqdn_out;
        return is_registered(ctx, conflicting_fqdn_out);
    }


    bool CheckDomain::is_available(OperationContext& ctx) const
    {
        try
        {
            if(is_invalid_handle(ctx)
            || is_bad_length(ctx)
            || is_registered(ctx)
            || is_blacklisted(ctx))
            {
                return false;
            }
        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return true;//meaning ok
    }

    std::string CheckDomain::to_string() const
    {
        return Util::format_operation_state("CheckDomain",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("fqdn",fqdn_))
        );
    }

}//namespace Fred

