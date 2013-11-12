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
 *  templates for contact enums
 */

#ifndef CONTACT_ENUM_H
#define CONTACT_ENUM_H

#include <string>

#include "fredlib/opexception.h"
#include "fredlib/opcontext.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"

namespace Fred {
namespace Contact {

/**
* Gets "type of identification" database id and locks row for reading.
* @param ssntype is "type of identification" to get
* @param ctx contains reference to database and logging interface
* @param ex_ptr is pointer to exception instance to set in case of failure,
* if 0, new exception instance is created, set and thrown in case of failure,
* if not 0, refered exception instance is only set
* @param ex_setter is exception member pointer used to set data into exception in case of failure
* @return "type of identification" database id locked for reading
*/
template <class EXCEPTION, typename EXCEPTION_SETTER>
unsigned long long get_ssntype_id(const Optional<std::string>& ssntype, OperationContext& ctx
        , EXCEPTION* ex_ptr, EXCEPTION_SETTER ex_setter)
{
    unsigned long long ssntype_id = 0;
    if(ssntype.isset())
    {
        Database::Result ssntype_res = ctx.get_conn().exec_params(
            "SELECT id FROM enum_ssntype WHERE type = UPPER($1::text) FOR SHARE"
            , Database::query_param_list(ssntype.get_value()));
        if(ssntype_res.size() == 0)
        {
            //BOOST_THROW_EXCEPTION(EXCEPTION().set_unknown_ssntype(ssntype.get_value()));
            if(ex_ptr == 0)//make new exception instance, set data and throw
            {
                BOOST_THROW_EXCEPTION((EXCEPTION().*ex_setter)(ssntype.get_value()));
            }
            else//set unknown ssntype to given exception instance (don't throw) and return
            {
                (ex_ptr->*ex_setter)(ssntype.get_value());
                return 0;
            }
        }
        if(ssntype_res.size() != 1)
        {
            BOOST_THROW_EXCEPTION(InternalError("failed to get ssntype"));
        }

        ssntype_id = static_cast<unsigned long long>(ssntype_res[0][0]);
    }
    return ssntype_id;
}

/**
* Gets two character country code and locks row for reading.
* @param country is name of the country or country code
* @param ctx contains reference to database and logging interface
* @param ex_ptr is pointer to exception instance to set in case of failure,
* if 0, new exception instance is created, set and thrown in case of failure,
* if not 0, refered exception instance is only set
* @param ex_setter is exception member pointer used to set data into exception in case of failure
* @return two character country code locked for reading
*/
template <class EXCEPTION, typename EXCEPTION_SETTER>
std::string get_country_code(const Optional<std::string>& country, OperationContext& ctx
        , EXCEPTION* ex_ptr, EXCEPTION_SETTER ex_setter)
{
    std::string country_code;
    if(country.isset())
    {
        Database::Result country_code_res = ctx.get_conn().exec_params(
            "SELECT id FROM enum_country WHERE id = $1::text OR country = $1::text OR country_cs = $1::text FOR SHARE"
            , Database::query_param_list(country.get_value()));
        if(country_code_res.size() == 0)
        {
            //BOOST_THROW_EXCEPTION(EXCEPTION().set_unknown_country(country.get_value()));
            if(ex_ptr == 0)//make new exception instance, set data and throw
            {
                BOOST_THROW_EXCEPTION((EXCEPTION().*ex_setter)(country.get_value()));
            }
            else//set unknown country to given exception instance (don't throw) and return ""
            {
                (ex_ptr->*ex_setter)(country.get_value());
                return "";
            }
        }
        if(country_code_res.size() != 1)
        {
            BOOST_THROW_EXCEPTION(InternalError("failed to get country"));
        }

        country_code = static_cast<std::string>(country_code_res[0][0]);
    }
    return country_code;
}

}//namespace Contact
}//namespace Fred

#endif//CONTACT_ENUM_H
