/*
 * Copyright (C) 2015  CZ.NIC, z.s.p.o.
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
 *  implementation for MojeID CORBA conversion
 */

#include "mojeid_corba_conversion.h"

namespace CorbaConversion
{
    void Unwrapper_Registry_MojeID_Date_var_into_boost_date::unwrap( const CORBA_TYPE& ct_in, NON_CORBA_TYPE& nct_out)
    {
        if(ct_in.operator->() == NULL)
        {
            throw PointerIsNULL();
        }

        nct_out = boost::gregorian::from_simple_string(ct_in->value.in());

        if(nct_out.is_special())
        {
            throw ArgumentIsSpecial();
        }
    }

    void Wrapper_boost_date_into_Registry_MojeID_Date_var::wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out )
    {
        if(nct_in.is_special())
        {
            throw ArgumentIsSpecial();
        }

        Registry::MojeID::Date_var res = new Registry::MojeID::Date;
        res->value = wrap_into<CORBA::String_var>(boost::gregorian::to_iso_extended_string(nct_in));
        ct_out = res._retn();
    }

    void Unwrapper_Registry_MojeID_DateTime_var_into_boost_ptime::unwrap( const CORBA_TYPE& ct_in, NON_CORBA_TYPE& nct_out)
    {
        if(ct_in.operator->() == NULL)
        {
            throw PointerIsNULL();
        }

        nct_out = boost::date_time::parse_delimited_time<ptime>(ct_in->value.in(), 'T');

        if(nct_out.is_special())
        {
            throw ArgumentIsSpecial();
        }
    }

    void Wrapper_boost_ptime_into_Registry_MojeID_DateTime_var::wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out )
    {
        if(nct_in.is_special())
        {
            throw ArgumentIsSpecial();
        }

        Registry::MojeID::DateTime_var res = new Registry::MojeID::DateTime;
        res->value = wrap_into<CORBA::String_var>(boost::posix_time::to_iso_extended_string(nct_in));
        ct_out = res._retn();
    }

    void Unwrapper_Registry_MojeID_NullableDate_var_into_Nullable_boost_date::unwrap( const CORBA_TYPE& ct_in, NON_CORBA_TYPE& nct_out)
    {
        if(ct_in.in() == NULL)
        {
            nct_out = Nullable<boost::gregorian::date>();
        }
        else
        {
            nct_out = Nullable<boost::gregorian::date>(
                unwrap_into<boost::gregorian::date>(
                    Registry::MojeID::Date_var(
                        new Registry::MojeID::Date(
                            ct_in.in()->_boxed_in()))));
        }
    }

    void Wrapper_Nullable_boost_date_into_Registry_MojeID_NullableDate_var::wrap( const NON_CORBA_TYPE& nct_in, CORBA_TYPE& ct_out )
    {
        if(nct_in.isnull())
        {
            ct_out = NULL;
        }
        else
        {
            Registry::MojeID::NullableDate_var valuetype_date = new Registry::MojeID::NullableDate;
            valuetype_date->value(CorbaConversion::wrap_into<Registry::MojeID::Date_var>(nct_in.get_value())->value.in());
            ct_out = valuetype_date._retn();
        }
    }


}
