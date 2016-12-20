/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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

#include "src/epp/domain/impl/domain_enum_validation.h"


bool is_new_enum_domain_validation_expiration_date_invalid(
        const boost::gregorian::date& new_valexdate,//local date
        const boost::gregorian::date& current_local_date ,
        const unsigned enum_validation_period,//in months
        const boost::optional<boost::gregorian::date>& current_valexdate, //if not set, ENUM domain is not currently validated
        Fred::OperationContext& _ctx
        )
    {
        bool validation_continuation = false;
        if(current_valexdate.is_initialized())
        {
            const boost::gregorian::date validation_continuation_begin = boost::gregorian::from_simple_string(
                static_cast<std::string>(_ctx.get_conn().exec_params(Database::ParamQuery
                    ("SELECT (").param_date(*current_valexdate)
                    (" - val::bigint * ('1 day'::interval))::date ")
                    ("FROM enum_parameters WHERE name = 'enum_validation_continuation_window'")
                    )[0][0]));

            if(current_local_date >= validation_continuation_begin
                && current_local_date < *current_valexdate)
            {
                validation_continuation = true;
            }
        }

        const boost::gregorian::date max_valexdate = boost::gregorian::from_simple_string(
            static_cast<std::string>(_ctx.get_conn().exec_params(Database::ParamQuery
            ("SELECT (").param_date(validation_continuation ? *current_valexdate : current_local_date)
            (" + ").param_bigint(enum_validation_period)(" * ('1 month'::interval))::date ")
            )[0][0]));
        ;

        //if new_valexdate is not valid
        if(new_valexdate.is_special()
        || new_valexdate <= current_local_date
        || new_valexdate > max_valexdate)
        {
            return true;
        }
        return false;
    }
